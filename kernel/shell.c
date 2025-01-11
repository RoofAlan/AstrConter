/*
 *
 *		shell.c
 *		内核自带的shell交互程序
 *
 *		2024/7/1 By Rainy101112
 *		基于 GPL-3.0 开源协议
 *		Copyright © 2020 ViudiraTech，保留最终解释权。
 *
 */

#include "fifo.h"
#include "keyboard.h"
#include "vbe.h"
#include "debug.h"
#include "types.h"
#include "stdlib.h"
#include "cpu.h"
#include "pci.h"
#include "printk.h"
#include "task.h"
#include "sched.h"
#include "astrknl.h"
#include "acpi.h"
#include "timer.h"
#include "file.h"
#include "pl_readline.lib.h"
#include "os_terminal.lib.h"
#include "elf.h"
#include "bmp.h"
#include "klogo.lib.h"
#include "mouse.h"
#include "serial.h"

#define MAX_COMMAND_LEN	100
#define MAX_ARG_NR		30

static int root_status = 0;

/* 解析命令行字符串 */
static int cmd_parse(uint8_t *cmd_str, uint8_t **argv, uint8_t token) // 用uint8_t是因为" "使用8位整数
{
	int arg_idx = 0;

	while (arg_idx < MAX_ARG_NR) {
		argv[arg_idx] = 0;
		arg_idx++;
	}
	uint8_t *next = cmd_str;					// 下一个字符
	int	argc = 0;								// 这就是要返回的argc了

	while (*next) {								// 循环到结束为止
		while (*next == token) next++;			// 多个token就只保留第一个，windows cmd就是这么处理的
		if (*next == 0) break;					// 如果跳过完token之后结束了，那就直接退出
		argv[argc] = next;						// 将首指针赋值过去，从这里开始就是当前参数
		while (*next && *next != token) next++;	// 跳到下一个token
		if (*next) {							// 如果这里有token字符
			*next++ = 0;						// 将当前token字符设为0（结束符），next后移一个
		}
		if (argc > MAX_ARG_NR) return -1;		// 参数太多，超过上限了
		argc++; // argc增一，如果最后一个字符是空格时不提前退出，argc会错误地被多加1
	}
	return argc;
}

/* help命令 */
void shell_help(void)
{
	printk("+-----------+---------------------------------------+\n"
           "| Command   | Command description                   |\n"
           "+-----------+---------------------------------------+\n"
           "| help      | Show help like this.                  |\n"
           "| clear     | Clear the screen.                     |\n"
           "| cpuid     | List for CPU information.             |\n"
           "| lspci     | List for All the PCI device.          |\n"
           "| stask     | List for all task processes.          |\n"
           "| hltst     | Test the Kernel-Panic.                |\n"
           "| taskkill  | Kill task which is running.           |\n"
           "| uname     | Show unix name.                       |\n"
           "| flushing  | Test screen flushing.                 |\n"
           "| echo      | String output.                        |\n"
           "| poweroff  | Power off your computer.              |\n"
           "| reboot    | Reboot the system.                    |\n"
           "| cd        | Change the working directory.         |\n"
           "| ls        | Lists the current directory files.    |\n"
		   "| slogo     | Show the kernel logo                  |\n"
		   "| free      | Display the memory status             |\n"
		   "| cat       | Read the file                         |\n"
		   "| mount     | Mount the device to a directory       |\n"
		   "| umount    | Umount the device                     |\n"
		   "| mkdir     | Create direcotory                     |\n"
		   "| touch     | Create file                           |\n"
           "| cetsl     | Enable/Disable serial console output. |\n"
           "+-----------+---------------------------------------+\n");
	printk("+----------------------------------+----------------------------------+\n"
           "| OST Shortcut key                 | Instructions                     |\n"
           "+----------------------------------+----------------------------------+\n"
           "| Ctrl+Shift+(F1~F8)               | Switch to different theme.       |\n"
           "| Ctrl+Shift+(ArrowUp/ArrowDown)   | Scroll up/down history.          |\n"
           "| Ctrl+Shift+(PageUP/PageDown)     | Scroll up/down history by page.  |\n"
           "+----------------------------------+----------------------------------+\n\n");
	return;
}

void shell_free(void)
{
	printk("       total    used     free\n");
	printk("Mem:   %-9d%-9d%d\n", (glb_mboot_ptr->mem_upper + glb_mboot_ptr->mem_lower), (get_kernel_memory_usage() / 1024),
                                  (glb_mboot_ptr->mem_upper + glb_mboot_ptr->mem_lower) - (get_kernel_memory_usage() / 1024));
	return;
}

void shell_clear(void)
{
	screen_clear();
	return;
}

void shell_lspci(void)
{
	pci_device_info();
	printk("\n");
	return;
}

void shell_proc(void)
{
	int p = print_task();
	printk("\nNumber of processes: %d\n", p);
	return;
}

void shell_hltst(void)
{
	panic(P000);
}

void shell_cat(int argc, char *argv[]) {
	if (argc == 1) {
        printk("Usage: %s <file>\n", argv[0]);
        return;
    }
    char bufx[100];
    sprintf(bufx, "%s", argv[1]);
    vfs_node_t file = vfs_open(bufx);
    if (file != NULL) {
        char *buf = kmalloc(file->size);
        if (vfs_read(file, buf, 0, file->size) == -1) {
            goto read_error;
        }
        for (size_t i = 0; i < file->size; i++) {
            printk("%c", buf[i]);
        }
        printk("\n");
        return;
    }
    read_error:
    printk("Cannot read file.\n");
}

void shell_taskkill(int argc, char *argv[])
{
	int value = atoi(argv[1]);

	if (argc > 1) {
		if (value == 0 && (argv[1][0] != '0' || argv[1][1] != '\0')) {
			printk("Argument is not an integer.\n");
		} else if (argc > 1) {
			task_kill(value);
		}
	} else {
		printk("Usage: %s [PID]\n", argv[0]);
	}
	printk("\n");
	return;
}

void shell_uname(int argc, char *argv[])
{
	if (argc > 1) {
		if (strcmp(argv[1], "-a") == 0) {
			printk("AstrConter-Kernel %s %s (build-%d) x86 AstrConter\n", KERNL_VERS, OS_INFO_, KERNL_BUID);
		} else if (strcmp(argv[1], "-m") == 0) {
			printk("x86\n");
		} else if (strcmp(argv[1], "-v") == 0) {
			printk("%s %s (build-%d) \n", KERNL_VERS, OS_INFO_, KERNL_BUID);
		} else if (strcmp(argv[1], "-o") == 0) {
			printk("AstrConter\n");
		} else {
			printk("uname: invalid option -- '%s'\n", argv[1]);
		}
	} else {
		printk("Usage: %s [OPTION]\n", argv[0]);
		printk("       -a print all information.\n");
		printk("       -m print the machine hardware name.\n");
		printk("       -v print the kernel version.\n");
		printk("       -o print the operating system.\n");
	}
	printk("\n");
	return;
}

void shell_flushing(int argc, char *argv[])
{
	if (argc > 1) {
		int flushtime = atoi(argv[1]);
		if (flushtime > 0) {
			const static char* conversation_list = "\033[31m[TESTFLUSH] Hello! This is a flushing test!!\n\033[0m"
                                                   "\033[32m[TESTFLUSH] A quick fox jump over a lazy dog.\n\033[0m"
                                                   "\033[33m[TESTFLUSH] Success is not the end, failure is not the death.\033[0m\n"
                                                   "\033[34m[TESTFLUSH] Believe!\n\033[0m"
                                                   "\033[35m[TESTFLUSH] I love apples, but I don't like bananas.\n\033[0m"
                                                   "\033[36m[TESTFLUSH] Uinxed-Kernel - ViudiraTech - Microfish & Rainy101112 & XIAOYI12 ...\n\033[0m"
                                                   "\033[37m[TESTFLUSH] Open source on github!!\n\033[0m";
			for (int times = 0; times <= flushtime; times++) {
				printk("%s", conversation_list);
			}
		}
		printk("\n");
		return;
	} else {
		printk("Usage: %s [COUNT]\n", argv[0]);
		printk("\n");
		return;
	}
}

void shell_echo(int argc, char *argv[])
{
	for(int i=1; i < argc; i++){
		printk("%s ",argv[i]);
	}
	printk("\n");
	return;
}

void shell_poweroff(void)
{
	printk("The system is going to power off NOW!");
	sleep(200);
	power_off();
}

void shell_reboot(void)
{
	printk("The system is going to reboot NOW!");
	sleep(200);
	power_reset();
}

void shell_cd(int argc, char *argv[])
{
	if (argc > 1) {
		if (file_cd(argv[1]) == -1) printk("cd: %s: No such file or directory\n", argv[1]);
	}
	return;
}

void shell_ls(int argc, char *argv[])
{
	if (argc > 1) {
		if (file_ls(argv[1]) == -1) printk("Not such file or directory\n");
	} else {
		file_ls(vfs_node_to_path(working_dir));
	}
	return;
}

void shell_cetsl(int argc, char *argv[])
{
	if (strcmp(argv[1], "1") == 0 || strcmp(argv[1], "true") == 0) {
		vbe_to_serial(1);
		printk("The kernel console has been output to the serial port.\n");
	} else if (strcmp(argv[1], "0") == 0 || strcmp(argv[1], "false") == 0) {
		vbe_to_serial(0);
		printk("Stopped outputting the kernel console to the serial port.\n");
	} else {
		printk("Usage: %s [BOOLEAN]\n", argv[0]);
	}
	return;
}

void shell_slogo(int argc, char *argv[])
{
	bmp_analysis((Bmp *)klogo, vbe_get_width() / 2 - 150, 100, 0);
}

void shell_mount(int argc, char *argv[])
{
	if(argv[1] == NULL || argv[2] == NULL) {
		printk("Usage: %s <device> <path>\n", argv[0]);
		return;
	}
	vfs_node_t path = vfs_open(argv[2]);
	if (vfs_open(argv[1]) == NULL) {
		printk("Failed to find device: %s\n", argv[1]);
		return;
	}
	if(path == NULL) {
		printk("Failed to open: %s\n",argv[2]);
		return;
	} else if(strcmp(argv[2], "/") == 0) {
		if(root_status != 1) {
			printk("Waring: you are trying to mount a device to root ('%s' -> '/')\n", argv[1]);
		} else {
			printk("Error: root filesystem already mounted\n");
			print_erro("Error: root filesystem already mounted\n");
			return;
		}
	}
	if(vfs_mount(argv[1], path) != 0) {
		printk("Failed to mount device: %s\n", argv[1]);
		return;
	} else {
		root_status = 1;
	}
	return;
}

void shell_umount(int argc, char *argv[]) {
	if(argc != 2) {
		printk("Usage: %s <MountPoint>\n",argv[0]);
		return;
	}
	if (vfs_umount(argv[1]) == -1) {
		printk("Failed to umount: %s\n", argv[1]);
		return;
	}
	return;
}

void shell_mkdir(int argc, char *argv[]) {
	if(argc != 2) {
		printk("Usage: %s <Name>\n",argv[0]);
		return;
	}
	if(vfs_mkdir(argv[1]) == -1) {
		printk("Failed to create directory: %s\n",argv[1]);
		return;
	}
	return;
}

void shell_touch(int argc, char *argv[]) {
	if(argc != 2) {
		printk("Usage: %s <filename>\n",argv[0]);
		return;
	}
	if(vfs_mkfile(argv[1]) == -1) {
		printk("Failed to create file: %s\n", argv[1]);
		return;
	}
	return;
}
typedef struct builtin_cmd
{
	const char *name;
	void (*func)(int, char **);
} builtin_cmd_t;

builtin_cmd_t builtin_cmds[] = {
	{"clear", (void (*)(int, char **))shell_clear},
	{"help", (void (*)(int, char **))shell_help},
	{"cpuid", (void (*)(int, char **))print_cpu_info},
	{"lspci", (void (*)(int, char **))shell_lspci},
	{"stask", (void (*)(int, char **))shell_proc},
	{"hltst", (void (*)(int, char **))shell_hltst},
	{"kill", (void (*)(int, char **))shell_taskkill},
	{"uname", (void (*)(int, char **))shell_uname},
	{"flushing", (void (*)(int, char **))shell_flushing},
	{"echo", (void (*)(int, char **))shell_echo},
	{"poweroff", (void (*)(int, char **))shell_poweroff},
	{"reboot", (void (*)(int, char **))shell_reboot},
	{"cd", (void (*)(int, char **))shell_cd},
	{"ls", (void (*)(int, char **))shell_ls},
	{"free",(void (*)(int, char **))shell_free},
	{"cetsl", shell_cetsl},
	{"slogo", shell_slogo},
	{"cat", shell_cat},
	{"mount", shell_mount},
	{"umount", shell_umount},
	{"mkdir", shell_mkdir},
	{"touch", shell_touch}
};

/* 内建命令数量 */
static const int builtin_cmd_num = sizeof(builtin_cmds) / sizeof(builtin_cmd_t);

/* 在预定义的命令数组中查找给定的命令字符串 */
int find_cmd(uint8_t *cmd)
{
	for (int i = 0; i < builtin_cmd_num; ++i)
	{
		if (strcmp((const char *)cmd, builtin_cmds[i].name) == 0){
			return i;
		}
	}
	return -1;
}

static int plreadln_getch(void)
{
	char ch;
	getch(&ch);
	if (ch == 0x0d) {
		return PL_READLINE_KEY_ENTER;
	}
	if (ch == 0x7f) {
		return PL_READLINE_KEY_BACKSPACE;
	}
	if (ch == 0x9) {
		return PL_READLINE_KEY_TAB;
	}
	if (ch == 0x1b) {
		ch = plreadln_getch();
		if (ch == '[') {
			ch = plreadln_getch();
			switch (ch) {
			case 'A':
				return PL_READLINE_KEY_UP;
			case 'B':
				return PL_READLINE_KEY_DOWN;
			case 'C':
				return PL_READLINE_KEY_RIGHT;
			case 'D':
				return PL_READLINE_KEY_LEFT;
			case 'H':
				return PL_READLINE_KEY_HOME;
			case 'F':
				return PL_READLINE_KEY_END;
			case '5':
				if (plreadln_getch() == '~')
					return PL_READLINE_KEY_PAGE_UP;
				break;
			case '6':
				if (plreadln_getch() == '~')
				    return PL_READLINE_KEY_PAGE_DOWN;
				break;
			default:
				return -1;
			}
		}
	}
	return ch;
}

static void plreadln_putch(int ch)
{
	putchar(ch);
}

static void handle_tab(char *buf, pl_readline_words_t words)
{
	for (int i = 0; i < builtin_cmd_num; ++i) {
		pl_readline_word_maker_add(builtin_cmds[i].name, words, 1, ' ');
	}
}

static void plreadln_flush(void)
{
	/* Nothing */
}

/* shell主程序 */
void shell(const char *cmdline)
{
	printk("Basic shell program v1.0\n");
	printk("Type 'help' for help.\n\n");

	char prompt[128];
	uint8_t cmd[MAX_COMMAND_LEN];
	uint8_t *argv[MAX_ARG_NR];
	int argc = -1;

	memset(cmd, 0, MAX_COMMAND_LEN);

	pl_readline_t pl;
	pl = pl_readline_init(plreadln_getch, plreadln_putch, plreadln_flush, handle_tab);

	while (1) {
		while(cmd[0] == 0) {
			memset(cmd, 0, MAX_COMMAND_LEN);					// 清空上轮输入
			sprintf(prompt, "\033[32mUser@localhost: \033[34m%s\033[0m # ", vfs_node_to_path(working_dir));
			pl_readline(pl, prompt, (char *)cmd, MAX_COMMAND_LEN);
		}

		/* com就是完整的命令 */
		//if (cmd[0] == 0) continue;					// 只有一个回车，continue
		argc = cmd_parse(cmd, argv, ' ');

		/* argc, argv 都拿到了 */
		if (argc == -1) {
			print_erro("shell: number of arguments exceed MAX_ARG_NR(30)\n");
			printk("Out of MAX_ARGS(30)");
			continue;
		} else if (argc == 0) {
			vbe_write_newline();
			continue;
		}

		int cmd_index = find_cmd(argv[0]);
		if (cmd_index < 0) {
			// sprintf(bin_name, "%s.AS", (const char *)argv[0]);
			// if (vfs_do_search(vfs_open(vfs_node_to_path(working_dir)), bin_name)) {
			// 	sprintf(bin_path, "%s/%s", working_dir->name, bin_name);
			// 	elf_thread(bin_path, argv[1], bin_name, USER_TASK);
			// } else {
			/* 找不到该命令 */
			printk("%s: Command not found\n", argv[0]);
			// }
		} else {
			builtin_cmds[cmd_index].func(argc, (char **)argv);
		}
		memset(cmd, 0, MAX_COMMAND_LEN);
	}
}

#include "idt.h"
#include "console.h"
#include "gdt.h"
#include "debug.h"
#include "printk.h"
#include "memory.h"
#include "keyboard.h"
#include "astrknl.h"
#include "pci.h"
#include "serial.h"
#include "block.h"
#include "timer.h"
#include "beep.h"
#include "cpu.h"
#include "vbe.h"
#include "multiboot.h"
#include "task.h"
#include "fpu.h"
#include "sched.h"
#include "bmp.h"
#include "acpi.h"
#include "klogo.lib.h"
#include "os_terminal.lib.h"
#include "vdisk.h"
#include "devfs.h"
#include "fat.h"
#include "list.h"
#include "vfs.h"
#include "file.h"
#include "mouse.h"
#include "syscall.h"
#include "tty.h"
#include "bochs.h"
#include "cmdline.h"

void shell(const char *); // 声明shell程序入口

/* 内核shell进程 */
int kthread_shell(void *arg)
{
	char *cmdline = strchr((char *)arg, ' ');
	if(cmdline)
		++cmdline;
	shell(cmdline);
	return 0;
}

/* 内核入口 */
void kernel_init(multiboot_t *glb_mboot_ptr)
{
	program_break_end = program_break + 0x300000 + 1 + KHEAP_INITIAL_SIZE;
	memset(program_break, 0, program_break_end - program_break);

	console_to_serial(1);
	init_bochs(glb_mboot_ptr);
	init_vbe(glb_mboot_ptr, 0x151515, 0xffffff);
	/* 检测内存是否达到最低要求 */
	if ((glb_mboot_ptr->mem_upper + glb_mboot_ptr->mem_lower) / 1024 + 1 < 16) {
		panic(P001);
	}

	print_succ("");
	printk("AstrConter-Kernel "KERNL_VERS" %s (build-%d) Powered by Uinxed-kernel -- of Viudira\n",OS_INFO_ ,KERNL_BUID);
	print_succ("");
	printk("KernelArea: 0x00000000 - 0x%08X | GraphicsBuffer: 0x%08X\n", program_break_end,
                                                                    	 glb_mboot_ptr->framebuffer_addr);
	init_cmdline(glb_mboot_ptr);
	if(find_cmdline_args("klogo=on", get_cmdline(), get_cmdline_count()) == 0) {
		bmp_analysis((Bmp *)klogo, vbe_get_width() - 200, 0, 1);
	}
	char *vdr, *mdn;
	int pbs, vbs; 
	get_cpu_info(&vdr, &mdn, &pbs, &vbs);
	print_succ("");
	printk("CPU name: %s | Vendor: %s\n", mdn, vdr);
	print_succ("");
	printk("CPU cache: %d | Virtual Address 0x%x\n", pbs, vbs);

	init_gdt();
	init_idt();
	ISR_registe_Handle();
	acpi_init();

	init_timer(1);
	init_pit();

	init_page(glb_mboot_ptr);
	setup_free_page();
	init_fpu();
	init_pci();
	init_serial();
	init_keyboard();
	mouse_init();
	init_sched();
	syscall_init();
	block_init();
	
	vfs_init();
	devfs_regist();
	fatfs_regist();
	file_init();
	if (vfs_do_search(vfs_open("/dev"), "sda")) {
		vfs_mount("/dev/sda", vfs_open("/"));
		print_succ("Root filesystem mounted ('/dev/sda' -> '/')\n");
	} else {
		if(strcmp(find_cmdargs("dbg-shell",get_cmdline(), get_cmdline_count()), "on") == 0) {
			print_warn("The root file system could not be mounted.\n");
		} else {
			print_erro("The root file system could not be mounted.\n");
			krn_halt();
		}
	}

	init_timer(1);
	init_pit();

	enable_intr();

	system_beep(1000);
	sleep(10);
	system_beep(0);

	terminal_set_color_scheme(0);	// 重置终端主题
	console_to_serial(0);				// 停止输出内核启动日志到串口
	
	enable_scheduler();
	print_succ("");
	printk("AstrCounter On tty%d\n", get_boot_tty());
	if(find_cmdline_args("dbg-shell=on",get_cmdline(), get_cmdline_count()) == 0) {
		kernel_thread(kthread_shell, (void *)((glb_mboot_ptr->flags&MULTIBOOT_INFO_CMDLINE)?glb_mboot_ptr->cmdline:0), "Shell", USER_TASK);
	} else {
		print_succ("Run /sbin/init ...\n");
		if (vfs_do_search(vfs_open("/"), "sbin")) {
			if (vfs_do_search(vfs_open("/sbin"), "init")) {
				elf_thread("/sbin/init", 0, "INIT", USER_TASK);
			} else {
				print_erro("Unable to find '/sbin/init'\n");
			}
		} else {
			print_erro("No working init found '/sbin/init'\n");
		}
	}
	terminal_set_auto_flush(0);
	while(1) {
		uint32_t eflags = load_eflags();
		if(eflags & (1 << 9)) disable_intr();
		terminal_flush();
		if(eflags & (1 << 9)) enable_intr();
		free_pages();
		__asm__ __volatile__("hlt");
	}
}

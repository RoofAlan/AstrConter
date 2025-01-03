/*
 *
 *	cmdline.c
 *	内核处理命令行输入
 *
 *	2024/12/28 By RoofAlan
 *	基于GPL-3.0开源协议
 *
 *
 */

#include "multiboot.h"
#include "cmdline.h"
#include "string.h"
#include "printk.h"

#define MAX_TOKENS 100
#define TOKEN_LENGTH 100

static char *cmdv[MAX_TOKENS];
static char tokens_storage[MAX_TOKENS][TOKEN_LENGTH];
int num_tokens;

/* 字符分割 */
int split_string(const char *str,const char delim) {
	int token_count = 0;
	const char *p = str;
	char *token_start;

	while (*p && token_count < MAX_TOKENS) {
		token_start = (char *)p; // 子串的开始位置
		while (*p && *p != delim) { // 假设分隔符是空格
			p++;
		}
		int token_len = p - token_start;
		if (token_len > 0) { // 如果找到了非空的子串
			strncpy(tokens_storage[token_count], token_start, token_len);
			tokens_storage[token_count][token_len] = '\0'; // 确保子串以空字符结尾
			cmdv[token_count] = tokens_storage[token_count]; // 存储指向子串的指针
			token_count++;
		}
		if (*p) {
			p++; // 跳过分隔符
		}
	}

	return token_count;
}

/* 返回命令行数组 */
char **get_cmdline() {
	return cmdv;
}

/* 返回命令行参数的总数 */
int get_cmdline_count() {
	return num_tokens;
}

/* 查找参数 */
int find_cmdline_args(const char *arg, char **cmdv_input,int num) {
	for(int c = 0;c<num;c++) {
		if(strcmp(arg, cmdv_input[c]) == 0) {
			return 0;
		}
	}
	return 1;
}

/* 获取参数值 */
char *find_cmdargs(const char *arg, char **cmdv_input, int num) {
    // 遍历字符串数组
    for (int i = 0; i < num; i++) {
        // 查找当前字符串中是否包含 arg
        char *eq = strchr(cmdv_input[i], '=');
        if (eq != NULL) {
            // 检查 arg 是否匹配等号前的部分
            if (strncmp(cmdv_input[i], arg, eq - cmdv_input[i]) == 0 && arg[eq - cmdv_input[i]] == '\0') {
                // 返回等号后面的值
                return eq + 1;
            }
        }
    }
    return NULL; // 如果未找到，返回 NULL
}

/* 初始化 */
int init_cmdline(multiboot_t *info) {
	char *str = (char *)info->cmdline;
	num_tokens = split_string(str, ' ');

	char **cmdv_ptr = get_cmdline();
	print_succ("Cmdline: ");
	for (int i = 0; i < num_tokens; i++) {
		printk("%s ", cmdv_ptr[i]);
	}
	printk("\n");
	return 0;
}

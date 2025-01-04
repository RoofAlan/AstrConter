/*
 *
 *	    cmdline.h
 *	    内核处理命令行输入头文件
 *
 *  	2024/12/28 By RoofAlan
 *  	基于GPL-3.0开源协议
 *
 */

#ifndef INCLUDE_CMDLINE_H
#define INCLUDE_CMDLINE_H

#include "multiboot.h"

/* 初始化命令行处理程序 */
int init_cmdline(multiboot_t *info);

/* 获取命令行参数 */
char **get_cmdline();

/* 获取命令行参数数量 */
int get_cmdline_count();

/* 查找参数 */
int find_cmdline_args(const char *arg, char **cmdv_input, int num);

/* 获取参数的值 */
char *find_cmdargs(const char *arg, char **cmdv_input, int num);

/* 打印cmdline信息 */
void print_cmdline();

#endif

/*
 *
 *		printk.h
 *		内核调试和打印信息程序头文件
 *
 *		2024/6/27 By Rainy101112
 *		基于 GPL-3.0 开源协议
 *		Copyright © 2020 ViudiraTech，保留最终解释权。
 *
 */

#include "vargs.h"

/* 内核打印字符 */
void putchar(char ch);

/* 内核打印字符串 */
void printk(const char *format, ...);

/* 格式化打印日志 */
void printlog_serial(const char *format, ...);

/* 格式化打印到串口 */
void printk_serial(const char *format, ...);

/* 内核日志输出位置(serial or TTY) */
void klog_to(int serial);

/* 带前缀的打印函数 */
void print_busy(const char *str); // 打印带有”[ ** ]“的字符串
void print_succ(const char *str); // 打印带有”[ OK ]“的字符串
void print_warn(const char *str); // 打印带有”[WARN]“的字符串
void print_erro(const char *str); // 打印带有”[ERRO]“的字符串
void print_time(const char *str);// 打印带有[HH:MM:SS]的字符串

//#define print_busy(msg) printk("[    \033[31m**\033[0m    %s:%d] %s", __FILE__, __LINE__, msg)
//#define print_succ(msg) printk("[    \033[36mOK\033[0m    %s:%d] %s", __FILE__, __LINE__, msg)
//#define print_warn(msg) printk("[  \033[33mWARN\033[0m  %s:%d] %s", __FILE__, __LINE__, msg)
//#define print_erro(msg) printk("[  \033[31mERRO\033[0m  %s:%d] %s", __FILE__, __LINE__, msg)
#define print_fl(msg) printk("[%s:%d] %s",__FILE__,__LINE__,msg)

/* 格式化字符串并将其输出到一个字符数组中 */
int vsprintf(char *buff, const char *format, va_list args);

/* 将格式化的输出存储在字符数组中 */
void sprintf(char *str,const char *fmt, ...);

/*
 *
 *		console.h
 *		VGA文字模式下的控制台驱动头文件
 *
 *		2024/6/27 By Rainy101112
 *		基于 GPL-3.0 开源协议
 *		Copyright © 2020 ViudiraTech，开放所有权利。
 *
 */

#ifndef INCLUDE_CONSOLE_H_
#define INCLUDE_CONSOLE_H_

#include "types.h"

typedef
enum real_color {
	rc_black = 0,
	rc_blue = 1,
	rc_green = 2,
	rc_cyan = 3,
	rc_red = 4,
	rc_magenta = 5,
	rc_brown = 6,
	rc_light_grey = 7,
	rc_dark_grey = 8,
	rc_light_blue = 9,
	rc_light_green = 10,
	rc_light_cyan = 11,
	rc_light_red = 12,
	rc_light_magenta = 13,
	rc_light_brown  = 14,
	rc_white = 15
} real_color_t;

/* 清屏操作 */
void console_clear(void);

/* 清屏操作（带颜色） */
void console_clear_color(real_color_t back, real_color_t fore);

/* 屏幕输出一个字符（带颜色） */
void console_putc_color(char c, real_color_t back, real_color_t fore);

/* 屏幕打印一个空行 */
void console_write_newline(void);

/* 屏幕打印一个以 \0 结尾的字符串（默认黑底白字） */
void console_write(const char *cstr);

/* 屏幕打印一个以 \0 结尾的字符串（带颜色） */
void console_write_color(const char *cstr, real_color_t back, real_color_t fore);

/* 屏幕输出一个十六进制的整型数 */
void console_write_hex(uint32_t n, real_color_t back, real_color_t fore);

/* 屏幕输出一个十进制的整型数 */
void console_write_dec(uint32_t n, real_color_t back, real_color_t fore);

/* 屏幕输出映射到串口 */
void console_to_serial(int op);

#endif // INCLUDE_CONSOLE_H_

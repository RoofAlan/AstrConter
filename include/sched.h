/*
 *
 *		sched.h
 *		进程调度程序头文件
 *
 *		2024/9/1 By Rainy101112
 *		基于 GPL-3.0 开源协议
 *		Copyright © 2020 ViudiraTech，开放所有权利。
 *
 */

#ifndef INCLUDE_SCHED_H_
#define INCLUDE_SCHED_H_

#include "task.h"
#include "idt.h"

/* 可调度进程链表 */
extern struct task_struct *running_proc_head;

/* 等待进程链表 */
extern struct task_struct *wait_proc_head;

/* 当前运行的任务 */
extern struct task_struct *current;

/* 初始化任务调度 */
void init_sched(void);

/* 允许进程调度 */
void enable_scheduler(void);

/* 停止进程调度 */
void disable_scheduler(void);

/* 任务调度 */
void schedule(pt_regs *regs);

/* 任务切换准备 */
void change_task_to(struct task_struct *next, pt_regs *regs);

/* 任务切换 */
void switch_to(struct context *prev, struct context *next);

#endif // INCLUDE_SCHED_H_

# 欢迎来到AstrConter内核项目
这是一个基于Uinxed-Kernel的开源内核，因原项目为GPL-3.0协议，所以该项目也会基于GPL-3.0 协议开源 

## 项目特点
**自由开放** 保证没有种族歧视，可供个人学习研究、商业用途
**同步更新** 该项目会跟随Uinxed-Kernel不断更新（当然，前提是我有时间）

## 环境搭建
**ArchLinux**
```bash
sudo pacman -Sy make gcc clang nasm grub-pc xorriso qemu-system
```
**Debian & Ubuntu & Kali**
```bash
sudo apt update && sudo apt install make apt clang nasm xorriso qemu-system
```
如果您是Windows用户，可以使用Cygwin或WSL搭建环境并编译

## 构建
**Clone** 将项目克隆到本地
**Make** 使用make命令进行编译
**Run！** 使用make run运行打包好的iso镜像、make runk运行内核、make run-db运行打包好的镜像并伴随着调试模式、make runk-db同理

## 外部链接
**Uinxed-Kernel** https://github.com/ViudiraTech/Uinxed-Kernel

C_SOURCES		= $(shell find . -name "*.c")
C_OBJECTS		= $(patsubst %.c, %.o, $(C_SOURCES))
S_SOURCES		= $(shell find . -name "*.s")
S_OBJECTS		= $(patsubst %.s, %.o, $(S_SOURCES))

INFO = $(shell whoami)@$(shell hostname)

OTHER_OBJECTS	= ./lib/libos_terminal.a ./lib/pl_readline.lib ./lib/libelf_parse.lib ./lib/klogo.lib

CC				= gcc
LD				= ld
ASM				= nasm
RM				= rm
QEMU			= qemu-system-x86_64

C_FLAGS			= -Wall -Werror -Wcast-align -Winline -Wwrite-strings \
                  -c -I include -m32 -O3 -g -DNDEBUG -nostdinc -fno-pic \
                  -fno-builtin -fno-stack-protector -D OS_INFO_=\"$(INFO)\" \
				  -mno-mmx -mno-sse -mno-sse2

LD_FLAGS		= -T scripts/kernel.ld -m elf_i386 --strip-all
ASM_FLAGS		= -f elf -g -F stabs

all: link grub_iso

.c.o:
	@printf "  CC\t$@\n"
	@$(CC) $(C_FLAGS) $< -o $@

.s.o:
	@printf "  ASM\t$<\n"
	@$(ASM) $(ASM_FLAGS) $<

link:$(S_OBJECTS) $(C_OBJECTS)
	@printf "  LD\tastrknl\n"
	@$(LD) $(LD_FLAGS) $(S_OBJECTS) $(C_OBJECTS) $(OTHER_OBJECTS) -o astrknl

.PHONY:grub
grub_iso:astrknl
	@echo
	@echo Packing ISO file...
	@mkdir -p iso/boot/grub
	@cp $< iso/boot/

	@echo 'set timeout=3' > iso/boot/grub/grub.cfg
	@echo 'set default=0' >> iso/boot/grub/grub.cfg

	@echo 'menuentry "AstrConter"{' >> iso/boot/grub/grub.cfg
	@echo '	multiboot /boot/astrknl dbg-shell=on tty=1 klogo=on' >> iso/boot/grub/grub.cfg
	@echo '	boot' >> iso/boot/grub/grub.cfg
	@echo '}' >> iso/boot/grub/grub.cfg

	@grub-mkrescue --locales="" --output=AstrConter.iso iso
	@rm -rf iso
	@echo Compilation complete.

.PHONY:clean
clean:
	$(RM) -f $(S_OBJECTS) $(C_OBJECTS) astrknl AstrConter.iso

.PHONY:qemu_iso
run:
	$(QEMU) -cdrom AstrConter.iso -serial stdio -m 1G

.PHONY:qemu_iso_debug
run_db:
	$(QEMU) -cdrom AstrConter.iso -serial stdio -d in_asm -m 1G

.PHONY:qemu_kernel
runk:
	$(QEMU) -kernel astrknl -serial stdio -m 1G

.PHONY:qemu_kernel_debug
runk_db:
	$(QEMU) -kernel astrknl -serial stdio -d in_asm -m  1G

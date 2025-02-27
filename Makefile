ifeq ($(VERBOSE), 1)
  V=
  Q=
else
  V=@printf "[Build] $@ ...\n";
  Q=@
endif

BUILDDIR		?= build

C_SOURCES		:= $(shell find * -name "*.c")
S_SOURCES		:= $(shell find * -name "*.s")
OBJS			:= $(C_SOURCES:%.c=$(BUILDDIR)/%.o) $(S_SOURCES:%.s=$(BUILDDIR)/%.o)
DEPS			:= $(OBJS:%.o=%.d)
LIBS			:= $(wildcard lib/lib*.a)

CC				= gcc
LD				= ld
ASM				= nasm
RM				= rm
QEMU			= qemu-system-x86_64
QEMU_FLAGS		= -serial stdio -audiodev pa,id=speaker -machine pcspk-audiodev=speaker -m 1G
KERNEL_APPEND = console=tty0 loglevel=4 shell-log=off root=/dev/cdrom0

C_FLAGS			= -MMD -Wall -Werror -Wcast-align -Winline -Wwrite-strings \
                  -c -I include -m32 -O3 -g -DNDEBUG -nostdinc -fno-pic \
				  -mno-mmx -mno-sse -mno-sse2 \
                  -fno-builtin -fno-stack-protector -DOS_INFO_=\"$(shell whoami)@$(shell hostname)\"

LD_FLAGS		= -T scripts/kernel.ld -m elf_i386 --strip-all
ASM_FLAGS		= -f elf -g -F stabs

all: AstrConter.iso

makedirs:
	$(Q)mkdir -p $(dir $(OBJS))

$(BUILDDIR)/%.o: %.c | makedirs
	$(Q)printf "  CC      $@\n"
	$(Q)$(CC) $(C_FLAGS) -o $@ $<

$(BUILDDIR)/%.o: %.s | makedirs
	$(Q)printf "  ASM     $@\n"
	$(Q)$(ASM) $(ASM_FLAGS) -o $@ $<

astrknl: $(OBJS) $(LIBS)
	$(Q)printf "  LD      $@\n"
	$(Q)$(LD) $(LD_FLAGS) -o $@ $^

.PHONY: grub_iso
grub_iso: AstrConter.iso

.PHONY: config
menuconfig:
	@kconfig-mconf Kconfig
	@python3 Kconfig.py

AstrConter.iso: astrknl
	@echo
	@printf "Packing ISO file...\n"
	@mkdir -p iso/boot/grub
	@cp $< iso/boot/

	@echo 'set timeout=3' > iso/boot/grub/grub.cfg
	@echo 'set default=0' >> iso/boot/grub/grub.cfg

	@echo 'menuentry "AstrConter"{' >> iso/boot/grub/grub.cfg
	@echo '	multiboot /boot/astrknl $(KERNEL_APPEND)' >> iso/boot/grub/grub.cfg
	@echo '	boot' >> iso/boot/grub/grub.cfg
	@echo '}' >> iso/boot/grub/grub.cfg

	@printf 'Hello, AstrConter! This hello.txt. You should use the "cat" command to read this file' > iso/hello.txt

	@grub-mkrescue --locales="" --output=AstrConter.iso iso
	@rm -rf iso
	@printf " Compilation complete.\n"

.PHONY: clean
clean:
	$(Q)$(RM) -f $(OBJS) $(DEPS) astrknl AstrConter.iso
	$(Q)$(RM) -rf build/ .xmake

.PHONY: run
run: AstrConter.iso
	$(QEMU) $(QEMU_FLAGS) -cdrom AstrConter.iso

.PHONY: run_db
run_db: AstrConter.iso
	$(QEMU) $(QEMU_FLAGS) -cdrom AstrConter.iso -d in_asm

.PHONY: runk
runk: astrknl
	$(QEMU) $(QEMU_FLAGS) -kernel astrknl -append "$(KERNEL_APPEND)"

.PHONY: runk_db
runk_db: astrknl
	$(QEMU) $(QEMU_FLAGS) -kernel astrknl -d in_asm -append "$(KERNEL_APPEND)"

.PHONY: run_kvm
run_kvm: AstrConter.iso
	$(QEMU) $(QEMU_FLAGS) -cdrom AstrConter.iso -enable-kvm

.PHONY: runk_kvm
runk_kvm: astrknl
	$(QEMU) $(QEMU_FLAGS) -kernel astrknl -append "$(KERNEL_APPEND)" -enable-kvm

.PRECIOUS: $(OBJS)

-include $(DEPS)

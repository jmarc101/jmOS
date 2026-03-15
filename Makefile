OS_NAME    := jmOS
OS_TARGET  := bare
ISODIR     := isodir
KERNEL     := $(OS_NAME)-$(OS_TARGET)
ISO        := $(KERNEL).iso

QEMU    := qemu-system-i386
AS      := nasm
ASFLAGS := -felf32
LD      := linker/linker.ld
LDFLAGS := -ffreestanding -O2 -nostdlib

CC          := i686-elf-gcc
CFLAGS      := -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Iinclude

# Source files
KERNEL_SRCS := $(wildcard kernel/*.c) $(wildcard kernel/cpu/*.c) $(wildcard kernel/drivers/*.c)
CPU_ASM_SRCS := $(wildcard kernel/cpu/*.asm)
LIB_SRCS    := $(wildcard lib/*.c)

# Object files
KERNEL_OBJS := $(KERNEL_SRCS:.c=.o)
CPU_ASM_OBJS := $(CPU_ASM_SRCS:.asm=.o)
LIB_OBJS    := $(LIB_SRCS:.c=.o)



.PHONY: boot clean all kernel iso run qemu

run: all qemu

all: kernel iso

clean:
	rm -f boot/boot.o $(KERNEL_OBJS) $(CPU_ASM_OBJS) $(LIB_OBJS) $(KERNEL) $(ISO)
	rm -rf $(ISODIR)

# Build bootable ISO image
# Requires: grub-mkrescue, xorriso, mtools
# Notes(bash):
# > creates/overwrites grub.cfg with first line
# >> appends each subsequent line
# Steps:
# - clean staging area
# - dynamically make config file based on GLOBAL vars
# - create ISO file
# - remove staging area
iso: $(KERNEL)
	rm -rf $(ISODIR)
	mkdir -p $(ISODIR)/boot/grub
	cp $(KERNEL) $(ISODIR)/boot/$(KERNEL)
	echo 'menuentry "$(OS_NAME)" {' > $(ISODIR)/boot/grub/grub.cfg
	echo '    multiboot /boot/$(KERNEL)' >> $(ISODIR)/boot/grub/grub.cfg
	echo '}' >> $(ISODIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) $(ISODIR)
	rm -rf $(ISODIR)
	echo built $(ISO)

# Notes(makefile):
# $< dependency t: d1 d2  => d1
# $@ target     t: d      => t
# $^ expands    t: d1 d2  => d1 d2
#
# Bootloader assembly file is assembled into an object file
# The -felf32 flag tells NASM to generate a 32-bit ELF object file,
# which is suitable for linking with other object files in a 32-bit environment.
boot/boot.o: boot/boot.asm
	$(AS) $(ASFLAGS) $< -o $@

kernel/kernel.o: kernel/kernel.c
	$(CC) -c $< -o $@ $(CFLAGS)

# Generic rule for kernel/cpu/*.asm files
kernel/cpu/%.o: kernel/cpu/%.asm
	$(AS) $(ASFLAGS) $< -o $@

kernel: boot/boot.o $(KERNEL_OBJS) $(CPU_ASM_OBJS) $(LIB_OBJS)
	$(CC) -T $(LD) -o $(KERNEL) $(LDFLAGS) $^ -lgcc
	@if grub-file --is-x86-multiboot $(KERNEL); then \
		echo multiboot confirmed; \
	else \
		echo the file is not multiboot; \
	fi \

qemu: $(ISO)
	$(QEMU) -cdrom $(ISO)




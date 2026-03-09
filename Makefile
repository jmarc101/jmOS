AS      := nasm
ASFLAGS := -felf32

.PHONY: boot clean all

all: boot

clean:
	rm -f boot/boot.o

boot: boot/boot.o

# $< dependency => boot/boot.o
# $@ target     => boot/boot.asm
# Bootloader assembly file is assembled into an object file
# The -felf32 flag tells NASM to generate a 32-bit ELF object file,
# which is suitable for linking with other object files in a 32-bit environment.
boot/boot.o: boot/boot.asm
	$(AS) $(ASFLAGS) $< -o $@





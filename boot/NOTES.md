# boot.asm notes

## The Big Picture

grub talks to hardware, loads kernel into memory, gets you into 32bit protected mode.
boot.asm is the glue, speaks multiboot to grub, sets up the stack so C can run.
kernel is the actual code.

grub -> boot.asm -> kernel. each layer hands off to the next.

---

## Constants

`equ` is like #define in C. just naming a value.

bit shifting: `1<<0` = 1, `1<<1` = 2, `1<<2` = 4. readable way to say "bit N".

- MBALIGN: tell grub to align modules to 4096 byte (page) boundaries. matters later when you enable paging
- MEMINFO: tell grub to give you a memory map. which ram is usable, which is reserved by hardware
- MBFLAGS: just OR both together. 1 | 2 = 3
- MAGIC: 0x1BADB002. fixed by the multiboot spec, not your choice. grub scans for this exact value
- CHECKSUM: whatever makes MAGIC + FLAGS + CHECKSUM = 0. grub verifies this to make sure header isnt corrupted

---

## Multiboot Header Section

```nasm
section .multiboot
align 4
    dd MAGIC
    dd MBFLAGS
    dd CHECKSUM
```

`section .multiboot` is its own section so the linker can put it first in the binary.

grub scans the first 8KiB of your kernel looking for the magic number. if its past that it wont find it and wont boot. linker script (written later) puts .multiboot first so its guaranteed to be near the start.

`align 4` pads to next address divisible by 4. spec requires it.

`dd` is "define doubleword". emits 4 bytes into the binary. these three lines are the actual header grub reads.

---

## Stack

```nasm
section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
```

`.bss` is the section for uninitialized data. just reserved space, no values needed.

`align 16` pads to next address divisible by 16. x86 calling convention requires esp to be 16-byte aligned when calling functions.

`resb 16384` reserves 16384 bytes (16 KiB).

`stack_bottom` and `stack_top` are just labels = addresses. NASM replaces them with the actual numbers.

x86 stack grows downward. push does `esp -= 4` then writes. so you start esp at the top (high address) and it grows toward the bottom. thats why you set `esp = stack_top` not `stack_bottom`.

`stack_bottom` isnt used in code, just there for debugging or future stack overflow checks.

---

## _start

```nasm
section .text
global _start
_start:
```

`.text` is the section for executable code.

`global _start` exports the symbol so the linker can see it. without global its private to this file.

`_start` is the first instruction the CPU runs after grub hands off control. grub jumps here.

when _start runs grub already switched CPU into 32bit protected mode, disabled interrupts, disabled paging. you have full control, no OS underneath, no protections, nothing.

### mov esp, stack_top

first thing. set the stack pointer.

why assembly? chicken and egg. cant do this in C because C needs a stack to make function calls. this one mov is literally why boot.asm exists.

### Gap between mov esp and call kernel_main

this empty space is where youll eventually put: GDT setup, paging, FPU init, C++ global constructors. for bare bones skip all of it.

### call kernel_main

```nasm
extern kernel_main
call kernel_main
```

`extern` tells nasm this symbol is defined somewhere else (your C file). linker connects them.

`call` jumps to your kernel. there is no return. kernel runs forever.

### The Hang Loop

```nasm
cli
.hang: hlt
       jmp .hang
```

safety net if kernel_main somehow returns (it shouldnt).

- `cli` disables interrupts (already disabled by grub, just defensive)
- `hlt` halts CPU. since interrupts are off, just stops
- `jmp .hang` if something wakes it anyway (non-maskable interrupt, system management mode) loop back and halt again

---

## No Stdlib

once youre in the kernel: no printf, no malloc, no segfault protection, no debugger. bad pointer = silent corruption or reboot. if you want any of it you build it yourself. thats the whole point.
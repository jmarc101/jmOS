# linker.ld

## What even is the linker

When you write C, each `.c` file gets compiled and assembled independently into a `.o` object file. These files have holes in them, references to functions and variables defined in other files that haven't been resolved yet.

The linker is the last step. It takes all the `.o` files, stitches them together, patches all those holes, and produces one final binary.

```
boot.o   ──┐
vga.o    ──┼──→  [ld -T linker.ld]  →  kernel.elf
kernel.o ──┘
```

Full pipeline for reference:

```
source.c → [preprocessor] → [compiler] → [assembler] → .o file
                                                            ↓
                                                        [linker] → kernel.elf
```

## Why do we need a linker script at all

Normally the linker just follows the OS's rules, the OS loads your program, sets up memory, calls main(). The linker doesn't need to think hard.

Bare metal has no OS. Nobody is setting up memory for you. The linker has no idea where your kernel will land in physical RAM, what address your entry point is at, or how to lay out your sections. Left alone it guesses, and it guesses wrong.

The linker script is you saying: **there's no OS here, so I'm telling you exactly how memory is laid out myself.**

## Why is boot.s also a .o file

`boot.s` is not a binary. It's human-readable assembly text, same as how `.c` is human-readable C. It still needs to go through the assembler to become a `.o` before the linker can touch it. It just skips the compiler step since it's already assembly.

The reason we write the boot file in assembly at all: the very first code that runs needs to set up a stack before C can do anything. C assumes a valid stack exists the moment any function is called. Someone has to set `esp` first, and that has to be assembly.

## Line by line

### `ENTRY(_start)`
Tells the linker which symbol is the entry point. Gets written into the ELF header. GRUB reads that header and jumps to whatever address `_start` landed at. This is why `_start` lives in `boot.s`, it's literally the first thing that runs.

### `. = 2M`
The `.` is the location counter, tracks what address we're currently at. Setting it to `2M` means everything gets placed starting at 2MB in physical RAM. That's where GRUB loads us. 1M used to be the standard but UEFI made that unreliable, 2M is safer.

### `.text BLOCK(4K) : ALIGN(4K)`
`ALIGN(4K)` pads whatever came before so this section starts on a 4KB boundary. Matters later when you set up paging since pages are 4KB, misaligned sections are a headache. `BLOCK(4K)` is older syntax that means the same thing.

`.text` is your **executable code** — functions, instructions. The name "text" is a old Unix convention for the code segment.

### `*(.multiboot)` first inside `.text`
GRUB scans the first 8KB of your kernel image looking for the multiboot magic number. If it's buried deep in the binary GRUB won't find it and won't boot your kernel. Listing `*(.multiboot)` first forces it to land at the very top of the binary, the first bytes GRUB reads.

### `.rodata`
Read-only data. String literals like `"hello"` go here. You don't explicitly put things here, the compiler decides. If the data can't change, it goes in `.rodata` not `.data`.

### `.data`
Initialized globals. If you write `int x = 5;` at global scope, `x` lives here. The value `5` has to actually be stored in the binary so it's there when the kernel loads.

### `.bss`
Stands for **Block Started by Symbol**, ancient Unix term, basically meaningless. Just remember it means **uninitialized globals**:

```c
int x;  // no value → goes in .bss
```

Not stored in the binary (no point storing zeros), just reserved space. Important: no OS will zero this for you on bare metal. You have to do it manually in boot code or your globals contain garbage.

**This is not the stack.** The stack is separate and you set it up manually in `boot.s`.

### `*(COMMON)`
Tentative definitions, globals the compiler isn't sure are duplicated across files. Think of it as overflow `.bss`. Include it or you might silently lose some globals.

## The full memory picture

| Section | Contains | Example |
|---|---|---|
| `.text` | executable code | your functions |
| `.rodata` | constant data | string literals |
| `.data` | initialized globals | `int x = 5;` |
| `.bss` | uninitialized globals | `int x;` |
| stack | function call stack | set up manually in boot.s |

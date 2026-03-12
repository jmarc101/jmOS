# Kernel Refactoring Plan

## Goal
Split `kernel.c` (237 lines) into modular files with headers.

## New Structure
```
include/
├── vga.h          # VGA types, colors, constants
├── terminal.h     # Terminal function declarations
└── string.h       # String utility declarations
kernel/
├── main.c         # Just kernel_main()
├── terminal.c     # Terminal implementation
└── string.c       # strlen implementation
```

---

## Step 1: Create the include directory
```bash
mkdir include
```

## Step 2: Create include/vga.h
```c
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__linux__)
#error "You are not using cross-compiler, things will break ;)"
#endif

#if !defined(__i386__)
#error "This needs to be compiled with ix86-elf compiler"
#endif

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY_ADDRESS 0xB8000

enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | (uint16_t)color << 8;
}
```

## Step 3: Create include/string.h
```c
#pragma once

#include <stddef.h>

size_t strlen(const char* str);
```

## Step 4: Create include/terminal.h
```c
#pragma once

#include "vga.h"

#define NEWLINE_CHAR '\n'
#define SCROLL_HISTORY 100
#define BUFFER_HEIGHT (VGA_HEIGHT + SCROLL_HISTORY)

void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
void terminal_putchar(char c);
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_writedecimal(size_t n);
```

## Step 5: Create kernel/string.c
```c
#include "string.h"

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}
```

## Step 6: Create kernel/terminal.c
```c
#include "terminal.h"
#include "string.h"

static size_t terminal_row;
static size_t terminal_column;
static size_t terminal_offset;
static uint8_t terminal_color;
static uint16_t terminal_buffer[BUFFER_HEIGHT * VGA_WIDTH];
static uint16_t* vga_memory = (uint16_t*)VGA_MEMORY_ADDRESS;

static void draw_buffer(bool init) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t memory_index = y * VGA_WIDTH + x;
            const size_t buffer_index = (y + terminal_offset % BUFFER_HEIGHT) * VGA_WIDTH + x;
            if (init) {
                vga_memory[memory_index] = vga_entry(' ', terminal_color);
            } else {
                vga_memory[memory_index] = terminal_buffer[buffer_index];
            }
        }
    }
}

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_offset = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    draw_buffer(true);
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c) {
    if (c == NEWLINE_CHAR) {
        terminal_column = 0;
        terminal_row++;
    } else {
        terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
        if (++terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            terminal_row++;
        }
    }

    if (terminal_row >= terminal_offset + VGA_HEIGHT) {
        terminal_offset++;
        if (terminal_offset + VGA_HEIGHT >= BUFFER_HEIGHT) {
            terminal_offset = 0;
        }
    }
    if (terminal_row >= VGA_HEIGHT + SCROLL_HISTORY) {
        terminal_row = 0;
    }
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
    draw_buffer(false);
}

void terminal_writedecimal(size_t n) {
    if (n == 0) {
        terminal_putchar('0');
        return;
    }
    char buffer[20];
    size_t i = 0;
    while (n > 0) {
        buffer[i++] = '0' + (n % 10);
        n /= 10;
    }
    while (i > 0) {
        terminal_putchar(buffer[--i]);
    }
}
```

## Step 7: Create kernel/main.c
```c
#include "terminal.h"

void kernel_main(void) {
    terminal_initialize();

    size_t lines = 110;
    for (size_t i = 0; i < lines; i++) {
        terminal_writestring("Lines Number: ");
        terminal_writedecimal(i);
        terminal_writestring("\n");
    }
}
```

## Step 8: Update Makefile

Replace the relevant parts:

```makefile
# Change these lines:
CFLAGS  := -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Iinclude

KERNEL_SRCS := kernel/main.c kernel/terminal.c kernel/string.c
KERNEL_OBJS := $(KERNEL_SRCS:.c=.o)

# Replace the kernel/kernel.o rule with:
kernel/%.o: kernel/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

# Update the kernel target:
kernel: $(KERNEL_OBJS) boot/boot.o
	$(CC) -T $(LD) -o $(KERNEL) $(LDFLAGS) $^ -lgcc
	@if grub-file --is-x86-multiboot $(KERNEL); then \
		echo multiboot confirmed; \
	else \
		echo the file is not multiboot; \
	fi

# Update clean:
clean:
	rm -f boot/boot.o $(KERNEL_OBJS) $(KERNEL) $(ISO)
	rm -rf $(ISODIR)
```

## Step 9: Delete old file and test
```bash
rm kernel/kernel.c
make clean
make all
make run
```

---

## Checklist
- [ ] Create `include/` directory
- [ ] Create `include/vga.h`
- [ ] Create `include/string.h`
- [ ] Create `include/terminal.h`
- [ ] Create `kernel/string.c`
- [ ] Create `kernel/terminal.c`
- [ ] Create `kernel/main.c`
- [ ] Update `Makefile`
- [ ] Delete `kernel/kernel.c`
- [ ] Test with `make run`

---

## Next Steps (After Refactoring)

### Recommended Order

1. **GDT (Global Descriptor Table)**
   - Set up your own segments instead of relying on bootloader's
   - Understand protected mode segmentation

2. **IDT (Interrupt Descriptor Table)**
   - Handle CPU exceptions (divide by zero, page faults, etc.)
   - Foundation for everything else

3. **PIT Timer (Programmable Interval Timer)**
   - First hardware interrupt (IRQ0)
   - Gives you a heartbeat/tick

4. **Keyboard Driver**
   - PS/2 keyboard via IRQ1
   - Now you can actually interact with your OS

5. **Simple Shell**
   - Read input, parse commands, execute
   - Tie it all together

### Why This Order

```
GDT → IDT → PIT → Keyboard → Shell
         ↓
    (interrupts required)
```

You can't handle keyboard or timer without interrupts. You need GDT set up properly before IDT.

### Resources

- [OSDev Wiki - GDT](https://wiki.osdev.org/GDT)
- [OSDev Wiki - IDT](https://wiki.osdev.org/IDT)
- James Molloy's kernel tutorials

---

## Makefile Syntax Reference

### Variables

```makefile
KERNEL_SRCS := $(wildcard kernel/*.c)
```
- `KERNEL_SRCS` - variable name
- `:=` - immediate assignment (evaluate now, not later)
- `$(wildcard ...)` - function that finds files matching pattern
- `kernel/*.c` - all `.c` files in `kernel/` directory

```makefile
KERNEL_OBJS := $(KERNEL_SRCS:.c=.o)
```
- `$(VAR:.c=.o)` - substitution: replace `.c` with `.o` in each word
- If `KERNEL_SRCS` = `kernel/main.c kernel/terminal.c`
- Then `KERNEL_OBJS` = `kernel/main.o kernel/terminal.o`

### Pattern Rule

```makefile
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)
```
- `%.o: %.c` - pattern rule: any `.o` depends on matching `.c`
- `%` - wildcard that matches any stem (e.g., `kernel/main`)
- `$(CC)` - compiler variable (your cross-compiler)
- `-c` - compile only, don't link
- `$<` - automatic variable: first prerequisite (`%.c`)
- `-o $@` - output file; `$@` = target name (`%.o`)
- `$(CFLAGS)` - compiler flags variable

### Target Rule

```makefile
kernel: boot/boot.o $(KERNEL_OBJS)
	$(CC) -T $(LD) -o $(KERNEL) $(LDFLAGS) $^ -lgcc
```
- `kernel:` - target name
- `boot/boot.o $(KERNEL_OBJS)` - prerequisites (must exist first)
- `$^` - automatic variable: all prerequisites
- `-T $(LD)` - use linker script
- `-lgcc` - link with libgcc

### Automatic Variables Summary

| Variable | Meaning |
|----------|---------|
| `$@` | Target name |
| `$<` | First prerequisite |
| `$^` | All prerequisites |

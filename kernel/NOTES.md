# jmOS Kernel Development Notes

## Environment & Toolchain

### Cross Compiler
You need `i686-elf-gcc` — the system gcc targets Linux and will break things.
The `#if defined(__linux__)` and `#if !defined(__i386__)` guards in kernel.c catch this.

### Dependencies
```bash
sudo apt install build-essential bison flex libgmp-dev libmpc-dev \
    libmpfr-dev texinfo libisl-dev nasm xorriso grub-pc-bin grub-common mtools
```
- `grub-pc-bin` — easy to miss, but without it `grub-mkrescue` builds a broken ISO with no BIOS boot modules
- `mtools` — required by `grub-mkrescue` for FAT filesystem operations, missing it gives `mformat invocation failed`

---

## Makefile

### Core Syntax
```makefile
target: dependencies
    recipe
```
**Common mistakes:**
- Swapping target and dependency (e.g. `kernel.c: kernel.o` instead of `kernel.o: kernel.c`)
- Using `-02` (zero) instead of `-O2` (capital O) in CFLAGS
- Using `-p` instead of `-o` for output flag

### Automatic Variables
```makefile
$@  # target
$<  # first dependency only
$^  # ALL dependencies (use this for linking)
```

### Suppress Command Echo
Prefix with `@` to only show output, not the command itself (not used in this project):
```makefile
@$(CC) -c $< -o $@ $(CFLAGS)
```

### Shell if/else in Makefiles
Must use `\` continuations — Make runs each line as a separate shell:
```makefile
if grub-file --is-x86-multiboot $(KERNEL); then \
    echo multiboot confirmed; \
else \
    echo not multiboot; \
fi
```

### #define vs = in Makefile
`#define` is C preprocessor, Makefile variables use `:=`:
```makefile
OS_NAME := jmOS   # Makefile variable, no = after :=... wait, := IS the assignment
```
```c
#define VGA_WIDTH 80  // C macro, NO = sign
```

### `>` vs `>>` in shell
```makefile
echo 'line 1' > file.cfg    # create/overwrite
echo 'line 2' >> file.cfg   # append
```

### ISO Build Target
```makefile
iso: $(KERNEL)
    rm -rf $(ISODIR)                          # clean stale staging
    mkdir -p $(ISODIR)/boot/grub              # stage structure
    cp $(KERNEL) $(ISODIR)/boot/$(KERNEL)     # copy kernel
    echo 'menuentry "$(OS_NAME)" {' > $(ISODIR)/boot/grub/grub.cfg   # generate grub.cfg
    echo '    multiboot /boot/$(KERNEL)' >> $(ISODIR)/boot/grub/grub.cfg
    echo '}' >> $(ISODIR)/boot/grub/grub.cfg
    grub-mkrescue -o $(ISO) $(ISODIR)         # pack ISO
    rm -rf $(ISODIR)                          # clean temp staging
```
`grub.cfg` is generated dynamically so it always matches `$(KERNEL)` name.

---

## C Freestanding Gotchas

### What you DO have
- `<stdbool.h>`, `<stddef.h>`, `<stdint.h>` — safe to include
- All control flow: `for`, `while`, `if`, `switch`
- Structs, enums, arrays, pointers
- Inline assembly (`__asm__ volatile`)

### What you DON'T have
- `printf`, `malloc`, `free`, `memcpy`, `memmove`, `sprintf`
- Any function from `<stdio.h>`, `<stdlib.h>`, `<string.h>`
- You must implement everything yourself

### `static` keyword
```c
static size_t terminal_row;  // private to this file, no name collisions
static inline uint8_t vga_entry_color(...) // inline + file-private
```

### `#define` vs `const` for constants
```c
const int SIZE = 100;
int arr[SIZE];        // ERROR in gnu99 — not a constant expression

#define SIZE 100
int arr[SIZE];        // OK — preprocessor replaces before compile
```
Always use `#define` for array sizes in kernel code.
Always wrap `#define` expressions in parens:
```c
#define BUFFER_HEIGHT (VGA_HEIGHT + SCROLL_HISTORY)  // correct
#define BUFFER_HEIGHT VGA_HEIGHT + SCROLL_HISTORY    // dangerous, precedence bugs
```

### `size_t` vs `uintN_t`
- `size_t` — for indexes, counts, positions. Semantically means "a size"
- `uint16_t` — when exact bit width matters (VGA cells, hardware registers)
- `uint8_t` — for color attributes, byte-level hardware values

---

## VGA Text Mode

### Layout
- 80x25 grid, each cell = 2 bytes
- Physical address: `0xB8000`
- Flat array: `index = y * VGA_WIDTH + x`

### Cell Packing
```
uint16_t cell: [15..8 = color] [7..0 = char]
uint8_t color: [7..4 = bg    ] [3..0 = fg  ]
```

### Colors
16 hardwired colors (0-15), fixed by VGA standard since the 80s.

---

## Terminal Architecture

### Two Buffer Approach
```c
static uint16_t terminal_buffer[BUFFER_HEIGHT * VGA_WIDTH]; // back buffer (125 rows)
static uint16_t* vga_memory = (uint16_t*)VGA_MEMORY_ADDRESS; // hardware (25 rows)
```
Write to `terminal_buffer` first, then flush visible window to `vga_memory`.

### Ring Buffer Scrolling
```
BUFFER_HEIGHT = VGA_HEIGHT + SCROLL_HISTORY = 25 + 100 = 125 rows

terminal_row:    where next char is written (wraps at 125)
terminal_offset: which row the screen window starts at (wraps at 100)

visible window = terminal_buffer[terminal_offset .. terminal_offset + VGA_HEIGHT]
```

When `terminal_row >= terminal_offset + VGA_HEIGHT`:
- scroll: `terminal_offset++`
- wrap offset when `terminal_offset + VGA_HEIGHT >= BUFFER_HEIGHT`

### `draw_buffer`
Copies the visible window from `terminal_buffer` to `vga_memory`:
```c
buffer_index = ((y + terminal_offset) % BUFFER_HEIGHT) * VGA_WIDTH + x;
```
Modulo handles the ring buffer wrap point.

---

## Writing Numbers (no stdlib)

### The ASCII trick
```
number 0 != ASCII '0'
ASCII '0' = 48, '1' = 49 ... '9' = 57
So: '0' + 3 = 51 = ASCII '3'  ✓
```

### `terminal_writedecimal`
```c
// % 10 extracts last digit:  123 % 10 = 3
// / 10 chops last digit:     123 / 10 = 12
// digits come out reversed, so print buffer backwards with --i not i--
while (i > 0) {
    terminal_putchar(buffer[--i]);  // --i: decrement THEN read
    // NOT i--: read THEN decrement (off by one, reads garbage)
}
```

---

## QEMU

```bash
qemu-system-i386 -cdrom jmOS-bare.iso
```

```makefile
QEMU := qemu-system-i386
run: all
    $(QEMU) -cdrom $(ISO)
```

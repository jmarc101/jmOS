# jmOS

personal OS dev learning project. following the osdev wiki.

---

## whats here

```
jmOS/
├── boot/
│   └── boot.asm            # multiboot bootloader entry point
├── docs/
│   ├── ACRONYMS.md         # OS dev terminology
│   ├── BOOT.md             # bootloader notes
│   ├── CROSS_COMPILER.md   # cross-compiler setup
│   ├── GLOBAL_DESCRIPTOR_TABLE.md
│   ├── KERNEL.md           # kernel implementation notes
│   ├── LINKER.md           # linker script notes
│   └── MAKEFILE.md         # makefile syntax reference
├── include/
│   ├── string.h
│   ├── terminal.h
│   └── vga.h
├── kernel/
│   ├── cpu/                # x86 CPU state management
│   │   ├── gdt.c           # Global Descriptor Table setup
│   │   └── gdt.asm         # GDT flush (loads via LGDT)
│   ├── drivers/            # hardware drivers
│   │   └── terminal.c      # VGA terminal driver with scrolling
│   └── kernel.c            # kernel entry point
├── lib/
│   └── string.c            # string utilities (strlen)
├── linker/
│   └── linker.ld           # memory layout script
├── Makefile
├── ROAD_MAP.md
└── LICENSE
```

the kernel initializes a VGA terminal driver with scrolling support and outputs test lines to demonstrate the scrolling functionality.

---

## prerequisites

**system packages** (debian/ubuntu):

```bash
sudo apt install build-essential bison flex libgmp-dev libmpc-dev \
    libmpfr-dev texinfo libisl-dev nasm xorriso grub-pc-bin grub-common mtools \
    qemu-system-x86 wget curl
```

| Package | Purpose |
|---|---|
| `build-essential` | GCC, make, and core build tools |
| `bison` | Parser generator required to build GCC from source |
| `flex` | Lexer generator required to build GCC from source |
| `libgmp-dev` | GNU math library, GCC build dependency |
| `libmpc-dev` | Complex number library, GCC build dependency |
| `libmpfr-dev` | Floating point library, GCC build dependency |
| `libisl-dev` | Integer set library, GCC build dependency |
| `texinfo` | Required to build GCC documentation |
| `nasm` | Assembler for x86 assembly source files |
| `xorriso` | ISO image creation, used by `grub-mkrescue` |
| `grub-pc-bin` | GRUB x86 BIOS bootloader binaries, required for bootable ISO |
| `grub-common` | GRUB shared utilities including `grub-mkrescue` and `grub-file` |
| `mtools` | FAT filesystem tools, required by `grub-mkrescue` |
| `qemu-system-x86` | x86 emulator for running the kernel with `make run` |

**cross-compiler**: you need `i686-elf-gcc`. see [docs/CROSS_COMPILER.md](docs/CROSS_COMPILER.md) to build it.

verify its working:

```bash
i686-elf-gcc --version
```

---

## Build System

Build the OS with:
```bash
make all
```

Alternatively you can simply build and run with:
```bash
make run
```

### Targets

| Target | Purpose |
|---|---|
| `make all` | Builds kernel and ISO image |
| `make kernel` | Compiles and links the kernel binary |
| `make iso` | Packages kernel into a bootable ISO using GRUB |
| `make run` | Launches the ISO in QEMU |
| `make clean` | Removes all build artifacts |

### Variables

| Variable | Default | Purpose |
|---|---|---|
| `OS_NAME` | `jmOS` | Name of the OS, used in output filenames and GRUB menu |
| `OS_TARGET` | `bare` | Current build target, appended to kernel name e.g. `jmOS-bare` |
| `CC` | `i686-elf-gcc` | Cross compiler for 32-bit x86 |
| `AS` | `nasm` | Assembler for boot code |
| `LD` | `linker/linker.ld` | Linker script, controls kernel memory layout |

### Output Files

| File | Purpose |
|---|---|
| `jmOS-bare` | Kernel ELF binary |
| `jmOS-0.1.iso` | Bootable CD-ROM image |

---

## docs/

notes and lessons learned along the way:

| File | Topic |
|------|-------|
| [BOOT.md](docs/BOOT.md) | how grub loads the kernel, multiboot header, protected mode |
| [KERNEL.md](docs/KERNEL.md) | kernel development notes, cross-compiler, VGA driver |
| [LINKER.md](docs/LINKER.md) | what the linker does, sections, memory layout |
| [MAKEFILE.md](docs/MAKEFILE.md) | makefile syntax reference |
| [CROSS_COMPILER.md](docs/CROSS_COMPILER.md) | why you need a cross-compiler, how to build i686-elf-gcc |

---

## coming back?

lost after a break? start here:

- **what does boot.asm do?** read [docs/BOOT.md](docs/BOOT.md)
- **what does kernel.c do?** VGA text mode driver, writes directly to video memory. see [docs/KERNEL.md](docs/KERNEL.md)
- **cross-compiler broken?** check your `$PATH`, see [docs/CROSS_COMPILER.md](docs/CROSS_COMPILER.md)
- **what's the roadmap?** see [ROAD_MAP.md](ROAD_MAP.md)
- **osdev wiki:** https://wiki.osdev.org/Main_Page

---

## references

- https://wiki.osdev.org/Main_Page
- https://wiki.osdev.org/GCC_Cross-Compiler

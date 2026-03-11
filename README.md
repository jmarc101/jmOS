# jmOS

personal OS dev learning project. following the osdev wiki.

---

## whats here

```
jmOS/
├── boot/
│   ├── boot.asm      # bootloader entry point
│   └── NOTES.md      # notes on how boot.asm works
├── kernel/
│   └── kernel.c      # bare metal C kernel with VGA text mode driver
├── Makefile
└── CROSS_COMPILER.md # how to build the cross-compiler
```

the kernel writes "Hello, kernel World!" to the screen using direct VGA memory writes at 0xB8000.

---

## prerequisites

**system packages** (debian/ubuntu):

```bash
sudo apt install build-essential bison flex libgmp-dev libmpc-dev \
    libmpfr-dev texinfo libisl-dev nasm xorriso grub-pc-bin grub-common mtools \
    wget curl 
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

**cross-compiler**: you need `i686-elf-gcc`. see [CROSS_COMPILER.md](CROSS_COMPILER.md) to build it.

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

## coming back?

lost after a break? start here:

- **what does boot.asm do?** read `boot/NOTES.md`
- **what does kernel.c do?** VGA text mode driver, writes directly to video memory
- **cross-compiler broken?** check your `$PATH`, see `CROSS_COMPILER.md`
- **osdev wiki:** https://wiki.osdev.org/Main_Page

---

## references

- https://wiki.osdev.org/Main_Page
- https://wiki.osdev.org/GCC_Cross-Compiler

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
    libmpfr-dev texinfo libisl-dev nasm xorriso grub-pc-bin
```

**cross-compiler**: you need `i686-elf-gcc`. see [CROSS_COMPILER.md](CROSS_COMPILER.md) to build it.

verify its working:

```bash
i686-elf-gcc --version
```

---

## build

```bash
make boot    # assemble bootloader
make clean   # clean up
```

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

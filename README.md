# jmOS toy Operating System

> **Purpose:** This repo is a personal learning reference for bare-metal OS development. If you're coming back after a while and feel lost, start here.

> **Primary reference:** This project follows the OSDev Wiki — https://wiki.osdev.org/Main_Page. The cross-compiler setup specifically follows https://wiki.osdev.org/GCC_Cross-Compiler.

---

## Table of Contents

1. [Project Structure](#project-structure)
2. [Toolchain](#toolchain)
   - [System Packages](#1-install-system-packages)
   - [Why We Build a Cross-Compiler](#2-why-we-build-a-cross-compiler)
   - [Downloading GCC & Binutils](#3-downloading-gcc--binutils)
   - [Build & Install the Cross-Compiler](#4-build--install-the-cross-compiler)
   - [Adding the Toolchain to PATH](#5-adding-the-toolchain-to-path)

---

## Project Structure

### What lives in the repo

```
your-repo/
├── src/          # All source code lives here (.c, .asm, linker scripts)
│   ├── boot/     # Bootloader / entry point (e.g. boot.asm)
│   ├── kernel/   # Kernel C source files
│   └── linker.ld # Linker script
│
├── build/        # Compiled object files — add to .gitignore, do not commit
│   └── ...       # (regenerated every build, safe to delete)
│
└── Makefile      # Build rules
```

### What lives outside the repo (on your machine only)

The OSDev wiki convention is to use `$HOME/src` as the working area for downloading and building the toolchain. None of this goes in the repo.

```
$HOME/src/
├── binutils-2.41/        # Extracted binutils source
├── gcc-13.2.0/           # Extracted GCC source
├── binutils-build/       # Binutils out-of-tree build directory
└── gcc-build/            # GCC out-of-tree build directory

/opt/cross/               # The finished, installed cross-compiler
├── bin/                  # i686-elf-gcc, i686-elf-ld, etc.
├── lib/
└── ...
```

> The toolchain is a one-time setup on your machine. It doesn't belong in the repo — it's too large, and anyone cloning the repo needs to build it themselves for their own host anyway. The `build/` directory is also git-ignored since it's just compiled output that gets regenerated.

---

## Toolchain

OS development requires a **cross-compiler** — a version of GCC that runs on your machine (the *host*) but produces code for a completely bare target with no OS assumptions. The host system's GCC won't do; it's been compiled to assume Linux exists underneath it.

### 1. Install System Packages

These are the host tools needed before building anything:

```bash
sudo apt update
sudo apt install -y \
    build-essential \   # gcc, g++, make — the baseline C toolchain for YOUR machine
    bison \             # parser generator, required by GCC's build system
    flex \              # lexer generator, also required by GCC
    libgmp-dev \        # arbitrary precision math — GCC uses this internally
    libmpc-dev \        # complex number library — GCC dependency
    libmpfr-dev \       # multiple precision floats — GCC dependency
    texinfo \           # required to build Binutils documentation
    libisl-dev \        # integer set library — used by GCC's loop optimiser (min version 0.15)
    nasm \              # assembler for x86 (for writing boot code in asm)
    xorriso \           # creates bootable ISO images
    grub-pc-bin         # GRUB bootloader binaries for generating a bootable image
```

> **`build-essential`** is a meta-package. It installs `gcc`, `g++`, `make`, and the standard C headers on your host. You need these to *compile GCC itself*, not to compile your OS.

---

### 2. Why We Build a Cross-Compiler

Your distro's `gcc` is configured to produce ELF binaries that link against glibc and assume Linux system calls exist. When your kernel runs, there is no Linux, no glibc, no syscalls — just bare hardware. Linking against host assumptions will silently corrupt your kernel.

The solution is a **freestanding cross-compiler** targeted at `i686-elf`. "ELF" here just means the output binary format — there's no OS tied to it.

> **Rule of thumb:** Never use `/usr/bin/gcc` to compile kernel code. Always use your cross-compiler (e.g. `i686-elf-gcc`).

---

### 3. Downloading GCC & Binutils

You need two source packages:

- **Binutils** — provides the linker (`ld`), assembler (`as`), and binary utilities for your target
- **GCC** — the compiler itself

Find the latest stable releases at:
- https://ftp.gnu.org/gnu/binutils/
- https://ftp.gnu.org/gnu/gcc/

> **Note:** Not all combinations of GCC and Binutils work together. Use a Binutils release that was released at roughly the same time as your GCC version. Check https://wiki.osdev.org/Cross-Compiler_Successful_Builds for known working pairs.

```bash
# Create the working directory (OSDev wiki convention)
mkdir -p $HOME/src
cd $HOME/src

# Example versions — substitute your chosen versions
export BINUTILS_VERSION=2.41
export GCC_VERSION=13.2.0

# Download
wget https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.xz
wget https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.xz

# Extract
tar -xf binutils-${BINUTILS_VERSION}.tar.xz
tar -xf gcc-${GCC_VERSION}.tar.xz
```

---

### 4. Build & Install the Cross-Compiler

Set up your environment variables first. These are used throughout the build process:

```bash
# Where the finished cross-compiler will be installed
export PREFIX="/opt/cross"

# The target triple: architecture-vendor-ABI
# i686-elf = 32-bit x86, no vendor, freestanding ELF output
export TARGET=i686-elf

# Make the cross-compiler binaries findable during the GCC build itself
export PATH="$PREFIX/bin:$PATH"
```

> **What is the target triple?**
> It's a string in the format `arch-vendor-system`. Using `i686-elf` tells GCC to:
> - Target 32-bit x86 (`i686`)
> - Have no specific vendor (`none` is also common)
> - Produce raw ELF output with no OS ABI (`elf`)

#### 4a. Build Binutils

```bash
cd $HOME/src
mkdir binutils-build && cd binutils-build

../binutils-${BINUTILS_VERSION}/configure \
    --target=$TARGET \       # produce tools that emit code for our target
    --prefix="$PREFIX" \     # install into /opt/cross, not /usr/local
    --with-sysroot \         # enable sysroot support (keeps host headers out)
    --disable-nls \          # no native language support — keeps things simple
    --disable-werror         # don't treat warnings as errors (avoids build failures on newer hosts)

make
make install
```

#### 4b. Build GCC

```bash
cd $HOME/src
mkdir gcc-build && cd gcc-build

../gcc-${GCC_VERSION}/configure \
    --target=$TARGET \          # same target as binutils
    --prefix="$PREFIX" \        # same install location
    --disable-nls \             # no native language support
    --enable-languages=c,c++ \  # only build C and C++ — skip Fortran, Ada, etc.
    --without-headers           # critical: don't use any host OS headers; we have none

make all-gcc               # build only the compiler (not libgcc yet)
make all-target-libgcc     # build the minimal runtime library for our target
make install-gcc           # install the compiler
make install-target-libgcc # install libgcc
```

> **Why `--without-headers`?**
> Normally GCC uses system headers to build support libraries. Since our target has no OS and no headers yet, this flag tells GCC to proceed without them.

> **Why build `all-gcc` and `all-target-libgcc` separately instead of just `make all`?**
> A full `make all` would also try to build `libstdc++`, which requires OS headers and a working libc. We don't have those. Building in stages avoids that dependency trap.

**Flag reference table:**

| Flag | What it does |
|---|---|
| `--target` | Sets the architecture/ABI the compiler outputs code for |
| `--prefix` | Sets the installation root directory |
| `--with-sysroot` | Allows a custom sysroot, prevents host headers leaking in |
| `--disable-nls` | Skips internationalization — not needed, reduces build time |
| `--disable-werror` | Warnings don't abort the build (newer GCC/host combos often have minor warnings) |
| `--enable-languages=c,c++` | Only compile the C and C++ frontends |
| `--without-headers` | Build GCC without any target system headers |

---

### 5. Adding the Toolchain to PATH

After building, your cross-compiler binaries are in `/opt/cross/bin/`. Add them to your shell's PATH so you can call them directly:

```bash
# Add this to your ~/.bashrc or ~/.zshrc to make it permanent
export PATH="/opt/cross/bin:$PATH"
```

Reload your shell, then verify:

```bash
i686-elf-gcc --version
# Should print something like: i686-elf-gcc (GCC) 13.2.0

i686-elf-ld --version
# Should print: GNU ld (GNU Binutils) 2.41
```

If both commands print version info, your cross-compiler is ready to use.

---

## Quick Reference

| Variable | Example value | Purpose |
|---|---|---|
| `$PREFIX` | `/opt/cross` | Where the cross-compiler is installed |
| `$TARGET` | `i686-elf` | Target triple for the cross-compiler |
| `$PATH` | prepend `$PREFIX/bin` | Makes cross-compiler binaries callable by name |

---

*Come back confused? Read [Why We Build a Cross-Compiler](#2-why-we-build-a-cross-compiler) first, then check your `$PATH` and `$PREFIX`.*

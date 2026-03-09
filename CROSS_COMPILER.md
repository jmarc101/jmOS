# cross-compiler setup

os dev needs a cross-compiler. your distro's gcc assumes linux exists underneath - links against glibc, expects syscalls. when your kernel runs theres no linux, no libc, nothing. using host gcc will silently corrupt your kernel.

the solution is a freestanding cross-compiler targeting `i686-elf`. produces raw ELF binaries with no OS assumptions.

> **rule of thumb:** never use `/usr/bin/gcc` for kernel code. always use `i686-elf-gcc`.

this guide follows https://wiki.osdev.org/GCC_Cross-Compiler

---

## 1. system packages

install these first (debian/ubuntu):

```bash
sudo apt update
sudo apt install -y \
    build-essential \   # gcc, g++, make for YOUR machine
    bison \             # parser generator for GCC build
    flex \              # lexer generator for GCC build
    libgmp-dev \        # arbitrary precision math
    libmpc-dev \        # complex number library
    libmpfr-dev \       # multiple precision floats
    texinfo \           # for binutils docs
    libisl-dev \        # loop optimizer (min 0.15)
    nasm \              # x86 assembler
    xorriso \           # creates bootable ISOs
    grub-pc-bin         # GRUB bootloader binaries
```

---

## 2. download gcc & binutils

get both source packages:
- https://ftp.gnu.org/gnu/binutils/
- https://ftp.gnu.org/gnu/gcc/

> not all versions work together. check https://wiki.osdev.org/Cross-Compiler_Successful_Builds for known working pairs.

```bash
mkdir -p $HOME/src
cd $HOME/src

export BINUTILS_VERSION=2.41
export GCC_VERSION=13.2.0

wget https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.xz
wget https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.xz

tar -xf binutils-${BINUTILS_VERSION}.tar.xz
tar -xf gcc-${GCC_VERSION}.tar.xz
```

---

## 3. build & install

set up environment first:

```bash
export PREFIX="/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"
```

> **target triple:** `i686-elf` = 32-bit x86, no vendor, freestanding ELF output

### 3a. build binutils

```bash
cd $HOME/src
mkdir binutils-build && cd binutils-build

../binutils-${BINUTILS_VERSION}/configure \
    --target=$TARGET \       # emit code for our target
    --prefix="$PREFIX" \     # install into /opt/cross
    --with-sysroot \         # keep host headers out
    --disable-nls \          # skip internationalization
    --disable-werror         # don't treat warnings as errors

make
make install
```

### 3b. build gcc

```bash
cd $HOME/src
mkdir gcc-build && cd gcc-build

../gcc-${GCC_VERSION}/configure \
    --target=$TARGET \
    --prefix="$PREFIX" \
    --disable-nls \
    --enable-languages=c,c++ \
    --without-headers           # no target OS headers exist yet

make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc
```

> **why staged builds?** a full `make all` tries to build libstdc++ which needs libc. we don't have that.

---

## 4. add to PATH

```bash
# add to ~/.bashrc or ~/.zshrc
export PATH="/opt/cross/bin:$PATH"
```

verify:

```bash
i686-elf-gcc --version
# i686-elf-gcc (GCC) 13.2.0

i686-elf-ld --version
# GNU ld (GNU Binutils) 2.41
```

---

## directory layout

the toolchain lives outside the repo:

```
$HOME/src/
├── binutils-2.41/        # extracted source
├── gcc-13.2.0/           # extracted source
├── binutils-build/       # build directory
└── gcc-build/            # build directory

/opt/cross/               # installed cross-compiler
├── bin/                  # i686-elf-gcc, i686-elf-ld, etc
├── lib/
└── ...
```

---

## flag reference

| flag | what it does |
|---|---|
| `--target` | architecture/ABI the compiler outputs for |
| `--prefix` | installation root |
| `--with-sysroot` | custom sysroot, prevents host header leaks |
| `--disable-nls` | skip internationalization |
| `--disable-werror` | warnings don't abort build |
| `--enable-languages=c,c++` | only build C and C++ frontends |
| `--without-headers` | build without target system headers |

---

## quick reference

| variable | value | purpose |
|---|---|---|
| `$PREFIX` | `/opt/cross` | where cross-compiler is installed |
| `$TARGET` | `i686-elf` | target triple |
| `$PATH` | prepend `$PREFIX/bin` | makes tools callable |

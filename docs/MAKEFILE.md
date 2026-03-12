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

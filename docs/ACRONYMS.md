# OS Dev Acronym Reference

## CPU Modes

| Acronym | Full Name | What it actually is |
|---|---|---|
| RM | Real Mode | CPU power-on state, no memory protection, 16-bit, where BIOS runs |
| PM | Protected Mode | 32-bit mode with memory protection and rings, where your kernel runs |
| LM | Long Mode | 64-bit mode, extension of protected mode |
| SMM | System Management Mode | Special CPU mode for firmware, you will never touch this |

## Descriptor Tables

| Acronym | Full Name | What it actually is |
|---|---|---|
| GDT | Global Descriptor Table | Array of segment permission entries in RAM, applies to all processes |
| LDT | Local Descriptor Table | Per-process segment table, obsolete, ignore |
| IDT | Interrupt Descriptor Table | Array of interrupt handler entries, same idea as GDT but for interrupts |
| TSS | Task State Segment | Special entry that holds the kernel stack address for ring switches |
| DPL | Descriptor Privilege Level | The ring (0 or 3) required to use a given entry |
| CPL | Current Privilege Level | The ring the CPU is currently in, derived from CS bits 0-1 |
| RPL | Requested Privilege Level | The ring embedded in a selector value (bits 0-1) |

## Registers — General Purpose

| Acronym | Full Name | What it actually is |
|---|---|---|
| EAX | Extended Accumulator | General purpose, also holds syscall return values |
| EBX | Extended Base | General purpose |
| ECX | Extended Counter | General purpose, also used for loop counters |
| EDX | Extended Data | General purpose, also used in division and I/O |
| ESI | Extended Source Index | General purpose, used in string/memory operations as source |
| EDI | Extended Destination Index | General purpose, used in string/memory operations as destination |
| EBP | Extended Base Pointer | Points to the base of the current stack frame |
| ESP | Extended Stack Pointer | Points to the top of the current stack |

> In 64-bit (long mode) these are prefixed with R instead of E: RAX, RBX, RCX, etc. and are 64-bit wide.

## Registers — Special Purpose

| Acronym | Full Name | What it actually is |
|---|---|---|
| EIP | Extended Instruction Pointer | Address of the next instruction to execute |
| EFLAGS | Extended Flags | CPU status bits: interrupts enabled, zero flag, overflow, etc |
| CR0 | Control Register 0 | Bit flags that control CPU features, including enabling protected mode and paging |
| CR2 | Control Register 2 | Holds the address that caused the last page fault |
| CR3 | Control Register 3 | Points to the current page directory, used for paging |
| CR4 | Control Register 4 | More CPU feature flags (PAE, SSE, etc) |
| MSR | Model Specific Register | Special registers for advanced CPU features like SYSCALL/SYSRET |
| GDTR | GDT Register | Holds the address and size of the GDT, loaded via LGDT |
| IDTR | IDT Register | Holds the address and size of the IDT, loaded via LIDT |
| LDTR | LDT Register | Holds the selector for the current LDT, ignore |
| TR | Task Register | Holds the selector for the current TSS, loaded via LTR |

## Segment Registers

| Acronym | Full Name | What it actually is |
|---|---|---|
| CS | Code Segment | Which GDT entry covers the currently running code |
| DS | Data Segment | Which GDT entry covers data reads/writes |
| SS | Stack Segment | Which GDT entry covers the stack |
| ES | Extra Segment | General purpose extra segment, mostly unused in flat mode |
| FS | F Segment | General purpose, used by some OSes for thread-local storage |
| GS | G Segment | General purpose, used by some OSes for per-CPU data |

## Assembly Instructions You'll Actually Use

| Acronym | Full Name | What it does |
|---|---|---|
| LGDT | Load GDT | Points the CPU at your GDT array |
| LIDT | Load IDT | Points the CPU at your IDT array |
| LTR | Load Task Register | Loads the TSS selector, required before ring 3 |
| STI | Set Interrupt Flag | Enables hardware interrupts |
| CLI | Clear Interrupt Flag | Disables hardware interrupts |
| HLT | Halt | Stops the CPU until next interrupt, ring 0 only |
| IRET | Interrupt Return | Returns from an interrupt handler, also how you drop to ring 3 |
| PUSHA | Push All | Pushes all general purpose registers onto the stack |
| POPA | Pop All | Restores all general purpose registers from the stack |
| PUSHF | Push Flags | Pushes EFLAGS onto the stack |
| POPF | Pop Flags | Restores EFLAGS from the stack |
| IN | Input | Reads a byte from a hardware I/O port |
| OUT | Output | Writes a byte to a hardware I/O port |
| SYSCALL | System Call | Fast way to enter ring 0 from ring 3 (64-bit) |
| SYSRET | System Return | Fast way to return to ring 3 from ring 0 (64-bit) |
| INVLPG | Invalidate Page | Flushes one page from the TLB cache |

## Memory Management

| Acronym | Full Name | What it actually is |
|---|---|---|
| MMU | Memory Management Unit | CPU hardware that translates virtual addresses to physical ones |
| VA | Virtual Address | The address your code sees, may not match physical RAM |
| PA | Physical Address | The actual address in RAM |
| PD | Page Directory | Top-level table for address translation (32-bit) |
| PT | Page Table | Second-level table for address translation (32-bit) |
| PDE | Page Directory Entry | One entry in the page directory |
| PTE | Page Table Entry | One entry in a page table |
| PML4 | Page Map Level 4 | Top-level table for address translation (64-bit) |
| TLB | Translation Lookaside Buffer | CPU cache for recently translated addresses, flushed on CR3 write |
| PAE | Physical Address Extension | 32-bit mode feature to address more than 4GB of RAM |
| PSE | Page Size Extension | Allows 4MB pages instead of 4KB |

## Interrupts and Hardware

| Acronym | Full Name | What it actually is |
|---|---|---|
| IRQ | Interrupt Request | A signal from hardware asking the CPU's attention (keyboard, timer, etc) |
| ISR | Interrupt Service Routine | The handler function the CPU jumps to when an interrupt fires |
| IST | Interrupt Stack Table | A set of known-good stacks to use for critical interrupts (64-bit) |
| PIC | Programmable Interrupt Controller | Old chip (8259) that manages IRQs, you remap and talk to it via I/O ports |
| APIC | Advanced PIC | Modern replacement for the PIC, one per CPU core |
| LAPIC | Local APIC | The APIC built into each CPU core |
| IOAPIC | I/O APIC | Routes external IRQs to the right CPU core |
| PIT | Programmable Interval Timer | Chip that fires IRQ0 on a regular interval, your first timer |
| HPET | High Precision Event Timer | More accurate modern timer, eventually replaces PIT |
| RTC | Real Time Clock | Battery-backed clock chip, tells you the current date and time |

## Boot and Firmware

| Acronym | Full Name | What it actually is |
|---|---|---|
| BIOS | Basic Input/Output System | Old firmware standard, inits hardware and loads bootloader |
| UEFI | Unified Extensible Firmware Interface | Modern replacement for BIOS |
| MBR | Master Boot Record | First 512 bytes of a disk, contains the initial bootloader |
| GRUB | Grand Unified Bootloader | Bootloader that loads your kernel and hands off in protected mode |
| POST | Power On Self Test | Hardware check the BIOS runs on startup before loading anything |

## File Systems and Storage

| Acronym | Full Name | What it actually is |
|---|---|---|
| VFS | Virtual File System | Abstraction layer that lets the kernel support multiple filesystems |
| ATA | Advanced Technology Attachment | Old disk interface (IDE), you talk to it via I/O ports |
| SATA | Serial ATA | Modern disk interface |
| DMA | Direct Memory Access | Hardware feature that lets devices write to RAM without involving the CPU |

## Misc

| Acronym | Full Name | What it actually is |
|---|---|---|
| GPF | General Protection Fault | CPU exception when code does something illegal (wrong ring, bad selector, etc) |
| PF | Page Fault | CPU exception when code accesses an unmapped or protected virtual address |
| NMI | Non-Maskable Interrupt | Interrupt that fires even with CLI, used for hardware errors |
| ABI | Application Binary Interface | Calling convention: how arguments are passed, which registers to save |
| SMP | Symmetric Multiprocessing | Running the kernel on multiple CPU cores |
| GDT | see above | — |

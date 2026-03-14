# GDT + Ring Switching

Wiki Reference: https://wiki.osdev.org/GDT_Tutorial

## Why any of this exists

The CPU in protected mode doesn't let you just run code at any address or touch
any memory. Every memory access goes through a segment register, and each
segment register points at an entry in the descriptor table. That entry tells
the CPU: what memory, how big, and who's allowed.

In practice for a modern kernel you don't actually care about segmentation.
You set everything to cover all 4GB and use paging for real memory enforcement.
But you still have to set the table up or the CPU won't work correctly.

---

## The table itself

Just an array of 8-byte entries sitting in regular RAM. You pick where it lives.
You tell the CPU where it is by running the LGDT instruction (privileged,
ring 0 only) with a small 6-byte structure:

```
2 bytes: size of the table - 1
4 bytes: address of the table in memory
```

### The entries

Each entry is 8 bytes. The layout is ugly because of 286 legacy. The base
address and limit are split across non-contiguous bytes. Don't worry about
memorizing the layout, that's what the encode function is for.

What actually matters per entry:

| field | what it is |
|---|---|
| base | where the segment starts in memory |
| limit | how big the segment is |
| access byte | who can use it and what kind (code vs data) |
| flags | granularity + 16/32/64 bit mode |

For flat mode (what you want): base=0, limit=0xFFFFF, flags=0xC on
everything. The flags granularity bit (G=1) turns that limit into 4GB.
The only thing that varies between entries is the access byte.

### The access byte

```
[ P | DPL DPL | S | type type type type ]
  7    6   5    4    3    2    1    0
```

- P: always 1
- DPL: the ring level (0 for kernel, 3 for user)
- S: 1 for code/data, 0 for system stuff
- type bit 3: 0 means data segment, 1 means code segment
- type bit 1: readable (code) or writable (data), set to 1

So for flat kernel mode:
```
kernel code: 1 00 1 1010 = 0x9A
kernel data: 1 00 1 0010 = 0x92
user code:   1 11 1 1010 = 0xFA
user data:   1 11 1 0010 = 0xF2
```

### Minimum viable table

```
offset 0x00  null entry         (required, all zeros)
offset 0x08  kernel code        access=0x9A flags=0xC
offset 0x10  kernel data        access=0x92 flags=0xC
offset 0x18  user code          access=0xFA flags=0xC
offset 0x20  user data          access=0xF2 flags=0xC
```

---

## The segment registers

The CPU has several segment registers. Each one holds a selector, which is just a
byte offset into the table (0x08, 0x10, etc). The CPU uses the right one
automatically depending on what it's doing:

| register | used for | constant value (flat mode) |
|---|---|---|
| CS | fetching every instruction | 0x08 (kernel) / 0x1B (user) |
| DS | reading/writing memory | 0x10 (kernel) / 0x23 (user) |
| SS | push/pop | 0x10 (kernel) / 0x23 (user) |
| ES/FS/GS | mostly ignore for now | same as data |

When you load a selector into a segment register, the CPU doesn't just store
the number. It goes to the table, reads the full 8-byte entry, validates it,
and caches the result internally in a hidden register. After that it uses the
cache, not the table. This means:
- Loading the table does nothing until you reload the segment registers
- If you modify a table entry later, you have to reload the register to pick it up

### Why you can't MOV into CS

`MOV CS, AX` is not a valid x86 instruction, it literally doesn't exist.
The reason is that changing CS means changing where the CPU fetches instructions
from. That has to happen simultaneously with changing EIP, atomically. A far
jump does both at once. A MOV can only change one thing.

The ring check still happens on far jumps. Ring 3 code can't far jump to a
ring 0 CS selector, the CPU will fault.

---

## Rings

### Instructions don't have rings

This is the key thing to get right. MOV, ADD, JMP look exactly the same in
ring 0 and ring 3. What has a ring is the table entry via the DPL field.

What IS ring-restricted is certain privileged instructions like LGDT, CLI,
and HLT. If ring 3 code tries to execute those, the CPU checks the current
ring and faults. But that's separate from the segment ring check.

### How the CPU knows what ring you're in

It looks at the bottom 2 bits of CS. Always. It's not a separate register,
it's literally bits 0-1 of whatever selector is loaded in CS:

```
0x08 → bits 0-1 = 00 → ring 0
0x1B → bits 0-1 = 11 → ring 3
```

That's why the user selectors are 0x1B and 0x23. It's the table entry offset
(0x18, 0x20) with the ring bits OR'd in (| 0x3).

### Ring check

When you load a selector into a segment register, the CPU reads the DPL out
of that table entry and compares it to the current ring (bits 0-1 of CS).
If your current ring doesn't have permission it faults.

---

## The boot flow

```
CPU resets
  → hardcoded state (Intel spec guarantees this, not all zeros)
  → jumps to BIOS at 0xFFFF0
BIOS inits hardware
  → loads bootloader
GRUB switches to protected mode
  → sets up ring 0 CS selector
  → loads your kernel
  → jumps to your entry point
You are now in ring 0. Guaranteed.
```

By the time your first line of C runs you are in ring 0. Always. This is
guaranteed by the spec and GRUB follows it.

---

## Switching to ring 3

You can't just set CS to a ring 3 selector because that would be a privilege
escalation and the CPU blocks it. Instead you abuse IRET.

IRET is normally for returning from interrupts. It pops 5 values off the
stack and loads them into registers atomically. The kernel fakes a "return
from interrupt that never happened" to drop into ring 3.

Stack frame IRET expects (bottom to top):
```
SS selector   → ring 3 data selector (0x23)
ESP           → where the ring 3 stack lives
EFLAGS        → just push current flags, mostly don't care
CS selector   → ring 3 code selector (0x1B)
EIP           → address of first user instruction
```

The CPU pops all five, loads them, and you're in ring 3 at your entry point.

After this:
- CS = 0x1B so the CPU knows it's ring 3
- DS / SS = 0x23 so ring 3 data access works
- EIP points at your user code entry point
- ESP points at wherever you allocated the user stack

CS and DS are constant per ring. Only EIP (where to execute) and ESP (which
stack to use) change between calls. You set up the selectors once in the table
and never touch them again.

---

## Syscall flow (ring 3 to ring 0 and back to ring 3)

```
user code executes INT 0x80 or SYSCALL
  → CPU automatically switches to ring 0
  → saves user EIP + ESP
  → calls kernel interrupt handler
kernel does the work (reads file, etc)
  → puts result somewhere user can read
  → executes IRET
CPU restores saved EIP + ESP
  → back in ring 3 exactly where user left off
```

The kernel never thinks about CS / DS selectors during this because those are
fixed. It only saves and restores EIP and ESP so it can return cleanly.

---


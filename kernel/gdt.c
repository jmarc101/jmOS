#include <stdint.h>

// Access byte layout: [ P | DPL DPL | S | E | DC | RW | A ]
//                       7    6   5    4   3    2    1    0
// P=1  present (valid segment, always 1)
// DPL  ring level (00=kernel, 11=user)
// S=1  code/data segment (not a system segment)
// E    executable (1=code, 0=data)
// DC   for code: conforming=0 (only callable from same ring)
//      for data: direction=0 (grows up)
// RW   for code: readable=1 (can read, never writable)
//      for data: writable=1 (can read and write)
// A=0  accessed, CPU sets this when segment is used
//      leave 0 unless your GDT is in read-only memory
// 0x9A = 1 00 1 1 0 1 0 — kernel code (ring 0, executable, readable)
#define KERNEL_CS_ACCESS 0x9A
// 0x92 = 1 00 1 0 0 1 0 — kernel data (ring 0, writable)
#define KERNEL_DS_ACCESS 0x92
// 0xFA = 1 11 1 1 0 1 0 — user code (ring 3, executable, readable)
#define USER_CS_ACCESS   0xFA
// 0xF2 = 1 11 1 0 0 1 0 — user data (ring 3, writable)
#define USER_DS_ACCESS   0xF2


// Flags layout: [ G | DB | L  | Reserved ]
//                 3    2   1       0
// G=1  granularity: limit is in 4KB blocks, so 0xFFFFF * 4KB = 4GB
// DB=1 size: 32-bit proteected mode segment
// L=0  long mode: 0 bevcause we are 32-bit
#define SEG_FLAGS 0xC // 1100

// GDT entry — 8 bytes, 32-bit protected mode
// ref: https://wiki.osdev.org/Global_Descriptor_Table#Segment_Descriptor
//
// byte: [0      ][1        ][2     ][3      ][4      ][5     ][6          ][7      ]
//       [limit lo][limit hi][base 0][base 1 ][base 2 ][access][flags|limit][base 3 ]
//
// base and limit are split across non-contiguous bytes — 286 legacy.
// flat 32-bit: base=0, limit=0xFFFFF, flags=0xC, access varies per entry.
//
// Why does limit have smaller representation than base ???
// limit is 20 bits: covers 4GB via G flag (2^20 * 4KB = 4GB), no full address needed
// base is 32 bits: raw memory address, must cover all of 32-bit space (0 to 4GB)
void set_gdt_entry(uint8_t* entry, uint8_t access)
{
  // base: don't need to set, but do for educational purposes
  entry[2] = 0x00;
  entry[3] = 0x00;
  entry[4] = 0x00;
  entry[7] = 0x00;

  // access byte: [5]
  entry[5] = access;

  // limit: 0xFFFFF split across bytes 0, 1, and low nibble of byte 6
  // byte 6 is shared: high nibble = flags, low nibble = limit bits 19-16
  entry[0] = 0xFF;              // limit bits 0-7
  entry[1] = 0xFF;              // limit bits 8-15
  entry[6] = SEG_FLAGS << 4;    // 0xC << 4 = 0xC0 = 1100 0000 (flags in high nibble)
  entry[6] |= 0x0F;             // 1100 0000 | 0000 1111 = 1100 1111 = 0xCF (limit bits 16-19 in low nibble)
}

#define GDT_ENTRY_SIZE 8

// struct used to comunicate with register
// limit is size of the table - 1
// base is memory address of that table
//
// __attribute__((packed)) prevents compiler from adding padding between fields.
// without it, base would be aligned to a 4-byte boundary, making the struct
// 8 bytes instead of the 6 bytes LGDT expects.
struct GDTDescriptor {
  uint16_t limit;    // 2bytes
  uint32_t base;     // 4bytes
} __attribute__((packed));


void gdt_init()
{
  // 0:    Null
  // 1,2:  Kernel
  // 3,4:  User
  //
  //entry 0: null is handled by static zero initialize
  static uint8_t entries[5 * GDT_ENTRY_SIZE];

  set_gdt_entry(&entries[1*GDT_ENTRY_SIZE], KERNEL_CS_ACCESS);
  set_gdt_entry(&entries[2*GDT_ENTRY_SIZE], KERNEL_DS_ACCESS);
  set_gdt_entry(&entries[3*GDT_ENTRY_SIZE], USER_CS_ACCESS);
  set_gdt_entry(&entries[4*GDT_ENTRY_SIZE], USER_DS_ACCESS);

  struct GDTDescriptor gdtr;
  gdtr.limit = (sizeof(entries) - 1);
  // Array name => pointer, we jsut cast it.
  gdtr.base = (uint32_t)entries;
}


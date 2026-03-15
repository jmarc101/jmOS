#include <stdint.h>

#define KERNEL_CS_ACCESS_BYTE 0x9A;
#define KERNEL_DS_ACCESS_BYTE 0x92;
#define USER_DS_ACCESS_BYTE 0xF2;
#define USER_CS_ACCESS_BYTE 0xFA;

struct GDTDescriptor {
  uint16_t size;    // 2bytes
  uint32_t offsetl; // 4bytes
};

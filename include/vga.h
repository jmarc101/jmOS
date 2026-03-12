#pragma once

#include <stdbool.h> // bool, true, false
#include <stddef.h>  // size_t, null
#include <stdint.h>  // unitx_t

/*
Hardware constants. Hardwired by the VGA standard, never changes.
0xB8000 is the physical memory address of the VGA text buffer, shows on screen.
*/
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY_ADDRESS 0xB8000

/*
 VGA text mode: 80x25 grid of characters. Each cell has foreground (the char)
 and a background color. 16 hardwired colors, values fixed by VGA standards since 80s.
 */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

/*
  VGA expects foreground background packed into single byte.
  Layout is bits: [7 6 5 4] [3 2 1 0]
                    bg         fg
  bg << 4 just shifts the bg to correct starting bit
*/
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
  return fg | bg << 4;
}

/*
 Same packing trick. Bits: [15..8] [7..0]
                            color   char
 Just packed into 16 bits, so each character on screen is 2 bytes.
*/
static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
  return (uint16_t) uc | (uint16_t) color << 8;
}

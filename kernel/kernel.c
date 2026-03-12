/*
IMPORTANT REMINDER: Most std libs are NOT available since we are using freestanding and not hosted.

C refresher:

static: function limits the visibility to this file only. other .c files can't see them.
This helps us prevent name collisions.

inline: hints the compiler to 
"Just paste code where called instead of doing a function call"
no call overhead no stack frame.
*/


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Check to make sure you aren't targeting linux OS
#if defined(__linux__)
#error "You are not  using cross-compiler, things will break ;)"
#endif

// Make sure we are targeting the correct architecture for this kernel
#if !defined(__i386__)
#error "This ain't going to work, needs to be compiled in ix86-elf compiler"
#endif

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

// no stdlib, so we need to implement this.
size_t strlen(const char* str)
{
  size_t len = 0;
  while (str[len])
    len++;
  return len;
}

/*
Hardware constants. Hardwired by the VGA standard, never changes.
0xB8000 is the physical memory address of the VGA text buffer, shows on screen.
*/
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY_ADDRESS 0xB8000
#define NEWLINE_CHAR '\n'
#define SCROLL_HISTORY 100
#define BUFFER_HEIGHT (VGA_HEIGHT + SCROLL_HISTORY)

// Global states for the terminal
static size_t terminal_row;
static size_t terminal_column;
static size_t terminal_offset;
static uint8_t terminal_color;
static uint16_t terminal_buffer[BUFFER_HEIGHT * VGA_WIDTH];
static uint16_t* vga_memory = (uint16_t*)VGA_MEMORY_ADDRESS;

/*
 Used to draw the visible window on our VGA ouput
 init: initialize all the cell to be black else redraw while respecting scroll offset
*/
void draw_buffer(bool init)
{
  for (size_t  y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t memory_index = y * VGA_WIDTH + x;
      const size_t buffer_index = (y + terminal_offset % BUFFER_HEIGHT) * VGA_WIDTH + x;

      if (init) {
	vga_memory[memory_index] = vga_entry(' ', terminal_color);
      } else {
	vga_memory[memory_index] = terminal_buffer[buffer_index];
      }
    }
  }
}

/*
 Clears screen and resets cursor.
 VGA memory is flat array, 2d coords maps index at 0
 row0: 0..79
 row1: 80..159
 rown: n*VGA_WIDTH, (n+1)VGA_*WIDTH-1

 Index of cursor: y * VGA_WIDTH + x
*/
void terminal_initialize(void)
{
  terminal_row = 0;
  terminal_column = 0;
  terminal_offset = 0;
  terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

  draw_buffer(true);
}


void terminal_setcolor(uint8_t color)
{
  terminal_color = color;
}

// Writes a single character at a specific (x,y)
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
  const size_t index = y * VGA_WIDTH + x;
  terminal_buffer[index] = vga_entry(c, color);
}


// Writes a character at current position and moves the index in respect to 
// VGA_WIDTH and VGA_HEIGHT
void terminal_putchar(char c)
{
  if (c == NEWLINE_CHAR) {
    terminal_column = 0;
    terminal_row++;
  } else {
    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == VGA_WIDTH){
      terminal_column = 0;
      terminal_row++;
    }
  }

  //scroll when we pass visible height
  if (terminal_row >= terminal_offset + VGA_HEIGHT) {
    terminal_offset++;

    // wrap offset to prevent overflowing buffer
    if (terminal_offset + VGA_HEIGHT >= BUFFER_HEIGHT) {
      terminal_offset = 0;
    }
  }
  // wrap when full
  if (terminal_row >= VGA_HEIGHT + SCROLL_HISTORY) {
    terminal_row = 0;
  }
}

void terminal_write(const char* data, size_t size)
{
  for (size_t i = 0; i < size; i++) {
    terminal_putchar(data[i]);
  }
}

void terminal_writestring(const char* data)
{
  terminal_write(data, strlen(data));
  draw_buffer(false);
}

void terminal_writedecimal(size_t n)
{
  if (n == 0) {
    terminal_putchar('0');
    return;
  }

  char buffer[20];
  size_t i = 0;

  while (n > 0){
    // % 10 extracts the last digit (123 % 10 = 3)
    // '0' + digit converts number to ASCII character ('0' + 3 = '3')
    buffer[i++] = '0' + (n % 10);
    // /10 chops off the last digit (123/10 = 12)
    n /= 10 ;
  }

  // buffer is filled backwards => (123->['3', '2', '1'])
  // so we need to print it backwards
  while (i > 0) {
    terminal_putchar(buffer[--i]);
  }
}

// Main kernel function, calls the functions on top to simply write Hello world
void kernel_main(void)
{
  // Initialize terminal interface
  terminal_initialize();


  size_t lines = 110;
  // Write string in loop to test terminal_scrolling
  for (size_t i = 0; i < lines; i++) {
    terminal_writestring("Lines Number: ");
    terminal_writedecimal(i);
    terminal_writestring("\n");
  }
}

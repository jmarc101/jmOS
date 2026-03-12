#pragma once

#include "vga.h"

#define NEWLINE_CHAR '\n'
#define SCROLL_HISTORY 100
#define BUFFER_HEIGHT (VGA_HEIGHT + SCROLL_HISTORY)

void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
void terminal_putchar(char c);
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_writedecimal(size_t n);

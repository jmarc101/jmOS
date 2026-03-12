#include "terminal.h"

// Kernel entrypoint
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

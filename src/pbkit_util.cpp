#include "pbkit_util.h"

#include <cstdarg>
#include <cstdio>

extern "C" {
void _putchar(char character) { putchar(character); }
}

void pb_print_with_floats(const char* format, ...) {
  char buffer[512];

  va_list argList;
  va_start(argList, format);
  vsnprintf_(buffer, 512, format, argList);
  va_end(argList);

  char* str = buffer;
  while (*str != 0) {
    pb_print_char(*str++);
  }
}

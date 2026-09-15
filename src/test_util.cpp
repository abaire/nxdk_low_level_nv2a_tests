#include "test_util.h"

#include <pbkit/pbkit.h>

#include <cstdarg>
#include <cstdio>

#include "printf/printf.h"

extern "C" {
void _putchar(char character) { putchar(character); }
}

void pb_print_with_floats(const char* format, ...) {
  char buffer[512];

  va_list argList;
  va_start(argList, format);
  vsnprintf_(buffer, sizeof(buffer), format, argList);
  va_end(argList);

  char* str = buffer;
  while (*str != 0) {
    pb_print_char(*str++);
  }
}

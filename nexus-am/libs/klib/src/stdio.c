#include "klib.h"
#include <stdarg.h>
#include <limits.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int num2str(char* out, int num) {
  char* end = out;
  char tmp;
  while (num) {
    *end++ = num % 10 + '0';
    num /= 10;
  }
  int r = end - out;
  end--;
  while (out < end) {
    tmp = *out;
    *out++ = *end;
    *end-- = tmp;
  }
  return r;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  int num;
  char* start = out;
  char* end = out + n;
  char* str;

  if (end < start) {
    end = (void*)-1;
    n = end - start;
  }

  while (*fmt) {
    if (*fmt != '%') {
      *start++ = *fmt++;
      continue;
    }
    fmt++;
    switch (*fmt++) {
      case 'd': {
        num = va_arg(ap, int);
        start += num2str(start, num);
        break;
      }
      case 's': {
        str = va_arg(ap, char*);
        while (start < end && (*start++ = *str++) != '\0');
        break;
      }
    }
  }
  if (start < end) {
    start[0] = '\0';
  } else {
    end[-1] = '\0';
  }
  return start - out;
}

int printf(const char *fmt, ...) {
  char buf[1024];
  va_list args;
  int i;

  va_start(args, fmt);
  i = vsnprintf(buf, 1024, fmt, args);
  va_end(args);

  for (char* p = buf; *p != '\0'; p++) {
    _putc(*p);
  }

  return i;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return vsnprintf(out, INT_MAX, fmt, ap);;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list args;
  int i;

  va_start(args, fmt);
  i = vsnprintf(out, INT_MAX, fmt, args);
  va_end(args);

  return i;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list args;
  int i;

  va_start(args, fmt);
  i = vsnprintf(out, n, fmt, args);
  va_end(args);

  return i;
}

#endif

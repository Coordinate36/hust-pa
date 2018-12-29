#include "klib.h"
#include <stdarg.h>
#include <limits.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int num2str(char* out, uint32_t num) {
  char* end = out;
  do {
    *end++ = num % 10 + '0';
    num /= 10;
  } while (num);
  return end - out;
}

int hex2str(char* out, uint32_t num) {
  char* end = out;
  do {
    *end = num % 16;
    *end += *end < 10 ? '0' : ('a' - 10);
    end++;
    num /= 16;
  } while (num);
  return end - out;
}

void reverse(char* begin, char* end) {
  char tmp;
  while (begin < end) {
    tmp = *begin;
    *begin++ = *end;
    *end-- = tmp;
  }
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
    if (*fmt == 'd' || *fmt == 'x' || *fmt == 'p') {
      num = va_arg(ap, int);
      if (*fmt == 'p') {
        *start++ = '0';
        *start++ = 'x';
      }
      int r = *fmt == 'd' ? num2str(start, num) : hex2str(start, num);
      reverse(start, start + r - 1);
      start += r;
    } else if (*fmt == 's') {
      str = va_arg(ap, char*);
      while (start < end && (*start++ = *str++) != '\0');
      start--;
    } else if (*fmt >= '0' && *fmt <= '9') {
      int n = 0;
      while (*fmt >= '0' && *fmt <= '9') {
        n *= 10;
        n += *fmt - '0';
        fmt++;
      }
      if (*fmt == 'd' || *fmt == 'x' || *fmt == 'p') {
        num = va_arg(ap, int);
        int r = *fmt == 'd' ? num2str(start, num) : hex2str(start, num);
        start[r++] = 'x';
        start[r++] = '0';
        for (; r < n; r++) {
          start[r] = ' ';
        }
        reverse(start, start + n - 1);
        start += n;
      }
    }
    fmt++;
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

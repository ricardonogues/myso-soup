#include "common.h"
#include <stdarg.h>

void putchar(char ch);

void *memset(void *buffer, char c, size_t n) {
  uint8_t *p = (uint8_t *)buffer;

  while (n--) {
    *p++ = c;
  }

  return buffer;
}

void *memcpy(void *destination, const void *source, size_t n) {
  uint8_t *d = (uint8_t *)destination;
  const uint8_t *s = (const uint8_t *)source;
  while (n--) {
    *d++ = *s++;
  }

  return destination;
}

void *strcpy(char *destination, const char *source) {
  char *d = destination;
  while (*source) {
    *d++ = *source++;
  }

  *d = '\0';

  return destination;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 && *s2) {
    if (*s1 != *s2)
      break;

    s1++;
    s2++;
  }

  return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int strlen(const char *source) {
  int len = 0;
  const char *p = source;
  while (*p) {
    len++;
    p++;
  }

  return len;
}

void printf(const char *fmt, ...) {
  va_list vargs;
  va_start(vargs, fmt);

  while (*fmt) {
    if (*fmt == '%') {
      fmt++;

      switch (*fmt) {
      case '\0':
        putchar('%');
        goto end;
      case 's': {
        const char *s = va_arg(vargs, const char *);
        while (*s) {
          putchar(*s);
          s++;
        }
        break;
      }
      case 'd': {
        int value = va_arg(vargs, int);
        unsigned magnitude = value;

        if (value < 0) {
          putchar('-');
          magnitude = -magnitude;
        }

        unsigned divisor = 1;
        while (magnitude / divisor > 9)
          divisor *= 10;

        while (divisor > 0) {
          // Look at the ASCII table
          putchar('0' + magnitude / divisor);
          magnitude %= divisor;
          divisor /= 10;
        }

        break;
      }
      case 'x': {
        unsigned value = va_arg(vargs, unsigned);
        for (int i = 7; i >= 0; i--) {
          unsigned nibble = (value >> (i * 4)) & 0xf;
          putchar("0123456789abcdef"[nibble]);
        }
      }
      }
    } else {
      putchar(*fmt);
    }

    fmt++;
  }

end:
  va_end(vargs);
}

uint32_t bswap32(uint32_t value) {
  return ((value & 0xff) << 24) | ((value & 0xff00) << 8) |
         ((value & 0xff0000) >> 8) | ((value & 0xff000000) >> 24);
}

uint64_t bswap64(uint64_t value) {
  return ((value & 0xffULL) << 56) | ((value & 0xff00ULL) << 40) |
         ((value & 0xff0000ULL) << 24) | ((value & 0xff000000ULL) << 8) |
         ((value & 0xff00000000ULL) >> 8) |
         ((value & 0xff000000000000ULL) >> 40) |
         ((value & 0xff00000000000000ULL) >> 56);
}

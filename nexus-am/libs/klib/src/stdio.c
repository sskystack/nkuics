#include <klib.h>
#include <limits.h>

typedef uint32_t size32_t;

static void buf_putc(char **out, size32_t *left, char ch, size32_t *count) {
  if (*left > 1) {
    **out = ch;
    (*out)++;
    (*left)--;
  }
  (*count)++;
}

static void buf_puts(char **out, size32_t *left, const char *s, size32_t *count) {
  if (s == NULL) {
    s = "(null)";
  }
  while (*s) {
    buf_putc(out, left, *s++, count);
  }
}

static void buf_putu32(char **out, size32_t *left, uint32_t val,
                       unsigned base, int upper, size32_t *count) {
  char tmp[32];
  int idx = 0;
  const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";

  if (base < 2 || base > 16) {
    return;
  }

  if (val == 0) {
    tmp[idx++] = '0';
  }
  else {
    while (val > 0 && idx < (int)sizeof(tmp)) {
      tmp[idx++] = digits[val % base];
      val /= base;
    }
  }

  while (idx-- > 0) {
    buf_putc(out, left, tmp[idx], count);
  }
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
  char *out = str;
  size32_t left = (size > UINT32_MAX) ? UINT32_MAX : (size32_t)size;
  size32_t count = 0;

  for (; *format; format++) {
    if (*format != '%') {
      buf_putc(&out, &left, *format, &count);
      continue;
    }

    format++;
    if (*format == '%') {
      buf_putc(&out, &left, '%', &count);
      continue;
    }

    switch (*format) {
      case 'c': {
        int ch = va_arg(ap, int);
        buf_putc(&out, &left, (char)ch, &count);
        break;
      }
      case 's': {
        const char *s = va_arg(ap, const char *);
        buf_puts(&out, &left, s, &count);
        break;
      }
      case 'd':
      case 'i': {
        int v = va_arg(ap, int);
        if (v < 0) {
          buf_putc(&out, &left, '-', &count);
          buf_putu32(&out, &left, (uint32_t)(-v), 10, 0, &count);
        }
        else {
          buf_putu32(&out, &left, (uint32_t)v, 10, 0, &count);
        }
        break;
      }
      case 'u': {
        unsigned int v = va_arg(ap, unsigned int);
        buf_putu32(&out, &left, (uint32_t)v, 10, 0, &count);
        break;
      }
      case 'x':
      case 'X': {
        unsigned int v = va_arg(ap, unsigned int);
        buf_putu32(&out, &left, (uint32_t)v, 16, *format == 'X', &count);
        break;
      }
      case 'p': {
        uintptr_t v = (uintptr_t)va_arg(ap, void *);
        buf_puts(&out, &left, "0x", &count);
        buf_putu32(&out, &left, (uint32_t)v, 16, 0, &count);
        break;
      }
      default:
        buf_putc(&out, &left, '%', &count);
        buf_putc(&out, &left, *format, &count);
        break;
    }
  }

  if (size > 0) {
    if (left > 0) {
      *out = '\0';
    }
    else {
      str[size - 1] = '\0';
    }
  }

  return (int)count;
}

int vsprintf(char *str, const char *format, va_list ap) {
  return vsnprintf(str, UINT32_MAX, format, ap);
}

int snprintf(char *s, size_t n, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  int ret = vsnprintf(s, n, format, ap);
  va_end(ap);
  return ret;
}

int sprintf(char *out, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  int ret = vsnprintf(out, UINT32_MAX, format, ap);
  va_end(ap);
  return ret;
}

int printf(const char *fmt, ...) {
  char buf[1024];
  va_list ap;
  va_start(ap, fmt);
  int ret = vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);

  for (int i = 0; i < ret && i < (int)sizeof(buf) - 1; i++) {
    _putc(buf[i]);
  }
  return ret;
}

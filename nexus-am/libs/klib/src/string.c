#include <klib.h>

void *memset(void *v, int c, size_t n) {
  unsigned char *p = (unsigned char *)v;
  while (n-- > 0) {
    *p++ = (unsigned char)c;
  }
  return v;
}

void *memcpy(void *dst, const void *src, size_t n) {
  unsigned char *d = (unsigned char *)dst;
  const unsigned char *s = (const unsigned char *)src;
  while (n-- > 0) {
    *d++ = *s++;
  }
  return dst;
}

void *memmove(void *dst, const void *src, size_t n) {
  unsigned char *d = (unsigned char *)dst;
  const unsigned char *s = (const unsigned char *)src;
  if (d == s || n == 0) {
    return dst;
  }
  if (d < s) {
    while (n-- > 0) {
      *d++ = *s++;
    }
  }
  else {
    d += n;
    s += n;
    while (n-- > 0) {
      *--d = *--s;
    }
  }
  return dst;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;
  while (n-- > 0) {
    if (*p1 != *p2) {
      return (int)*p1 - (int)*p2;
    }
    p1++;
    p2++;
  }
  return 0;
}

size_t strlen(const char *s) {
  const char *p = s;
  while (*p) {
    p++;
  }
  return (size_t)(p - s);
}

char *strcpy(char *dst, const char *src) {
  char *d = dst;
  while ((*d++ = *src++) != '\0') {
  }
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i = 0;
  for (; i < n && src[i] != '\0'; i++) {
    dst[i] = src[i];
  }
  for (; i < n; i++) {
    dst[i] = '\0';
  }
  return dst;
}

char *strcat(char *dst, const char *src) {
  char *d = dst + strlen(dst);
  while ((*d++ = *src++) != '\0') {
  }
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 && *s1 == *s2) {
    s1++;
    s2++;
  }
  return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  while (n > 0 && *s1 && *s1 == *s2) {
    s1++;
    s2++;
    n--;
  }
  if (n == 0) {
    return 0;
  }
  return (unsigned char)*s1 - (unsigned char)*s2;
}

const char *strchr(const char *s, int c) {
  char ch = (char)c;
  while (*s) {
    if (*s == ch) {
      return s;
    }
    s++;
  }
  return (ch == '\0') ? s : NULL;
}

char *strstr(const char *haystack, const char *needle) {
  if (*needle == '\0') {
    return (char *)haystack;
  }
  for (; *haystack != '\0'; haystack++) {
    const char *h = haystack;
    const char *n = needle;
    while (*h && *n && *h == *n) {
      h++;
      n++;
    }
    if (*n == '\0') {
      return (char *)haystack;
    }
  }
  return NULL;
}

static int is_delim(char ch, const char *delim) {
  for (; *delim; delim++) {
    if (ch == *delim) {
      return 1;
    }
  }
  return 0;
}

char *strtok(char *s, const char *delim) {
  static char *next;
  if (s == NULL) {
    s = next;
  }
  if (s == NULL) {
    return NULL;
  }
  while (*s && is_delim(*s, delim)) {
    s++;
  }
  if (*s == '\0') {
    next = NULL;
    return NULL;
  }
  char *start = s;
  while (*s && !is_delim(*s, delim)) {
    s++;
  }
  if (*s) {
    *s = '\0';
    next = s + 1;
  }
  else {
    next = NULL;
  }
  return start;
}

#include <klib.h>

static unsigned long next_rand = 1;

void srand(unsigned int seed) {
  next_rand = (unsigned long)seed;
}

int rand() {
  // ANSI C compatible LCG
  next_rand = next_rand * 1103515245u + 12345u;
  return (int)((next_rand / 65536u) % 32768u);
}

int atoi(const char *nptr) {
  if (nptr == NULL) return 0;

  while (*nptr == ' ' || *nptr == '\t' || *nptr == '\n' ||
         *nptr == '\r' || *nptr == '\f' || *nptr == '\v') {
    nptr++;
  }

  int sign = 1;
  if (*nptr == '+') {
    nptr++;
  } else if (*nptr == '-') {
    sign = -1;
    nptr++;
  }

  int val = 0;
  while (*nptr >= '0' && *nptr <= '9') {
    val = val * 10 + (*nptr - '0');
    nptr++;
  }

  return sign * val;
}

int abs(int x) {
  return x < 0 ? -x : x;
}

unsigned long time() {
  return _uptime() / 1000;
}

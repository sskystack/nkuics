#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  int sign = 1;
  uint32_t x, y;

  if (a < 0) {
    sign = -sign;
    x = -a;
  } else {
    x = a;
  }

  if (b < 0) {
    sign = -sign;
    y = -b;
  } else {
    y = b;
  }

  uint32_t x_int = x >> 16;
  uint32_t x_frac = x & 0xffff;
  uint32_t y_int = y >> 16;
  uint32_t y_frac = y & 0xffff;
  uint32_t result = (x_int * y_int << 16)
    + x_int * y_frac
    + y_int * x_frac
    + ((x_frac * y_frac) >> 16);

  return sign < 0 ? -(FLOAT)result : (FLOAT)result;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  assert(b != 0);
  return ((a / b) << 16) + (((a % b) << 16) / b);
}

FLOAT f2F(float a) {
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   */

  uint32_t u = *(uint32_t *)&a;
  int sign = u >> 31;
  int exp_raw = (u >> 23) & 0xff;

  if (exp_raw == 0) {
    return 0;
  }

  int exp = exp_raw - 127;
  uint32_t frac = (u & 0x7fffff) | 0x800000;

  int shift = exp - 7;
  int32_t result = 0;
  if (shift >= 0) {
    result = (int32_t)(frac << shift);
  } else if (shift > -31) {
    result = (int32_t)(frac >> -shift);
  }

  return sign ? -result : result;
}

FLOAT Fabs(FLOAT a) {
  return a < 0 ? -a : a;
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);

  do {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);

  do {
    t2 = F_mul_F(t, t);
    dt = (F_div_F(x, t2) - t) / 3;
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

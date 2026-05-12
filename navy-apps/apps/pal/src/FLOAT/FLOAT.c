#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  return ((int64_t)a * b) / (1 << 16);
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  assert(b != 0);
  return ((int64_t)a * (1 << 16)) / b;
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

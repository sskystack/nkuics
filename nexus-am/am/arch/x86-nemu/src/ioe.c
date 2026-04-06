#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48   // Note that this is not standard
static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
  return inl(RTC_PORT) - boot_time;
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  int row;
  int width = _screen.width;
  for (row = 0; row < h; row++) {
    uint32_t *dst = fb + (y + row) * width + x;
    const uint32_t *src = pixels + row * w;
    for (int col = 0; col < w; col++) {
      dst[col] = src[col] | 0xff000000;
    }
  }
}

void _draw_p(int x, int y, uint32_t color) {
  _draw_rect(&color, x, y, 1, 1);
}

void _draw_sync() {
}

int _read_key() {
  uint8_t status = inb(0x64);
  if (status & 0x1) {
    return (int)inl(0x60);
  }
  return _KEY_NONE;
}

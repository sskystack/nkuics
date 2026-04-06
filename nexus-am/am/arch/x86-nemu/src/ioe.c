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
  int draw_w = (w > _screen.width - x) ? (_screen.width - x) : w;
  int copy_bytes = sizeof(uint32_t) * draw_w;
  for (int j = 0; j < h && (y + j) < _screen.height; j++) {
    memcpy(&fb[(y + j) * _screen.width + x], pixels, copy_bytes);
    pixels += w;
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

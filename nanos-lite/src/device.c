#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

#define KEYDOWN_MASK 0x8000

size_t events_read(void *buf, size_t len) {
  int key = _read_key();
  if (key == _KEY_NONE) {
    return 0;
  }

  bool is_keydown = (key & KEYDOWN_MASK) != 0;
  key &= ~KEYDOWN_MASK;

  int n = snprintf((char *)buf, len, "%s %s\n", is_keydown ? "kd" : "ku", keyname[key]);
  if (n < 0) return 0;
  return n;
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
  memcpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  const uint32_t *pixels = (const uint32_t *)buf;
  int width = _screen.width;
  int pixel_off = offset / sizeof(uint32_t);
  int x = pixel_off % width;
  int y = pixel_off / width;
  int n = len / sizeof(uint32_t);

  while (n > 0) {
    int w = width - x;
    if (w > n) w = n;
    _draw_rect(pixels, x, y, w, 1);
    pixels += w;
    n -= w;
    x = 0;
    y++;
  }
  _draw_sync();
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  snprintf(dispinfo, sizeof(dispinfo), "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}

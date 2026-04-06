#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

#define KEYDOWN_MASK 0x8000

size_t events_read(void *buf, size_t len) {
  if (len == 0) return 0;

  int key = _read_key();
  char event[64];
  static int event_log_cnt = 0;

  if (key != _KEY_NONE) {
    bool is_keydown = (key & KEYDOWN_MASK) != 0;
    key &= ~KEYDOWN_MASK;
    const char *name = (key >= 0 && key < (int)(sizeof(keyname) / sizeof(keyname[0])) && keyname[key] != NULL)
      ? keyname[key]
      : "UNKNOWN";
    snprintf(event, sizeof(event), "%s %s\n", is_keydown ? "kd" : "ku", name);
  } else {
    snprintf(event, sizeof(event), "t %lu\n", _uptime());
  }

  size_t n = strlen(event);
  if (n > len) n = len;
  if (event_log_cnt < 20 || (event_log_cnt < 200 && (event_log_cnt % 20 == 0))) {
    Log("events_read: %s", event);
  }
  event_log_cnt++;
  memcpy(buf, event, n);
  return n;
}

static char dispinfo[128] __attribute__((used));

size_t dispinfo_read(void *buf, off_t offset, size_t len) {
  size_t info_len = strlen(dispinfo);
  if (offset >= (off_t)info_len) return 0;
  if (offset + len > info_len) {
    len = info_len - offset;
  }
  memcpy(buf, dispinfo + offset, len);
  return len;
}

size_t fb_write(const void *buf, off_t offset, size_t len) {
  const uint32_t *pixels = (const uint32_t *)buf;
  static int fb_pixel_log_cnt = 0;
  if (fb_pixel_log_cnt < 3 && len >= sizeof(uint32_t)) {
    Log("fb_write: offset=%d len=%d first_pixel=%x", offset, len, pixels[0]);
    fb_pixel_log_cnt++;
  }
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
  return len;
}

size_t dispinfo_size(void) {
  return strlen(dispinfo);
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  snprintf(dispinfo, sizeof(dispinfo), "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}

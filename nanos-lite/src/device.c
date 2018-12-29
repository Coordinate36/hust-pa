#include "common.h"
#include "fs.h"
#include <amdev.h>

int fg_pcb = 2;

size_t serial_write(const void *buf, size_t offset, size_t len) {
  for (int i = 0; i < len; i++) {
    _putc(((char*)buf)[i]);
  }
  return len;
}

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t offset, size_t len) {
  int key = read_key();
  bool down = false;
  if (key & 0x8000) {
    down = true;
    key ^= 0x8000;
  }
  if (key == _KEY_NONE) {
    sprintf(buf, "t %d\n", uptime());
  } else {
    if (key == 2 || key == 3 || key == 4) {
      fg_pcb = key;
    }
    sprintf(buf, "%s %s\n", down ? "kd" : "ku", keyname[key]);
  }
  return strlen(buf);
}

static char dispinfo[128] __attribute__((used));

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  memcpy(buf, dispinfo, len);
  return len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  int x, y;
  int rest, height;
  offset >>= 2;
  len >>= 2;
  int width = screen_width();
  y = offset / width;
	x = offset - y * width;
  
	rest = len <= width - x ? len : width - x;
	draw_rect((uint32_t *)buf, x, y, rest, 1);

  len -= rest;
  rest = 0;
  height = len / width;
  if (height > 0) {
	  draw_rect((uint32_t *)buf, 0, y + 1, width, height);
    rest = len - height * width;
  }

  if (rest > 0) {
    draw_rect((uint32_t *)buf, 0, y + height + 1, rest, 1);
  }

  return len;
}

void init_device() {
  Log("Initializing devices...");
  _ioe_init();

  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d", screen_width(), screen_height());
}

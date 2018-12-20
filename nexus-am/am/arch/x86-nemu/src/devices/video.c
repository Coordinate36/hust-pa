#include <am.h>
#include <x86.h>
#include <amdev.h>
#include <klib.h>

static uint32_t* const fb __attribute__((used)) = (uint32_t *)0x40000;

size_t video_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_VIDEO_INFO: {
      _VideoInfoReg *info = (_VideoInfoReg *)buf;
      uint32_t screensize = inl(0x100);
      info->width = screensize >> 16;
      info->height = screensize & 0xffff;
      return sizeof(_VideoInfoReg);
    }
  }
  return 0;
}

size_t video_write(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_VIDEO_FBCTL: {
      _FBCtlReg *ctl = (_FBCtlReg *)buf;

      int width = screen_width();
      for (int y = ctl->y; y < ctl->y + ctl->h; y++) {
        for (int x = ctl->x; x < ctl->x + ctl->w; x++) {
          fb[x + y * width] = ctl->pixels[(y - ctl->y) * ctl->w + (x - ctl->x)];
        }
      }

      if (ctl->sync) {
        // do nothing, hardware syncs.
      }
      return sizeof(_FBCtlReg);
    }
  }
  return 0;
}

void vga_init() {
}

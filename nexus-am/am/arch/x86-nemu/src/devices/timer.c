#include <am.h>
#include <x86.h>
#include <amdev.h>

static uint32_t boot_time;

size_t timer_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_TIMER_UPTIME: {
      _UptimeReg *uptime = (_UptimeReg *)buf;
      uptime->hi = 0;
      uptime->lo = inl(0x48) - boot_time;
      return sizeof(_UptimeReg);
    }
    case _DEVREG_TIMER_DATE: {
      _RTCReg *rtc = (_RTCReg *)buf;
      rtc->second = 5;
      rtc->minute = 4;
      rtc->hour   = 3;
      rtc->day    = 2;
      rtc->month  = 1;
      rtc->year   = 2018;
      return sizeof(_RTCReg);
    }
  }
  return 0;
}

void timer_init() {
  boot_time = inl(0x48);
}

#include "common.h"
#include "syscall.h"

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  uint32_t rst;

  switch (a[0]) {
    case SYS_yield: _yield(); rst = 0; break;
    case SYS_brk: rst = 0; break;
    case SYS_write: {
      char* buf = (char*)a[2];
      if (a[1] == 1 || a[1] == 2) {
        int i;
        for (i = 0; i < a[3]; i++) {
          _putc(buf[i]);
        }
        rst = i;
      }
      rst = -1;
      break;
    }
    case SYS_exit: _halt(a[1]); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  c->GPRx = rst;

  return NULL;
}

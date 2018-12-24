#include "common.h"
#include "syscall.h"
#include "fs.h"
#include "proc.h"

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  uint32_t rst = 0;

  switch (a[0]) {
    case SYS_yield: _yield(); rst = 0; break;
    case SYS_execve: naive_uload(NULL, (const char*)a[1]); break;
    case SYS_brk: rst = 0; break;
    case SYS_lseek: rst = fs_lseek(a[1], a[2], a[3]); break;
    case SYS_write: rst = fs_write(a[1], (void*)a[2], a[3]); break;
    case SYS_read: rst = fs_read(a[1], (void*)a[2], a[3]); break;
    case SYS_open: rst = fs_open((const char*)a[1], a[2], a[3]); break;
    case SYS_close: rst = fs_close(a[1]); break;
    case SYS_exit: _halt(a[1]); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  c->GPRx = rst;

  return NULL;
}

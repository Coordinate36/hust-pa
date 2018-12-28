#include "proc.h"
#include "memory.h"
#include "fs.h"

#define DEFAULT_ENTRY 0x8048000

static uintptr_t loader(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  int size = fs_filesz(fd);
  int offset = 0;
  for (; size > 0; size -= PGSIZE) {
    void* pp = new_page(1);
    _map(&pcb->as, (void*)(DEFAULT_ENTRY + offset), pp, 0);
    fs_read(fd, pp, size > PGSIZE ? PGSIZE: size);
    offset += PGSIZE;
  }
  pcb->max_brk = DEFAULT_ENTRY + size + offset;
  fs_close(fd);
  return DEFAULT_ENTRY;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  ((void(*)())entry) ();
}

void context_kload(PCB *pcb, void *entry) {
  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _kcontext(stack, entry, NULL);
}

void context_uload(PCB *pcb, const char *filename) {
  _protect(&pcb->as);
  uintptr_t entry = loader(pcb, filename);

  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
}

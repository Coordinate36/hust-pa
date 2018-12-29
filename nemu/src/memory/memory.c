#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int map_NO = is_mmio(addr);
  if (map_NO != -1) {
    return mmio_read(addr, len, map_NO);
  }
  return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, uint32_t data, int len) {
  int map_NO = is_mmio(addr);
  if (map_NO != -1) {
    mmio_write(addr, len, data, map_NO);
  } else {
    memcpy(guest_to_host(addr), &data, len);
  }
}

paddr_t page_translate(paddr_t addr) {
  if (cpu.cr0.paging == 0 || cpu.cr0.protect_enable == 0) {
    return addr;
  }
  uint32_t kp = cpu.cr3.val & ~0xfff;     // PDE
  uint32_t idx = addr >> 22;              // dir
  kp = paddr_read(kp + (idx << 2), 4);
  if ((kp & 1) == 0) {
    Log("cpu.cr3.val:%u, addr:%u, kp:%u\n", cpu.cr3.val, addr, kp);
  }
  assert(kp & 1);
  kp &= ~0xfff;
  idx = addr << 10 >> 22;                 // page
  kp = paddr_read(kp + (idx << 2), 4);    // page frame
  if ((kp & 1) == 0) {
    Log("cpu.cr3.val:%u, addr:%u, idx:%u, kp:%u\n", cpu.cr3.val, addr, idx, kp);
  }
  assert(kp & 1);
  kp &= ~0xfff;
  idx = addr & 0xfff;                     // offset
  return kp + idx;
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if ((addr & 0xfff) + len > 0x1000) {
    uint32_t shift =  0x1000 - (addr & 0xfff);
    uint32_t paddr_low = page_translate(addr);
    uint32_t paddr_high = paddr_low + shift;
    return paddr_read(paddr_low, shift) | (paddr_read(paddr_high, len - shift) << (shift << 3));
  }
  paddr_t paddr = page_translate(addr);
  return paddr_read(paddr, len);
}

void vaddr_write(vaddr_t addr, uint32_t data, int len) {
  if ((addr & 0xfff) + len > 0x1000) {
    uint32_t shift =  0x1000 - (addr & 0xfff);
    uint32_t paddr_low = page_translate(addr);
    uint32_t paddr_high = paddr_low + shift;
    paddr_write(paddr_low, data, shift);
    paddr_write(paddr_high, data >> shift, len - shift);
  }
  paddr_t paddr = page_translate(addr);
  paddr_write(paddr, data, len);
}

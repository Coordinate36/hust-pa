#include "cpu/exec.h"
#include "device/port-io.h"

void difftest_skip_ref();
void difftest_skip_dut();

make_EHelper(lidt) {
  cpu.idtr.limit = vaddr_read(id_dest->addr, 2);
  cpu.idtr.base = vaddr_read(id_dest->addr + 2, 4);
  if (decoding.is_operand_size_16) {
    cpu.idtr.base &= 0xffffff;
  }

  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  assert(id_dest->reg == 0 || id_dest->reg == 3);
  if (id_dest->reg == 0) {
    cpu.cr0.val = id_src->val;
  } else {
    cpu.cr3.val = id_src->val;
  } 

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  assert(id_src->reg == 0 || id_src->reg == 3);
  t0 = id_src->reg == 0 ? cpu.cr0.val : cpu.cr3.val;
  operand_write(id_dest, &t0);

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#if defined(DIFF_TEST)
  difftest_skip_ref();
#endif
}

make_EHelper(int) {
  raise_intr(id_dest->val, *eip);

  print_asm("int %s", id_dest->str);

#if defined(DIFF_TEST) && defined(DIFF_TEST_QEMU)
  difftest_skip_dut();
#endif
}

make_EHelper(iret) {
  rtl_pop(eip);
  rtl_pop(&cpu.cs);
  rtl_pop(&cpu.eflags);

  print_asm("iret");
}

make_EHelper(in) {
  switch (id_dest->width) {
    case 1: t1 = pio_read_b(id_src->val); break;
    case 2: t1 = pio_read_w(id_src->val); break;
    case 4: t1 = pio_read_l(id_src->val); break;
    default: assert(0);
  }
  operand_write(id_dest, &t1);

  print_asm_template2(in);

#if defined(DIFF_TEST)
  difftest_skip_ref();
#endif
}

make_EHelper(out) {
  switch (id_dest->width) {
    case 1: pio_write_b(id_dest->val, id_src->val); break;
    case 2: pio_write_w(id_dest->val, id_src->val); break;
    case 4: pio_write_l(id_dest->val, id_src->val); break;
    default: assert(0);
  }

  print_asm_template2(out);

#if defined(DIFF_TEST)
  difftest_skip_ref();
#endif
}

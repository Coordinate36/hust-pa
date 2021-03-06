#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  rtl_push(&id_dest->val);

  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&id_dest->val);
  operand_write(id_dest, &id_dest->val);

  print_asm_template1(pop);
}

make_EHelper(pusha) {
  uint32_t width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_lr(&t0, R_ESP, width);
  rtl_lr(&t1, R_EAX, width);
  rtl_push(&t1);
  rtl_lr(&t1, R_ECX, width);
  rtl_push(&t1);
  rtl_lr(&t1, R_EDX, width);
  rtl_push(&t1);
  rtl_lr(&t1, R_EBX, width);
  rtl_push(&t1);
  rtl_push(&t0);
  rtl_lr(&t1, R_EBP, width);
  rtl_push(&t1);
  rtl_lr(&t1, R_ESI, width);
  rtl_push(&t1);
  rtl_lr(&t1, R_EDI, width);
  rtl_push(&t1);

  print_asm("pusha");
}

make_EHelper(popa) {
  uint32_t width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_pop(&t1);
  rtl_sr(R_EDI, &t1, width);
  rtl_pop(&t1);
  rtl_sr(R_ESI, &t1, width);
  rtl_pop(&t1);
  rtl_sr(R_EBP, &t1, width);
  rtl_pop(&t1);
  rtl_pop(&t1);
  rtl_sr(R_EBX, &t1, width);
  rtl_pop(&t1);
  rtl_sr(R_EDX, &t1, width);
  rtl_pop(&t1);
  rtl_sr(R_ECX, &t1, width);
  rtl_pop(&t1);
  rtl_sr(R_EAX, &t1, width);

  print_asm("popa");
}

make_EHelper(leave) {
  cpu.esp = cpu.ebp;
  rtl_pop(&cpu.ebp);

  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    rtl_lr(&t0, R_AX, 2);
    rtl_sext(&t0, &t0, 2);
    rtl_sr(R_EAX, &t0, 4);
  }
  else {
    rtl_lr(&t0, R_EAX, 4);
    rtl_msb(&t0, &t0, 4);
    t0 = 0 - t0;
    rtl_sr(R_EDX, &t0, 4);
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    rtl_lr(&t0, R_AL, 1);
    rtl_sext(&t0, &t0, 1);
    rtl_sr(R_AX, &t0, 2);
  }
  else {
    rtl_lr(&t0, R_AX, 2);
    rtl_sext(&t0, &t0, 2);
    rtl_sr(R_EAX, &t0, 4);
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t0, &id_src->val, id_src->width);
  operand_write(id_dest, &t0);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  operand_write(id_dest, &id_src->addr);
  print_asm_template2(lea);
}

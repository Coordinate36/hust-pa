#include "cpu/exec.h"

make_EHelper(mov);
make_EHelper(movsx);
make_EHelper(movzx);
make_EHelper(lea);
make_EHelper(call);

make_EHelper(push);
make_EHelper(pop);

make_EHelper(add);
make_EHelper(sub);
make_EHelper(xor);
make_EHelper(or);
make_EHelper(and);
make_EHelper(cmp);
make_EHelper(test);
make_EHelper(adc);
make_EHelper(sar);
make_EHelper(shl);
make_EHelper(dec);
make_EHelper(not);
make_EHelper(inc);
make_EHelper(imul2);
make_EHelper(cltd);
make_EHelper(idiv);

make_EHelper(setcc);

make_EHelper(jcc);
make_EHelper(jmp);
make_EHelper(ret);
make_EHelper(leave);

make_EHelper(operand_size);

make_EHelper(nop);
make_EHelper(inv);
make_EHelper(nemu_trap);

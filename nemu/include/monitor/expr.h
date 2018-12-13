#ifndef __EXPR_H__
#define __EXPR_H__

#include "common.h"

typedef struct {
    int type;
    union {
        int int_;
        double double_;
    };
} Operand;

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  NUMBER, TK_HEX, TK_REG, TK_NEQ, TK_AND, TK_DEFER, TK_NEG, DECIMAL
};

Operand expr(char *, bool *);
bool operand_equal(Operand* a, Operand* b);

#endif

#include "nemu.h"
#include "monitor/expr.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"\\(", '('},
  {"\\)", ')'},
  {"-", '-'},
  {"\\*", '*'},
  {"/", '/'},
  {"0[xX][0-9a-fA-F]+", TK_HEX},
  {"[0-9]+", NUMBER},
  {"[0-9]+\\.[0-9]+", DECIMAL},
  {"!=", TK_NEQ},
  {"&&", TK_AND},
  {"\\$[a-z]+", TK_REG}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];


/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[65535];
int nr_token;
Token poland_stack[65535];
Token poland_output[65535];
Operand num_stack[65535];

bool operand_equal(Operand* a, Operand* b) {
  if (a->type != b->type) {
    return false;
  }
  if (a->type == NUMBER) {
    return a->int_ == b->int_;
  }
  return a->double_ == b->double_;
}

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE: break;
          case DECIMAL:
          case TK_REG:
          case TK_HEX:
          case NUMBER: {
            if (substr_len > 32) {
              Log("The value of number is out of range");
              return false;
            }
            int j;
            for (j = 0; j < substr_len; j++) {
              tokens[nr_token].str[j] = substr_start[j];
            }
            tokens[nr_token].str[substr_len] = '\0';
          }
          default: {
            tokens[nr_token++].type = rules[i].token_type;
          }
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

int op_priority(int op) {
  switch (op) {
    case TK_AND: return 1;
    case TK_EQ:
    case TK_NEQ: return 2;
    case '+':
    case '-': return 3;
    case '*':
    case '/': return 4;
    case TK_NEG:
    case TK_DEFER: return 5;
    default: return 0;
  }
}

int make_poland() {
  int top = 0;
  int poland_len = 0;
  int i;
  for (i = 0; i < nr_token; i++) {
    switch (tokens[i].type) {
      case '(': poland_stack[top++] = tokens[i]; break;
      case ')': {
        while (top > 0 && poland_stack[--top].type != '(') {
          poland_output[poland_len++] = poland_stack[top];
        }
        break;
      }
      case DECIMAL:
      case TK_REG:
      case TK_HEX:
      case NUMBER: poland_output[poland_len++] = tokens[i]; break;
      default: {
        if (op_priority(tokens[i].type) > op_priority(poland_stack[top-1].type)) {
          poland_stack[top++] = tokens[i];
        } else {
          while (top > 0 && op_priority(tokens[i].type) <= op_priority(poland_stack[top-1].type)) {
            poland_output[poland_len++] = poland_stack[--top];
          }
          poland_stack[top++] = tokens[i];
        }
      }
    }
  }
  while (top > 0) {
    poland_output[poland_len++] = poland_stack[--top];
  }
  return poland_len;
}

int reg_value(char* reg) {
  int i;
  for (i = 0; i < 8; i++) {
    if (strcmp(reg, regsl[i]) == 0) {
      return reg_l(i);
    }
    if (strcmp(reg, regsw[i]) == 0) {
      return reg_w(i);
    }
    if (strcmp(reg, regsb[i]) == 0) {
      return reg_b(i);
    }
  }
  return cpu.eip;
}

Operand cal_poland(int poland_len) {
  int top = 0;
  Operand ans;
  Operand val;
  Operand top1, top2;
  int i;
  for (i = 0; i < poland_len; i++) {
    if (poland_output[i].type == NUMBER) {
      val.type = NUMBER;
      sscanf(poland_output[i].str, "%d", &val.int_);
      num_stack[top++] = val;
    } else if (poland_output[i].type == TK_HEX) {
      val.type = NUMBER;
      sscanf(poland_output[i].str, "%x", &val.int_);
      num_stack[top++] = val;
    } else if (poland_output[i].type == TK_REG) {
      val.type = NUMBER;
      val.int_ = reg_value(poland_output[i].str + 1);
      num_stack[top++] = val;
    } else if (poland_output[i].type == DECIMAL) {
      val.type = DECIMAL;
      sscanf(poland_output[i].str, "%lf", &val.double_);
      num_stack[top++] = val;
    } else {
      top1 = num_stack[--top];
      ans.type = top1.type;
      if (poland_output[i].type == TK_DEFER) {
        ans.int_ = top1.int_ * 2;
      } else if (poland_output[i].type == TK_NEG) {
        if (top1.type == NUMBER) {
          ans.int_ = -top1.int_;
        } else {
          ans.double_ = -top1.double_;
        }
      } else {
        // binary op
        top2 = num_stack[--top];
        if (top2.type == DECIMAL) {
          ans.type = DECIMAL;
        }
        if (ans.type == NUMBER) {
          switch (poland_output[i].type) {
            case '+': ans.int_ = top2.int_ + top1.int_; break;
            case '-': ans.int_ = top2.int_ - top1.int_; break;
            case '*': ans.int_ = top2.int_ * top1.int_; break;
            case '/': ans.int_ = top2.int_ / top1.int_; break;
            case TK_AND: ans.int_ = top2.int_ && top1.int_; break;
            case TK_NEQ: ans.int_ = top2.int_ != top1.int_; break;
            case TK_EQ:  ans.int_ = top2.int_ == top1.int_; break;
          }
        } else {
          switch (poland_output[i].type) {
            case '+': ans.double_ = (top2.type == NUMBER ? top2.int_: top2.double_) + (top1.type == NUMBER ? top1.int_: top1.double_); break;
            case '-': ans.double_ = (top2.type == NUMBER ? top2.int_: top2.double_) - (top1.type == NUMBER ? top1.int_: top1.double_); break;
            case '*': ans.double_ = (top2.type == NUMBER ? top2.int_: top2.double_) * (top1.type == NUMBER ? top1.int_: top1.double_); break;
            case '/': ans.double_ = (top2.type == NUMBER ? top2.int_: top2.double_) / (top1.type == NUMBER ? top1.int_: top1.double_); break;
            case TK_AND: ans.double_ = (top2.type == NUMBER ? top2.int_: top2.double_) && (top1.type == NUMBER ? top1.int_: top1.double_); break;
            case TK_NEQ: ans.double_ = (top2.type == NUMBER ? top2.int_: top2.double_) != (top1.type == NUMBER ? top1.int_: top1.double_); break;
            case TK_EQ:  ans.double_ = (top2.type == NUMBER ? top2.int_: top2.double_) == (top1.type == NUMBER ? top1.int_: top1.double_); break;
          }
        }
      }
      num_stack[top++] = ans;
    }
  }
  return num_stack[top-1];
}

Operand expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    Operand rst = {type: NUMBER, int_: 0};
    return rst;
  }

  /* TODO: Insert codes to evaluate the expression. */
  int i;
  int cnt = 0;
  for (i = 0; i < nr_token; i++) {
    if (tokens[i].type == '(') {
      cnt++;
    } else if (tokens[i].type == ')') {
      cnt--;
    }
    if (cnt < 0) {
      printf("Parentheses not matched\n");
      *success = false;
      Operand rst = {type: NUMBER, int_: 0};
      return rst;
    }
  }

  *success = true;

  for (i = 0; i < nr_token; i++) {
    if (tokens[i].type == '*') {
      if (i == 0) {
        tokens[i].type = TK_DEFER;
      }
      switch (tokens[i-1].type) {
        case '+': case '-': case '*': case '/': case '(':
        case TK_AND: case TK_NEQ: case TK_EQ: {
          tokens[i].type = TK_DEFER;
        }
      }
    } else if (tokens[i].type == '-') {
      if (i == 0) {
        tokens[i].type = TK_NEG;
      }
      switch (tokens[i-1].type) {
        case '+': case '-': case '*': case '/': case '(':
        case TK_AND: case TK_NEQ: case TK_EQ: {
          tokens[i].type = TK_NEG;
        }
      }
    }
  }

  int poland_len = make_poland();
  return cal_poland(poland_len);
}

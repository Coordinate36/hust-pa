#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  NUMBER, TK_HEX, TK_REG, TK_NEQ, TK_AND, TK_DEFER
};

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
int num_stack[65535];

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
    case TK_DEFER:
    case TK_REG: return 5;
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
  return -1;
}

uint32_t cal_poland(int poland_len) {
  int top = 0;
  uint32_t ans = 0;
  int val;
  int top1, top2;
  int i;
  for (i = 0; i < poland_len; i++) {
    if (poland_output[i].type == NUMBER) {
      sscanf(poland_output[i].str, "%d", &val);
      num_stack[top++] = val;
    } else if (poland_output[i].type == TK_HEX) {
      sscanf(poland_output[i].str, "%x", &val);
      num_stack[top++] = val;
    } else if (poland_output[i].type == TK_REG) {
      num_stack[top++] = reg_value(poland_output[i].str + 1);
    } else {
      top1 = num_stack[--top];
      if (poland_output[i].type == TK_DEFER) {
        ans = vaddr_read(top1, 1);
      } else {
        top2 = num_stack[--top];
        switch (poland_output[i].type) {
          case TK_AND: ans = top2 && top1; break;
          case TK_EQ:  ans = top2 == top1; break;
          case TK_NEQ: ans = top2 != top1; break;
          case '+': ans = top2 + top1; break;
          case '-': ans = top2 - top1; break;
          case '*': ans = top2 * top1; break;
          case '/': ans = top2 / top1; break;
        }
      }
      num_stack[top++] = ans;
    }
  }
  return num_stack[top-1];
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
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
      panic("Parentheses not matched");
    }
  }

  *success = true;

  for (i = 0; i < nr_token; i++) {
    if (tokens[i].type == '*') {
      if (i == 0) {
        tokens[i].type = TK_DEFER;
      }
      switch (tokens[i-1].type) {
        case '+': case '-': case '*': case '/':
        case TK_AND: case TK_NEQ: case TK_EQ: {
          tokens[i].type = TK_DEFER;
        }
      }
    }
  }

  int poland_len = make_poland();
  return cal_poland(poland_len);
}

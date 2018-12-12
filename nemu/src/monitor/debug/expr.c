#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  NUMBER, TK_HEX, TK_REG, TK_NEQ, TK_AND
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
  {"[0-9]+", NUMBER},
  {"0x[0-9a-f]+", TK_HEX},
  {"!=", TK_NEQ},
  {"&&", TK_AND}
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
    case '+':
    case '-': return 1;
    case '*':
    case '/': return 2;
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

int cal_poland(int poland_len) {
  int top = 0;
  int ans = 0;
  int val;
  int top1, top2;
  int i;
  for (i = 0; i < poland_len; i++) {
    if (poland_output[i].type == NUMBER) {
      sscanf(poland_output[i].str, "%d", &val);
      num_stack[top++] = val;
    } else {
      top1 = num_stack[--top];
      top2 = num_stack[--top];
      switch (poland_output[i].type) {
        case '+': ans = top2 + top1; break;
        case '-': ans = top2 - top1; break;
        case '*': ans = top2 * top1; break;
        case '/': ans = top2 / top1; break;
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
  *success = true;
  int poland_len = make_poland();
  return cal_poland(poland_len);
}

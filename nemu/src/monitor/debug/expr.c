#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  NUMBER
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
  {"[0-9]+", NUMBER}
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

Token tokens[32];
int nr_token;

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

uint32_t walk(int* step) {
  uint32_t rst = 0;
  uint32_t num = 0, next = 0;
  int op = '+';

  int i;
  for (i = *step; i < nr_token; i++) {
    if (tokens[i].type == NUMBER) {
      sscanf(tokens[i].str, "%u", &next);
    } else if (tokens[i].type == '(') {
      i++;
      next = walk(&i);
    } else {
      panic("Invalid token %d next to %c", tokens[i].type, op);
    }
    switch (op) {
      case '+': {
        rst += num;
        num = next;
        break;
      }
      case '-': {
        rst += num;
        num = -next;
        break;
      }
      case '*': {
        num *= next;
        break;
      }
      case '/': {
        num /= next;
        break;
      }
    }

    if (++i < nr_token) {
      op = tokens[i].type;
      if (op == ')') {
        *step = i;
        break;
      }
    }
  }

  rst += num;
  return rst;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  *success = true;
  int step = 0;
  return walk(&step);
}

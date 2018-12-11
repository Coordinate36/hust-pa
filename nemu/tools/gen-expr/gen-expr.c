#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536];
static char ops[] = {'+', '-', '*', '/'};

static inline void gen_rand_num(int begin) {
  sprintf(buf + begin, "%10d", abs(rand()) + 1);
}

static inline void gen_rand_op(int begin) {
  int choice = rand() % 4;
  buf[begin] = ops[choice];
}

static inline void gen_rand_expr(int begin) {
  int choice = rand() % 3;
  switch (choice) {
    case 0: {
      gen_rand_num(begin);
      break;
    }
    case 1: {
      if (begin >= 60000) {
        gen_rand_num(begin);
        break;
      }
      
      buf[begin] = '(';
      gen_rand_expr(begin + 1);
      int end = begin + strlen(buf + begin);
      if (end >= 60000) {
        buf[begin] = ' ';
        break;
      }
      buf[end++] = ')';
      buf[end] = '\0';
      break;
    }
    case 2: {
      if (begin >= 60000) {
        gen_rand_num(begin);
        break;
      }
      
      gen_rand_expr(begin);
      begin += strlen(buf + begin);
      if (begin >= 60000) {
        break;
      }
      gen_rand_op(begin);
      if (buf[begin] == '/') {
        gen_rand_num(begin + 1);
      } else {
        gen_rand_expr(begin + 1);
      }
      if (begin + strlen(buf + begin) >= 60000) {
        buf[begin] = '\0';
      }
    }
  }
}

static char code_buf[65536];
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr(0);

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen(".code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc .code.c -o .expr");
    if (ret != 0) continue;

    fp = popen("./.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}

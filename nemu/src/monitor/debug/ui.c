#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_si(char *args) {
  int n = args == NULL ? 1 : atoi(args);
  cpu_exec(n);
  return 0;
}

static int cmd_x(char *args) {
  uint32_t n;
  uint32_t addr;
  sscanf(args, "%d", &n);
  for (; *args != ' '; args++);
  for (; *args == ' '; args++);
  bool success;
  addr = expr(args, &success);
  printf("0x%x: ", addr);
  int i;
  for (i = 0; i < n; i++) {
    printf("0x%x\t", vaddr_read(addr + i, 1));
  }
  puts("");
  return 0;
}

static int cmd_p(char *args) {
  bool success;
  unsigned ans = expr(args, &success);
  assert(success);
  printf("%u\n", ans);
  return 0;
}

static int cmd_info(char *args) {
  int i;
  for (i = R_EAX; i <= R_EDI; i++) {
    printf("%s\t0x%x\t%d\n", regsl[i], reg_l(i), reg_l(i));
  }
  for (i = R_AX; i <= R_DI; i++) {
    printf("%s\t0x%x\t%d\n", regsw[i], reg_w(i), reg_w(i));
  }
  for (i = R_AL; i <= R_BH; i++) {
    printf("%s\t0x%x\t%d\n", regsb[i], reg_b(i), reg_b(i));
  }
  return 0;
}

static int cmd_w(char *args) {
  WP* wp = new_wp();
  bool success;
  strcpy(wp->expr, args);
  wp->value = expr(args, &success);
  assert(success);
  printf("Watchpoint %d %s\n", wp->NO, wp->expr);
  return 0;
}

static int cmd_d(char *args) {
  int n;
  sscanf(args, "%d", &n);
  free_wp(n - 1);
  return 0;
}

static int cmd_help(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "Single step execution", cmd_si },
  { "info", "Print program status", cmd_info },
  { "p", "Eval the expression", cmd_p },
  { "x", "Scan memory", cmd_x },
  { "w", "Set monitoring points", cmd_w },
  { "d", "Delete monitoring points", cmd_d }
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

#include "monitor/expr.h"
#include "monitor/watchpoint.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp() {
  if (free_ == NULL) {
    panic("The number of watch points can not exceed 32");
  }
  WP* wp = free_;
  free_ = free_->next;
  wp->next = head;
  head = wp;
  return wp;
}

void free_wp(int n) {
  assert(n >= 0 && n < NR_WP);
  WP *wp = &wp_pool[n];
  if (wp == head) {
    head = head->next;
  } else {
    WP* prior;
    for (prior = head; prior != NULL && prior->next != wp; prior = prior->next);
    assert(prior != NULL);
    prior->next = wp->next;
  }
  wp->next = free_;
  free_ = wp;
}

WP* changed_wp(Operand *old) {
  WP* wp;
  bool success;
  Operand val;
  for (wp = head; wp != NULL; wp = wp->next) {
    val = expr(wp->expr, &success);
    assert(success);
    if (!operand_equal(&val, &wp->value)) {
      *old = wp->value;
      wp->value = val;
      return wp;
    }
  }
  return NULL;
}

void info_wp() {
  puts("Num\tType\t\tHit\tWhat\t");
  WP* wp;
  for (wp = head; wp != NULL; wp = wp->next) {
    printf("%d\twatchpoint\t%d\t%s\t\n", wp->NO + 1, wp->hit, wp->expr);
  }
}

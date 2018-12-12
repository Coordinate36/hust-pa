#include "monitor/watchpoint.h"
#include "monitor/expr.h"

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
  WP* wp = head;
  if (head == NULL) {
    panic("Watch points number can not exceed %d", NR_WP);
  }
  head = head->next;
  return wp;
}

void free_wp(WP *wp) {
  wp->next = head;
  head = wp;
}

bool is_wp_changed() {
  WP* wp;
  bool success;
  unsigned val;
  for (wp = head; wp != NULL; wp++) {
    val = expr(wp->expr, &success);
    if (val != wp->value) {
      wp->value = val;
      return false;
    }
  }
  return true;
}

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
  if (free_ == NULL) {
    panic("The number of watch points can not exceed 32");
  }
  WP* wp = free_;
  free_ = free_->next;
  wp->next = head;
  head = wp;
  return wp;
}

void free_wp(WP *wp) {
  assert(wp != NULL);
  if (wp == head) {
    head = head->next;
  } else {
    WP* prior;
    for (prior = head; prior->next != wp; prior = prior->next);
    prior->next = wp->next;
  }
  wp->next = free_;
  free_ = wp;
}

bool is_wp_changed() {
  WP* wp;
  bool success;
  unsigned val;
  for (wp = head; wp != NULL; wp = wp->next) {
    val = expr(wp->expr, &success);
    if (val != wp->value) {
      wp->value = val;
      return true;
    }
  }
  return false;
}

#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char expr[256];
  unsigned value;
} WP;

WP* new_wp();
void free_wp(int n);
WP* changed_wp(unsigned* old);

#endif

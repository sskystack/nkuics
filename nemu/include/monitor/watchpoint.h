#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  char expr[256];
  uint32_t old_val;


} WP;

void init_wp_pool();
WP* new_wp(char *expr);
bool free_wp(int no);
void print_wp();
bool check_watchpoints();

#endif

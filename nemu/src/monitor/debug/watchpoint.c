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

WP* new_wp(char *expr) {
  if (expr == NULL || expr[0] == '\0') {
    printf("Usage: w EXPR\n");
    return NULL;
  }

  if (free_ == NULL) {
    printf("No free watchpoint. Max = %d\n", NR_WP);
    return NULL;
  }

  WP *wp = free_;
  free_ = free_->next;

  wp->next = head;
  head = wp;

  strncpy(wp->expr, expr, sizeof(wp->expr) - 1);
  wp->expr[sizeof(wp->expr) - 1] = '\0';
  wp->old_val = 0;

  return wp;
}

bool free_wp(int no) {
  WP *prev = NULL;
  WP *cur = head;

  while (cur != NULL && cur->NO != no) {
    prev = cur;
    cur = cur->next;
  }

  if (cur == NULL) {
    return false;
  }

  if (prev == NULL) {
    head = cur->next;
  } else {
    prev->next = cur->next;
  }

  cur->next = free_;
  free_ = cur;
  return true;
}

void print_wp() {
  WP *cur = head;
  if (cur == NULL) {
    printf("No watchpoints.\n");
    return;
  }

  printf("Num\tWhat\n");
  while (cur != NULL) {
    printf("%d\t%s\n", cur->NO, cur->expr);
    cur = cur->next;
  }
}



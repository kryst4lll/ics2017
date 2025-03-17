#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  struct watchpoint *tail;
  char *expr;              // 监视的表达式
  uint32_t value;
} WP;

WP* new_wp(char* e);
void free_wp(WP* wp);

extern WP *head;
extern WP *free_;

#endif

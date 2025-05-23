#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
    wp_pool[i].tail = NULL;
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
  free_->tail = &wp_pool[NR_WP - 1];
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(char* e){
  printf("create a new wp\n");
  if(free_ != NULL){
    WP* newWp = free_;
    if(free_->next != NULL){
      free_->next->tail = free_->tail;
      free_ = free_->next;
    }else {
      free_ = NULL; 
    }

    newWp->next = NULL;
    // newWp->expr = e;
    strncpy(newWp->expr, e, sizeof(newWp->expr) - 1); // 复制字符串
    newWp->expr[sizeof(newWp->expr) - 1] = '\0'; // 确保字符串以 \0 结尾
    if(head == NULL){
      head = newWp;
      head->tail = newWp;
    }else{
      head->tail->next = newWp;
      head->tail = newWp;
    }
    
    // printf("The head number is: %d\n", head->NO);
    // printf("The head string is: %s\n", head->expr);
    return newWp;
  }
  
  return NULL;
}

void free_wp(WP* wp){
  WP* cur = head;
  if(cur == wp){
    free_->tail->next = wp;
    free_->tail = wp;
    head = cur->next;
    return;
  }
  while(cur != NULL){
    if(cur->next == wp){
      cur->next = cur->next->next;
      free_->tail->next = wp;
      free_->tail = wp;
      return;
    }
    cur = cur->next;
  }
}


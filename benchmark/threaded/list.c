/******************************************************************************
 * File:        list.c                                                        *
 * Description: TODO                                                          *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171001                                                      *
 ******************************************************************************/
#include <stdlib.h>
#include "list.h"

typedef struct node {
  struct node *next;
  void *element;
} *node_t;

struct list {
  node_t head;
  node_t now;
  size_t size;
};

static node_t node_new(void *element);
static void node_free(node_t);

list_t list_new(void) {
  list_t list = (list_t) malloc(sizeof(struct list));
  list->head = node_new(NULL);
  list->head->next = list->head;
  list->now = list->head;
  list->size = 0;
  return list;
}

void list_free(list_t list) {
  list_clear(list);
  free(list->head);
  free(list);
}

void list_clear(list_t list) {
  list_first(list);
  while (list_size(list))
    list_remove(list);
}

void list_insert(list_t list, void *element) {
  node_t node = node_new(element);
  node->next = list->now->next;
  list->now->next = node;
  list->size++;
}


void list_remove(list_t list) {
  if (!list_has_next(list))
    return;

  node_t node = list->now->next;
  list->now->next = node->next;
  list->size--;
  node_free(node);
}

inline void list_first(list_t list) {
  list->now = list->head;
}

inline void list_next(list_t list) {
  if (list_has_next(list))
    list->now = list->now->next;
}

inline void *list_inspect(list_t list) {
  return list_has_next(list) ? list->now->next->element : NULL;
}

inline size_t list_size(list_t list) {
  return list->size;
}

inline int list_has_next(list_t list) {
  return list->now->next != list->head;
}

node_t node_new(void *element) {
  node_t node = (node_t) malloc(sizeof(struct node));
  node->element = element;
  node->next = NULL;
  return node;
}

inline void node_free(node_t node) {
  free(node);
}

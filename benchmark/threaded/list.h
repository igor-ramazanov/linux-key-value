/******************************************************************************
 * File:        list.h                                                        *
 * Description: TODO                                                          *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171001                                                      *
 ******************************************************************************/
#pragma once
#include <stddef.h>

typedef struct list *list_t;

list_t list_new(void);
void list_free(list_t);
void list_first(list_t);
void list_clear(list_t);
void list_insert(list_t, void *);
void list_remove(list_t);
void list_next(list_t);
void *list_inspect(list_t);
size_t list_size(list_t);
int list_has_next(list_t);

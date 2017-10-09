/******************************************************************************
 * File:        entry.h                                                       *
 * Description: TODO                                                          *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20170924                                                      *
 ******************************************************************************/
#pragma once
#include <linux/rcupdate.h>
#include <linux/rhashtable.h>

typedef struct entry {
  char *key;
  void *value;
  int length;
  struct entry __rcu *next;
  struct rhash_head head;
  struct rcu_head rcu;
} *entry_t;

entry_t entry_new(const char *key, const void *value, size_t length);
void entry_free(entry_t);

/******************************************************************************
 * File:        database_entry.h                                              *
 * Description: TODO                                                          *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20170924                                                      *
 ******************************************************************************/
#pragma once
#include <linux/rcupdate.h>
#include <linux/rhashtable.h>

typedef struct database_entry {
  char *key;
  void *data;
  size_t length;
  struct database_entry __rcu *next;
  struct rhash_head rhash;
  struct rcu_head rcu;
} *entry_t;

entry_t entry_new(char *key, char *value, size_t length);
void entry_free(entry_t);

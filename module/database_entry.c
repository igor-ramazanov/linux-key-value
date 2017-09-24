/******************************************************************************
 * File:        database_entry.c                                              *
 * Description: TODO                                                          *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20170924                                                      *
 ******************************************************************************/
#include <linux/slab.h>
#include "database_entry.h"

entry_t database_entry(char *key, char *data, size_t length) {
  entry_t entry = (entry_t) kmalloc(sizeof(struct database_entry), GFP_KERNEL);
  entry->key = key;
  entry->value = value;
  entry->length = length;
  return entry;
}

void entry_free(entry_t entry) {
  kfree(entry->key);
  kfree(entry->value);
  kfree_rcu(entry);
}

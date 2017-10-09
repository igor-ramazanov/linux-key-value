/******************************************************************************
 * File:        entry.c                                                       *
 * Description: TODO                                                          *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20170924                                                      *
 ******************************************************************************/
#include <linux/slab.h>
#include <linux/string.h>
#include "entry.h"

entry_t entry_new(const char *key, const void *value, size_t length) {
  entry_t entry = (entry_t) kmalloc(sizeof(struct entry), GFP_KERNEL);
  entry->key = kstrdup(key, GFP_KERNEL);
  entry->value = kmemdup(value, length, GFP_KERNEL);
  entry->length = length;
  return entry;
}

void entry_free(entry_t entry) {
  if (!entry)
    return;

  kfree(entry->key);
  kfree(entry->value);
  kfree_rcu(entry, rcu);
}

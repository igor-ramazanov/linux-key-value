/*****************************************************************************
 * File:        database.c                                                   *
 * Description:                                                              *
 * Version:                                                                  *
 *****************************************************************************/
#include <net/sock.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/rcupdate.h>
#include <linux/rhashtable.h>
#include <linux/slab.h>
#include "command.h"
#include "database.h"
#include "error_code.h"

/* "Private member functions". */
static void database_rhashtable_cleanup(void *, void *);
static int database_has_key(const char *);
static int database_key_compare(struct rhashtable_compare_arg *, const void *);

struct database {
  struct rhashtable table;
  struct rhashtable_params params;
};

struct database_entry {
  char *key;
  void *data;
  size_t length;
  struct database_entry __rcu *next;
  struct rhash_head rhash;
  struct rcu_head rcu;
};

static const struct rhashtable_params params = {
  .nelem_hint = 1024,
  .head_offset = offsetof(struct database_entry, rhash),
  .key_offset = offsetof(struct database_entry, key),
  .key_len = sizeof(char *),
  .max_size = 1048576,
  .min_size = 256,
  .automatic_shrinking = true,
  .obj_cmpfn = database_key_compare
};

static struct database database;

int database_init(void) {
  /* Initialize the rhashtable. */
  if (rhashtable_init(&database.table, &params))
    return DB_INIT_RHASHTABLE;

  return SUCCESS;
}

/*
 * TODO Persistent storage before exit (maybe called from moddb.c?)
 */
void database_free(void) {
  rhashtable_free_and_destroy(&database.table, database_rhashtable_cleanup, 0);
}

int database_key_compare(struct rhashtable_compare_arg *arg, const void *obj) {
  struct database_entry *entry = (struct database_entry *) obj;
  return strcmp(entry->key, arg->key);
}

void database_rhashtable_cleanup(void *ptr, void *arg) {
  struct database_entry *entry;
  struct database_entry *next;

  entry = (struct database_entry *) ptr;

  while (entry) {
    next = rcu_access_pointer(entry->next);
    kfree_rcu(entry, rcu);
    entry = next;
  }
}

int database_insert(char *key, char *value, size_t length) {
  /* FIXME atomic context required. */

  struct rhash_head *head;
  head = rhashtable_lookup_fast(&database.table, key, params);

  if (!head) {

    /* Insert a new element. */
  } else {
  }
}

inline int database_has_key(const char *key) {
  /* FIXME make sure we hold RCU lock here. */
  /* FIXME this only finds the first element in the list. */
  return (rhashtable_lookup_fast(&database.table, key, params) != NULL);
}

/* TODO actual code here. */
void database_save(void) {
}

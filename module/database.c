/*****************************************************************************
 * File:        database.c                                                   *
 * Description:                                                              *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                *
 * Version:     20170926                                                     *
 *****************************************************************************/
#include <linux/rcupdate.h>
#include <linux/rhashtable.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include "database.h"
#include "entry.h"
#include "error_code.h"

/*
 * FIXME hashing of keys and objects do the same thing, otherwise objects
 *       are not considered to have the same key. Why? Remove one?
 * TODO  RCU protection.
 * TODO  Persistent storage.
 * TODO  Test or remove database_has_key (do we need it?)
 */

/* "Private member functions". */
static void database_rhashtable_cleanup(void *, void *);
static int database_key_compare(struct rhashtable_compare_arg *, const void *);
static u32 database_key_hash(const void *, u32, u32);
static u32 database_obj_hash(const void *, u32, u32);

/*
 * The actual database object.
 */
static struct database {
  struct rhashtable table;
  struct rhashtable_params params;
  spinlock_t lock;
} database;

/*
 * This object is used to set up and make insertsions into the rhashtable.
 */ 
static const struct rhashtable_params params = {
  .head_offset = offsetof(struct entry, head),
  .key_offset = offsetof(struct entry, key),
  .key_len = sizeof(char *),
  .hashfn = database_key_hash,
  .obj_hashfn = database_obj_hash,
  .obj_cmpfn = database_key_compare
};

int database_init(void) {

  /* Initialize the rhashtable. */
  if (rhashtable_init(&database.table, &params))
    return DB_INIT_RHASHTABLE;

  spin_lock_init(&database.lock);
  return SUCCESS;
}

void database_free(void) {
  rhashtable_free_and_destroy(&database.table, database_rhashtable_cleanup, 0);
}

/*
 * Removes mappings from the hashtable.
 */
void database_rhashtable_cleanup(void *ptr, void *arg) {
  entry_t entry = (entry_t) ptr;
  entry_free(entry);
}

/*
 * Compares two keys.
 */
int database_key_compare(struct rhashtable_compare_arg *arg, const void *obj) {
  entry_t entry = (entry_t) obj;
  return strcmp(entry->key, arg->key);
}

/*
 * Custom hash function to compare cstring keys.
 */
u32 database_key_hash(const void *data, u32 len, u32 seed) {
  const char *key = (const char *) data;
  return jhash(key, strlen(key), seed);
}

/*
 * Custom hash function to compare cstring keys.
 */
u32 database_obj_hash(const void *data, u32 len, u32 seed) {
  entry_t entry = (entry_t) data;
  return jhash(entry->key, strlen(entry->key), seed);
}

/*
 * TODO RCU protect the replaced element?
 */
int database_insert(char *key, char *value, size_t length) {
  entry_t old_entry;
  entry_t new_entry;
  int err = 0;

  /* Create the node to insert. */
  new_entry = entry_new(key, value, length);
  if (!new_entry)
	  return -ENOMEM;

  spin_lock(&database.lock);

  /* Replace the old mapping or create a new if none exists. */
  old_entry = (entry_t) rhashtable_lookup_fast(&database.table, key, params);
  if (old_entry) {
    err = rhashtable_replace_fast(&database.table, &old_entry->head, &new_entry->head, params);
    database_rhashtable_cleanup(old_entry, NULL);
  } else {
    err = rhashtable_insert_fast(&database.table, &new_entry->head, params);
  }

  if (err) {
    entry_free(new_entry);
    printk(KERN_WARNING "moddb: Insertion failed.\n");
  }

  spin_unlock(&database.lock);
  return err;
}

int database_lookup(char *key, char **value, size_t *length) {
  entry_t entry;

  spin_lock(&database.lock);

  entry = rhashtable_lookup_fast(&database.table, key, params);
  if (entry) {
    *value = entry->value;
    *length = entry->length;
    spin_unlock(&database.lock);
    return 0;
  }

  spin_unlock(&database.lock);
  return 1;
}

/* TODO actual code here. */
int database_save(void) {
  return 0;
}

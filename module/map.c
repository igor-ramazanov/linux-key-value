/*****************************************************************************
 * File:        map.c                                                        *
 * Description:                                                              *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                *
 *              Igor Ramazanov <ens17irm@cs.umu.se>                          *
 * Version:     20171006                                                     *
 *****************************************************************************/
#include <linux/rcupdate.h>
#include <linux/rhashtable.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include "entry.h"
#include "logger.h"
#include "map.h"

/*
 * TODO move entry here instead - should be a "memeber class"
 * TODO RCU protection.
 * TODO Persistent storage.
 */

/* "Private member functions". */
static void map_remove_entry(void *, void *);
static int map_key_compare(struct rhashtable_compare_arg *, const void *);
static u32 map_key_hash(const void *, u32, u32);
static u32 map_obj_hash(const void *, u32, u32);

/*
 * The actual database object.
 */
static DEFINE_RWLOCK(lock);
static struct map {
  struct rhashtable table;
  struct rhashtable_params params;
} map;

/*
 * This object is used to set up and make insertsions into the rhashtable.
 */ 
static const struct rhashtable_params params = {
  .head_offset = offsetof(struct entry, head),
  .key_offset = offsetof(struct entry, key),
  .key_len = sizeof(char *),
  .hashfn = map_key_hash,
  .obj_hashfn = map_obj_hash,
  .obj_cmpfn = map_key_compare
};

int map_init(void) {

  /* Initialize the rhashtable. */
  if (rhashtable_init(&map.table, &params))
    return MAP_INIT_FAILED;

  return MAP_INIT_SUCCESS;
}

void map_destroy(void) {
  rhashtable_free_and_destroy(&map.table, map_remove_entry, 0);
}

/*
 * Removes mappings from the hashtable.
 */
void map_remove_entry(void *ptr, void *arg) {
  entry_free((entry_t) ptr);
}

/*
 * Compares two keys.
 */
int map_key_compare(struct rhashtable_compare_arg *arg, const void *obj) {
  entry_t entry = (entry_t) obj;
  return strcmp(entry->key, arg->key);
}

/*
 * Custom hash function to compare cstring keys.
 */
u32 map_key_hash(const void *data, u32 len, u32 seed) {
  const char *key = (const char *) data;
  return jhash(key, strlen(key), seed);
}

/*
 * Custom hash function to compare cstring keys.
 */
u32 map_obj_hash(const void *data, u32 len, u32 seed) {
  entry_t entry = (entry_t) data;
  return jhash(entry->key, strlen(entry->key), seed);
}

/*
 * TODO RCU protect the replaced element?
 */
int map_insert(const char *key, const void *value, size_t length) {
  entry_t old_entry;
  entry_t new_entry;
  int err = MAP_INSERT_SUCCESS;

  /* Create the node to insert. */
  new_entry = entry_new(key, value, length);
  if (!new_entry)
	  return MAP_INSERT_FAILED;

  write_lock(&lock);

  /* Replace the old mapping or create a new if none exists. */
  old_entry = (entry_t) rhashtable_lookup_fast(&map.table, key, params);
  if (old_entry && !rhashtable_replace_fast(&map.table, &old_entry->head,
      &new_entry->head, params)) {

    /* Replace an old entry. */
    map_remove_entry(old_entry, NULL);
    err = MAP_INSERT_REPLACED;
  } else if (!old_entry) {

    /* Insert a new entry. */
    err = rhashtable_insert_fast(&map.table, &new_entry->head, params)
        ? MAP_INSERT_FAILED
        : MAP_INSERT_SUCCESS;
  } else {

    /* This should never happen. */
    logger_error("a mapping exists! ...but it also doesn't.\n");
    err = MAP_INSERT_FAILED;
  }

  if (err == MAP_INSERT_FAILED)
    entry_free(new_entry);

  write_unlock(&lock);
  return err;
}

int map_lookup(const char *key, void **value, size_t *length) {
  entry_t entry;

  read_lock(&lock);

  entry = rhashtable_lookup_fast(&map.table, key, params);
  if (!entry) {
    read_unlock(&lock);
    return MAP_LOOKUP_FAILED;
  }

  *value = entry->value;
  *length = entry->length;
  read_unlock(&lock);
  return MAP_LOOKUP_SUCCESS;
}

/* TODO actual code here. */
int map_save(void) {
  return 0;
}

/******************************************************************************
 * File:        hashtable.c                                                   *
 * Description: TODO                                                          *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171001                                                      *
 ******************************************************************************/
#include <stdlib.h>
#include "hashtable.h"
#include "list.h"

typedef struct entry {
  void *key;
  void *element;
} *entry_t;

struct hashtable {
  list_t *buckets;
  hashfn hash;
  keycmp cmp;
  size_t size;
  size_t capacity;
  size_t lomark;
  size_t himark;
  float loadfactor;

  /* used for iteration. */
  entry_t entry;
  size_t bucket;
  size_t index;
};

static entry_t entry_new(void *key, void *element);
static void entry_free(entry_t);
static void hashtable_resize(hashtable_t, size_t capacity);

static const size_t INITIAL_CAPACITY = 16;

hashtable_t hashtable_new(hashfn hash, keycmp cmp, float loadfactor) {
  size_t i;

  hashtable_t table = (hashtable_t) malloc(sizeof(struct hashtable));
  table->hash = hash;
  table->cmp = cmp;
  table->loadfactor = loadfactor;
  table->capacity = INITIAL_CAPACITY;
  table->himark = (size_t) (loadfactor * INITIAL_CAPACITY);
  table->lomark = table->himark / 4;
  table->size = 0;
  table->index = 0;
  table->bucket = 0;
  table->entry = NULL;

  table->buckets = (list_t *) malloc(INITIAL_CAPACITY * sizeof(list_t));
  for (i = 0; i < INITIAL_CAPACITY; i++)
    table->buckets[i] = list_new();

  return table;
}

void hashtable_free(hashtable_t table) {
  size_t i;

  /* Free the buckets. */
  for (i = 0; i < table->capacity; i++) {
    list_t bucket = table->buckets[i];

    /* Free the elements in the bucket. */
    list_first(bucket);
    while (list_has_next(bucket)) {
      entry_free((entry_t) list_inspect(bucket));
      list_remove(bucket);
    }

    list_free(bucket);
  }

  free(table->buckets);
  free(table);
}

/*
 * This function returns a pointer to the element replaced by the insertion,
 * or NULL if the element was the first entry with this key.
 */
void *hashtable_insert(hashtable_t table, void *key, void *element) {
  list_t bucket;
  size_t index;
  int found;
  void *replaced = NULL;

  /* Make room if the table has become too crowded. */
  if (table->size == table->himark)
    hashtable_resize(table, table->capacity * 2);

  /* Get the bucket. */
  index = table->hash(key) % table->capacity;
  bucket = table->buckets[index];
  found = 0;

  /* Look for a previously inserted element with the same key. */
  list_first(bucket);
  while (list_has_next(bucket)) {
    entry_t entry = (entry_t) list_inspect(bucket);
    list_next(bucket);

    /* Replace the element if the key was found. */
    if (!table->cmp(key, entry->key)) {
      replaced = entry->element;
      entry->element = element;
      found = 1;
      break;
    }
  }

  /* Insert the new element. */
  if (!found) {
    list_first(bucket);
    list_insert(bucket, entry_new(key, element));
    table->size++;
  }

  return replaced;
}

void *hashtable_remove(hashtable_t table, const void *key) {
  list_t bucket;
  size_t index;
  void *element = NULL;

  /*  Get the bucket. */
  index = table->hash(key) % table->capacity;
  bucket = table->buckets[index];

  /* Find the key in the bucket. */
  list_first(bucket);
  while (list_has_next(bucket)) {
    entry_t entry = (entry_t) list_inspect(bucket);
    list_next(bucket);

    /* If the key was found - remove the entry. */
    if (!table->cmp(key, entry->key)) {
      list_remove(bucket);
      element = entry->element;
      entry_free(entry);
      break;
    }
  }

  /* Check if the hashtable is too sparsely populated. */
  if ((table->size < table->lomark) && (table->capacity > INITIAL_CAPACITY))
    hashtable_resize(table, table->capacity / 2);

  return element;
}

void *hashtable_lookup(hashtable_t table, const void *key) {
  list_t bucket;
  size_t index;

  /* Get the bucket. */
  index = table->hash(key) % table->capacity;
  bucket = table->buckets[index];

  /* Look for the key. */
  list_first(bucket);
  while (list_has_next(bucket)) {
    entry_t entry = (entry_t) list_inspect(bucket);
    list_next(bucket);

    /* The element was found. */
    if (!table->cmp(key, entry->key))
      return entry->element;
  }

  /* No element found. */
  return NULL;
}

inline size_t hashtable_size(hashtable_t table) {
  return table->size;
}

void hashtable_begin(hashtable_t table) {
  table->index = 0;
  table->entry = NULL;

  /* Find the first non-empty bucket. */
  for (table->bucket = 0; table->bucket < table->capacity; table->bucket++) {
    list_t bucket = table->buckets[table->bucket];
    if (list_size(bucket)) {
      list_first(bucket);
      table->entry = (entry_t) list_inspect(bucket);
      list_next(bucket);
      break;
    }
  }
}

inline int hashtable_has_next(hashtable_t table) {
  return table->index < table->size;
}

void *hashtable_key(hashtable_t table) {
  return table->entry ? table->entry->key : NULL;
}

void *hashtable_value(hashtable_t table) {
  return table->entry ? table->entry->element : NULL;
}

void hashtable_next(hashtable_t table) {
  list_t bucket;

  if ((table->index + 1) == table->size) {
    table->index++;
    return;
  }

  /* Find the non-empty bucket. */
  bucket = table->buckets[table->bucket];
  while (!list_has_next(bucket)) {
    table->bucket++;
    bucket = table->buckets[table->bucket];
    list_first(bucket);
  }

  /* Get the next entry. */
  table->entry = list_inspect(bucket);
  list_next(bucket);
  table->index++;
}

void hashtable_resize(hashtable_t table, size_t capacity) {
  list_t *buckets;
  size_t length;
  size_t i;

  /* Replace the previous array of buckets. */
  buckets = table->buckets;
  table->buckets = (list_t *) malloc(capacity * sizeof(list_t));
  for (i = 0; i < capacity; i++)
    table->buckets[i] = list_new();

  /* Update the table variables. */
  length = table->capacity;
  table->capacity = capacity;
  table->himark = (size_t) (table->loadfactor * capacity);
  table->lomark = table->himark / 4;

  /* Insert the old entries into the new buckets. */
  for (i = 0; i < length; i++) {
    list_t bucket = buckets[i];

    /* Move the element from the old bucket to some new. */
    list_first(bucket);
    while (list_has_next(bucket)) {
      entry_t entry = (entry_t) list_inspect(bucket);
      hashtable_insert(table, entry->key, entry->element);
      entry_free(entry);
      list_remove(bucket);
    }

    /* No need for this anymore. */
    list_free(bucket);
  }

  /* Finally, remove the old bucket array. */
  free(buckets);
}

entry_t entry_new(void *key, void *element) {
  entry_t entry = (entry_t) malloc(sizeof(struct entry));
  entry->key = key;
  entry->element = element;
  return entry;
}

inline void entry_free(entry_t entry) {
  free(entry);
}

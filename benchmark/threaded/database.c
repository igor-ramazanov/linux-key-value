/******************************************************************************
 * File:        database.c                                                    *
 * Description: TODO                                                          *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171001                                                      *
 ******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include "hashtable.h"

typedef struct value {
  char *data;
  size_t length;
} *value_t;

struct database {
  hashtable_t table;
};

static size_t key_hash(const void *);
static int key_cmp(const void *, const void *);
static value_t value_new(char *data, size_t length);
static void value_free(value_t);

database_t database_new(void) {
  database_t db = (database_t) malloc(sizeof(struct database));
  db->table = hashtable_new(key_hash, key_cmp, 0.75f);
  return db;
}

void database_free(database_t db) {

  /* Free the values. */
  hashtable_begin(db->table);
  while (hashtable_has_next(db->table)) {
    value_free(hashtable_value(db->table));
    hashtable_next(db->table);
  }

  /* Free everything else. */
  hashtable_free(db->table);
  free(db);
}

void database_set(database_t db, char *key, char *val, size_t len){
  value_free(hashtable_insert(db->table, key, value_new(val, len)));
}

void database_get(database_t db, char *key, char **val, size_t *len) {
  value_t entry = hashtable_lookup(db->table, key);

  if (entry == NULL) {
    *val = NULL;
    *len = 0;
    return;
  }

  *val = entry->data;
  *len = entry->length;
}

inline size_t database_size(database_t db) {
  return hashtable_size(db->table);
}

static int key_cmp(const void *e1, const void *e2) {
  return strcmp((const char *) e1, (const char *) e2);
}

/* Probably not a good hash function. */
static size_t key_hash(const void *e) {
  size_t sum;
  int length;
  int i;

  length = strlen((const char *) e);
  sum = 0;

  for (i = 0; i < length; i++)
    sum += (size_t) ((const char *) e)[i];

  return sum;
}

value_t value_new(char *val, size_t len) {
  value_t value = (value_t) malloc(sizeof(struct value));
  value->data = val;
  value->length = len;
  return value;
}

void value_free(value_t value) {
  free(value);
}

/******************************************************************************
 * File:        hashtable.h                                                   *
 * Description: TODO                                                          *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171001                                                      *
 ******************************************************************************/
#pragma once
#include <stddef.h>

typedef size_t (*hashfn)(const void *);
typedef int (*keycmp)(const void *, const void *);

typedef struct hashtable *hashtable_t;

hashtable_t hashtable_new(hashfn hash, keycmp cmp, float loadfactor);
void hashtable_free(hashtable_t);
void *hashtable_insert(hashtable_t, void *key, void *value);
void *hashtable_remove(hashtable_t, const void *key);
void *hashtable_lookup(hashtable_t, const void *key);
size_t hashtable_size(hashtable_t);

/* For iterating over the values in the hashtable. */
void hashtable_begin(hashtable_t);
void hashtable_next(hashtable_t);
void *hashtable_key(hashtable_t);
void *hashtable_value(hashtable_t);
int hashtable_has_next(hashtable_t);

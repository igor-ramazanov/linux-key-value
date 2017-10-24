/******************************************************************************
 * File:        test.c                                                        *
 * Description: TODO                                                          *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171001                                                      *
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"


static const char *KEYS[5] = { "key1", "key2", "key3", "key4", "key5" };

static char **strings;
static database_t db;

int main(int argc, char **argv) {
  int i;

  db = database_new();

  strings = (char **) malloc(5 * sizeof(char *));
  for (i = 0; i < 5; i++) {
    strings[i] = (char *) malloc(5);
    strcpy(strings[i], KEYS[i]);
  }

  for (i = 0; i < 5; i++)
    database_set(db, strings[i], (char *) i, sizeof(char *));

  database_print(db);

  for (i = 0; i < 5; i++)
    free(strings[i]);
  free(strings);
  
  
  database_free(db);
  return 0;
}

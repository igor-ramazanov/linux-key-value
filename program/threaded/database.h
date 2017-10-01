/******************************************************************************
 * File:        database.h                                                    *
 * Description: TODO                                                          *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171001                                                      *
 ******************************************************************************/
#pragma once

typedef struct database *database_t;

database_t database_new(void);
void database_free(database_t);
void database_set(database_t, char *key, char *val, size_t len);
void database_get(database_t, char *key, char **val, size_t *len);
size_t database_size(database_t);

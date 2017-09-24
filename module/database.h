/*****************************************************************************
 * File:        database.h                                                   *
 * Description:                                                              *
 * Version:                                                                  *
 *****************************************************************************/
#pragma once

int database_init(void);
void database_free(void);
int database_save(void);
int database_has_key(const char *key);
int database_insert(char *key, char *data, size_t length);
int database_lookup(char *key, char **data, size_t *length);

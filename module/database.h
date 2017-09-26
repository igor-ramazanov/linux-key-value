/*****************************************************************************
 * File:        database.h                                                   *
 * Description:                                                              *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                *
 * Version:     20170926                                                     *
 *****************************************************************************/
#pragma once

int database_init(void);
void database_free(void);
int database_save(void);
int database_insert(char *key, char *data, size_t length);
int database_lookup(char *key, char **data, size_t *length);

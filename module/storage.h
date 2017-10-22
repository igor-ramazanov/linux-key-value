/******************************************************************************
 * File:        storage.h                                                     *
 * Description: TODO                                                          *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171022                                                      *
 ******************************************************************************/
#pragma once
#include <stddef.h>

int storage_init(void (*callback)(const char *, const void *, size_t));
void storage_exit(void);
int storage_load(void);
int storage_save(const char *key, const void *data, size_t length);

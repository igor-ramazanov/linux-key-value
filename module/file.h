/******************************************************************************
 * File:        file.h                                                        *
 * Description: TODO                                                          *
 * Author:      Igor Ramazanov <ens17irv@cs.umu.se>                           *
 *              Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171022                                                      *
 ******************************************************************************/
#pragma once
#include <linux/types.h>

typedef struct file *file_t;

file_t file_open(const char *path, int flags, int rights);
int file_exists(const char *path, int flags, int rights);
void file_close(file_t file);
int file_sync(file_t file);
ssize_t file_write(file_t file, const char *buf, size_t buflen, loff_t pos);
ssize_t file_read(file_t file, char *buf, size_t buflen, loff_t pos);

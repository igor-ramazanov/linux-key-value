/******************************************************************************
 * File:        storage.c                                                     *
 * Description: TODO                                                          *
 * Author:      Igor Ramazanov <ens17irv@cs.umu.se>                           *
 *              Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171022                                                      *
 ******************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include "file.h"
#include "logger.h"
#include "storage.h"

#define DIRECTORY "/var/tmp/shared_map"

static int storage_process_file(struct dir_context *ctx, const char *, int,
    loff_t, u64, unsigned int);

static struct storage {
  void (*callback)(const char *, const void *, size_t);
} storage;

int storage_init(void (*callback)(const char *, const void *, size_t)) {

  if (!file_exists(DIRECTORY, O_DIRECTORY | O_RDONLY, 0777)) {
    logger_error("directory %s does not exist\n", DIRECTORY);
    return -1;
  }

  storage.callback = callback;
  return 0;
}

void storage_exit(void) {
}

int storage_load(void) {
  struct dir_context dir_context = { storage_process_file, 0 };
  
  file_t file = file_open(DIRECTORY, O_RDONLY, 0);
  if (!file || iterate_dir(file, &dir_context)) {
    logger_error("could not list files in %s\n", DIRECTORY);
    return -1;
  }

  return 0;
}

int storage_save(const char *key, const void *value, size_t length) {
  size_t path_length;
  size_t llength;
  char *filepath;
  file_t file;

  path_length = strlen(DIRECTORY) + strlen(key) + 2;
  filepath = (char *) kmalloc(path_length, GFP_KERNEL);
  if (!filepath) {
    logger_error("out of memory?\n");
    return -1;
  }

  sprintf(filepath, "%s/%s", DIRECTORY, key);
  file = file_open(filepath, O_WRONLY | O_CREAT, 0644);
  if (!file) {
    logger_error("could not open file: %s\n", filepath);
    return -1;
  }

  llength = sizeof(size_t);
  if ((file_write(file, (char *) &length, llength, 0) != llength)
      || (file_write(file, value, length, llength) != length)) {
    logger_error("could not write to file %s\n", filepath);
    file_close(file);
    return -1;
  }

  file_close(file);
  return 0;
}

int storage_process_file(struct dir_context *ctx, const char *name, int namlen,
    loff_t offset, u64 ino, unsigned int d_type) {
  size_t value_length;
  size_t llength;
  char *filepath;
  char *value;
  file_t file;

  if (d_type != DT_REG)
    return 0;

  filepath = (char *) kmalloc(strlen(DIRECTORY) + strlen(name) + 2, GFP_KERNEL);
  if (!filepath) {
    logger_error("out of memory?\n");
    return -1;
  }

  sprintf(filepath, "%s/%s", DIRECTORY, name);
  file = file_open(filepath, O_RDONLY, 0);
  if (!file) {
    logger_error("could not open file: %s\n", filepath);
    return -1;
  }

  llength = sizeof(size_t);
  if (file_read(file, (char *) &value_length, llength, 0) != llength) {
    logger_error("failed to read from file %s\n", filepath);
    file_close(file);
    return -1;
  }

  value = kmalloc(value_length, GFP_KERNEL);
  if (!value) {
    logger_error("out of memory?\n");
    return -1;
  }

  if (file_read(file, value, value_length, llength) != value_length) {
    logger_error("failed to read from file %s\n", filepath);
    file_close(file);
    return -1;
  }

  file_close(file);
  storage.callback(name, value, value_length);
  kfree(value);
  kfree(filepath);

  return 0;
}

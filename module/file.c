/******************************************************************************
 * File:        file.c                                                        *
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

file_t file_open(const char *path, int flags, int rights) {
  mm_segment_t oldfs;
  file_t file;

  oldfs = get_fs();
  set_fs(get_ds());

  file = filp_open(path, flags, rights);
  set_fs(oldfs);

  return IS_ERR(file) ? NULL : file;
}

inline int file_exists(const char *path, int flags, int rights) {
  file_t file = file_open(path, flags, rights);
  file_close(file);
  return file != NULL;
}

inline void file_close(file_t file) {
  if (file)
    filp_close(file, NULL);
}

inline int file_sync(file_t file) {
  return vfs_fsync(file, 0);
}

ssize_t file_read(file_t file, char *buffer, size_t buflen, loff_t offset) {
  mm_segment_t oldfs;
  ssize_t ret;

  oldfs = get_fs();
  set_fs(get_ds());

  ret = vfs_read(file, buffer, buflen, &offset);
  set_fs(oldfs);

  return ret;
}

ssize_t file_write(file_t file, const char *buffer, size_t buflen,
    loff_t offset) {
  mm_segment_t oldfs;
  ssize_t ret;

  oldfs = get_fs();
  set_fs(get_ds());

  ret = vfs_write(file, buffer, buflen, &offset);
  set_fs(oldfs);

  return ret;
}

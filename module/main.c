/*****************************************************************************
 * File:        moddb.c                                                      *
 * Description:                                                              *
 * Version:                                                                  *
 *****************************************************************************/
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include "database.h"

// @formatter:off
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Shared key-value pair database.");
MODULE_AUTHOR("Erik Ramos <c03ers@cs.umu.se>");
MODULE_AUTHOR("Igor Ramazanov <ens17irv@cs.umu.se>");
MODULE_AUTHOR("Harendarczyk Bastien<ens17bhk@cs.umu.se>");
MODULE_VERSION("0.1");
// @formatter:on

static int __init moddb_init(void) {
  int err = database_init();

  switch (err) {
  case DB_INIT_SUCCESS:
    printk(KERN_ALERT "moddb: Module successfull installed\n");
    break;
  case DB_INIT_NETLINK:
    printk(KERN_ALERT "moddb: Failed to initialize netlink\n");
    break;
  case DB_INIT_RHASHTABLE:
    printk(KERN_ALERT "moddb: Failed to initialize rhashtable\n");
    break;
  default:
    printk(KERN_ALERT "moddb: Failed to install module - unknown cause\n");
    break;
  }

  return err;
}

static void __exit moddb_exit(void) {
  database_save();
  database_free();
  printk(KERN_ALERT "moddb: Module uninstalled\n");
}

module_init(moddb_init);
module_exit(moddb_exit);

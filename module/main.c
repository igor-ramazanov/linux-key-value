/*****************************************************************************
 * File:        moddb.c                                                      *
 * Description:                                                              *
 * Version:                                                                  *
 *****************************************************************************/
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include "database.h"
#include "error_code.h"
#include "adaptor.h"

// @formatter:off
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Shared key-value pair database.");
MODULE_AUTHOR("Erik Ramos <c03ers@cs.umu.se>");
MODULE_AUTHOR("Igor Ramazanov <ens17irv@cs.umu.se>");
MODULE_AUTHOR("Harendarczyk Bastien<ens17bhk@cs.umu.se>");
MODULE_VERSION("0.1");
// @formatter:on

static int __init moddb_init(void) {
  int adaptor_err = adaptor_init();
  if (adaptor_err == ADAPTOR_INIT_NETLINK) {
    printk(KERN_ALERT "moddb: Failed to initialize netlink\n");
    return adaptor_err;
  } else if (adaptor_err == SUCCESS) {
    printk(KERN_ALERT "moddb: Netlink was successfully initialized\n");
  }

  int db_err = database_init();
  if (db_err == DB_INIT_RHASHTABLE) {
      printk(KERN_ALERT "moddb: Failed to initialize rhashtable\n");
      return db_err;
  } else if (db_err == SUCCESS) {
      printk(KERN_ALERT "moddb: Database was successfully initialized\n");
  }

  return SUCCESS;
}

static void __exit moddb_exit(void) {
  adaptor_free();
  database_save();
  database_free();
  printk(KERN_ALERT "moddb: Module uninstalled\n");
}

module_init(moddb_init);
module_exit(moddb_exit);

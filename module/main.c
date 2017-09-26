/*****************************************************************************
 * File:        moddb.c                                                      *
 * Description:                                                              *
 * Version:     20170926                                                     *
 *****************************************************************************/
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include "database.h"
#include "error_code.h"
#include "adaptor.h"
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
  int adaptor_err;
  int db_err;
 
  adaptor_err = adaptor_init();
  if (adaptor_err == ADAPTOR_INIT_NETLINK) {
    printk(KERN_ALERT "moddb: Failed to initialize netlink\n");
    return adaptor_err;
  }

  db_err = database_init();
  if (db_err == DB_INIT_RHASHTABLE) {
    printk(KERN_ALERT "moddb: Failed to initialize rhashtable\n");
    return db_err;
  }

  printk(KERN_INFO "moddb: Module successfully installed\n");
  return SUCCESS;
}

static void __exit moddb_exit(void) {
  adaptor_free();
  database_save();
  database_free();
  printk(KERN_INFO "moddb: Module removed\n");
}

module_init(moddb_init);
module_exit(moddb_exit);

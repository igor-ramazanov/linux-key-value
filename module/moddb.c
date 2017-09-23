#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPLv2");
MODULE_DESCRIPTION("Shared key-value pair database.");
MODULE_AUTHOR("Erik Ramos <c03ers@cs.umu.se>");
MODULE_AUTHOR("Igor Ramazanov <ens17irv@cs.umu.se>");
MODULE_AUTHOR("Harendarczyk Bastien<ens17bhk@cs.umu.se>");
MODULE_VERSION("0.1");

static int __init database_init(void)
{
  printk(KERN_DEBUG "moddb: module installed\n");
  return 0;
}

static void __exit database_exit(void)
{
  printk(KERN_DEBUG "moddb: module removed\n");
}

module_init(database_init);
module_exit(database_exit);

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include "command.c"

MODULE_LICENSE("GPLv2");
MODULE_DESCRIPTION("Shared key-value pair database.");
MODULE_AUTHOR("Erik Ramos <c03ers@cs.umu.se>");
MODULE_AUTHOR("Igor Ramazanov <ens17irv@cs.umu.se>");
MODULE_AUTHOR("Harendarczyk Bastien<ens17bhk@cs.umu.se>");
MODULE_VERSION("0.1");

static int __init database_init(void)
{
    struct Command command;
    command.operation = 1;
    char * key = "test_key";
    char * value = "test_value";
    command.key = key;
    command.value = value;
    command.key_size = strlen(key);
    command.value_size = strlen(value);

    char * serialized = serialise(&command);
    struct Command deserialised = deserialise(serialized);
    printk(KERN_EMERG "Operation: %d", deserialised.operation);
    printk(KERN_EMERG "Key size: %d", deserialised.key_size);
    printk(KERN_EMERG "Key: %s", deserialised.key);
    printk(KERN_EMERG "Value size: %d", deserialised.value_size);
    printk(KERN_EMERG "Value: %s", deserialised.value);
    return 0;
}

static void __exit database_exit(void)
{
    printk(KERN_DEBUG "moddb: module removed");
}

module_init(database_init);
module_exit(database_exit);


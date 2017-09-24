#include <crypto/hash.h>
#include <crypto/sha.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rhashtable.h>
#include <linux/string.h>
#include <linux/rcupdate.h>
#include "command.c"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Shared key-value pair database.");
MODULE_AUTHOR("Erik Ramos <c03ers@cs.umu.se>");
MODULE_AUTHOR("Igor Ramazanov <ens17irv@cs.umu.se>");
MODULE_AUTHOR("Harendarczyk Bastien<ens17bhk@cs.umu.se>");
MODULE_VERSION("0.1");

struct db_entry {
	char *key;
	void *value;
	size_t length;
	struct rhash_head head;
	struct db_entry __rcu *next;
	struct rcu_head rcu;
};

static int db_key_compare(struct rhashtable_compare_arg *arg, const void *obj);
static void db_cleanup(void *ptr, void *arg);

static struct rhashtable table;
static const struct rhashtable_params params = {
	.nelem_hint = 1024,
	.head_offset = offsetof(struct db_entry, head),
	.key_offset = offsetof(struct db_entry, key),
	.key_len = sizeof(char *),
	.max_size = 1048576,
	.min_size = 256,
	.automatic_shrinking = true,
	.obj_cmpfn = db_key_compare
};

static int __init database_init(void)
{


	rhashtable_init(&table, &params);
	rhashtable_free_and_destroy(&table, db_cleanup, NULL);

	/*
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

	test_hash(test_string, strlen(test_string), hash_result);
	print_hash(hash_result);
*/

    return 0;
}

static void __exit database_exit(void)
{
    printk(KERN_DEBUG "moddb: module removed");
}

static int test_hash(const char *string, int length, char *result)
{
	struct shash_desc *desc = kmalloc(sizeof(struct shash_desc), GFP_KERNEL);
	if (!desc) {
		printk(KERN_ERR "moddb: Out of memory?");
		return 1;
	}

	desc->tfm = crypto_alloc_shash("sha256", CRYPTO_ALG_TYPE_SHASH, 0);
	desc->flags = CRYPTO_TFM_REQ_MAY_SLEEP;
	if (IS_ERR(desc->tfm)) {
		printk(KERN_ERR "moddb: failed to allocate tfm");
		kfree(desc);
		return 1;
	}

	memset(result, 0, SHA256_DIGEST_SIZE);
	crypto_shash_init(desc);
	crypto_shash_update(desc, string, length);
	crypto_shash_final(desc, result);

	crypto_free_shash(desc->tfm);
	kfree(desc);
	return 0;
}

static void print_hash(const char *hash)
{
	char string[2 * SHA256_DIGEST_SIZE + 1];
	int i;

	memset(string, 0, 2 * SHA256_DIGEST_SIZE + 1);
	for (i = 0; i < SHA256_DIGEST_SIZE; i++)
		snprintf(string + 2 * i, 2, "%02x", hash[i]);

	printk(KERN_EMERG "0x%s\n", string);
}

int db_key_compare(struct rhashtable_compare_arg *arg, const void *obj)
{
	const struct db_entry *entry = (const struct db_entry *) obj;
	return strcmp((const char *) arg->key, entry->key);
}

void db_cleanup(void *ptr, void *arg)
{
	struct db_entry *entry;
	struct db_entry *next;
	
	entry = (struct db_entry *) ptr;

	while (entry) {
		next = rcu_access_pointer(entry->next);
		kfree_rcu(entry, rcu);
		entry = next;
	}
}

module_init(database_init);
module_exit(database_exit);

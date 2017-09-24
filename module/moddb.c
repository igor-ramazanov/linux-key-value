#include <crypto/hash.h>
#include <crypto/sha.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <net/sock.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/rcupdate.h>
#include <linux/rhashtable.h>
#include <linux/string.h>
#include <linux/slab.h>
#include "command.c"

#ifndef NETLINK_USER
#define NETLINK_USER 31
#endif

struct db_entry {
	char *key;
	void *value;
	size_t length;
	struct rhash_head head;
	struct db_entry __rcu *next;
	struct rcu_head rcu;
};

// @formatter:off
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Shared key-value pair database.");
MODULE_AUTHOR("Erik Ramos <c03ers@cs.umu.se>");
MODULE_AUTHOR("Igor Ramazanov <ens17irv@cs.umu.se>");
MODULE_AUTHOR("Harendarczyk Bastien<ens17bhk@cs.umu.se>");
MODULE_VERSION("0.1");
// @formatter:on


static int db_key_compare(struct rhashtable_compare_arg *arg, const void *obj);
static void db_cleanup(void *ptr, void *arg);

static struct sock *nlsk = NULL;
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

static void proceed(char *payload, int len) {
  struct command *a = kmalloc(len, GFP_KERNEL);
  deserialise_command(a, payload);

  printk(KERN_EMERG "Payload size: %d", len);
  printk(KERN_EMERG "Operation: %d", a->operation);
  printk(KERN_EMERG "Key size: %d", a->key_size);
  printk(KERN_EMERG "Key: %s", a->key);
  printk(KERN_EMERG "Value size: %d", a->value_size);
  printk(KERN_EMERG "Value: %s", a->value);
}

static void nl_callback(struct sk_buff *skb) {
  struct nlmsghdr *nlh;
  int pid;
  struct sk_buff *skb_out;
  int msg_size;

  struct command command;
  command.operation = 1;
  command.key = "key_from_kernel";
  command.value = "value_from_kernel";
  command.key_size = strlen(command.key) + 1;
  command.value_size = strlen(command.value) + 1;
  msg_size = sizeof(char) * (1 + command.value_size + command.key_size) + sizeof(int) * 2;
  char *serialised = kmalloc(msg_size, GFP_KERNEL);
  serialise_command(&command, serialised);

  char *payload;
  int len;

  nlh = (struct nlmsghdr *) skb->data;
  payload = (char *) nlmsg_data(nlh);
  len = NLMSG_PAYLOAD(nlh, 0);
  proceed(payload, len);
  pid = nlh->nlmsg_pid; /*pid of sending process */

  skb_out = nlmsg_new(msg_size, 0);

  nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
  NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
  strncpy(nlmsg_data(nlh), serialised, msg_size);

  nlmsg_unicast(nlsk, skb_out, pid);
}

static int __init database_init(void)
{
  struct netlink_kernel_cfg cfg = {
          .input = nl_callback,
  };

  nlsk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
  if (!nlsk) {
    printk(KERN_ALERT "Error creating socket.\n");
    return -10;
  }

  return 0;
}

static void __exit database_exit(void)
{
  netlink_kernel_release(nlsk);
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

/* Keeping just in case, for now.
static int test_hash(const char *string, int length, char *result) {
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
*/

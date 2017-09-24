/*****************************************************************************
 * File:        database.c                                                   *
 * Description:                                                              *
 * Version:                                                                  *
 *****************************************************************************/
#include <net/sock.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/rcupdate.h>
#include <linux/rhashtable.h>
#include <linux/slab.h>
#include "command.h"
#include "database.h"

#ifndef NETLINK_USER
#define NETLINK_USER 31
#endif

/* "Private member functions". */
static void database_recv(struct sk_buff *);
static void database_proceed(char *, int);
static void database_rhashtable_cleanup(void *, void *);
static int database_key_compare(struct rhashtable_compare_arg *, const void *);

struct database {
  struct sock *socket;
  struct rhashtable table;
  struct rhashtable_params params;
};

struct database_entry {
  char *key;
  void *data;
  size_t length;
  struct database_entry __rcu *next;
  struct rhash_head rhash;
  struct rcu_head rcu;
};

static const struct rhashtable_params params = {
  .nelem_hint = 1024,
  .head_offset = offsetof(struct database_entry, rhash),
  .key_offset = offsetof(struct database_entry, key),
  .key_len = sizeof(char *),
  .max_size = 1048576,
  .min_size = 256,
  .automatic_shrinking = true,
  .obj_cmpfn = database_key_compare
};

static struct netlink_kernel_cfg cfg = {
  .input = database_recv
};

static struct database database;

int database_init(void) {

  /* Initiaize netlink. */
  database.socket = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
  if (!database.socket)
    return DB_INIT_NETLINK;

  /* Initialize the rhashtable. */
  if (rhashtable_init(&database.table, &params))
    return DB_INIT_RHASHTABLE;

  return DB_INIT_SUCCESS;
}

/*
 * TODO Persistent storage before exit (maybe called from moddb.c?)
 */
void database_free(void) {
  netlink_kernel_release(database.socket);
  rhashtable_free_and_destroy(&database.table, database_rhashtable_cleanup, 0);
}

void database_proceed(char *payload, int len) {
  command_t command = command_new();
  command_deserialize(command, payload);

  printk(KERN_EMERG "Payload size: %d\n", len);
  printk(KERN_EMERG "Operation:    %d\n", command->operation);

  /* Key size is not necessary - always a cstring. */
  printk(KERN_EMERG "Key size:     %d\n", command->key_size);
  printk(KERN_EMERG "Key:          %s\n", command->key);
  printk(KERN_EMERG "Value size:   %d\n", command->value_size);

  /* Not safe in general - not a cstring. */
  printk(KERN_EMERG "Value:        %s\n", command->value);
  command_free(command);
}

void database_recv(struct sk_buff *skb) {
  struct nlmsghdr *nlh;
  struct sk_buff *skb_out;
  char *payload;
  char *serialized;
  int msg_size;
  int pid;
  int len;

  struct command command;
  command.operation = 1;
  command.key = "key_from_kernel";
  command.value = "value from kernel";
  command.key_size = strlen(command.key) + 1;
  command.value_size = strlen(command.value) + 1;
  msg_size = sizeof(char) * (1 + command.value_size + command.key_size) + sizeof(int) * 2;
  serialized = kmalloc(msg_size, GFP_KERNEL);
  command_serialize(&command, serialized);

  nlh = (struct nlmsghdr *) skb->data;
  payload = (char *) nlmsg_data(nlh);
  len = NLMSG_PAYLOAD(nlh, 0);
  database_proceed(payload, len);
  pid = nlh->nlmsg_pid;

  skb_out = nlmsg_new(msg_size, 0);
  nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
  NETLINK_CB(skb_out).dst_group = 0;
  strncpy(nlmsg_data(nlh), serialized, msg_size);

  nlmsg_unicast(database.socket, skb_out, pid);
}

int database_key_compare(struct rhashtable_compare_arg *arg, const void *obj) {
  struct database_entry *entry = (struct database_entry *) obj;
  return strcmp(entry->key, arg->key);
}

void database_rhashtable_cleanup(void *ptr, void *arg) {
  struct database_entry *entry;
  struct database_entry *next;
  
  entry = (struct database_entry *) ptr;

  while (entry) {
    next = rcu_access_pointer(entry->next);
    kfree_rcu(entry, rcu);
    entry = next;
  }
}

/* TODO actual code here. */
void database_save(void) {
}

//
// Created by Igor Ramazanov on 24/09/2017.
//

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <net/genetlink.h>
#include "adaptor.h"
#include "error_code.h"
#include "command.h"
#include <net/sock.h>

#ifndef NETLINK_USER
#define NETLINK_USER 31
#endif

struct adaptor {
  struct sock *socket;
};

static void adaptor_recv(struct sk_buff *);

static struct netlink_kernel_cfg cfg = {
        .input = adaptor_recv
};

static struct adaptor adaptor;

int adaptor_init(void) {
  /* Initialize netlink. */
  adaptor.socket = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
  if (!adaptor.socket) {
    printk(KERN_ALERT "moddb: Failed to create socket\n");
    return ADAPTOR_INIT_NETLINK;
  }
  return SUCCESS;
}

void adaptor_free(void) {
  netlink_kernel_release(adaptor.socket);
}

void adaptor_send(int pid, int seq) {
  command_t command = command_new();
  command->operation = 0;
  command->key = "hello from kernel!";
  command->value = "I'm the value from the kernel";
  command->key_size = (int) (strlen(command->key) + 1);
  command->value_size = (int) (strlen(command->value) + 1);
  char * serialized = command_serialize(command);
  int msg_size = command_size(command);
  struct sk_buff *skb = nlmsg_new(NLMSG_SPACE(msg_size), GFP_KERNEL);
  skb->len = NLMSG_SPACE(msg_size);
  struct nlmsghdr *nlh = (struct nlmsghdr *) skb->data;
  nlh->nlmsg_pid = 0;
  nlh->nlmsg_flags = 0;
  nlh->nlmsg_type = 0x20;
  memcpy(NLMSG_DATA(nlh), serialized, msg_size);
  netlink_unicast(adaptor.socket, skb, pid, MSG_WAITALL);
}

void adaptor_recv(struct sk_buff *skb) {
  struct nlmsghdr *nl = (struct nlmsghdr *) skb->data;
  char *payload = (char *) nlmsg_data(nl);
  int pid = nl->nlmsg_pid;
  int seq = nl->nlmsg_seq;
  int type = nl->nlmsg_type;

  command_t request = command_deserialize(payload);
  printk(KERN_EMERG "FROM USER - Operation:    %d\n", request->operation);
  printk(KERN_EMERG "FROM USER - Key:          %s\n", request->key);
  printk(KERN_EMERG "FROM USER - Value:        %s\n", request->value);
  command_free(request);

  adaptor_send(pid, seq);
}

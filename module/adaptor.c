//
// Created by Igor Ramazanov on 24/09/2017.
//
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <net/netlink.h>
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

void adaptor_recv(struct sk_buff *skb) {
  struct nlmsghdr *nlh;
  struct sk_buff *skb_out;
  char *payload;
  char *serialized;
  int msg_size;
  int pid;
  int len;

  nlh = (struct nlmsghdr *) skb->data;

  payload = (char *) nlmsg_data(nlh);
  len = NLMSG_PAYLOAD(nlh, 0);
  command_t request = command_deserialize(payload);
  printk(KERN_EMERG "FROM USER - Payload size: %d\n", len);
  printk(KERN_EMERG "FROM USER - Operation:    %d\n", request->operation);
  printk(KERN_EMERG "FROM USER - Key size:     %d\n", request->key_size);
  printk(KERN_EMERG "FROM USER - Key:          %s\n", request->key);
  printk(KERN_EMERG "FROM USER - Value size:   %d\n", request->value_size);
  printk(KERN_EMERG "FROM USER - Value:        %s\n", request->value);
  command_free(request);


  struct command response;
  response.operation = 1;
  response.key = "Hello from kernel!";
  response.value = "Test message from kernel space";
  response.key_size = strlen(response.key) + 1;
  response.value_size = strlen(response.value) + 1;
  serialized = command_serialize(&response);

  pid = nlh->nlmsg_pid;

  skb_out = nlmsg_new(msg_size, 0);
  nlh = (struct nlmsghdr *)kmalloc(NLMSG_SPACE(command_size(&response)), GFP_KERNEL);
  nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
  NETLINK_CB(skb_out).dst_group = 0;
  memcpy(nlmsg_data(nlh), serialized, msg_size);

  nlmsg_unicast(adaptor.socket, skb_out, pid);
}
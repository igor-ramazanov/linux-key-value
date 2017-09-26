//
// Created by Igor Ramazanov on 24/09/2017.
//

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <net/genetlink.h>
#include <net/sock.h>
#include "adaptor.h"
#include "error_code.h"
#include "command.h"
#include "database.h"

#ifndef NETLINK_USER
#define NETLINK_USER 31
#endif

struct adaptor {
  struct sock *socket;
};

static void adaptor_recv(struct sk_buff *);
void adaptor_send_not_found(int pid, int seq);

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

void adaptor_send(int pid, int seq, command_t command) {
  struct nlmsghdr *nlh;
  struct sk_buff *skb;
  char *serialized;
  int msg_size;

  serialized = command_serialize(command);
  msg_size = command_size(command);
  skb = nlmsg_new(NLMSG_SPACE(msg_size), GFP_KERNEL);
  skb->len = NLMSG_SPACE(msg_size);
  nlh = (struct nlmsghdr *) skb->data;
  nlh->nlmsg_pid = 0;
  nlh->nlmsg_type = OPERATION_RESPONSE_FOUND;
  nlh->nlmsg_flags = 0;
  memcpy(NLMSG_DATA(nlh), serialized, msg_size);
  netlink_unicast(adaptor.socket, skb, pid, MSG_WAITALL);
}

void adaptor_send_not_found(int pid, int seq) {
  struct nlmsghdr *nlh;
  struct sk_buff *skb;
  char *serialized;
  int msg_size;

  serialized = "";
  msg_size = 1;
  skb = nlmsg_new(NLMSG_SPACE(msg_size), GFP_KERNEL);
  skb->len = NLMSG_SPACE(msg_size);
  nlh = (struct nlmsghdr *) skb->data;
  nlh->nlmsg_pid = 0;
  nlh->nlmsg_type = OPERATION_RESPONSE_NOT_FOUND;
  nlh->nlmsg_flags = 0;
  memcpy(NLMSG_DATA(nlh), serialized, msg_size);
  netlink_unicast(adaptor.socket, skb, pid, MSG_WAITALL);
}

void adaptor_recv(struct sk_buff *skb) {
  struct nlmsghdr *nl;
  char *payload;
  int pid;
  int seq;
  char type;
  nl = (struct nlmsghdr *) skb->data;
  payload = (char *) nlmsg_data(nl);
  pid = nl->nlmsg_pid;
  seq = nl->nlmsg_seq;
  type = nl->nlmsg_type;

  command_t request = command_deserialize(payload);

  switch (type) {
    case OPERATION_REQUEST_GET: {
      char *value;
      size_t size;

      if (!database_lookup(request->key, &value, &size)) {
        command_t response;
        response = command_new();
        response->key = request->key;
        response->key_size = strlen(request->key) + 1;
        response->value = value;
        response->value_size = size;
        adaptor_send(pid, seq, response);
        command_free(response);
      }

      adaptor_send_not_found(pid, seq);
      break;
    }
    case OPERATION_REQUEST_SET:
      database_insert(request->key, request->value, request->value_size);
      break;
    default:
      printk(KERN_ALERT "moddb: user request for unknown operation\n");
      break;
  }

  command_free(request);
}

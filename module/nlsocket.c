/******************************************************************************
 * File:        nlsocket.c                                                    *
 * Description: TODO                                                          *
 * Author:      Igor Ramazanov <ens17irm@cs.umu.se>                           *
 *              Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171008                                                      *
 ******************************************************************************/
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <net/genetlink.h>
#include <net/sock.h>
#include "nlsocket.h"

/* The nlsocket object. */
static struct nlsocket {
  recv_callback recvfrom; /* Callback function for received data. */
  int header;             /* Header type for Netlink to use. */
  struct sock *socket;    /* The actual Netlink socket. */
} netlink;

static void nlsocket_receive(struct sk_buff *);

static struct netlink_kernel_cfg cfg = {
  .input = nlsocket_receive
};

int nlsocket_init(int protocol, int header, recv_callback callback) {

  /* Initialize netlink. */
  netlink.socket = netlink_kernel_create(&init_net, protocol, &cfg);
  if (!netlink.socket)
    return NLSOCKET_INIT_FAILED;

  netlink.recvfrom = callback;
  netlink.header = header;
  return NLSOCKET_INIT_SUCCESS;
}

void nlsocket_destroy(void) {
  netlink_kernel_release(netlink.socket);
}

/*
 * should handle split messages to be a good sendto function.
 * pointers should be small enough to fit in one message, so we'll be fine.
 *
 * returns -1 on failure and 0 on success.
 */
int nlsocket_sendto(pid_t port, const void *data, size_t size) {
  struct nlmsghdr *header;
  struct sk_buff *skb;

  /* Create a Netlink message. */
  skb = nlmsg_new(NLMSG_SPACE(size), GFP_KERNEL);
  if (!skb)
    return NLSOCKET_SENDTO_FAILED;

  /* Create the message header. */
  skb->len = NLMSG_SPACE(size);
  header = (struct nlmsghdr *) skb->data;
  header->nlmsg_type = netlink.header;
  header->nlmsg_pid = 0;
  header->nlmsg_flags = 0;
  memcpy(NLMSG_DATA(header), data, size);

  /* Send the message. */
  if (netlink_unicast(netlink.socket, skb, port, MSG_WAITALL) < 0)
    return NLSOCKET_SENDTO_FAILED;

  /* Clean up and return. */
  nlmsg_free(skb);
  return NLSOCKET_SENDTO_SUCCESS;
}

/*
 * should handle split messages to be a good receive function.
 */
void nlsocket_receive(struct sk_buff *skb) {

  /* We only use netlink.header. */
  struct nlmsghdr *header = (struct nlmsghdr *) skb->data;
  if (header->nlmsg_type != netlink.header)
    return;

  /* Use the callback to process the data. */
  netlink.recvfrom(header->nlmsg_pid, nlmsg_data(header), nlmsg_len(header));
}

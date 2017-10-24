/******************************************************************************
 * File:        nlsocket.h                                                    *
 * Description:                                                               *
 * Author:      Igor Ramazanov <ens17irm@cs.umu.se>                           *
 *              Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171006                                                      *
 ******************************************************************************/
#include <linux/netlink.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "message.h"
#include "nlsocket.h"
#define READ_BUFFER_SIZE 4096          /* Size of the read buffer. */
#define KERNEL_PID 0                   /* Kernel's port number. */

#include <stdio.h>

/* The socket. */
struct nlsocket {
  struct sockaddr_nl source;
  struct sockaddr_nl destination;
  int header;
  int sockfd;
  pid_t port;
};

nlsocket_t nlsocket_new(pid_t port, int protocol, int header) {

  /* Create a new shared map object. */
  nlsocket_t sock = (nlsocket_t) malloc(sizeof(struct nlsocket));
  if (!sock)
    return NULL;

  /* Create a Netlink socket. */
  sock->sockfd = socket(PF_NETLINK, SOCK_RAW, protocol);
  if (sock->sockfd == -1)
    return NULL;

  /* Create a source address. */
  memset(&sock->source, 0, sizeof(struct sockaddr_nl));
  sock->source.nl_family = AF_NETLINK;
  sock->source.nl_pid = port;

  /* Bind the source address. */
  if (bind(sock->sockfd, (const struct sockaddr *) &sock->source,
      sizeof(struct sockaddr_nl)) == -1) {
    close(sock->sockfd);
    return NULL;
  }

  /* Create a destination address (the kernel). */
  memset(&sock->destination, 0, sizeof(struct sockaddr_nl));
  sock->destination.nl_family = AF_NETLINK;
  sock->destination.nl_pid = KERNEL_PID;

  sock->header = header;
  sock->port = port;
  return sock;
}

/**
 * Frees a socket.
 * @param sock The socket.
 */
void nlsocket_free(nlsocket_t sock) {
  close(sock->sockfd);
  free(sock);
}

/*
 * Have to split messages to make this function a generic send.
 * Pointers are small enough to fit in one message, so we'll be fine.
 */
int nlsocket_send(nlsocket_t sock, const void *buf, size_t buflen) {
  struct nlmsghdr *header;
  struct msghdr message;
  struct iovec iovec;
  size_t msg_size;
  int error;

  /* Prepare the request message header .*/
  msg_size = NLMSG_SPACE(buflen);
  header = (struct nlmsghdr *) malloc(msg_size);
  if (!header)
    return -1;

  memset(header, 0, msg_size);
  memcpy(NLMSG_DATA(header), buf, buflen);
  header->nlmsg_type = sock->header;
  header->nlmsg_len = msg_size;
  header->nlmsg_pid = sock->port;

  /* Initialize the iovec structure. */
  iovec.iov_len = header->nlmsg_len;
  iovec.iov_base = header;

  /* Prepare the netlink message. */
  memset(&message, 0, sizeof(struct msghdr));
  message.msg_name = &sock->destination;
  message.msg_namelen = sizeof(struct sockaddr_nl);
  message.msg_iov = &iovec;
  message.msg_iovlen = 1;

  /* Send the message. */
  error = (sendmsg(sock->sockfd, &message, 0) < 0) ? -1 : 0;
  free(header);
  return error;
}

/*
 * Have to assume split messages to make this function a generic recv.
 * Pointers are small enough to fit in one message, so we'll be fine.
 * We need message types that are not Netlink headers already.
 * 
 * TODO special return value when pid != KERNEL_PID. EAGAIN?
 *
 * Returns 0 on success and -1 on failure.
 */
int nlsocket_recv(nlsocket_t sock, void **data, size_t *length) {
  char buffer[READ_BUFFER_SIZE];
  struct sockaddr_nl sender;
  struct iovec iovec;
  struct msghdr message;
  struct nlmsghdr *header;
  size_t payload;
  int error = -1;

  /* Initialize the iovec object. */
  iovec.iov_base = buffer;
  iovec.iov_len = READ_BUFFER_SIZE;

  /* Initialize the message. */
  memset(&message, 0, sizeof(struct msghdr));
  message.msg_namelen = sizeof(struct sockaddr_nl);
  message.msg_name = &sender;
  message.msg_iov = &iovec;
  message.msg_iovlen = 1;

  /* Receive a message. */
  if (recvmsg(sock->sockfd, &message, 0) == -1)
    goto out;

  /* Make sure that the message is from the kernel. */
  if (sender.nl_pid != KERNEL_PID)
    goto out;

  /* Check the returned message type. */
  header = (struct nlmsghdr *) buffer;
  if (header->nlmsg_type != NLMSG_DONE)
    goto out;

  /* Copy the length. */
  payload = header->nlmsg_len - NLMSG_HDRLEN;
  if (length)
    *length = payload;

  /* Only copy data if the user is interested. */
  if (!data) {
    error = 0;
    goto out;
  }

  /* Allocate memory and copy data into it. */
  *data = malloc(payload);
  if (*data == NULL)
    goto out;
  memcpy(*data, NLMSG_DATA(header), payload);
  error = 0;

out:
  return error;
}

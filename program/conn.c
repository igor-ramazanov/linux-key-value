/******************************************************************************
 * File:        conn.c                                                        *
 * Description: TODO                                                          *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20170924                                                      *
 ******************************************************************************/
#include <linux/netlink.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "conn.h"

struct conn {
  struct sockaddr_nl src;
  struct sockaddr_nl dst;
  int fd;
};

conn_t conn_new(void) {
  conn_t conn = (conn_t) malloc(sizeof(struct conn));

  memset(&conn->src, 0, sizeof(struct sockaddr_nl));
  conn->src.nl_family = AF_NETLINK;
  conn->src.nl_pid = getpid();

  memset(&conn->dst, 0, sizeof(struct sockaddr_nl));
  conn->dst.nl_family = AF_NETLINK;

  conn->fd = socket(AF_NETLINK, SOCK_RAW, 31);
  if (conn->fd == -1) {
    perror("socket");
    free(conn);
    return NULL;
  }

  if (bind(conn->fd, (struct sockaddr *) &conn->src, sizeof(struct sockaddr_nl)) == -1) {
    perror("bind");
    close(conn->fd);
    free(conn);
    return NULL;
  }

  return conn;
}

void conn_free(conn_t conn) {
  close(conn->fd);
  free(conn);
}

ssize_t conn_send(conn_t conn, const char *data, size_t len) {
  struct nlmsghdr nlh;
  struct iovec iovec;
  struct msghdr msg;
  ssize_t err;

  memset(&nlh, 0, sizeof(struct nlmsghdr));
  nlh.nlmsg_pid = 0;
  nlh.nlmsg_len = NLMSG_LENGTH(len);
  nlh.nlmsg_pid = conn->src.nl_pid;

  iovec.iov_base = &nlh;
  iovec.iov_len = nlh.nlmsg_len;

  msg.msg_name = &conn->dst;
  msg.msg_namelen = sizeof(struct sockaddr_nl);
  msg.msg_iov = &iovec;
  msg.msg_iovlen = 1;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;

  err = sendmsg(conn->fd, &msg, 0);
  if (err == -1)
    perror("sendmsg");

  return err;
}

ssize_t conn_recv(conn_t conn, char *data, size_t len) {
  char buffer[4096];
  struct sockaddr_nl sa;
  struct nlmsghdr *nlh;
  struct iovec iovec;
  struct msghdr msg;
  size_t length;
  size_t bytes_read;
  size_t total_read;

  msg.msg_name = &sa;
  msg.msg_namelen = sizeof(struct sockaddr_nl);
  msg.msg_iov = &iovec;
  msg.msg_iovlen = 1;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;

  bytes_read = 0;
  total_read = 0;
  length = recvmsg(conn->fd, &msg, 0);

  if (length == -1) {
    perror("recvmsg");
    return -1;
  }

  for (nlh = (struct nlmsghdr *) buffer;
       NLMSG_OK(nlh, length);
       nlh = NLMSG_NEXT(nlh, length)) {
    if (nlh->nlmsg_type == NLMSG_DONE)
      break;

    if (nlh->nlmsg_type == NLMSG_ERROR) {
      fprintf(stderr, "Received error message from kernel.\n");
      return -1;
    }

    bytes_read = NLMSG_PAYLOAD(nlh, length);
    if (bytes_read > len) {
      fprintf(stderr, "Reading would overflow buffer.\n");
      return -1;
    }

    memcpy(data + total_read, NLMSG_DATA(nlh), bytes_read);
    total_read += bytes_read;
  }

  return total_read;
}

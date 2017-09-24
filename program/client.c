#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "client.h"
#include "command.h"

#define NETLINK_USER 31

typedef struct client {
    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr *nlh;
    struct iovec iov;
    int sock_fd;
    struct msghdr msg;
};

static struct client client;

int client_init() {
    client.sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (client.sock_fd < 0)
        return NETLINK_INIT_ERROR;

    memset(&client.src_addr, 0, sizeof(client.src_addr));
    client.src_addr.nl_family = AF_NETLINK;
    client.src_addr.nl_pid = getpid();

    bind(client.sock_fd, (struct sockaddr *) &client.src_addr, sizeof(client.src_addr));

    memset(&client.dest_addr, 0, sizeof(client.dest_addr));
    memset(&client.dest_addr, 0, sizeof(client.dest_addr));
    client.dest_addr.nl_family = AF_NETLINK;
    client.dest_addr.nl_pid = 0;
    client.dest_addr.nl_groups = 0;

    return SUCCESS;
}

void client_free() {
    close(client.sock_fd);
}

void client_set(char *key, char *data) {
    command_t request = command_new();
    request->operation = SET;
    request->key = strdup(key);
    request->value = strdup(data);
    request->key_size = (int) (strlen(request->key) + 1);
    request->value_size = (int) (strlen(request->value) + 1);
    char *serialized_request = command_serialize(request);
    size_t msg_size = NLMSG_SPACE(command_size(request));

    client.nlh = (struct nlmsghdr *) malloc(msg_size);
    memset(client.nlh, 0, msg_size);
    client.nlh->nlmsg_len = msg_size;
    client.nlh->nlmsg_pid = getpid();
    client.nlh->nlmsg_flags = 0;

    memcpy(NLMSG_DATA(client.nlh), serialized_request, command_size);

    client.iov.iov_base = (void *) client.nlh;
    client.iov.iov_len = client.nlh->nlmsg_len;
    client.msg.msg_name = (void *) &client.dest_addr;
    client.msg.msg_namelen = sizeof(client.dest_addr);
    client.msg.msg_iov = &client.iov;
    client.msg.msg_iovlen = 1;

    sendmsg(client.sock_fd, &client.msg, 0);
}

void client_get(char *key) {
    command_t request = command_new();
    request->operation = SET;
    request->key = strdup(key);
    request->value = strdup("");
    request->key_size = (int) (strlen(request->key) + 1);
    request->value_size = (int) (strlen(request->value) + 1);
    char *serialized_request = command_serialize(request);
    size_t msg_size = NLMSG_SPACE(command_size(request));

    client.nlh = (struct nlmsghdr *) malloc(msg_size);
    memset(client.nlh, 0, msg_size);
    client.nlh->nlmsg_len = msg_size;
    client.nlh->nlmsg_pid = getpid();
    client.nlh->nlmsg_flags = 0;

    memcpy(NLMSG_DATA(client.nlh), serialized_request, command_size);

    client.iov.iov_base = (void *) client.nlh;
    client.iov.iov_len = client.nlh->nlmsg_len;
    client.msg.msg_name = (void *) &client.dest_addr;
    client.msg.msg_namelen = sizeof(client.dest_addr);
    client.msg.msg_iov = &client.iov;
    client.msg.msg_iovlen = 1;

    sendmsg(client.sock_fd, &client.msg, 0);
    free(client.nlh);
    recvmsg(client.sock_fd, &client.msg, 0);
    char *buff = (char *) NLMSG_DATA(client.nlh);
    command_t response = command_deserialize(buff);
    printf("%s\n", response->value);
}
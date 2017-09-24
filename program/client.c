#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "client.h"
#include "command.h"

#define NETLINK_USER 31

struct client {
    struct sockaddr_nl src_addr, dest_addr;
    int sock_fd;
};

static struct client client;

void client_send_request(struct command *request);

void client_receive_msg();

int client_init() {
    client.sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (client.sock_fd < 0)
        return NETLINK_INIT_ERROR;

    memset(&client.src_addr, 0, sizeof(client.src_addr));
    client.src_addr.nl_family = AF_NETLINK;
    client.src_addr.nl_pid = getpid();
    client.src_addr.nl_groups = 0;

    bind(client.sock_fd, (struct sockaddr *) &client.src_addr, sizeof(client.src_addr));
    perror("bind");

    memset(&client.dest_addr, 0, sizeof(client.dest_addr));
    client.dest_addr.nl_family = AF_NETLINK;
    client.dest_addr.nl_pid = 0;
    client.dest_addr.nl_groups = 0;

    return SUCCESS;
}

void client_free() {
    close(client.sock_fd);
}

void client_send_request(struct command *request) {
    char *serialized_request = command_serialize(request);
    size_t msg_size = NLMSG_SPACE(command_size(request));


    struct nlmsghdr *nlh = (struct nlmsghdr *) malloc(NLMSG_SPACE(msg_size));
    memset(nlh, 0, NLMSG_SPACE(msg_size));
    nlh->nlmsg_seq = 0;
    nlh->nlmsg_type = 0x31;
    nlh->nlmsg_len = NLMSG_SPACE(msg_size);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    memcpy(NLMSG_DATA(nlh), serialized_request, command_size(request));

    struct iovec iov;
    iov.iov_len = nlh->nlmsg_len;
    iov.iov_base = nlh;
    struct msghdr msg;
    msg.msg_name = &client.dest_addr;
    msg.msg_namelen = sizeof(client.dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;

    sendmsg(client.sock_fd, &msg, 0);
}

void client_set(char *key, char *data) {
    command_t request = command_new();
    request->operation = SET;
    request->key = strdup(key);
    request->value = strdup(data);
    request->key_size = (int) (strlen(request->key) + 1);
    request->value_size = (int) (strlen(request->value) + 1);

    client_send_request(request);
}

void client_get(char *key) {
    command_t request = command_new();
    request->operation = GET;
    request->key = strdup(key);
    request->value = strdup("");
    request->key_size = (int) (strlen(request->key) + 1);
    request->value_size = (int) (strlen(request->value) + 1);

    client_send_request(request);
    client_receive_msg();
}

void client_receive_msg() {
    char buf[4096];
    struct iovec iov = { buf, sizeof(buf) };
    struct sockaddr_nl sa;
    struct msghdr msg = { &sa, sizeof(sa), &iov, 1, NULL, 0, 0 };
    struct nlmsghdr *nh;

    recvmsg(client.sock_fd, &msg, 0);
    nh = (struct nlmsghdr *) buf;
    char * payload = NLMSG_DATA(nh);
    command_t command = command_deserialize(payload);
    printf("FROM KERNEL - Key: %s\n", command->key);
    printf("FROM KERNEL - Value: %s\n", command->value);
}

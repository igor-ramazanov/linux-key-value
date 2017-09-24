#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define NETLINK_USER 31
#define MAX_PAYLOAD 1024

struct command {
    //0 - Set, 1 - Get
    char operation;
    int key_size;
    int value_size;
    char *key;
    char *value;
};

void serialise_command(struct command *command, char *data) {
    *data = command->operation;
    data++;

    int *int_ptr = (int *) data;
    *int_ptr = command->key_size;
    int_ptr++;
    *int_ptr = command->value_size;
    int_ptr++;
    char *char_ptr = (char *) int_ptr;
    int i = 0;
    while (i < command->key_size) {
        *char_ptr = command->key[i];
        char_ptr++;
        i++;
    }

    i = 0;
    while (i < command->value_size) {
        *char_ptr = command->value[i];
        char_ptr++;
        i++;
    }
};

void deserialise_command(struct command *command, char *data) {
    command->operation = *data;
    data++;
    int *int_ptr = (int *) data;
    command->key_size = *int_ptr;
    int_ptr++;
    command->value_size = *int_ptr;
    int_ptr++;

    command->key = malloc(sizeof(char) * (command->key_size));
    command->value = malloc(sizeof(char) * (command->value_size));
    char *char_ptr = (char *) int_ptr;
    int i = 0;
    while (i < command->key_size) {
        command->key[i] = *char_ptr;
        char_ptr++;
        i++;
    }
    i = 0;
    while (i < command->value_size) {
        command->value[i] = *char_ptr;
        char_ptr++;
        i++;
    }
};

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;

int main() {
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0)
        return -1;

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* self pid */

    bind(sock_fd, (struct sockaddr *) &src_addr, sizeof(src_addr));

    memset(&dest_addr, 0, sizeof(dest_addr));
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */

    struct command command;
    command.operation = 1;
    command.key = "test_key";
    command.value = "test_value";
    command.key_size = strlen(command.key) + 1;
    command.value_size = strlen(command.value) + 1;
    int command_size = sizeof(char) * (1 + command.value_size + command.key_size) + sizeof(int) * 2;
    char *serialised = malloc(command_size);
    serialise_command(&command, serialised);

    printf("Command size: %d\n", command_size);

    nlh = (struct nlmsghdr *) malloc(NLMSG_SPACE(command_size));
    memset(nlh, 0, NLMSG_SPACE(command_size));
    nlh->nlmsg_len = NLMSG_SPACE(command_size);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    memcpy(NLMSG_DATA(nlh), serialised, command_size);

    iov.iov_base = (void *) nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *) &dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    printf("Sending message to kernel\n");
    sendmsg(sock_fd, &msg, 0);
    printf("Waiting for message from kernel\n");

/* Read message from kernel */
    recvmsg(sock_fd, &msg, 0);
    int len = NLMSG_PAYLOAD(nlh, 0);
    char * buff = malloc(len);
    struct command* cmd;
    deserialise_command(cmd, buff);

    printf("%s\n", cmd->key);
    printf("%s\n", cmd->value);
    close(sock_fd);
}
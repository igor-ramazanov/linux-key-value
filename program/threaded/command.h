#include <linux/netlink.h>

#pragma once

enum operation {
    OPERATION_REQUEST_SET = 17,
    OPERATION_REQUEST_GET = 18,
    OPERATION_RESPONSE_FOUND = 19,
    OPERATION_RESPONSE_NOT_FOUND = 20
};

typedef struct command {
    int key_size;
    int value_size;
    char *key;
    char *value;
} *command_t;

command_t command_new(void);

void command_free(command_t);

char *command_serialize(command_t);

command_t command_deserialize(char *);

int command_size(command_t);
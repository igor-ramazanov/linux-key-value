#pragma once

enum operation {
    GET, SET
};

typedef struct command {
    char operation;
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
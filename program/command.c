#include <string.h>
#include <stdlib.h>
#include <printf.h>
#include "command.h"

void serialize(command_t, char *);

void deserialize(command_t, char *);

command_t command_new(void) {
    command_t command;
    command = (command_t) malloc(sizeof(struct command));
    return command;
}

void command_free(command_t command) {
    free(command->key);
    free(command->value);
    free(command);
}

char *command_serialize(command_t command) {
    char *data = (char *) malloc(command_size(command));
    serialize(command, data);
    return data;
}

void serialize(command_t command, char *data) {
    char *char_ptr;
    int *int_ptr;
    int i;

    int_ptr = (int *) data;
    *int_ptr = command->key_size;
    int_ptr++;
    *int_ptr = command->value_size;
    int_ptr++;
    char_ptr = (char *) int_ptr;
    i = 0;
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

command_t command_deserialize(char *data) {
    command_t command;
    command = command_new();
    deserialize(command, data);
    return command;
}

void deserialize(struct command *command, char *data) {
    char *char_ptr;
    int *int_ptr;
    int i;

    int_ptr = (int *) data;
    command->key_size = *int_ptr;
    int_ptr++;
    command->value_size = *int_ptr;
    int_ptr++;

    command->key = malloc(sizeof(char) * (command->key_size));
    command->value = malloc(sizeof(char) * (command->value_size));
    char_ptr = (char *) int_ptr;

    i = 0;
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

int command_size(command_t command) {
    return sizeof(char) * (command->value_size + command->key_size) + sizeof(int) * 2;
}

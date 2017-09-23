#include <linux/string.h>
#include <linux/slab.h>

struct Command {
    //0 - Set, 1 - Get
    char operation;
    int key_size;
    int value_size;
    char *key;
    char *value;
};

char *serialise(struct Command *command) {
    char *serialised = kmalloc(sizeof(*command), GFP_KERNEL);

    char *operation_ptr = serialised;
    int *key_size_ptr = (int *) (serialised + sizeof(char));
    int *value_size_ptr = key_size_ptr + sizeof(int);
    char *key_ptr = (char *) (value_size_ptr + sizeof(char) * command->key_size);
    char *value_ptr = key_ptr + sizeof(char) * command->value_size;

    *operation_ptr = command->operation;
    *key_size_ptr = command->key_size;
    *value_size_ptr = command->value_size;
    memcpy(key_ptr, command->key, command->key_size);
    memcpy(value_ptr, command->value, command->value_size);

    return serialised;
};

struct Command deserialise(char *data) {
    char *operation_ptr = data;
    int *key_size_ptr = (int *) (data + sizeof(char));
    int *value_size_ptr = key_size_ptr + sizeof(int);
    char *key_ptr = (char *) (value_size_ptr + sizeof(char) * (*key_size_ptr));
    char *value_ptr = key_ptr + sizeof(char) * (*value_size_ptr);

    struct Command command;
    command.operation = *operation_ptr;
    command.key_size = *key_size_ptr;
    command.value_size = *value_size_ptr;
    memcpy(command.key, key_ptr, command.key_size);
    memcpy(command.value, value_ptr, command.value_size);
    return command;
};

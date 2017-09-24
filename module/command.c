#include <linux/slab.h>
#include <linux/string.h>
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

  command->key = kmalloc(sizeof(char) * (command->key_size), GFP_KERNEL);
  command->value = kmalloc(sizeof(char) * (command->value_size), GFP_KERNEL);
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

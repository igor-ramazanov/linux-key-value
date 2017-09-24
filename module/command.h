/*****************************************************************************
 * File:        command.h                                                    *
 * Description:                                                              *
 * Version:                                                                  *
 *****************************************************************************/
#pragma once

typedef struct command {
  char operation;
  int key_size;
  int value_size;
  char *key;
  char *value;
} *command_t;

command_t command_new(void);
void command_free(command_t);
void command_serialize(command_t, char *);
void command_deserialize(command_t, char *);

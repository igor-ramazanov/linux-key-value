/******************************************************************************
 * File:        message.c                                                     *
 * Description: Implementations of message constructing functions.            *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171006                                                      *
 ******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "message.h"

#include <stdio.h>

void message_print(message_t message) {
  if (!message) {
    printf("message: (NULL)\n");
    return;
  }

  printf("Type:         0x%02x\n", (int) message->type);
  printf("Key length:   %lu\n", message->key_length);
  printf("Value length: %lu\n", message->value_length);
  printf("Key:          %s\n", (message->key_length ? message->key : "(NULL)"));
  printf("Value:        %s\n", (message->value_length ? (char *) message->value : "(NULL)"));
}

static message_t message_build(unsigned char type, const char *key,
    const void *value, size_t value_length) {
  size_t message_length;
  size_t key_length;
  message_t message;

  /* Compute the length of the message. */
  key_length = strlen(key) + 1;
  message_length = sizeof(struct message);
  message_length += key_length;
  message_length += value_length;

  /* Create the message. */
  message = (message_t) malloc(message_length);
  if (!message)
    return NULL;

  /* Copy data into it. */
  message->type = type;
  message->key_length = key_length;
  message->value_length = value_length;
  message->key = (char *) message + sizeof(struct message);
  message->value = message->key + key_length;
  memcpy(message->key, key, key_length);
  memcpy(message->value, value, value_length);

  return message;
}

/**
 * Frees a message.
 * @param message the message.
 */
void message_free(message_t message) {
  free(message);
}

message_t message_cast(void *data) {
  message_t message = (message_t) data;
  message->key = (char *) message + sizeof(struct message);
  message->value = message->key + message->key_length;
  return message;
}

message_t message_lookup(const char *key) {
  return message_build(MESSAGE_LOOKUP, key, NULL, 0);
}

message_t message_insert(const char *key, const void *value, size_t length) {
  return message_build(MESSAGE_INSERT, key, value, length);
}

inline size_t message_length(message_t message) {
  size_t data_length = message->key_length + message->value_length;
  return sizeof(struct message) + data_length;
}

/******************************************************************************
 * File:        message.c                                                     *
 * Description: Implementations of message constructing functions.            *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171006                                                      *
 ******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "message.h"

static void *memdup(const void *data, size_t length);

/**
 * Frees a message.
 * @param message the message.
 */
void message_free(message_t message) {
  free(message->key);
  free(message->value);
  free(message);
}

message_t message_lookup(const char *key) {
  message_t message = (message_t) malloc(sizeof(struct message));
  message->key_length = strlen(key) + 1;
  message->value_length = 0;
  message->key = strdup(key);
  message->value = NULL;
  message->type = MESSAGE_REQUEST_LOOKUP;
  return message;
}

message_t message_insert(const char *key, const void *value, size_t length) {
  message_t message = (message_t) malloc(sizeof(struct message));
  message->key_length = strlen(key) + 1;
  message->value_length = length;
  message->key = strdup(key);
  message->value = memdup(value, length);
  message->type = MESSAGE_REQUEST_INSERT;
  return message;
}

/* Why don't this function exist already? */
void *memdup(const void *data, size_t size) {
  void *copy = malloc(size);
  memcpy(copy, data, size);
  return copy;
}

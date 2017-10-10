/******************************************************************************
 * File:        message.c                                                     *
 * Description: Implementations of message builder functions.                 *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171007                                                      *
 ******************************************************************************/
#include <linux/slab.h>
#include <linux/string.h>
#include "message.h"

static message_t message_build(unsigned char type, const void *value,
    size_t value_length) {
  message_t message;
  size_t message_length;

  /* Create the message. */
  message_length = sizeof(struct message) + value_length;
  message = (message_t) kmalloc(message_length, GFP_KERNEL);
  if (!message)
    return NULL;

  /* Copy data into it. */
  message->type = type;
  message->key_length = 0;
  message->value_length = value_length;
  message->key = (char *) message + sizeof(struct message);
  message->value = (void *) message->key;
  memcpy(message->value, value, value_length);

  return message;
}

/*
 * This function updates the pointers (for convenience).
 * The returned message must NOT be freed.
 */
message_t message_cast(void *data) {
  message_t message = (message_t) data;
  message->key = (char *) message + sizeof(struct message);
  message->value = message->key + message->key_length;
  return message;
}

void message_free(message_t message) {
  kfree(message);
}

inline size_t message_length(message_t message) {
  size_t data_length = message->key_length + message->value_length;
  return sizeof(struct message) + data_length;
}

inline message_t message_lookup_ok(const void *value, size_t value_length) {
  return message_build(MESSAGE_LOOKUP_OK, value, value_length);
}

inline message_t message_key_not_found(void) {
  return message_build(MESSAGE_KEY_NOT_FOUND, NULL, 0);
}

inline message_t message_value_inserted(void) {
  return message_build(MESSAGE_VALUE_INSERTED, NULL, 0);
}

inline message_t message_value_replaced(void) {
  return message_build(MESSAGE_VALUE_REPLACED, NULL, 0);
}

inline message_t message_error(void) {
  return message_build(MESSAGE_ERROR, NULL, 0);
}

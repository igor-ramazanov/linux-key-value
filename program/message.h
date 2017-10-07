/******************************************************************************
 * File:        message.h                                                     *
 * Description: Builder functions for request messages to the kernel.         *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171006                                                      *
 ******************************************************************************/
#pragma once

/* Message types. */
typedef enum message_type {

  /* User-space requests. */
  MESSAGE_REQUEST_LOOKUP         = 0x00, /* For lookup requests. */
  MESSAGE_REQUEST_INSERT         = 0x01, /* For insertion requests. */

  /* Kernel-space responses. */
  MESSAGE_RESPONSE_LOOKUP        = 0x20, /* For successful lookups. */
  MESSAGE_RESPONSE_INSERT        = 0x21, /* For new insertions. */
  MESSAGE_RESPONSE_REPLACED      = 0x22, /* For updates. */
  MESSAGE_RESPONSE_KEY_NOT_FOUND = 0x23, /* For lookup on non-existent keys. */

  /* Generic error messages. */
  MESSAGE_RESPONSE_ERROR         = 0x40  /* For other, unexpected errors. */
} message_type_t;

/* The message. */
typedef struct message {
  message_type_t type;
  size_t key_length;
  size_t value_length;
  char *key;
  void *value;
} *message_t;

message_t message_insert(const char *key, const void *value, size_t length);
message_t message_lookup(const char *key);
void message_free(message_t message);

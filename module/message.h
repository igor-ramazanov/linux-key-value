/******************************************************************************
 * File:        message.h                                                     *
 * Description: Builder functions for response messages from the kernel.      *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171006                                                      *
 ******************************************************************************/
#pragma once
#include <stddef.h>

/* User-space requests. */
#define MESSAGE_LOOKUP         0x00 /* For lookup requests. */
#define MESSAGE_INSERT         0x01 /* For insertion requests. */

/* Kernel-space responses. */
#define MESSAGE_LOOKUP_OK      0x20 /* For successful lookups. */
#define MESSAGE_VALUE_INSERTED 0x21 /* For new insertions. */
#define MESSAGE_VALUE_REPLACED 0x22 /* For updates. */
#define MESSAGE_KEY_NOT_FOUND  0x23 /* For lookup on non-existent keys. */

/* Generic error messages. */
#define MESSAGE_ERROR          0x40 /* For other, unexpected errors. */

typedef struct message {
  unsigned char type;    /* Message type - one of the above. */
  size_t key_length;     /* Length of the key, including '\0'. */
  size_t value_length;   /* Length of the value. */
  char *key;             /* Cstring key. */
  void *value;           /* The value. */
} *message_t;

message_t message_cast(void *data);
message_t message_lookup_ok(const void *value, size_t length);
message_t message_key_not_found(void);
message_t message_value_inserted(void);
message_t message_value_replaced(void);
message_t message_error(void);
size_t message_length(message_t message);
void message_free(message_t message);

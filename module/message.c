/******************************************************************************
 * File:        message.c                                                     *
 * Description: Implementations of message builder functions.                 *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171007                                                      *
 ******************************************************************************/
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include "message.h"

/* XXX What happens with copy_to_user/copy_from_user if value i NULL? */ 

static message_t message_empty(gfp_t flags);
static message_t message_user(const char *key, size_t key_length,
    const void *value, size_t value_length, message_type_t type);

/*
 * Frees a message.
 */
void message_free(message_t message) {

  //if (!message)
    //return;

  //kfree(message->value);
  //kfree(message->key);
  //kfree(message);
}

/*
 * Creates and returns a message containing the key-value mapping.
 * The returned message exists in user-space.
 */
inline message_t message_lookup(const char *key, const void *value, size_t length) {
  message_type_t type = MESSAGE_REQUEST_LOOKUP;
  return message_user(key, strlen(key) + 1, value, length, type);
}

/*
 * Creates and returns a message indicating that the key was not mapped.
 * The returned message exists in user-space.
 */
inline message_t message_key_not_found(const char *key) {
  message_type_t type = MESSAGE_RESPONSE_KEY_NOT_FOUND;
  return message_user(key, strlen(key) + 1, NULL, 0, type);
}

/*
 * Creates and returns a message indicating that a new pair was mapped.
 * The returned message exists in user-space.
 */
inline message_t message_inserted(const char *key) {
  message_type_t type = MESSAGE_RESPONSE_INSERT;
  return message_user(key, strlen(key) + 1, NULL, 0, type);
}

/*
 * Creates and returns a message indicating that the mapped value was replaced.
 * The returned message exists in user-space.
 */
inline message_t message_replaced(const char *key) {
  message_type_t type = MESSAGE_RESPONSE_INSERT;
  return message_user(key, strlen(key) + 1, NULL, 0, type);
}

/*
 * Creates an returns an error message.
 * The returned message exists in user-space.
 */
inline message_t message_error(void) {
  return message_user(NULL, 0, NULL, 0, MESSAGE_RESPONSE_ERROR);
}

/*
 * Copies a message in user-space to kernel-space.
 */
message_t message_copy(const message_t message) {
  const char *user_key;
  const void *user_value;

  printk(KERN_DEBUG "virt_addr_valid: %d\n", virt_addr_valid(message));

  /* Create a copy of the message. */
  message_t copy = message_empty(GFP_KERNEL);
  if (!copy || !copy_from_user(copy, message, sizeof(struct message))) {
    printk(KERN_DEBUG "failed to copy from user\n");
    goto error;
  }

  printk(KERN_DEBUG "type: %d\n", copy->type);
  printk(KERN_DEBUG "klen: %lu\n", copy->key_length);
  printk(KERN_DEBUG "vlen: %lu\n", copy->value_length);

  /* Copy the key. */
  user_key = copy->key;
  copy->key = (char *) kmalloc(copy->key_length, GFP_KERNEL);
  if (!copy->key || !copy_from_user(copy->key, user_key, copy->key_length)) {
    printk(KERN_DEBUG "failed to copy key\n");
    goto error;
  }

  /* Copy the value. */
  user_value = copy->value;
  copy->value = kmalloc(copy->value_length, GFP_KERNEL);
  if (!copy->value || !copy_from_user(copy->value, user_value,
      copy->value_length)) {
    printk(KERN_DEBUG "failed to copy value\n");
    goto error;
  }

  /* No errors. */
  return copy;

error:
  message_free(copy);
  return NULL;
}

/*
 * Helper function for the response message builder functions.
 * The message will be created in user-space and key and value copied to it.
 */
message_t message_user(const char *key, size_t key_length,
    const void *value, size_t value_length, message_type_t type) {

  /* Create a message in user-space and copy data to it. */
  message_t message = message_empty(GFP_USER);
  if (!message)
    goto error;
    
  /* Copy the key. */
  message->key = (char *) kmalloc(key_length, GFP_USER);
  if (!message->key || !copy_to_user(message->key, key, key_length))
    goto error;

  /* Copy the value. */
  message->value = kmalloc(value_length, GFP_USER);
  if ((value && !message->value)
      || !copy_to_user(message->value, value, value_length))
    goto error;

  /* Copy the lengths and and type. */
  if (!copy_to_user(&message->key_length, &key_length, sizeof(size_t))
      || !copy_to_user(&message->value_length, &value_length, sizeof(size_t))
      || !copy_to_user(&message->type, &type, sizeof(message_type_t)))
    goto error;

  /* No errors - return the message. */
  return message;

error:
  message_free(message);
  return NULL;
}

/*
 * Hepler function for the other builder functions.
 * Creates a message (kernel- or user-space) and initializes pointer to NULL.
 */
message_t message_empty(gfp_t flags) {

  /* Create the message. */
  message_t message = (message_t) kmalloc(sizeof(struct message), flags);
  if (!message)
    return NULL;

  /* Initialize everything and return the message. */
  memset(message, 0, sizeof(struct message));
  return message;
}

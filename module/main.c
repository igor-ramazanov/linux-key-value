/*****************************************************************************
 * File:        main.c                                                       *
 * Description:                                                              *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                *
 *              Igor Ramazanov <ens17irm@cs.umu.se>                          *
 * Version:     20171006                                                     *
 *****************************************************************************/
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include "nlsocket.h"
#include "map.h"

#ifndef SHARED_MAP_PROTOCOL
#define SHARED_MAP_PROTOCOL 31
#endif

#define SHARED_MAP_HEADER 17

// @formatter:off
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Shared key-value pair database.");
MODULE_AUTHOR("Erik Ramos <c03ers@cs.umu.se>");
MODULE_AUTHOR("Igor Ramazanov <ens17irv@cs.umu.se>");
MODULE_AUTHOR("Bastien Harendarczyk<ens17bhk@cs.umu.se>");
MODULE_VERSION("0.1");
// @formatter:on

static void request_handler(pid_t, const void *, size_t);
static void handle_insert_request(pid_t, message_t);
static void handle_lookup_request(pid_t, message_t);

static int __init shared_map_init(void) {

  /* Initialize the hashtable. */
  if (map_init()) {
    printk(KERN_ALERT "shared_map: Failed to initialize rhashtable\n");
    return 1;
  }

  /* Initialize Netlink. */
  if (nlsocket_init(SHARED_MAP_PROTOCOL, SHARED_MAP_HEADER, request_handler)) {
    printk(KERN_ALERT "shared_map: Failed to initialize Netlink\n");
    return 1;
  }
 
  /* Initialization successful. */
  printk(KERN_INFO "shared_map: Module successfully installed\n");
  return 0;
}

static void __exit shared_map_exit(void) {
  nlsocket_destroy();
  map_save();
  map_destroy();
  printk(KERN_INFO "shared_map: Module removed\n");
}

/*
 * Handles messages sent to the module's Netlink socket.
 */
static void request_handler(pid_t user, const void *data, size_t size) {
  message_t request = message_copy(*(message_t *) data);
  if (!request)
    return;

  switch (request->type) {
  case MESSAGE_REQUEST_LOOKUP:
    handle_lookup_request(user, request);
    break;
  case MESSAGE_REQUEST_INSERT:
    handle_insert_request(user, request);
    break;
  default:
    message_free(request);
    break;
  }
}

/*
 * Handles requests to create a mapping.
 */
void handle_insert_request(pid_t user, message_t request) {
  message_t response;

  /* Attempt to create the mapping. */
  switch (map_insert(request->key, request->value, request->value_length)) {
  case MAP_INSERT_SUCCESS:

    /* Successful insertion of a new element. */
    response = message_inserted(request->key);
    break;
  case MAP_INSERT_REPLACED:

    /* Successful replacement of an old element. */
    response = message_replaced(request->key);
    break;
  default:

    /* Insertion failed. */
    response = message_error();
    message_free(request);
    return;
  }

  printk(KERN_DEBUG "resonse address is: %lx\n", response);

  /* Send a response. */
  if (response)
    nlsocket_sendto(user, &response, sizeof(message_t));

  /* Note that key and value in the request must not be freed. */
  //kfree(request);
}

/*
 * NOTE sending pointer to a variable on the stack seems dangerous
 *      but it should be pointing to memory in user-space... right?
 */
void handle_lookup_request(pid_t user, message_t request) {
  void *data;
  size_t length;

  /* Insert the value and generate an appropriate response. */
  message_t response = !map_lookup(request->key, &data, &length)
      ? message_lookup(request->key, data, length)
      : message_key_not_found(request->key);

  /* Send the response. */
  if (nlsocket_sendto(user, &response, sizeof(message_t)))
    message_free(response);

  /* Clean up and return. */
  message_free(request);
}

module_init(shared_map_init);
module_exit(shared_map_exit);

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

static void request_handler(pid_t, void *, size_t);
static void handle_insert_request(pid_t, message_t);
static void handle_lookup_request(pid_t, message_t);

static int __init shared_map_init(void) {

  message_t message = message_lookup_ok("Hello, World!", 14);
  message_print(message);
  //printk(KERN_DEBUG "value:        %s\n", (char *) message->value);
  //printk(KERN_DEBUG "value length: %lu\n", message->value_length);
  //printk(KERN_DEBUG "key length:   %lu\n", message->key_length);
  message_free(message);


  /* Initialize the hashtable. */
  //if (map_init()) {
    //printk(KERN_ALERT "shared_map: Failed to initialize rhashtable\n");
    //return 1;
  //}

  /* Initialize Netlink. */
  //if (nlsocket_init(SHARED_MAP_PROTOCOL, SHARED_MAP_HEADER, request_handler)) {
    //printk(KERN_ALERT "shared_map: Failed to initialize Netlink\n");
    //return 1;
  //}
 
  /* Initialization successful. */
  //printk(KERN_INFO "shared_map: Module successfully installed\n");
  return 0;
}

static void __exit shared_map_exit(void) {
  //nlsocket_destroy();
  //map_save();
  //map_destroy();
  printk(KERN_INFO "shared_map: Module removed\n");
}

/*
 * Handles messages sent to the module's Netlink socket.
 */
static void request_handler(pid_t user, void *data, size_t size) {
  message_t request = message_cast(data);
  if (!request)
    return;

  switch (request->type) {
  case MESSAGE_LOOKUP:
    handle_lookup_request(user, request);
    break;
  case MESSAGE_INSERT:
    handle_insert_request(user, request);
    break;
  default:
    break;
  }
}

/*
 * Handles requests to create a mapping.
 */
void handle_insert_request(pid_t user, const message_t request) {
  message_t response;

  /* TODO copy key and value. */

  /* Attempt to create the mapping. */
  switch (map_insert(request->key, request->value, request->value_length)) {
  case MAP_INSERT_SUCCESS:

    /* Successful insertion of a new element. */
    response = message_value_inserted();
    break;
  case MAP_INSERT_REPLACED:

    /* Successful replacement of an old element. */
    response = message_value_replaced();
    break;
  default:

    /* Insertion failed. */
    response = message_error();
    return;
  }

  if (!response) {
    printk(KERN_ERR "shared_map: out of memory?\n");
    return;
  }

  /* Send a response. */
  nlsocket_sendto(user, response, message_length(response));
  message_free(response);
}

void handle_lookup_request(pid_t user, const message_t request) {
  size_t length;
  void *data;

  /* Insert the value and generate an appropriate response. */
  message_t response = !map_lookup(request->key, &data, &length)
      ? message_lookup_ok(data, length)
      : message_key_not_found();

  if (!response) {
    printk(KERN_ERR "shared_map: out of memory?\n");
    return;
  }

  /* Send the response. */
  nlsocket_sendto(user, response, message_length(response));
  message_free(response);
}

module_init(shared_map_init);
module_exit(shared_map_exit);

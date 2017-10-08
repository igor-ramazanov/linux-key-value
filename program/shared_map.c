/******************************************************************************
 * File:        shared_map.c                                                  *
 * Description: Implements the interface with the shared map kernel module.   *
 * Author:      Igor Ramazanov <ens17irm@cs.umu.se>                           *
 *              Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171006                                                      *
 ******************************************************************************/
#include <stdlib.h>
#include "nlsocket.h"
#include "shared_map.h"
#define SHARED_MAP_PROTOOCL 31 /* Our Netlink protocol number. */
#define SHARED_MAP_HEADER   17 /* Header type for our Netlink messages. */

/* The shared_map. */
struct shared_map {
  nlsocket_t socket;
};

shared_map_t shared_map_new(pid_t tid) {

  /* Create the shared map object. */
  shared_map_t map = (shared_map_t) malloc(sizeof(struct shared_map));
  if (!map)
    return NULL;

  /* Create a socket for communication. */
  map->socket = nlsocket_new(tid, SHARED_MAP_PROTOOCL, SHARED_MAP_HEADER);
  if (!map->socket) {
    free(map);
    return NULL;
  }

  return map;
}

void shared_map_free(shared_map_t map) {
  nlsocket_free(map->socket);
  free(map);
}

#include <stdio.h>
#include <unistd.h>

int shared_map_insert(shared_map_t map,
    const char *key,
    const void *value,
    size_t size) {
  message_t request;
  message_t response;

  /* Send a request to insert a value. */
  request = message_insert(key, value, size);
  if (nlsocket_send(map->socket, &request, sizeof(message_t)))
    goto out;

  printf("message sent.\n");

  /* Receive a response from the kernel. */
  if (nlsocket_recv(map->socket, &response, sizeof(message_t)))
    goto out;

  printf("%s => %s\n", response->key, (char *) response->value);
  return 0;

  /* On success, this message must be freed. */
  message_free(response);

out:

  /* Clean up and return. */
  message_free(request);
  return -1;
}

/*
 * For now, this function makes no difference between
 * successful lookups and ones where the key does not exist.
 * If the key does not exist, the value will be set to NULL
 * and length will be 0.
 */
int shared_map_lookup(shared_map_t map,
    const char *key,
    void **value,
    size_t *size) {
  message_t request;
  message_t response;
  int error;

  /* Send the lookup request. */
  request = message_lookup(key);
  error = nlsocket_send(map->socket, &request, sizeof(message_t));
  if (!error)
    goto out;

  /* Receive the response from the kernel. */
  error = nlsocket_recv(map->socket, &response, sizeof(message_t));
  if (!error)
    goto out;

  /* Copy the values. */
  error = (response->type == MESSAGE_RESPONSE_ERROR) ? -1 : 0;
  *value = response->value;
  *size = response->value_length;

  /*
   * message_free also frees the value and we don't want that.
   * On errors and when the key does not exist, value is NULL,
   * so no memory will leak.
   */
  free(response->key);
  free(response);

out:

  /* Clean up and return. */
  message_free(request);
  return error;
}

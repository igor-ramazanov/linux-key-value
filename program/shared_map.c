/******************************************************************************
 * File:        shared_map.c                                                  *
 * Description: Implements the interface with the shared map kernel module.   *
 * Author:      Igor Ramazanov <ens17irm@cs.umu.se>                           *
 *              Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171006                                                      *
 ******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "nlsocket.h"
#include "shared_map.h"
#define SHARED_MAP_PROTOOCL 31 /* Our Netlink protocol number. */
#define SHARED_MAP_HEADER   17 /* Header type for our Netlink messages. */

/* The shared_map. */
struct shared_map {
  nlsocket_t socket;
};

static void *memdup(const void *data, size_t length) {
  void *copy = malloc(length);
  if (copy)
    memcpy(copy, data, length);
  return copy;
}

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

int shared_map_insert(shared_map_t map, const char *key, const void *value,
    size_t value_length) {
  message_t request;
  message_t response;
  void *data;
  int error = -1;

  /* Send a request to insert a value. */
  request = message_insert(key, value, value_length);
  if (!request || nlsocket_send(map->socket, request, message_length(request)))
    goto out;

  /* Receive a response from the kernel. */
  if (nlsocket_recv(map->socket, &data, NULL))
    goto out;

  response = message_cast(data);
  message_print(response);
  //printf("%s => %s\n", response->key, (char *) response->value);

  /* On success, this message must be freed. */
  //message_free(response);
  error = 0;

out:

  /* Clean up and return. */
  //message_free(request);
  return error;
}

int shared_map_lookup(shared_map_t map, const char *key, void **value,
    size_t *value_length) {
  message_t request;
  message_t response;
  void *data;
  int error = -1;

  /* Send the lookup request. */
  response = NULL;
  request = message_lookup(key);
  if (nlsocket_send(map->socket, request, message_length(request)))
    goto out;

  /* Receive the response from the kernel. */
  if (nlsocket_recv(map->socket, &data, NULL))
    goto out;

  /* Check the message type. */
  response = message_cast(data);
  message_print(response);
  if (response->type != MESSAGE_LOOKUP_OK)
    goto out;

  /* Copy the values. */
  *value_length = response->value_length;
  *value = memdup(response->value, *value_length);
  error = 0;

out:

  /* Clean up and return. */
  message_free(response);
  message_free(request);
  return error;
}

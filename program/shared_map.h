/******************************************************************************
 * File:        shared_map.h                                                  *
 * Description: Interface for the shared map kernel module.                   *
 * Version:     20171006                                                      *
 ******************************************************************************/
#pragma once
#include <sys/types.h>

typedef struct shared_map *shared_map_t;

shared_map_t shared_map_new(pid_t port);
void shared_map_free(shared_map_t map);
int shared_map_insert(shared_map_t map, const char *key, const void *value,
    size_t value_length);
int shared_map_lookup(shared_map_t map, const char *key, void **value,
    size_t *value_length);

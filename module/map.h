/*****************************************************************************
 * File:        map.h                                                        *
 * Description:                                                              *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                *
 * Version:     20171008                                                     *
 *****************************************************************************/
#pragma once

/* map_init return values. */
#define MAP_INIT_SUCCESS  0    /* map is ready to use. */
#define MAP_INIT_FAILED  -1    /* rhashtable could not be initiallized. */

/* map_insert return values. */
#define MAP_INSERT_SUCCESS   0 /* New element inserted. */
#define MAP_INSERT_REPLACED  1 /* Old element replaced. */
#define MAP_INSERT_FAILED   -1 /* No element inserted. */

/* map_lookup return values. */
#define MAP_LOOKUP_SUCCESS  0  /* Key found. */
#define MAP_LOOKUP_FAILED  -1  /* Key not found.*/

int map_init(void);
void map_destroy(void);
int map_save(void);
int map_insert(char *key, void *data, size_t length);
int map_lookup(const char *key, void **data, size_t *length);

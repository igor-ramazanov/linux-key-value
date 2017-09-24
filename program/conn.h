/******************************************************************************
 * File:        conn.h                                                        *
 * Description: TODO                                                          *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20170924                                                      *
 ******************************************************************************/
#pragma once
#include <stddef.h>

typedef struct conn *conn_t;

conn_t conn_new(void);
void conn_free(conn_t);
ssize_t conn_send(conn_t, const char *buf, size_t len);
ssize_t conn_recv(conn_t, char *buf, size_t len);


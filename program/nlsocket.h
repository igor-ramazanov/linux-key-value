/******************************************************************************
 * File:        nlsocket.h                                                    *
 * Description: TODO                                                          *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171008                                                      *
 ******************************************************************************/
#pragma once
#include <sys/types.h>
#include "message.h"

typedef struct nlsocket *nlsocket_t;

nlsocket_t nlsocket_new(pid_t port, int protocol);
void nlsocket_free(nlsocket_t sock);
int nlsocket_send(nlsocket_t sock, const void *buf, size_t buflen);
int nlsocket_recv(nlsocket_t sock, void *buf, size_t buflen);

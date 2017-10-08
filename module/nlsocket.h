/******************************************************************************
 * File:        nlsocket.h                                                    *
 * Description: Netlink socket for communicating with user-space processes.   *
 * Author:      Igor Ramazanov <ens17irm@cs.umu.se>                           *
 *              Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171006                                                      *
 ******************************************************************************/
#pragma once
#include "message.h"

#define NLSOCKET_INIT_SUCCESS  0
#define NLSOCKET_INIT_FAILED  -1

#define NLSOCKET_SENDTO_SUCCESS  0
#define NLSOCKET_SENDTO_FAILED  -1

typedef void (*recv_callback)(pid_t sender, const void *data, size_t size);

int nlsocket_init(int protocol, int header, recv_callback callback);
void nlsocket_destroy(void);
int nlsocket_sendto(pid_t destination, const void *data, size_t size);

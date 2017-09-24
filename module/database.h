/*****************************************************************************
 * File:        database.h                                                   *
 * Description:                                                              *
 * Version:                                                                  *
 *****************************************************************************/
#pragma once

enum database_error {
  DB_INIT_SUCCESS    = 0,
  DB_INIT_NETLINK    = 1,
  DB_INIT_RHASHTABLE = 2
};

int database_init(void);
void database_free(void);

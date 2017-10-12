/******************************************************************************
 * File:        pstore.h                                                      *
 * Description: TODO                                                          *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171011                                                      *
 ******************************************************************************/
#pragma once

int pstore_init(unsigned long capacity);
void pstore_destroy(void);
void pstore_clear(void);
// void pstore_save(...);
// void pstore_load(...);

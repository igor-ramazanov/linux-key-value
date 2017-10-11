/******************************************************************************
 * File:        logger.h                                                      *
 * Description: TODO                                                          *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171011                                                      *
 ******************************************************************************/
#pragma once

/* Redefine this to change log-level. */
#ifndef LOGGER_LOGLEVEL
#define LOGGER_LOGLEVEL 1 /* Defaults to showing only error messages. */
#endif

void logger_error(const char *format, ...);
void logger_warn(const char *format, ...);
void logger_info(const char *format, ...);
void logger_debug(const char *format, ...);

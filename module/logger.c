/******************************************************************************
 * File:        logger.c                                                      *
 * Description: TODO                                                          *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                 *
 * Version:     20171011                                                      *
 ******************************************************************************/
#include <linux/printk.h>
#include "logger.h"

void logger_error(const char *format, ...) {
#if LOGGER_LOGLEVEL > 0
  va_list list;
  va_start(list, format);
  vprintk_emit(0, LOGLEVEL_ERR, NULL, 0, format, list);
  va_end(list);
#endif
}

void logger_warn(const char *format, ...) {
#if LOGGER_LOGLEVEL > 1
  va_list list;
  va_start(list, format);
  vprintk_emit(0, LOGLEVEL_WARNING, NULL, 0, format, list);
  va_end(list);
#endif
}

void logger_info(const char *format, ...) {
#if LOGGER_LOGLEVEL > 2
  va_list list;
  va_start(list, format);
  vprintk_emit(0, LOGLEVEL_INFO, NULL, 0, format, list);
  va_end(list);
#endif
}

void logger_debug(const char *format, ...) {
#if LOGGER_LOGLEVEL > 3
  va_list list;
  va_start(list, format);
  vprintk_emit(0, LOGLEVEL_DEBUG, NULL, 0, format, list);
  va_end(list);
#endif
}

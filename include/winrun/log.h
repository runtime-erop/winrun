#ifndef WINRUN_LOG_H
#define WINRUN_LOG_H

#include <stdarg.h>

void log_set_debug(int enabled);
void log_debug(const char *fmt, ...);
void log_info(const char *fmt, ...);
void log_warn(const char *fmt, ...);
void log_error(const char *fmt, ...);
void log_errno(const char *context);

#endif

#include "winrun/log.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

static int g_log_debug_enabled = 0;

void log_set_debug(int enabled) {
    g_log_debug_enabled = enabled ? 1 : 0;
}

static void vlog_emit(const char *level, const char *fmt, va_list ap) {
    fprintf(stderr, "winrun [%s] ", level);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
}

void log_debug(const char *fmt, ...) {
    if (!g_log_debug_enabled) {
        return;
    }
    va_list ap;
    va_start(ap, fmt);
    vlog_emit("debug", fmt, ap);
    va_end(ap);
}

void log_info(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vlog_emit("info", fmt, ap);
    va_end(ap);
}

void log_warn(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vlog_emit("warn", fmt, ap);
    va_end(ap);
}

void log_error(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vlog_emit("error", fmt, ap);
    va_end(ap);
}

void log_errno(const char *context) {
    int err = errno;
    log_error("%s: %s (%d)", context, strerror(err), err);
}

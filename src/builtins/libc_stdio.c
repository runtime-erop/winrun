#include "winrun/abi.h"
#include "winrun/builtin_registry.h"
#include "winrun/runtime_state.h"

#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

typedef void (WINRUN_MS_ABI *winrun_onexit_fn)(void);

static FILE *sanitize_stream(FILE *stream) {
    if (stream == stdin || stream == stdout || stream == stderr) return stream;
    return stdout;
}

static FILE *WINRUN_MS_ABI builtin___acrt_iob_func(unsigned idx) {
    switch (idx) { case 0: return stdin; case 1: return stdout; case 2: return stderr; default: return NULL; }
}
static FILE *WINRUN_MS_ABI builtin___iob_func(void) { return stdin; }
static int WINRUN_MS_ABI builtin___stdio_common_vfprintf(unsigned long long options, FILE *stream, const char *format, void *locale, va_list args) {
    (void)options; (void)locale;
    stream = sanitize_stream(stream);
    if (!format) { errno = EINVAL; return -1; }
    return vfprintf(stream, format, args);
}
static void *WINRUN_MS_ABI builtin_malloc(size_t size) { return malloc(size); }
static void *WINRUN_MS_ABI builtin_calloc(size_t n, size_t size) { return calloc(n, size); }
static void WINRUN_MS_ABI builtin_free(void *ptr) { free(ptr); }
static void *WINRUN_MS_ABI builtin_memcpy(void *dst, const void *src, size_t size) { return memcpy(dst, src, size); }
static void *WINRUN_MS_ABI builtin_memset(void *dst, int c, size_t size) { return memset(dst, c, size); }
static size_t WINRUN_MS_ABI builtin_strlen(const char *s) { return strlen(s); }
static size_t WINRUN_MS_ABI builtin_wcslen(const wchar_t *s) { return wcslen(s); }
static int WINRUN_MS_ABI builtin_strncmp(const char *lhs, const char *rhs, size_t n) { return strncmp(lhs, rhs, n); }
static struct lconv *WINRUN_MS_ABI builtin_localeconv(void) { return localeconv(); }
static char *WINRUN_MS_ABI builtin_strerror(int errnum) { return strerror(errnum); }
static int WINRUN_MS_ABI builtin_fprintf(FILE *stream, const char *format, ...) {
    va_list args; va_start(args, format);
    int rc = vfprintf(sanitize_stream(stream), format, args);
    va_end(args); return rc;
}
static int WINRUN_MS_ABI builtin_vfprintf(FILE *stream, const char *format, va_list args) { return vfprintf(sanitize_stream(stream), format, args); }
static int WINRUN_MS_ABI builtin_fputc(int c, FILE *stream) { return fputc(c, sanitize_stream(stream)); }
static size_t WINRUN_MS_ABI builtin_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) { return fwrite(ptr, size, nmemb, sanitize_stream(stream)); }
static int WINRUN_MS_ABI builtin_getchar(void) { return getchar(); }
static int WINRUN_MS_ABI builtin_puts(const char *s) { return puts(s); }
static winrun_onexit_fn WINRUN_MS_ABI builtin__onexit(winrun_onexit_fn fn) {
    runtime_state_push_onexit((void (*)(void))fn);
    return fn;
}
static void WINRUN_MS_ABI builtin__amsg_exit(int rc) { exit(rc); }

__attribute__((constructor)) static void register_libc_stdio(void) {
    builtin_registry_register("__acrt_iob_func", builtin___acrt_iob_func);
    builtin_registry_register("__iob_func", builtin___iob_func);
    builtin_registry_register("__stdio_common_vfprintf", builtin___stdio_common_vfprintf);
    builtin_registry_register("malloc", builtin_malloc);
    builtin_registry_register("calloc", builtin_calloc);
    builtin_registry_register("free", builtin_free);
    builtin_registry_register("memcpy", builtin_memcpy);
    builtin_registry_register("memset", builtin_memset);
    builtin_registry_register("strlen", builtin_strlen);
    builtin_registry_register("wcslen", builtin_wcslen);
    builtin_registry_register("strncmp", builtin_strncmp);
    builtin_registry_register("localeconv", builtin_localeconv);
    builtin_registry_register("strerror", builtin_strerror);
    builtin_registry_register("fprintf", builtin_fprintf);
    builtin_registry_register("vfprintf", builtin_vfprintf);
    builtin_registry_register("fputc", builtin_fputc);
    builtin_registry_register("fwrite", builtin_fwrite);
    builtin_registry_register("getchar", builtin_getchar);
    builtin_registry_register("puts", builtin_puts);
    builtin_registry_register("_onexit", builtin__onexit);
    builtin_registry_register("_amsg_exit", builtin__amsg_exit);
}

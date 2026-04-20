#include "winrun/abi.h"
#include "winrun/builtin_registry.h"
#include "winrun/runtime_state.h"

#include <errno.h>
#include <stdlib.h>

static int WINRUN_MS_ABI builtin__set_new_mode(int mode) {
    *runtime_state_commode_ptr() = mode;
    return *runtime_state_commode_ptr();
}
static int WINRUN_MS_ABI builtin__configthreadlocale(int mode) { (void)mode; return 0; }
static int WINRUN_MS_ABI builtin___setusermatherr(void *handler) { (void)handler; return 0; }
static void *WINRUN_MS_ABI builtin___C_specific_handler(void) { return NULL; }
static char ***WINRUN_MS_ABI builtin___p__environ(void) { return runtime_state_environ_ptr(); }
static int *WINRUN_MS_ABI builtin___p___argc(void) { return runtime_state_argc_ptr(); }
static char ***WINRUN_MS_ABI builtin___p___argv(void) { return runtime_state_argv_ptr(); }
static int *WINRUN_MS_ABI builtin___p__fmode(void) { return runtime_state_fmode_ptr(); }
static int *WINRUN_MS_ABI builtin___p__commode(void) { return runtime_state_commode_ptr(); }
static unsigned int WINRUN_MS_ABI builtin____lc_codepage_func(void) { return runtime_state_lc_codepage(); }
static int WINRUN_MS_ABI builtin____mb_cur_max_func(void) { return runtime_state_mb_cur_max(); }
static int *WINRUN_MS_ABI builtin__errno(void) { return &errno; }
static int WINRUN_MS_ABI builtin__configure_narrow_argv(int mode) { (void)mode; return 0; }
static int WINRUN_MS_ABI builtin__crt_atexit(void (*fn)(void)) { return fn ? atexit(fn) : 0; }
static int WINRUN_MS_ABI builtin__initialize_narrow_environment(void) { return 0; }
static void WINRUN_MS_ABI builtin__set_app_type(int type) { (void)type; }
static void *WINRUN_MS_ABI builtin__set_invalid_parameter_handler(void *handler) { return handler; }
static int WINRUN_MS_ABI builtin__getmainargs(int *argc, char ***argv, char ***env, int do_wildcard, int *startinfo) {
    (void)do_wildcard;
    (void)startinfo;
    if (argc) *argc = *runtime_state_argc_ptr();
    if (argv) *argv = *runtime_state_argv_ptr();
    if (env) *env = *runtime_state_environ_ptr();
    return 0;
}
static void WINRUN_MS_ABI builtin__initterm(void (**start)(void), void (**end)(void)) {
    if (!start || !end) return;
    for (void (**it)(void) = start; it < end; ++it) if (*it) (*it)();
}
static int WINRUN_MS_ABI builtin__initterm_e(int (**start)(void), int (**end)(void)) {
    if (!start || !end) return 0;
    for (int (**it)(void) = start; it < end; ++it) if (*it) { int rc = (*it)(); if (rc != 0) return rc; }
    return 0;
}
static void WINRUN_MS_ABI builtin__lock(int locknum) { (void)locknum; }
static void WINRUN_MS_ABI builtin__unlock(int locknum) { (void)locknum; }
static void WINRUN_MS_ABI builtin__cexit(void) { runtime_state_run_onexit(); }
static void WINRUN_MS_ABI builtin_abort(void) { abort(); }
static void WINRUN_MS_ABI builtin_exit(int code) { runtime_state_run_onexit(); exit(code); }
static void WINRUN_MS_ABI builtin__exit(int code) { _Exit(code); }

__attribute__((constructor)) static void register_crt_core(void) {
    builtin_registry_register("__p__environ", builtin___p__environ);
    builtin_registry_register("__p___argc", builtin___p___argc);
    builtin_registry_register("__p___argv", builtin___p___argv);
    builtin_registry_register("__p__fmode", builtin___p__fmode);
    builtin_registry_register("__p__commode", builtin___p__commode);
    builtin_registry_register("___lc_codepage_func", builtin____lc_codepage_func);
    builtin_registry_register("___mb_cur_max_func", builtin____mb_cur_max_func);
    builtin_registry_register("_set_new_mode", builtin__set_new_mode);
    builtin_registry_register("_configthreadlocale", builtin__configthreadlocale);
    builtin_registry_register("__setusermatherr", builtin___setusermatherr);
    builtin_registry_register("__C_specific_handler", builtin___C_specific_handler);
    builtin_registry_register("__getmainargs", builtin__getmainargs);
    builtin_registry_register("_cexit", builtin__cexit);
    builtin_registry_register("_commode", runtime_state_commode_ptr());
    builtin_registry_register("_fmode", runtime_state_fmode_ptr());
    builtin_registry_register("_errno", builtin__errno);
    builtin_registry_register("_configure_narrow_argv", builtin__configure_narrow_argv);
    builtin_registry_register("_crt_atexit", builtin__crt_atexit);
    builtin_registry_register("_initialize_narrow_environment", builtin__initialize_narrow_environment);
    builtin_registry_register("_set_app_type", builtin__set_app_type);
    builtin_registry_register("__set_app_type", builtin__set_app_type);
    builtin_registry_register("_set_invalid_parameter_handler", builtin__set_invalid_parameter_handler);
    builtin_registry_register("_initterm", builtin__initterm);
    builtin_registry_register("_initterm_e", builtin__initterm_e);
    builtin_registry_register("_lock", builtin__lock);
    builtin_registry_register("_unlock", builtin__unlock);
    builtin_registry_register("abort", builtin_abort);
    builtin_registry_register("exit", builtin_exit);
    builtin_registry_register("_exit", builtin__exit);
    builtin_registry_register("__initenv", runtime_state_initenv_ptr());
}

#include "winrun/runtime_state.h"

#include <stddef.h>
#include <stdbool.h>

extern char **environ;

static int g_loader_argc = 0;
static char **g_loader_argv = NULL;
static int g_loader_fmode = 0;
static int g_loader_commode = 0;
static char **g_loader_initenv = NULL;
static unsigned int g_loader_lc_codepage = 65001;
static int g_loader_mb_cur_max = 1;
static __thread uint32_t g_win_last_error = 0;
static void (*g_onexit_callbacks[256])(void);
static size_t g_onexit_count = 0;
static bool g_running_onexit_callbacks = false;

void runtime_state_set_args(int argc, char **argv) {
    g_loader_argc = argc;
    g_loader_argv = argv;
    g_loader_initenv = environ;
}

int *runtime_state_argc_ptr(void) { return &g_loader_argc; }
char ***runtime_state_argv_ptr(void) { return &g_loader_argv; }
char ***runtime_state_environ_ptr(void) { return &environ; }
char ***runtime_state_initenv_ptr(void) { return &g_loader_initenv; }
int *runtime_state_fmode_ptr(void) { return &g_loader_fmode; }
int *runtime_state_commode_ptr(void) { return &g_loader_commode; }
unsigned int runtime_state_lc_codepage(void) { return g_loader_lc_codepage; }
int runtime_state_mb_cur_max(void) { return g_loader_mb_cur_max; }
uint32_t runtime_state_get_last_error(void) { return g_win_last_error; }
void runtime_state_set_last_error(uint32_t value) { g_win_last_error = value; }

void runtime_state_push_onexit(void (*fn)(void)) {
    if (!fn) {
        return;
    }
    if (g_onexit_count < (sizeof(g_onexit_callbacks) / sizeof(g_onexit_callbacks[0]))) {
        g_onexit_callbacks[g_onexit_count++] = fn;
    }
}

void runtime_state_run_onexit(void) {
    if (g_running_onexit_callbacks) {
        return;
    }
    g_running_onexit_callbacks = true;
    while (g_onexit_count > 0) {
        void (*fn)(void) = g_onexit_callbacks[--g_onexit_count];
        if (fn) {
            fn();
        }
    }
    g_running_onexit_callbacks = false;
}

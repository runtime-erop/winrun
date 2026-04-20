#include "winrun/abi.h"
#include "winrun/builtin_registry.h"
#include "winrun/runtime_state.h"

#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

static void *WINRUN_MS_ABI builtin_signal(int signum, void *handler) {
    (void)signum; (void)handler; return NULL;
}
static uint32_t WINRUN_MS_ABI builtin_GetLastError(void) { return runtime_state_get_last_error(); }
static void WINRUN_MS_ABI builtin_SetLastError(uint32_t err) { runtime_state_set_last_error(err); }
static void WINRUN_MS_ABI builtin_Sleep(uint32_t ms) {
    struct timespec req = { .tv_sec = (time_t)(ms / 1000), .tv_nsec = (long)((ms % 1000) * 1000000UL) };
    while (nanosleep(&req, &req) != 0 && errno == EINTR) {}
}
static uint32_t WINRUN_MS_ABI builtin_GetTickCount(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0;
    uint64_t total_ms = (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
    return (uint32_t)total_ms;
}
static uint32_t WINRUN_MS_ABI builtin_GetCurrentProcessId(void) { return (uint32_t)getpid(); }

__attribute__((constructor)) static void register_winapi_core(void) {
    builtin_registry_register("signal", builtin_signal);
    builtin_registry_register("GetLastError", builtin_GetLastError);
    builtin_registry_register("SetLastError", builtin_SetLastError);
    builtin_registry_register("Sleep", builtin_Sleep);
    builtin_registry_register("GetTickCount", builtin_GetTickCount);
    builtin_registry_register("GetCurrentProcessId", builtin_GetCurrentProcessId);
}

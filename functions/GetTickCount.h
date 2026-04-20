#include "../lib.h"

DWORD GetTickCount(void){
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0){
        return 0;
    }

    uint64_t total_ms = (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
    return (DWORD)(total_ms & 0xFFFFFFFFu);
}

#include "../lib.h"

void Sleep(DWORD dwMilliseconds){
    usleep((useconds_t)dwMilliseconds * 1000u);
}

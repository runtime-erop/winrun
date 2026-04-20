#include "../lib.h"
#include <SendInput.h>

void mouse_event(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, ULONG_PTR dwExtraInfo){
    if (!is_initilized) MAIN_INIT();
    INPUT inp;
    inp.type = INPUT_MOUSE;
    inp.mi.dwFlags = dwFlags;
    inp.mi.dx = dx;
    inp.mi.dy = dy;
    inp.mi.mouseData = dwData;
    inp.mi.dwExtraInfo = dwExtraInfo;
    INPUT inps[1] = {inp};
    SendInput(1, inps, sizeof(INPUT));
}
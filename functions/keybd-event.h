#include "../lib.h"
#include <SendInput.h>

void keybd_event(BYTE bVk, BYTE bScan, BYTE dwFlags, ULONG_PTR dwExtraInfo){
    if (!is_initilized) MAIN_INIT();
    INPUT inp;
    inp.type = INPUT_KEYBOARD;
    inp.ki.wVk = bVk;
    inp.ki.wScan = bScan;
    inp.ki.dwFlags = dwFlags;
    inp.ki.dwExtraInfo = dwExtraInfo;
    INPUT inps[1] = {inp};
    SendInput(1, inps, sizeof(INPUT));
}
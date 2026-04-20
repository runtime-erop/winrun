#include "../lib.h"

short GetKeyState(int nVirtKey){
    if (!is_initilized) MAIN_INIT();
    short linux_key = winapi_to_linux_key(nVirtKey);
    if (current_keybd_state[linux_key]){
        return 0x8000;
    }
    return 0x0000;
}
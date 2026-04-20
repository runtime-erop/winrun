#include "../lib.h"

bool SetKeyboardState(PBYTE lpKeyState[256]){
    memcpy(current_keybd_state, lpKeyState, 256);
    return true;
}
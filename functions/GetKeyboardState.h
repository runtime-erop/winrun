#include "../lib.h"

bool GetKeyboardState(PBYTE lpKeyState[256]){
    PBYTE temp[256];
    for (int i = 0; i < 256; i++){
        if (linux_to_winapi_key(i) < 256 && linux_to_winapi_key(i) >= 0){
            temp[linux_to_winapi_key(i)] = is_button_pressed[i];
        }
    }
    if (temp != current_pressed_buttons){
        memcpy(current_keybd_state, temp, 256);
        lpKeyState = temp;
        return true;
    } 
    lpKeyState = current_keybd_state;
    return true;
}
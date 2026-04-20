#include "../lib.h"

int GetKeyNameTextA(LONG lParam, LPSTR lpString, int cchSize){
    UINT scanCode = (lParam >> 16) & 0xFF;
    strncpy(lpString, GetKeyNameFromScanCode(scanCode), cchSize - 1);
    return 0;
}
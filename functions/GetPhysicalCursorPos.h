#include "../lib.h"

bool GetPhysicalCursorPos(POINT *lpPoint){

    if (!is_initilized) MAIN_INIT();
    double k = 143 / 96;
    (*lpPoint).x = cursor_x * k;
    (*lpPoint).y = cursor_y * k;
    return 1;
}
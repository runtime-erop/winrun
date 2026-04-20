#include "../lib.h"

bool GetCursorPos(POINT *point){
    if (!is_initilized) MAIN_INIT();
    update_cursor_pos();
    (*point).x = cursor_x;
    (*point).y = cursor_y;
    return 1;
}
#include "../lib.h"

DWORD GetLastError(void){
    return (DWORD)errno;
}

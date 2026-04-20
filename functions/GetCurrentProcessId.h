#include "../lib.h"

DWORD GetCurrentProcessId(void){
    return (DWORD)getpid();
}

#include "../lib.h"

void SetLastError(DWORD dwErrCode){
    errno = (int)dwErrCode;
}

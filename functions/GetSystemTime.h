#include "../lib.h"

void GetSystemTime(SYSTEMTIME *lpSystemTime){
    time_t rawtime;
    time(&rawtime);
    struct tm *utc_time = gmtime(&rawtime);
    (*lpSystemTime).wDay = utc_time -> tm_mday;
    (*lpSystemTime).wHour = utc_time -> tm_hour;
    (*lpSystemTime).wMinute = utc_time -> tm_min;
    (*lpSystemTime).wSecond = utc_time -> tm_sec;
    (*lpSystemTime).wYear = utc_time -> tm_year + 1900;
    (*lpSystemTime).wMonth = utc_time -> tm_mon + 1;
    (*lpSystemTime).wDayOfWeek = utc_time -> tm_wday;
}
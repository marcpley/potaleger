// C++ Program to implement Date and Time parsing using
// <ctime>
#include "qdatetime.h"
#include <ctime>
#include <iostream>
using namespace std;

QDate StrToDatetime(const char* datetimeString)
{
    struct tm tmStruct;
    strptime(datetimeString, "%Y-%m-%d", &tmStruct);
    QDate d = mktime(&tmStruct);
    return d;
}

string DateTimeToStr(time_t time)
{
    char buffer[90];
    struct tm* timeinfo = localtime(&time);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
    return buffer;
}

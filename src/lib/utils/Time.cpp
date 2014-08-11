/*
 * Time.cpp
 *
 *  Created on: Jan 1, 2014
 *      Author: arnd
 */

#include "Time.h"

#include <cstdio>
#include <stdexcept>
#include <chrono>
#include "utils/logging.h"

#include "utility.h"

using namespace std;
using namespace chrono;
using namespace log4cplus;

namespace db_agg {
    static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("Time"));

    string Time::to_string(string format) {
        struct tm *ltime = localtime(&_time);
        char buf[512];
        if (strftime(buf,512,format.c_str(),ltime)>512) {
            throw runtime_error("time buffer overflow");
        }
        //LOG_TRACE("create time " << buf);
        return std::string(buf);
    }

    string Time::getDuration(long int ticks) {
        system_clock::time_point::duration tp(nanoseconds((long)ticks));
        auto m = tp / minutes(1);
        auto rest = (tp % minutes(1));
        long s = rest / seconds(1);
        rest = (rest % seconds(1));
        long ms = rest / milliseconds(1);
        string fmt = string_format("%02d:%02d.%03d",m,s,ms);
        return fmt;
    }

    Time::Time(string ts) {
        //LOG_TRACE("create time " << ts);
        int year, month, day, hour, minute, second, micros;
        sscanf(ts.c_str(), "%d-%d-%d %d:%d:%d.%d", &year, &month, &day, &hour,
                &minute, &second, &micros);
        struct tm timeinfo;
        timeinfo.tm_year = year - 1900;
        timeinfo.tm_mon = month - 1;
        timeinfo.tm_mday = day;
        timeinfo.tm_hour = hour;
        timeinfo.tm_min = minute;
        timeinfo.tm_sec = second;
        timeinfo.tm_isdst = -1;
        _time = mktime(&timeinfo);
    }

}



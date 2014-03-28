/*
 * Time.h
 *
 *  Created on: Jan 1, 2014
 *      Author: arnd
 */

#ifndef TIME_H_
#define TIME_H_

#include <ctime>
#include <string>

namespace db_agg {
    class Time {
        private:
            time_t _time;
        public:
            Time() {
                time(&_time);
            }
            Time(time_t t) {
                _time = t;
            }
            Time(std::string);
            std::string to_string(std::string format="%Y-%m-%d %H:%M:%S");
            static Time from_string(std::string format="%Y-%m-%d %H:%M:%S");
            time_t getUnixTime() {
                return _time;
            }
            static std::string getDuration(long int ticks);
    };
}

#endif /* TIME_H_ */

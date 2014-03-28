/*
 * CursesListener.h
 *
 *  Created on: Jan 24, 2014
 *      Author: arnd
 */

#ifndef CURSESLISTENER_H_
#define CURSESLISTENER_H_

#include <stddef.h>
#include <map>
#include <array>
#include <string>
#include <chrono>

#include "core/Application.h"


namespace db_agg {

class Column {
public:
    Column(std::string label, size_t width):
        label(label),
        offset(0),
        width(width) {}
    std::string label;
    size_t offset;
    size_t width;
};

class CursesListener: public EventListener {
private:
    Application& application;
    std::map<std::string,size_t> resultIdToLine;
    std::map<std::string,size_t> queryIdToLine;
    std::map<size_t,std::chrono::system_clock::time_point> timeSpent;
    std::vector<Column> columns = {
            {"execution",0},
            {"status",17},
            {"time spent",11},
            {"received",11},
            {"last executed",21},
            {"duration",11},
            {"last rcvd",11}
    };
public:
    CursesListener(Application& application);
    virtual ~CursesListener();
    virtual void handleEvent(Event& event) override;
    virtual void updateClock();
    void print(std::string resultId, std::string col, std::string value, bool leftJustified = false);
};
}



#endif /* CURSESLISTENER_H_ */

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
#include <memory>
#include <assert.h>

#include "core/Application.h"


namespace db_agg {

enum class ColumnType {
    EXECUTION,
    STATUS,
    TIME_SPENT,
    RECEIVED,
    SENT,
    LAST_EXECUTED,
    LAST_DURATION,
    LAST_RECEIVED
};

class Column {
public:
    Column(
            ColumnType type,
            std::string label,
            size_t requestedWidth,
            size_t minimalWidth,
            bool leftJustified
          );
    ColumnType type;
    std::string label;
    size_t width = 0;
    size_t requestedWidth;
    size_t minimalWidth;
    bool show = true;
    bool leftJustified = false;
};

struct position {
    size_t line;
    size_t col;
    std::string value;
};

class CursesListener: public EventListener {
private:
    Application& application;
    bool initialized = false;
    bool compactView = true;
    size_t clusterNo = 0;
    size_t clusterWidth = 0;
    size_t statusOffset = 0;
    std::map<std::string,std::map<ColumnType,position>> resultIdToPosition;
    std::map<std::string,position> queryIdToLine;
    std::map<std::string,std::chrono::system_clock::time_point> timeSpent;
    std::vector<Column> columns = {
            {ColumnType::EXECUTION,"execution",30, 15, true},
            {ColumnType::STATUS,"status",17, 10, true},
            {ColumnType::TIME_SPENT,"time spent",11, 11,true},
            {ColumnType::RECEIVED,"received", 13, 13, false},
            {ColumnType::SENT,"sent",13, 13, false},
            {ColumnType::LAST_EXECUTED,"last executed",21, 21, true},
            {ColumnType::LAST_DURATION,"duration",11, 11, true},
            {ColumnType::LAST_RECEIVED,"last rcvd",13, 13, false}
    };
    void calculateLayout();
    bool calculateRequiredSpace();
    void refresh();
    void print(std::string resultId, ColumnType col, std::string value);
    virtual void updateClock();
public:
    CursesListener(Application& application);
    virtual ~CursesListener();
    virtual void handleEvent(shared_ptr<Event> event) override;
};
}



#endif /* CURSESLISTENER_H_ */

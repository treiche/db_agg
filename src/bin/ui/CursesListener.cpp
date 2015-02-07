/*
 * CursesListener.cpp
 *
 *  Created on: Jan 24, 2014
 *      Author: arnd
 */

#include "CursesListener.h"

#include "utils/logging.h"
#include "utils/string.h"


extern "C" {
#include "cursesw.h"
#include <signal.h>
}

#include <thread>
#include <mutex>
#include <chrono>

#include "utils/utility.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
DECLARE_LOGGER("CursesListener");

static std::thread *clockThread;
static std::mutex screenMutex;
static bool running = true;


Column::Column(ColumnType type, std::string label, size_t requestedWidth, size_t minimalWidth, bool leftJustified):
    type(type),
    label(label),
    width(requestedWidth),
    requestedWidth(requestedWidth),
    minimalWidth(minimalWidth),
    leftJustified(leftJustified) {
    assert(requestedWidth >= minimalWidth);
}


CursesListener::CursesListener(Application& application): application(application) {

}

CursesListener::~CursesListener() {

}

void CursesListener::handleEvent(shared_ptr<Event> event) {

    LOG_DEBUG( "handleEvent " << (int)event->type);
    if (event->type == EventType::APPLICATION_INITIALIZED) {
        screenMutex.lock();
        //setlocale(LC_ALL, "");
        initscr();
        timeout(10);
        keypad(stdscr, TRUE);
        nonl();
        cbreak();
        noecho();
        curs_set(0);
        if (has_colors()) {
            start_color();
            init_pair(1, COLOR_WHITE, COLOR_BLACK);
            init_pair(2, COLOR_GREEN, COLOR_BLACK);
            init_pair(3, COLOR_RED, COLOR_BLACK);
            init_pair(4, COLOR_RED, COLOR_WHITE);
        }
        attrset(COLOR_PAIR(1));
        calculateLayout();
        refresh();
        wrefresh(stdscr);
        screenMutex.unlock();
        clockThread = new std::thread(&CursesListener::updateClock,this);
        initialized = true;

    } else if (event->type == EventType::APPLICATION_FINISHED || event->type == EventType::APPLICATION_FAILED || event->type == EventType::APPLICATION_CANCELED) {
        LOG_ERROR( "got finish event "<<(int)event->type);
        running = false;
        if (initialized) {
            clockThread->join();
        }
        if (event->type == EventType::APPLICATION_FAILED) {
            auto e = (ApplicationFailedEvent*)event.get();
            if (initialized) {
                attrset(COLOR_PAIR(3));
                if (e->reason.empty()) {
                    mvaddstr(statusOffset,0,"processing failed.\npress any key to exit...");
                } else {
                    string message = "processing failed: " + e->reason + "\npress any key to exit...";
                    mvaddstr(statusOffset,0,message.c_str());
                }
            } else {
                cout << "ERROR: " << e->reason << endl;
            }
        } else if (event->type == EventType::APPLICATION_CANCELED) {
            attrset(COLOR_PAIR(3));
            mvaddstr(statusOffset,0,"processing canceled.\npress any key to exit...");
        } else {
            attrset(COLOR_PAIR(2));
            mvaddstr(statusOffset,0,"processing done.\npress any key to exit...");
        }
        curs_set(1);
        timeout(-1);
        wgetch(stdscr);
        endwin();
    } else if (event->type == EventType::EXECUTION_STATE_CHANGE) {
        ExecutionStateChangeEvent *e = (ExecutionStateChangeEvent*)event.get();
        assert(!e->resultId.empty());
        assert(!e->state.empty());
        if (e->state=="CONNECTED") {
            timeSpent[e->resultId] = std::chrono::system_clock::now();
        } else if (e->state=="DONE") {
            timeSpent.erase(e->resultId);
        }
        screenMutex.lock();
        if (e->state=="DONE") {
            attrset(COLOR_PAIR(2));
        } else if (e->state=="FAILED") {
            attrset(COLOR_PAIR(3));
        }
        print(e->resultId, ColumnType::STATUS,e->state);
        attrset(COLOR_PAIR(1));
        wrefresh(stdscr);
        screenMutex.unlock();
    } else if (event->type == EventType::RECEIVE_DATA) {
        auto e = (ReceiveDataEvent*)event.get();
        LOG_DEBUG("recive data event for result " << e->resultId  << " received " << e->rowsReceived);
        screenMutex.lock();
        print(e->resultId,ColumnType::RECEIVED,thousand_grouping(e->rowsReceived));
        attrset(COLOR_PAIR(1));
        wrefresh(stdscr);
        screenMutex.unlock();
    } else if (event->type == EventType::SENT_DATA) {
        auto e = (SentDataEvent*)event.get();
        LOG_DEBUG("sent data event for result " << e->resultId  << " received " << e->rowsSent);
        screenMutex.lock();
        print(e->resultId,ColumnType::SENT,thousand_grouping(e->rowsSent));
        attrset(COLOR_PAIR(1));
        wrefresh(stdscr);
        screenMutex.unlock();
    } else if (event->type == EventType::CACHE_LOADED) {
        auto e = (CacheLoadEvent*)event.get();
        screenMutex.lock();
        print(e->resultId,ColumnType::LAST_EXECUTED,e->lastExecuted.to_string());
        string lds = Time::getDuration(e->lastDuration);
        print(e->resultId,ColumnType::LAST_DURATION,lds);
        print(e->resultId,ColumnType::LAST_RECEIVED,thousand_grouping(e->lastRowsReceived));
        wrefresh(stdscr);
        screenMutex.unlock();
    }
}

void CursesListener::print(std::string resultId, ColumnType colType, std::string value) {
    if (resultIdToPosition.find(resultId) == resultIdToPosition.end()) {
        return;
    }
    size_t width = 0;
    bool leftJustified = false;
    bool show = false;
    for (auto& column:columns) {
        if (column.type == colType) {
            width = column.width;
            leftJustified = column.leftJustified;
            show = column.show;
            break;
        }
    }
    position& pos = resultIdToPosition[resultId][colType];
    string widthAsString = to_string(width - 2);
    string format = " %";
    if (leftJustified) {
        format += "-";
    }
    format.append(widthAsString + "." + widthAsString + "s ");
    char buf[256];
    sprintf(buf,format.c_str(),value.c_str());
    if (show) {
        mvaddstr(pos.line,pos.col,buf);
    }
    resultIdToPosition[resultId][colType].value = string(buf);
}


void CursesListener::updateClock() {
    using namespace chrono;
    // LOG_DEBUG("updateClock ");
    while(running) {
        screenMutex.lock();
        for (auto& p:timeSpent) {
            auto current = std::chrono::system_clock::now();
            auto delta = current - p.second;
            auto rest = (delta % hours(1));
            long m = (rest / minutes(1));
            rest = (rest % minutes(1));
            long s = rest / seconds(1);
            rest = (rest % seconds(1));
            long ms = rest / milliseconds(1);
            rest = (rest % milliseconds(1));
            string fmt = string_format(" %02d:%02d.%03d ",m,s,ms);
            print(p.first,ColumnType::TIME_SPENT,fmt);
        }
        wrefresh(stdscr);
        screenMutex.unlock();
        int c = wgetch(stdscr);
        if (c != -1) {
            if (c == 410) {
                screenMutex.lock();
                clear();
                calculateLayout();
                refresh();
                wrefresh(stdscr);
                screenMutex.unlock();
            }
        }
        this_thread::yield();
        //this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool CursesListener::calculateRequiredSpace() {
    size_t lines = 0;
    size_t cols = 0;
    size_t availableLines = LINES - 4;
    ExecutionGraph& executionGraph = application.getExecutionGraph();
    vector<Query*> queries = executionGraph.getQueries();
    for (auto query:queries) {
        if (!compactView) {
            lines++;
        }
        lines += executionGraph.getQueryExecutions(query).size();
    }
    for (size_t idx = 0; idx < columns.size(); idx++) {
        if (columns[idx].show) {
            cols += columns[idx].width;
        }
    }
    if (lines > availableLines) {
        clusterNo = lines / availableLines;
        if (lines % availableLines > 0) {
            clusterNo++;
        }
        LOG_INFO("calculate number of needed clusters: " << clusterNo)
        clusterWidth = COLS / clusterNo;
        LOG_INFO("calculate cluster width is " << clusterWidth);
        if (cols > clusterWidth) {
            return false;
        }
    } else {
        clusterNo = 1;
    }
    LOG_INFO("calculated required space: [" << lines << "," << cols << "] cluster = " << clusterWidth);
    statusOffset = LINES - 2;
    return true;
}

void CursesListener::calculateLayout() {
    size_t availableLines = LINES - 4;
    LOG_INFO("calculate layout for screen [" << LINES << "," << COLS << "]");

    for (auto& colDef:columns) {
        colDef.show = true;
    }

    while (!calculateRequiredSpace()) {
        for (size_t idx = columns.size() - 1; idx >= 0; idx--) {
            auto colDef = &columns[idx];
            if (colDef->show) {
                LOG_INFO("calculate disable column " << colDef->label)
                colDef->show = false;
                break;
            }
        }
    }

    ExecutionGraph& executionGraph = application.getExecutionGraph();
    vector<Query*> queries = executionGraph.getQueries();

    size_t line = 0;
    size_t relLine = 0;
    for (auto query:queries) {
        if (!compactView) {
        }
        for (auto exec:executionGraph.getQueryExecutions(query)) {
            size_t col = clusterWidth * (line / availableLines);
            for (auto& colDef:columns) {
                if (colDef.show) {
                    position pos;
                    pos.line = relLine + 2;
                    pos.col = col;
                    pos.value = resultIdToPosition[exec->getId()][colDef.type].value;
                    /*
                    if (colDef.type == ColumnType::EXECUTION) {
                        pos.value = exec->getName();
                    } else {
                        pos.value = resultIdToPosition[exec->getId()][colDef.type].value;
                    }
                    */
                    resultIdToPosition[exec->getId()][colDef.type] = pos;
                    if (colDef.type == ColumnType::EXECUTION) {
                        print(exec->getId(),colDef.type,exec->getName());
                    }
                }
                col += colDef.width;
            }
            line++;
            relLine++;
            if (relLine >= availableLines) {
                relLine = 0;
            }
        }
    }

}

void CursesListener::refresh() {
    // print headers
    attrset(COLOR_PAIR(1));
    size_t col = 0;
    size_t visibleColumns = 0;

    for (auto colDef:columns) {
        if (colDef.show) {
            visibleColumns++;
        }
    }

    for (size_t cluster = 0; cluster < clusterNo; cluster++) {
        col = (cluster * clusterWidth);
        size_t vc = 0;
        for (auto colDef:columns) {
            if (colDef.show) {
                LOG_INFO("calculate col = " << col)
                mvaddstr(0,col,colDef.label.c_str());
                for (size_t idx = 0; idx < colDef.width - 1; idx++) {
                    mvaddch(1,col+idx,ACS_HLINE);
                }
                if (vc < visibleColumns - 1) {
                    mvaddch(1,col+colDef.width - 1,ACS_BTEE);
                } else {
                    mvaddch(1,col+colDef.width - 1,ACS_LRCORNER);
                }
                col += colDef.width;
                vc++;
            } else {
                LOG_INFO("calculate col hidden " << colDef.label)
            }
        }
    }

    for (auto r:resultIdToPosition) {
        for (auto c:r.second) {
            mvaddstr(c.second.line, c.second.col, c.second.value.c_str());
        }
    }

    attrset(COLOR_PAIR(1));

}

}

/*
 * CursesListener.cpp
 *
 *  Created on: Jan 24, 2014
 *      Author: arnd
 */

#include "CursesListener.h"

#include <log4cplus/logger.h>


extern "C" {
#include "curses.h"
#include <signal.h>
}

#include <thread>
#include <mutex>
#include <chrono>

#include "utils/utility.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("CursesListener"));

static std::thread *clockThread;
static std::mutex screenMutex;
static bool running = true;

CursesListener::CursesListener(Application& application): application(application) {

}

CursesListener::~CursesListener() {

}

void CursesListener::handleEvent(Event& event) {

    LOG4CPLUS_DEBUG(LOG, "handleEvent " << (int)event.type);
    if (event.type == EventType::APPLICATION_INITIALIZED) {
        screenMutex.lock();
        setlocale(LC_ALL, "");
        initscr();
        keypad(stdscr, TRUE);
        nonl();
        cbreak();
        echo();
        curs_set(0);
        if (has_colors()) {
            start_color();
            init_pair(1, COLOR_WHITE, COLOR_BLACK);
            init_pair(2, COLOR_GREEN, COLOR_BLACK);
            init_pair(3, COLOR_RED, COLOR_BLACK);
            init_pair(4, COLOR_RED, COLOR_WHITE);
        }
        attrset(COLOR_PAIR(1));

        deque<Query>& queries = application.getQueryParser().getQueries();
        size_t cnt = 2;
        size_t statusOffset = 0;
        for (auto& query:queries) {
            attron(A_BOLD|A_UNDERLINE);
            mvaddstr(cnt,2,query.getName().c_str());
            attroff(A_BOLD|A_UNDERLINE);
            if (query.getName().size()+2 > statusOffset) {
                statusOffset = query.getName().size()+2;
            }
            queryIdToLine[query.getId()] = cnt;
            cnt++;
            for (auto& exec:query.getQueryExecutions()) {
                mvaddstr(cnt,4,exec.getName().c_str());
                if (exec.getName().size()+4 > statusOffset) {
                    statusOffset = exec.getName().size()+4;
                }
                resultIdToLine[exec.getId()] = cnt;
                cnt++;
            }
        }
        for (auto result:resultIdToLine) {
            mvaddstr(result.second,statusOffset+2,"-->");
        }
        columns[0].width = statusOffset + 6;
        mvprintw(0,0,"%s","execution");
        for (size_t idx=1;idx < columns.size(); idx++) {
            columns[idx].offset = columns[idx-1].offset + columns[idx-1].width;
            mvprintw(0,columns[idx].offset,"│%s",columns[idx].label.c_str());
        }
        string secondLine = "━";
        for (size_t col=0; col < columns.size(); col++) {
            for (size_t idx=0; idx < columns[col].width-1;idx++) {
                secondLine += "━";
            }
            if (col < columns.size() -1) {
                secondLine += "┷";
            }
        }
        mvprintw(1,0,"%s",secondLine.c_str());
        wrefresh(stdscr);
        screenMutex.unlock();
        clockThread = new std::thread(&CursesListener::updateClock,this);

    } else if (event.type == EventType::APPLICATION_FINISHED || event.type == EventType::APPLICATION_FAILED || event.type == EventType::APPLICATION_CANCELED) {
        LOG4CPLUS_DEBUG(LOG, "got finish event "<<(int)event.type);
        running = false;
        clockThread->join();
        if (event.type == EventType::APPLICATION_FAILED) {
            ApplicationFailedEvent& e = (ApplicationFailedEvent&)event;
            attrset(COLOR_PAIR(3));
            if (e.reason.empty()) {
                mvaddstr(resultIdToLine.size() + queryIdToLine.size()+3,0,"processing failed.\npress any key to exit...");
            } else {
                string message = "processing failed: " + e.reason + "\npress any key to exit...";
                mvaddstr(resultIdToLine.size() + queryIdToLine.size()+3,0,message.c_str());
            }
        } else if (event.type == EventType::APPLICATION_CANCELED) {
            attrset(COLOR_PAIR(3));
            mvaddstr(resultIdToLine.size() + queryIdToLine.size()+3,0,"processing canceled.\npress any key to exit...");
        } else {
            attrset(COLOR_PAIR(2));
            mvaddstr(resultIdToLine.size() + queryIdToLine.size()+3,0,"processing done.\npress any key to exit...");
        }
        curs_set(1);
        wgetch(stdscr);
        endwin();
    } else if (event.type == EventType::EXECUTION_STATE_CHANGE) {
        ExecutionStateChangeEvent& e = (ExecutionStateChangeEvent&)event;
        assert(!e.state.empty());
        size_t lineNo = resultIdToLine[e.resultId];
        if (e.state=="CONNECTED") {
            timeSpent[lineNo] = std::chrono::system_clock::now();
        } else if (e.state=="DONE") {
            timeSpent.erase(lineNo);
        }
        screenMutex.lock();
        if (e.state=="DONE") {
            attrset(COLOR_PAIR(2));
        } else if (e.state=="FAILED") {
            attrset(COLOR_PAIR(3));
        }
        print(e.resultId, "status",e.state,true);
        attrset(COLOR_PAIR(1));
        wrefresh(stdscr);
        screenMutex.unlock();
    } else if (event.type == EventType::RECEIVE_DATA) {
        ReceiveDataEvent& e = (ReceiveDataEvent&)event;
        LOG4CPLUS_DEBUG(LOG, "recive data event for result " << e.resultId  << " received " << e.rowsReceived);
        screenMutex.lock();
        print(e.resultId,"received",to_string(e.rowsReceived));
        attrset(COLOR_PAIR(1));
        wrefresh(stdscr);
        screenMutex.unlock();
    } else if (event.type == EventType::SENT_DATA) {
        SentDataEvent& e = (SentDataEvent&)event;
        LOG4CPLUS_DEBUG(LOG, "sent data event for result " << e.resultId  << " received " << e.rowsSent);
        screenMutex.lock();
        print(e.resultId,"sent",to_string(e.rowsSent));
        attrset(COLOR_PAIR(1));
        wrefresh(stdscr);
        screenMutex.unlock();
    } else if (event.type == EventType::CACHE_LOADED) {
        CacheLoadEvent& e = (CacheLoadEvent&)event;
        screenMutex.lock();
        print(e.resultId,"last executed",e.lastExecuted.to_string());
        string lds = Time::getDuration(e.lastDuration);
        print(e.resultId,"duration",lds);
        print(e.resultId,"last rcvd",to_string(e.lastRowsReceived));
        wrefresh(stdscr);
        screenMutex.unlock();
    }
}

void CursesListener::print(std::string resultId, std::string col, std::string value, bool leftJustified) {
    size_t lineNo = resultIdToLine[resultId];
    size_t offset = 0;
    size_t width = 0;
    for (auto& column:columns) {
        if (column.label == col) {
            offset = column.offset;
            width = column.width;
            break;
        }
    }
    string format = " %";
    if (leftJustified) {
        format += "-";
    }
    format.append(to_string(width - 2) + "s ");
    mvprintw(lineNo,offset,format.c_str(),value.c_str());
}


void CursesListener::updateClock() {
    using namespace chrono;
    // LOG4CPLUS_DEBUG(LOG, "updateClock ");
    while(running) {
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
            size_t offset = 0;
            for (auto col:columns) {
                if (col.label == "time spent") {
                    offset = col.offset;
                }
            }
            screenMutex.lock();
            mvaddstr(p.first,offset,fmt.c_str());
            screenMutex.unlock();
        }
        screenMutex.lock();
        wrefresh(stdscr);
        screenMutex.unlock();
        this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}


}

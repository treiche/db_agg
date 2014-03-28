/*
 * SignalHandler.h
 *
 *  Created on: Jan 26, 2014
 *      Author: arnd
 */

#ifndef SIGNALHANDLER_H_
#define SIGNALHANDLER_H_

#include <stdexcept>
#include <deque>

namespace db_agg {
class SignalHandler {
public:
    virtual void handleSignal(int signal) = 0;
    virtual ~SignalHandler() {};
};

class CancelException: public std::runtime_error {
public:
    CancelException(std::string what): runtime_error(what) {}
};


class GlobalSignalHandler: public SignalHandler {
private:
    static GlobalSignalHandler instance;
    std::deque<SignalHandler*> childs;
public:
    static GlobalSignalHandler& getInstance();
    virtual ~GlobalSignalHandler() override;
    virtual void handleSignal(int signal);
    void addHandler(SignalHandler *handler);
};

}

#endif /* SIGNALHANDLER_H_ */

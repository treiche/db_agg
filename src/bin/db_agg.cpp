/*
 * db_agg.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: arnd
 */

#include <iostream>
#include <getopt.h>

#include <log4cplus/configurator.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/fileappender.h>


#include "installation.h"
#include "db_agg.h"
#include "cli/db_agg_parser.h"
#include "utils/SignalHandler.h"
#include <signal.h>
#include <string>
#include "ui/CursesListener.h"

using namespace std;
using namespace db_agg;
using namespace log4cplus;
using namespace log4cplus::helpers;

static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("db_agg"));

void initLogging(std::string logConf) {
    PropertyConfigurator::doConfigure(logConf);
}

void initLogging(std::string logFile, std::string logLevel) {
    SharedObjectPtr<Appender> append_1(new FileAppender(logFile));
    append_1->setName(LOG4CPLUS_TEXT("A1"));
    tstring pattern = LOG4CPLUS_TEXT("%-5p [%-30l] %m%n");
    append_1->setLayout( std::auto_ptr<Layout>(new PatternLayout(pattern)) );
    Logger::getRoot().addAppender(append_1);
    if (logLevel.compare("TRACE")==0) {
        Logger::getRoot().setLogLevel(TRACE_LOG_LEVEL);
    } else if (logLevel.compare("DEBUG")==0) {
        Logger::getRoot().setLogLevel(DEBUG_LOG_LEVEL);
    } else if (logLevel.compare("INFO")==0) {
        Logger::getRoot().setLogLevel(INFO_LOG_LEVEL);
    } else if (logLevel.compare("WARN")==0) {
        Logger::getRoot().setLogLevel(WARN_LOG_LEVEL);
    } else if (logLevel.compare("ERROR")==0) {
        Logger::getRoot().setLogLevel(ERROR_LOG_LEVEL);
    } else if (logLevel.compare("FATAL")==0) {
        Logger::getRoot().setLogLevel(FATAL_LOG_LEVEL);
    } else {
        throw runtime_error("invalid log level '" + logLevel + "'");
    }
}

void disableConsoleLogging() {
    SharedAppenderPtrList appenders = Logger::getRoot().getAllAppenders();
    for (auto appender:appenders) {
        ConsoleAppender *ca = dynamic_cast<ConsoleAppender*>(appender.get());
        if (ca != nullptr) {
            Logger::getRoot().removeAppender(appender);
        }
    }
}

void cancel(int param) {
    LOG_DEBUG("cancel db_agg");
    GlobalSignalHandler::getInstance().handleSignal(param);
}


int main(int argc, char **argv) {
    string defaultLogConfig = DBAGG_SYSCONFDIR;
    defaultLogConfig += "/log4cplus.properties";
    initLogging(defaultLogConfig);
    Configuration config;
    db_agg::db_agg_parser parser;
    parser.parse(argc,argv,config);
    string logConf = Application::findConfigurationFile(config.getLogConf(),false,false);
    initLogging(logConf);
    config.setUseRegExpParser(true);
    config.setSearchPasswordInPgPass(true);
    // parser.dumpConfiguration(config);
    signal(SIGINT,&cancel);
    signal(SIGHUP,&cancel);
    Application& app = Application::getInstance();
    app.bootstrap(config);
    CursesListener *cl = nullptr;
    if (config.getShowProgress()) {
        disableConsoleLogging();
        cl = new CursesListener(app);
        app.addEventListener(cl);
    }
    GlobalSignalHandler::getInstance().addHandler(&app);
    app.run();
    if (cl!=nullptr) {
        delete cl;
    }
}


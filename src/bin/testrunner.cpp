/*
 * Main.cpp
 *
 *  Created on: Oct 24, 2014
 *      Author: arnd
 */

#include "gtest/gtest.h"
#include <log4cplus/logger.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h>


using ::testing::InitGoogleTest;
using namespace log4cplus;
using namespace log4cplus::helpers;

void initLogging() {
    // PropertyConfigurator::doConfigure("src/test/resources/log4cplus.properties");
    SharedObjectPtr<Appender> append_1(new FileAppender("testrunner.log"));
    append_1->setName(LOG4CPLUS_TEXT("First"));
    //log4cplus::tstring pattern = LOG4CPLUS_TEXT("%d{%m/%d/%y %H:%M:%S,%Q} [%t] %-5p %c{2} %%%x%% - %m [%l]%n");
    tstring pattern = LOG4CPLUS_TEXT("%-5p [%-30l] %m%n");
    append_1->setLayout( std::auto_ptr<Layout>(new PatternLayout(pattern)) );
    Logger::getRoot().addAppender(append_1);
    Logger::getRoot().setLogLevel(DEBUG_LOG_LEVEL);
}


int main(int argc, char **argv) {
	initLogging();
	InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}




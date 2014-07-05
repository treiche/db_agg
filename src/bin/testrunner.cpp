/* Generated file, do not edit */

#ifndef CXXTEST_RUNNING
#define CXXTEST_RUNNING
#endif

#define _CXXTEST_HAVE_STD
#define _CXXTEST_HAVE_EH
#include <cxxtest/TestListener.h>
#include <cxxtest/TestTracker.h>
#include <cxxtest/TestRunner.h>
#include <cxxtest/RealDescriptions.h>
#include <cxxtest/TestMain.h>
#include <cxxtest/ErrorPrinter.h>
#include <cxxtest/XmlPrinter.h>
#include <cxxtest/Descriptions.h>
#include "utils/logging.h"
#include <log4cplus/consoleappender.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/ndc.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/threads.h>
#include <log4cplus/helpers/sleep.h>

#include <log4cplus/configurator.h>
#include <iostream>
#include <string>

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;

class TestListenerImpl : public CxxTest::ErrorPrinter {
private:
    Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("TestListenerImpl"));
public:
    virtual void enterTest(const CxxTest::TestDescription & desc) override {
        LOG4CPLUS_DEBUG(logger, "enterTest " << desc.testName());
        // CxxTest::ErrorPrinter::enterTest(desc);
    }
    virtual void enterWorld(const CxxTest::WorldDescription & desc) override {
        std::cout << "enterWorld " << desc.worldName() << std::endl;
    }
    virtual void enterSuite(const CxxTest::SuiteDescription & desc) override {
        LOG4CPLUS_DEBUG(logger, "enterSuite " << desc.suiteName());
    }
};

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

int main( int argc, char *argv[] ) {
    initLogging();
    Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("main"));
    LOG4CPLUS_DEBUG(logger, "run testrunner ");
    for (int idx=0;idx<argc;idx++) {
        LOG4CPLUS_INFO(logger, "arg " << idx << " = " << argv[idx]);
    }
    LOG4CPLUS_DEBUG(logger, "start running tests ...");

    int status;
    CxxTest::ErrorPrinter tmp;
    TestListenerImpl listener;
    CxxTest::setAbortTestOnFail(true);
    status = CxxTest::Main<TestListenerImpl>( listener, argc, argv );
    return status;
}
#include <cxxtest/Root.cpp>
const char* CxxTest::RealWorldDescription::_worldName = "cxxtest";

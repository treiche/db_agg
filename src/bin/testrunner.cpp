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
#include <iostream>
#include <libgen.h>
#include <unistd.h>
#include "testrunner.h"

using ::testing::InitGoogleTest;
using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace std;


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

string calculateHomePath(string testrunnerLocation) {
    char *ap = ::realpath(testrunnerLocation.c_str(),nullptr);
    char *dirName = dirname(ap);
    return string(dirName);
}

string findTestDir(string homeDir) {
    struct stat info;
    string sameDir = homeDir + "/tests";
    int ret = stat(sameDir.c_str(), &info);
    if (ret==0 && S_ISDIR(info.st_mode)) {
        return sameDir;
    } else {
        // search one level up
        string::size_type idx = homeDir.rfind("/");
        if (idx == string::npos) {
            throw runtime_error("can't find test directory");
        }
        return findTestDir(homeDir.substr(0,idx));
    }
}

std::string TEST_DATA_DIR;

int main(int argc, char **argv) {
    string homeDir = calculateHomePath(argv[0]);
    TEST_DATA_DIR = findTestDir(homeDir);
    initLogging();
    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}




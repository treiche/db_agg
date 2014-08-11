/*
 * logging.h
 *
 *  Created on: Jul 4, 2014
 *      Author: arnd
 */

#ifndef LOGGING_H_
#define LOGGING_H_

#include <log4cplus/logger.h>

#define DECLARE_LOGGER(name) \
    static log4cplus::Logger LOG = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT(name));


#define LOG_TRACE(msg) \
    LOG4CPLUS_TRACE(LOG,msg);

#define LOG_DEBUG(msg) \
    LOG4CPLUS_DEBUG(LOG,msg);

#define LOG_INFO(msg) \
    LOG4CPLUS_INFO(LOG,msg);

#define LOG_WARN(msg) \
    LOG4CPLUS_WARN(LOG,msg);

#define LOG_ERROR(msg) \
    LOG4CPLUS_ERROR(LOG,msg);

#define THROW_EXC(msg) \
    stringstream ss; \
    ss << msg; \
    LOG4CPLUS_FATAL(LOG,ss.str()); \
    throw runtime_error(ss.str());

#endif /* LOGGING_H_ */

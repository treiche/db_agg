/*
 * logging.h
 *
 *  Created on: Jul 4, 2014
 *      Author: arnd
 */

#ifndef LOGGING_H_
#define LOGGING_H_

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
    LOG4CPLUS_FATAL(LOG,msg); \
    stringstream ss; \
    ss << msg; \
    throw runtime_error(ss.str());

#endif /* LOGGING_H_ */

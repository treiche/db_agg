/*
 * Locator.cpp
 *
 *  Created on: Dec 29, 2013
 *      Author: arnd
 */


#include "core/Locator.h"

#include <stdexcept>
#include <log4cplus/logger.h>

using namespace std;
using namespace log4cplus;

namespace db_agg {
    static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("Locator"));
    string Locator::defaultEnvironment;

    Locator::Locator(string name, short shardId, string environment):
            name(name),
            shardId(shardId),
            environment(environment) {}

    short Locator::compare(Locator& other) {
        LOG4CPLUS_DEBUG(LOG,"compare " << getQName() << " and " << other.getQName());
        if (other.name.compare(name) != 0) {
            LOG4CPLUS_DEBUG(LOG,name << " != " << other.name);
            return -1;
        }
        string thisEnv = environment;
        string otherEnv = other.environment;
        if (environment.empty()) {
            LOG4CPLUS_DEBUG(LOG, "this env empty, set to '" << Locator::defaultEnvironment << "'");
            thisEnv = Locator::defaultEnvironment;
        }
        if (other.environment.empty()) {
            LOG4CPLUS_DEBUG(LOG, "other env empty, set to '" << Locator::defaultEnvironment << "'");
            otherEnv = Locator::defaultEnvironment;
        }
        if (thisEnv.compare(otherEnv) != 0) {
            LOG4CPLUS_DEBUG(LOG,"'" << thisEnv << "' != '" << otherEnv << "'");
            return -1;
        }
        if (shardId==other.shardId) {
            LOG4CPLUS_DEBUG(LOG,shardId << " == " << other.shardId);
            return 0;
        }
        if (shardId==-1 && other.shardId!=-1) {
            LOG4CPLUS_DEBUG(LOG,shardId << " < " << other.shardId);
            return other.shardId;
        }
        if (shardId!=-1 && other.shardId==-1) {
            LOG4CPLUS_DEBUG(LOG,shardId << " > " << other.shardId);
            return shardId;
        }
        return -1;
    }

    void Locator::setDefaultEnvironment(string env) {
        Locator::defaultEnvironment = env;
    }

    string Locator::getQName() {
        string qname = name;
        if (shardId!=-1) {
            qname += "$" + to_string(shardId);
        }
        if (!environment.empty()) {
            qname += "$" + environment;
        }
        return qname;
    }

}


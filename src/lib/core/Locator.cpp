/*
 * Locator.cpp
 *
 *  Created on: Dec 29, 2013
 *      Author: arnd
 */


#include "core/Locator.h"

#include <stdexcept>
#include "utils/logging.h"

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
        LOG_DEBUG("compare " << getQName() << " and " << other.getQName());
        if (other.name.compare(name) != 0) {
            LOG_DEBUG(name << " != " << other.name);
            return -1;
        }
        string thisEnv = environment;
        string otherEnv = other.environment;
        if (environment.empty()) {
            LOG_DEBUG("this env empty, set to '" << Locator::defaultEnvironment << "'");
            thisEnv = Locator::defaultEnvironment;
        }
        if (other.environment.empty()) {
            LOG_DEBUG("other env empty, set to '" << Locator::defaultEnvironment << "'");
            otherEnv = Locator::defaultEnvironment;
        }
        if (thisEnv.compare(otherEnv) != 0) {
            LOG_DEBUG("'" << thisEnv << "' != '" << otherEnv << "'");
            return -1;
        }
        if (shardId==other.shardId) {
            LOG_DEBUG(shardId << " == " << other.shardId);
            return 0;
        }
        if (shardId==-1 && other.shardId!=-1) {
            LOG_DEBUG(shardId << " < " << other.shardId);
            return other.shardId;
        }
        if (shardId!=-1 && other.shardId==-1) {
            LOG_DEBUG(shardId << " > " << other.shardId);
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

    const string& Locator::getEnvironment() const {
        return environment;
    }

    const string& Locator::getName() const {
        return name;
    }

    short Locator::getShardId() const {
        return shardId;
    }


}


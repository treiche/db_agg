/*
 * QueryParser.cpp
 *
 *  Created on: Aug 29, 2014
 *      Author: arnd
 */


#include "QueryParser.h"
#include "utils/logging.h"
#include "utils/RegExp.h"
#include "utils/string.h"

using namespace std;

namespace db_agg {

DECLARE_LOGGER("QueryParser")

static RegExp NS_EXTRACT(R"((?<!distinct )(table|from|join)\s+([a-z_0-9]+)\.[a-z_]+\s*)");


string QueryParser::normalizeQuery(std::string query) {
    string normalized;
    bool isWhitespace = false;
    char lastChar;
    for (size_t idx = 0; idx < query.size(); idx++) {
        char c = query[idx];
        if (isspace(c) && isspace(lastChar)) {
            // skip
        } else {
            if (isspace(c)) {
                normalized += ' ';
            } else {
                normalized += c;
            }
        }
        lastChar = c;
    }
    return normalized;
}


set<string> QueryParser::extractUsedNamespaces(std::string query) {
    set<string> usedNamespaces;
    int offset = 0;
    vector<string> matches;
    while(NS_EXTRACT.find(query,matches,offset)) {
        usedNamespaces.insert(matches[2]);
    }
    return usedNamespaces;
}


Query* QueryParser::getSourceQuery(Dependency dep, vector<Query*>& queries) {
    Query *src = nullptr;
    for (auto query:queries) {
        int diff = query->getLocator().compare(dep.locator);
        if (diff>-1) {
            LOG_DEBUG("found candidate '" << query->getLocator().getQName());
            src = query;
        }
        if (src==nullptr) {
            // search for inter system transition
            if (query->getName().compare(dep.locator.getName())==0) {
                if (!query->getEnvironment().empty() &&
                    dep.locator.getEnvironment().empty()) {
                    src = query;
                }
            }
        }
    }
    LOG_TRACE("getSourceQuery(" << dep.locator.getQName() << ") = " << (Query*)src);
    return src;
}

void QueryParser::detectDependencies(vector<Query*>& queries) {
    string re = "(from|join)\\s+(";
    int cnt=0;
    int len = queries.size();
    for (size_t idx = 0; idx < queries.size(); idx++) {
        re += queries[idx]->getName();
        if (cnt<len-1) {
            re += "|";
        }
        cnt++;
    }
    re+=")\\$?([0-9]*)\\$?([a-zA-Z0-9_]*)(\\s+([a-z0-9_$]+))?";
    LOG_DEBUG("REGEXP = " << re);
    RegExp regexp(re);
    for (auto q:queries) {
        string qs = q->getQuery();
        vector<string> matches;
        int offset = 0;
        while(regexp.find(qs,matches,offset)) {
            LOG_DEBUG("found dependency " << matches[2] << " in " << q->getName());
            string name = matches[2];
            string alias = "";
            short shardId = -1;
            string environment;
            if (matches.size()>3) {
                if (matches[3].compare("")!=0) {
                    shardId = atoi(matches[3].c_str());
                }
            }
            if (matches.size()>4) {
                if (matches[4].compare("")!=0) {
                    environment = matches[4];
                }
            }
            if (matches.size()>5) {
                // TODO: find better solution
                if (matches[5].compare("join")!=0) {
                    alias = matches[5];
                }
            }
            LOG_DEBUG("with name = " << name);
            LOG_DEBUG("with shard = " << shardId);
            LOG_DEBUG("with environment = " << environment);
            LOG_DEBUG("with alias = " << alias);
            Locator loc(name,shardId,environment);
            q->addDependency(loc, alias);
        }
        for (auto& dep:q->getDependencies()) {
            LOG_DEBUG("detected dependency " + dep.locator.getQName());
        }
    }
}

void QueryParser::detectScriptQueries(vector<Query*>& queries, vector<string>& functions) {
    string expr = "(" + join(functions,"|") + ")\\(([a-zA-Z_0-9, ]+)\\)";
    RegExp re(expr);
    for (auto query:queries) {
        int offset = 0;
        vector<string> matches;
        while(re.find(query->getQuery(),matches,offset)) {
            string executorName = matches[1];
            string arguments = matches[2];
            vector<string> scriptArgs;
            split(arguments,',',scriptArgs);
            for (auto& arg:scriptArgs) {
                trim(arg);
            }
            query->setType(executorName);
            query->setArguments(scriptArgs);
        }
    }
}

void QueryParser::extractMetaData(std::vector<Query*>& queries) {
    RegExp re{"\\@([a-z_]+)\\s*:\\s*([a-zA-Z0-9_,]+)"};
    for (auto query:queries) {
        LOG_INFO("extract metadata for query " << query->getQuery());
        int offset = 0;
        vector<string> matches;
        map<string,string> metaData;
        while(re.find(query->getQuery(),matches,offset)) {
            LOG_INFO("found meta data " << matches[1] << " = " << matches[2]);
            metaData[matches[1]] = matches[2];
        }
        query->setMetaData(metaData);
    }
}


}

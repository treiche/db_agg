#include "RegExpQueryParser.h"

#include "utils/logging.h"
#include <cstdlib>
#include <iostream>
#include <vector>

#include "utils/utility.h"
#include "utils/md5.h"
#include "core/Query.h"
#include "utils/RegExp.h"
#include "utils/Template.h"


using namespace std;
using namespace log4cplus;

namespace db_agg {
static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("RegExpQueryParser"));


static RegExp CTE_EXTRACT(R"((WITH\s+|,\s*)([a-z_0-9]+)\$?([0-9]{0,1})\$?([a-z]*)\s+AS\s+\((.+?)\n\))");
static RegExp NS_EXTRACT(R"((?<!distinct )(table|from|join)\s+([a-z_0-9]+)\.[a-z_]+\s*)");

RegExpQueryParser::RegExpQueryParser() {
}

RegExpQueryParser::~RegExpQueryParser() {
}

set<string> RegExpQueryParser::extractUsedNamespaces(std::string query) {
    set<string> usedNamespaces;
    int offset = 0;
    vector<string> matches;
    while(NS_EXTRACT.find(query,matches,offset)) {
        usedNamespaces.insert(matches[2]);
    }
    return usedNamespaces;
}

vector<Query*> RegExpQueryParser::parse(string q, map<string,string>& externalSources, map<string,string>& queryParameter, vector<string> functions) {
    assert(!q.empty());
    vector<Query*> queries;
    LOG_DEBUG("start query parsing");
    int offset = 0;
    vector<string> matches;
    set<string> locs;
    string query = q;
    // replace query parameters
    if (!queryParameter.empty()) {
        Template t;
        t.set(queryParameter);
        query = t.render(q);
    }
    while(CTE_EXTRACT.find(query,matches,offset)) {
        string environment = matches[4];
        string sql = trim(matches[5]);
        string formattedSql = cutBlock(matches[5]);
        set<string> usedNamespaces = extractUsedNamespaces(sql);
        //Query *q = new Query(matches[2], tq,usedNamespaces);
        short shardId = -1;
        if (trim(matches[3]).compare("")!=0) {
            shardId = atoi(trim(matches[3]).c_str());
        }
        string id = string(md5hex(matches[2] + "$" + to_string(shardId) + "$" + environment + ":" + sql));
        //pImpl->queryMap[matches[2]] = Query(id, matches[2], tq, usedNamespaces, shardId, environment);
        Locator loc(matches[2], shardId, environment);
        if (locs.find(loc.getQName())!=locs.end()) {
            throw runtime_error("duplicate locator " + loc.getQName());
        }
        locs.insert(loc.getQName());
        queries.push_back(new Query(id, "postgres", loc, sql, formattedSql, usedNamespaces));
    }
    offset++;

    string mainQuery(query, offset + 1, query.size() - offset);
    string sql = mainQuery;
    string formattedSql = cutBlock(mainQuery);
    LOG_DEBUG("main query = '" << formattedSql << "'");
    set<string> usedNamespaces = extractUsedNamespaces(sql);
    //Query *q = new Query("__main_query__", tq, usedNamespaces);
    string id = string(md5hex("__main_query__$-1:" + sql));
    //pImpl->queryMap["__main_query__"] = Query(id, "__main_query__", tq, usedNamespaces, -1, "");
    Locator loc("__main_query__",-1,"");
    queries.push_back(new Query(id, "postgres", loc, sql, formattedSql, usedNamespaces));

    // create pseudo entries for external sources
    LOG_DEBUG("create " << externalSources.size() << "pseudo entries");
    for (auto& externalSource:externalSources) {
        LOG_DEBUG("create pseudo queries for external source " << externalSource.first);
        Locator loc(externalSource.first,-1,"");
        string id = string(md5hex(externalSource.first + ":" + externalSource.second + "$-1:"));
        set<string> empty;
        queries.push_back(new Query(id, "resource", loc, externalSource.second, externalSource.second, empty));
    }

    detectDependencies(queries);

    for (auto query:queries) {
        LOG_DEBUG("    "  << query->toString());
        for (auto& dep:query->getDependencies()) {
            Query *src = getSourceQuery(dep, queries);
            if (src==nullptr) {
                throw runtime_error("no source found for dependency " + dep.locator.getName());
            }
            dep.sourceQuery = src;
            LOG_DEBUG(query->getLocator().getQName() << ": " << dep.locator.getQName() << " -> " << src->getLocator().getQName());
        }
    }

    detectScriptQueries(queries,functions);

    extractMetaData(queries);
    return queries;
}

void RegExpQueryParser::detectDependencies(vector<Query*>& queries) {
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

void RegExpQueryParser::detectScriptQueries(vector<Query*>& queries, vector<string>& functions) {
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

void RegExpQueryParser::extractMetaData(std::vector<Query*>& queries) {
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

Query* RegExpQueryParser::getSourceQuery(Dependency dep, vector<Query*>& queries) {
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

}

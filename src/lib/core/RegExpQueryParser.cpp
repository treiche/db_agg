#include "RegExpQueryParser.h"

#include "utils/logging.h"
#include <cstdlib>
#include <iostream>
#include <vector>

#include "utils/utility.h"
#include "utils/File.h"
#include "utils/string.h"
#include "utils/md5.h"
#include "core/Query.h"
#include "utils/RegExp.h"
#include "utils/Template.h"


using namespace std;

namespace db_agg {
DECLARE_LOGGER("RegExpQueryParser");


static RegExp CTE_EXTRACT(R"((WITH\s+|,\s*)([a-z_0-9]+)\$?([0-9]{0,1})\$?([a-z]*)\s+AS\s+\((.+?)\n\))");

RegExpQueryParser::RegExpQueryParser() {
}

RegExpQueryParser::~RegExpQueryParser() {
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
        string normalizedSql = normalizeQuery(matches[5]);
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
        queries.push_back(new Query(id, "postgres", loc, sql, formattedSql, normalizedSql, usedNamespaces));
    }
    offset++;

    string mainQuery(query, offset + 1, query.size() - offset);
    string sql = mainQuery;
    string formattedSql = cutBlock(mainQuery);
    string normalizedSql = cutBlock(mainQuery);
    LOG_DEBUG("main query = '" << formattedSql << "'");
    set<string> usedNamespaces = extractUsedNamespaces(sql);
    //Query *q = new Query("__main_query__", tq, usedNamespaces);
    string id = string(md5hex("__main_query__$-1:" + sql));
    //pImpl->queryMap["__main_query__"] = Query(id, "__main_query__", tq, usedNamespaces, -1, "");
    Locator loc("__main_query__",-1,"");
    queries.push_back(new Query(id, "postgres", loc, sql, formattedSql, normalizedSql, usedNamespaces));

    // create pseudo entries for external sources
    LOG_DEBUG("create " << externalSources.size() << "pseudo entries");
    for (auto& externalSource:externalSources) {
        LOG_DEBUG("create pseudo queries for external source " << externalSource.first);
        Locator loc(externalSource.first,-1,"");
        string id = string(md5hex(externalSource.first + ":" + externalSource.second + "$-1:"));
        set<string> empty;
        /*
        File resourceFile(externalSource.second);
        if (!resourceFile.exists()) {
            THROW_EXC("the resource '" << externalSource.second << "' does not exist.");
        }
        string absolutePath = resourceFile.abspath();
        */
        queries.push_back(new Query(id, "resource", loc, externalSource.second, externalSource.second, externalSource.second, empty));
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





}

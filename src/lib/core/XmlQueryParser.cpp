/*
 * XmlQueryParser.cpp
 *
 *  Created on: Aug 28, 2014
 *      Author: arnd
 */

#include "XmlQueryParser.h"
#include "utils/logging.h"
#include "utils/md5.h"
#include "utils/string.h"

using namespace std;



namespace db_agg {

DECLARE_LOGGER("XmlQueryParser")

XmlQueryParser::~XmlQueryParser() {
}


vector<Query*> XmlQueryParser::parse(string q, map<string,string>& externalSources, map<string,string>& queryParameter, vector<string> functions) {
    xmlDocPtr doc = xmlReadMemory(q.c_str(), q.size(), "queries.xml", NULL, 0);
    if (doc == nullptr) {
        THROW_EXC("failed to parse xml queries");
    }
    int ret = xmlXIncludeProcess(doc);
    if (ret < 0) {
        THROW_EXC("xinclude failed");
    }
    vector<Query*> queries;

    xmlNodePtr child = doc->children->children;

    while(child) {
        if (child->type == XML_ELEMENT_NODE) {
            Query *q = parseQuery((xmlElementPtr)child);
            queries.push_back(q);
        }
        child = child->next;
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

Query *XmlQueryParser::parseQuery(xmlElementPtr executionNode) {
    map<string,string> properties = getProperties(executionNode);
    for (auto prop:properties) {
        LOG_DEBUG("property: " << prop.first << " = " << prop.second);
    }

    short shardId = -1;
    /*
    if (trim(matches[3]).compare("")!=0) {
        shardId = atoi(trim(matches[3]).c_str());
    }
    string id = string(md5hex(matches[2] + "$" + to_string(shardId) + "$" + environment + ":" + sql));
    */
    string name = properties["name"];
    string query = trim(properties["query"]);
    string formattedSql = cutBlock(properties["query"]);
    string normalizedSql = normalizeQuery(properties["query"]);
    string id = string(md5hex(name + ":" + query));
    string environment;
    string type = properties["type"];
    if (type.empty()) {
        type = "postgres";
    }

    Locator loc(name,shardId,environment);

    set<string> usedNamespaces = extractUsedNamespaces(query);
    string uns = properties["usedNamespaces"];
    vector<string> unv;
    split(uns,',',unv);
    for (auto un:unv) {
        usedNamespaces.insert(un);
    }

    Query *q = new Query(id,type,loc,query,formattedSql,normalizedSql,usedNamespaces);

    string depends = properties["depends"];
    vector<string> dependencies;
    split(depends,',',dependencies);
    for (auto dep:dependencies) {
        Locator dloc(dep,-1,"");
        LOG_DEBUG("add dependency " << dep);
        q->addDependency(dloc,"");
    }

    return q;
}

map<string,string> XmlQueryParser::getProperties(xmlElementPtr element) {
    map<string,string> properties;
    xmlAttribute *attr = element->attributes;
    while(attr) {
        properties[(char*)attr->name] = (char*)attr->children->content;
        attr = (xmlAttribute*)attr->next;
    }
    xmlNodePtr child = element->children;
    while (child) {
        if (child->type == XML_ELEMENT_NODE) {
            xmlElementPtr childElement = (xmlElementPtr)child;
            properties[(char*)childElement->name] = (char*)childElement->children->content;
        }
        child = child->next;
    }
    return properties;
}



}
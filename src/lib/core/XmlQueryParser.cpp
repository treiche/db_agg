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
#include "utils/Template.h"
#include "utils/File.h"
#include "utils/xml.h"

extern "C" {
    #include <libxml/xmlschemas.h>
    #include <libxml/uri.h>
}


using namespace std;

namespace db_agg {

DECLARE_LOGGER("XmlQueryParser")

static void errorHandler(void * ctx, const char * msg, ...) {
    va_list ap;
    va_start(ap, msg);
    char buf[1024];
    vsprintf(buf,msg,ap);
    cout << "ERROR: " << buf << endl;
    LOG_ERROR(buf);
}

static void warningHandler(void * ctx, const char * msg, ...) {
    va_list ap;
    va_start(ap, msg);
    char buf[1024];
    vsprintf(buf,msg,ap);
    cout << "WARNING: " << buf << endl;
    LOG_WARN(buf);
}


XmlQueryParser::~XmlQueryParser() {
}


vector<Query*> XmlQueryParser::parse(string qu, string url, map<string,string>& externalSources, map<string,string>& queryParameter, vector<string> functions) {
    string q = qu;
    // replace query parameters
    if (!queryParameter.empty()) {
        Template t;
        t.set(queryParameter);
        q = t.render(q);
    }

    xmlDocPtr doc = parseDoc(qu,url);

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
                THROW_EXC("no source found for dependency " << dep.locator.getName() + " used in query " << query->getName());
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

    xmlChar *base =  xmlNodeGetBase(executionNode->doc, (xmlNodePtr)executionNode);
    string baseUrl((char*)base);
    xmlFree(base);

    short shardId = -1;
    if (properties.find("shardId") != properties.end()) {
        shardId = stoi(properties["shardId"]);
    }
    string name = properties["name"];
    string query = trim(properties["query"]);
    string formattedSql = cutBlock(properties["query"]);
    string normalizedSql = normalizeQuery(properties["query"]);
    string id = string(md5hex(name + ":" + query));
    string environment;
    string type = properties["type"];
    if (type == "resource") {
        char *absUri = (char*)xmlBuildURI((xmlChar*)query.c_str(),(xmlChar*)baseUrl.c_str());
        query = string(absUri);
        free(absUri);
    }
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

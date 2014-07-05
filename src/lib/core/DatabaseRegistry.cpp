
#include "utils/logging.h"

using namespace std;
using namespace log4cplus;

#include "core/DatabaseRegistry.h"
#include "utils/File.h"
#include "utils/Template.h"

bool hasAttribute(xmlElementPtr element, string attrName) {
    xmlAttribute *attr = element->attributes;
    while(true) {
        if (attrName.compare((const char *)attr->name) == 0) {
            return true;
        }
        attr = (xmlAttribute*)attr->next;
        if (!attr) {
            break;
        }
    }
    return false;
}

string getAttribute(xmlElementPtr element, string attrName) {
    xmlAttribute *attr = element->attributes;
    while(attrName.compare((const char *)attr->name) != 0) {
        attr = (xmlAttribute*)attr->next;
    }
    return string((const char *)attr->children->content);
}

namespace db_agg {
    static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("DatabaseRegistry"));

    DatabaseRegistry::DatabaseRegistry(string regfile) {
        LOG_DEBUG("load database registry from " << regfile);
        File regFile(regfile);
        if (!regFile.exists()) {
            throw runtime_error("no file found " + regfile);
        }
        xmlParserCtxtPtr ctxt = xmlNewParserCtxt();
        if (ctxt==nullptr) {
            THROW_EXC("Failed to allocate parser context");
        }
        xmlDocPtr doc = xmlCtxtReadFile(ctxt, regfile.c_str(), NULL, XML_PARSE_DTDVALID);
        if (doc == NULL) {
            THROW_EXC("parsing failed");
        }
        if (ctxt->valid == 0) {
            xmlFreeDoc(doc);
            THROW_EXC("document is not valid");
        }
        xpathCtx = xmlXPathNewContext(doc);
        this->doc = doc;
        this->databaseNamingStrategy = getDatabaseNamingStrategy();
        xmlFreeParserCtxt(ctxt);
    }

    DatabaseRegistry::~DatabaseRegistry() {
        LOG_TRACE("delete database registry instance");
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        LOG_TRACE("delete database registry instance done");
    }

    vector<string> DatabaseRegistry::getSystems() {
        vector<string> systems;
        string expr("/registry/system/@name");
        xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((const xmlChar*)expr.c_str(), this->xpathCtx);
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        for (int idx=0;idx<nodes->nodeNr;idx++) {
            xmlNodePtr node = nodes->nodeTab[idx];
            if(node->type == XML_ATTRIBUTE_NODE) {
                systems.push_back(string((const char *)node->children->content));
            }
        }
        xmlXPathFreeObject(xpathObj);
        return systems;
    }

    shared_ptr<Url> DatabaseRegistry::getWorker() {
        string expr("//database-instance[@worker]");
        xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((const xmlChar*)expr.c_str(), this->xpathCtx);
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        if (nodes->nodeNr!=1) {
            cout << "nodeNr " << nodes->nodeNr << endl;
            throw runtime_error("can't find worker " + nodes->nodeNr);
        }
        xmlNodePtr node = nodes->nodeTab[0];
        if(node->type == XML_ELEMENT_NODE) {
            shared_ptr<Url> c = getUrl((xmlElementPtr)node);
            xmlXPathFreeObject(xpathObj);
            return c;
        }
        throw runtime_error("can't find worker " + nodes->nodeNr);
    }


    vector<shared_ptr<Url>> DatabaseRegistry::getUrls(string database, string environment, short shardId) {
        assert(!database.empty());
        assert(!environment.empty());
        LOG_DEBUG("called getUrls(" << database << "," << environment << "," << shardId << ")");
        string ss;
        if (shardId == -1) {
            ss += "//system[@name='" + string(environment) + "']//database-instance[@id='" + string(database) + "']";
        } else {
            ss += "//system[@name='" + string(environment) + "']//database-instance[@id='" + string(database) + "' and @shard='" + to_string(shardId) + "']";
        }
        xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((const xmlChar*)ss.c_str(), this->xpathCtx);
        LOG_DEBUG("xpath obj = " << xpathObj);
        if (xpathObj==0) {
            LOG_ERROR(ss);
            THROW_EXC("xpath eval returned 0");
        }
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        
        int found = nodes->nodeNr;
        
        vector<shared_ptr<Url>> urls;
        for (int cnt=0;cnt<found;cnt++) {
            xmlNodePtr node = nodes->nodeTab[cnt];
            if(node->type == XML_ELEMENT_NODE) {
                shared_ptr<Url> c = getUrl((xmlElementPtr)node);
                urls.push_back(c);
            }
        
        }
        LOG_DEBUG("found " << found << " nodes");
        xmlXPathFreeObject(xpathObj);
        return urls;
    }

    string DatabaseRegistry::evaluateXPath(string expr) {
        xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((const xmlChar*)expr.c_str(), this->xpathCtx);
        LOG_DEBUG("xpath obj = " << xpathObj);
        if (xpathObj->nodesetval==0) {
            LOG_ERROR("no result found for '" << expr.c_str() << "'");
            return "";
        }
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        if (nodes->nodeNr != 1) {
            throw runtime_error("unexpected result count " + nodes->nodeNr);
        }
        xmlNodePtr node = nodes->nodeTab[0];
        xmlChar * sval = xmlXPathCastNodeToString(node);
        int len = xmlStrlen(sval);
        string result = string((const char*)sval,(size_t)len);
        xmlFree(sval);
        xmlXPathFreeObject(xpathObj);
        return result;
    }


    string DatabaseRegistry::getShardingStrategyName(std::string databaseId) {
        try {
            return evaluateXPath("/registry/database-definition[@name='" + databaseId + "']/@sharder");
        } catch(runtime_error& re) {
            return "";
        }
    }

    string DatabaseRegistry::getShardColumn(std::string databaseId) {
        return evaluateXPath("/registry/database-definition[@name='" + databaseId + "']/@shardCol");
    }

    string DatabaseRegistry::getDatabaseByNamespace(set<string> namespaces) {
        string ss = "/registry/database-definition[namespace/@name='" + *namespaces.begin() + "']/@name";
        xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((const xmlChar*)ss.c_str(), this->xpathCtx);
        LOG_DEBUG("xpath obj = " << xpathObj);
        if (xpathObj->nodesetval==0) {
            LOG_WARN("no result found for '" << ss.c_str() << "'");
            return "";
        }
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        if (nodes->nodeNr!=1) {
            THROW_EXC("unexpected result count " << nodes->nodeNr);
        }
        xmlNodePtr node = nodes->nodeTab[0];
        xmlChar * sval = xmlXPathCastNodeToString(node);
        int len = xmlStrlen(sval);
        string result = string((const char*)sval,(size_t)len);
        xmlFree(sval);
        xmlXPathFreeObject(xpathObj);
        return result;
    }

    string DatabaseRegistry::getDatabaseNamingStrategy() {
        string ss = "/registry/@databaseNamingStrategy";
        xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((const xmlChar*)ss.c_str(), this->xpathCtx);
        LOG_DEBUG("xpath obj = " << xpathObj);
        if (xpathObj->nodesetval==0) {
            LOG_WARN("no result found for '" << ss.c_str() << "'");
            return "";
        }
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        if (nodes->nodeNr!=1) {
            return string("");
        }
        xmlNodePtr node = nodes->nodeTab[0];
        xmlChar * sval = xmlXPathCastNodeToString(node);
        int len = xmlStrlen(sval);
        string result = string((const char*)sval,(size_t)len);
        xmlFree(sval);
        xmlXPathFreeObject(xpathObj);
        return result;
    }

    shared_ptr<Url> DatabaseRegistry::getUrl(xmlElementPtr databaseNode) {
        string database = getAttribute(databaseNode, "id");
        xmlElementPtr serverNode = (xmlElementPtr)databaseNode->parent;
        xmlElementPtr hostNode = (xmlElementPtr)serverNode->parent;
        xmlElementPtr systemNode = (xmlElementPtr)hostNode->parent;
        string environment = getAttribute(systemNode,"name");
        int shard = -1;
        if (hasAttribute(databaseNode,"shard")) {
            shard = atoi(getAttribute(databaseNode,"shard").c_str());
        }
        int port = atoi(getAttribute(serverNode,"port").c_str());
        string hostAddr = getAttribute(hostNode,"name");
        string databaseName;
        if (hasAttribute(databaseNode,"name")) {
            databaseName = getAttribute(databaseNode,"name");
        } else {
            if (this->databaseNamingStrategy.empty()) {
                throw runtime_error("no database name attribute found");
            }
            Template t("{","}");
            t.set("system",environment);
            t.set("id",database);
            if (shard == -1) {
                t.set("shardId","");
            } else {
                t.set("shardId",to_string(shard));
            }
            databaseName = t.render(this->databaseNamingStrategy);
        }
        shared_ptr<Url> url(new Url("postgres",hostAddr,port,databaseName));
        return url;
    }
}

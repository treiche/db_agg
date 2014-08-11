/*
 * UrlRegistry.cpp
 *
 *  Created on: Jul 24, 2014
 *      Author: arnd
 */

#include "UrlRegistry.h"
#include "utils/logging.h"

using namespace std;
using namespace log4cplus;



namespace db_agg {

static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("UrlRegistry"));

void errorHandler(void * ctx, const char * msg, ...) {
    va_list ap;
    va_start(ap, msg);
    char buf[1024];
    vsprintf(buf,msg,ap);
    cout << "ERROR: " << buf << endl;
    LOG_ERROR(buf);
}

void warningHandler(void * ctx, const char * msg, ...) {
    va_list ap;
    va_start(ap, msg);
    char buf[1024];
    vsprintf(buf,msg,ap);
    cout << "WARNING: " << buf << endl;
    LOG_WARN(buf);
}


bool hasAttribute(xmlElementPtr element, string attrName) {
    xmlAttribute *attr = element->attributes;
    while(attr != nullptr) {
        if (attrName.compare((const char *)attr->name) == 0) {
            return true;
        }
        attr = (xmlAttribute*)attr->next;
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

void UrlRegistry::recurse(xmlNodePtr node, shared_ptr<Url> parentUrl, string environment) {
    xmlNodePtr child = node->children;
    if (!parentUrl) {
        parentUrl.reset(new Url());
    }
    while(child != nullptr) {
        if (child->type == XML_ELEMENT_NODE) {
            // cout << "got child " << child->name << endl;
            xmlElementPtr childElement = (xmlElementPtr)child;
            string elementName((char *)childElement->name);
            if (elementName == "globals") {
                child = child->next;
                continue;
            } else if (elementName == "urls") {
                recurse(child,parentUrl,"");
            } else if (elementName == "environment") {
                recurse(child,parentUrl,getAttribute((xmlElementPtr)child,"name"));
            } else {
                shared_ptr<Url> childUrl(new Url(*parentUrl));
                if (hasAttribute(childElement,"value")) {
                    string value = getAttribute(childElement, "value");
                    if (elementName == "protocol") {
                        childUrl->setProtocol(value);
                    } else if (elementName == "host") {
                        childUrl->setHost(value);
                    } else if (elementName == "port") {
                        childUrl->setPort(value);
                    } else if (elementName == "path") {
                        childUrl->addPathItem(value);
                    } else if (elementName == "parameter") {
                        string name = getAttribute((xmlElementPtr)child, "name");
                        childUrl->setParameter(name,value);
                    }
                    if (child->children == nullptr) {
                        urls[environment].push_back(childUrl);
                    } else {
                        recurse(child,childUrl,environment);
                    }
                } else if (hasAttribute(childElement,"ref")) {
                    // jump to ref
                    string refId = getAttribute(childElement,"ref");
                    xmlElementPtr target = this->elementById[refId];
                    recurse((xmlNodePtr)target, childUrl, environment);
                }
            }
        }
        child = child->next;
    }
}

void UrlRegistry::getElementsById(xmlNodePtr node) {
    xmlNodePtr child = node->children;
    while(child != nullptr) {
        if (child->type == XML_ELEMENT_NODE) {
            xmlElementPtr element = (xmlElementPtr)child;
            if (hasAttribute(element,"id")) {
                string id = getAttribute(element,"id");
                elementById[id] = element;
            }
            getElementsById(child);
        }
        child = child->next;
    }
}

UrlRegistry::UrlRegistry(string regfile, string schemaFile) {
    // load schema file
    xmlSchemaParserCtxtPtr ctx = xmlSchemaNewParserCtxt(schemaFile.c_str());
    xmlSchemaSetParserErrors(ctx,(xmlSchemaValidityErrorFunc)errorHandler,(xmlSchemaValidityWarningFunc)warningHandler,nullptr);
    xmlSchemaPtr schema = xmlSchemaParse(ctx);
    xmlSchemaFreeParserCtxt(ctx);
    // load registry file
    this->document = xmlReadFile(regfile.c_str(), NULL, 0);
    // validate
    xmlSchemaValidCtxtPtr vctx = xmlSchemaNewValidCtxt(schema);
    xmlSchemaSetValidErrors(vctx, (xmlSchemaValidityErrorFunc)errorHandler, (xmlSchemaValidityErrorFunc)warningHandler,nullptr);
    int ret = xmlSchemaValidateDoc(vctx, this->document);
    xmlSchemaFreeValidCtxt(vctx);
    xmlSchemaFree(schema);
    if (ret != 0) {
        THROW_EXC("url registry file is not valid");
    }
    // get id to element map
    getElementsById((xmlNodePtr)this->document);

    shared_ptr<Url> parentUrl;
    recurse((xmlNodePtr)this->document, parentUrl, "");
    cout << "got " << this->urls.size() << " urls" << endl;
    for (auto env:this->urls) {
        cout << env.first << ":" << endl;
        for (auto url:env.second) {
            cout << "    " << url->getUrl() << endl;
        }
    }
}



UrlRegistry::~UrlRegistry() {
    LOG_TRACE("delete registry");
    xmlFreeDoc(this->document);
}

vector<shared_ptr<Url>> UrlRegistry::findUrls(string env, shared_ptr<Url> wcUrl) {
    vector<shared_ptr<Url>> matches;
    for (auto url:this->urls[env]) {
        if (url->matches(wcUrl)) {
            matches.push_back(url);
        }
    }
    return matches;
}

}



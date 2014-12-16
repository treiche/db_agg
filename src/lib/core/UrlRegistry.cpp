/*
 * UrlRegistry.cpp
 *
 *  Created on: Jul 24, 2014
 *      Author: arnd
 */

#include "UrlRegistry.h"
#include "utils/logging.h"
#include "utils/utility.h"
#include "utils/xml.h"
#include "utils/File.h"
#include "utils/RegExp.h"

#include <list>
#include <set>

using namespace std;
using namespace log4cplus;



namespace db_agg {

static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("UrlRegistry"));

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

UrlRegistry::UrlRegistry(string regfile) {
    string q = readFile(regfile);
    File rf(regfile);
    string baseUrl = "file://" + rf.abspath();
    LOG_DEBUG("baseUrl = " << baseUrl);

    this->document = parseDoc(q,baseUrl,false);
    // get id to element map
    getElementsById((xmlNodePtr)this->document);

    shared_ptr<Url> parentUrl;
    recurse((xmlNodePtr)this->document, parentUrl, "");
    LOG_DEBUG("got " << this->urls.size() << " urls");
    for (auto env:this->urls) {
        LOG_DEBUG(env.first << ":");
        for (auto url:env.second) {
            LOG_DEBUG("    " << url->getUrl());
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

vector<shared_ptr<Url>> UrlRegistry::findUrls(string env, string type, string query, string& sid) {
    vector<shared_ptr<Url>> matches;
    string expr("/urls/environment[@name='" + env + "']//match");
    xmlXPathContextPtr xpathCtx = xmlXPathNewContext(this->document);
    xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((const xmlChar*)expr.c_str(), xpathCtx);
    xmlNodeSetPtr nodes = xpathObj->nodesetval;
    LOG_DEBUG("found " << nodes->nodeNr << " matchers");
    set<string> serverIds;
    for (int idx=0;idx<nodes->nodeNr;idx++) {
        xmlNodePtr node = nodes->nodeTab[idx];
        if(node->type == XML_ELEMENT_NODE) {
        	string regexp = getAttribute((xmlElementPtr)node,"regexp");
        	LOG_DEBUG("regexp = " << regexp);
        	RegExp re(regexp);
        	if (re.matches(query)) {
        		LOG_DEBUG("matched");
        		string serverId;
        		matches.push_back(getUrl((xmlElementPtr)node,serverId));
        		serverIds.insert(serverId);
        	}
        }
    }
    if (serverIds.size() > 1) {
        THROW_EXC("ambigous server id");
    }
    sid = *(serverIds.begin());
    return matches;
}

shared_ptr<Url> UrlRegistry::getUrl(xmlElementPtr element, string& serverId) {
	shared_ptr<Url> url(new Url());
	xmlNodePtr tmp = (xmlNodePtr)element;
	list<string> path;
	while(tmp->type != XML_DOCUMENT_NODE) {
		LOG_DEBUG("tmp = " << tmp->name);
		string name((const char*)tmp->name);
		if (hasAttribute((xmlElementPtr)tmp,"ref")) {
		    serverId = getAttribute((xmlElementPtr)tmp,"ref");
		}
		if (name == "path") {
			path.push_front(getAttribute((xmlElementPtr)tmp,"value"));
		} else if (name == "host") {
			url->setHost(getAttribute((xmlElementPtr)tmp,"value"));
		} else if (name == "port") {
			url->setPort(getAttribute((xmlElementPtr)tmp,"value"));
		} else if (name == "protocol") {
			url->setProtocol(getAttribute((xmlElementPtr)tmp,"value"));
		}
		tmp = tmp->parent;
	}
	for (auto p:path) {
		url->addPathItem(p);
	}
	return url;
}

vector<ShardingStrategyConfiguration> UrlRegistry::getShardingStrategies(string serverId) {
    vector<ShardingStrategyConfiguration> strategies;
    xmlElementPtr ge = elementById[serverId];
    xmlNodePtr tmp = ge->children;
    while(tmp) {
        if (tmp->type == XML_ELEMENT_NODE && string((char*)tmp->name) == "sharding-strategy") {
            xmlElementPtr shardingStrategy = (xmlElementPtr)tmp;
            string name = getAttribute(shardingStrategy,"name");
            string shardCol = getAttribute(shardingStrategy,"shardCol");
            strategies.push_back(ShardingStrategyConfiguration(name,shardCol));
        }
        tmp = tmp->next;
    }
    return strategies;
}

}



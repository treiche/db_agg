/*
 * UrlRegistry.h
 *
 *  Created on: Jul 24, 2014
 *      Author: arnd
 */

#ifndef URLREGISTRY_H_
#define URLREGISTRY_H_

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Url.h"
extern "C" {
    #include <libxml/xmlschemas.h>
	#include <libxml/xpath.h>
}
#include "sharding/ShardingStrategyConfiguration.h"


namespace db_agg {
class UrlRegistry {
private:
    xmlDocPtr document;
    std::map<std::string, std::vector<std::shared_ptr<Url>>> urls;
    std::map<std::string, xmlElementPtr> elementById;
    void getElementsById(xmlNodePtr node);
    void recurse(xmlNodePtr node, std::shared_ptr<Url> parentUrl, std::string environment);
    std::shared_ptr<Url> getUrl(xmlElementPtr element, std::string& serverId);
public:
    UrlRegistry(std::string regfile);
    ~UrlRegistry();
    std::vector<std::shared_ptr<Url>> findUrls(std::string env, std::shared_ptr<Url> wcUrl);
    std::vector<std::shared_ptr<Url>> findUrls(std::string env, std::string type, std::string query, std::string& serverId);
    std::vector<ShardingStrategyConfiguration> getShardingStrategies(std::string databaseId);
};
}


#endif /* URLREGISTRY_H_ */

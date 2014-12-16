/*
 * Locator.h
 *
 *  Created on: Dec 29, 2013
 *      Author: arnd
 */

#ifndef LOCATOR_H_
#define LOCATOR_H_

#include <string>

namespace db_agg {
    class Locator {
        private:
            std::string name;
            short shardId = -1;
            std::string environment;
            static std::string defaultEnvironment;
        public:
            Locator() = default;
            Locator(const Locator& other) {
                name = other.name;
                shardId = other.shardId;
                environment = other.environment;
            }
            Locator(std::string name, short shardId, std::string environment);
            static void setDefaultEnvironment(std::string env);
            short compare(Locator& other);
            std::string getQName();
            const std::string& getEnvironment() const;
            const std::string& getName() const;
            short getShardId() const;
    };
}



#endif /* LOCATOR_H_ */

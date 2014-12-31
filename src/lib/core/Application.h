#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <string>
#include <map>
#include <memory>

#include "cache/CacheRegistry.h"
#include "core/Configuration.h"
#include "core/DatabaseRegistry.h"
#include "core/UrlRegistry.h"
#include "extension/ExtensionLoader.h"
#include "utils/PasswordManager.h"
#include "core/QueryParser.h"
#include "core/QueryProcessor.h"
#include "utils/SignalHandler.h"
#include "table/TableData.h"
#include "event/AsyncEvent.h"

namespace db_agg {
    class Application: public AsyncEventProducer, public EventListener, public SignalHandler {
        private:
            PasswordManager *passwordManager = nullptr;
            QueryProcessor *queryProcessor = nullptr;
            QueryParser *queryParser = nullptr;
            DatabaseRegistry *databaseRegistry = nullptr;
            UrlRegistry *urlRegistry = nullptr;
            CacheRegistry *cacheRegistry = nullptr;
            ExtensionLoader *extensionLoader = nullptr;
            std::string query;
            std::string queryUrl;
            std::string environment;
            std::map<std::string,std::string> externalSources;
            std::map<std::string,std::string> queryParameter;
        public:
            Application();
            static std::string findConfigurationFile(std::string name, bool createIfNeeded, bool isDir);
            virtual ~Application();
            virtual void handleEvent(std::shared_ptr<Event> event) override;
            void bootstrap(Configuration& config);
            bool run();
            bool step();
            QueryParser& getQueryParser() {
                return *queryParser;
            }
            virtual void handleSignal(int signal) override;
            ExecutionGraph& getExecutionGraph();
            void stop();
    };
}

#endif

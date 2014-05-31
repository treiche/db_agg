#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <string>
#include <map>

#include "cache/CacheRegistry.h"
#include "core/Configuration.h"
#include "core/DatabaseRegistry.h"
#include "extension/ExtensionLoader.h"
#include "utils/PasswordManager.h"
#include "core/QueryParser.h"
#include "core/QueryProcessor.h"
#include "utils/SignalHandler.h"
#include "table/TableData.h"

namespace db_agg {
    class Application: public EventProducer, public EventListener, public SignalHandler {
        private:
            PasswordManager *passwordManager = nullptr;
            QueryProcessor *queryProcessor = nullptr;
            QueryParser *queryParser = nullptr;
            DatabaseRegistry *databaseRegistry = nullptr;
            CacheRegistry *cacheRegistry = nullptr;
            ExtensionLoader extensionLoader;
            std::string query;
            std::string environment;
            std::map<std::string,shared_ptr<TableData>> externalSources;
            std::map<std::string,std::string> queryParameter;
            static Application instance;
            Application();
        public:
            static Application& getInstance() {
                return instance;
            }
            virtual ~Application();
            virtual void handleEvent(Event& event) override;
            void bootstrap(Configuration& config);
            bool run();
            QueryParser& getQueryParser() {
                return *queryParser;
            }
            virtual void handleSignal(int signal) override;
    };
}

#endif

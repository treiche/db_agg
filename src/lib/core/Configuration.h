#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <string>
#include <map>
#include <vector>

extern "C" {
#include <jansson.h>
}

using namespace std;

namespace db_agg {

class Configuration {
private:
    string queryFile;
    string environment{ "local" };
    bool help;
    string logLevel{ "INFO" };
    bool showProgress;
    size_t copyThreshold{ 0 };
    map<string,string> externalSources;
    vector<string> externalExcelSources;
    size_t statementTimeout{ 600000 };
    bool searchPasswordInPgPass{ true };
    bool useRegExpParser{ true };
    map<string,string> queryParameter;
    bool dontExecute;
    size_t maxParallelExecutions{ 1000 };
    bool disableCache;
    string outputDir{ "." };
    string resultDir{ "{outputDir}/{queryName}/{environment}" };
    string cacheDir{ "${HOME}/cache" };
    string prefix{ "/usr/local/db_agg" };
    string databaseRegistryFile{ "${HOME}/etc/database-registry.xml" };
    string cacheRegistryFile{ "${HOME}/cache/cache-registry.json" };
    string extensionDir{ "${HOME}/extensions" };
    string logConf{ "${HOME}/etc/log4cplus.properties" };
    string logFile{ "db_agg.log" };
public:
    void fromJson(std::string json);
    string getQueryFile();
    void setQueryFile(string queryFile);
    string getEnvironment();
    void setEnvironment(string environment);
    bool getHelp();
    void setHelp(bool help);
    string getLogLevel();
    void setLogLevel(string logLevel);
    bool getShowProgress();
    void setShowProgress(bool showProgress);
    size_t getCopyThreshold();
    void setCopyThreshold(size_t copyThreshold);
    map<string,string> getExternalSources();
    void setExternalSources(map<string,string> externalSources);
    vector<string> getExternalExcelSources();
    void setExternalExcelSources(vector<string> externalExcelSources);
    size_t getStatementTimeout();
    void setStatementTimeout(size_t statementTimeout);
    bool getSearchPasswordInPgPass();
    void setSearchPasswordInPgPass(bool searchPasswordInPgPass);
    bool getUseRegExpParser();
    void setUseRegExpParser(bool useRegExpParser);
    map<string,string> getQueryParameter();
    void setQueryParameter(map<string,string> queryParameter);
    bool getDontExecute();
    void setDontExecute(bool dontExecute);
    size_t getMaxParallelExecutions();
    void setMaxParallelExecutions(size_t maxParallelExecutions);
    bool getDisableCache();
    void setDisableCache(bool disableCache);
    string getOutputDir();
    void setOutputDir(string outputDir);
    string getResultDir();
    void setResultDir(string resultDir);
    string getCacheDir();
    void setCacheDir(string cacheDir);
    string getPrefix();
    void setPrefix(string prefix);
    string getDatabaseRegistryFile();
    void setDatabaseRegistryFile(string databaseRegistryFile);
    string getCacheRegistryFile();
    void setCacheRegistryFile(string cacheRegistryFile);
    string getExtensionDir();
    void setExtensionDir(string extensionDir);
    string getLogConf();
    void setLogConf(string logConf);
    string getLogFile();
    void setLogFile(string logFile);
};


}

#endif /* CONFIGURATION_H_ */
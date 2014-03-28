#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_


#include <string>
#include <map>
#include <vector>

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
    bool disableCache;
    string outputDir{ "." };
    string resultDir{ "{outputDir}/{queryName}/{environment}" };
    string cacheDir{ "~/cache" };
    string databaseRegistryFile{ "~/etc/database-registry.xml" };
    string cacheRegistryFile{ "~/cache/cache-registry.json" };
    string extensionDir{ "~/extensions" };
    string logConf{ "~/etc/log4cplus.properties" };
    string logFile{ "db_agg.log" };
    string findConfigurationFile(std::string name, bool createIfNeeded, bool isDir);
public:
    void setQueryFile(string queryFile);
    string getQueryFile();
    void setEnvironment(string environment);
    string getEnvironment();
    void setHelp(bool help);
    bool getHelp();
    void setLogLevel(string logLevel);
    string getLogLevel();
    void setShowProgress(bool showProgress);
    bool getShowProgress();
    void setCopyThreshold(size_t copyThreshold);
    size_t getCopyThreshold();
    void setExternalSources(map<string,string> externalSources);
    map<string,string> getExternalSources();
    void setExternalExcelSources(vector<string> externalExcelSources);
    vector<string> getExternalExcelSources();
    void setStatementTimeout(size_t statementTimeout);
    size_t getStatementTimeout();
    void setSearchPasswordInPgPass(bool searchPasswordInPgPass);
    bool getSearchPasswordInPgPass();
    void setUseRegExpParser(bool useRegExpParser);
    bool getUseRegExpParser();
    void setQueryParameter(map<string,string> queryParameter);
    map<string,string> getQueryParameter();
    void setDisableCache(bool disableCache);
    bool getDisableCache();
    void setOutputDir(string outputDir);
    string getOutputDir();
    void setResultDir(string resultDir);
    string getResultDir();
    void setCacheDir(string cacheDir);
    string getCacheDir();
    void setDatabaseRegistryFile(string databaseRegistryFile);
    string getDatabaseRegistryFile();
    void setCacheRegistryFile(string cacheRegistryFile);
    string getCacheRegistryFile();
    void setExtensionDir(string extensionDir);
    string getExtensionDir();
    void setLogConf(string logConf);
    string getLogConf();
    void setLogFile(string logFile);
    string getLogFile();
};

}

#endif /* CONFIGURATION_H_ */

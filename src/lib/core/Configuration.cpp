#include "Configuration.h"

namespace db_agg {
    string Configuration::getEnvironment() {
        return environment;
    }
    void Configuration::setEnvironment(string environment) {
        this->environment = environment;
    }
    bool Configuration::getHelp() {
        return help;
    }
    void Configuration::setHelp(bool help) {
        this->help = help;
    }
    string Configuration::getLogLevel() {
        return logLevel;
    }
    void Configuration::setLogLevel(string logLevel) {
        this->logLevel = logLevel;
    }
    bool Configuration::getShowProgress() {
        return showProgress;
    }
    void Configuration::setShowProgress(bool showProgress) {
        this->showProgress = showProgress;
    }
    size_t Configuration::getCopyThreshold() {
        return copyThreshold;
    }
    void Configuration::setCopyThreshold(size_t copyThreshold) {
        this->copyThreshold = copyThreshold;
    }
    map<string,string> Configuration::getExternalSources() {
        return externalSources;
    }
    void Configuration::setExternalSources(map<string,string> externalSources) {
        this->externalSources = externalSources;
    }
    vector<string> Configuration::getExternalExcelSources() {
        return externalExcelSources;
    }
    void Configuration::setExternalExcelSources(vector<string> externalExcelSources) {
        this->externalExcelSources = externalExcelSources;
    }
    size_t Configuration::getStatementTimeout() {
        return statementTimeout;
    }
    void Configuration::setStatementTimeout(size_t statementTimeout) {
        this->statementTimeout = statementTimeout;
    }
    bool Configuration::getSearchPasswordInPgPass() {
        return searchPasswordInPgPass;
    }
    void Configuration::setSearchPasswordInPgPass(bool searchPasswordInPgPass) {
        this->searchPasswordInPgPass = searchPasswordInPgPass;
    }
    bool Configuration::getUseRegExpParser() {
        return useRegExpParser;
    }
    void Configuration::setUseRegExpParser(bool useRegExpParser) {
        this->useRegExpParser = useRegExpParser;
    }
    map<string,string> Configuration::getQueryParameter() {
        return queryParameter;
    }
    void Configuration::setQueryParameter(map<string,string> queryParameter) {
        this->queryParameter = queryParameter;
    }
    bool Configuration::getDontExecute() {
        return dontExecute;
    }
    void Configuration::setDontExecute(bool dontExecute) {
        this->dontExecute = dontExecute;
    }
    bool Configuration::getDisableCache() {
        return disableCache;
    }
    void Configuration::setDisableCache(bool disableCache) {
        this->disableCache = disableCache;
    }
    string Configuration::getOutputDir() {
        return outputDir;
    }
    void Configuration::setOutputDir(string outputDir) {
        this->outputDir = outputDir;
    }
    string Configuration::getResultDir() {
        return resultDir;
    }
    void Configuration::setResultDir(string resultDir) {
        this->resultDir = resultDir;
    }
    string Configuration::getCacheDir() {
        return cacheDir;
    }
    void Configuration::setCacheDir(string cacheDir) {
        this->cacheDir = cacheDir;
    }
    string Configuration::getPrefix() {
        return prefix;
    }
    void Configuration::setPrefix(string prefix) {
        this->prefix = prefix;
    }
    string Configuration::getDatabaseRegistryFile() {
        return databaseRegistryFile;
    }
    void Configuration::setDatabaseRegistryFile(string databaseRegistryFile) {
        this->databaseRegistryFile = databaseRegistryFile;
    }
    string Configuration::getCacheRegistryFile() {
        return cacheRegistryFile;
    }
    void Configuration::setCacheRegistryFile(string cacheRegistryFile) {
        this->cacheRegistryFile = cacheRegistryFile;
    }
    string Configuration::getExtensionDir() {
        return extensionDir;
    }
    void Configuration::setExtensionDir(string extensionDir) {
        this->extensionDir = extensionDir;
    }
    string Configuration::getLogConf() {
        return logConf;
    }
    void Configuration::setLogConf(string logConf) {
        this->logConf = logConf;
    }
    string Configuration::getLogFile() {
        return logFile;
    }
    void Configuration::setLogFile(string logFile) {
        this->logFile = logFile;
    }
    string Configuration::getQueryFile() {
        return queryFile;
    }
    void Configuration::setQueryFile(string queryFile) {
        this->queryFile = queryFile;
    }



}
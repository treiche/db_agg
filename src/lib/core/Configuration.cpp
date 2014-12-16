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
    size_t Configuration::getMaxParallelExecutions() {
        return maxParallelExecutions;
    }
    void Configuration::setMaxParallelExecutions(size_t maxParallelExecutions) {
        this->maxParallelExecutions = maxParallelExecutions;
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
    string Configuration::getUrlRegistryFile() {
        return urlRegistryFile;
    }
    void Configuration::setUrlRegistryFile(string urlRegistryFile) {
        this->urlRegistryFile = urlRegistryFile;
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

void Configuration::fromJson(std::string json) {
    json_error_t *error = 0;
    json_t *jsConfig = json_loads(json.c_str(),0,error);
    json_t *js = nullptr;
    const char *key;
    json_t *value;
    size_t index;
        js = json_object_get(jsConfig,"queryFile");
        if (js != nullptr) {
            string queryFile = json_string_value(js);
            setQueryFile(queryFile);
        }
        js = json_object_get(jsConfig,"environment");
        if (js != nullptr) {
            string environment = json_string_value(js);
            setEnvironment(environment);
        }
        js = json_object_get(jsConfig,"help");
        if (js != nullptr) {
            if (json_is_true(js)) {
                setHelp(true);
            } else {
                setHelp(false);
            }
        }
        js = json_object_get(jsConfig,"logLevel");
        if (js != nullptr) {
            string logLevel = json_string_value(js);
            setLogLevel(logLevel);
        }
        js = json_object_get(jsConfig,"showProgress");
        if (js != nullptr) {
            if (json_is_true(js)) {
                setShowProgress(true);
            } else {
                setShowProgress(false);
            }
        }
        js = json_object_get(jsConfig,"copyThreshold");
        if (js != nullptr) {
            size_t copyThreshold = json_integer_value(js);
            setCopyThreshold(copyThreshold);
        }
        js = json_object_get(jsConfig,"externalSources");
        if (js != nullptr) {
            map<string,string> externalSources;
            json_object_foreach(js, key, value) {
                externalSources[key] = json_string_value(value);
            }
            setExternalSources(externalSources);
        }
        js = json_object_get(jsConfig,"externalExcelSources");
        if (js != nullptr) {
            vector<string> externalExcelSources;
            json_array_foreach(js, index, value) {
                externalExcelSources.push_back(json_string_value(value));
            }
            setExternalExcelSources(externalExcelSources);
        }
        js = json_object_get(jsConfig,"statementTimeout");
        if (js != nullptr) {
            size_t statementTimeout = json_integer_value(js);
            setStatementTimeout(statementTimeout);
        }
        js = json_object_get(jsConfig,"searchPasswordInPgPass");
        if (js != nullptr) {
            if (json_is_true(js)) {
                setSearchPasswordInPgPass(true);
            } else {
                setSearchPasswordInPgPass(false);
            }
        }
        js = json_object_get(jsConfig,"useRegExpParser");
        if (js != nullptr) {
            if (json_is_true(js)) {
                setUseRegExpParser(true);
            } else {
                setUseRegExpParser(false);
            }
        }
        js = json_object_get(jsConfig,"queryParameter");
        if (js != nullptr) {
            map<string,string> queryParameter;
            json_object_foreach(js, key, value) {
                queryParameter[key] = json_string_value(value);
            }
            setQueryParameter(queryParameter);
        }
        js = json_object_get(jsConfig,"dontExecute");
        if (js != nullptr) {
            if (json_is_true(js)) {
                setDontExecute(true);
            } else {
                setDontExecute(false);
            }
        }
        js = json_object_get(jsConfig,"maxParallelExecutions");
        if (js != nullptr) {
            size_t maxParallelExecutions = json_integer_value(js);
            setMaxParallelExecutions(maxParallelExecutions);
        }
        js = json_object_get(jsConfig,"disableCache");
        if (js != nullptr) {
            if (json_is_true(js)) {
                setDisableCache(true);
            } else {
                setDisableCache(false);
            }
        }
        js = json_object_get(jsConfig,"outputDir");
        if (js != nullptr) {
            string outputDir = json_string_value(js);
            setOutputDir(outputDir);
        }
        js = json_object_get(jsConfig,"resultDir");
        if (js != nullptr) {
            string resultDir = json_string_value(js);
            setResultDir(resultDir);
        }
        js = json_object_get(jsConfig,"cacheDir");
        if (js != nullptr) {
            string cacheDir = json_string_value(js);
            setCacheDir(cacheDir);
        }
        js = json_object_get(jsConfig,"prefix");
        if (js != nullptr) {
            string prefix = json_string_value(js);
            setPrefix(prefix);
        }
        js = json_object_get(jsConfig,"databaseRegistryFile");
        if (js != nullptr) {
            string databaseRegistryFile = json_string_value(js);
            setDatabaseRegistryFile(databaseRegistryFile);
        }
        js = json_object_get(jsConfig,"urlRegistryFile");
        if (js != nullptr) {
            string urlRegistryFile = json_string_value(js);
            setUrlRegistryFile(urlRegistryFile);
        }
        js = json_object_get(jsConfig,"extensionDir");
        if (js != nullptr) {
            string extensionDir = json_string_value(js);
            setExtensionDir(extensionDir);
        }
        js = json_object_get(jsConfig,"logConf");
        if (js != nullptr) {
            string logConf = json_string_value(js);
            setLogConf(logConf);
        }
        js = json_object_get(jsConfig,"logFile");
        if (js != nullptr) {
            string logFile = json_string_value(js);
            setLogFile(logFile);
        }
}

}
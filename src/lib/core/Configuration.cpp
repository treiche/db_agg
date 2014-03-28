/*
 * Configuration.cpp
 *
 *  Created on: Jan 3, 2014
 *      Author: arnd
 */

#include "Configuration.h"

#include <log4cplus/logger.h>

#include "utils/utility.h"
#include "utils/File.h"
#include "utils/Template.h"
#include "installation.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
    static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("Configuration"));

    static string getSysConfigDir() {
        Template t{"${","}"};
        t.set("prefix",DBAGG_PREFIX);
        return t.render(DBAGG_SYSCONFIGDIR);
    }


    string Configuration::getQueryFile() {
        return queryFile;
    }
    void Configuration::setQueryFile(string queryFile) {
        this->queryFile = queryFile;
    }
    string Configuration::getOutputDir() {
        return outputDir;
    }
    void Configuration::setOutputDir(string outputDir) {
        this->outputDir = outputDir;
    }
    void Configuration::setResultDir(string resultDir) {
        this->resultDir = resultDir;
    }
    string Configuration::getResultDir() {
        return resultDir;
    }
    string Configuration::getCacheDir() {
        return findConfigurationFile(cacheDir, true, true);
    }

    void Configuration::setCacheDir(string cacheDir) {
        this->cacheDir = cacheDir;
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

    string Configuration::getEnvironment() {
        return environment;
    }
    void Configuration::setEnvironment(string environment) {
        this->environment = environment;
    }
    string Configuration::getDatabaseRegistryFile() {
        return findConfigurationFile(databaseRegistryFile, false, false);
    }
    void Configuration::setDatabaseRegistryFile(string databaseRegistryFile) {
        this->databaseRegistryFile = databaseRegistryFile;
    }
    string Configuration::getCacheRegistryFile() {
        return findConfigurationFile(cacheRegistryFile, false, false);
    }
    void Configuration::setCacheRegistryFile(string cacheRegistryFile) {
        this->cacheRegistryFile = cacheRegistryFile;
    }
    string Configuration::getExtensionDir() {
        return findConfigurationFile(extensionDir, false, false);
    }
    void Configuration::setExtensionDir(string extensionDir) {
        this->extensionDir = extensionDir;
    }

    string Configuration::findConfigurationFile(string name, bool createIfNeeded, bool isDir) {
		LOG4CPLUS_INFO(LOG, "find configuration file '" << name << "'");
        if (name.find("~") == 0) {
            string homeLocation = getHomeDir() + "/.db_agg/" + name.substr(1);
            File homeFile{homeLocation};
            if (homeFile.exists()) {
            	return homeLocation;
            }
            string prefixLocation = getSysConfigDir() + "/" + name.substr(1);
            File prefixFile{prefixLocation};
            if (prefixFile.exists()) {
            	return prefixLocation;
            }
            if (createIfNeeded) {
            	if (isDir) {
            		LOG4CPLUS_INFO(LOG, "create configuration directory '" << homeLocation << "'");
            		homeFile.mkdirs();
            	}
            }
            return homeLocation;
        }
        return name;
    }
    bool Configuration::getDisableCache() {
        return disableCache;
    }
    void Configuration::setDisableCache(bool disableCache) {
        this->disableCache = disableCache;
    }
    void Configuration::setLogLevel(std::string logLevel) {
        this->logLevel = logLevel;
    }
    std::string Configuration::getLogLevel() {
        return logLevel;
    }
    void Configuration::setLogFile(std::string logFile) {
        this->logFile = logFile;
    }
    std::string Configuration::getLogFile() {
        return logFile;
    }
    void Configuration::setLogConf(std::string logConf) {
        this->logConf = logConf;
    }
    std::string Configuration::getLogConf() {
        return findConfigurationFile(logConf, false, false);
    }

    void Configuration::setCopyThreshold(size_t copyThreshold) {
        this->copyThreshold = copyThreshold;
    }
    size_t Configuration::getCopyThreshold() {
        return copyThreshold;
    }
    void Configuration::setExternalSources(std::map<std::string,std::string> externalSources) {
        for (auto& externalSource:externalSources) {
            File src(externalSource.second);
            if (!src.exists()) {
                LOG4CPLUS_WARN(LOG, "external source '" << externalSource.second << "' does not exist. skipping ...");
            } else {
                this->externalSources[externalSource.first] = externalSource.second;
            }
        }
    }
    std::map<std::string,std::string> Configuration::getExternalSources() {
        return externalSources;
    }

    void Configuration::setExternalExcelSources(vector<string> externalExcelSources) {
        for (auto& externalExcelSource:externalExcelSources) {
            File src(externalExcelSource);
            if (!src.exists()) {
                LOG4CPLUS_WARN(LOG, "external source '" << externalExcelSource << "' does not exist. skipping ...");
            } else {
                this->externalExcelSources.push_back(externalExcelSource);
            }
        }
    }
    vector<string> Configuration::getExternalExcelSources() {
        return externalExcelSources;
    }

    size_t Configuration::getStatementTimeout() {
        return statementTimeout;
    }
    void Configuration::setStatementTimeout(size_t statementTimeout) {
    	this->statementTimeout = statementTimeout;
    }

    void Configuration::setHelp(bool help) {
        this->help = help;
    }

    void Configuration::setShowProgress(bool showProgress) {
        this->showProgress = showProgress;
    }

    bool Configuration::getShowProgress() {
        return showProgress;
    }

    void Configuration::setQueryParameter(map<string,string> queryParameter) {
        for (auto& param:queryParameter) {
            this->queryParameter[param.first] = param.second;
        }
    }
    map<string,string> Configuration::getQueryParameter() {
        return queryParameter;
    }

    bool Configuration::getHelp() {
        return help;
    }

}


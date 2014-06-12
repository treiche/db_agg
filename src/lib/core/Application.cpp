/*
 * Application.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: arnd
 */

#include "core/Application.h"

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/tstring.h>
#include <fstream>

#include "core/RegExpQueryParser.h"
#include "utils/utility.h"
#include "utils/File.h"
#include "excel/ExcelToTextFormat.h"
#include "utils/Template.h"
#include "installation.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("Application"));


void Application::handleSignal(int signal) {
    LOG4CPLUS_TRACE(LOG, "Application::handleSignal " << signal);
    LOG4CPLUS_TRACE(LOG, "received signal " << signal);
    if (queryProcessor!=nullptr) {
        queryProcessor->stop();
    }
}

Application Application::instance;

Application::Application() {
}

Application::~Application() {
    LOG4CPLUS_TRACE(LOG, "delete Application instance");
    if (queryParser) {
        LOG4CPLUS_TRACE(LOG, "delete query parser");
        delete queryParser;
    }
    if (queryProcessor) {
        LOG4CPLUS_TRACE(LOG, "delete query processor");
        delete queryProcessor;
    }
    if (passwordManager) {
        LOG4CPLUS_TRACE(LOG, "delete PasswordManager instance");
        delete passwordManager;
    }
}


void Application::bootstrap(Configuration& config) {
    LOG4CPLUS_DEBUG(LOG, "bootstrap application");
    passwordManager = new PasswordManager(config.getSearchPasswordInPgPass());
    if (config.getUseRegExpParser()) {
        queryParser = new RegExpQueryParser();
    } else {
        throw runtime_error("currently only RegExpQueryParser supported");
    }
    LOG4CPLUS_DEBUG(LOG, "load query file '"+config.getQueryFile()+"'");
    File queryFile{config.getQueryFile()};
    query = readFile(queryFile.abspath());
    environment = config.getEnvironment();
    LOG4CPLUS_DEBUG(LOG, "load database registry");
    string databaseRegistryFile = findConfigurationFile(config.getDatabaseRegistryFile(), false, false);
    databaseRegistry = new DatabaseRegistry(databaseRegistryFile);
    vector<string> environments = databaseRegistry->getSystems();
    bool environmentExists = false;
    for (auto& env:environments) {
        if (env == config.getEnvironment()) {
            environmentExists = true;
        }
    }
    if (!environmentExists) {
        string message = "environment " + config.getEnvironment() + " does not exists.\n choose one of:" + join(environments,",");
        LOG4CPLUS_ERROR(LOG,message);
        throw runtime_error(message);
    }
    LOG4CPLUS_DEBUG(LOG, "load cache registry");
    string cacheDir = findConfigurationFile(config.getCacheDir(),true,true);
    string cacheRegistryFile = findConfigurationFile(config.getCacheRegistryFile(), false, false);
    cacheRegistry = new CacheRegistry(cacheDir, cacheRegistryFile);

    string outputDir = config.getOutputDir();
    string queryFileName = queryFile.getName();
    int extIdx = queryFileName.find(".");
    string queryName = queryFileName.substr(0,extIdx);

    // interpolate config entries
    LOG4CPLUS_DEBUG(LOG,"result dir before interpolating: " << config.getResultDir());
    Template t;
    t.set("environment",environment);
    t.set("outputDir",outputDir);
    t.set("queryName",queryName);
    string resultDir = t.render(config.getResultDir());
    LOG4CPLUS_DEBUG(LOG,"result dir after interpolating : " << resultDir);
    File rf{resultDir};
    if (!rf.exists()) {
        rf.mkdirs();
    }

    // load external csv files
    for (auto& externalCsvFile:config.getExternalSources()) {
        File f(externalCsvFile.second);
        if (f.exists()) {
            externalSources[externalCsvFile.first] = "file://" + f.abspath();
        }
    }
    // load external excel files
    for (auto& externalExcelSource:config.getExternalExcelSources()) {
    	File f(externalExcelSource);
    	if (f.exists()) {
    	    string url = "file://" + f.abspath();
            ExcelToTextFormat ett;
            vector<string> sheets = ett.getSheetNames(f.abspath());
            for (auto& sheet:sheets) {
                externalSources[sheet] = url + "#" + sheet;
            }
    	}
    }
    queryParameter = config.getQueryParameter();
    queryProcessor = new QueryProcessor(*queryParser, *databaseRegistry,extensionLoader,*passwordManager,*cacheRegistry, resultDir, config.getDisableCache(), config.getCopyThreshold(), externalSources, config.getStatementTimeout(), queryParameter, config.getDontExecute());
    queryProcessor->addEventListener(this);
    LOG4CPLUS_DEBUG(LOG, "load extensions");
    string extensionDir = findConfigurationFile(config.getExtensionDir(), false,false);
    extensionLoader.loadExtensions(extensionDir);
    LOG4CPLUS_DEBUG(LOG, "bootstrapping ok");

}

static string getSysConfigDir() {
    Template t{"${","}"};
    t.set("prefix",DBAGG_PREFIX);
    return t.render(DBAGG_SYSCONFIGDIR);
}

static string getLocalStateDir() {
    Template t{"${","}"};
    t.set("prefix",DBAGG_PREFIX);
    return t.render(DBAGG_LOCALSTATEDIR);
}

string Application::findConfigurationFile(string name, bool createIfNeeded, bool isDir) {
        LOG4CPLUS_INFO(LOG, "find configuration file '" << name << "'");
        string effectiveFile;
        if (name.find("${HOME}") == 0) {
            string homeLocation = getHomeDir() + "/.db_agg/" + name.substr(7);
            File homeFile{homeLocation};
            if (homeFile.exists()) {
                effectiveFile = homeFile.abspath();
            } else {
                string prefixLocation = getSysConfigDir() + "/" + name.substr(7);
                File prefixFile{prefixLocation};
                effectiveFile = prefixLocation;
            }
        } else {
            effectiveFile = name;
        }
        File ef{effectiveFile};
        if (!ef.exists() && createIfNeeded) {
            if (isDir) {
                ef.mkdirs();
            }
        }
        return effectiveFile;
    }

void Application::handleEvent(Event& event) {
    LOG4CPLUS_DEBUG(LOG, "received event");
    fireEvent(event);
    LOG4CPLUS_DEBUG(LOG, "fired event");
}

bool Application::run() {
    LOG4CPLUS_DEBUG(LOG, "run application");
    try {
        queryProcessor->process(query, environment);
        Event event{EventType::APPLICATION_FINISHED,""};
        fireEvent(event);
        return true;
    } catch(CancelException& ce) {
        LOG4CPLUS_ERROR(LOG, "application canceled");
        Event event{EventType::APPLICATION_CANCELED,""};
        fireEvent(event);
    } catch(runtime_error& re) {
        LOG4CPLUS_ERROR(LOG, "caught exception:" << re.what());
        ApplicationFailedEvent event{re.what()};
        fireEvent(event);
    } catch(...) {
        LOG4CPLUS_ERROR(LOG, "application failed");
        ApplicationFailedEvent event{""};
        fireEvent(event);
    }
    return false;
}

ExecutionGraph& Application::getExecutionGraph() {
    return queryProcessor->getExecutionGraph();
}

}


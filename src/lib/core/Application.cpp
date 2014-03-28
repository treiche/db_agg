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
#include "table/CsvTableData.h"
#include "excel/ExcelToTextFormat.h"
#include "utils/Template.h"

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
    databaseRegistry = new DatabaseRegistry(config.getDatabaseRegistryFile());
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
    cacheRegistry = new CacheRegistry(config.getCacheDir(), config.getCacheRegistryFile());

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
    	CsvTableData *data = new CsvTableData(externalCsvFile.second);
    	externalSources[externalCsvFile.first] = data;
    }
    // load external excel files
    for (auto& externalExcelSource:config.getExternalExcelSources()) {
    	ExcelToTextFormat ett;
    	map<string,TableData*> sheets = ett.transform(externalExcelSource);
    	for (auto& sheet:sheets) {
    		externalSources[sheet.first] = sheet.second;
    	}
    }
    queryParameter = config.getQueryParameter();
    queryProcessor = new QueryProcessor(*queryParser, *databaseRegistry,extensionLoader,*passwordManager,*cacheRegistry, resultDir, config.getDisableCache(), config.getCopyThreshold(), externalSources, config.getStatementTimeout(), queryParameter);
    queryProcessor->addEventListener(this);
    LOG4CPLUS_DEBUG(LOG, "load extensions");
    extensionLoader.loadExtensions(config.getExtensionDir());
    LOG4CPLUS_DEBUG(LOG, "bootstrapping ok");

}

void Application::handleEvent(Event& event) {
    LOG4CPLUS_DEBUG(LOG, "received event");
    fireEvent(event);
    LOG4CPLUS_DEBUG(LOG, "fired event");
}

void Application::run() {
    LOG4CPLUS_DEBUG(LOG, "run application");
    try {
        queryProcessor->process(query, environment);
        Event event{EventType::APPLICATION_FINISHED,""};
        fireEvent(event);
    } catch(CancelException& ce) {
        Event event{EventType::APPLICATION_CANCELED,""};
        fireEvent(event);
    } catch(runtime_error& re) {
        ApplicationFailedEvent event{re.what()};
        fireEvent(event);
    } catch(...) {
        ApplicationFailedEvent event{""};
        fireEvent(event);
    }
}
}


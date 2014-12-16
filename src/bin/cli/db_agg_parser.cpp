#include "db_agg_parser.h"


#include <vector>
#include "cli/OptionGroup.h"
#include "cli/Argument.h"

using namespace std;

namespace db_agg {


static vector<OptionGroup> options = {
    {"general",{
        { 'e', "environment", true, "select default environment" } ,
        { 'h', "help", false, "show help" } ,
        { 'L', "log-level", true, "set the log level" } ,
        { 'S', "show-progress", false, "shows curses based progress of running queries" } ,
        { 't', "copy-threshold", true, "use copy instead of value injection when size of rows > threshold" } ,
        { 'E', "external-sources", true, "load external csv data source" } ,
        { 'X', "external-excel-sources", true, "load excel file as data source" } ,
        { 's', "statement-timeout", true, "timeout when processing database queries" } ,
        { 'j', "search-password-in-pg-pass", true, "look for passwords in ~/.pgpass" } ,
        { 'p', "use-reg-exp-parser", true, "parse query with parser based on regular expression" } ,
        { 'a', "query-parameter", true, "query parameter" } ,
        { 'z', "dont-execute", false, "only dump execution plan and exit" } ,
        { 'q', "max-parallel-executions", true, "maximal number of parallel executions" } 
    }},
    {"cache",{
        { 'd', "disable-cache", false, "don't load cached results" } 
    }},
    {"files",{
        { 'o', "output-dir", true, "path to the output directory" } ,
        { 'R', "result-dir", true, "path to the result directory" } ,
        { 'c', "cache-dir", true, "path to the cache directory" } ,
        { 'P', "prefix", true, "path to the db_agg installation" } ,
        { 'k', "database-registry-file", true, "path to the database registry" } ,
        { 'i', "url-registry-file", true, "path to the url registry" } ,
        { 'x', "extension-dir", true, "path to the extension directory" } ,
        { 'l', "log-conf", true, "path to the log configuration file" } ,
        { 'F', "log-file", true, "path to the log file" } 
    }}
};

static vector<Argument> args= {
    {"query-file", "path to the db_agg query file"}
};

db_agg_parser::db_agg_parser(): CommandLineParser("db_agg",args,options) {
}


void db_agg_parser::parse(int argc, char **argv, Configuration& config) {
    vector<string> posArgs = CommandLineParser::parse(argc,argv);

    if (getFlag("help")) {
        cout << getUsage() << endl;
        exit(1);
    }

    if (posArgs.size() != 1) {
        cout << "wrong number of positional arguments" << endl;
        cout << getUsage() << endl;
        exit(1);
    }
    config.setQueryFile(getOptionValue("query-file"));
    if (hasOption("environment")) {
        config.setEnvironment(getOptionValue("environment"));
    }
        config.setHelp(getFlag("help"));
    if (hasOption("log-level")) {
        config.setLogLevel(getOptionValue("log-level"));
    }
        config.setShowProgress(getFlag("show-progress"));
        if (hasOption("copy-threshold")) {
            config.setCopyThreshold(stoi(getOptionValue("copy-threshold")));
        }
        if (hasOption("external-sources")) {
            config.setExternalSources(getOptionMapValue("external-sources"));
        }
        if (hasOption("external-excel-sources")) {
            config.setExternalExcelSources(getOptionListValue("external-excel-sources"));
        }
        if (hasOption("statement-timeout")) {
            config.setStatementTimeout(stoi(getOptionValue("statement-timeout")));
        }
        config.setSearchPasswordInPgPass(getFlag("search-password-in-pg-pass"));
        config.setUseRegExpParser(getFlag("use-reg-exp-parser"));
        if (hasOption("query-parameter")) {
            config.setQueryParameter(getOptionMapValue("query-parameter"));
        }
        config.setDontExecute(getFlag("dont-execute"));
        if (hasOption("max-parallel-executions")) {
            config.setMaxParallelExecutions(stoi(getOptionValue("max-parallel-executions")));
        }
        config.setDisableCache(getFlag("disable-cache"));
    if (hasOption("output-dir")) {
        config.setOutputDir(getOptionValue("output-dir"));
    }
    if (hasOption("result-dir")) {
        config.setResultDir(getOptionValue("result-dir"));
    }
    if (hasOption("cache-dir")) {
        config.setCacheDir(getOptionValue("cache-dir"));
    }
    if (hasOption("prefix")) {
        config.setPrefix(getOptionValue("prefix"));
    }
    if (hasOption("database-registry-file")) {
        config.setDatabaseRegistryFile(getOptionValue("database-registry-file"));
    }
    if (hasOption("url-registry-file")) {
        config.setUrlRegistryFile(getOptionValue("url-registry-file"));
    }
    if (hasOption("extension-dir")) {
        config.setExtensionDir(getOptionValue("extension-dir"));
    }
    if (hasOption("log-conf")) {
        config.setLogConf(getOptionValue("log-conf"));
    }
    if (hasOption("log-file")) {
        config.setLogFile(getOptionValue("log-file"));
    }
}

void db_agg_parser::dumpConfiguration(Configuration& config) {
    cout << "general:" << endl;
        cout << "    environment = " << config.getEnvironment() << endl;
        cout << "    help = " << config.getHelp() << endl;
        cout << "    log-level = " << config.getLogLevel() << endl;
        cout << "    show-progress = " << config.getShowProgress() << endl;
        cout << "    copy-threshold = " << config.getCopyThreshold() << endl;
        cout << "    external-sources" << endl;
        for (auto& item:config.getExternalSources()) {
            cout << "        " << item.first << "=" << item.second << endl;
        }
        cout << "    external-excel-sources" << endl;
        for (auto& item:config.getExternalExcelSources()) {
            cout << "        " << item << endl;
        }
        cout << "    statement-timeout = " << config.getStatementTimeout() << endl;
        cout << "    search-password-in-pg-pass = " << config.getSearchPasswordInPgPass() << endl;
        cout << "    use-reg-exp-parser = " << config.getUseRegExpParser() << endl;
        cout << "    query-parameter" << endl;
        for (auto& item:config.getQueryParameter()) {
            cout << "        " << item.first << "=" << item.second << endl;
        }
        cout << "    dont-execute = " << config.getDontExecute() << endl;
        cout << "    max-parallel-executions = " << config.getMaxParallelExecutions() << endl;
    cout << "cache:" << endl;
        cout << "    disable-cache = " << config.getDisableCache() << endl;
    cout << "files:" << endl;
        cout << "    output-dir = " << config.getOutputDir() << endl;
        cout << "    result-dir = " << config.getResultDir() << endl;
        cout << "    cache-dir = " << config.getCacheDir() << endl;
        cout << "    prefix = " << config.getPrefix() << endl;
        cout << "    database-registry-file = " << config.getDatabaseRegistryFile() << endl;
        cout << "    url-registry-file = " << config.getUrlRegistryFile() << endl;
        cout << "    extension-dir = " << config.getExtensionDir() << endl;
        cout << "    log-conf = " << config.getLogConf() << endl;
        cout << "    log-file = " << config.getLogFile() << endl;
}



}
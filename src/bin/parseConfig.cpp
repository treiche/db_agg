#include "core/Configuration.h"
#include "cli/CommandLineParser.h"

namespace db_agg {

static vector<Argument> arguments = {
   {"query-file", "path to the db_agg query file" }
};

static vector<OptionGroup> optionGroups = {
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
        { 'a', "query-parameter", true, "query parameter" } 
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
        { 'f', "cache-registry-file", true, "path to the cache registry" } ,
        { 'x', "extension-dir", true, "path to the extension directory" } ,
        { 'l', "log-conf", true, "path to the log configuration file" } ,
        { 'F', "log-file", true, "path to the log file" } 
    }}
};

void parseConfiguration(CommandLineParser& parser, Configuration& config) {
    if (parser.hasOption("query-file")) {
        config.setQueryFile(parser.getOptionValue("query-file"));
    }
    if (parser.hasOption("environment")) {
        config.setEnvironment(parser.getOptionValue("environment"));
    }
        config.setHelp(parser.getFlag("help"));
    if (parser.hasOption("log-level")) {
        config.setLogLevel(parser.getOptionValue("log-level"));
    }
        config.setShowProgress(parser.getFlag("show-progress"));
        if (parser.hasOption("copy-threshold")) {
            config.setCopyThreshold(stoi(parser.getOptionValue("copy-threshold")));
        }
        if (parser.hasOption("external-sources")) {
            config.setExternalSources(parser.getOptionMapValue("external-sources"));
        }
        if (parser.hasOption("external-excel-sources")) {
            config.setExternalExcelSources(parser.getOptionListValue("external-excel-sources"));
        }
        if (parser.hasOption("statement-timeout")) {
            config.setStatementTimeout(stoi(parser.getOptionValue("statement-timeout")));
        }
        config.setSearchPasswordInPgPass(parser.getFlag("search-password-in-pg-pass"));
        config.setUseRegExpParser(parser.getFlag("use-reg-exp-parser"));
        if (parser.hasOption("query-parameter")) {
            config.setQueryParameter(parser.getOptionMapValue("query-parameter"));
        }
        config.setDisableCache(parser.getFlag("disable-cache"));
    if (parser.hasOption("output-dir")) {
        config.setOutputDir(parser.getOptionValue("output-dir"));
    }
    if (parser.hasOption("result-dir")) {
        config.setResultDir(parser.getOptionValue("result-dir"));
    }
    if (parser.hasOption("cache-dir")) {
        config.setCacheDir(parser.getOptionValue("cache-dir"));
    }
    if (parser.hasOption("database-registry-file")) {
        config.setDatabaseRegistryFile(parser.getOptionValue("database-registry-file"));
    }
    if (parser.hasOption("cache-registry-file")) {
        config.setCacheRegistryFile(parser.getOptionValue("cache-registry-file"));
    }
    if (parser.hasOption("extension-dir")) {
        config.setExtensionDir(parser.getOptionValue("extension-dir"));
    }
    if (parser.hasOption("log-conf")) {
        config.setLogConf(parser.getOptionValue("log-conf"));
    }
    if (parser.hasOption("log-file")) {
        config.setLogFile(parser.getOptionValue("log-file"));
    }
}

void dumpConfiguration(Configuration& config) {
    cout << "arguments:" << endl;
        cout << "    query-file = " << config.getQueryFile() << endl;
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
    cout << "cache:" << endl;
        cout << "    disable-cache = " << config.getDisableCache() << endl;
    cout << "files:" << endl;
        cout << "    output-dir = " << config.getOutputDir() << endl;
        cout << "    result-dir = " << config.getResultDir() << endl;
        cout << "    cache-dir = " << config.getCacheDir() << endl;
        cout << "    database-registry-file = " << config.getDatabaseRegistryFile() << endl;
        cout << "    cache-registry-file = " << config.getCacheRegistryFile() << endl;
        cout << "    extension-dir = " << config.getExtensionDir() << endl;
        cout << "    log-conf = " << config.getLogConf() << endl;
        cout << "    log-file = " << config.getLogFile() << endl;
}


    void parse(int argc, char **argv, Configuration& config) {
        CommandLineParser parser{"db_agg", arguments, optionGroups};
        vector<string> posArgs = parser.parse(argc,argv);
        if (posArgs.size() != 1) {
            cout << "missing query file argument" << endl;
            cout << parser.getUsage();
            exit(0);
        }
        if (parser.getFlag("help")) {
            cout << parser.getUsage();
            exit(0);
        }
        parseConfiguration(parser,config);
   }

}

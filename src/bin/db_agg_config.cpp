#include <iostream>

#include "installation.h"
#include "utils/Template.h"

#include <log4cplus/configurator.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/fileappender.h>

#include "cli/CommandLineParser.h"
#include "cli/OptionGroup.h"
#include <vector>

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace db_agg;

static vector<OptionGroup> options = {
    {"general",{
        { 'h', "help", false, "show available options" },
        { 'I', "includedir", false, "path to the db_agg include dir" },
        { 'L', "libdir", false, "path to the db_agg libraries"},
        { 'l', "libs", false, "libs used to compile db_agg"},
        { 'V', "localstatedir", false, "localstatedir"},
        { 'p', "prefix", false, "db_agg home directory"},
        { 'v', "version", false, "db_agg version"}
    }}
};


static string interpolate(string path) {
    db_agg::Template t{"${","}"};
    t.set("prefix",DBAGG_PREFIX);
    t.set("exec_prefix", DBAGG_EXEC_PREFIX);
    string ip = t.render(path);
    return t.render(ip);
}


int main(int argc, char **argv) {
    SharedObjectPtr<Appender> append_1(new ConsoleAppender());
    Logger::getRoot().addAppender(append_1);
    Logger::getRoot().setLogLevel(ERROR_LOG_LEVEL);

    CommandLineParser parser("db_agg_config",{},options);
    parser.parse(argc,argv);
    if (parser.hasOption("help")) {
        cout << parser.getUsage() << endl;
        exit(1);
    } else if (parser.hasOption("includedir")) {
        cout << interpolate(DBAGG_INCLUDEDIR) << endl;
    } else if (parser.hasOption("libdir")) {
        cout << interpolate(DBAGG_LIBDIR) << endl;
    } else if (parser.hasOption("libs")) {
        cout << interpolate(DBAGG_LIBS) << endl;
    } else if (parser.hasOption("localstatedir")) {
        cout << interpolate(DBAGG_LOCALSTATEDIR) << endl;
    } else if (parser.hasOption("prefix")) {
        cout << interpolate(DBAGG_PREFIX) << endl;
    } else if (parser.hasOption("version")) {
        cout << DBAGG_VERSION << endl;
    }
}

/*
 * CommandLineParser.cpp
 *
 *  Created on: Jan 16, 2014
 *      Author: arnd
 */


#include "cli/CommandLineParser.h"
#include <getopt.h>
#include <typeinfo>
#include <sstream>
#include <memory>
#include "utils/utility.h"

using namespace std;


namespace db_agg {


CommandLineParser::CommandLineParser(string program, vector<OptionGroup> optionGroups) {
    this->program = program;
    this->optionGroups = optionGroups;
}

vector<string> CommandLineParser::parse(int argc, char**argv) {
    vector<string> args;
    for (int idx=0;idx<argc;idx++) {
        args.push_back(argv[idx]);
    }
    return parse(args);
}

vector<string> CommandLineParser::parse(std::vector<std::string> args) {
    auto options = getOptions();
    unique_ptr<option[]> long_options_ptr(new option[options.size()+1]);
    option *long_options = long_options_ptr.get(); // new option[options.size()+1];
    string shortOptions;
    for (size_t idx = 0; idx < options.size(); idx++) {
        long_options[idx].name = options[idx].longOption.c_str();
        shortOptions += options[idx].shortOption;
        if (options[idx].hasArgs) {
            long_options[idx].has_arg = required_argument;
            shortOptions += ':';
        } else {
            long_options[idx].has_arg = no_argument;
        }
        long_options[idx].flag = 0;
        long_options[idx].val = 256 + idx;
    }
    long_options[options.size()] = {nullptr,0,nullptr,0};
    int option_index = 0;
    // opterr = 0; // disable getopt logging
    do {
        int c = getopt_long(args.size(),(char**)args.data(),shortOptions.c_str(),long_options,&option_index);
        if (c == -1) {
            break;
        }
        //cout << "c=" << c << endl;
        if (c=='?') {
            cout << "ignoring unknown option '" << args[optind-1] << endl;
            //throw runtime_error("unknown option");
        }
        if (c>255) {
            const char *val = "";
            if (options[option_index].hasArgs) {
                val = optarg;
            }
            arguments[options[option_index].longOption].push_back(val);
        } else {
            for (size_t idx=0; idx<options.size(); idx++) {
                if (options[idx].shortOption == c) {
                    const char *val = "";
                    if (options[idx].hasArgs) {
                        val = optarg;
                    }
                    arguments[options[idx].longOption].push_back(val);
                }
            }
        }
    } while (true);

    vector<string> posArgs;
    if (optind < static_cast<int>(args.size())) {
        while (optind < static_cast<int>(args.size())) {
            posArgs.push_back(args[optind++]);
        }
    }

    for (auto& optionGroup:optionGroups) {
        if (optionGroup.name == "arguments") {
            if (optionGroup.options.size() != posArgs.size()) {
                throw runtime_error("wrong number of positional arguments");
            }
            int argLen = posArgs.size();
            for (int cnt=0;cnt<argLen;cnt++) {
                auto& option = optionGroup.options[cnt];
                // cout << "set position arg " << option.longOption << " to " << posArgs[cnt] << endl;
                arguments[option.longOption].push_back(posArgs[cnt]);
            }
        }
    }

    return posArgs;
}

bool CommandLineParser::hasOption(std::string name) {
    if (arguments.find(name)==arguments.end()) {
        return false;
    }
    if (arguments[name].empty()) {
        return false;
    }
    return true;
}

vector<Option> CommandLineParser::getOptions() {
    vector<Option> options;
    for (auto& group:optionGroups) {
        for (auto& option:group.getOptions()) {
            options.push_back(option);
        }
    }
    return options;
}

string CommandLineParser::getOptionValue(string optName) {
    return arguments[optName].at(0);
}

vector<string> CommandLineParser::getOptionListValue(string optName) {
    return arguments[optName];
}

map<string, string> CommandLineParser::getOptionMapValue(std::string optName) {
    vector<string> entries = getOptionListValue(optName);
    map<string, string> result;
    for (string entry:entries) {
        vector<string> splitted;
        split(entry,'=',splitted);
        if (splitted.size() != 2) {
            cout << "invalid map value '" << entry << "'" << endl;
        } else {
            result[splitted[0]] = splitted[1];
        }
    }
    return result;
}

bool CommandLineParser::getFlag(string optName) {
    return arguments.find(optName) != arguments.end();
}

string CommandLineParser::getUsage() {
    stringstream usage;
    usage << "usage: " << program << " [OPTIONS]" << endl;
    for (auto& group:optionGroups) {
        usage << "  " << group.name << ":" << endl;
        for (auto& option:group.options) {
            usage << "    -" << option.shortOption << " --" << option.longOption << endl;
            usage << "        " << option.description  << endl;
        }
        usage << endl;
    }
    return usage.str();
}


}


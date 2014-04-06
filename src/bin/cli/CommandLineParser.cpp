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


CommandLineParser::CommandLineParser(string program, vector<Argument> arguments, vector<OptionGroup> optionGroups) {
    this->program = program;
    this->optionGroups = optionGroups;
    this->arguments = arguments;
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
            values[options[option_index].longOption].push_back(val);
        } else {
            for (size_t idx=0; idx<options.size(); idx++) {
                if (options[idx].shortOption == c) {
                    const char *val = "";
                    if (options[idx].hasArgs) {
                        val = optarg;
                    }
                    values[options[idx].longOption].push_back(val);
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

    if (posArgs.size() != arguments.size()) {
        throw runtime_error("wrong number of positional arguments");
    }

    for (int cnt=0; cnt<posArgs.size(); cnt++) {
        values[arguments[cnt].name].push_back(posArgs[cnt]);
    }
    return posArgs;
}

bool CommandLineParser::hasOption(std::string name) {
    if (values.find(name)==values.end()) {
        return false;
    }
    if (values[name].empty()) {
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
    return values[optName].at(0);
}

vector<string> CommandLineParser::getOptionListValue(string optName) {
    return values[optName];
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
    return values.find(optName) != values.end();
}

string CommandLineParser::getUsage() {
    stringstream usage;
    usage << "usage: " << program << " [OPTIONS]";
    for (auto& argument:arguments) {
        usage << " <" << argument.name << ">";
    }
    usage << endl << endl;
    usage << "ARGUMENTS:" << endl;
    for (auto& argument:arguments) {
        usage << "  " << argument.name << ": " << argument.description << endl;
    }
    usage << endl;
    usage << "OPTIONS:" << endl;
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


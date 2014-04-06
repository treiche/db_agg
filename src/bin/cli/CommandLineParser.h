/*
 * CommandLineParser.h
 *
 *  Created on: Jan 16, 2014
 *      Author: arnd
 */

#ifndef COMMANDLINEPARSER_H_
#define COMMANDLINEPARSER_H_

#include <map>
#include <string>
#include <vector>

#include "cli/Option.h"
#include "cli/Argument.h"
#include "cli/OptionGroup.h"
#include "cli/ArgumentConverter.h"

namespace db_agg {
class CommandLineParser {
    std::string program;
    std::map<std::string, std::vector<std::string>> values;
    std::vector<OptionGroup> optionGroups;
    std::vector<Argument> arguments;
public:
    CommandLineParser(std::string program, std::vector<Argument> arguments, std::vector<OptionGroup> optionGroups);
    std::vector<std::string> parse(int argc, char **argv);
    std::vector<std::string> parse(std::vector<std::string> args);
    std::vector<Option> getOptions();
    bool getFlag(std::string name);
    std::string getUsage();
    std::string getOptionValue(std::string name);
    std::vector<std::string> getOptionListValue(std::string name);
    std::map<std::string, std::string> getOptionMapValue(std::string name);
    bool hasOption(std::string name);
    template<typename T>
    void getOptionValue(std::string name, T& value) {
        ArgumentConverter<T> ac;
        value = ac.convert(values[name].at(0));
    }
};
}



#endif /* COMMANDLINEPARSER_H_ */

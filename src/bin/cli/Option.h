/*
 * Option.h
 *
 *  Created on: Jan 16, 2014
 *      Author: arnd
 */

#ifndef OPTION_H_
#define OPTION_H_

#include <tuple>
#include <iostream>
#include <initializer_list>
#include <string>
#include <typeinfo>

namespace db_agg {

enum class OptionType {
    STRING,
    INT,
    LIST
};

class Option {
    char shortOption = 0;
    std::string longOption;
    bool hasArgs;
    std::string description;
    OptionType type;
public:
    Option(char shortOption, std::string longOption, bool hasArgs, std::string description, OptionType type=OptionType::STRING);

    friend class CommandLineParser;
};
}



#endif /* OPTION_H_ */

/*
 * Option.cpp
 *
 *  Created on: Jan 16, 2014
 *      Author: arnd
 */

#include <initializer_list>

#include "cli/Option.h"


using namespace std;

namespace db_agg {

Option::Option(char shortOption, string longOption, bool hasArgs, string description, OptionType type):
    shortOption(shortOption),
    longOption(longOption),
    hasArgs(hasArgs),
    description(description),
    type(type) {

    }
}


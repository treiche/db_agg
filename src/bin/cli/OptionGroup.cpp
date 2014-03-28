/*
 * OptionGroup.cpp
 *
 *  Created on: Jan 18, 2014
 *      Author: arnd
 */

#include "cli/OptionGroup.h"

using namespace std;

namespace db_agg {
OptionGroup::OptionGroup(string name, vector<Option> options): name(name), options(options) {

}

vector<Option>& OptionGroup::getOptions() {
    return options;
}
}




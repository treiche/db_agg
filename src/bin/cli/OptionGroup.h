/*
 * OptionGroup.h
 *
 *  Created on: Jan 18, 2014
 *      Author: arnd
 */

#ifndef OPTIONGROUP_H_
#define OPTIONGROUP_H_

#include <string>
#include <vector>

#include "cli/Option.h"

namespace db_agg {
class OptionGroup {
private:
    std::string name;
    std::vector<Option> options;
public:
    OptionGroup(std::string name, std::vector<Option> options);
    std::vector<Option>& getOptions();
    friend class CommandLineParser;

};
}



#endif /* OPTIONGROUP_H_ */

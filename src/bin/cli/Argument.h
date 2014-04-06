/*
 * Argument.h
 *
 *  Created on: Mar 30, 2014
 *      Author: arnd
 */

#ifndef ARGUMENT_H_
#define ARGUMENT_H_

namespace db_agg {
class Argument {
private:
    std::string name;
    std::string description;
public:
    Argument(std::string name, std::string description) {
        this->name = name;
        this->description = description;
    }
    friend class CommandLineParser;
};
}



#endif /* ARGUMENT_H_ */

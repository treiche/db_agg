/*
 * ArgumentConverter.h
 *
 *  Created on: Jan 17, 2014
 *      Author: arnd
 */

#ifndef ARGUMENTCONVERTER_H_
#define ARGUMENTCONVERTER_H_

#include <stddef.h>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <typeinfo>

namespace db_agg {
template<typename T>
class ArgumentConverter {
public:
    size_t getTypeId() {
        return typeid(T).hash_code();
    }
    T convert(std::string arg);
};

template<>
int ArgumentConverter<int>::convert(std::string arg);
template<>
double ArgumentConverter<double>::convert(std::string arg);
template<>
bool ArgumentConverter<bool>::convert(std::string arg);

}
#endif /* ARGUMENTCONVERTER_H_ */

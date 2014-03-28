/*
 * ArgumentConverter.cpp
 *
 *  Created on: Jan 18, 2014
 *      Author: arnd
 */


#include "ArgumentConverter.h"

using namespace std;

namespace db_agg {

template<>
int ArgumentConverter<int>::convert(string arg) {
    return atoi(arg.c_str());
}

template<>
double ArgumentConverter<double>::convert(string arg) {
    return atof(arg.c_str());
}

template<>
long ArgumentConverter<long>::convert(string arg) {
    return atol(arg.c_str());
}


template<>
bool ArgumentConverter<bool>::convert(string arg) {
    if (arg.compare("true") == 0) {
        return true;
    }
    if (arg.compare("false") == 0) {
        return false;
    }
    throw std::runtime_error("wrong bool repr " + arg);
}

}

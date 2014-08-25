/*
 * string.h
 *
 *  Created on: Aug 23, 2014
 *      Author: arnd
 */

#ifndef STRING_H_
#define STRING_H_

#include <string>
#include <vector>

namespace db_agg {

std::string thousand_grouping(uint64_t number);
std::string cutBlock(std::string text);
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::string join(std::vector<std::string> v, std::string delim);
std::string trim(std::string& str);
std::string string_format(const std::string &fmt, ...);
std::string replace_all(std::string str, std::string search, std::string replacement);

}



#endif /* STRING_H_ */

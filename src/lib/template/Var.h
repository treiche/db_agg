/*
 * Var.h
 *
 *  Created on: Nov 16, 2014
 *      Author: arnd
 */

#ifndef VAR_H_
#define VAR_H_

#include <string>
#include <vector>

namespace db_agg {

enum class VarType {
	LIST,
	MAP,
	BOOL,
	STRING,
	INTEGER,
	DOUBLE,
	VOID
};

class Var {
public:
    virtual ~Var();
    virtual size_t size(std::string path) = 0;
    virtual std::vector<std::string> keys(std::string path) = 0;
    virtual VarType type(std::string path) = 0;
    virtual std::string get_string(std::string path) = 0;
    virtual bool get_bool(std::string path) = 0;
    virtual int get_integer(std::string path) = 0;
    virtual double get_double(std::string path) = 0;
};
}



#endif /* VAR_H_ */

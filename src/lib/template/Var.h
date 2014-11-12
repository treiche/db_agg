/*
 * Var.h
 *
 *  Created on: Oct 31, 2014
 *      Author: arnd
 */

#ifndef VAR_H_
#define VAR_H_

#include <algorithm>
#include <map>
#include "any.h"

extern "C" {
    #include "jansson.h"
}

namespace db_agg {
class Var {
private:
	std::string name;
	any value;
	void fromJson(std::string path, json_t *json);
	json_t *toJson(any& value);
public:
	Var();
	virtual ~Var();
	Var(std::string name);
	Var(std::string name, any value);
	virtual any& get(std::string path);
	any& get();
	void set(std::string path, any value);
	void createList(std::string path);
	void fromJson(std::string json);
	std::string toJson(std::string path);
	std::string getName();
	size_t size(std::string path);
	std::vector<std::string> keys(std::string path);
	bool isList(std::string path);
	bool isMap(std::string path);
};

std::ostream& operator<<(std::ostream&, any);

}



#endif /* VAR_H_ */

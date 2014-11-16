/*
 * JsonVar.h
 *
 *  Created on: Oct 31, 2014
 *      Author: arnd
 */

#ifndef JSONVAR_H_
#define JSONVAR_H_

#include <algorithm>
#include <map>
#include "any.h"
#include "Var.h"

extern "C" {
    #include "jansson.h"
}

namespace db_agg {
class JsonVar: public Var {
private:
	std::string name;
	any value;
	void fromJson(std::string path, json_t *json);
	json_t *toJson(any& value);
	any& get(std::string path);
	any& get();
public:
	JsonVar();
	virtual ~JsonVar();
	JsonVar(std::string name);
	JsonVar(std::string name, any value);
	void set(std::string path, any value);
	void createList(std::string path);
	void fromJson(std::string json);
	std::string toJson(std::string path);
	std::string getName();
	// Var API
	virtual size_t size(std::string path) override;
	virtual std::vector<std::string> keys(std::string path) override;
	virtual VarType type(std::string path) override;
	virtual std::string get_string(std::string path) override;
    virtual bool get_bool(std::string path) override;
    virtual int get_integer(std::string path) override;
    virtual double get_double(std::string path) override;
};

std::ostream& operator<<(std::ostream&, any);

}



#endif /* JSONVAR_H_ */

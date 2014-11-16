/*
 * Var.cpp
 *
 *  Created on: Oct 31, 2014
 *      Author: arnd
 */

#include <iostream>
#include "JsonVar.h"
#include "utils/logging.h"


using namespace std;

namespace db_agg {

DECLARE_LOGGER("JsonVar")

static any emptyAny = string("");

JsonVar::JsonVar() {}

JsonVar::JsonVar(string name, any value):
	name(name),
	value(value) {
}

JsonVar::JsonVar(string name):
	name(name) {
}

JsonVar::~JsonVar() {
	value = 0;
}

any& JsonVar::get(string path) {
	if (path == name) {
		return value;
	}
	// split to parent and child
	auto idx = path.rfind(".");
	string parentPath = path.substr(0,idx);
	string childName = path.substr(idx + 1, path.size() - idx - 1);
	LOG_DEBUG("parentPath = " << parentPath);
	LOG_DEBUG("childName = " << childName);
	any& parent = get(parentPath);
	LOG_DEBUG("parent = " << parent);

	if (parent.type() == typeid(map<string,any>)) {
		map<string,any>& parentValue = any_cast<map<string,any>&>(parent);
		any& av = parentValue[childName];
		return av;
	} else if (parent.type() == typeid(vector<any>)) {
		for (char& c:childName) {
			if (!isdigit(c)) {
				THROW_EXC("path " << path << " is not a index");
			}
		}
		size_t idx = stoi(childName);
		vector<any>& parentValue = any_cast<vector<any>&>(parent);
		if (idx >= parentValue.size()) {
			LOG_WARN("about to get idx " << idx << " but list size is " << parentValue.size());
			LOG_WARN("path = " << path);
		}
		any& av = parentValue.at(idx);
		return av;
	} else {
		// THROW_EXC("unknown type " << string(parent.type().name()) << " [path=" << path << "]");
	}
	return emptyAny;
}

any& JsonVar::get() {
	return value;
}

VarType JsonVar::type(std::string path) {
	any& a = get(path);
	if (a.type() == typeid(string)) {
		return VarType::STRING;
	} else if (a.type() == typeid(int)) {
		return VarType::INTEGER;
	} else if (a.type() == typeid(bool)) {
		return VarType::BOOL;
	} else if (a.type() == typeid(double)) {
		return VarType::DOUBLE;
	} else if (a.type() == typeid(vector<any>)) {
		return VarType::LIST;
	} else if (a.type() == typeid(map<any,string>)) {
		return VarType::MAP;
	}
	THROW_EXC("path '" << path << "' has unknown type '" << a.type().name() << "'");
}


string JsonVar::get_string(string path) {
	any& a = get(path);
	stringstream ss;
	ss << a;
	return ss.str();
}

void JsonVar::set(std::string path, any value) {
	if (path == name) {
		this->value = value;
		return;
	}
	auto idx = path.rfind(".");
	string parentPath = path.substr(0,idx);
	string childName = path.substr(idx + 1, path.size() - idx - 1);
	any& parent = get(parentPath);
	if (parent.type() == typeid(map<string,any>)) {
		map<string,any>& parentValue = any_cast<map<string,any>&>(parent);
		parentValue[childName] = value;
	} else if (parent.type() == typeid(vector<any>)) {
		size_t idx = stoi(childName);
		vector<any>& parentValue = any_cast<vector<any>&>(parent);
		if (idx > parentValue.size()) {
			THROW_EXC("out_of_bounds");
		}
		if (idx == parentValue.size()) {
			parentValue.push_back(value);
		} else {
			parentValue.at(idx) = value;
		}
	} else {
		THROW_EXC("unknown type " << string(parent.type().name()) << " [path=" << path << "]");
	}
}

void JsonVar::createList(std::string path) {
	auto idx = path.rfind(".");
	string parentPath = path.substr(0,idx);
	string childName = path.substr(idx + 1, path.size() - idx - 1);
	any& parent = get(parentPath);
}

void JsonVar::fromJson(std::string path, json_t *json) {
	if (json_typeof(json) == JSON_ARRAY) {
		set(path,vector<any>());
		size_t len = json_array_size(json);
		for (size_t idx = 0; idx < len; idx++) {
			json_t *item = json_array_get(json,idx);
			fromJson(path + "." + to_string(idx),item);
		}
		return;
	}
	if (json_typeof(json) == JSON_OBJECT) {
		set(path,map<string,any>());
		const char *key;
		json_t *ov;
		json_object_foreach(json, key, ov) {
			fromJson(path + "." + key, ov);
		}
		return;
	}
	if (json_typeof(json) == JSON_STRING) {
		string item = string(json_string_value(json));
		set(path, item);
		return;
	}
	if (json_typeof(json) == JSON_TRUE) {
		set(path, true);
		return;
	}
	if (json_typeof(json) == JSON_FALSE) {
		set(path, false);
		return;
	}
	if (json_typeof(json) == JSON_INTEGER) {
		int item = json_integer_value(json);
		set(path, item);
		return;
	}
	if (json_typeof(json) == JSON_REAL) {
		double item = json_real_value(json);
		set(path, item);
		return;
	}
	if (json_typeof(json) == JSON_NULL) {
		void *item = 0;
		set(path,item);
		return;
	}
	THROW_EXC("unknown type");
}


void JsonVar::fromJson(string json) {
	json_error_t error;
	json_t *js = json_loads(json.c_str(),0,&error);
	if (!js) {
		THROW_EXC("invalid json");
	}
	fromJson(name,js);
	json_decref(js);
}

json_t *JsonVar::toJson(any& value) {
	if (value.type() == typeid(string)) {
		json_t *ret = json_string(any_cast<string>(value).c_str());
		return ret;
	} else if (value.type() == typeid(bool)) {
		bool bv = any_cast<bool>(value);
		if (bv) {
			return json_true();
		} else {
			return json_false();
		}
	} else if (value.type() == typeid(map<string,any>)) {
		map<string,any>& mv = any_cast<map<string,any>&>(value);
		json_t *ret = json_object();
		for (auto& v:mv) {
			json_object_set(ret,v.first.c_str(),toJson(v.second));
		}
		return ret;
	} else if (value.type() == typeid(vector<any>)) {
		vector<any>& vv = any_cast<vector<any>&>(value);
		json_t *ret = json_array();
		for (auto& v:vv) {
			json_array_append(ret,toJson(v));
		}
		return ret;
	} else if (value.type() == typeid(int)) {
		int iv = any_cast<int>(value);
		json_t *ret = json_integer(iv);
		return ret;
	} else if (value.type() == typeid(double)) {
		double iv = any_cast<double>(value);
		json_t *ret = json_real(iv);
		return ret;
	} else if (value.type() == typeid(void*)) {
		void *vp = any_cast<void*>(value);
		if (vp == nullptr) {
			return json_null();
		}
	} else {
		THROW_EXC("unknown type " << value.type().name())
	}
	return json_null();
}

string JsonVar::toJson(string path) {
	any& v = get(path);
	const json_t *js = toJson(v);
	char *ret = json_dumps(js,JSON_INDENT(2));
	return string(ret);
}

string JsonVar::getName() {
	return name;
}

size_t JsonVar::size(string path) {
	any& av = get(path);
	if (av.type() != typeid(vector<any>)) {
		THROW_EXC("path " << path << " is not a list [" << av.type().name() << "]");
	}
	vector<any>& list = any_cast<vector<any>&>(av);
	return list.size();
}

vector<std::string> JsonVar::keys(string path) {
	any& mv = get(path);
	if (mv.type() != typeid(map<string,any>)) {
		THROW_EXC(path << " is not a map");
	}
	map<string,any>& m = any_cast<map<string,any>&>(mv);
	vector<string> ks;
	for (auto k:m) {
		ks.push_back(k.first);
	}
	return ks;
}

bool JsonVar::get_bool(std::string path) {
	any& av = get(path);
	if (av.type() == typeid(bool)) {
		return any_cast<bool>(av);
	}
	THROW_EXC("path '" << path << "' does not hold a boolean value [type='" << av.type().name() << "'");
}

int JsonVar::get_integer(std::string path) {
	any& av = get(path);
	if (av.type() == typeid(int)) {
		return any_cast<int>(av);
	}
	THROW_EXC("path '" << path << "' does not hold an integer value [type='" << av.type().name() << "'");
}

double JsonVar::get_double(std::string path) {
	any& av = get(path);
	if (av.type() == typeid(double)) {
		return any_cast<bool>(av);
	}
	THROW_EXC("path '" << path << "' does not hold a double value [type='" << av.type().name() << "'");
}

ostream& operator<<(ostream& os, any value) {
	if (value.type() == typeid(string)) {
		os << any_cast<string>(value);
	} else if (value.type() == typeid(bool)) {
		os << any_cast<bool>(value);
	} else if (value.type() == typeid(int)) {
		os << any_cast<int>(value);
	} else if (value.type() == typeid(double)) {
		os << any_cast<double>(value);
	} else if (value.type() == typeid(void*)) {
		os << any_cast<void*>(value);
	} else if (value.type() == typeid(vector<any>)) {
		os << "[";
		vector<any>& v = any_cast<vector<any>&>(value);
		for (size_t idx = 0; idx < v.size(); idx++) {
			os << v[idx];
			if (idx < v.size() -1) {
				os << ",";
			}
		}
		os << "]";
	} else if (value.type() == typeid(map<string,any>)) {
		os << "{";
		map<string,any>& v = any_cast<map<string,any>&>(value);
		int len = v.size();
		size_t idx = 0;
		for (auto& p:v) {
			os << p.first << ":" << p.second;
			if (idx < len -1) {
				os << ",";
			}
			idx++;
		}
		os << "}";
	} else if (value.type() == typeid(void)) {
		os << "null";
	} else {
		THROW_EXC("unknown type " << string(value.type().name()));
	}
	return os;
}

}



/*
 * Port.h
 *
 *  Created on: Dec 30, 2014
 *      Author: arnd
 */

#ifndef PORT_H_
#define PORT_H_

#include <string>
#include <memory>
#include "table/TableData.h"

namespace db_agg {

class Port {
private:
    std::string id;
    std::string name;
    std::shared_ptr<TableData> result;
public:
    Port(std::string id, std::string name);
    std::string getId();
    void setId(std::string portId);
    std::string getName();
    void setResult(std::shared_ptr<TableData> result);
    std::shared_ptr<TableData> getResult();
    void release();
};

}



#endif /* PORT_H_ */

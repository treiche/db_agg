/*
 * DataReceiver.h
 *
 *  Created on: Jun 7, 2014
 *      Author: arnd
 */

#ifndef DATARECEIVER_H_
#define DATARECEIVER_H_

#include <memory>
#include "table/TableData.h"

namespace db_agg {
class DataReceiver {
public:
    virtual ~DataReceiver() {};
    virtual void receive(std::string name, std::shared_ptr<TableData> data) = 0;
};
}



#endif /* DATARECEIVER_H_ */

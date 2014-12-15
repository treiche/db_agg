/*
 * SoapExecution.h
 *
 *  Created on: Dec 6, 2014
 *      Author: arnd
 */

#ifndef SOAPEXECUTION_H_
#define SOAPEXECUTION_H_

#include "table/TableData.h"
#include "utils/DomToTable.h"
#include "utils/TableToDom.h"
#include "core/Url.h"
#include <memory>

namespace db_agg {
class SoapExecution: public db_agg::QueryExecution {
private:
    uint64_t lastOffset = 0;
    uint64_t chunkSize = 1;
    std::shared_ptr<db_agg::TableData> resultTable;
    TableToDom inputTemplate;
    DomToTable outputTemplate;
    std::string callServer(std::shared_ptr<Url> url, std::string request);
    void prepareQuery();
public:
    virtual bool process() override;
    virtual bool isResourceAvailable() override;
};
}

#endif /* SOAPEXECUTION_H_ */

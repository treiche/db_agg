/*
 * DbAggProcessingApi.h
 *
 *  Created on: Dec 18, 2014
 *      Author: arnd
 */

#ifndef DBAGGPROCESSINGAPI_H_
#define DBAGGPROCESSINGAPI_H_

#include <string>

namespace db_agg {

class DbAggProcessingApi {
private:
    class XImpl;
    XImpl *pImpl;
public:
    DbAggProcessingApi();
    ~DbAggProcessingApi();
    void configure(std::string config);
    bool run(bool async);
    std::string receive();
    void stop();
};


}
#endif /* DBAGGPROCESSINGAPI_H_ */

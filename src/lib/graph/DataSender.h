/*
 * DataSender.h
 *
 *  Created on: Jun 7, 2014
 *      Author: arnd
 */

#ifndef DATASENDER_H_
#define DATASENDER_H_

#include "Channel.h"

namespace db_agg {
class DataSender {
public:
    virtual ~DataSender() {};
    virtual void addChannel(Channel* channel) = 0;
};
}




#endif /* DATASENDER_H_ */

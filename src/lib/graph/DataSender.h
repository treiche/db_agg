/*
 * DataSender.h
 *
 *  Created on: Jun 7, 2014
 *      Author: arnd
 */

#ifndef DATASENDER_H_
#define DATASENDER_H_


namespace db_agg {
class Channel;

class DataSender {
public:
    virtual ~DataSender() {};
    virtual void addChannel(Channel* channel) = 0;
};
}




#endif /* DATASENDER_H_ */

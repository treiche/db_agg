/*
 * ManyToOne.h
 *
 *  Created on: Sep 7, 2014
 *      Author: arnd
 */

#ifndef MANYTOONE_H_
#define MANYTOONE_H_

#include "QueryExecution.h"

namespace db_agg {
class ManyToOne: public QueryExecution {
public:
	virtual bool process() override;
	virtual bool isTransition() override;
};
}



#endif /* MANYTOONE_H_ */

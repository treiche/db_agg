/*
 * parseConfig.h
 *
 *  Created on: Mar 17, 2014
 *      Author: arnd
 */

#ifndef PARSECONFIG_H_
#define PARSECONFIG_H_

#include "cli/CommandLineParser.h"
#include "core/Configuration.h"

namespace db_agg {

void parseConfiguration(CommandLineParser& parser, Configuration& config);
void dumpConfiguration(Configuration& config);
void parse(int argc, char **argv, Configuration& config);

}

#endif /* PARSECONFIG_H_ */

/*
 * ExcelToTextFormat.h
 *
 *  Created on: Mar 1, 2014
 *      Author: arnd
 */

#ifndef EXCELTOTEXTFORMAT_H_
#define EXCELTOTEXTFORMAT_H_

#include <string>
#include <functional>
#include <map>
#include <memory>

#include "table/TableData.h"

extern "C" {
	#include <zip.h>
	#include <libxml/xmlreader.h>
}

namespace db_agg {

class ExcelToTextFormat {
private:
	struct ParseState;
	struct zip *archive;
	std::vector<std::string> sharedStrings;
	std::map<std::string,std::string> sheetNamesById;
	void parseEntry(std::string entry,std::function<void (xmlTextReaderPtr)> handler);
	void parseData(xmlTextReaderPtr reader, ParseState *state);
	std::pair<uint32_t,uint32_t> regionToPoint(std::string region);
public:
	std::map<std::string,std::shared_ptr<TableData>> transform(std::string excelFile);
	std::vector<std::string> getSheetNames(std::string excelFile);
};

}



#endif /* EXCELTOTEXTFORMAT_H_ */

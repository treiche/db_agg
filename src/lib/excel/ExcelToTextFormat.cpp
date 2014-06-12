/*
 * ExcelToTextFormat.cpp
 *
 *  Created on: Mar 1, 2014
 *      Author: arnd
 */


#include "excel/ExcelToTextFormat.h"
#include <log4cplus/logger.h>

#include "table/TableDataFactory.h"
#include "utils/utility.h"

using namespace std;
using namespace log4cplus;


namespace db_agg {
static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("AsyncQueryExecutor"));

struct ExcelToTextFormat::ParseState {
	uint32_t currentRow = 0;
	uint32_t currentCol = 0;
	string data{""};
	vector<string> columns;
	bool insideSheetData = false;
	bool sharedValue = false;
};

map<string,shared_ptr<TableData>> ExcelToTextFormat::transform(string excelFile) {
    int error;
    this->archive = zip_open(excelFile.c_str(),ZIP_CHECKCONS,&error);
    if (this->archive==0) {
    	LOG4CPLUS_ERROR(LOG,"unable to open excel file " << excelFile);
        throw runtime_error("unable to open archive");
    }
    map<string,shared_ptr<TableData>> result;
    uint32_t stringIndex = 0;
    parseEntry("xl/sharedStrings.xml", [&] (xmlTextReaderPtr reader) {
        int nodeType = xmlTextReaderNodeType(reader);
        string nodeName = (const char*)xmlTextReaderName(reader);
        LOG4CPLUS_DEBUG(LOG,"name = " << nodeName << " type = " << nodeType );
        if (nodeType == XML_READER_TYPE_END_ELEMENT) {
        	if (nodeName == "si") {
        		stringIndex++;
        	}
        } else if (nodeType == XML_READER_TYPE_TEXT) {
            string strVal = (const char*)xmlTextReaderValue(reader);
            LOG4CPLUS_TRACE(LOG,"register " << strVal);
            if (sharedStrings.size()-1==stringIndex) {
            	sharedStrings[stringIndex] += strVal;
            } else {
            	sharedStrings.push_back(strVal);
            }
        }
    });
    LOG4CPLUS_TRACE(LOG, "parsed " << sharedStrings.size() << " shared string");
    parseEntry("xl/workbook.xml", [&] (xmlTextReaderPtr reader) {
        int nodeType = xmlTextReaderNodeType(reader);
        xmlChar* nodeName = xmlTextReaderName(reader);
        if (nodeType == XML_READER_TYPE_ELEMENT) {
        	string elementName((const char*)nodeName);
        	if (elementName == "sheet") {
        		xmlTextReaderMoveToAttribute(reader,(xmlChar*)"name");
        		string sheetName = (const char*)xmlTextReaderValue(reader);
        		LOG4CPLUS_DEBUG(LOG,"sheetName = " << sheetName);
        		xmlTextReaderMoveToAttribute(reader,(xmlChar*)"sheetId");
        		string sheetId = (const char*)xmlTextReaderValue(reader);
        		LOG4CPLUS_DEBUG(LOG,"sheetId = " << sheetId);
        		LOG4CPLUS_TRACE(LOG, sheetId << " -> " << sheetName);
        		sheetNamesById[sheetId] = sheetName;
        	}
        }
    });
    //parseSheetNames();
    for (auto& sheet:sheetNamesById) {
    	ParseState state;
    	parseEntry("xl/worksheets/sheet" + sheet.first + ".xml", [&] (xmlTextReaderPtr reader) {
    		this->parseData(reader, &state);
    	});
    	LOG4CPLUS_TRACE(LOG, "data = \n" << state.data);
    	shared_ptr<TableData> data = TableDataFactory::getInstance().create("text", state.columns);
    	data->appendRaw((void*)state.data.c_str(),state.data.size());
    	result[sheet.second] = data;
    }
    return result;
}

void ExcelToTextFormat::parseData(xmlTextReaderPtr reader, ParseState *state) {
    int nodeType = xmlTextReaderNodeType(reader);
    string nodeName{(const char*)xmlTextReaderName(reader)};
    string delim = "\t";
    // LOG4CPLUS_DEBUG(LOG, "parseData " << nodeName);
    if (nodeType == XML_READER_TYPE_TEXT && state->insideSheetData) {
		string valueId = (const char *)xmlTextReaderValue(reader);
		LOG4CPLUS_TRACE(LOG, "valueId = " << valueId << " is shared = " << state->sharedValue);
		string colVal = valueId;
		if (state->sharedValue) {
			int stringId = stoi(valueId);
			colVal = sharedStrings[stringId];
		}
		LOG4CPLUS_TRACE(LOG, "colVal = " << colVal);
    	if (state->currentRow == 0) {
    		state->columns.push_back(colVal);
    		LOG4CPLUS_TRACE(LOG, "got header " << colVal);
    		state->currentCol++;
    	} else {
    		LOG4CPLUS_TRACE(LOG, "append " << colVal);
    		if (state->currentCol >= state->columns.size()) {
    			return;
    		}
    		// TODO: make utility function
    		string escaped;
    		for (char c:colVal) {
    			switch (c) {
    				case '\r': escaped += "\\r"; break;
    				case '\n': escaped += "\\n"; break;
    				case '\t': escaped += "\\t"; break;
    				case '\v': escaped += "\\v"; break;
    				default:
    					escaped += c;
    			}
    		}
    		state->data += escaped;
    	}
    } else if (nodeType == XML_READER_TYPE_ELEMENT) {
    	if (nodeName == "sheetData") {
    		state->insideSheetData = true;
    	} else if (nodeName == "c") {
    		int typeAttr = xmlTextReaderMoveToAttribute(reader,(xmlChar*)"t");
    		if (typeAttr) {
    			string type = (const char*)xmlTextReaderValue(reader);
    			if (type == "s") {
    				state->sharedValue = true;
    			} else {
    				state->sharedValue = false;
    			}
    		} else {
    			state->sharedValue = false;
    		}
    		xmlTextReaderMoveToAttribute(reader,(xmlChar*)"r");
    		string region = (const char*)xmlTextReaderValue(reader);
    		auto coord = regionToPoint(region);
    		int gap = 0;
    		if (coord.second > state->columns.size()) {
    			LOG4CPLUS_TRACE(LOG, "ignore orphan value");
    			gap = state->columns.size() - state->currentCol;
    		} else {
    			gap = coord.second - state->currentCol;
    		}
    		if (gap > 0) {
				LOG4CPLUS_TRACE(LOG, "coord = " << coord.first << "," << coord.second << " current col = " << state->currentCol);
				LOG4CPLUS_TRACE(LOG, "col gap " << gap);
				for (uint32_t cnt=0;cnt<gap;cnt++) {
					LOG4CPLUS_TRACE(LOG, "insert null value " << cnt);
					state->data += "\\N";
					if (state->currentCol < state->columns.size()-1) {
						state->data += delim;
					}
					state->currentCol++;
				}
    		}
    	}
    } else if (nodeType == XML_READER_TYPE_END_ELEMENT) {
    	if (nodeName == "sheetData") {
    		state->insideSheetData = false;
    	} else if (nodeName == "row") {
    		if (state->currentRow > 0) {
    			if (state->currentCol != state->columns.size()) {
    				for (uint32_t cnt=state->currentCol;cnt<state->columns.size();cnt++) {
    					state->data += "\\N";
    					if (cnt < state->columns.size()-1) {
    						state->data += delim;
    					}
    					state->currentCol++;
    				}
    			}
    			state->data += "\n";
    		}
    		state->currentRow++;
    		state->currentCol = 0;
    	} else if (nodeName == "c") {
    		state->currentCol++;
    		if (state->currentRow>0 && state->currentCol < state->columns.size()) {
    			state->data += delim;
    		}
    	}
    }
}


pair<uint32_t,uint32_t> ExcelToTextFormat::regionToPoint(std::string region) {
	uint32_t row = 0;
	uint32_t col = 0;
	col = region[0]-65;
	row = stoi(region.substr(1));
	return make_pair(row,col);
}

void ExcelToTextFormat::parseEntry(std::string entry,std::function<void (xmlTextReaderPtr)> handler) {
    struct zip_stat info;
    if (zip_stat(archive,entry.c_str(),ZIP_STAT_SIZE,&info) == 0) {
        zip_file *zipEntry = zip_fopen(archive,entry.c_str(),0);
        char *buf = new char[info.size+1];
        int ret = zip_fread(zipEntry,buf,info.size);
        if (ret != info.size) {
            LOG4CPLUS_ERROR(LOG,"zip_fread returned " << ret);
        }
        zip_fclose(zipEntry);
        LOG4CPLUS_DEBUG(LOG, "read " << ret << " bytes");
        xmlParserInputBufferPtr input = xmlParserInputBufferCreateMem(buf, info.size, XML_CHAR_ENCODING_UTF8);
        xmlTextReaderPtr reader = xmlNewTextReader(input,entry.c_str());
        ret = xmlTextReaderRead(reader);
        while (ret == 1) {
        	handler(reader);
            ret = xmlTextReaderRead(reader);
        }
        xmlFreeParserInputBuffer(input);
        xmlFreeTextReader(reader);
        delete [] buf;
        LOG4CPLUS_DEBUG(LOG,"found " << sheetNamesById.size() << " sheets");
    } else {
        throw runtime_error("entry " + entry + " not found");
    }
}


vector<std::string> ExcelToTextFormat::getSheetNames(std::string excelFile) {
    int error;
    this->archive = zip_open(excelFile.c_str(),ZIP_CHECKCONS,&error);
    if (this->archive==0) {
        LOG4CPLUS_ERROR(LOG,"unable to open excel file " << excelFile);
        throw runtime_error("unable to open archive");
    }
    vector<string> sheetNames;
    parseEntry("xl/workbook.xml", [&] (xmlTextReaderPtr reader) {
        int nodeType = xmlTextReaderNodeType(reader);
        xmlChar* nodeName = xmlTextReaderName(reader);
        if (nodeType == XML_READER_TYPE_ELEMENT) {
            string elementName((const char*)nodeName);
            if (elementName == "sheet") {
                xmlTextReaderMoveToAttribute(reader,(xmlChar*)"name");
                string sheetName = (const char*)xmlTextReaderValue(reader);
                LOG4CPLUS_DEBUG(LOG,"sheetName = " << sheetName);
                xmlTextReaderMoveToAttribute(reader,(xmlChar*)"sheetId");
                string sheetId = (const char*)xmlTextReaderValue(reader);
                LOG4CPLUS_DEBUG(LOG,"sheetId = " << sheetId);
                LOG4CPLUS_TRACE(LOG, sheetId << " -> " << sheetName);
                sheetNames.push_back(sheetName);
            }
        }
    });
    zip_close(this->archive);
    return sheetNames;
}

}


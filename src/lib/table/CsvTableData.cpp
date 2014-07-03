#include "CsvTableData.h"

#include <cstring>

#include <log4cplus/logger.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <ios>

#include "utils/md5.h"
#include "type/oids.h"
#include "utils/utility.h"
#include "utils/File.h"
#include "type/TypeRegistry.h"
#include "TableIndex.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
    static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("CsvTableData"));

    CsvTableData::CsvTableData(vector<string> columns) {
        vector<ColDef> colDefs;
    	for (string& column:columns) {
    		vector<string> splitted;
    		split(column,':',splitted);
    		string colName = splitted[0];
    		uint32_t colType = TEXT;
    		if (splitted.size()==2) {
    			string colTypeName = splitted[1];
    			TypeInfo *ti = TypeRegistry::getInstance().getTypeInfo(colTypeName);
    			if (ti!=nullptr) {
    				colType = ti->oid;
    			}
    		}
    		colDefs.push_back(make_pair(colName,colType));
    	}
    	setColumns(colDefs);
    }

    CsvTableData::CsvTableData(vector<pair<string,uint32_t>> columns) {
        setColumns(columns);
    }

    CsvTableData::CsvTableData(string csvFile, vector<pair<string,uint32_t>> columns) {
        setColumns(columns);
        fileName = csvFile;
    }

    CsvTableData::CsvTableData(string csvFile) {
        fileName = csvFile;
    }

    CsvTableData::~CsvTableData() {
        LOG4CPLUS_DEBUG(LOG, "delete table data '" << fileName << "'");
        if (data) {
            free(data);
        }
    }

    string CsvTableData::calculateMD5Sum() {
        loadOnDemand("calculateMD5Sum");
        MD5 digest;
        string cols;
        for (int idx = 0;idx < getColCount(); idx++) {
            pair<string,uint32_t> column = getColumns()[idx];
            TypeInfo *ti = TypeRegistry::getInstance().getTypeInfo(column.second);
            cols += column.first + ":" + ti->name;
            if (idx < getColCount() -1) {
                cols += "\t";
            }
        }
        cols += "\n";
        digest.update(cols.c_str(), cols.size());
        digest.update(data, size);
        digest.finalize();
    	return digest.hexdigest();
    }

    uint64_t CsvTableData::getRowCount() {
        loadOnDemand("getRowCount");
        return TableData::getRowCount();
    }

    uint32_t CsvTableData::getColCount() {
        if (data == nullptr && !fileName.empty() && getColumns().empty()) {
            loadColumns();
        }
        return TableData::getColCount();
    }

    vector<ColDef>& CsvTableData::getColumns() {
        if (data == nullptr && !fileName.empty() && TableData::getColumns().empty()) {
            loadColumns();
        }
        return TableData::getColumns();
    }

    void CsvTableData::loadColumns() {
        LOG4CPLUS_DEBUG(LOG, "read columns from file '" << fileName << "'");
        ifstream is{fileName,ios::in | ios::binary | ios::ate};
        if (is.is_open()) {
            LOG4CPLUS_DEBUG(LOG, "size of file " << is.tellg());
            size = is.tellg();
            is.seekg(0, ios::beg);
            string firstLine;
            getline(is,firstLine,'\n');
            if (!is.good()) {
                LOG4CPLUS_ERROR(LOG, "failed getting first line. probably long line header");
            }
            readColumns(firstLine);
            is.close();
        } else {
            throw runtime_error("unable to open file '" + fileName + "'");
        }
    }

    void CsvTableData::readData() {
        LOG4CPLUS_DEBUG(LOG, "load file " << fileName);
        LOG4CPLUS_DEBUG(LOG, "read data from file '" << fileName << "'");
        ifstream is{fileName,ios::in | ios::binary | ios::ate};
        if (is.is_open()) {
            LOG4CPLUS_DEBUG(LOG, "size of file " << is.tellg());
            size = is.tellg();
            is.seekg(0, ios::beg);
            string firstLine;
            getline(is,firstLine,'\n');
            if (!is.good()) {
                LOG4CPLUS_ERROR(LOG, "failed getting first line. probably long line header");
            }
            readColumns(firstLine);
            size -= is.tellg();
            LOG4CPLUS_DEBUG(LOG, "offset after first line " << is.tellg());
            data = (char*)malloc(size);
            is.read(data, size);
            is.close();
        } else {
            throw runtime_error("unable to open file '" + fileName + "'");
        }
        ptr = 0;
        currentRow = 0;
        currentColumn = 0;
        LOG4CPLUS_TRACE(LOG, "calculate row count");
        File indexFile{fileName + ".idx"};
        if (indexFile.exists()) {
            index.load(indexFile.abspath());
            assert(getColCount() == index.getColCount());
            setRowCount(index.getRowCount());
        } else {
            calculateRowCount();
            LOG4CPLUS_TRACE(LOG, "build index");
            buildIndex();
        }
        LOG4CPLUS_TRACE(LOG, "loading file done");
        LOG4CPLUS_DEBUG(LOG, "load file " << fileName << " done");
    }

    void *CsvTableData::getRaw() {
        loadOnDemand("getRaw");
        return data;
    }

    uint64_t CsvTableData::getSize() {
        loadOnDemand("getSize");
        return size;
    }

    void CsvTableData::setRaw(void *raw, uint64_t size) {
        data = (char*)malloc(size);
        this->size = size;
        memcpy(data, raw, size);
        calculateRowCount();
        buildIndex();
    }


    void CsvTableData::appendRaw(void *raw, uint64_t rsize) {
        if (data==nullptr) {
            setRaw(raw,rsize);
            return;
        }
        data = (char*)realloc(data, size + rsize);
        memcpy(data + size, raw, rsize);
        for (uint64_t idx=0; idx < rsize; idx++) {
            char c = ((char*)data + size -1)[idx];
            if (c == '\t' || c == '\n') {
                index.addOffset(size + idx);
            }
            if (c == '\n') {
                setRowCount(getRowCount() + 1);
                index.setRowCount(getRowCount());
            }
        }
        size += rsize;
    }

    void CsvTableData::addRow(std::vector<std::string> row) {
        assert(row.size() == getColCount());
        string joined = join(row,"\t") + "\n";
        appendRaw((void*)joined.c_str(),joined.size());
    }

    void CsvTableData::readColumns(string firstLine) {
        TableData::getColumns().clear();
        LOG4CPLUS_DEBUG(LOG, "firstLine = " << firstLine);
        vector<string> cols;
        vector<ColDef> colDefs;
        split(firstLine,'\t',cols);
        for (string col:cols) {
            vector<string> nt;
            split(col,':',nt);
            string name = nt[0];
            LOG4CPLUS_TRACE(LOG, "column name = " << name);
            string typeName;
            if (nt.size()>1) {
                typeName = nt[1];
            } else {
                typeName = "TEXT";
            }
            LOG4CPLUS_TRACE(LOG, "column type = " << typeName);
            TypeInfo *ti = TypeRegistry::getInstance().getTypeInfo(typeName);
            uint32_t typeId;
            if (ti==nullptr) {
                LOG4CPLUS_WARN(LOG, "unknown type " + typeName + " for column " + name + ". treat as type TEXT ...");
                typeId = TEXT;
            } else {
            	typeId = ti->oid;
            }
            LOG4CPLUS_DEBUG(LOG, "push column " << name << "[" << typeId << "]");
            //columns.push_back(pair<string,uint32_t>(name,typeId));
            colDefs.push_back(ColDef(name,typeId));
        }
        LOG4CPLUS_DEBUG(LOG, "found " << colDefs.size() << " columns");
        //colCount = columns.size();
        setColumns(colDefs);
    }

    void CsvTableData::calculateRowCount() {
        uint64_t rowCount = 0;
        for (uint32_t ptr = 0; ptr<size;ptr++) {
            if (data[ptr] == '\n') {
                rowCount++;
            }
        }
        setRowCount(rowCount);
    }

    void CsvTableData::save(std::string filePath) {
        LOG4CPLUS_DEBUG(LOG, "save data in file " + filePath);
        ofstream os{filePath};
        for (size_t idx = 0; idx < getColCount(); idx++) {
            pair<string,uint32_t> p = getColumns()[idx];
            TypeInfo *ti =  TypeRegistry::getInstance().getTypeInfo((long int)p.second);
            if (ti == nullptr) {
                LOG4CPLUS_WARN(LOG, "unknown type id '" << p.second << "' for columns '" << p.first << "'. assuming text");
                ti =  TypeRegistry::getInstance().getTypeInfo(TEXT);
            }
            os << p.first << ":" << ti->name;
            if (idx < getColCount() - 1) {
                os << "\t";
            }
        }
        os << endl;
        os.write(data, size);
        os.close();
        index.setRowCount(getRowCount());
        index.setColCount(getColCount());
        index.save(filePath+".idx");
    }

    void CsvTableData::buildIndex() {
        index.clear();
        index.setRowCount(getRowCount());
        index.setColCount(getColCount());
        uint64_t ptr = 0;
        uint64_t lastPtr = 0;
        while (ptr < size) {
            if (data[ptr] == '\t' || data[ptr] == '\n') {
                index.addOffset(lastPtr);
                lastPtr = ptr + 1;
            }
            ptr++;
        }
    }

    void * CsvTableData::getRawRow(uint32_t row, uint32_t& rsize) {
        loadOnDemand("getRawRow");
        uint64_t ptr = index.getOffset(row,0);
        rsize = 0;
        char *rowData = data + ptr;
        while (data[ptr] != '\n' && ptr < size) {
            rsize++;
            ptr++;
        }
        if (data[ptr]=='\n') {
            rsize++;
        }
        return rowData;
    }

    inline void CsvTableData::loadOnDemand(string reason) {
        if (data == nullptr && !fileName.empty()) {
            LOG4CPLUS_DEBUG(LOG, "load on demand -> " << reason);
            readData();
        }
    }

    string CsvTableData::getValue(uint64_t row, uint32_t col) {
        int startIndex = index.getOffset(row,col);
        int len = 0;
        while (data[startIndex+len] != '\n' && data[startIndex+len] != '\t' && startIndex+len < size) {
            len++;
        }
        return string(data+startIndex,len);
    }
}

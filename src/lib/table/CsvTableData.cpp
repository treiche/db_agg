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

    struct CsvTableData::XImpl {
        vector<pair<string,uint32_t>> columns;
        char *data = nullptr;
        uint64_t size = 0;
        uint32_t currentColumn = 0;
        uint64_t currentRow = 0;
        size_t rowCount = 0;
        size_t colCount = 0;
        uint64_t ptr;
        //vector<int> index;
        TableIndex index;
        string fileName;
    };

    CsvTableData::CsvTableData(void *data, uint64_t size) {
        pImpl = new XImpl();
        pImpl->data = (char*)malloc(size);
        memcpy(data,pImpl->data,size);
        pImpl->size = size;
    }

    CsvTableData::CsvTableData(vector<string> columns) {
        pImpl = new XImpl();
        pImpl->colCount = columns.size();
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
    		pImpl->columns.push_back(make_pair(colName,colType));
    	}
    }

    CsvTableData::CsvTableData(vector<pair<string,uint32_t>> columns) {
        pImpl = new XImpl();
        pImpl->columns = columns;
        pImpl->colCount = columns.size();
    }

    CsvTableData::CsvTableData(string csvFile, vector<pair<string,uint32_t>> columns) {
        pImpl = new XImpl();
        pImpl->columns = columns;
        pImpl->colCount = columns.size();
        pImpl->data = nullptr;
        pImpl->fileName = csvFile;
    }

    CsvTableData::CsvTableData(string csvFile) {
        pImpl = new XImpl();
        pImpl->fileName = csvFile;
    }

    CsvTableData::~CsvTableData() {
        if (pImpl->data) {
            free(pImpl->data);
        }
        delete pImpl;
    }

    string CsvTableData::calculateMD5Sum() {
        loadOnDemand("calculateMD5Sum");
        MD5 digest;
        string cols;
        for (int idx = 0;idx < pImpl->columns.size(); idx++) {
            pair<string,uint32_t> column = pImpl->columns[idx];
            TypeInfo *ti = TypeRegistry::getInstance().getTypeInfo(column.second);
            cols += column.first + ":" + ti->name;
            if (idx < pImpl->columns.size() -1) {
                cols += "\t";
            }
        }
        cols += "\n";
        digest.update(cols.c_str(), cols.size());
        digest.update(pImpl->data, pImpl->size);
        digest.finalize();
    	return digest.hexdigest();
    }

    uint64_t CsvTableData::getRowCount() {
        loadOnDemand("getRowCount");
        return pImpl->rowCount;
    }
    uint32_t CsvTableData::getColCount() {
        if (pImpl->data == nullptr && !pImpl->fileName.empty() && pImpl->columns.empty()) {
            loadColumns();
        }
        return pImpl->columns.size();
    }

    vector<pair<string,uint32_t>> CsvTableData::getColumns() {
        if (pImpl->data == nullptr && !pImpl->fileName.empty() && pImpl->columns.empty()) {
            loadColumns();
        }
        return pImpl->columns;
    }

    void CsvTableData::setPointer(uint64_t row, uint32_t col) {

    }

    void CsvTableData::loadColumns() {
        LOG4CPLUS_DEBUG(LOG, "read columns from file '" << pImpl->fileName << "'");
        ifstream is{pImpl->fileName,ios::in | ios::binary | ios::ate};
        if (is.is_open()) {
            LOG4CPLUS_DEBUG(LOG, "size of file " << is.tellg());
            pImpl->size = is.tellg();
            is.seekg(0, ios::beg);
            string firstLine;
            getline(is,firstLine,'\n');
            if (!is.good()) {
                LOG4CPLUS_ERROR(LOG, "failed getting first line. probably long line header");
            }
            readColumns(firstLine);
            is.close();
        } else {
            throw runtime_error("unable to open file '" + pImpl->fileName + "'");
        }
    }

    void CsvTableData::readData() {
        LOG4CPLUS_DEBUG(LOG, "load file " << pImpl->fileName);
        LOG4CPLUS_DEBUG(LOG, "read data from file '" << pImpl->fileName << "'");
        ifstream is{pImpl->fileName,ios::in | ios::binary | ios::ate};
        if (is.is_open()) {
            LOG4CPLUS_DEBUG(LOG, "size of file " << is.tellg());
            pImpl->size = is.tellg();
            is.seekg(0, ios::beg);
            string firstLine;
            getline(is,firstLine,'\n');
            if (!is.good()) {
                LOG4CPLUS_ERROR(LOG, "failed getting first line. probably long line header");
            }
            readColumns(firstLine);
            pImpl->size -= is.tellg();
            LOG4CPLUS_DEBUG(LOG, "offset after first line " << is.tellg());
            pImpl->data = (char*)malloc(pImpl->size);
            is.read(pImpl->data, pImpl->size);
            is.close();
        } else {
            throw runtime_error("unable to open file '" + pImpl->fileName + "'");
        }
        pImpl->ptr = 0;
        pImpl->currentRow = 0;
        pImpl->currentColumn = 0;
        LOG4CPLUS_TRACE(LOG, "calculate row count");
        File indexFile{pImpl->fileName + ".idx"};
        if (indexFile.exists()) {
            pImpl->index.load(indexFile.abspath());
            assert(pImpl->colCount == pImpl->index.getColCount());
            pImpl->rowCount = pImpl->index.getRowCount();
        } else {
            calculateRowCount();
            LOG4CPLUS_TRACE(LOG, "build index");
            buildIndex();
        }
        LOG4CPLUS_TRACE(LOG, "loading file done");
        LOG4CPLUS_DEBUG(LOG, "load file " << pImpl->fileName << " done");
    }

    void *CsvTableData::getRaw() {
        loadOnDemand("getRaw");
        return pImpl->data;
    }

    uint64_t CsvTableData::getSize() {
        loadOnDemand("getSize");
        return pImpl->size;
    }

    void CsvTableData::setRaw(void *data, uint64_t size) {
        pImpl->data = (char*)malloc(size);
        pImpl->size = size;
        memcpy(pImpl->data, data, size);
        calculateRowCount();
        buildIndex();
    }

    void CsvTableData::appendRaw(void *data, uint64_t size) {
        if (pImpl->data==nullptr) {
            setRaw(data,size);
            return;
        }
        pImpl->data = (char*)realloc(pImpl->data, pImpl->size + size);
        memcpy(pImpl->data + pImpl->size, data, size);
        for (uint64_t idx=0; idx < size; idx++) {
            char c = ((char*)data)[idx];
            if (c == '\t' || c == '\n') {
                pImpl->index.addOffset(pImpl->size+idx);
            }
        }
        pImpl->size += size;
        //calculateRowCount();
        //buildIndex();
    }

    void CsvTableData::readColumns(string firstLine) {
        pImpl->columns.clear();
        LOG4CPLUS_DEBUG(LOG, "firstLine = " << firstLine);
        vector<string> cols;
        split(firstLine,'\t',cols);
        for (string col:cols) {
            vector<string> nt;
            split(col,':',nt);
            string name = nt[0];
            LOG4CPLUS_TRACE(LOG, "column name = " << name);
            string typeName = nt[1];
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
            pImpl->columns.push_back(pair<string,uint32_t>(name,typeId));
        }
        LOG4CPLUS_DEBUG(LOG, "found " << pImpl->columns.size() << " columns");
        pImpl->colCount = pImpl->columns.size();
    }

    void CsvTableData::calculateRowCount() {
        pImpl->rowCount = 0;
        for (uint32_t ptr = 0; ptr<pImpl->size;ptr++) {
            if (pImpl->data[ptr] == '\n') {
                pImpl->rowCount++;
            }
        }
    }

    void CsvTableData::save(std::string filePath) {
        LOG4CPLUS_DEBUG(LOG, "save data in file " + filePath);
        ofstream os{filePath};
        for (size_t idx = 0; idx < pImpl->columns.size(); idx++) {
            pair<string,uint32_t> p = pImpl->columns[idx];
            TypeInfo *ti =  TypeRegistry::getInstance().getTypeInfo((long int)p.second);
            if (ti == nullptr) {
                LOG4CPLUS_WARN(LOG, "unknown type id '" << p.second << "' assuming text");
                ti =  TypeRegistry::getInstance().getTypeInfo(TEXT);
            }
            os << p.first << ":" << ti->name;
            if (idx<pImpl->columns.size()-1) {
                os << "\t";
            }
        }
        os << endl;
        os.write(pImpl->data, pImpl->size);
        os.close();
        pImpl->index.save(filePath+".idx");
    }

    void CsvTableData::buildIndex() {
        pImpl->index.clear();
        pImpl->index.setRowCount(pImpl->rowCount);
        pImpl->index.setColCount(pImpl->colCount);
        uint32_t ptr = 0;
        uint32_t lastPtr = 0;
        while (ptr < pImpl->size) {
            if (pImpl->data[ptr] == '\t' || pImpl->data[ptr] == '\n') {
                pImpl->index.addOffset(lastPtr);
                lastPtr = ptr + 1;
            }
            ptr++;
        }
    }

    void * CsvTableData::getRawRow(uint32_t row, uint32_t& size) {
        loadOnDemand("getRawRow");
        uint32_t ptr = pImpl->index.getOffset(row,0);
        size = 0;
        char *rowData = pImpl->data + ptr;
        while (pImpl->data[ptr] != '\n' && ptr < pImpl->size) {
            size++;
            ptr++;
        }
        if (pImpl->data[ptr]=='\n') {
            size++;
        }
        return rowData;
    }

    inline void CsvTableData::loadOnDemand(string reason) {
        if (pImpl->data == nullptr && !pImpl->fileName.empty()) {
            LOG4CPLUS_DEBUG(LOG, "load on demand -> " << reason);
            readData();
        }
    }

    string CsvTableData::getValue(uint64_t row, uint32_t col) {
        int startIndex = pImpl->index.getOffset(row,col);
        int len = 0;
        while (pImpl->data[startIndex+len] != '\n' && pImpl->data[startIndex+len] != '\t' && startIndex+len < pImpl->size) {
            len++;
        }
        return string(pImpl->data+startIndex,len);
    }
}

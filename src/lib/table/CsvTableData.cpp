#include "CsvTableData.h"

#include <cstring>

#include "utils/logging.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <ios>

#include "utils/md5.h"
#include "type/oids.h"
#include "utils/utility.h"
#include "utils/string.h"
#include "utils/File.h"
#include "type/TypeRegistry.h"
#include "TableIndex.h"

using namespace std;
using namespace log4cplus;

namespace db_agg {
    DECLARE_LOGGER("CsvTableData");

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
        readData();
    }

    CsvTableData::~CsvTableData() {
        LOG_INFO("delete table data '" << fileName << "' use_count");
        if (data) {
            free(data);
        }
    }

    string CsvTableData::calculateMD5Sum() const {
        MD5 digest;
        string cols;
        for (uint32_t idx = 0;idx < getColCount(); idx++) {
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

    uint64_t CsvTableData::getRowCount() const {
        return TableData::getRowCount();
    }

    uint32_t CsvTableData::getColCount() const {
        return TableData::getColCount();
    }

    const vector<ColDef>& CsvTableData::getColumns() const {
        return TableData::getColumns();
    }

    void CsvTableData::loadColumns() {
        LOG_DEBUG("read columns from file '" << fileName << "'");
        ifstream is{fileName,ios::in | ios::binary | ios::ate};
        if (is.is_open()) {
            LOG_DEBUG("size of file " << is.tellg());
            size = is.tellg();
            is.seekg(0, ios::beg);
            string firstLine;
            getline(is,firstLine,'\n');
            if (!is.good()) {
                THROW_EXC("failed getting first line. probably long line header");
            }
            readColumns(firstLine);
            is.close();
        } else {
            THROW_EXC("unable to open file '" << fileName << "'");
        }
    }

    void CsvTableData::readData() {
        cout << "load data " << fileName << endl;
        LOG_DEBUG("load file " << fileName);
        LOG_DEBUG("read data from file '" << fileName << "'");
        ifstream is{fileName,ios::in | ios::binary | ios::ate};
        if (is.is_open()) {
            LOG_DEBUG("size of file " << is.tellg());
            size = is.tellg();
            is.seekg(0, ios::beg);
            string firstLine;
            getline(is,firstLine,'\n');
            if (!is.good()) {
                THROW_EXC("failed getting first line. probably long line header");
            }
            readColumns(firstLine);
            size -= is.tellg();
            LOG_DEBUG("offset after first line " << is.tellg());
            data = (char*)malloc(size);
            is.read(data, size);
            is.close();
        } else {
            throw runtime_error("unable to open file '" + fileName + "'");
        }
        ptr = 0;
        currentRow = 0;
        currentColumn = 0;
        LOG_TRACE("calculate row count");
        File indexFile{fileName + ".idx"};
        if (indexFile.exists()) {
            index.load(indexFile.abspath());
            assert(getColCount() == index.getColCount());
            setRowCount(index.getRowCount());
        } else {
            calculateRowCount();
            LOG_TRACE("build index");
            buildIndex();
        }
        LOG_TRACE("loading file done");
        LOG_DEBUG("load file " << fileName << " done");
    }

    void CsvTableData::appendRaw(void *raw, uint64_t rsize) {
        if (data==nullptr) {
            data = (char*)malloc(rsize);
            this->size = rsize;
            memcpy(data, raw, rsize);
            calculateRowCount();
            buildIndex();
            return;
        } else {
            data = (char*)realloc(data, size + rsize);
        }
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
        if (TableData::getColumns().size() > 0) {
            LOG_WARN("read columns called again");
            return;
        }
        LOG_DEBUG("firstLine = " << firstLine);
        vector<string> cols;
        vector<ColDef> colDefs;
        split(firstLine,'\t',cols);
        for (string col:cols) {
            if (col.empty()) {
                THROW_EXC("empty column definition");
            }
            vector<string> nt;
            split(col,':',nt);
            string name = nt[0];
            LOG_TRACE("column name = " << name);
            string typeName;
            if (nt.size()>1) {
                typeName = nt[1];
            } else {
                typeName = "TEXT";
            }
            LOG_TRACE("column type = " << typeName);
            TypeInfo *ti = TypeRegistry::getInstance().getTypeInfo(typeName);
            uint32_t typeId;
            if (ti==nullptr) {
                LOG_WARN("unknown type " + typeName + " for column " + name + ". treat as type TEXT ...");
                typeId = TEXT;
            } else {
                typeId = ti->oid;
            }
            LOG_DEBUG("push column " << name << "[" << typeId << "]");
            colDefs.push_back(ColDef(name,typeId));
        }
        LOG_DEBUG("found " << colDefs.size() << " columns");
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
        LOG_DEBUG("save data in file " + filePath);
        ofstream os{filePath};
        for (size_t idx = 0; idx < getColCount(); idx++) {
            pair<string,uint32_t> p = getColumns()[idx];
            TypeInfo *ti =  TypeRegistry::getInstance().getTypeInfo((long int)p.second);
            if (ti == nullptr) {
                LOG_WARN("unknown type id '" << p.second << "' for columns '" << p.first << "'. assuming text");
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

    void CsvTableData::getRows(uint64_t startRow, uint64_t rows, std::vector<DataChunk>& chunks) const {
        assert(startRow + rows <= getRowCount());
        // loadOnDemand("getRows");
        uint64_t startOffset = index.getOffset(startRow,0);
        uint64_t endOffset = 0;
        if (startRow + rows == getRowCount()) {
            endOffset = size;
        } else {
            endOffset = index.getOffset(startRow + rows,0);
        }
        chunks.push_back(DataChunk(data + startOffset, endOffset - startOffset));
    }

    DataChunk CsvTableData::getColumn(uint64_t row, uint32_t col) const {
        uint64_t startIndex = index.getOffset(row,col);
        int len = 0;
        while (data[startIndex+len] != '\n' && data[startIndex+len] != '\t' && startIndex+len < size) {
            len++;
        }
        return DataChunk(data+startIndex,len);
    }

    string CsvTableData::encode(string value) {
        string result;
        for (size_t idx=0; idx < value.size(); idx++) {
            switch (value[idx]) {
                case '\\':
                    result += "\\\\";
                    break;
                case '\b':
                    result += "\\b";
                    break;
                case '\f':
                    result += "\\f";
                    break;
                case '\n':
                    result += "\\n";
                    break;
                case '\r':
                    result += "\\r";
                    break;
                case '\t':
                    result += "\\t";
                    break;
                case '\v':
                    result += "\\v";
                    break;
                default:
                    result += value[idx];
            }
        }
        return result;
    }

    string CsvTableData::decode(string value) {
        string result;
        for (size_t idx=0; idx < value.size(); idx++) {
            if (value[idx] == '\\') {
                switch (value[idx+1]) {
                    case '\\':
                        result += '\\';
                        break;
                    case 'b':
                        result += '\b';
                        break;
                    case 'f':
                        result += '\f';
                        break;
                    case 'n':
                        result += '\n';
                        break;
                    case 'r':
                        result += '\r';
                        break;
                    case 't':
                        result += '\t';
                        break;
                    case 'v':
                        result += '\v';
                        break;
                }
                idx += 1;
            } else {
                result += value[idx];
            }
        }
        return result;
    }
}

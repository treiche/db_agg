/*
 * string.cpp
 *
 *  Created on: Aug 23, 2014
 *      Author: arnd
 */

#include "string.h"
#include <iostream>
#include <cstdarg>
#include <sstream>

using namespace std;

namespace db_agg {

string string_format(const std::string &fmt, ...) {
       int size=100;
       std::string str;
       va_list ap;
       while (1) {
       str.resize(size);
       va_start(ap, fmt);
       int n = vsnprintf((char *)str.c_str(), size, fmt.c_str(), ap);
       va_end(ap);
       if (n > -1 && n < size)
           return str;
       if (n > -1)
           size=n+1;
       else
           size*=2;
       }
}

string join(vector<string> v, string delim) {
    string res;
    uint32_t size = v.size();
    for (size_t cnt=0;cnt<size;cnt++) {
        res += v[cnt];
        if (cnt<size-1) {
            res += delim;
        }
    }
    return res;
}

vector<string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

string trim(string& str) {
    str.erase(0, str.find_first_not_of(" \n\r\t"));       //prefixing spaces
    str.erase(str.find_last_not_of(" \n\r\t") + 1);         //surfixing spaces
    return str;
}

string replace_all(string str, string search, string replacement) {
    string replaced;
    size_t offset = 0;
    size_t lastOffset = 0;
    do {
        size_t idx = str.find(search,offset);
        if (idx == string::npos) {
            break;
        }
        if (idx >= 0) {
            replaced += str.substr(lastOffset, idx - lastOffset);
            replaced += replacement;
        }
        lastOffset += replacement.size();
        offset = idx + search.size();
    } while(offset <= str.size());
    if (offset <= str.size()) {
        replaced += str.substr(offset);
    }
    return replaced;
}

string thousand_grouping(uint64_t number) {
    uint64_t hundred = number % 1000;
    //cout << "hundred = " << hundred << endl;
    uint64_t thousand = number % 1000000 / 1000;
    //cout << "thousand = " << thousand << endl;
    uint64_t million = number % 1000000000 / 1000000;
    //cout << "million = " << million << endl;
    string grouped;
    if (million > 0) {
        grouped += to_string(million) + ".";
    }
    if (thousand > 0 || million > 0) {
        if (million > 0) {
            if (thousand < 100) {
                grouped += "0";
            }
            if (thousand < 10) {
                grouped += "0";
            }
        }
        grouped += to_string(thousand) + ".";
    }
    if (thousand > 0 || million > 0) {
        if (hundred < 100) {
            grouped += "0";
        }
        if (hundred < 10) {
            grouped += "0";
        }
        if (hundred == 10) {
            grouped += "0";
        }
    }
    grouped += to_string(hundred);
    return grouped;
}

bool isEmptyLine(string line) {
    for (size_t idx=0; idx < line.length();idx++) {
        if (!isspace(line[idx])) {
            return false;
        }
    }
    return true;
}

int getRightExtent(string line) {
    for (int idx=line.length()-1;idx>=0;idx--) {
        char c = line[idx];
        if (!isspace(c)) {
            return idx + 1;
        }
    }
    return 0;
}

int getLeftMargin(string line) {
    for (size_t idx=0; idx < line.length();idx++) {
        char c = line[idx];
        if (!isspace(c)) {
            return idx;
        }
    }
    return 0;
}

string cutBlock(string text) {
    vector<string> lines;
    split(text,'\n',lines);
    int rightExtent = 0;
    for (auto line:lines) {
        int re = getRightExtent(line);
        if (re > rightExtent) {
            rightExtent = re;
        }
    }
    int leftMargin = rightExtent;
    for (size_t lineNo = 0; lineNo < lines.size(); lineNo++) {
        string line = lines[lineNo];
        if (isEmptyLine(line)) {
            continue;
        }
        int lm = getLeftMargin(line);
        if (lm < leftMargin) {
            leftMargin = lm;
        }
    }
    vector<string> cutted;
    for (size_t lineNo = 0; lineNo < lines.size(); lineNo++) {
        string line = lines[lineNo];
        line.erase(line.find_last_not_of(" \n\r\t") + 1);
        if (isEmptyLine(line)) {
            continue;
        }
        cutted.push_back(line.substr(leftMargin));
    }
    return join(cutted,"\n");
}


}


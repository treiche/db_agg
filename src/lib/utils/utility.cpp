/*
 * utility
 *
 *  Created on: Nov 24, 2013
 *      Author: arnd
 */

#include "utility.h"

#include <cstdio>
#include <cstdarg>
#include <dirent.h>
#include <pwd.h>
#include <stddef.h>
#include <unistd.h>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>



using namespace std;

namespace db_agg {

string string_format(const std::string &fmt, ...) {
       int n, size=100;
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

string maskPassword(std::string url) {
    size_t pos = url.find("password=");
    string masked;
    bool insidePassword = false;
    for (size_t idx=0; idx < url.size(); idx++) {
        char c = url[idx];
        if (idx>pos+8) {
            insidePassword = true;
        }
        if (c==' ') {
            insidePassword = false;
        }
        if (insidePassword) {
            masked += 'x';
        } else {
            masked += c;
        }
    }
    return masked;
}

vector<string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

string readFile(string fileName) {
    ifstream is{fileName,ios::in | ios::binary | ios::ate};
    string content;
    if (is.is_open()) {
        size_t size = is.tellg();
        char *data = new char[size];
        is.seekg(0, ios::beg);
        is.read(data, size);
        is.close();
        content = string(data,size);
        delete[] data;
    } else {
        throw runtime_error("unable to open file '" + fileName + "'");
    }
    return content;
}

bool fileExists(string fileName) {
    ifstream fp(fileName);
    return fp;
}

bool dirExists(string dirName) {
        DIR *pDir;
        bool bExists = false;

        pDir = opendir (dirName.c_str());

        if (pDir != NULL)
        {
            bExists = true;
            (void) closedir (pDir);
        }

        return bExists;
}

string getHomeDir() {
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    return string(homedir);
}

string trim(string& str) {
    str.erase(0, str.find_first_not_of(" \n\r\t"));       //prefixing spaces
    str.erase(str.find_last_not_of(" \n\r\t") + 1);         //surfixing spaces
    return str;
}

bool isEmptyLine(string line) {
    for (int idx=0; idx < line.length();idx++) {
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
    for (int idx=0; idx < line.length();idx++) {
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
    for (int lineNo = 0; lineNo < lines.size(); lineNo++) {
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
    for (int lineNo = 0; lineNo < lines.size(); lineNo++) {
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

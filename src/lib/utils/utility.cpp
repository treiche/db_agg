/*
 * utility
 *
 *  Created on: Nov 24, 2013
 *      Author: arnd
 */

#include "utility.h"

#include <cstdio>
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

string maskPassword(std::string url) {
    size_t pos = url.find("password=");
    string masked;
    bool insidePassword = false;
    for (size_t idx=0; idx < url.size(); idx++) {
        char c = url[idx];
        if (idx == pos+9) {
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




}

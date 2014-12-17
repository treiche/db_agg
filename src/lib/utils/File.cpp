/*
 * File.cpp
 *
 *  Created on: Dec 11, 2013
 *      Author: arnd
 */

#include "File.h"

#include <dirent.h>
#include "utils/logging.h"
#include "utils/string.h"
#include <log4cplus/tstring.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdexcept>

#include "utils/utility.h"

using namespace std;
using namespace log4cplus;


namespace db_agg {

DECLARE_LOGGER("File");

File::File(string path) : path(path) {
}

File::FileType File::getType() {
    struct stat info;
    int ret = stat(path.c_str(), &info);
    if (ret==-1) {
        ret = lstat(path.c_str(), &info);
        if (ret == -1) {
            return FileType::UNDEFINED;
        }
    }
    if (S_ISDIR(info.st_mode)) {
        return FileType::DIRECTORY;
    } else if (S_ISREG(info.st_mode)) {
        return FileType::FILE;
    } else if (S_ISLNK(info.st_mode)) {
        return FileType::LINK;
    }
    throw runtime_error("unhandled file type " + info.st_mode);
}

bool File::exists() {
    struct stat info;
    int ret = stat(path.c_str(), &info);
    if (ret == 0) {
        return true;
    }
    return false;
}

bool File::mkdir() {
    int ret = ::mkdir(path.c_str(), 0777);
    if (ret==-1) {
        return false;
    }
    return true;
}

bool File::mkdirs() {
    vector<string> spl;
    string abs = abspath();
    split(abs.c_str(),'/',spl);
    string tmppath="";
    bool success = true;
    for (auto& item:spl) {
        tmppath += "/" + item;
        File tf(tmppath);
        if (!tf.exists()) {
            success &= tf.mkdir();
        }
    }
    return success;
}

string File::getParent() {
    string abs = abspath();
    vector<string> spl;
    split(abs.c_str(),'/',spl);
    spl.pop_back();
    return join(spl,"/");
}

string File::abspath() {
    if (path[0]=='/') {
        return path;
    }
    char buffer[512];
    string pwd = getcwd(buffer,512);
    if (path[0] != '.') {
        return pwd + '/' + path;
    } else if (path[1] == '/') {
        return pwd + path.substr(1);
    }
    return pwd + path;
}

string File::realpath() {
    char *ap = ::realpath(path.c_str(),nullptr);
    if (ap == nullptr) {
        THROW_EXC("unable to get realpath for '" << path << "'");
    }
    string res(ap);
    free(ap);
    return res;
}

bool File::rmdir() {
    vector<string> childs;
    getChilds(childs);
    for (auto& child:childs) {
        File cf(path + "/" + child);
        if (cf.getType() == FileType::DIRECTORY) {
            cf.rmdir();
        } else {
            cf.remove();
        }
    }
    int status = ::rmdir(path.c_str());
    if (status == 0) {
        return true;
    }
    return false;
}

bool File::remove() {
    FileType type = getType();
    if (type == FileType::DIRECTORY) {
        throw runtime_error("can't remove file '" + path + "' [type = " + (char)type + "]");
    }
    int status = ::remove(path.c_str());
    if (status == 0) {
        return false;
    }
    return true;
}

void File::getChilds(vector<string>& childs) {
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(path.c_str())) == NULL) {
        throw runtime_error("can't open directory '" + path + "'");
    }
    while ((dirp = readdir(dp)) != NULL) {
        if (dirp->d_name[0] != '.') {
            childs.push_back(string(dirp->d_name));
        }
    }
    closedir(dp);
}

void File::linkTo(File source) {
    unlink(abspath().c_str());
    if (symlink(source.abspath().c_str(),abspath().c_str()) == -1) {
        string message = "symlink failed: " + abspath() + " -> " + source.abspath();
        throw runtime_error(message);
    }
}

string File::getName() {
    int lastSlash = path.rfind("/");
    return path.substr(lastSlash+1);
}

}

/*
 * File.h
 *
 *  Created on: Dec 11, 2013
 *      Author: arnd
 */

#ifndef FILE_H_
#define FILE_H_

#include <string>
#include <vector>

namespace db_agg {

class File {
private:
    std::string path;
public:
    enum class FileType : char {
        DIRECTORY = 'D',
        FILE = 'F',
        LINK = 'L',
        UNDEFINED = 'U'
    };
    File(std::string path);
    std::string getName();
    FileType getType();
    bool exists();
    bool mkdir();
    bool mkdirs();
    std::string abspath();
    std::string realpath();
    std::string getParent();
    bool rmdir();
    bool remove();
    void getChilds(std::vector<std::string>& childs);
    void linkTo(File source);
};

}



#endif /* FILE_H_ */

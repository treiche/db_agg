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
#include <functional>

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
    std::string getName() const;
    std::string getPath() const;
    FileType getType() const;
    bool exists() const;
    bool mkdir() const;
    bool mkdirs() const;
    std::string abspath() const;
    std::string realpath() const;
    std::string getParent() const;
    std::string getExtension() const;
    bool rmdir() const;
    bool remove() const;
    void getChilds(std::vector<std::string>& childs) const;
    void getChildFiles(std::vector<File>& childs) const;
    void linkTo(File source) const;
};

std::ostream& operator<<(std::ostream& out, const File& file);


class FileWalker {
public:
    void walk(const File& file, std::function<void (const File&)> visit);
};

}



#endif /* FILE_H_ */

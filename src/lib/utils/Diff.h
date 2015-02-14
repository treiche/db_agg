/*
 * Diff.h
 *
 *  Created on: Feb 8, 2015
 *      Author: arnd
 */

#ifndef DIFF_H_
#define DIFF_H_

#include "File.h"


namespace db_agg {
class Diff {
private:
    const File file1;
    const File file2;
public:
    Diff(std::string path1, std::string path2);
    bool compare(std::string excludes = "");
};
}



#endif /* DIFF_H_ */

#ifndef UTILITY_H_
#define UTILITY_H_

#include <string>
#include <vector>

namespace db_agg {
    std::string string_format(const std::string &fmt, ...);
    std::string readFile(std::string fileName);
    std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
    std::string join(std::vector<std::string> v, std::string delim);
    bool fileExists(std::string fileName);
    bool dirExists(std::string dirName);
    std::string getHomeDir();
    std::string cutBlock(std::string text);
    std::string maskPassword(std::string url);
    std::string trim(std::string& str);
}
#endif /* UTILITY_H_ */

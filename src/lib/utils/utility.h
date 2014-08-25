#ifndef UTILITY_H_
#define UTILITY_H_

#include <string>
#include <vector>

namespace db_agg {
    std::string readFile(std::string fileName);
    bool fileExists(std::string fileName);
    bool dirExists(std::string dirName);
    std::string getHomeDir();
    std::string maskPassword(std::string url);
}
#endif /* UTILITY_H_ */

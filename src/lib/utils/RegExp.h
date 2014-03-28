#ifndef REGEXP_H_
#define REGEXP_H_

#include <string>
#include <vector>

namespace db_agg {

class Matcher {
public:
    int start(int group);
    int end(int group);
    std::string group(int group);
};

class RegExp {
    struct XImpl;
    XImpl *pImpl;
    void compile(std::string re);
public:
    struct match {
        std::string substr;
        int start;
        int end;
    };
    RegExp();
    RegExp(std::string re);
    ~RegExp();
    void setExpr(std::string re);
    std::vector<match> exec(std::string str);

    bool find(std::string str, std::vector<std::string>& matches, int& offset);
};
}

#endif /* REGEXP_H_ */

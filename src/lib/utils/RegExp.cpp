#include "RegExp.h"

#include "utils/logging.h"
#include "utils/string.h"
#include <iostream>

extern "C" {
#include <pcre.h>
}

using namespace std;
using namespace log4cplus;

namespace db_agg {

DECLARE_LOGGER("RegExp");

struct RegExp::XImpl {
    string exp;
    pcre *regexp;
    pcre_extra *pcreExtra;
};

RegExp::RegExp() {
    pImpl = new XImpl();
}

RegExp::RegExp(string re) {
    pImpl = new XImpl();
    pImpl->exp = re;
    compile(re);
    //LOG_DEBUG("re = " << re);
    //LOG_DEBUG("RegExp extra = " << pImpl->pcreExtra);
}

void RegExp::setExpr(std::string re) {
    pImpl->exp = re;
    compile(re);
}

string RegExp::getExpr() {
    return pImpl->exp;
}

void RegExp::compile(string re) {
    const char *errMsg;
    int errOffset;
    pImpl->regexp = pcre_compile(re.c_str(),PCRE_CASELESS|PCRE_DOTALL,&errMsg,&errOffset,NULL);
    if (pImpl->regexp == NULL) {
        string msg("syntax error at ");
        msg += to_string(errOffset);
        msg += ": ";
        msg += errMsg;
        throw runtime_error(msg);
    }
    pImpl->pcreExtra = pcre_study(pImpl->regexp, 0, &errMsg);
    if (errMsg != NULL) {
        throw "error when study";
    }
}

RegExp::~RegExp() {
    pcre_free(pImpl->regexp);
    pcre_free(pImpl->pcreExtra);
    delete pImpl;
}

vector<RegExp::match> RegExp::exec(string str) {
    vector<RegExp::match> result;
    const char *qs = str.c_str();
    int len = str.length();
    int subStrVec[30];
    int ret = pcre_exec(pImpl->regexp, pImpl->pcreExtra, qs, len, 0, 0, subStrVec, 30);
    if (ret < 0) {
        LOG_TRACE("no more matches found");
        return result;
    }
    for (int cnt = 0; cnt < ret; cnt++) {
        match m;
        m.start = subStrVec[cnt * 2];
        m.end = subStrVec[cnt * 2 + 1];
        if (m.start != m.end) {
            m.substr = string(qs, subStrVec[cnt * 2], subStrVec[(cnt * 2) + 1] - subStrVec[cnt * 2]);
        }
        result.push_back(m);
    }
    return result;
}

bool RegExp::find(string str, vector<string>& matches, int& offset) {
    LOG_TRACE("find " << str)
    const char *qs = str.c_str();
    int len = str.length();
    int subStrVec[30];
    int ret = pcre_exec(pImpl->regexp, pImpl->pcreExtra, qs + offset, len - offset, 0, 0, subStrVec, 30);
    if (ret < 0) {
        return false;
    }
    LOG_TRACE("found " << ret << " matches");
    matches.clear();
    for (int cnt = 0; cnt < ret; cnt++) {
        LOG_TRACE("got match[" << cnt << "]" << subStrVec[cnt * 2] << " to " << subStrVec[(cnt * 2) + 1] << " len=" << len << " offset " << offset);
        if (subStrVec[cnt * 2] == subStrVec[(cnt * 2) + 1]) {
            matches.push_back("");
            if (cnt==ret-1) {
                offset += subStrVec[cnt * 2 + 1];
            }
            continue;
        }
        string substr = string(qs + offset, subStrVec[cnt * 2], subStrVec[(cnt * 2) + 1] - subStrVec[cnt * 2]);
        LOG_TRACE("substr=" << substr);
        matches.push_back(substr);
        if (cnt==ret-1) {
            offset += subStrVec[cnt * 2 + 1];
        }
    }
    return true;
}

vector<string> RegExp::split(string str) {
    const char *qs = str.c_str();
    int len = str.length();
    int subStrVec[30];
    int ret = 0;
    int offset = 0;
    int lastOffset = 0;
    vector<string> splitted;
    do {
        ret = pcre_exec(pImpl->regexp, pImpl->pcreExtra, qs, len, offset, 0, subStrVec, 30);
        if (ret > 0) {
            if (subStrVec[0]==subStrVec[1]) {
                offset++;
                continue;
            }
            offset = subStrVec[1];
            splitted.push_back(string(qs + lastOffset, subStrVec[0] - lastOffset));
            lastOffset = offset;
        }
    } while(ret > 0);
    if (offset < len) {
        splitted.push_back(string(qs + offset, len - offset));
    }
    return splitted;
}

string RegExp::replace(std::string value, std::string replace, bool& matched) {
    const char *qs = value.c_str();
    size_t len = value.size();
    size_t offset = 0;
    int groups[30];
    int ret;
    string result;
    matched = false;
    do {
        ret = pcre_exec(pImpl->regexp, pImpl->pcreExtra, qs, len, offset, 0, groups, 30);
        if (ret > 0) {
            matched = true;
            vector<string> matches;
            string rpl = replace;
            for (int idx = 0; idx < ret; idx++) {
                size_t start = groups[idx*2];
                size_t end = groups[(idx*2)+1];
                size_t len = end - start;
                string match;
                if (len > 0) {
                    match = string(qs,start,len);
                }
                matches.push_back(match);
                // find group in replacement
                string g = "\\" + to_string(idx);
                rpl = replace_all(rpl,g,match);
            }
            result += value.substr(offset, groups[0] - offset);
            result += rpl;
            offset = groups[1];
        }
    } while(ret > 0);
    if (offset < value.size()) {
        result += value.substr(offset);
    }
    //cout << "ret = " << ret << endl;
    return result;
}

bool RegExp::matches(std::string value) {
    const char *qs = value.c_str();
    size_t len = value.size();
    int groups[30];
    int ret = pcre_exec(pImpl->regexp, pImpl->pcreExtra, qs, len, 0, 0, groups, 30);
    if (ret > 0) {
        return true;
    }
    return false;
}
}

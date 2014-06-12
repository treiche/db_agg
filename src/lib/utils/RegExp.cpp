#include "RegExp.h"

#include <log4cplus/logger.h>
#include <iostream>

extern "C" {
#include <pcre.h>
}

using namespace std;
using namespace log4cplus;

namespace db_agg {
    static Logger LOG = Logger::getInstance(LOG4CPLUS_TEXT("RegExp"));

    struct RegExp::XImpl {
        pcre *regexp;
        pcre_extra *pcreExtra;
    };

    RegExp::RegExp() {
        pImpl = new XImpl();
    }

    RegExp::RegExp(string re) {
        pImpl = new XImpl();
        compile(re);
        //LOG4CPLUS_DEBUG(LOG, "re = " << re);
        //LOG4CPLUS_DEBUG(LOG, "RegExp extra = " << pImpl->pcreExtra);
    }

    void RegExp::setExpr(std::string re) {
        compile(re);
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
            LOG4CPLUS_TRACE(LOG, "no more matches found");
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
        const char *qs = str.c_str();
        int len = str.length();
        int subStrVec[30];
        int ret = pcre_exec(pImpl->regexp, pImpl->pcreExtra, qs + offset, len - offset, 0, 0, subStrVec, 30);
        if (ret < 0) {
            return false;
        }
        matches.clear();
        for (int cnt = 0; cnt < ret; cnt++) {
            string substr = string(qs + offset, subStrVec[cnt * 2], subStrVec[(cnt * 2) + 1] - subStrVec[cnt * 2]);
            matches.push_back(substr);
            if (cnt==ret-1) {
                offset += subStrVec[cnt * 2 + 1];
            }
        }
        return true;
    }
}

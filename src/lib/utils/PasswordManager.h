#ifndef PASSWORDMANAGER_H_
#define PASSWORDMANAGER_H_

#include <string>
#include <utility>

#include "core/Url.h"

namespace db_agg {
class PasswordManager {
private:
    bool searchInPgPass;
public:
    PasswordManager(bool searchInPgPass);
    std::pair<std::string,std::string> getCredential(Url *url);
    std::pair<std::string,std::string> getCredentialFromPrompt(Url *url);
    std::pair<std::string,std::string> getCredentialFromPgPass(Url *url);
};
}

#endif /* PASSWORDMANAGER_H_ */

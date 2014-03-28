#ifndef PASSWORDMANAGER_H_
#define PASSWORDMANAGER_H_

#include <string>
#include <utility>

#include "core/Connection.h"

namespace db_agg {
class PasswordManager {
private:
    bool searchInPgPass;
public:
    PasswordManager(bool searchInPgPass);
    std::pair<std::string,std::string> getCredential(Connection connection);
    std::pair<std::string,std::string> getCredentialFromPrompt(Connection connection);
    std::pair<std::string,std::string> getCredentialFromPgPass(Connection connection);
};
}

#endif /* PASSWORDMANAGER_H_ */

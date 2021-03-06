#include <iostream>
#include <fstream>
#include <vector>

extern "C" {
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <cstdlib>
#include <pwd.h>
}

#include "utils/logging.h"

#include "utils/utility.h"
#include "utils/string.h"
#include "utils/PasswordManager.h"

using namespace std;
using namespace log4cplus;


// taken from http://www.cplusplus.com/articles/E6vU7k9E/#UNIX-e1

int getch() {
    int ch;
    struct termios t_old, t_new;

    tcgetattr(STDIN_FILENO, &t_old);
    t_new = t_old;
    t_new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t_new);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &t_old);
    return ch;
}

string getinput(const char *prompt) {
    //const char BACKSPACE=127;
    const char RETURN=10;
    cout << prompt;
    string in;
    char ch;
    while((ch=getch())!=RETURN) {
        in += ch;
        cout << ch;
    }
    cout << endl;
    return in;
}

string getpass(const char *prompt, bool show_asterisk=true)
{
  const char BACKSPACE=127;
  const char RETURN=10;

  string password;
  unsigned char ch=0;

  cout <<prompt<<endl;

  while((ch=getch())!=RETURN)
    {
       if(ch==BACKSPACE)
         {
            if(password.length()!=0)
              {
                 if(show_asterisk)
                 cout <<"\b \b";
                 password.resize(password.length()-1);
              }
         }
       else
         {
             password+=ch;
             if(show_asterisk)
                 cout <<'*';
         }
    }
  cout <<endl;
  return password;
}


namespace db_agg {

DECLARE_LOGGER("PasswordManager");

PasswordManager::PasswordManager(bool searchInPgPass) {
    this->searchInPgPass = searchInPgPass;
}

pair<string,string> PasswordManager::getCredential(Url *url) {
    pair<string,string> c;
    if (searchInPgPass) {
        c = getCredentialFromPgPass(url);
        if (!c.first.empty()) {
            return c;
        }
    }
    return getCredentialFromPrompt(url);
}

pair<string,string> PasswordManager::getCredentialFromPgPass(Url *url) {
    pair<string,string> c;
    uid_t uid = getuid();
    LOG_DEBUG("uid=" << uid);
    struct passwd *pw = getpwuid(uid);
    const char *homedir = pw->pw_dir;
    LOG_DEBUG("homedir=" << homedir);
    homedir = getenv("HOME");
    LOG_DEBUG("homedir=" << homedir);

    ifstream pgpass(string(homedir) + "/.pgpass");

    LOG_DEBUG("pgpass = " << pgpass);

    if (pgpass) {
        while(pgpass) {
            string line;
            getline(pgpass,line,'\n');
            //cout << line << endl;
            vector<string> columns;
            split(line,':',columns);
            if (columns.size() == 5) {
                if (columns[0] == url->getHost() && columns[1] == url->getPort()) {
                    LOG_DEBUG("found entry for host " << columns[0]);
                    c.first = columns[3];
                    c.second = columns[4];
                    LOG_DEBUG("return " << columns[3]);
                    pgpass.close();
                    return c;
                }
            } else {
                LOG_WARN("found line with " << columns.size() << " columns");
            }
        }
        pgpass.close();
        LOG_DEBUG("no credentials found");
    }
    return c;
}

pair<string,string> PasswordManager::getCredentialFromPrompt(Url *url) {
    pair<string,string> c;
    cout << "connecting to " << url->getUrl(false,false,false) << endl;
    string user = getinput("user>");
    string pass=getpass("password>", true);
    c.first = user;
    c.second = pass;
    return c;
}

}

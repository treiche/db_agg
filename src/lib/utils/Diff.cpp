/*
 * Diff.cpp
 *
 *  Created on: Feb 8, 2015
 *      Author: arnd
 */

#include "Diff.h"
#include <iostream>
#include <set>
#include <algorithm>
#include "utils/RegExp.h"

using namespace std;

namespace db_agg {

Diff::Diff(string path1, string path2): file1(File(path1)), file2(File(path2)) {}

bool Diff::compare(string exclude) {
    RegExp exc;
    if (!exclude.empty()) {
        exc.setExpr(exclude);
    }
    bool same = true;
    FileWalker fw;
    vector<string> files1;
    fw.walk(file1, [&] (const File& f) {
        if (f.getType() == File::FileType::DIRECTORY) {
            return;
        }
        if (!exclude.empty() && exc.matches(f.getName())) {
            return;
        }
        string relPart = f.getPath().substr(file1.getPath().size());
        files1.push_back(relPart);
    });
    sort(files1.begin(),files1.end());
    vector<string> files2;
    fw.walk(file2, [&] (const File& f) {
        if (f.getType() == File::FileType::DIRECTORY) {
            return;
        }
        if (!exclude.empty() && exc.matches(f.getName())) {
            return;
        }
        string relPart = f.getPath().substr(file2.getPath().size());
        files2.push_back(relPart);
    });
    sort(files2.begin(),files2.end());
    vector<string> intersected;
    set_intersection(files1.begin(),files1.end(),files2.begin(), files2.end(),back_inserter(intersected));
    for (auto f:intersected) {
        string f1 = file1.getPath() + f;
        string f2 = file2.getPath() + f;
        string cmd = "cmp -s '" + f1 + "' '" + f2 + "'";
        int exitCode = system(cmd.c_str());
        if (exitCode != 0) {
            cout << "meld " << f1 << " " << f2 << endl;
            same =false;
        }
    }
    vector<string> only1;
    set_difference(files1.begin(),files1.end(),files2.begin(), files2.end(),back_inserter(only1));
    for (auto f:only1) {
        cout << "only in " << file1.getPath() << ": " << f << endl;
        same = false;
    }
    vector<string> only2;
    set_difference(files2.begin(),files2.end(),files1.begin(), files1.end(),back_inserter(only2));
    for (auto f:only2) {
        cout << "only in " << file2.getPath() << ": " << f << endl;
        same = false;
    }

    return same;
}

}

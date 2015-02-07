/*
 * ApplicationTest.cpp
 *
 *  Created on: Dec 15, 2014
 *      Author: arnd
 */

#include <iostream>
#include <thread>

#include "db_agg.h"
#include "gmock/gmock.h"
#include "../bin/testrunner.h"

#include "sharding/MD5ShardingStrategy.h"
#include "sharding/ModuloShardingStrategy.h"

using namespace std;
using namespace db_agg;
using namespace testing;

static bool system_diff(std::string path1, std::string path2) {
    string cmd = "diff  -r " + path1 + " " + path2;
    int exitCode = system(cmd.c_str());

    if (exitCode == 0) {
        return true;
    }
    cout << "diff exit code = " << exitCode << endl;
    return false;
}


class ApplicationTest: public ::testing::Test, public ::testing::WithParamInterface<string> {
protected:
    Application& app = *new Application();
    Configuration config;

    virtual void SetUp() {
        File testlink("/var/tmp/db_agg");
        if (!testlink.exists()) {
            File wd(".");
            testlink.linkTo(wd);
        }
        File rd("build/testresults/" + GetParam());
        if (rd.exists()) {
            if (!rd.rmdir()) {
                FAIL() << "unable to remove dir '" << rd.abspath() << "'" << endl;
            }
        }
        File cd("build/testresults/cache");
        if (cd.exists()) {
            if (!cd.rmdir()) {
                FAIL() << "unable to remove dir '" << cd.abspath() << "'" << endl;
            }
        }
        config.setEnvironment("local");
        config.setDatabaseRegistryFile(TEST_DATA_DIR + "/data/database-registry.xml");
        config.setUrlRegistryFile(TEST_DATA_DIR + "/data/url-registry.xml");
        config.setDisableCache(true);
        config.setCacheDir("build/testresults/cache");
        config.setCopyThreshold(100000);
        config.setExtensionDir(TEST_DATA_DIR + "/../extensions");
        config.setOutputDir("build/testresults");
        config.setDontExecute(false);

        config.setQueryParameter({
            {"sku","skuval"}
        });
    }

    void runQuery(string queryFile) {
        /*
        vector<string> keys = {"JEA","TSH","SHO","SCA","RIN","BAL","PAN","HAT"};
        MD5ShardingStrategy md5ss;
        md5ss.setShardCount(4);
        for (auto key:keys) {
            cout << "shard for " << key << " = " << md5ss.getShardId(key) << endl;
        }
        */
        config.setQueryFile(queryFile);
        app.bootstrap(config);
        bool success = app.run();
        ASSERT_TRUE(success) << "running query failed";
        ASSERT_TRUE(system_diff("build/testresults/" + GetParam(), TEST_DATA_DIR + "/queries/" + GetParam()));
    }

};


vector<string> queries = {"one_to_one","one_to_many","many_to_one","many_to_many","locator"};
//vector<string> queries = {"many_to_one"};
INSTANTIATE_TEST_CASE_P(TestQueries, ApplicationTest,
                         ValuesIn(queries));


TEST_P(ApplicationTest, SmokeTest) {
    runQuery("tests/queries/" + GetParam() + ".xml");
}




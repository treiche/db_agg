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

#include "utils/Diff.h"

using namespace std;
using namespace db_agg;
using namespace testing;

class ApplicationTest: public ::testing::Test, public ::testing::WithParamInterface<string> {
protected:
    string RESULT_DIR = "build/testresults/";
    Application& app = *new Application();
    Configuration config;

    virtual void SetUp() {
        File testlink("/var/tmp/db_agg");
        if (!testlink.exists()) {
            File wd(TEST_DATA_DIR + "/..");
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
        config.setExtensionDir("extensions");
        config.setOutputDir("build/testresults");
        config.setDontExecute(false);

        config.setQueryParameter({
            {"sku","skuval"}
        });
    }

    bool runQuery(string queryFile) {
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
        return app.run();
    }

};


vector<string> queries = {"one_to_one","one_to_many","many_to_one","many_to_many","locator"};
//vector<string> queries = {"many_to_one"};
INSTANTIATE_TEST_CASE_P(TestQueries, ApplicationTest,
                         ValuesIn(queries));


TEST_P(ApplicationTest, SmokeTest) {
    bool success = runQuery(TEST_DATA_DIR + "/queries/" + GetParam() + ".xml");
    ASSERT_TRUE(success) << "running query failed";
    // ASSERT_TRUE(system_diff(RESULT_DIR + GetParam(), TEST_DATA_DIR + "/queries/" + GetParam(),""));
    Diff diff{RESULT_DIR + GetParam(),TEST_DATA_DIR + "/queries/" + GetParam()};
    ASSERT_TRUE(diff.compare(".*\\.svg")) << "result differs";
}

TEST_P(ApplicationTest, RunTwice) {
    runQuery(TEST_DATA_DIR + "/queries/" + GetParam() + ".xml");
    if (GetParam() == "one_to_one") {
        cout << "remove source" << endl;
        // target
        //File f(RESULT_DIR + "cache/5bb5a2805f9c6bd32693989cdf52648c.csv");
        // source
        File f(RESULT_DIR + "cache/038f34c63f94a6ab560326884fa27d80.csv");
        f.remove();
    }
    config.setDisableCache(false);
    bool success = runQuery(TEST_DATA_DIR + "/queries/" + GetParam() + ".xml");
    ASSERT_TRUE(success) << "running query failed";
    Diff diff{RESULT_DIR + GetParam(),TEST_DATA_DIR + "/queries/" + GetParam()};
    ASSERT_TRUE(diff.compare(".*\\.[svg|dot]"));
    ASSERT_FALSE(diff.compare(".*\\.csv"));
}


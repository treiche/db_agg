/*
 * SoapExecution.cpp
 *
 *  Created on: Dec 6, 2014
 *      Author: arnd
 */

#include <log4cplus/logger.h>

#include "db_agg.h"
#include "SoapExecution.h"
#include "utils/TableToDom.h"
#include "utils/xml.h"

#include <cassert>


using namespace std;
using namespace db_agg;
using namespace log4cplus;

extern "C" {
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
}


namespace db_agg {

DECLARE_LOGGER("SoapExecution");

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    LOG_DEBUG("called callback");
    size_t realsize = size * nmemb;
      struct MemoryStruct *mem = (struct MemoryStruct *)userp;

      mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
      if(mem->memory == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
      }

      memcpy(&(mem->memory[mem->size]), contents, realsize);
      mem->size += realsize;
      mem->memory[mem->size] = 0;

      return realsize;
}

bool SoapExecution::process() {

    if (lastOffset == 0) {
        LOG_DEBUG("process url = " << getUrl()->getUrl());
        LOG_DEBUG("query: '" << getSql() << "'");

        prepareQuery();

        shared_ptr<Event> ev(new ExecutionStateChangeEvent(getId(),"CONNECTED"));
        fireEvent(ev);
    }

    auto& source = (*getDependencies().begin()).second;

    uint64_t row;
    for (row = lastOffset; row < lastOffset + chunkSize; row++) {
        auto range = TableDataFactory::getInstance().range(source,row,chunkSize);
        xmlNodePtr result = inputTemplate.transform(range);
        string response = callServer(getUrl(),as_string(result));
        auto rangeResult = outputTemplate.transform(response);
        if (!resultTable) {
            resultTable = TableDataFactory::getInstance().create("text",rangeResult->getColumns());
        }
        for (uint64_t rrow = 0; rrow < rangeResult->getRowCount(); rrow++) {
            vector<string> rv;
            for (uint32_t rcol = 0; rcol < rangeResult->getColCount(); rcol++) {
                rv.push_back(rangeResult->getValue(rrow,rcol));
            }
            resultTable->addRow(rv);
        }
    }

    shared_ptr<Event> rde(new ReceiveDataEvent(getId(),resultTable->getRowCount()));
    fireEvent(rde);

    if (row >= source->getRowCount()) {
        setResult("",resultTable);
        setState(QueryExecutionState::DONE);
        return true;
    }
    lastOffset = row;
    return false;
}

bool SoapExecution::isResourceAvailable() {
    return true;
}

void SoapExecution::prepareQuery() {
    string xml = "<query xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>";
    xml += getSql();
    xml += "</query>";
    LOG_DEBUG("about to parse");
    xmlDocPtr doc = xmlParseDoc((xmlChar*)xml.c_str());
    xmlNodePtr queryElement = xmlDocGetRootElement(doc);
    xmlNodePtr tmp = queryElement->children;

    xmlNodePtr inputNode = nullptr;
    xmlNodePtr outputNode = nullptr;

    while(tmp) {
        if (tmp->type == XML_ELEMENT_NODE) {
            if (inputNode == nullptr) {
                inputNode = tmp;
            } else {
                outputNode = tmp;
            }
        }
        tmp = tmp->next;
    }

    if (inputNode == nullptr || outputNode == nullptr) {
        THROW_EXC("input or output node missing.");
    }

    LOG_DEBUG("inputNode = " << inputNode->name);
    LOG_DEBUG("outputNode = " << outputNode->name);;

    xmlNodePtr inputQuery = xmlDocCopyNode(inputNode,doc,1);
    xmlNodePtr outputQuery = xmlDocCopyNode(outputNode,doc,1);

    assert(getDependencies().size() == 1);

    inputTemplate.setStylesheet(inputQuery);
    outputTemplate.setStylesheet(outputQuery);

}

string SoapExecution::callServer(shared_ptr<Url> url, string request) {
    struct MemoryStruct chunk;
    chunk.memory = (char*)malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl = curl_easy_init();
    if (!curl) {
        THROW_EXC("curl initialization failed");
    }

    curl_easy_setopt(curl, CURLOPT_URL, getUrl()->getUrl().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)request.size());


    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        THROW_EXC("perform failed:" << curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);

    return string(chunk.memory);
}


}



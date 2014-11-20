#include "xml.h"
#include "utils/logging.h"

using namespace std;

namespace db_agg {

DECLARE_LOGGER("xml");

static void errorHandler(void * ctx, const char * msg, ...) {
    va_list ap;
    va_start(ap, msg);
    char buf[1024];
    vsprintf(buf,msg,ap);
    cout << "ERROR: " << buf << endl;
    LOG_ERROR(buf);
}

static void warningHandler(void * ctx, const char * msg, ...) {
    va_list ap;
    va_start(ap, msg);
    char buf[1024];
    vsprintf(buf,msg,ap);
    cout << "WARNING: " << buf << endl;
    LOG_WARN(buf);
}


xmlDocPtr parseDoc(std::string document, std::string baseUri, bool resolveXIncludes) {
    xmlDocPtr doc = xmlReadMemory(document.c_str(), document.size(), baseUri.c_str(), NULL, 0);
    if (doc == nullptr) {
        THROW_EXC("failed to parse xml queries");
    }

    if (resolveXIncludes) {
        int ret = xmlXIncludeProcess(doc);
        if (ret < 0) {
            THROW_EXC("xinclude failed");
        }
    }

    // search for noNamespaceSchemaLocation attribute
    string schemaUrl;
    xmlNodePtr tmp = doc->children;
    while (tmp) {
        if (tmp->type == XML_ELEMENT_NODE) {
            xmlAttributePtr attr = ((xmlElementPtr)tmp)->attributes;
            while (attr) {
                if (string((char*)attr->name) == "noNamespaceSchemaLocation" &&
                    string("http://www.w3.org/2001/XMLSchema-instance") == (char*)((xmlNodePtr)attr)->ns->href
                    ) {

                    schemaUrl = (char*)attr->children->content;
                    ((xmlNodePtr)attr)->ns->href;
                }
                attr = (xmlAttribute*)attr->next;
            }
        }
        tmp = tmp->next;
    }
    LOG_INFO("using schema '" << schemaUrl << "' for validating");
    if (!schemaUrl.empty()) {
        validateDoc(doc,schemaUrl);
    }
    return doc;
}

void validateDoc(xmlDocPtr doc, std::string schemaFile) {
    xmlSchemaParserCtxtPtr ctx = xmlSchemaNewParserCtxt(schemaFile.c_str());
    if (ctx) {
        xmlSchemaSetParserErrors(ctx,(xmlSchemaValidityErrorFunc)errorHandler,(xmlSchemaValidityWarningFunc)warningHandler,nullptr);
        xmlSchemaPtr schema = xmlSchemaParse(ctx);
        xmlSchemaFreeParserCtxt(ctx);
        // validate
        xmlSchemaValidCtxtPtr vctx = xmlSchemaNewValidCtxt(schema);
        xmlSchemaSetValidErrors(vctx, (xmlSchemaValidityErrorFunc)errorHandler, (xmlSchemaValidityErrorFunc)warningHandler,nullptr);
        int ret = xmlSchemaValidateDoc(vctx, doc);
        xmlSchemaFreeValidCtxt(vctx);
        xmlSchemaFree(schema);
        if (ret != 0) {
            THROW_EXC("xml query file is not valid");
        }
    }
}

}

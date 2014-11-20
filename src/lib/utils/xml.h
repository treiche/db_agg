#ifndef XML_H_
#define XML_H_

#include <string>

extern "C" {
    #include <libxml/xmlschemas.h>
    #include <libxml/uri.h>
    #include <libxml/xinclude.h>
}

namespace db_agg {

xmlDocPtr parseDoc(std::string document, std::string baseUri, bool resolveXIncludes = false);
void validateDoc(xmlDocPtr doc, std::string schemaFile);


}


#endif /* XML_H_ */

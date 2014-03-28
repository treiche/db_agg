#ifndef TYPEREGISTRY_H_
#define TYPEREGISTRY_H_

#include <map>
#include <string>

using namespace std;

namespace db_agg {

    class TypeInfo {
        public:
            TypeInfo() {}
            string name;
            long oid = -1;
            char category = 0;
            short length = 0;
            TypeInfo(long oid,string name,char category,short length);
            bool needsQuoting();
    };

    class TypeRegistry {
        private:
            map<long,TypeInfo> types;
            TypeRegistry();
            string getTypeName(long oid);
            static TypeRegistry instance;
        public:
            static TypeRegistry& getInstance() {
                return instance;
            }
            TypeInfo *getTypeInfo(long oid);
            TypeInfo *getTypeInfo(string name);
    };
}

#endif /* TYPEREGISTRY_H_ */

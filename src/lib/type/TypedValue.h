#ifndef TYPEDVALUE_H_
#define TYPEDVALUE_H_

#include <cstdint>
#include <stddef.h>

namespace db_agg {

    class TypedValue {
        friend class PgCopyReader;
        private:
            uint32_t typeId = -1;
            int size;
        public:
            union Value {
                short shortVal;
                int intVal;
                bool boolVal;
                long longVal;
                float floatVal;
                double doubleVal;
                const char *stringVal;
                void *ptr;
            } value;
            TypedValue() {
                this->typeId = -1;
                this->size = 0;
                this->value.ptr = nullptr;
            }
            TypedValue(int typeId, int size, void* value) {
                this->typeId = typeId;
                this->size = size;
                this->value.ptr = value;
            }
            int getTypeId() {
                return typeId;
            }
            void setTypeId(uint32_t typeId) {
                this->typeId = typeId;
            }
            size_t getSize() {
                return size;
            }
            void setSize(size_t size) {
                this->size = size;
            }
            Value getValue() {
                return value;
            }
    };

}

#endif /* TYPEDVALUE_H_ */

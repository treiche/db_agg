#include "TypeRegistry.h"

#include "utils/logging.h"

#include "utils/utility.h"
#include "type/oids.h"

using namespace std;
using namespace log4cplus;


namespace db_agg {

DECLARE_LOGGER("TypeRegistry");

TypeInfo::TypeInfo(long oid,string name,char category,short length) {
    this->oid = oid;
    this->name = name;
    this->category = category;
    this->length = length;
}

bool TypeInfo::needsQuoting() {
    if (this->category == 'S' || this->category == 'D' || this->category == 'T' || this->category == 'A') {
        return true;
    }
    return false;
}

TypeRegistry TypeRegistry::instance;

TypeRegistry::TypeRegistry() {
    types[BOOL] = TypeInfo(BOOL,"bool",'B',1);
    types[BYTEA] = TypeInfo(BYTEA,"bytea",'U',-1);
    types[CHAR] = TypeInfo(CHAR,"char",'S',1);
    types[NAME] = TypeInfo(NAME,"name",'S',64);
    types[INT8OID] = TypeInfo(INT8OID,"int8",'N',8);
    types[INT2OID] = TypeInfo(INT2OID,"int2",'N',2);
    types[INT2VECTOR] = TypeInfo(INT2VECTOR,"int2vector",'A',-1);
    types[INT4OID] = TypeInfo(INT4OID,"int4",'N',4);
    types[REGPROC] = TypeInfo(REGPROC,"regproc",'N',4);
    types[TEXT] = TypeInfo(TEXT,"text",'S',-1);
    types[OID] = TypeInfo(OID,"oid",'N',4);
    types[TID] = TypeInfo(TID,"tid",'U',6);
    types[XID] = TypeInfo(XID,"xid",'U',4);
    types[CID] = TypeInfo(CID,"cid",'U',4);
    types[OIDVECTOR] = TypeInfo(OIDVECTOR,"oidvector",'A',-1);
    types[JSON] = TypeInfo(JSON,"json",'U',-1);
    types[XML] = TypeInfo(XML,"xml",'U',-1);
    types[PGNODETREE] = TypeInfo(PGNODETREE,"pg_node_tree",'S',-1);
    types[POINT] = TypeInfo(POINT,"point",'G',16);
    types[LSEG] = TypeInfo(LSEG,"lseg",'G',32);
    types[PATH] = TypeInfo(PATH,"path",'G',-1);
    types[BOX] = TypeInfo(BOX,"box",'G',32);
    types[POLYGON] = TypeInfo(POLYGON,"polygon",'G',-1);
    types[LINE] = TypeInfo(LINE,"line",'G',32);
    types[FLOAT4OID] = TypeInfo(FLOAT4OID,"float4",'N',4);
    types[FLOAT8OID] = TypeInfo(FLOAT8OID,"float8",'N',8);
    types[ABSTIME] = TypeInfo(ABSTIME,"abstime",'D',4);
    types[RELTIME] = TypeInfo(RELTIME,"reltime",'T',4);
    types[TINTERVAL] = TypeInfo(TINTERVAL,"tinterval",'T',12);
    types[UNKNOWN] = TypeInfo(UNKNOWN,"unknown",'X',-2);
    types[CIRCLE] = TypeInfo(CIRCLE,"circle",'G',24);
    types[CASH] = TypeInfo(CASH,"money",'N',8);
    types[MACADDR] = TypeInfo(MACADDR,"macaddr",'U',6);
    types[INET] = TypeInfo(INET,"inet",'I',-1);
    types[CIDR] = TypeInfo(CIDR,"cidr",'I',-1);
    types[INT4ARRAY] = TypeInfo(INT4ARRAY,"_int4",'A',-1);
    types[TEXTARRAY] = TypeInfo(TEXTARRAY,"_text",'A',-1);
    types[FLOAT4ARRAY] = TypeInfo(FLOAT4ARRAY,"_float4",'A',-1);
    types[ACLITEM] = TypeInfo(ACLITEM,"aclitem",'U',12);
    types[CSTRINGARRAY] = TypeInfo(CSTRINGARRAY,"_cstring",'A',-1);
    types[BPCHAR] = TypeInfo(BPCHAR,"bpchar",'S',-1);
    types[VARCHAR] = TypeInfo(VARCHAR,"varchar",'S',-1);
    types[DATE] = TypeInfo(DATE,"date",'D',4);
    types[TIME] = TypeInfo(TIME,"time",'D',8);
    types[TIMESTAMP] = TypeInfo(TIMESTAMP,"timestamp",'D',8);
    types[TIMESTAMPTZ] = TypeInfo(TIMESTAMPTZ,"timestamptz",'D',8);
    types[INTERVAL] = TypeInfo(INTERVAL,"interval",'T',16);
    types[TIMETZ] = TypeInfo(TIMETZ,"timetz",'D',12);
    types[BIT] = TypeInfo(BIT,"bit",'V',-1);
    types[VARBIT] = TypeInfo(VARBIT,"varbit",'V',-1);
    types[NUMERIC] = TypeInfo(NUMERIC,"numeric",'N',-1);
    types[REFCURSOR] = TypeInfo(REFCURSOR,"refcursor",'U',-1);
    types[REGPROCEDURE] = TypeInfo(REGPROCEDURE,"regprocedure",'N',4);
    types[REGOPER] = TypeInfo(REGOPER,"regoper",'N',4);
    types[REGOPERATOR] = TypeInfo(REGOPERATOR,"regoperator",'N',4);
    types[REGCLASS] = TypeInfo(REGCLASS,"regclass",'N',4);
    types[REGTYPE] = TypeInfo(REGTYPE,"regtype",'N',4);
    types[REGTYPEARRAY] = TypeInfo(REGTYPEARRAY,"_regtype",'A',-1);
    types[TSVECTOR] = TypeInfo(TSVECTOR,"tsvector",'U',-1);
    types[GTSVECTOR] = TypeInfo(GTSVECTOR,"gtsvector",'U',-1);
    types[TSQUERY] = TypeInfo(TSQUERY,"tsquery",'U',-1);
    types[REGCONFIG] = TypeInfo(REGCONFIG,"regconfig",'N',4);
    types[REGDICTIONARY] = TypeInfo(REGDICTIONARY,"regdictionary",'N',4);
    types[INT4RANGE] = TypeInfo(INT4RANGE,"int4range",'R',-1);
    types[RECORD] = TypeInfo(RECORD,"record",'P',-1);
    types[RECORDARRAY] = TypeInfo(RECORDARRAY,"_record",'P',-1);
    types[CSTRING] = TypeInfo(CSTRING,"cstring",'P',-2);
    types[ANY] = TypeInfo(ANY,"any",'P',4);
    types[ANYARRAY] = TypeInfo(ANYARRAY,"anyarray",'P',-1);
    types[VOID] = TypeInfo(VOID,"void",'P',4);
    types[TRIGGER] = TypeInfo(TRIGGER,"trigger",'P',4);
    types[LANGUAGE_HANDLER] = TypeInfo(LANGUAGE_HANDLER,"language_handler",'P',4);
    types[INTERNAL] = TypeInfo(INTERNAL,"internal",'P',8);
    types[OPAQUE] = TypeInfo(OPAQUE,"opaque",'P',4);
    types[ANYELEMENT] = TypeInfo(ANYELEMENT,"anyelement",'P',4);
    types[ANYNONARRAY] = TypeInfo(ANYNONARRAY,"anynonarray",'P',4);
    types[ANYENUM] = TypeInfo(ANYENUM,"anyenum",'P',4);
    types[FDW_HANDLER] = TypeInfo(FDW_HANDLER,"fdw_handler",'P',4);
    types[ANYRANGE] = TypeInfo(ANYRANGE,"anyrange",'P',-1);
}

string TypeRegistry::getTypeName(long oid) {
    if (types.find(oid)==types.end()) {
        return nullptr;
    }
    TypeInfo& typeInfo =  types[oid];
    return typeInfo.name;
}

TypeInfo *TypeRegistry::getTypeInfo(long oid) {
    if (types.find(oid)==types.end()) {
        return nullptr;
    }
    return &types[oid];
}

TypeInfo *TypeRegistry::getTypeInfo(string name) {
    for (auto& typeInfo:types) {
        if (typeInfo.second.name.compare(name) == 0) {
            return &typeInfo.second;
        }
    }
    return nullptr;
}
}

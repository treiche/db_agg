/*
 * VirtualTableData.cpp
 *
 *  Created on: Nov 29, 2014
 *      Author: arnd
 */

#include "VirtualTableData.h"
#include "utils/logging.h"
#include "type/TypeRegistry.h"


using namespace std;

DECLARE_LOGGER("VirtualTableData")

map<string, shared_ptr<db_agg::TableData>> tables;

void registerTableData(std::string name,
        std::shared_ptr<db_agg::TableData> tabledata) {
    tables[name] = tabledata;
}

typedef struct {
    sqlite3_vtab vtab;
    shared_ptr<db_agg::TableData> tableData;
} tabledata_vtab;

typedef struct {
    sqlite3_vtab_cursor cursor;
    uint64_t pos;
} tabledata_cursor;

static int tabledata_connect(sqlite3* db, void *pAux, int argc,
        const char * const *argv, sqlite3_vtab **ppVTab, char **pzErr) {

    for (int idx = 0; idx < argc; idx++) {
        LOG_DEBUG("arg[" << idx << "] = " << argv[idx]);
    }
    LOG_DEBUG("tabledata_connect");
    string depName = argv[3];
    if (tables.find(depName) == tables.end()) {
        THROW_EXC("table '" << depName << "' is not registered");
    }
    LOG_DEBUG("depName = " << depName);
    shared_ptr<db_agg::TableData> currentTable = tables[depName];
    LOG_DEBUG("tableData = " << currentTable->getRowCount());

    string schema = "CREATE TABLE x(";
    size_t colCount = currentTable->getColCount();
    for (size_t idx = 0; idx < colCount; idx++) {
        LOG_DEBUG("get column definition " << idx);
        auto col = currentTable->getColumns()[idx];
        db_agg::TypeInfo *ti = db_agg::TypeRegistry::getInstance().getTypeInfo(col.second);
        schema += col.first + " " + ti->name;
        if (idx < colCount - 1) {
            schema += ",";
        }
    }
    schema += ")";
    LOG_DEBUG("schema = " << schema);
    int rc = sqlite3_declare_vtab(db, schema.c_str());
    if (rc != SQLITE_OK) {
        THROW_EXC("schema declaration failed");
    }

    tabledata_vtab * vtab = (tabledata_vtab *) calloc(1,sizeof(tabledata_vtab));
    vtab->tableData = currentTable;
    *ppVTab = &vtab->vtab;
    return SQLITE_OK;
}

static int tabledata_create(sqlite3* db, void *pAux, int argc,
        const char * const *argv, sqlite3_vtab **ppVTab, char **pzErr) {

    LOG_DEBUG("tabledata_create");
    return tabledata_connect(db, pAux, argc, argv, ppVTab, pzErr);
}

static
int tabledata_disconnect(sqlite3_vtab *pVTab) {
    LOG_DEBUG("tabledata_disconnect");
    tabledata_vtab * tab = (tabledata_vtab *) pVTab;
    free(tab);
    return SQLITE_OK;
}

static
int tabledata_destroy(sqlite3_vtab *pVTab) {
    LOG_DEBUG("tabledata_destroy");
    return SQLITE_OK;
}

static
int tabledata_best_index(sqlite3_vtab *pVTab, sqlite3_index_info* info) {
    LOG_DEBUG("tabledata_best_index");
    return SQLITE_OK;
}

static
int tabledata_open(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor) {
    LOG_DEBUG("tabledata_open");
    tabledata_cursor * cur = (tabledata_cursor *) malloc(sizeof(*cur));
    if (!cur)
        return SQLITE_ERROR;

    cur->cursor.pVtab = pVTab;
    cur->pos = 0;

    *ppCursor = &cur->cursor;
    return SQLITE_OK;
}

static int tabledata_close(sqlite3_vtab_cursor* pCursor) {
    LOG_DEBUG("tabledata_close");
    free(pCursor);
    return SQLITE_OK;
}

static int tabledata_filter(sqlite3_vtab_cursor* pCursor, int idxNum,
        const char *idxStr, int argc, sqlite3_value **argv) {
    LOG_DEBUG("tabledata_filter " << idxNum << "," << idxStr);
    tabledata_cursor * cur = (tabledata_cursor *) pCursor;
    cur->pos = 0;
    return SQLITE_OK;
}

static
int tabledata_eof(sqlite3_vtab_cursor* pCursor) {
    tabledata_cursor * cur = (tabledata_cursor *) pCursor;
    tabledata_vtab * tab = (tabledata_vtab *) cur->cursor.pVtab;

    LOG_DEBUG("tabledata_eof " << cur->pos);

    if (cur->pos >= tab->tableData->getRowCount()) {
        return 1;
    }

    return SQLITE_OK;
}

static
int tabledata_next(sqlite3_vtab_cursor* pCursor) {
    tabledata_cursor * cur = (tabledata_cursor *) pCursor;
    //tabledata_vtab * tab = (tabledata_vtab *) cur->cursor.pVtab;
    LOG_DEBUG("tabledata_next pos = " << cur->pos);
    cur->pos++;
    return SQLITE_OK;
}

static
int tabledata_rowid(sqlite3_vtab_cursor* pCursor, sqlite3_int64 *pRowid) {
    LOG_DEBUG("tabledata_rowid");
    return SQLITE_OK;
}

static
int tabledata_column(sqlite3_vtab_cursor* pCursor, sqlite3_context* ctx,
        int n) {
    LOG_DEBUG("tabledata_column " << n);
    tabledata_cursor * cur = (tabledata_cursor *) pCursor;
    tabledata_vtab * tab = (tabledata_vtab *) cur->cursor.pVtab;

    string value = tab->tableData->getValue(cur->pos, n);
    LOG_DEBUG("value = " << value);
    if (value == "\\N") {
        sqlite3_result_null(ctx);
        return SQLITE_OK;
    }
    sqlite3_result_text(ctx, value.c_str(), -1, SQLITE_TRANSIENT);
    return SQLITE_OK;
}

int sq3_register_virtual_tabledata(sqlite3 * db) {
    static sqlite3_module tabledata_module;
    tabledata_module.iVersion = 1;
    tabledata_module.xCreate = tabledata_create;
    tabledata_module.xConnect = tabledata_connect;
    tabledata_module.xDisconnect = tabledata_disconnect;
    tabledata_module.xDestroy = tabledata_destroy;
    tabledata_module.xBestIndex = tabledata_best_index;
    tabledata_module.xOpen = tabledata_open;
    tabledata_module.xClose = tabledata_close;
    tabledata_module.xFilter = tabledata_filter;
    tabledata_module.xEof = tabledata_eof;
    tabledata_module.xNext = tabledata_next;
    tabledata_module.xRowid = tabledata_rowid;
    tabledata_module.xColumn = tabledata_column;
    return sqlite3_create_module(db, "DBAGG", &tabledata_module, nullptr);
}


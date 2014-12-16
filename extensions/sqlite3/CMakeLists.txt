

include_directories(../../src/lib)

add_library(sqlite3 SHARED
    Sqlite3Execution.cpp
    VirtualTableData.cpp
    module.cpp
)

target_link_libraries(sqlite3 "/usr/lib/x86_64-linux-gnu/libsqlite3.so")
install(TARGETS sqlite3 DESTINATION extensions)



include_directories(../../src/lib)

find_package(PostgreSQL REQUIRED)
include_directories(${POSTGRESQL_INCLUDE_DIR})

add_library(postgresql SHARED
    PostgresqlExecution.cpp
    PGQueryExecution.cpp
    AsyncQueryExecutor.cpp
    PGConnection.cpp
    PGResult.cpp
    module.cpp
)

target_link_libraries(postgresql
    ${POSTGRESQL_LIBRARIES}
)

if (dpkg_build)
    install(TARGETS postgresql DESTINATION ${libdir}/db_agg/extensions)
else()
    install(TARGETS postgresql DESTINATION extensions)
endif()

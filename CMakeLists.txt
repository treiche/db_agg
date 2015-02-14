cmake_minimum_required(VERSION 2.8.4)

project(db_agg)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 9)
set(VERSION_PATCH 4)
set(PACKAGE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(PCRE REQUIRED)
find_package(Jansson REQUIRED)
find_package(LibZip REQUIRED)
find_package(Sqlite3)

#message("sqlite3 = ${SQLITE3_FOUND} ${SQLITE3_INCLUDE_DIR} $ENV{SHELL} ")
#find_package(Log4cplus REQUIRED)

set(LOG4CPLUS_LIBRARIES "/usr/lib/liblog4cplus.so")

set(CURSES_NEED_WIDE "True")
find_package(Curses REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(LibXslt REQUIRED)
find_package (Threads REQUIRED)

include_directories(${LIBXSLT_INCLUDE_DIR})
include_directories(${LIBXML2_INCLUDE_DIR})


SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -std=c++11 -fPIC")

#
# configuration options formerly given by autoconf
#

if (NOT dpkg_build)

    SET(prefix ${CMAKE_INSTALL_PREFIX})
    SET(multiarchlibdir "\${prefix}/lib")
    SET(libdir "\${prefix}/lib")
    SET(sysconfdir "\${prefix}/etc")
    SET(includedir "\${prefix}/include")
    SET(mandir "\${prefix}/share/man")
    SET(infodir "\${prefix}/share/info")
    SET(localstatedir "\${prefix}/var")

endif()

configure_file (
    "${PROJECT_SOURCE_DIR}/src/installation.h.in"
    "${PROJECT_BINARY_DIR}/installation.h"
    set(DB_AGG_CONFIG_HEADER_DIR ${PROJECT_BINARY_DIR})
)

add_subdirectory("src/lib")
add_subdirectory("extensions/memcached")
add_subdirectory("extensions/sqlite3")
add_subdirectory("extensions/soap")
add_subdirectory("extensions/postgresql")
add_subdirectory("bindings/python")

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src/lib")
include_directories("${PROJECT_SOURCE_DIR}/src/bin")
#include_directories("${PROJECT_SOURCE_DIR}/src/lib/core")

set(BIN_SUBDIRS cli ui)
foreach (subdir ${BIN_SUBDIRS})
    file(GLOB sources src/bin/${subdir}/*.cpp)
    add_library(bin-${subdir} OBJECT ${sources})
endforeach(subdir)

add_executable(db_agg
    src/bin/db_agg.cpp
    $<TARGET_OBJECTS:bin-cli>
    $<TARGET_OBJECTS:bin-ui>
)

target_link_libraries(db_agg libdbagg
    ${CURSES_LIBRARIES}
)

add_executable(db_agg_config
    src/bin/db_agg_config.cpp
    $<TARGET_OBJECTS:bin-cli>
    $<TARGET_OBJECTS:bin-ui>
)

target_link_libraries(db_agg_config libdbagg
    ${CURSES_LIBRARIES}
)


file(GLOB sources src/test/*.cpp)
add_library(testsrc OBJECT ${sources})

add_executable(testrunner
    src/bin/testrunner.cpp
    "/usr/src/gmock/gtest/src/gtest-all.cc"
    $<TARGET_OBJECTS:bin-cli>
    $<TARGET_OBJECTS:bin-ui>
    $<TARGET_OBJECTS:testsrc>
)

include_directories("/usr/src/gmock/gtest/")
include_directories("/usr/src/gmock/gtest/include")

target_link_libraries(testrunner
    libdbagg
    ${LIBXML2_LIBRARIES}
    ${PCRE_LIBRARIES}
    ${LOG4CPLUS_LIBRARIES}
    ${JANSSON_LIBRARIES}
    ${CURSES_LIBRARIES}
    ${LIBZIP_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${LIBXSLT_LIBRARIES}

)

install(TARGETS db_agg DESTINATION bin)
install(TARGETS testrunner DESTINATION bin)

install(DIRECTORY tests/queries DESTINATION tests)
install(FILES tests/data/database-registry.xml DESTINATION tests/data/)
install(FILES tests/data/url-registry.xml DESTINATION tests/data/)
install(FILES tests/data/local_unsharded_db.sqlite3 DESTINATION tests/data/)
install(FILES tests/data/local_sharded1_db.sqlite3 DESTINATION tests/data/)
install(FILES tests/data/local_sharded2_db.sqlite3 DESTINATION tests/data/)
install(FILES tests/data/local_sharded3_db.sqlite3 DESTINATION tests/data/)
install(FILES tests/data/local_sharded4_db.sqlite3 DESTINATION tests/data/)

if (dpkg_build)
    install(
        DIRECTORY etc/
        DESTINATION "${sysconfdir}/db_agg"
    )
else()
    install(
        DIRECTORY etc/
        DESTINATION etc
    )
endif()

install(FILES src/lib/db_agg.h "${PROJECT_BINARY_DIR}/installation.h" DESTINATION include/${CMAKE_PROJECT_NAME})

enable_testing()
add_test(NAME testrunner COMMAND testrunner --gtest_filter=TestQueries/ApplicationTest.SmokeTest/*)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
                  DEPENDS testrunner)
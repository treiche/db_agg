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
find_package(Log4cplus REQUIRED)

find_package(Curses REQUIRED)

find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})

find_package(PostgreSQL REQUIRED)
include_directories(${POSTGRESQL_INCLUDE_DIR})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -fPIC")

#
# configuration options formerly given by autoconf
#

if (NOT dpkg_build)

    SET(prefix ${CMAKE_INSTALL_PREFIX})
    SET(libdir "\${prefix}/libs")
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

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src/lib")
include_directories("${PROJECT_SOURCE_DIR}/src/bin")
include_directories("${PROJECT_SOURCE_DIR}/src/lib/core")

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
    ${POSTGRESQL_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    ${PCRE_LIBRARIES}
    ${LOG4CPLUS_LIBRARIES}
    ${JANSSON_LIBRARIES}
    ${CURSES_LIBRARIES}
    ${LIBZIP_LIBRARY}
)

add_executable(db_agg_config
    src/bin/db_agg_config.cpp
    $<TARGET_OBJECTS:bin-cli>
    $<TARGET_OBJECTS:bin-ui>
)

target_link_libraries(db_agg_config libdbagg
    ${POSTGRESQL_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    ${PCRE_LIBRARIES}
    ${LOG4CPLUS_LIBRARIES}
    ${JANSSON_LIBRARIES}
    ${CURSES_LIBRARIES}
    ${LIBZIP_LIBRARY}
)

install(TARGETS db_agg DESTINATION bin)
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

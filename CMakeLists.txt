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

list(APPEND CMAKE_CXX_FLAGS " -Wall -std=c++11 -fPIC")

configure_file (
    "${PROJECT_SOURCE_DIR}/src/installation_cmake.h.in"
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

install(TARGETS db_agg DESTINATION bin COMPONENT bin)
install(TARGETS db_agg_config DESTINATION bin COMPONENT bin)
install(FILES src/lib/db_agg.h src/installation.h DESTINATION include/${CMAKE_PROJECT_NAME} COMPONENT dev)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)


set(CPACK_GENERATOR "DEB")

# Set package per component
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_GROUPING "IGNORE")
set(CPACK_DEB_CHANGELOG_FILE debian/changelog)
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "debhelper (>= 9), autotools-dev, libtool, libjansson-dev, libncurses5-dev, libncursesw5-dev, liblog4cplus-dev, libxml2-dev, cxxtest, libpcre3-dev, libzip-dev, libpq-dev")
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION "Join multiple datasources with a single query")
set(CPACK_PACKAGE_CONTACT "Arnd Brusdeilins")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/debian/postinst")

include(CPack)

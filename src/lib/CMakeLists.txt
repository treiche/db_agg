cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x -fPIC")

find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})

set(PostgreSQL_ADDITIONAL_SEARCH_PATHS "/usr/include/postgresql/9.3/server")
find_package(PostgreSQL REQUIRED)
include_directories(${PostgreSQL_INCLUDE_DIRS})

configure_file (
  "${PROJECT_SOURCE_DIR}/../installation_cmake.h.in"
  "${PROJECT_BINARY_DIR}/installation.h"
  )

include_directories("${PROJECT_BINARY_DIR}")

set(SUBDIRS cache core event excel extension graph injection postgres sharding table type utils)
foreach (subdir ${SUBDIRS})
    file(GLOB sources ${subdir}/*.cpp)
    add_library(${subdir} OBJECT ${sources})
endforeach(subdir)

add_library(dbagg SHARED
    $<TARGET_OBJECTS:cache>
    $<TARGET_OBJECTS:core>
    $<TARGET_OBJECTS:event>
    $<TARGET_OBJECTS:excel>
    $<TARGET_OBJECTS:extension>
    $<TARGET_OBJECTS:graph>
    $<TARGET_OBJECTS:injection>
    $<TARGET_OBJECTS:postgres>
    $<TARGET_OBJECTS:sharding>
    $<TARGET_OBJECTS:table>
    $<TARGET_OBJECTS:type>
    $<TARGET_OBJECTS:utils>)

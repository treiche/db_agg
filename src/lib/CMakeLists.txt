cmake_minimum_required(VERSION 2.8)

include_directories("${CMAKE_CURRENT_LIST_DIR}")

if(NOT DEFINED DB_AGG_CONFIG_HEADER_DIR)
    set(DB_AGG_CONFIG_HEADER_DIR ${CMAKE_CURRENT_LIST_DIR}/../)
endif()

include_directories("${DB_AGG_CONFIG_HEADER_DIR}")

set(SUBDIRS cache core event excel extension graph injection postgres sharding table type utils)
foreach (subdir ${SUBDIRS})
    file(GLOB sources ${subdir}/*.cpp)
    add_library(${subdir} OBJECT ${sources})
endforeach(subdir)

set(OUTPUT_NAME db_agg)
add_library(libdbagg SHARED
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

SET_TARGET_PROPERTIES(libdbagg PROPERTIES OUTPUT_NAME db_agg)
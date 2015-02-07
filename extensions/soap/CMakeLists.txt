

include_directories(../../src/lib)

add_library(soap SHARED
    SoapExecution.cpp
    module.cpp
)

target_link_libraries(soap "/usr/lib/x86_64-linux-gnu/libcurl.so")

if (dpkg_build)
    install(TARGETS soap DESTINATION ${libdir}/db_agg/extensions)
else()
    install(TARGETS soap DESTINATION extensions)
endif()

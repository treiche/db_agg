

include_directories(../../src/lib)

add_library(soap SHARED
    SoapExecution.cpp
    module.cpp
)

target_link_libraries(soap "/usr/lib/x86_64-linux-gnu/libcurl.so")

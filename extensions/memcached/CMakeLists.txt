

include_directories(../../src/lib)

add_library(memcached SHARED
    Memcached.cpp
    MemcachedQuery.cpp
    module.cpp
)

target_link_libraries(memcached "/usr/lib/x86_64-linux-gnu/libmemcached.so")
install(TARGETS memcached DESTINATION extensions)

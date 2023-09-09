set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(UTIL_SOURCES
        util/IOBuf.cpp)

add_library(UTIL_LIB OBJECT ${UTIL_SOURCES})
add_library(zhengqi_rpc STATIC $<TARGET_OBJECTS:UTIL_LIB>)

file(GLOB UTIL_HEADERS "util/*.h")
install(FILES ${UTIL_HEADERS} DESTINATION include/brpc/util)
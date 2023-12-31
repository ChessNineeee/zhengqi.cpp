cmake_minimum_required(VERSION 3.10)
project(zhengqi.cpp)

set(CMAKE_CXX_STANDARD 14)
enable_testing()
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CXX_FLAGS
        -g
        # -DVALGRIND
        -DCHECK_PTHREAD_RETURN_VALUE
        -D_FILE_OFFSET_BITS=64
        -Wall
        -Wextra
        -Werror
        -Wconversion
        -Wno-unused-parameter
        -Wold-style-cast
        -Woverloaded-virtual
        -Wpointer-arith
        -Wshadow
        -Wwrite-strings
        -march=native
        # -MMD
        -std=c++14
        -rdynamic
)

if (CMAKE_BUILD_BITS EQUAL 32)
    list(APPEND CXX_FLAGS "-m32")
endif ()
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    list(APPEND CXX_FLAGS "-Wno-null-dereference")
    list(APPEND CXX_FLAGS "-Wno-sign-conversion")
    list(APPEND CXX_FLAGS "-Wno-unused-local-typedef")
    list(APPEND CXX_FLAGS "-Wthread-safety")
    list(REMOVE_ITEM CXX_FLAGS "-rdynamic")
endif ()
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_BUILD_PARALLEL_LEVEL 8)

find_package(Boost REQUIRED COMPONENTS  system thread chrono atomic)
set(BOOST_LIBRARYDIR "/usr/lib/x86_64-linux-gnu/")
set(BOOST_LATEST_INCLUDE_DIR    "/usr/include/boost")
add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)
find_package(CURL REQUIRED)
find_path(CARES_INCLUDE_DIR NAMES ares.h )
find_library(CARES_LIBRARY NAMES cares )
find_path(TCMALLOC_INCLUDE_DIR NAMES gperftools/heap-profiler.h)
find_library(TCMALLOC_LIBRARY NAMES tcmalloc_and_profiler)
find_package(FMT REQUIRED)
find_package(coost REQUIRED CONFIG)

if (coost_FOUND) 
  message(STATUS "found coost")
endif()
if (FMT_FOUND) 
  message(STATUS "found fmt")
endif()
if (CURL_FOUND)
  message(STATUS "found curl")
endif()

if(CARES_INCLUDE_DIR AND CARES_LIBRARY)
  message(STATUS "found cares")
endif()

if(TCMALLOC_INCLUDE_DIR AND TCMALLOC_LIBRARY)
  message(STATUS "found tcmalloc")
endif()

install()

include_directories(${PROJECT_SOURCE_DIR})

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

add_subdirectory(utility)
add_subdirectory(networking)
add_subdirectory(contrib)
add_subdirectory(networking-examples)
# add_subdirectory(abseil-cpp)

cmake_minimum_required(VERSION 3.16)
project(zhengqi.cpp)

set(CMAKE_CXX_STANDARD 11)
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
        -std=c++11
        -rdynamic
        )

set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

install()

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(utility)
add_subdirectory(contrib)




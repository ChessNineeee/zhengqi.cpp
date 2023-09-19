set(
        inspect_SRCS
        Inspector.cpp
        PerformanceInspector.cpp
        ProcessInspector.cpp
        SystemInspector.cpp
        )

add_library(zhengqi_inspect ${inspect_SRCS})
target_link_libraries(zhengqi_inspect zhengqi_http)

if(TCMALLOC_INCLUDE_DIR AND TCMALLOC_LIBRARY)
        set_target_properties(zhengqi_inspect PROPERTIES COMPILE_FLAGS "-DHAVE_TCMALLOC")
        target_link_libraries(zhengqi_inspect tcmaclloc_and_profiler)
endif()

set(HEADER 
        Inspector.h)
install(FILES ${HEADERS} DESTINATION include/networking/inspect)


set(base_SRCS
        Condition.cpp
        CountDownLatch.cpp
        Thread.cpp
        Exception.cpp
        CurrentThread.cpp
        Timestamp.cpp
        ThreadPool.cpp
        LogStream.cpp
        Date.cpp
)

add_library(zhengqi_utility ${base_SRCS})
target_link_libraries(zhengqi_utility pthread)

install(TARGETS zhengqi_utility DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/utility)

add_subdirectory(tests)
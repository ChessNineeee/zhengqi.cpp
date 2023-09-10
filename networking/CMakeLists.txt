set(networking_SRCS
    Socket.cpp
    InetAddress.cpp
    )

add_library(zhengqi_networking ${networking_SRCS})
target_link_libraries(zhengqi_networking zhengqi_utility)


install(TARGETS zhengqi_networking DESTINATION lib)


set(HEADERS
    InetAddress.h
    )

install(FILES ${HEADERS} DESTINATION include/networking)


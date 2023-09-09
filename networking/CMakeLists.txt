set(networking_SRCS
    Socket.cpp
    )

add_library(zhengqi_networking ${networking_SRCS})
target_link_libraries(zhengqi_networking zhengqi_utility)


install(TARGETS zhengqi_networking DESTINATION lib)


set(HEADERS
    
    )

install(FILES ${HEADERS} DESTINATION include/networking)


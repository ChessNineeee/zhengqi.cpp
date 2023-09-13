set(networking_SRCS
        Socket.cpp
        InetAddress.cpp
        SocketsOps.cpp
        Channel.cpp
        Buffer.cpp
        EventLoop.cpp
        EventLoopThread.cpp
        EventLoopThreadPool.cpp
        Poller.cpp
        TimerQueue.cpp
        Timer.cpp
        poller/DefaultPoller.cpp
        poller/EPollPoller.cpp
        poller/PollPoller.cpp
        TcpConnection.cpp
)

add_library(zhengqi_networking ${networking_SRCS})
target_link_libraries(zhengqi_networking zhengqi_utility)


install(TARGETS zhengqi_networking DESTINATION lib)


set(HEADERS
        InetAddress.h
        Buffer.h
        Channel.h
        Callbacks.h
        Endian.h
        EventLoop.h
        EventLoopThread.h
        EventLoopThreadPool.h
        TimerId.h
        TcpConnection.h
)

install(FILES ${HEADERS} DESTINATION include/networking)
add_subdirectory(tests)


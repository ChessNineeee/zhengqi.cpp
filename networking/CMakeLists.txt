set(networking_SRCS
        Acceptor.cpp
        Socket.cpp
        InetAddress.cpp
        SocketsOps.cpp
        Channel.cpp
        Buffer.cpp
        Connector.cpp
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
        TcpServer.cpp
        TcpClient.cpp
)

add_library(zhengqi_networking ${networking_SRCS})
target_link_libraries(zhengqi_networking zhengqi_utility)


install(TARGETS zhengqi_networking DESTINATION lib)


set(HEADERS
        InetAddress.h
        Buffer.h
        Channel.h
        Callbacks.h
        Connector.h
        Endian.h
        EventLoop.h
        EventLoopThread.h
        EventLoopThreadPool.h
        TimerId.h
        TcpConnection.h
        TcpServer.h
        TcpClient.h
)

install(FILES ${HEADERS} DESTINATION include/networking)
add_subdirectory(tests)
add_subdirectory(http)
add_subdirectory(inspect)

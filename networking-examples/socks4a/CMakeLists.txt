add_executable(balancer balancer.cpp)
target_link_libraries(balancer zhengqi_networking)

add_executable(tcprelay tcprelay.cpp)
target_link_libraries(tcprelay zhengqi_networking)

add_executable(socks4a socks4a.cpp)
target_link_libraries(socks4a zhengqi_networking)

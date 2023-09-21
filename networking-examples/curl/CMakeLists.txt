add_library(zhengqi_curl Curl.cpp)
target_link_libraries(zhengqi_curl zhengqi_networking)
target_link_libraries(zhengqi_curl curl)

install(TARGETS muduo_curl DESTINATION lib)

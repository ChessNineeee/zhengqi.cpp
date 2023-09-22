add_library(zhengqi_curl Curl.cpp)
target_link_libraries(zhengqi_curl zhengqi_networking)
target_link_libraries(zhengqi_curl curl)

install(TARGETS zhengqi_curl DESTINATION lib)
install(FILES Curl.h DESTINATION include/zhengqi/curl)

add_executable(mcurl mcurl.cpp)
target_link_libraries(mcurl zhengqi_curl)

add_executable(curl_download download.cpp)
target_link_libraries(curl_download zhengqi_curl)

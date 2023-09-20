set(
        http_SRCS
        HttpServer.cpp
        HttpResponse.cpp
        HttpContext.cpp
)

add_library(zhengqi_http ${http_SRCS})
target_link_libraries(zhengqi_http zhengqi_networking)

install(TARGETS zhengqi_http DESTINATION lib)
set(
        HEADERS
        HttpContext.h
        HttpRequest.h
        HttpResponse.h
        HttpServer.h
)
install(FILES ${HEADERS} DESTINATION include/networking/http)

if (ZHENGQI_BUILD_EXAMPLES)
    add_executable(httpserver_test tests/HttpServer_test.cpp)
    target_link_libraries(httpserver_test zhengqi_http)
endif ()

add_executable(httprequest_unittest tests/HttpRequest_unittest.cpp)
target_link_libraries(httprequest_unittest zhengqi_http)
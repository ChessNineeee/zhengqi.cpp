add_library(zhengqi_cdns Resolver.cpp)
target_link_libraries(zhengqi_cdns zhengqi_networking)
target_link_libraries(zhengqi_cdns cares)

install(TARGETS zhengqi_cdns DESTINATION lib)
install(FILES Resolver.h DESTINATION include/zhengqi/cdns)


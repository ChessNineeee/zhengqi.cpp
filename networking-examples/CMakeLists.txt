add_subdirectory(socks4a)

if(CARES_INCLUDE_DIR AND CARES_LIBRARY)
  add_subdirectory(cdns)
else()
  add_subdirectory(cdns EXCLUDE_FROM_ALL)
endif()

if (CURL_FOUND)
  add_subdirectory(curl)
else()
  add_subdirectory(curl EXCLUDE_FROM_ALL)
endif()

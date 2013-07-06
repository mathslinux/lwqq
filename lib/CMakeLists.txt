set(LWQQ_LIST
  url.c
  md5.c
  unicode.c
  logger.c
  login.c
  json.c
  msg.c
  type.c
  smemory.c
  info.c
  swsqlite.c
  vplist.c
  utility.c
  internal.c
  async.c
  http.c
  )

add_definitions(-Wall)

ADD_LIBRARY(lwqq SHARED
    ${LWQQ_LIST}
    )
include_directories(
  ${ZLIB_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

include_directories(${EV_INCLUDE_DIRS})
target_link_libraries(lwqq ${EV_LIBRARIES})

if(SSL)
    add_definitions(-DLWQQ_ENABLE_SSL=1)
endif(SSL)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-strict-aliasing")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DUSE_DEBUG=1 -Wno-unused-function")

target_link_libraries(lwqq
  ${SQLITE_LIBRARIES}
  ${CURL_LIBRARIES}
  ${ZLIB_LIBRARIES}
  )

#INSTALL(TARGETS webqq DESTINATION ${LIB_INSTALL_DIR})

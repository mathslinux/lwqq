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
set(LWQQ_HEADER
    async.h
    http.h
    info.h
    logger.h
    login.h
    lwdb.h
    msg.h
    smemory.h
    type.h
    utility.h
    )

add_definitions(-Wall)

add_library(lwqq SHARED
    ${LWQQ_LIST}
    )
include_directories(
  ${ZLIB_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${EV_INCLUDE_DIRS}
  )

if(SSL)
    add_definitions(-DLWQQ_ENABLE_SSL=1)
endif(SSL)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-strict-aliasing")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DUSE_DEBUG=1 -Wno-unused-function")

target_link_libraries(lwqq
  ${EV_LIBRARIES}
  ${SQLITE_LIBRARIES}
  ${CURL_LIBRARIES}
  ${ZLIB_LIBRARIES}
  )

INSTALL(TARGETS lwqq DESTINATION lib)
INSTALL(FILES ${LWQQ_HEADER} DESTINATION include/lwqq)

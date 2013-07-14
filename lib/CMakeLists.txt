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
add_definitions(-Wall )

include_directories(
  ${ZLIB_INCLUDE_DIRS}
  ${CURL_INCLUDE_DIRS}
  ${SQLITE_INCLUDE_DIRS}
  ${PROJECT_BINARY_DIR}
  )

if(UNIX)
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-strict-aliasing")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DUSE_DEBUG=1 -Wno-unused-function")
endif(UNIX)

if(WIN32)
  set(LWQQ_LIST ${LWQQ_LIST} win32.c ../libev/ev.c ../libev/event.c)
  set(LWQQ_OTHER_LIBS ${LWQQ_OTHER_LIBS} ws2_32)
  add_definitions(-DEV_STANDALONE)
  include_directories(../libev)
endif(WIN32)

if(SSL)
    add_definitions(-DLWQQ_ENABLE_SSL=1)
endif(SSL)

add_library(lwqq SHARED
    ${LWQQ_LIST}
    )
target_link_libraries(lwqq
  ${SQLITE_LIBRARIES}
  ${CURL_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${LWQQ_OTHER_LIBS}
  pthread
  )

if(WITH_LIBEV)
    include_directories(${EV_INCLUDE_DIRS})
    target_link_libraries(lwqq ${EV_LIBRARIES})
endif()
if(WITH_LIBUV)
    include_directories(${UV_INCLUDE_DIRS})
    target_link_libraries(lwqq ${UV_LIBRARIES})
endif()

INSTALL(TARGETS lwqq DESTINATION lib)
INSTALL(FILES ${LWQQ_HEADER} DESTINATION include/lwqq)

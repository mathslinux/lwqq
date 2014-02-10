set(LWQQ_LIST
    url.c
    md5.c
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
    lwdb.c
	lwjs.c
    )
set(LWQQ_HEADER
	queue.h
    async.h
    async_impl.h
    http.h
    info.h
    logger.h
    login.h
    lwdb.h
    msg.h
    smemory.h
    type.h
    utility.h
    lwqq.h
    vplist.h
    swsqlite.h
    lwjs.h
    )
add_definitions(-Wall )

include_directories(
    ${ZLIB_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${SQLITE_INCLUDE_DIRS}
    ${MOZJS_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
    )

if(UNIX)
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-strict-aliasing")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ")
  add_definitions(-fPIC)
endif(UNIX)

if(WIN32)
  set(LWQQ_LIST ${LWQQ_LIST} win32.c )
  add_definitions(-D_GNU_SOURCE=1)
  set(LWQQ_OTHER_LIBS ${LWQQ_OTHER_LIBS} ws2_32)
endif(WIN32)

if(SSL)
    add_definitions(-DLWQQ_ENABLE_SSL=1)
endif(SSL)

if(WITH_LIBEV)
    include_directories(${EV_INCLUDE_DIRS})
    set(LWQQ_OTHER_LIBS ${EV_LIBRARIES} ${LWQQ_OTHER_LIBS})
endif()
if(WITH_LIBUV)
    include_directories(${UV_INCLUDE_DIRS})
    set(LWQQ_OTHER_LIBS ${LWQQ_OTHER_LIBS} ${UV_LIBRARIES})
endif()

add_library(lwqq-static STATIC ${LWQQ_LIST} )
set_target_properties(lwqq-static PROPERTIES OUTPUT_NAME lwqq)
add_library(lwqq SHARED ${LWQQ_LIST} )

target_link_libraries(lwqq
    ${SQLITE_LIBRARIES}
    ${CURL_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${MOZJS_LIBRARIES}
    ${LWQQ_OTHER_LIBS}
	-Wl,-Bdynamic pthread
    )

target_link_libraries(lwqq-static
    ${SQLITE_LIBRARIES}
    ${CURL_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${MOZJS_LIBRARIES}
    ${LWQQ_OTHER_LIBS}
	-Wl,-Bdynamic pthread
    )

if(LWQQ_INSTALL)
install(TARGETS lwqq DESTINATION lib)
install(TARGETS lwqq-static DESTINATION lib)
install(FILES ${LWQQ_HEADER} DESTINATION include/lwqq)
endif()

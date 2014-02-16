project(libwebqq)
cmake_minimum_required(VERSION 2.6)
set(VERSION_MAJOR "0")
set(VERSION_MINOR "2")
set(VERSION_PATCH "4")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

add_definitions(-Wall)
add_definitions(-DINST_PREFIX="${CMAKE_INSTALL_PREFIX}")
option(SSL "Enable SSL Support " On)
option(WITHOUT_ASYNC "Fully Disable Async Ability, Only Sync mode" Off)
option(LWQQ_INSTALL "Install Lwqq Library" On)

find_package(Sphinx)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	find_package(PkgConfig REQUIRED)
    #recommand libcurl>=7.22.0
	pkg_check_modules(CURL REQUIRED libcurl)
	pkg_check_modules(SQLITE REQUIRED sqlite3)
	pkg_check_modules(ZLIB REQUIRED zlib)
    pkg_check_modules(MOZJS mozjs185)
    set(MOZJS_185 ${MOZJS_FOUND})
    if(NOT MOZJS_FOUND)
        pkg_check_modules(MOZJS mozjs-17.0)
        set(MOZJS_17 ${MOZJS_FOUND})
    endif()
	find_package(EV)
	find_package(Libuv)
endif()

if(APPLE)
	find_package(PkgConfig REQUIRED)
	find_package(CURL REQUIRED)
	find_package(Sqlite3 REQUIRED)
	find_package(ZLIB REQUIRED)
    pkg_check_modules(MOZJS mozjs185)
    set(MOZJS_185 ${MOZJS_FOUND})
    if(NOT MOZJS_FOUND)
        pkg_check_modules(MOZJS mozjs-17.0)
        set(MOZJS_17 ${MOZJS_FOUND})
    endif()
	find_package(EV)
	find_package(Libuv)
endif()

if(WIN32)
	set(CMAKE_PREFIX_PATH 
	"${WIN32_LIBS_DIR}/libcurl" 
        "${WIN32_LIBS_DIR}/libcurl/include" 
        "${WIN32_LIBS_DIR}/libcurl/lib"
        )
	find_package(CURL REQUIRED)
	set(CMAKE_PREFIX_PATH "${WIN32_LIBS_DIR}/sqlite3")
	find_package(Sqlite3 REQUIRED)
	set(CMAKE_PREFIX_PATH 
		"${WIN32_LIBS_DIR}/zlib/include" 
		"${WIN32_LIBS_DIR}/zlib/lib"
		)
	find_package(ZLIB REQUIRED)
    set(CMAKE_PREFIX_PATH 
        "${WIN32_LIBS_DIR}/libuv/include" 
        "${WIN32_LIBS_DIR}/libuv/Release" 
        "${WIN32_LIBS_DIR}/libuv/Debug")
    find_package(Libuv)
    set(CMAKE_PREFIX_PATH 
        "${WIN32_LIBS_DIR}/js-1.8.5-mingw32/include/js" 
        "${WIN32_LIBS_DIR}/js-1.8.5-mingw32/lib"
        )
    find_package(MozJS REQUIRED)
    set(MOZJS_185 ${MOZJS_FOUND})
endif(WIN32)

option(WITH_LIBEV "Use Libev To Provide Async " ${EV_FOUND})
if(WITH_LIBEV AND NOT EV_FOUND)
	message(FATAL_ERROR "Required package libev not found")
endif()
if(WIN32)
	#only windows open libuv by default
	option(WITH_LIBUV "Use Libuv To Provide Async " ${UV_FOUND})
else()
	option(WITH_LIBUV "Use Libuv To Provide Async " OFF)
endif()
if(WITH_LIBUV AND NOT UV_FOUND)
    message(FATAL_ERROR "Required package libuv not found")
endif()

if(WITH_LIBEV)
    if(WITH_LIBUV)
        message(FATAL_ERROR "Libuv and Libev shouldn't used together; Turn off one")
    endif()
    if(WIN32)
        message(FATAL_ERROR "Windows do not support Libev")
    endif()
endif()

option(WITH_MOZJS "Use Mozjs To Provide hash.js support" ${MOZJS_FOUND})
if(WITH_MOZJS AND NOT MOZJS_FOUND)
	message(FATAL_ERROR "Reuired mozjs185 or mozjs-17.0 package not found")
endif()

option(ENABLE_DOCS "Build Documentation" OFF)
if(ENABLE_DOCS AND NOT SPHINX_FOUND)
	message(FATAL_ERROR "Reuired python-sphinx package not found")
endif()

#always true because we force enable sqlite
set(WITH_SQLITE True)

message( "================lwqq flags=====================")
if(WIN32)
message(STATUS "With Libuv (Option)     : ${WITH_LIBUV}")
else()
message(STATUS "With Libev (Option)     : ${WITH_LIBEV}")
endif()
message(STATUS "With Mozjs (Option)     : ${WITH_MOZJS}")
message(STATUS "Build Document (Option) : ${ENABLE_DOCS}")
message( "===============================================")

string(COMPARE EQUAL "${CMAKE_BUILD_TYPE}" "Debug" USE_DEBUG)
set(ENABLE_SSL "${SSL}")
configure_file(config.h.in ${PROJECT_BINARY_DIR}/lwqq-config.h)
install(FILES "${PROJECT_BINARY_DIR}/lwqq-config.h" DESTINATION include/lwqq)
add_subdirectory(lib)
add_subdirectory(src)
if(ENABLE_DOCS)
    add_subdirectory(doc)
endif()

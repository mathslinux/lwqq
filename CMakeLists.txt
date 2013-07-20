project(libwebqq)
cmake_minimum_required(VERSION 2.6)
set(VERSION_MAJOR "0")
set(VERSION_MINOR "1")
set(VERSION_PATCH "f")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

add_definitions(-Wall)
add_definitions(-DINST_PREFIX="${CMAKE_INSTALL_PREFIX}")
option(SSL "Enable SSL Support " On)
option(WITHOUT_ASYNC "Disable Async " Off)
option(WITH_LIBEV "Use Libev To Provide Async " On)
option(WITH_LIBUV "Use Libuv To Provide Async " Off)
option(STATIC_LIBRARY "Compile Static Library " Off)
option(LWQQ_INSTALL "Install Lwqq Library"      On)

if(WITH_LIBEV)
    if(WITH_LIBUV)
        message(FATAL_ERROR "Libuv and Libev shouldn't used together; Turn off one")
    endif()
    if(WIN32)
        message(FATAL_ERROR "Windows do not support Libev")
    endif()
endif()

if(UNIX)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(CURL REQUIRED libcurl)
	pkg_check_modules(SQLITE REQUIRED sqlite3)
	pkg_check_modules(ZLIB REQUIRED zlib)
	#recommand libcurl>=7.22.0
    if(WITH_LIBEV)
        find_package(EV REQUIRED)
    endif()
    if(WITH_LIBUV)
        find_package(Libuv REQUIRED)
    endif()
endif(UNIX)

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
    if(WITH_LIBUV)
        set(CMAKE_PREFIX_PATH 
            "${WIN32_LIBS_DIR}/libuv/include" 
            "${WIN32_LIBS_DIR}/libuv/Release" 
            "${WIN32_LIBS_DIR}/libuv/Debug")
        find_package(Libuv REQUIRED)
    endif()
endif(WIN32)


configure_file(config.h.in ${PROJECT_BINARY_DIR}/config.h)
add_subdirectory(lib)
add_subdirectory(src)

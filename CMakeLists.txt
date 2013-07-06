project(libwebqq)
cmake_minimum_required(VERSION 2.6)
set(VERSION_MAJOR "0")
set(VERSION_MINOR "1")
set(VERSION_PATCH "f")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

find_package(PkgConfig REQUIRED)
pkg_check_modules(CURL REQUIRED libcurl)
pkg_check_modules(SQLITE REQUIRED sqlite3)
pkg_check_modules(ZLIB REQUIRED zlib)
#recommand libcurl>=7.22.0
find_package(EV REQUIRED)

add_definitions(-Wall)
add_definitions(-DINST_PREFIX="${CMAKE_INSTALL_PREFIX}")
option(SSL "Enable SSL Support " On)

#set(package pidgin-lwqq)
#set(localedir ${CMAKE_INSTALL_PREFIX}/share/locale)
#configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/src/config.h)

add_subdirectory(lib)
add_subdirectory(src)

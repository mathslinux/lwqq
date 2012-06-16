AUX_SOURCE_DIRECTORY(liblwqq/libghttp GHTTP_LIST)
AUX_SOURCE_DIRECTORY(liblwqq LWQQ_LIST)
AUX_SOURCE_DIRECTORY(pidgin-lwqq SRC_LIST)


ADD_LIBRARY(libwebqq MODULE 
    ${SRC_LIST} 
    ${LWQQ_LIST} 
    ${GHTTP_LIST}
    )
set_target_properties(libwebqq PROPERTIES PREFIX "")

INCLUDE_DIRECTORIES(
	${LIBXML2_INCLUDE_DIRS}
	${OPENSSL_INCLUDE_DIRS}
	${PURPLE_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    liblwqq
    liblwqq/libghttp
	)

TARGET_LINK_LIBRARIES(libwebqq
   					${LIBXML2_LIBRARIES}
					${OPENSSL_LIBRARIES}
					${PURPLE_LIBRARIES}
                    ${GLIB_LIBRARIES}
                    pthread
					)


SET(ICON_FILE "res/openfetion.png")
SET(RES_FILE "res/city.xml" "res/province.xml")


IF(NOT PURPLE_MAJOR_VERSION)
	SET(PURPLE_MAJOR_VERSION "2")
ENDIF(NOT PURPLE_MAJOR_VERSION)

FIND_PACKAGE(PkgConfig REQUIRED)

PKG_CHECK_MODULES(LIBXML2 REQUIRED libxml-2.0)
PKG_CHECK_MODULES(OPENSSL REQUIRED openssl)
PKG_CHECK_MODULES(PURPLE REQUIRED  purple)
PKG_CHECK_MODULES(GLIB REQUIRED  glib-2.0)

option(NLS "Native language support" ON)
if(NLS)
	find_package(Gettext)
endif(NLS)

ADD_DEFINITIONS(-Wall -Wextra -g)


EXEC_PROGRAM("whereis libpurple| sed -e 's/[^\\/]*//' -e 's/\\(.*\\)lib\\/libpurple\\.so.*/\\1/'"
			OUTPUT_VARIABLE PURPLE_PREFIX)

EXEC_PROGRAM("whereis pidgin | sed -e 's/[^\\/]*//' -e 's/\\(.*\\)bin\\/pidgin.*/\\1/'"
			OUTPUT_VARIABLE PIDGIN_PREFIX)

SET(PIDGIN_PIX_INSTALL_DIR "${PIDGIN_PREFIX}share/pixmaps/pidgin/protocols/16/")
SET(RES_INSTALL_DIR "${PURPLE_PREFIX}share/purple/openfetion/")
SET(LIB_INSTALL_DIR "${PURPLE_PREFIX}lib/purple-${PURPLE_MAJOR_VERSION}")
SET(LOCALE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/locale/")

ADD_DEFINITIONS(-DRES_DIR="${RES_INSTALL_DIR}")
if(NLS AND GETTEXT_FOUND)
	add_definitions(-DGETTEXT_PACKAGE="${TARNAME}")
	add_definitions(-DENABLE_NLS="1")
	message(STATUS "Native language support:  YES" )
else(NLS AND GETTEXT_FOUND)
	message(STATUS "Native language support:  NO" )
endif(NLS AND GETTEXT_FOUND)
add_definitions(-DLOCALE_DIR="${LOCALE_INSTALL_DIR}")

if(NLS AND GETTEXT_FOUND)
	file(GLOB POFILES "${CMAKE_CURRENT_SOURCE_DIR}/po/*.po")
	gettext_create_translations("${CMAKE_CURRENT_SOURCE_DIR}/po/pidgin-webqq.pot" ALL ${POFILES})
endif(NLS AND GETTEXT_FOUND)
INSTALL(TARGETS libwebqq DESTINATION ${LIB_INSTALL_DIR})
INSTALL(FILES ${ICON_FILE} DESTINATION ${PIDGIN_PIX_INSTALL_DIR})
INSTALL(FILES ${RES_FILE} DESTINATION ${RES_INSTALL_DIR})

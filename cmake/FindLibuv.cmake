# -*- cmake -*-

# - Find uv library (libuv)
# Find the uv includes and library
# This module defines
#  UV_INCLUDE_DIR, where to find db.h, etc.
#  UV_LIBRARIES, the libraries needed to use libuv.
#  UV_FOUND, If false, do not try to use libuv.
# also defined, but not for general use are
#  UV_LIBRARY, where to find the libuv library.

FIND_PATH(UV_INCLUDE_DIR uv.h
/usr/include
/usr/include/libuv
/usr/local/include
/usr/local/include/libuv
)

SET(UV_NAMES ${UV_NAMES} uv)
FIND_LIBRARY(UV_LIBRARY
  NAMES ${UV_NAMES}
  PATHS /usr/lib /usr/local/lib /usr/lib64 /usr/local/lib64
  )

IF (UV_LIBRARY AND UV_INCLUDE_DIR)
    SET(UV_LIBRARIES ${UV_LIBRARY})
    SET(UV_FOUND "YES")
ELSE (UV_LIBRARY AND UV_INCLUDE_DIR)
  SET(UV_FOUND "NO")
ENDIF (UV_LIBRARY AND UV_INCLUDE_DIR)


IF (UV_FOUND)
   IF (NOT UV_FIND_QUIETLY)
      MESSAGE(STATUS "Found libuv: ${UV_LIBRARIES}")
   ENDIF (NOT UV_FIND_QUIETLY)
ELSE (UV_FOUND)
   IF (UV_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find libuv library")
   ENDIF (UV_FIND_REQUIRED)
ENDIF (UV_FOUND)

# Deprecated declarations.
SET (NATIVE_UV_INCLUDE_PATH ${UV_INCLUDE_DIR} )
GET_FILENAME_COMPONENT (NATIVE_UV_LIB_PATH ${UV_LIBRARY} PATH)

SET (UV_LIBRARIES ${UV_LIBRARY})
SET (UV_INCLUDE_DIRS ${UV_INCLUDE_DIR})

MARK_AS_ADVANCED(
    UV_LIBRARIES
    UV_INCLUDE_DIRS
  )

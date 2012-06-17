#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT([lwqq], [0.1], [riegamaths@gmail.com], [http://github.com/mathslinux/lwqq])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL

# libtools
LT_INIT
AC_PROG_LIBTOOL

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_create])
# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdint.h\
          stdlib.h string.h sys/ioctl.h sys/socket.h unistd.h zlib.h])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# checks for zlib
PKG_CHECK_MODULES(ZLIB, [zlib >= 1.2], [
            AC_DEFINE(USE_ZLIB, 1, [Use ZLIB])
            AC_SUBST(ZLIB_CFLAGS)
            AC_SUBST(ZLIB_LIBS)
            ],
            )

LIBEV_CFLAGS=""
LIBEV_LIBS=""
PKG_CHECK_MODULES([LIBEV], [libev], [], [
	# no pkg-config for libev, searching manually:

		AC_CHECK_HEADERS([ev.h],[
			AC_CHECK_LIB([ev], [ev_time], [
					LIBEV_LIBS="-lev"
				],[
					AC_MSG_ERROR([libev not found])
				]
			)],[
				AC_MSG_ERROR([libev not found])
			]
		)
])

AC_SUBST([LIBEV_CFLAGS])
AC_SUBST([LIBEV_LIBS])
# checks for gtk+3.0
PKG_CHECK_MODULES(GTK, [gtk+-3.0 >= 3.0], [
            AC_DEFINE(USE_GTK, 1, [Use GTK])
            AC_SUBST(GTK_CFLAGS)
            AC_SUBST(GTK_LIBS)
            ],
            )
# checks for glib2.0
PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.18], [
            AC_DEFINE(USE_GLIB, 1, [Use GLIB])
            AC_SUBST(GLIB_CFLAGS)
            AC_SUBST(GLIB_LIBS)
            ],
            )

 checks for libsqlite3
PKG_CHECK_MODULES(SQLITE3, [sqlite3 > 3.3], [
            AC_DEFINE(USE_SQLITE3, 1, [Use sqlite3])
            AC_SUBST(SQLITE3_CFLAGS)
            AC_SUBST(SQLITE3_LIBS)
        ] , [
            echo ""
            echo "configure: error: "
            echo "libsqlite3 development headers not found."
            echo ""
            exit -1
             ])
# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
# AC_FUNC_STRNLEN
AC_CHECK_FUNCS([bzero gethostbyname memset mkdir select socket strstr strtol])

# Set build CFLAGS
CFLAGS="$CFLAGS -fno-strict-aliasing -Wall"

######################################################################
AC_ARG_ENABLE([werror],
    [AS_HELP_STRING([--enable-werror],[enable compilation abort on warning (default is no)])],
    [if test "$enableval" == "yes"; then
        CFLAGS="$CFLAGS -Werror"
    fi
    ])
######################################################################

AC_CONFIG_FILES([Makefile
        src/Makefile 
        src/liblwqq/Makefile
        src/liblwqq/libghttp/Makefile
        src/lwdb/Makefile
        src/gui/Makefile
        src/cli/Makefile
        test/Makefile
        ])
AC_OUTPUT

echo ============================== OPTIONS ==============================
echo Build CFLAGS: $CFLAGS
echo =====================================================================

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT([lwqq], [0.1], [huangcongyu2006@gmail.com], [http://github.com/mathslinux/lwqq])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror])
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
            ] ,
            )
			
# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
# AC_FUNC_STRNLEN
AC_CHECK_FUNCS([bzero gethostbyname memset mkdir select socket strstr strtol])

AC_CONFIG_FILES([Makefile
         src/Makefile 
         src/liblwqq/Makefile
         src/liblwqq/libghttp/Makefile
         src/gui/Makefile
         ])
AC_OUTPUT

echo ======== OPTIONS ============
echo =============================

AM_CFLAGS=-I$(srcdir)/../liblwqq/\
      -I$(srcdir)\
	  $(GTK_CFLAGS)\
      -Wall -Werror

bin_PROGRAMS=lwqq
lwqq_LDADD=../liblwqq/liblwqq.la ../liblwqq/libghttp/libghttp.la \
		   ../lwdb/liblwdb.la $(ZLIB_LIBS) $(GTK_LIBS)
lwqq_SOURCES=main.c mainwindow.c

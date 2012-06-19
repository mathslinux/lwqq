AM_CFLAGS=-I$(srcdir)/../liblwqq -I$(srcdir)/../lwdb -I$(srcdir) $(GTK_CFLAGS)

bin_PROGRAMS=lwqq
lwqq_LDADD=../liblwqq/liblwqq.la ../liblwqq/libghttp/libghttp.la \
		   ../lwdb/liblwdb.la $(ZLIB_LIBS) $(GTK_LIBS)
lwqq_SOURCES=main.c mainwindow.c loginpanel.c

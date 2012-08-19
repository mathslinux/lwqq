AM_CFLAGS=-I$(srcdir)/../include/lwqq -I$(srcdir)

bin_PROGRAMS=lwqq-cli
lwqq_cli_LDADD=../liblwqq/liblwqq.la ../liblwqq/libghttp/libghttp.la \
		   $(ZLIB_LIBS) $(LIBEV_LIBS)
lwqq_cli_SOURCES=cli.c

AM_CFLAGS=-I$(srcdir)/../liblwqq -I$(srcdir)/../lwdb -I$(srcdir) \
	$(GTK_CFLAGS) -DLWQQ_INSTALL_DIR=\"$(pkgdatadir)\"

bin_PROGRAMS=lwqq
lwqq_LDADD=../liblwqq/liblwqq.la ../liblwqq/libghttp/libghttp.la \
		   ../lwdb/liblwdb.la $(ZLIB_LIBS) $(GTK_LIBS) $(LIBEV_LIBS)
lwqq_SOURCES=main.c mainwindow.c loginpanel.c splashpanel.c mainpanel.c \
		   statusbutton.c buddytree.c buddylist.c msgloop.c tray.c \
           chatwindow.c chatwidget.c chattextview.c

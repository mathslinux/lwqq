SUBDIRS=libghttp
lib_LTLIBRARIES=liblwqq.la
liblwqq_la_CPPFLAGS=-I./libghttp -I../include/lwqq ${LIBEV_CFLAGS} ${CURL_CFLAGS}
liblwqq_la_LIBADD= 
liblwqq_la_SOURCES=logger.c login.c type.c smemory.c\
			http.c md5.c url.c json.c info.c msg.c \
			unicode.c http_curl.c async.c
AM_CFLAGS = 


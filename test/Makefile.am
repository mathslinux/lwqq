AM_CFLAGS= -g -I${srcdir}/../src/liblwqq -I${srcdir}/../src/liblwqq/libghttp \
		-I${srcdir}/../src/lwdb
noinst_PROGRAMS=testlogin testlwdb testlogin_async

testlogin_SOURCES=test_login.c
testlogin_LDADD=../src/liblwqq/liblwqq.la \
		../src/liblwqq/libghttp/libghttp.la $(ZLIB_LIBS)

testlogin_async_SOURCES=test_login_async.c
testlogin_async_LDADD=../src/liblwqq/liblwqq.la \
		../src/liblwqq/libghttp/libghttp.la $(ZLIB_LIBS)

testlwdb_SOURCES= test_lwdb.c
testlwdb_LDADD=../src/liblwqq/liblwqq.la \
		../src/liblwqq/libghttp/libghttp.la $(ZLIB_LIBS) \
		../src/lwdb/liblwdb.la

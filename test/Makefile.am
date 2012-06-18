AM_CFLAGS=-I${srcdir}/../src/liblwqq -I${srcdir}/../src/liblwqq/libghttp \
		-I${srcdir}/../src/lwdb
noinst_PROGRAMS=testlogin testlwdb testsendmsg

testlogin_SOURCES=test_login.c
testlogin_LDADD=../src/liblwqq/liblwqq.la \
		../src/liblwqq/libghttp/libghttp.la $(ZLIB_LIBS)

testsendmsg_SOURCES=test_sendmsg.c
testsendmsg_LDADD=../src/liblwqq/liblwqq.la \
		../src/liblwqq/libghttp/libghttp.la $(ZLIB_LIBS)

testlwdb_SOURCES= test_lwdb.c
testlwdb_LDADD=../src/liblwqq/liblwqq.la \
		../src/liblwqq/libghttp/libghttp.la $(ZLIB_LIBS) \
		../src/lwdb/liblwdb.la

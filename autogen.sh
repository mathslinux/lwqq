#!/bin/bash
set -x
aclocal 					\
	&& autoconf					\
	&& libtoolize 	--copy --force --automake 	\
	&& autoreconf --install				\
	&& ./configure --prefix=/usr --enable-werror

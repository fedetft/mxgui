#!/usr/bin/env bash

## This script is meant to be run by cmake, don't run it manually
## It builds freetype 2.4.3

if [ ! -e freetype-2.4.3/objs/.libs/libfreetype.a ]; then
	echo "Building freetype 2.4.3"
	tar xjvf ../libs/freetype-2.4.3.tar.bz2
	cd freetype-2.4.3
	./configure --enable-static --disable-shared || exit 1
	# On MINGW2, *only when this script is invoked by cmake*,
	# for some reason make does not create the .libs directory,
	# which causes the build to fail at the last step.
	# Work around it by creating the directory now.
	# (also WTF is happening!?)
	mkdir -p objs/.libs
	make || exit 1
	# We want to forcefully link statically, and a possible way is to remove
	# the dynamic libraries
	cd objs/.libs
	rm -f libfreetype.so libfreetype.la libfreetype.lai
fi

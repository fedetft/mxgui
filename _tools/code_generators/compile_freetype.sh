## This script is meant to be run by cmake, don't run it manually
## It builds freetype 2.4.3

if [ ! -d freetype-2.4.3 ]; then
	echo "Building freetype 2.4.3"
	tar xjvf ../libs/freetype-2.4.3.tar.bz2
	cd freetype-2.4.3
	./configure
	make
	# We want to forcefully link statically, and a possible way is to remove
	# the dynamic libraries
	cd objs/.libs
	rm libfreetype.so libfreetype.la libfreetype.lai
fi

#!/bin/bash

## Designed to be called from the build directory
if [[ ! `pwd | grep build` ]]; then
	echo "Error, script must be run from the build directory"
	exit
fi

# Name and path of fontrendering tool
BIN=./fontrendering

# Output directory, to store processed fonts
OUT=../../../fonts

# Input directory, where TTF/BDF fonts are stored
B=../../font_collection

# Generate glyphs only for ASCII
RANGES="--add-range 0x20,0x7e"
#RANGES="--add-range 0x20,0x7e --add-range 0xa1,0xff --add-range 0x100,0x17f --add-range 0x180,0x24f --add-range 0x400,0x4ff"

rm -rf $OUT
mkdir $OUT

$BIN --font=$B/tahoma/tahoma-11.bdf --image=$OUT/tahoma.png \
	--header=$OUT/tahoma.h --name=tahoma #FIXME: some ranges don't work with tahoma

$BIN --font=$B/miscfixed/9x15.bdf --image=$OUT/miscfixed.png \
	--header=$OUT/miscfixed.h --name=miscfixed $RANGES

$BIN --font=$B/miscfixed/8x13B.bdf --image=$OUT/miscfixed_bold.png \
	--header=$OUT/miscfixed_bold.h --name=miscfixedBold $RANGES

$BIN --font=$B/droid/DroidSans.ttf --image=$OUT/droid11.png \
	--header=$OUT/droid11.h --name=droid11 --height=11 \
	--fixes=$B/droid/DroidSans11.fixes $RANGES

$BIN --font=$B/droid/DroidSans-Bold.ttf --image=$OUT/droid11b.png \
	--header=$OUT/droid11b.h --name=droid11b --height=11 \
	--fixes=$B/droid/DroidSansBold11.fixes $RANGES

$BIN --font=$B/droid/DroidSans.ttf --image=$OUT/droid21.png \
	--header=$OUT/droid21.h --name=droid21 --height=20 --pad=1 $RANGES

$BIN --font=$B/droid/DroidSans-Bold.ttf --image=$OUT/droid21b.png \
	--header=$OUT/droid21b.h --name=droid21b --height=20 --pad=1 $RANGES

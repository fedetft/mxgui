#!/bin/bash

## Designed to be called from the build directory
if [[ ! `pwd | grep build` ]]; then
	echo "Error, script must be run from the build directory"
	exit
fi

# Name and path of fontrendering tool
export BIN=./fontrendering

# Output directory, to store processed fonts
export OUT=../../../fonts

# Input directory, where TTF/BDF fonts are stored
export B=../../font_collection

rm -rf $OUT
mkdir $OUT

$BIN --font=$B/tahoma/tahoma-11.bdf --image=$OUT/tahoma.png \
	--header=$OUT/tahoma.h --name=tahoma

$BIN --font=$B/miscfixed/9x15.bdf --image=$OUT/miscfixed.png \
	--header=$OUT/miscfixed.h --name=miscfixed

$BIN --font=$B/miscfixed/8x13B.bdf --image=$OUT/miscfixed_bold.png \
	--header=$OUT/miscfixed_bold.h --name=miscfixedBold

$BIN --font=$B/droid/DroidSans.ttf --image=$OUT/droid11.png \
	--header=$OUT/droid11.h --name=droid11 --height=11 \
	--fixes=$B/droid/DroidSans11.fixes

$BIN --font=$B/droid/DroidSans-Bold.ttf --image=$OUT/droid11b.png \
	--header=$OUT/droid11b.h --name=droid11b --height=11 \
	--fixes=$B/droid/DroidSansBold11.fixes

$BIN --font=$B/droid/DroidSans.ttf --image=$OUT/droid21.png \
	--header=$OUT/droid21.h --name=droid21 --height=20 --pad=1

$BIN --font=$B/droid/DroidSans-Bold.ttf --image=$OUT/droid21b.png \
	--header=$OUT/droid21b.h --name=droid21b --height=20 --pad=1

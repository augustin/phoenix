#!/bin/sh
######
## Bootstrap utility for Phoenix
######

CXX=${CXX-g++}
LD=${LD-$CXX}
CXXFLAGS="-std=c++11 -Wall -O2 $CXXFLAGS"
LINKFLAGS="-O1 $LINKFLAGS"

run_command()
{
	echo "$1" && $1
	if [ $? -ne 0 ]; then exit 1; fi
}

compile()
{
	SOURCES=$1
	LOCAL_CXXFLAGS="$CXXFLAGS $2"
	LINKCMD="$LD $3 $LINKFLAGS"

	for file in $SOURCES; do
		outfile=build-bootstrap/$(basename $file).o
		LINKCMD="$LINKCMD $outfile"
		if [ ! -f $outfile ] || [ src/$file -nt $outfile ]; then
			run_command "$CXX src/$file -c -o $outfile $LOCAL_CXXFLAGS"
		fi
	done

	run_command "$LINKCMD"
}

# Create build directory
mkdir -p build-bootstrap

# Build phoenix_bootstrapped
PHOENIX_SRCS="
	Phoenix.cpp
	generators/Generators.cpp
	util/FSUtil.cpp
	util/StringUtil.cpp
	"
compile "$PHOENIX_SRCS" "-iquotesrc" "-o phoenix_bootstrapped"

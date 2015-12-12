#!/bin/sh
######
## Bootstrap utility for Phoenix
######

CXX=${CXX-g++}
LD=${LD-$CXX}
CXXFLAGS="-std=c++0x -Wall -O2 $CXXFLAGS"
LINKFLAGS="$LINKFLAGS"

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

if [ ! -f "Phoenixfile.phnx" ]; then
	echo "Please run this script from the root of the repository."
	exit 1
fi

# Create build directory
mkdir -p build-bootstrap

# Build phoenix_bootstrapped
PHOENIX_SRCS="
	Phoenix.cpp
	build/Generators.cpp
	build/LanguageInfo.cpp
	build/Target.cpp
	language/Function.cpp
	language/GlobalLanguageObject.cpp
	language/Object.cpp
	language/ObjectMap.cpp
	language/Parser.cpp
	language/Stack.cpp
	util/FSUtil.cpp
	util/StringUtil.cpp
	"
compile "$PHOENIX_SRCS" "-iquotesrc" "-o phoenix_bootstrapped"

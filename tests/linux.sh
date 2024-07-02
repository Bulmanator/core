#!/bin/bash

pushd "$(dirname $0)" > /dev/null

mkdir "../build" > /dev/null 2> /dev/null
pushd "../build" > /dev/null

mkdir "c"   > /dev/null 2> /dev/null
mkdir "cpp" > /dev/null 2> /dev/null

COMPILER_OPTS="-O0 -g -ggdb -Wall -I.. -Wno-format -Wno-unused-function -Wno-missing-braces"
LINKER_OPTS=""

echo "[building core.h tests]"

gcc   $COMPILER_OPTS -Wunused-function "../tests/core.c" -o "c/core_gcc"   $LINKER_OPTS
clang $COMPILER_OPTS -Wunused-function "../tests/core.c" -o "c/core_clang" $LINKER_OPTS

g++     $COMPILER_OPTS -Wunused-function "../tests/core.cpp" -o "cpp/core_gcc"   $LINKER_OPTS
clang++ $COMPILER_OPTS -Wunused-function "../tests/core.cpp" -o "cpp/core_clang" $LINKER_OPTS

echo "[building png.h tests]"

gcc   $COMPILER_OPTS "../tests/png.c" -o "c/png_gcc"   $LINKER_OPTS
clang $COMPILER_OPTS "../tests/png.c" -o "c/png_clang" $LINKER_OPTS

g++     $COMPILER_OPTS "../tests/png.cpp" -o "cpp/png_gcc"   $LINKER_OPTS
clang++ $COMPILER_OPTS "../tests/png.cpp" -o "cpp/png_clang" $LINKER_OPTS

popd > /dev/null
popd > /dev/null

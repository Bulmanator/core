#!/bin/bash

pushd "$(dirname $0)" > /dev/null

if [[ ! -d "../build" ]];
then
    mkdir "../build"
fi

pushd "../build" > /dev/null

COMPILER_OPTS="-O0 -g -ggdb -Wall -I.. -Wno-format"
LINKER_OPTS=""

gcc $COMPILER_OPTS "../tests/core.c"   -o "core_c_gcc"   $LINKER_OPTS
g++ $COMPILER_OPTS "../tests/core.cpp" -o "core_cpp_gcc" $LINKER_OPTS

clang   $COMPILER_OPTS "../tests/core.c"   -o "core_c_clang"   $LINKER_OPTS
clang++ $COMPILER_OPTS "../tests/core.cpp" -o "core_cpp_clang" $LINKER_OPTS

popd > /dev/null
popd > /dev/null

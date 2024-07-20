#!/bin/bash

pushd "$(dirname $0)" > /dev/null

# We can add more arguments later
for n in $*
do
    case $n in
    clean)
        echo "[cleaning build directory]"
        rm -rf "../build" && exit 1
    ;;
    esac
done

mkdir "../build" > /dev/null 2> /dev/null
pushd "../build" > /dev/null

mkdir -p "c/linux"   > /dev/null 2> /dev/null
mkdir -p "cpp/linux" > /dev/null 2> /dev/null

COMPILER_OPTS="-O0 -g -ggdb -Wall -I.. -Wno-format -Wno-unused-function -Wno-missing-braces"
LINKER_OPTS=""

echo "[building core.h tests]"

gcc   $COMPILER_OPTS -Wunused-function "../tests/core.c" -o "c/linux/core_gcc"   $LINKER_OPTS
clang $COMPILER_OPTS -Wunused-function "../tests/core.c" -o "c/linux/core_clang" $LINKER_OPTS

gcc   $COMPILER_OPTS -x c++ -Wunused-function "../tests/core.c" -o "cpp/linux/core_gcc"   $LINKER_OPTS
clang $COMPILER_OPTS -x c++ -Wunused-function "../tests/core.c" -o "cpp/linux/core_clang" $LINKER_OPTS

echo "[building png.h tests]"

gcc   $COMPILER_OPTS "../tests/png.c" -o "c/linux/png_gcc"   $LINKER_OPTS
clang $COMPILER_OPTS "../tests/png.c" -o "c/linux/png_clang" $LINKER_OPTS

gcc   $COMPILER_OPTS -x c++ "../tests/png.c" -o "cpp/linux/png_gcc"   $LINKER_OPTS
clang $COMPILER_OPTS -x c++ "../tests/png.c" -o "cpp/linux/png_clang" $LINKER_OPTS

popd > /dev/null
popd > /dev/null

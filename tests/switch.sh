#!/bin/bash

DKP=$(echo "$DEVKITPRO" | sed -e 's/^\([a-zA-Z]\):/\/\1/')

if [[ -z "$DKP" ]];
then
    echo "[error] '\$DEVKITPRO' enviroment variable missing, please install or configure devkitpro"
    exit 1
fi

echo "[error] :: switch is currently unsupported" && exit 1

if [[ ! -d "$DKP/devkitA64" ]];
then
    echo "[error] devkitA64 is required to build for switch"
    exit 1
fi

pushd "$(dirname $0)" > /dev/null

mkdir "../build" > /dev/null 2> /dev/null
pushd "../build" > /dev/null

mkdir -p "c/switch"   > /dev/null 2> /dev/null
mkdir -p "cpp/switch" > /dev/null 2> /dev/null

CC="$DKP/devkitA64/bin/aarch64-none-elf-gcc"

INCLUDE="-I$DKP/libnx/include -I$DKP/portlibs/switch/include"
LIB="-L$DKP/portlibs/switch/lib -L$DKP/libnx/lib"

# @todo: sort some of these arguments out probably don't need -MMD -MP -MF or
# -Wl,-Map options as they are Makefile oriented
#
ARCH="-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE"

C_OPTS="-g -Wall -Wno-format -O2 -ffunction-sections $ARCH $INCLUDE -D__SWITCH__ -MMD -MP -MF core.d"
CXX_OPTS="$C_OPTS -x c++ -std=gnu++17 -fno-exceptions -fno-rtti"

LINKER_OPTS= "-specs=$DKP/libnx/switch.specs -g $ARCH -Wl,-Map,core.map $LIB -lnx"

$CC $C_OPTS   -I".." -c "../tests/core.c" -o "c/switch/core.o"
$CC $CXX_OPTS -I".." -c "../tests/core.c" -o "cpp/switch/core.o"

$CC $LINKER_OPTS "c/switch/core.o"   -o "c/switch/core.elf"
$CC $LINKER_OPTS "cpp/switch/core.o" -o "cpp/switch/core.elf"

nacptool --create core_tests_c   'bulmanator' 1.0.0 "c/switch/core.nacp"
nacptool --create core_tests_cxx 'bulmanator' 1.0.0 "cpp/switch/core.nacp"

elf2nro "c/switch/core.elf"   "c/switch/core.nro"   --icon="$DKP/libnx/default_icon.jpg" --nacp="c/switch/core.nacp"
elf2nro "cpp/switch/core.elf" "cpp/switch/core.nro" --icon="$DKP/libnx/default_icon.jpg" --nacp="cpp/switch/core.nacp"

popd > /dev/null
popd > /dev/null

#!/bin/bash

DKP=$(echo "$DEVKITPRO" | sed -e 's/^\([a-zA-Z]\):/\/\1/')

if [[ -z "$DKP" ]];
then
    echo "[error] '\$DEVKITPRO' enviroment variable missing, please install or configure devkitpro"
    exit 1
fi

if [[ ! -d "$DKP/devkitA64" ]];
then
    echo "[error] devkitA64 is required to build for switch"
    exit 1
fi

pushd "$(dirname $0)" > /dev/null

if [[ ! -d "../build" ]];
then
    mkdir "../build"
fi

pushd "../build" > /dev/null

CC="$DKP/devkitA64/bin/aarch64-none-elf-gcc"
CXX="$DKP/devkitA64/bin/aarch64-none-elf-g++"

INCLUDE="-I$DKP/libnx/include -I$DKP/portlibs/switch/include"
LIB="-L$DKP/portlibs/switch/lib -L$DKP/libnx/lib"

# @todo: sort some of these arguments out probably don't need -MMD -MP -MF or -Wl,-Map options
#
ARCH="-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE"
C_OPTS="-g -Wall -Wno-format -O2 -ffunction-sections $ARCH $INCLUDE -D__SWITCH__ -MMD -MP -MF core.d"
CXX_OPTS="$C_OPTS -std=gnu++17 -fno-exceptions -fno-rtti"
LINKER_OPTS="-specs=$DKP/libnx/switch.specs -g $ARCH -Wl,-Map,core.map $LIB -lnx"

$CC $C_OPTS -I".." -c "../tests/core.c" -o "core_c.o"
$CXX $CXX_OPTS -I".." -c "../tests/core.cpp" -o "core_cpp.o"

$CC $LINKER_OPTS "core_c.o"   -o "core_c.elf"
$CC $LINKER_OPTS "core_cpp.o" -o "core_cpp.elf"

nacptool --create core 'bulmanator' 1.0.0 core.nacp

elf2nro core_c.elf   core_c.nro   --icon="$DKP/libnx/default_icon.jpg" --nacp="core.nacp"
elf2nro core_cpp.elf core_cpp.nro --icon="$DKP/libnx/default_icon.jpg" --nacp="core.nacp"

popd > /dev/null
popd > /dev/null

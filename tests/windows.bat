@echo off

pushd "%~dp0" > NUL

REM @todo: do argument processing

mkdir "..\build" > NUL 2> NUL
pushd "..\build" > NUL

mkdir "c\windows"   > NUL 2> NUL
mkdir "cpp\windows" > NUL 2> NUL

set cl_options=-nologo -Od -Zi -W4 -WX -wd4127 -I".."
set link_options=-subsystem:console

REM build core tests
REM
echo [msvc]

echo [building core]
cl %cl_options% -TC "..\tests\core.c" -Fe"c/windows/core_msvc.exe"   -link %link_options%
cl %cl_options% -TP "..\tests\core.c" -Fe"cpp/windows/core_msvc.exe" -link %link_options%

echo [building png]
cl %cl_options% -TC -wd4505 "..\tests\png.c" -Fe"c/windows/png_msvc.exe"   -link %link_options%
cl %cl_options% -TP -wd4505 "..\tests\png.c" -Fe"cpp/windows/png_msvc.exe" -link %link_options%

if exist where clang (
    set clang_options=-O0 -gcodeview -Wall -I".."
    set clang_link_options=

    echo [clang]

    echo [building core]
    clang %clang_options%        "..\tests\core.c" -o "c/windows/core_clang.exe"   %clang_link_options%
    clang %clang_options% -x c++ "..\tests\core.c" -o "cpp/windows/core_clang.exe" %clang_link_options%

    echo [building png]
    clang %clang_options%        -Wno-unused-function "..\tests\png.c" -o "c/windows/png_clang.exe"   %clang_link_options%
    clang %clang_options% -x c++ -Wno-unused-function "..\tests\png.c" -o "cpp/windows/png_clang.exe" %clang_link_options%
)

popd > NUL
popd > NUL

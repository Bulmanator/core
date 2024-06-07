@echo off

pushd "%~dp0" > NUL

if not exist "..\build" (mkdir "..\build")

pushd "..\build" > NUL

set cl_options=-nologo -Od -Zi -W4 -WX -wd4127 -I".."
set link_options=-subsystem:console

REM build core tests
REM
cl %cl_options% "..\tests\core.c"   -Fe"core_c_msvc.exe"   -link %link_options%
cl %cl_options% "..\tests\core.cpp" -Fe"core_cpp_msvc.exe" -link %link_options%

if exist where clang (
    set clang_options=-O0 -gcodeview -Wall -I".."
    set clang_link_options=

    clang   %clang_options% "..\tests\core.c"   -o "core_c_clang.exe"   %clang_link_options%
    clang++ %clang_options% "..\tests\core.cpp" -o "core_cpp_clang.exe" %clang_link_options%
)

popd > NUL
popd > NUL

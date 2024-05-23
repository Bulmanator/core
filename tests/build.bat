@echo off

pushd "%~dp0" > NUL

if not exist "..\build" (mkdir "..\build")

pushd "..\build" > NUL

set cl_options=-nologo -Od -Zi -W4 -WX -I".."
set link_options=-subsystem:console

REM build core tests
REM
cl %cl_options% "..\tests\core.c"   -Fe"core_c.exe"   -link %link_options%
cl %cl_options% "..\tests\core.cpp" -Fe"core_cpp.exe" -link %link_options%

popd > NUL
popd > NUL

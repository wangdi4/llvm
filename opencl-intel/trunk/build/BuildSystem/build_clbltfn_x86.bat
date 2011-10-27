%~d0
set CUR_DIR=%~dp0

del devenv.log

if "%LIBPATH%"=="" (call "%VS90COMNTOOLS%..\..\VC\vcvarsall.bat")
@echo on

md logs
md logs\add
md logs\add\build

set BUILD_TARGET=%2

cd %1
call devenv clbltfn.sln /Rebuild "%BUILD_TARGET%|Win32" /Out "%CUR_DIR%logs\add\build\%BUILD_TARGET%_clbltfn_build_x86.log"
cd %CUR_DIR%

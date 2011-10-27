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
call devenv LLVM.sln /Rebuild "%BUILD_TARGET%|x64" /Out "%CUR_DIR%logs\add\build\%BUILD_TARGET%_icsc_build_x64.log"
cd %CUR_DIR%

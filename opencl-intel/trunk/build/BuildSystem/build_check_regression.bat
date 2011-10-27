%~d0
set CUR_DIR=%~dp0

del devenv.log

if "%LIBPATH%"=="" (call "%VS90COMNTOOLS%..\..\VC\vcvarsall.bat")
@echo on

md logs
md logs\add
md logs\add\build

set BUILD_TARGET=%2

cd %1\bin\%BUILD_TARGET%
set PATH=%PATH%;C:\icsc_cygwin\bin;C:\cygwin\bin;%cd%
call devenv ..\..\LLVM.sln /build %BUILD_TARGET% /project check_regression
cd %CUR_DIR%

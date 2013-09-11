REM @ECHO OFF

set LIBRARY_NAME=%1

:loop
shift
if not "%1"=="" (
	set OBJS=%OBJS% %1
	goto :loop
)

set AR="C:\Program Files"\Intel\MPSS\x86_64-mpsssdk-linux\usr\bin\k1om-mpss-linux\k1om-mpss-linux-ar.exe
if not exist %AR% (
    set AR="C:\Program Files"\Intel\MPSS\sdk\linux-k1om-4.7\bin\x86_64-k1om-linux-ar.exe
)

REM All variables in the form of CMake vars will be replaced by CMake
call "@ICC_MIC_ENV_SCRIPT@" intel64 > nul

echo %AR% rcs "%LIBRARY_NAME%" %OBJS%
%AR% rcs "%LIBRARY_NAME%" %OBJS%

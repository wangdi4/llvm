@echo off
rem static lib build by ICC linker
rem All variables in the form of CMake vars will be replaced by CMake:
rem   ICC_CPU_ENV_SCRIPT_NATIVE - script setting the ICC environment
rem   ICC_LD_NATIVE             - executable of ICC linker
rem Invocation parameters:
rem     %1 - build type
rem     %2 - target DLL
rem     %3 - list of object files

:cont
call "@ICC_CPU_ENV_SCRIPT_NATIVE@" @ICC_PLATFORM@ >NUL
if "%1" == "Debug" goto debug

:release
if not "%1" == "Release" goto fail
"@ICC_CPU_LD_NATIVE@" /nologo /OUT:%2 %~3
goto fin

:debug
"@ICC_CPU_LD_NATIVE@" /nologo /OUT:%2 %~3

:fin
exit /b 0

:fail
echo Unsupported build configuration %1!
exit /b 1

@echo off
rem Compilation of a C file by ICC Compiler
rem All variables in the form of CMake vars will be replaced by CMake:
rem     ICC_CPU_ENV_SCRIPT_NATIVE - script setting the ICC environment
rem     ICC_CPU_CL_NATIVE         - executable of ICC compiler
rem     BUILD_TYPE_ICC_COMPILER_FLAGS_Debug   - Debug build specific compliler flags 
rem		BUILD_TYPE_ICC_COMPILER_FLAGS_Release - Release build specific compliler flags 
rem Invocation parameters:
rem     %1 - build type
rem     %2 - source file
rem     %3 - object file

call "@ICC_CPU_ENV_SCRIPT_NATIVE@" @ICC_PLATFORM@ > NUL
if "%1" == "Debug" goto debug

:release
if not "%1" == "Release" goto fail
"@ICC_CPU_CL_NATIVE@" /nologo /c /I /../cl_api /DWIN32 /D_WINDOWS /D_USRDLL /DCL_BUILTIN_FUNCTIONS_EXPORTS /D_WINDLL /D_UNICODE /DUNICODE /EHsc /GS /fp:source /fp:strict /Qlong-double /Qpc80 /Qprec-div /W3 /Zi @BUILD_TYPE_ICC_COMPILER_FLAGS_Release@ /Fo%3 %2
goto fin

:debug
"@ICC_CPU_CL_NATIVE@"  /nologo /c /I /../cl_api /DWIN32 /D_WINDOWS /D_USRDLL /DCL_BUILTIN_FUNCTIONS_EXPORTS /D_WINDLL /D_UNICODE /DUNICODE /EHsc /GS /fp:source /fp:strict /Qlong-double /Qpc80 /Qprec-div /W3 /Zi @BUILD_TYPE_ICC_COMPILER_FLAGS_Debug@ /Fo%3 %2

:fin
exit /b 0

:fail
echo Unsupported build configuration %1!
exit /b 1

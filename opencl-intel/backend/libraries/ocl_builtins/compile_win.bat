@echo off
rem Compilation of a C file by ICC Compiler
rem All variables in the form of CMake vars will be replaced by CMake:
rem     ICC_ENV_SCRIPT_NATIVE - script setting the ICC environment
rem     ICC_PLATFORM 		      - parameter to the script above
rem 	  ICC_CL_NATIVE         - executable of ICC compiler
rem 	  ICC_COMPILER_PARAMS   - flags for compilation
rem     BUILD_TYPE_ICC_COMPILER_FLAGS_Debug   - Debug build specific compliler flags 
rem			BUILD_TYPE_ICC_COMPILER_FLAGS_Release - Release build specific compliler flags 
rem Invocation parameters:
rem     %1 - build type
rem     %2 - source file
rem     %3 - object file
rem     %4 - cpp definition for target SIMD arch
rem     %5 - additional compilation option(s)

call "@ICC_ENV_SCRIPT_NATIVE@" @ICC_PLATFORM@ > NUL
if "%1" == "Debug" goto debug

:release
if not "%1" == "Release" goto fail
"@ICC_CL_NATIVE@" @ICC_COMPILER_PARAMS@ @BUILD_TYPE_ICC_COMPILER_FLAGS_Release@ /D%4 %5 /Fo%3 %2
goto fin

:debug
"@ICC_CL_NATIVE@" @ICC_COMPILER_PARAMS@ @BUILD_TYPE_ICC_COMPILER_FLAGS_Debug@ /D%4 %5 /Fo%3 %2

:fin
exit /b 0

:fail
echo Unsupported build configuration %1!
exit /b 1

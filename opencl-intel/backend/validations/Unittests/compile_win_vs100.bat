@echo off
echo "@VS100_CPU_CL_NATIVE@"
rem Compilation of a C file by VS100 Compiler
rem All variables in the form of CMake vars will be replaced by CMake:
rem     VS100_CPU_ENV_SCRIPT_NATIVE - script setting the VS100 environment
rem     VS100_CPU_CL_NATIVE         - executable of VS100 compiler
rem     BUILD_TYPE_ICC_COMPILER_FLAGS_Debug   - Debug build specific compliler flags 
rem		BUILD_TYPE_ICC_COMPILER_FLAGS_Release - Release build specific compliler flags 
rem Invocation parameters:
rem     %1 - build type
rem     %2 - source file
rem     %3 - object file

call "@VS100_CPU_ENV_SCRIPT_NATIVE@"
if "%1" == "Debug" goto debug

:release
if not "%1" == "Release" goto fail
"@VS100_CPU_CL_NATIVE@" /nologo /c /I ../../plugin_manager /I ../../llvm/utils/unittest/googletest/include /I ../Common /I ../OCLRecorder /I ../Comparator /I ../../dynamic_lib /I ../OCLBuiltins /I ../OCLBuilder /I ../../ocl_cpu_backend/export /I ../../../cl_api /I ../../llvm/include /DWIN32 /D_WINDOWS /D_UNICODE /DUNICODE /EHsc /GS /W3 /Zi @BUILD_TYPE_ICC_COMPILER_FLAGS_Release@ /Fo%3 %2 >nul
goto fin                                                                                                                                                                                                                                                   
                                                                                                                                                                                                                                                           
:debug
"@VS100_CPU_CL_NATIVE@" /nologo /c /I ../../plugin_manager /I ../../llvm/utils/unittest/googletest/include /I ../Common /I ../OCLRecorder /I ../Comparator /I ../../dynamic_lib /I ../OCLBuiltins /I ../OCLBuilder /I ../../ocl_cpu_backend/export /I ../../../cl_api /I ../../llvm/include /DWIN32 /D_WINDOWS /D_UNICODE /DUNICODE /EHsc /GS /W3 /Zi @BUILD_TYPE_ICC_COMPILER_FLAGS_Debug@ /Fo%3 %2 >nul

:fin
exit /b 0

:fail
echo Unsupported build configuration %1!
exit /b 1
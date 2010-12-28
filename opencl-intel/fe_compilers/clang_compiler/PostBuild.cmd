set PLATFORM_NAME=%1
set CONFIG_NAME=%2

REM -------- export OCL FE compiler header files ------------
mkdir %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\fe_include
xcopy .\export\clang_include /Y /S %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\fe_include

REM -------- Build precompiled headers ------------
%MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\clc.exe -cc1 -x cl %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\fe_include\opencl_.h -relocatable-pch -isysroot %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME% -emit-pch -o %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\opencl_.pch
@if not ERRORLEVEL 1 goto optim
@echo fe_include\opencl_.h(0): error: can't create Precompiled Header
exit /b 1

:optim
REM -------- Build optimized precompiled headers ------------
%MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\clc.exe -cc1 -x cl %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\fe_include\opencl_.h -relocatable-pch -isysroot %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME% -O3 -emit-pch -o %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\opencl_opt_.pch
@if not ERRORLEVEL 1 goto end
@echo fe_include\opencl_.h(0): error: can't create Optimized Precompiled Header
exit /b 1

:end

set PLATFORM_NAME=%1
set CONFIG_NAME=%2
set TARGET_NAME=%3
set DEST_DIR_SUFIX=%4

REM -------- export OCL FE compiler header files ------------
mkdir %MTV_LOCAL_BIN_DIR%\fe_include
xcopy .\export\clang_include /Y /S %MTV_LOCAL_BIN_DIR%\fe_include >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
mkdir %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX%\fe_include
xcopy .\export\clang_include /Y /S %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX%\fe_include >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- copy .lib file ------------
del /f %MTV_LOCAL_LIB_DIR%\%TARGET_NAME%.lib > %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.lib %MTV_LOCAL_LIB_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

del /f %MTV_LOCAL_LIB_DIR%.%DEST_DIR_SUFIX%\%TARGET_NAME%.lib >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.lib %MTV_LOCAL_LIB_DIR%.%DEST_DIR_SUFIX% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- copy .bin file ------------
del /f %MTV_LOCAL_BIN_DIR%\%TARGET_NAME%.dll > %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.dll %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

del /f %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX%\%TARGET_NAME%.dll >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.dll %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- copy .pdb file ------------
del /f %MTV_LOCAL_BIN_DIR%\%TARGET_NAME%.pdb >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.pdb %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

del /f %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX%\%TARGET_NAME%.pdb >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.pdb %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- Build precompiled headers ------------
%MTV_LOCAL_BIN_DIR%\clc.exe -cc1 -x cl %MTV_LOCAL_BIN_DIR%\fe_include\opencl_.h -relocatable-pch -isysroot %MTV_LOCAL_BIN_DIR% -emit-pch -o %MTV_LOCAL_BIN_DIR%\opencl_.pch
@if not ERRORLEVEL 1 goto optim
@echo fe_include\opencl_.h(0): error: can't create Precompiled Header
exit /b 1

:optim
REM -------- Build optimized precompiled headers ------------
%MTV_LOCAL_BIN_DIR%\clc.exe -cc1 -x cl %MTV_LOCAL_BIN_DIR%\fe_include\opencl_.h -relocatable-pch -isysroot %MTV_LOCAL_BIN_DIR% -O3 -emit-pch -o %MTV_LOCAL_BIN_DIR%\opencl_opt_.pch
@if not ERRORLEVEL 1 goto final
@echo fe_include\opencl_.h(0): error: can't create Optimized Precompiled Header
exit /b 1

:final
@copy %MTV_LOCAL_BIN_DIR%\opencl_.pch %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX%\
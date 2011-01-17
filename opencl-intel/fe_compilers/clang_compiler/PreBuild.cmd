set PLATFORM_NAME=%1
set CONFIG_NAME=%2

REM -------- Build precompiled headers ------------
%MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\clc.exe -cc1 -x cl -emit-pch -o opencl_.pch < export\clang_include\opencl_.h
@if not ERRORLEVEL 1 goto end
@echo fe_include\opencl_.h(0): error: can't create Precompiled Header
exit /b 1

:end
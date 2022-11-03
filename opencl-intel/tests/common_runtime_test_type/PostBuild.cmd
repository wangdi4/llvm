@echo off
setlocal enabledelayedexpansion

REM -------- usage: %1 - input filename, %2 - output directory ------------
REM Extracting the build type from the output directory
set OUTPUT_DIR=%2

set TARGET_NAME=%OUTPUT_DIR:~-10,10%
set TARGET_NAME=%TARGET_NAME:"=%
FOR /F "tokens=2 delims=_" %%A IN ("%TARGET_NAME%") DO SET TARGET_NAME=%%A

echo TARGET IS : %TARGET_NAME%

set n=#

REM copy kernel files to output directory
copy .\kernels\synch.cl %OUTPUT_DIR% > NUL
copy .\kernels\read_image.cl %OUTPUT_DIR% > NUL

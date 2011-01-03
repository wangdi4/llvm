set PLATFORM_NAME=%1
set CONFIG_NAME=%2

REM -------- export OCL FE compiler header files ------------
mkdir %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\fe_include
xcopy .\export\clang_include /Y /S %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\fe_include

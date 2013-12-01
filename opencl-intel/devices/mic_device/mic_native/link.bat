@ECHO ON

set CONFIG_NAME=%1

set EXE_NAME=%2

set OBJ_FILE=%3

set ICC="@ICC_INSTALL_DIR@"\bin\intel64_mic\icc

REM All variables in the form of CMake vars will be replaced by CMake
call "@DEVICE_INIT_ENV_SCRIPT@" intel64 > nul
REM ICC=/opt/intel/mic/Compiler/bin/icc

%ICC% -mmic -o "%EXE_NAME%" -Wl,--whole-archive @%OBJ_FILE% -Wl,--no-whole-archive @FINAL_MIC_LINK_FLAGS@


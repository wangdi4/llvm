@ECHO OFF

set SRC=%1
set ICC="@ICC_INSTALL_DIR@"\bin\intel64_mic\icc

REM All variables in the form of CMake vars will be replaced by CMake
call "@DEVICE_INIT_ENV_SCRIPT@" intel64 >  nul
REM #ICC=/opt/intel/mic/Compiler/bin/icc

%ICC% -mmic -M -MG -E @FINAL_MIC_FLAGS@ "%SRC%" -static-intel | python parse_dependency.py

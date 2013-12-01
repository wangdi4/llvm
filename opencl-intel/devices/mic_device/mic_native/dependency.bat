@ECHO OFF

set SRC=%1
set ICC="@ICC_INSTALL_DIR@"\bin\intel64_mic\icc

REM All variables in the form of CMake vars will be replaced by CMake
call "@DEVICE_INIT_ENV_SCRIPT@" intel64 >  nul
REM #ICC=/opt/intel/mic/Compiler/bin/icc

set FILE_NAME=dependency_args_%RANDOM%.txt

type NUL > %FILE_NAME%
echo -mmic -M -MG -E >> %FILE_NAME%
echo @FINAL_MIC_FLAGS@ >> %FILE_NAME%
echo "%SRC%" >> %FILE_NAME%
echo -static-intel >> %FILE_NAME%

%ICC% @%FILE_NAME% | python parse_dependency.py

del %FILE_NAME%

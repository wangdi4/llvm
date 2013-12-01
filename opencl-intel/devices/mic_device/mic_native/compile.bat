@ECHO OFF

set CONFIG_NAME=%1
shift

set SRC=%1
set OBJ=%2
set ICC="@ICC_INSTALL_DIR@"\bin\intel64_mic\icc

REM All variables in the form of CMake vars will be replaced by CMake
call "@DEVICE_INIT_ENV_SCRIPT@" intel64 > nul
REM #ICC=/opt/intel/mic/Compiler/bin/icc

set CONFIG_COMPILE_FLAG=@MIC_RELEASE_FLAGS@;
if %CONFIG_NAME%==Debug (
	set CONFIG_COMPILE_FLAG=@MIC_DEBUG_FLAGS@;
)

REM replace ';' with ' ', It come from the cmake with ';' separation because it is a list
if NOT CONFIG_COMPILE_FLAG==[] (
	set CONFIG_COMPILE_FLAG=%CONFIG_COMPILE_FLAG:;= %
)

set FILE_NAME=compile_args_%RANDOM%.txt

type NUL > %FILE_NAME%
echo -mmic -c >> %FILE_NAME%
echo @FINAL_MIC_FLAGS@ >> %FILE_NAME%
echo %CONFIG_COMPILE_FLAG% >> %FILE_NAME%
echo "%SRC%" >> %FILE_NAME%
echo -o "%OBJ%" >> %FILE_NAME%
echo -static-intel >> %FILE_NAME%

%ICC% @%FILE_NAME%

del %FILE_NAME%

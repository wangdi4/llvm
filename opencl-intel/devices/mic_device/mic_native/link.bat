@ECHO OFF

set CONFIG_NAME=%1
shift

set EXE_NAME=%1

:loop
shift
if not "%1"=="" (
	set OBJS=%OBJS% %1
	goto :loop
)

set ICC="@ICC_INSTALL_DIR@"\bin\intel64_mic\icc

REM All variables in the form of CMake vars will be replaced by CMake
call "@DEVICE_INIT_ENV_SCRIPT@" intel64 > nul
REM ICC=/opt/intel/mic/Compiler/bin/icc

set FILE_NAME=link_args_%RANDOM%.txt

type NUL > %FILE_NAME%
echo -mmic -o "%EXE_NAME%" -Wl,--whole-archive >> %FILE_NAME%
echo %OBJS% >> %FILE_NAME%
echo -Wl,--no-whole-archive >> %FILE_NAME%
echo @FINAL_MIC_LINK_FLAGS@ >> %FILE_NAME%

%ICC% @%FILE_NAME%

del %FILE_NAME%


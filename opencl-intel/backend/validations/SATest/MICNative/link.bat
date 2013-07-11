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

set CONFIG_LINK_FLAG=@MIC_RELEASE_LINK_FLAGS@;
if %CONFIG_NAME%==Debug (
	set CONFIG_LINK_FLAG=@MIC_DEBUG_LINK_FLAGS@;
)

REM replace ';' with ' ', It come from the cmake with ';' separation because it is a list
if NOT CONFIG_LINK_FLAG==[] (
	set CONFIG_LINK_FLAG=%CONFIG_LINK_FLAG:;= %
)

%ICC% -mmic -o "%EXE_NAME%" -Wl,--whole-archive %OBJS% -Wl,--no-whole-archive @FINAL_MIC_LINK_FLAGS@ %CONFIG_LINK_FLAG%


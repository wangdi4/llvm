@echo off
set CONFIG_NAME=%1
set PLATFORM_NAME=%2

if not defined PLATFORM_NAME (
	echo PLATFORM_NAME not defined, exiting
	exit /b 3
)

if not defined CONFIG_NAME (
	echo CONFIG_NAME not defined, exiting
	exit /b 5
)

cd .\ocl_builtins\cl_builtin_functions

@echo on
@echo Doing preparation work....
@echo Coping Clang (clc.exe)...
copy /Y %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\clc.exe .\bin\%PLATFORM_NAME%\%CONFIG_NAME%\Clang\

@echo Coping Linker (llvm-link.exe)...
copy /Y %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\llvm-link.exe .\bin\%PLATFORM_NAME%\%CONFIG_NAME%\

@echo Coping compiler headers
copy /Y %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\fe_include .\import\clang_include

@echo Build BuiltIns  for %PLATFORM_NAME%, Configuration: %CONFIG_NAME%
"%MTV_USER_CONF_DEVENV_DIR%\devenv" clbltfn.sln /build "%CONFIG_NAME%|%PLATFORM_NAME%"

@echo off
if not ERRORLEVEL 0 (
	echo Failed to build Built-ins solution
	exit /b 6
)

@echo on
@echo Coping auto-generated built-ins...
copy /Y .\%PLATFORM_NAME%\%CONFIG_NAME%\*.dll %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%%\
copy /Y .\%PLATFORM_NAME%\%CONFIG_NAME%\*.rtl %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%%\
copy /Y .\%PLATFORM_NAME%\%CONFIG_NAME%\*.pdb %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%%\

@echo off
echo Coping SVML binaries
if %PLATFORM_NAME% == x64 (
	set SVML_DIR_NAME=win32e
) else (
	set SVML_DIR_NAME=win32
)
@echo on
copy /Y ..\ocl_svml\%SVML_DIR_NAME%\*.dll %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%%\
copy /Y ..\ocl_svml\%SVML_DIR_NAME%\*.pdb %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%%\
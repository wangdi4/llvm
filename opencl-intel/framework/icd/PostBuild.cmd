set PLATFORM_NAME=%1
set CONFIG_NAME=%2
set TARGET_NAME=%3
set DEST_DIR_SUFIX=%4

REM -------- Check that all varibales are defined ------------
if not defined MTV_LOCAL_LIB_DIR (
	@echo MTV_LOCAL_LIB_DIR not defined, exiting
	exit /b 1
)
if not defined MTV_LOCAL_BIN_DIR (
	@echo MTV_LOCAL_BIN_DIR not defined, exiting
	exit /b 2
)
if not defined MTV_LOCAL_IMPORT_DIR (
	@echo MTV_LOCAL_IMPORT_DIR not defined, exiting
	exit /b 3
)

if not defined TARGET_NAME (
	@echo TARGET_NAME not defined, exiting
	exit /b 4
)

if not defined DEST_DIR_SUFIX (
	@echo DEST_DIR_SUFIX not defined, exiting
	exit /b 5
)

if not defined CONFIG_NAME (
	@echo CONFIG_NAME not defined, exiting
	exit /b 6
)

if not defined PLATFORM_NAME (
	@echo PLATFORM_NAME not defined, exiting
	exit /b 7
)

REM -------- export compiler header files ------------
REM xcopy .\export /Y /S %MTV_LOCAL_IMPORT_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\exportPhase.log 2>&1

REM -------- copy .lib file ------------
del /f %MTV_LOCAL_LIB_DIR%\%TARGET_NAME%.lib > %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.lib %MTV_LOCAL_LIB_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

del /f %MTV_LOCAL_LIB_DIR%.%DEST_DIR_SUFIX%\%TARGET_NAME%.lib >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.lib %MTV_LOCAL_LIB_DIR%.%DEST_DIR_SUFIX% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- copy .dll file ------------
del /f %MTV_LOCAL_BIN_DIR%\%TARGET_NAME%.dll > %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.dll %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

del /f %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX%\%TARGET_NAME%.dll >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.dll %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- copy .pdb file ------------
del /f %MTV_LOCAL_BIN_DIR%\%TARGET_NAME%.pdb >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.pdb %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

del /f %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX%\%TARGET_NAME%.pdb >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.pdb %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

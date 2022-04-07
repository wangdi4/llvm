set PLATFORM_NAME=%1
set CONFIG_NAME=%2
set TARGET_NAME=%3

REM -------- export header files ------------
attrib -r %MTV_LOCAL_IMPORT_DIR% > %PLATFORM_NAME%\%CONFIG_NAME%\exportPhase.log 2>&1
copy .\export\* %MTV_LOCAL_IMPORT_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\exportPhase.log 2>&1

REM -------- copy .lib file ------------
del /f %MTV_LOCAL_LIB_DIR%\%TARGET_NAME%.lib > %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.lib %MTV_LOCAL_LIB_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

del /f %MTV_LOCAL_LIB_DIR%.dbg\%TARGET_NAME%.lib >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.lib %MTV_LOCAL_LIB_DIR%.dbg >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- copy .pdb file ------------
del /f %MTV_LOCAL_BIN_DIR%\%TARGET_NAME%.pdb >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.pdb %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

del /f %MTV_LOCAL_BIN_DIR%.dbg\%TARGET_NAME%.pdb >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.pdb %MTV_LOCAL_BIN_DIR%.dbg >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

set PLATFORM_NAME=%1
set CONFIG_NAME=%2
set TARGET_NAME=%3

REM -------- copy .exe file ------------
del /f %MTV_LOCAL_BIN_DIR%\%TARGET_NAME%.exe > %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.exe %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

del /f %MTV_LOCAL_BIN_DIR%.dbg\%TARGET_NAME%.exe >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.exe %MTV_LOCAL_BIN_DIR%.dbg >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- copy .manifest file ------------
del /f %MTV_LOCAL_BIN_DIR%\%TARGET_NAME%.exe.manifest >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.exe.manifest %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

del /f %MTV_LOCAL_BIN_DIR%.dbg\%TARGET_NAME%.exe.manifest >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.exe.manifest %MTV_LOCAL_BIN_DIR%.dbg >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- copy .pdb file ------------
del /f %MTV_LOCAL_BIN_DIR%\%TARGET_NAME%.pdb >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.pdb %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

del /f %MTV_LOCAL_BIN_DIR%.dbg\%TARGET_NAME%.pdb >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.pdb %MTV_LOCAL_BIN_DIR%.dbg >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- copy .py file ------------
del /f %MTV_LOCAL_BIN_DIR%\%TARGET_NAME%.py > %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %TARGET_NAME%.py %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

del /f %MTV_LOCAL_BIN_DIR%.dbg\%TARGET_NAME%.py >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %TARGET_NAME%.py %MTV_LOCAL_BIN_DIR%.dbg >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- copy .xml file ------------
del /f %MTV_LOCAL_IMPORT_DIR%\%TARGET_NAME%.xml > %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %TARGET_NAME%.xml %MTV_LOCAL_IMPORT_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- copy .xsd file ------------
del /f %MTV_LOCAL_IMPORT_DIR%\%TARGET_NAME%.xsd > %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %TARGET_NAME%.xsd %MTV_LOCAL_IMPORT_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1



set PLATFORM_NAME=%1
set CONFIG_NAME=%2
set TARGET_NAME=%3

REM -------- copy .bin file ------------
copy /y %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.exe %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy /y %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.exe %MTV_LOCAL_BIN_DIR%.dbg >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- copy .pdb file ------------
copy /y %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.pdb %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy /y %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.pdb %MTV_LOCAL_BIN_DIR%.dbg >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- copy .cl file ------------
if exist %TARGET_NAME%.cl del /f %TARGET_NAME%.cl >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy /b /y *.cl %TARGET_NAME%.cl >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy /b /y %TARGET_NAME%.cl %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy /b /y %TARGET_NAME%.cl %MTV_LOCAL_BIN_DIR%.dbg >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

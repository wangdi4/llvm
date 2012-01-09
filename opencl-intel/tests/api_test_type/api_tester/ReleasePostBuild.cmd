set PLATFORM_NAME=%1
set CONFIG_NAME=%2
set TARGET_NAME=%3

REM -------- copy .exe file ------------
del /f %MTV_LOCAL_BIN_DIR%\%TARGET_NAME%.exe > %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.exe %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

del /f %MTV_LOCAL_BIN_DIR%.opt\%TARGET_NAME%.exe >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.exe %MTV_LOCAL_BIN_DIR%.opt >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- copy .py file ------------
del /f %MTV_LOCAL_BIN_DIR%\api_test_type.py > %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy .\..\api_test_type.py %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

del /f %MTV_LOCAL_BIN_DIR%.opt\api_test_type.py >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy .\..\api_test_type.py %MTV_LOCAL_BIN_DIR%.opt >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- copy .xml file ------------
del /f %MTV_LOCAL_IMPORT_DIR%\api_test_type.xml > %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy .\..\api_test_type.xml %MTV_LOCAL_IMPORT_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- copy .xsd file ------------
del /f %MTV_LOCAL_IMPORT_DIR%\api_test_type.xsd > %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy .\..\api_test_type.xsd %MTV_LOCAL_IMPORT_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1



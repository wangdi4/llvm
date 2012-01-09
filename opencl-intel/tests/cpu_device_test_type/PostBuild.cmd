set PLATFORM_NAME=%1
set CONFIG_NAME=%2
set TARGET_NAME=%3
set DEST_DIR_SUFIX=%4

REM -------- copy .exe file ------------
del /f %MTV_LOCAL_BIN_DIR%\%TARGET_NAME%.exe >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.exe %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

del /f %MTV_LOCAL_BIN_DIR%.opt\%TARGET_NAME%.exe >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.exe %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- copy .manifest file ------------
del /f %MTV_LOCAL_BIN_DIR%\%TARGET_NAME%.exe.manifest >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.exe.manifest %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

del /f %MTV_LOCAL_BIN_DIR%.opt\%TARGET_NAME%.exe.manifest >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.exe.manifest %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- copy .py file ------------
del /f %MTV_LOCAL_BIN_DIR%\%TARGET_NAME%.py >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %TARGET_NAME%.py %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

del /f %MTV_LOCAL_BIN_DIR%.opt\%TARGET_NAME%.py >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %TARGET_NAME%.py %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- copy .xml file ------------
del /f %MTV_LOCAL_IMPORT_DIR%\%TARGET_NAME%.xml >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %TARGET_NAME%.xml %MTV_LOCAL_IMPORT_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- copy .xsd file ------------
del /f %MTV_LOCAL_IMPORT_DIR%\%TARGET_NAME%.xsd >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %TARGET_NAME%.xsd %MTV_LOCAL_IMPORT_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM -------- Build tests file ------------
@echo Building test.bc
%MTV_LOCAL_BIN_DIR%\clc.exe -x cl test.cl -I %MTV_LOCAL_BIN_DIR%\fe_include -emit-llvm-bc -S -include opencl_.h -O3 -o %MTV_LOCAL_BIN_DIR%\test.bc

%MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX%\clc.exe -x cl test.cl -I %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX%\fe_include -emit-llvm-bc -S -include opencl_.h -O3 -o %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX%\test.bc

set PLATFORM_NAME=%1
set CONFIG_NAME=%2
set TARGET_NAME=%3
set DEST_DIR_SUFIX=%4

REM ------------------------------------------------------------------------------------------------------------------------------------------
REM Copy output (.exe) files to bin dir
REM ------------------------------------------------------------------------------------------------------------------------------------------

del /f %MTV_LOCAL_BIN_DIR%\%TARGET_NAME%.exe > %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.exe %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

del /f %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX%\%TARGET_NAME%.exe >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %PLATFORM_NAME%\%CONFIG_NAME%\%TARGET_NAME%.exe %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM ------------------------------------------------------------------------------------------------------------------------------------------
REM Copy .py files to bin dir
REM ------------------------------------------------------------------------------------------------------------------------------------------

del /f %MTV_LOCAL_BIN_DIR%\%TARGET_NAME%.py > %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %TARGET_NAME%.py %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

del /f %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX%\%TARGET_NAME%.py >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %TARGET_NAME%.py %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM ------------------------------------------------------------------------------------------------------------------------------------------
REM Copy .xml files to import dir
REM ------------------------------------------------------------------------------------------------------------------------------------------

del /f %MTV_LOCAL_IMPORT_DIR%\%TARGET_NAME%.xml > %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %TARGET_NAME%.xml %MTV_LOCAL_IMPORT_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM ------------------------------------------------------------------------------------------------------------------------------------------
REM Copy .xsd files to import dir
REM ------------------------------------------------------------------------------------------------------------------------------------------

del /f %MTV_LOCAL_IMPORT_DIR%\%TARGET_NAME%.xsd > %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy %TARGET_NAME%.xsd %MTV_LOCAL_IMPORT_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

REM ------------------------------------------------------------------------------------------------------------------------------------------
REM Copy VTune files to bin dir
REM ------------------------------------------------------------------------------------------------------------------------------------------

del /f %MTV_LOCAL_BIN_DIR%\VtuneApi.dll > %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy .\wolf\external\vtune\bin\VtuneApi.dll %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

del /f %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX%\VtuneApi.dll > %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy .\wolf\external\vtune\bin\VtuneApi.dll %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1


REM ------------------------------------------------------------------------------------------------------------------------------------------
REM Copy OpenCL kernels files (*.cl) to bin dir
REM ------------------------------------------------------------------------------------------------------------------------------------------

copy .\wolf\Workloads\tcc\tcc.cl %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy .\wolf\Workloads\tcc\tcc.cl %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy .\wolf\Workloads\intel_histogram\wlHistogram.cl %MTV_LOCAL_BIN_DIR% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy .\wolf\Workloads\intel_histogram\wlHistogram.cl %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy .\wolf\Workloads\ati_nbody\wlATINBody.cl %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1
copy .\wolf\Workloads\ati_matrix_transpose\wlATIMatrixTrans.cl %MTV_LOCAL_BIN_DIR%.%DEST_DIR_SUFIX% >> %PLATFORM_NAME%\%CONFIG_NAME%\customBuild.log 2>&1

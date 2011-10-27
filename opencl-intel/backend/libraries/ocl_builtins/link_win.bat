@echo off
rem Built-ins DLL build (out of BI object files) by ICC linker
rem All variables in the form of CMake vars will be replaced by CMake:
rem   ICC_ENV_SCRIPT_NATIVE - script setting the ICC environment
rem   ICC_PLATFORM          - parameter to the script above
rem 	ICC_LD_NATIVE         - executable of ICC linker
rem 	ICC_LINKER_PARAMS     - flags for linkage
rem   SVML_LIB_DIR_NATIVE   - directory with SVML import libraries
rem   RESOURCE_FILE_NATIVE  - resource file to be used for embedding of version info into DLL
rem   BUILD_TYPE_ICC_LINKER_FLAGS_Debug   - Debug build specific linker flags
rem   BUILD_TYPE_ICC_LINKER_FLAGS_Release - Release build specific linker flags
rem Invocation parameters:
rem     %1 - build type
rem     %2 - target DLL
rem     %3 - SVML import library
rem     %4 - PDB file
rem     %5 - list of object files

if exist @SVML_LIB_DIR_NATIVE@\%3 goto cont
	echo Cannot find SVML import library (@SVML_LIB_DIR_NATIVE@\%3). Link of %1 is terminated!
 	exit /b 1

:cont
call "@ICC_ENV_SCRIPT_NATIVE@" @ICC_PLATFORM@ >NUL
if "%1" == "Debug" goto debug

:release
if not "%1" == "Release" goto fail
rc /r /fo%1/clbltfn.res @RESOURCE_FILE_NATIVE@ >nul
"@ICC_LD_NATIVE@" @ICC_LINKER_PARAMS@ @BUILD_TYPE_ICC_LINKER_FLAGS_Release@ /LIBPATH:@SVML_LIB_DIR_NATIVE@ /OUT:%2 %3 /PDB:%4 %~5 %1/clbltfn.res >nul
goto fin

:debug
rc /r /fo%1/clbltfn.res @RESOURCE_FILE_NATIVE@ >nul
"@ICC_LD_NATIVE@" @ICC_LINKER_PARAMS@ @BUILD_TYPE_ICC_LINKER_FLAGS_Debug@ /LIBPATH:@SVML_LIB_DIR_NATIVE@ /OUT:%2 %3 /PDB:%4 %~5 %1/clbltfn.res >nul

:fin
exit /b 0

:fail
echo Unsupported build configuration %1!
exit /b 1

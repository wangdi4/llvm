@echo off
rem ImathLibd DLL build by ICC linker
rem All variables in the form of CMake vars will be replaced by CMake:
rem   ICC_CPU_ENV_SCRIPT_NATIVE - script setting the ICC environment
rem   ICC_PLATFORM 		        - parameter to the script above
rem   ICC_LD_NATIVE             - executable of ICC linker
rem   BUILD_TYPE_ICC_LINKER_FLAGS_Debug   - Debug build specific linker flags
rem   BUILD_TYPE_ICC_LINKER_FLAGS_Release - Release build specific linker flags
rem Invocation parameters:
rem     %1 - build type
rem     %2 - target DLL
rem     %3 - PDB file
rem     %4 - IMPLIB
rem     %5 - list of object files

:cont
call "@ICC_CPU_ENV_SCRIPT_NATIVE@" @ICC_PLATFORM@ >NUL
if "%1" == "Debug" goto debug

:release
if not "%1" == "Release" goto fail
"@ICC_CPU_LD_NATIVE@"  /nologo /INCREMENTAL:NO kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /TLBID:1 /SUBSYSTEM:WINDOWS /DYNAMICBASE /NXCOMPAT /DLL @BUILD_TYPE_ICC_LINKER_FLAGS_Release@ /OUT:%2 /PDB:%3 /IMPLIB:%4 %~5 >nul
goto fin

:debug
"@ICC_CPU_LD_NATIVE@"  /nologo /INCREMENTAL:NO kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /TLBID:1 /SUBSYSTEM:WINDOWS /DYNAMICBASE /NXCOMPAT /DLL @BUILD_TYPE_ICC_LINKER_FLAGS_Debug@ /OUT:%2 /PDB:%3 /IMPLIB:%4 %~5 >nul

:fin
exit /b 0

:fail
echo Unsupported build configuration %1!
exit /b 1

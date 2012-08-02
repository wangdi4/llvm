@echo off
rem Validation tests build by ICC linker
rem All variables in the form of CMake vars will be replaced by CMake:
rem   ICC_CPU_ENV_SCRIPT_NATIVE - script setting the ICC environment
rem   ICC_LD_NATIVE             - executable of ICC linker
rem   BUILD_TYPE_ICC_LINKER_FLAGS_Debug   - Debug build specific linker flags
rem   BUILD_TYPE_ICC_LINKER_FLAGS_Release - Release build specific linker flags
rem Invocation parameters:
rem     %1 - build type
rem     %2 - target executable
rem     %3 - PDB file
rem     %4 - IMPLIB
rem     %5 - additional lib dir
rem     %6 - list of object files

:cont
call "@ICC_CPU_ENV_SCRIPT_NATIVE@" @ICC_PLATFORM@ >NUL
if "%1" == "Debug" goto debug

:release
if not "%1" == "Release" goto fail
"@ICC_CPU_LD_NATIVE@" /nologo /INCREMENTAL:NO kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib %5/tinyxml_STL.lib %5/gtest.lib %5/NEAT.lib %5/REFALU.lib %5/Comparator.lib %5/DataGenerator.lib %5/DataManager.lib %5/LLVMSupport.lib %5/LLVMCore.lib %5/LLVMAsmParser.lib %5/OCLBuiltins.lib %5/ImathLibd.lib %5/gtest_main.lib %5/PlugInNEAT.lib %5/OclPluginManager.lib %5/OCLRecorderStat.lib %5/OpenCLKernelArgumentsParser.lib %5/OCLBuilder.lib %5/dynamic_load.lib @BUILD_TYPE_ICC_LINKER_FLAGS_Release@ /OUT:%2 /PDB:%3 /IMPLIB:%4 %~6 >nul
goto fin

:debug
"@ICC_CPU_LD_NATIVE@" /nologo /INCREMENTAL:NO kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib %5/tinyxml_STL.lib %5/gtest.lib %5/NEAT.lib %5/REFALU.lib %5/Comparator.lib %5/DataGenerator.lib %5/DataManager.lib %5/LLVMSupport.lib %5/LLVMCore.lib %5/LLVMAsmParser.lib %5/OpenCLKernelArgumentsParser.lib %5/OCLBuiltins.lib %5/ImathLibd.lib %5/gtest_main.lib %5/PlugInNEAT.lib %5/OclPluginManager.lib %5/OCLRecorderStat.lib %5/OCLBuilder.lib %5/dynamic_load.lib @BUILD_TYPE_ICC_LINKER_FLAGS_Debug@ /OUT:%2 /PDB:%3 /IMPLIB:%4 %~6 >nul
:fin
exit /b 0

:fail
echo Unsupported build configuration %1!
exit /b 1

rem @echo off
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
set CMDLINE="@ICC_CPU_LD_NATIVE@" /nologo /INCREMENTAL:NO @LDIRS_RELEASE@ kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib tinyxml_STL.lib googletest.lib NEAT.lib REFALU.lib Comparator.lib DataGenerator.lib DataManager.lib LLVMSupport.lib LLVMCore.lib LLVMAsmParser.lib LLVMSYCLTransforms.lib OCLBuiltins.lib ImathLibd.lib OCLKernelDataGenerator.lib PlugInNEAT.lib OclPluginManager.lib OCLRecorderStat.lib OpenCLKernelArgumentsParser.lib OCLBuilder.lib dynamic_load.lib antlr.lib @BUILD_TYPE_ICC_LINKER_FLAGS_Release@ /OUT:%2 /PDB:%3 /IMPLIB:%4 %~6 >nul
echo Running %CMDLINE%
%CMDLINE%
REM tinyxml_STL.lib gtest.lib NEAT.lib REFALU.lib Comparator.lib DataGenerator.lib DataManager.lib LLVMSupport.lib LLVMCore.lib LLVMAsmParser.lib LLVMSYCLTransforms.lib OCLBuiltins.lib ImathLibd.lib gtest_main.lib OCLKernelDataGenerator.lib PlugInNEAT.lib OclPluginManager.lib OCLRecorderStat.lib OpenCLKernelArgumentsParser.lib OCLBuilder.lib dynamic_load.lib antlr.lib
goto fin

:debug
set CMDLINE="@ICC_CPU_LD_NATIVE@" /nologo /INCREMENTAL:NO @LDIRS_DEBUG@ kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib tinyxml_STL.lib googletest.lib NEAT.lib REFALU.lib Comparator.lib DataGenerator.lib DataManager.lib LLVMSupport.lib LLVMCore.lib LLVMAsmParser.lib LLVMSYCLTransforms.lib OpenCLKernelArgumentsParser.lib OCLKernelDataGenerator.lib OCLBuiltins.lib ImathLibd.lib PlugInNEAT.lib OclPluginManager.lib OCLRecorderStat.lib OCLBuilder.lib dynamic_load.lib antlr.lib  @BUILD_TYPE_ICC_LINKER_FLAGS_Debug@ /OUT:%2 /PDB:%3 /IMPLIB:%4 %~6 >nul
echo Running %CMDLINE%
%CMDLINE%
REM tinyxml_STL.lib gtest.lib NEAT.lib REFALU.lib Comparator.lib DataGenerator.lib DataManager.lib LLVMSupport.lib LLVMCore.lib LLVMAsmParser.lib LLVMSYCLTransforms.lib OpenCLKernelArgumentsParser.lib OCLKernelDataGenerator.lib OCLBuiltins.lib ImathLibd.lib gtest_main.lib PlugInNEAT.lib OclPluginManager.lib OCLRecorderStat.lib OCLBuilder.lib dynamic_load.lib antlr.lib
:fin
exit /b 0

:fail
echo Unsupported build configuration %1!
exit /b 1

@echo off
set CONFIG_NAME=%1
set PLATFORM_NAME=%2

if not defined PLATFORM_NAME (
	echo PLATFORM_NAME not defined, exiting
	exit /b 3
)

if not defined CONFIG_NAME (
	echo CONFIG_NAME not defined, exiting
	exit /b 5
)

cd .\llvm_ext\build

if %PLATFORM_NAME% == Win32 (
	echo making build environment for Win32
	md win32
	cd win32
	cmake ..\..\llvm -DLLVM_ENABLE_WERROR:BOOL=ON  -DCMAKE_BINARY_DIR:PATH="..\..\build\Win32" -DCMAKE_INSTALL_PREFIX:PATH="..\..\build\Win32" -DLLVM_TARGETS_TO_BUILD:STRING="X86" -DPYTHON_EXECUTABLE:PATH="C:\Python24\python.exe" -G "Visual Studio 9 2008"
)

if %PLATFORM_NAME% == x64 (
	echo making build environment for x64
	md x64
	cd x64
	cmake ..\..\llvm -DLLVM_ENABLE_WERROR:BOOL=ON  -DCMAKE_BINARY_DIR:PATH="..\..\build\x64" -DCMAKE_INSTALL_PREFIX:PATH="..\..\build\x64" -DLLVM_TARGETS_TO_BUILD:STRING="X86" -DPYTHON_EXECUTABLE:PATH="C:\Python24\python.exe" -DCMAKE_ASM_MASM_COMPILER="C:/Program Files (x86)/Microsoft Visual Studio 9.0/VC/bin/x86_amd64/ml64.exe" -G "Visual Studio 9 2008 Win64"
)

echo build llvm for %PLATFORM_NAME%, Configuration: %CONFIG_NAME%
"%MTV_USER_CONF_DEVENV_DIR%\devenv" LLVM.sln /build %CONFIG_NAME%

echo Coping LLVM headers

md %MTV_LOCAL_IMPORT_DIR%\llvm
echo .svn > %MTV_USER_CONF_USER_TEMP_DIR%\exclude.txt
xcopy ..\..\llvm\include\llvm /y /e %MTV_LOCAL_IMPORT_DIR%\llvm /EXCLUDE:%MTV_USER_CONF_USER_TEMP_DIR%\exclude.txt
del %MTV_USER_CONF_USER_TEMP_DIR%\exclude.txt

copy /Y ..\..\build\%PLATFORM_NAME%\include\llvm\Config\*.* %MTV_LOCAL_IMPORT_DIR%\llvm\Config\
copy /Y ..\..\build\%PLATFORM_NAME%\include\llvm\System\*.* %MTV_LOCAL_IMPORT_DIR%\llvm\System\
copy /Y ..\..\build\%PLATFORM_NAME%\include\llvm\Intrinsics.gen %MTV_LOCAL_IMPORT_DIR%\llvm\

@echo on
@echo Coping CPU backend headers

copy /Y ..\..\ocl_cpu_backend\export\*.h %MTV_LOCAL_IMPORT_DIR%\

@echo copy LLVM binaries

copy /Y ..\..\build\%PLATFORM_NAME%\bin\%CONFIG_NAME%\tblgen.exe %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%%\
copy /Y ..\..\build\%PLATFORM_NAME%\bin\%CONFIG_NAME%\llc.exe %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%%\
copy /Y ..\..\build\%PLATFORM_NAME%\bin\%CONFIG_NAME%\llvm-link.exe %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%%\

@echo copy LLVM libraries
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMAnalysis.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMArchive.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMAsmParser.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMAsmPrinter.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMBitReader.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMBitWriter.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMCodeGen.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMCore.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMExecutionEngine.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMInstCombine.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMipa.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMipo.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMLinker.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMMC.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMScalarOpts.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMSelectionDAG.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMSupport.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMSystem.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMTarget.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMTransformUtils.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMX86AsmParser.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMX86AsmPrinter.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMX86CodeGen.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMX86Disassembler.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\LLVMX86Info.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\


@echo copy CPU backend binaries
copy /Y ..\..\build\%PLATFORM_NAME%\bin\%CONFIG_NAME%\OclCpuBackEnd.dll %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%%\
copy /Y ..\..\build\%PLATFORM_NAME%\bin\%CONFIG_NAME%\OclCpuBackEnd.pdb %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%%\

@echo copy CPU backend libraries
copy /Y ..\..\build\%PLATFORM_NAME%\lib\%CONFIG_NAME%\OclCpuBackEnd.lib %MTV_LOCAL_LIB_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%\

echo copy CPU backend builtin functions
copy ..\..\ocl_cpu_backend\bin\%PLATFORM_NAME%\%CONFIG_NAME%\*.* %MTV_LOCAL_BIN_DIR%.%PLATFORM_NAME%.%CONFIG_NAME%%\

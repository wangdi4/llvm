// clang_compiler.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "clang_compiler.h"
#include "clang_driver.h"
#include "logger.h"

#include <cl_device_api.h>

#include<windows.h>
#include<string.h>

using namespace Intel::OpenCL::ClangFE;
using namespace Intel::OpenCL::Utils;

// LLVM libraries
#pragma comment (lib, "LLVMAnalysis.lib")
#pragma comment (lib, "LLVMBitReader.lib")
#pragma comment (lib, "LLVMBitWriter.lib")
#pragma comment (lib, "LLVMCodeGen.lib")
#pragma comment (lib, "LLVMSupport.lib")
#pragma comment (lib, "LLVMSystem.lib")
#pragma comment (lib, "LLVMTarget.lib")
#pragma comment (lib, "LLVMTransformUtils.lib")
#pragma comment (lib, "LLVMCore.lib")
#pragma comment (lib, "LLVMX86Info.lib")
#pragma comment (lib, "LLVMX86CodeGen.lib")
#pragma comment (lib, "LLVMX86AsmParser.lib")
#pragma comment (lib, "LLVMX86AsmPrinter.lib")
#pragma comment (lib, "LLVMX86Disassembler.lib")
#pragma comment (lib, "LLVMMC.lib")
#pragma comment (lib, "LLVMSelectionDAG.lib")
#pragma comment (lib, "LLVMAsmPrinter.lib")
#pragma comment (lib, "LLVMScalarOpts.lib")
#pragma comment (lib, "LLVMipo.lib")
#pragma comment (lib, "LLVMipa.lib")
#pragma comment (lib, "LLVMInstCombine.lib")

// Clang libraries
#pragma comment (lib, "clangAST.lib")
#pragma comment (lib, "clangBasic.lib")
#pragma comment (lib, "clangCodeGen.lib")
#pragma comment (lib, "clangLex.lib")
#pragma comment (lib, "clangFrontend.lib")
#pragma comment (lib, "clangParse.lib")
#pragma comment (lib, "clangSema.lib")

extern DECLARE_LOGGER_CLIENT;

extern "C" _declspec(dllexport)	int	clFEBuildProgram(FEBuildProgramDesc* pDesc)
{
	LOG_INFO("Enter");

	ITask* pTask = new CompileTask(pDesc);
	if ( NULL == pTask )
		return CL_DEV_OUT_OF_MEMORY;

	GetTaskExecutor()->Execute(pTask);
	LOG_INFO("Exit");
	return CL_SUCCESS;
}

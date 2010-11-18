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
#pragma comment (lib, "Analysis.lib")
#pragma comment (lib, "Bitcode.lib")
#pragma comment (lib, "CodeGen.lib")
#pragma comment (lib, "Support.lib")
#pragma comment (lib, "System.lib")
#pragma comment (lib, "Target.lib")
#pragma comment (lib, "Transforms.lib")
#pragma comment (lib, "VMCore.lib")

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

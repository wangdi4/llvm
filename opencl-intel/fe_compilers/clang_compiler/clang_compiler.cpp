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

void ComposeBinaryContainer(void* pIR, size_t stSize, void** ppOutBuff)
{
	// Allocate continuous memory for IR container
	size_t	stTotSize = stSize+sizeof(cl_prog_container);
	*ppOutBuff = new char[stTotSize];
	if ( *ppOutBuff == NULL )
	{
		return;
	}

	cl_prog_container*	pHeader = (cl_prog_container*)*ppOutBuff;
	memcpy(pHeader->mask, _CL_CONTAINER_MASK_, 4);
	pHeader->container_size = stSize;
	pHeader->container_type = CL_PROG_CNT_PRIVATE;
	pHeader->description.bin_type = CL_PROG_BIN_LLVM;
	pHeader->description.bin_ver_major = 1;
	pHeader->description.bin_ver_minor = 0;
	pHeader->container = pHeader+1;
	// Copy IR
	memcpy_s((void*)pHeader->container, stTotSize, pIR, stSize);
}

// clang_compiler.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "clang_compiler.h"
#include "clang_driver.h"

#include <Logger.h>
#include <cl_sys_defines.h>
#include <cl_device_api.h>

#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ManagedStatic.h>

#if defined (_WIN32)
#include<windows.h>
#endif
#include<string.h>

using namespace Intel::OpenCL::ClangFE;
using namespace Intel::OpenCL::Utils;

#if defined (_WIN32)
#define DLL_EXPORT _declspec(dllexport)
// LLVM libraries
#pragma comment (lib, "LLVMAnalysis.lib")
#pragma comment (lib, "LLVMBitReader.lib")
#pragma comment (lib, "LLVMBitWriter.lib")
#pragma comment (lib, "LLVMCodeGen.lib")
#pragma comment (lib, "LLVMSupport.lib")
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
#else
#define DLL_EXPORT
#endif

using namespace Intel::OpenCL::FECompilerAPI;

extern DECLARE_LOGGER_CLIENT;

int InitClangDriver()
{
	INIT_LOGGER_CLIENT(L"ClangCompiler", LL_DEBUG);

	INIT_LOGGER_CLIENT(L"ClangCompiler", LL_DEBUG);
	
	LOG_INFO(TEXT("%s"), TEXT("Initialize ClangCompiler - start"));

	llvm::InitializeAllTargets();
	llvm::InitializeAllAsmPrinters();
	llvm::InitializeAllAsmParsers();
    llvm::InitializeAllTargetMCs();

	LOG_INFO(TEXT("%s"), TEXT("Initialize ClangCompiler - Finish"));
	return 0;
}

int CloseClangDriver()
{
	llvm::llvm_shutdown();

	LOG_INFO(TEXT("%s"), TEXT("Close ClangCompiler - done"));

	RELEASE_LOGGER_CLIENT;

	return 0;
}

Intel::OpenCL::Utils::AtomicCounter Intel::OpenCL::ClangFE::ClangFECompiler::s_llvmReferenceCount(0);

// ClangFECompiler class implementation
ClangFECompiler::ClangFECompiler(const char* pszDeviceExtensions)
{
	long prev = s_llvmReferenceCount++;
	if ( 0 == prev )
	{
		InitClangDriver();
	}

	m_pszDeviceExtensions = STRDUP(pszDeviceExtensions);
}

ClangFECompiler::~ClangFECompiler()
{
	if ( NULL != m_pszDeviceExtensions )
	{
		free(m_pszDeviceExtensions);
	}
	long prev = s_llvmReferenceCount--;
	if ( 1 == prev )
	{
		CloseClangDriver();
	}
}

int ClangFECompiler::CompileProgram(FECompileProgramDescriptor* pProgDesc, IOCLFEBinaryResult* *pBinaryResult)
{
	assert(NULL != pProgDesc);
	assert(NULL != pBinaryResult);

	// Create new compile task
	ClangFECompilerCompileTask* pCompileTask = new ClangFECompilerCompileTask(pProgDesc, m_pszDeviceExtensions);
	if ( NULL == pCompileTask )
	{
		*pBinaryResult = NULL;
		return CL_OUT_OF_HOST_MEMORY;
	}

    cl_err_code ret = pCompileTask->Compile();
	*pBinaryResult = pCompileTask;
	return ret;
}

int ClangFECompiler::LinkPrograms(Intel::OpenCL::FECompilerAPI::FELinkProgramsDescriptor* pProgDesc, 
                         Intel::OpenCL::FECompilerAPI::IOCLFEBinaryResult* *pBinaryResult)
{
    assert(NULL != pProgDesc);
	assert(NULL != pBinaryResult);

	// Create new link task
	ClangFECompilerLinkTask* pLinkTask = new ClangFECompilerLinkTask(pProgDesc);
	if ( NULL == pLinkTask )
	{
		*pBinaryResult = NULL;
		return CL_OUT_OF_HOST_MEMORY;
	}

    cl_err_code ret = pLinkTask->Link();
	*pBinaryResult = pLinkTask;
	return ret;
}

extern "C" DLL_EXPORT int CreateFrontEndInstance(const void* pDeviceInfo, size_t devInfoSize, IOCLFECompiler* *pFECompiler)
{
	assert(NULL != pFECompiler);

	IOCLFECompiler* pNewCompiler = new ClangFECompiler((const char*)pDeviceInfo);
	if ( NULL == pNewCompiler )
	{
		LOG_ERROR(TEXT("%S"), TEXT("Cann't allocate compiler instance"));
		return CL_OUT_OF_HOST_MEMORY;
	}
	
	*pFECompiler = pNewCompiler;
	return CL_SUCCESS;
}

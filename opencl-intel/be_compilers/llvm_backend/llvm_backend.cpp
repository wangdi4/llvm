// llvm_compiler.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include <string>
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Module.h"
#include "llvm/ModuleProvider.h"
#include "llvm\Support\ManagedStatic.h"
#include "logger.h"

using namespace llvm;
using namespace std;
using namespace Intel::OpenCL::Utils;

#pragma comment (lib, "cl_logger.lib")
#pragma comment (lib, "cl_sys_utils.lib")

// Global LLVM JIT
llvm::ExecutionEngine*	g_pExecEngine = NULL;
llvm::ModuleProvider*	g_ModuleProvider = NULL;

DECLARE_LOGGER_CLIENT;

ExecutionEngine* CreateLLVMBackend()
{
	INIT_LOGGER_CLIENT(L"LLVMBackend", LL_DEBUG);
	
	LOG_INFO("Initialize LLVMBackend - start");

	// Create some dummy module
	Module *M = new Module("dummy");
	g_ModuleProvider= new ExistingModuleProvider(M);

	// Now we going to create JIT
	string strLastError;
	g_pExecEngine = ExecutionEngine::createJIT(g_ModuleProvider, &strLastError);
	if ( NULL == g_pExecEngine )
	{
		LOG_ERROR("Initialize LLVMBackend - failed<%s>", strLastError.c_str());
		delete g_ModuleProvider;
		throw strLastError;
	}

	LOG_INFO("Initialize LLVMBackend - finished");
	return g_pExecEngine;
}

void ReleaseLLVMBackend()
{
	// Create some dummy module
	if ( NULL != g_pExecEngine )
	{
		g_pExecEngine->deleteModuleProvider(g_ModuleProvider);
		delete g_pExecEngine; 
		g_pExecEngine = NULL;
	}

	llvm_shutdown();

	LOG_INFO("LLVMBackend Released");
	RELEASE_LOGGER_CLIENT;
}

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

#ifdef __ENABLE_VTUNE__
#include "VTune\JITProfiling.h"
#pragma comment (lib, "JITProfiling.lib")
#endif

// Global LLVM JIT
llvm::ExecutionEngine*	g_pExecEngine = NULL;
llvm::ModuleProvider*	g_ModuleProvider = NULL;

llvm::Module*	        g_BuiltinsModule = NULL; 

llvm::Module *CreateBuiltinsModule();

DECLARE_LOGGER_CLIENT;

#ifdef __ENABLE_VTUNE__
void CallBack(void *UserData, iJIT_ModeFlags Flags)
{
}
#endif

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
#ifdef __ENABLE_VTUNE__
	//Register the call back to notifiy application of profiling mode changes
	iJIT_RegisterCallbackEx( NULL, &CallBack );
#endif

	g_BuiltinsModule = CreateBuiltinsModule();

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

	if( NULL != g_BuiltinsModule )
	{
		delete g_BuiltinsModule;
		g_BuiltinsModule = NULL;
	}

	llvm_shutdown();

	LOG_INFO("LLVMBackend Released");
	RELEASE_LOGGER_CLIENT;
}

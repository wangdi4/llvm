// llvm_backend.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "logger.h"
#include "cl_cpu_detect.h"
#include "cl_sys_info.h"

#include "llvm_backend.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Module.h"
#include "llvm\Support\MemoryBuffer.h"
#include "llvm\Support\ManagedStatic.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Constants.h"
#include "llvm/Target/TargetSelect.h"
#include "llvm/System/Threading.h"

#include <string>

using namespace llvm;
using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::DeviceBackend;

#pragma comment (lib, "cl_logger.lib")
#pragma comment (lib, "cl_sys_utils.lib")

#include "VTune\JITProfiling.h"
#pragma comment (lib, "JITProfiling.lib")

DECLARE_LOGGER_CLIENT;

static void VTuneCallBack(void *UserData, iJIT_ModeFlags Flags)
{
}

LLVMBackend* LLVMBackend::s_pLLVMInstance = NULL;

LLVMBackend::LLVMBackend() :
	m_pExecEngine(NULL), m_pRTModule(NULL), m_bRTLoaded(false), m_bVTuneInitialized(false)
{

}


LLVMBackend::~LLVMBackend()
{

}


LLVMBackend* LLVMBackend::GetInstance()
{
	if ( NULL != s_pLLVMInstance )
	{
		return s_pLLVMInstance;
	}

	LLVMBackend* pTmp = new LLVMBackend;
	if ( NULL == pTmp )
	{
		return NULL;
	}

	if ( pTmp->Init() )
	{
		s_pLLVMInstance = pTmp;
		return s_pLLVMInstance;
	}
	
	pTmp->Release();
	return NULL;
}

bool LLVMBackend::Init()
{
	INIT_LOGGER_CLIENT(L"LLVMBackend", LL_DEBUG);
	
	LOG_INFO("Initialize LLVMBackend - start");

	//if(!llvm::llvm_is_multithreaded())
	//	llvm::llvm_start_multithreaded();

	m_pLLVMContext = new LLVMContext;

	char szModuleName[MAX_PATH];
	char szRTLibName[MAX_PATH];


	GetModuleDirectory(szModuleName, MAX_PATH);

	unsigned int uFeatures = CPUDetect::GetInstance()->GetCPUFeatureSupport();
	const char* pCPUPrefix = NULL;
	if( uFeatures & CFS_AVX10 )
	{
		pCPUPrefix = "g9";
	}
	else if ( uFeatures & CFS_SSE42 )
	{
		pCPUPrefix = "n8";
	}
	else if ( uFeatures & CFS_SSE41 )
	{
		pCPUPrefix = "p8";
	}
	else if ( uFeatures & CFS_SSSE3 )
	{
		pCPUPrefix = "v8";
	}
	else if ( uFeatures & CFS_SSE3 )
	{
		pCPUPrefix = "t7";
	}
	else
	{
		pCPUPrefix = "w7";
	}

	// Load precompiled Built-in functions
	sprintf_s(szRTLibName, MAX_PATH, "%sclbltfn%s.dll", szModuleName, pCPUPrefix);
	m_dllBuiltIns.Load(szRTLibName);

	sprintf_s(szRTLibName, MAX_PATH, "%s__ocl_svml_%s.dll", szModuleName, pCPUPrefix);
	m_dllSVML.Load(szRTLibName);

	// Load LLVM built-ins module
	sprintf_s(szRTLibName, MAX_PATH, "%sclbltfn%s.rtl", szModuleName, pCPUPrefix);
	m_pRTModule = NULL;
	// Load library
	MemoryBuffer* pLibBuff = MemoryBuffer::getFile(szRTLibName);
	if ( NULL != pLibBuff )
	{
		m_pRTModule = ParseBitcodeFile(pLibBuff, *m_pLLVMContext);
		delete pLibBuff;
	}
	// Failed to load runtime library
	if ( NULL == m_pRTModule )
	{
		m_pRTModule = new Module("dummy", *m_pLLVMContext);
	}
	else
	{
		m_pRTModule->setModuleIdentifier("RTLibrary");
		m_bRTLoaded = true;
	}

	llvm::InitializeAllTargets();
	llvm::InitializeAllAsmPrinters();

	// Now we going to create JIT
	string strLastError;
	m_pExecEngine = ExecutionEngine::createJIT(m_pRTModule, &strLastError);
	if ( NULL == m_pExecEngine )
	{
		LOG_ERROR("Initialize LLVMBackend - failed<%s>", strLastError.c_str());
		delete m_pRTModule;
		m_pRTModule = NULL;
		return false;
	}

	// if loaded parse the RT library module
	if ( m_bRTLoaded )
	{
		ParseRTModule();
	}

	LOG_INFO("Initialize LLVMBackend - finished");
	return true;
}

void LLVMBackend::Release()
{
	m_dllBuiltIns.Close();
	m_dllSVML.Close();

	// Create some dummy module
	if ( NULL != m_pExecEngine )
	{
		delete m_pExecEngine; 
		m_pExecEngine = NULL;
		m_pRTModule = NULL;
	}

	delete m_pLLVMContext;

	llvm_shutdown();

	s_pLLVMInstance = NULL;
	delete this;

	LOG_INFO("LLVMBackend Released");
	RELEASE_LOGGER_CLIENT;
}

void LLVMBackend::InitVTune()
{
	if(!m_bVTuneInitialized)
	{
		//Register the call back to notifiy application of profiling mode changes
		iJIT_RegisterCallbackEx( NULL, VTuneCallBack );
	}
}

Module*	LLVMBackend::GetRTModule()
{
	return m_pRTModule;
}

void LLVMBackend::ParseRTModule()
{
	// Built usage table for globals,
	// Later we need to exported the globals to a kernel module

	Module* pModule = m_pRTModule;

	// Enumerate globals and build function to global map
	Module::GlobalListType &lstGlobals = pModule->getGlobalList();
	for ( Module::GlobalListType::iterator it = lstGlobals.begin(), e = lstGlobals.end(); it != e; ++it)
	{
		GlobalVariable* pVal = it;

		GlobalValue::use_iterator use_it, use_e;
		
		for (use_it= pVal->use_begin(), use_e=pVal->use_end(); use_it != use_e; ++use_it)
		{
			if ( isa<Instruction>(use_it) )
			{
				Function* pFunc = cast<Instruction>(use_it)->getParent()->getParent();
				m_mapFunc2Glb[pFunc].push_back(pVal);
			}
			if ( isa<ConstantExpr>(use_it))
			{
				ConstantExpr* pExp = cast<ConstantExpr>(use_it);
				ConstantExpr::use_iterator exp_it, exp_e;

				for (exp_it= pExp->use_begin(), exp_e=pExp->use_end(); exp_it != exp_e; ++exp_it)
				{
					Instruction* pInst = dyn_cast<Instruction>(exp_it);
					assert(pInst && "Should be an instruction");
					Function* pFunc = pInst->getParent()->getParent();
					m_mapFunc2Glb[pFunc].push_back(pVal);
				}

			}
			
		}
	} // Enumeration done

	// Enumerate functions and add to global map, those that are called from other built-ins
	for (Module::iterator extIt = pModule->begin(), extE = pModule->end(); extIt != extE; ++extIt)
	{
		// We need only external functions
		GlobalValue* pVal = extIt;

		GlobalValue::use_iterator use_it, use_e;

		for (use_it= pVal->use_begin(), use_e=pVal->use_end(); use_it != use_e; ++use_it)
		{
			if ( isa<Instruction>(use_it) )
			{
				Function* pFunc = cast<Instruction>(use_it)->getParent()->getParent();
				m_mapFunc2Glb[pFunc].push_back(pVal);
			}
		}
	} // Enumeration done

}
/////////////////////////////////////////////////////////////////////////
// llvm_program.cpp:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its 
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and 
// proprietary and confidential information of Intel Corporation and its 
// suppliers and licensors, and is protected by worldwide copyright and trade 
// secret laws and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, distributed, 
// or disclosed in any way without Intel’s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "cl_device_api.h"
#include "cl_thread.h"
#include "cpu_dev_limits.h"
#include "llvm_backend.h"

#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Module.h"
#include "llvm/Support/MemoryBuffer.h"

#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Constants.h"
#include "llvm/ModuleProvider.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Instructions.h"
#include "llvm/CallingConv.h"
#include "llvm/TypeSymbolTable.h"

#include "llvm/PassManager.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Analysis/Verifier.h"

#include "Vectorizer.h"

#include "llvm_program.h"
#include "llvm_kernel.h"
#include "logger.h"

#include <string>
#include <list>
#include <map>

#pragma comment(lib, "x86.lib")
#pragma comment(lib, "transforms.lib")
#pragma comment(lib, "vmcore.lib")
#pragma comment(lib, "target.lib")
#pragma comment(lib, "system.lib")
#pragma comment(lib, "executionengine.lib")
#pragma comment(lib, "codegen.lib")
#pragma comment(lib, "bitcode.lib")
#pragma comment(lib, "support.lib")
#pragma comment(lib, "analysis.lib")

using namespace Intel::OpenCL;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::DeviceBackend;
using namespace std;
using namespace llvm;

// Compilation Log strings
static const char szNone[] = "None";
static const char szBuilding[] = "Building";
static const char szError[] = "Error";

extern llvm::Module*	g_BuiltinsModule;
extern DECLARE_LOGGER_CLIENT;

// Constructor/Destructor
LLVMProgram::LLVMProgram(LLVMProgramConfig *pConfig) :
m_pMemBuffer(NULL), m_pModuleProvider(NULL),
m_bUseVectorizer(pConfig->bUseVectorizer)
{
	memset(&m_ContainerInfo, 0, sizeof(cl_prog_container));
	m_strLastError = szNone;
}

cl_int LLVMProgram::CreateProgram(const cl_prog_container* IN pContainer, ICLDevBackendProgram** OUT pProgram, LLVMProgramConfig *pConfig)
{
	LOG_INFO("Enter");
	// Allocate Program object
	LLVMProgram*	pMyProg = new LLVMProgram(pConfig);
	assert(pProgram != NULL);
	*pProgram = pMyProg;
	if ( pMyProg == NULL )
	{
		LOG_ERROR("Failed to allocate program class");
		return CL_DEV_OUT_OF_MEMORY;
	}
	const char* pIR;		// Pointer to LLVM representation

	// Container is provided by user
	if ( NULL == pContainer->container )
	{
		pIR = (const char*)pContainer+sizeof(cl_prog_container);
	}
	else
	{
		pIR = (const char*)pContainer->container;
	}

	// Create Memory buffer to store IR data
	pMyProg->m_pMemBuffer = MemoryBuffer::getMemBufferCopy(pIR, pIR+pContainer->container_size);
	if ( NULL == pMyProg->m_pMemBuffer )
	{
		LOG_ERROR("Failed to allocate container buffer");
		return CL_DEV_OUT_OF_MEMORY;
	}

	// Store container information
	memcpy(&pMyProg->m_ContainerInfo, pContainer, sizeof(cl_prog_container));
	pMyProg->m_ContainerInfo.container = pMyProg->m_pMemBuffer->getBufferStart();

	LOG_INFO("Exit");
	return CL_DEV_SUCCESS;
}

void LLVMProgram::Release()
{
	FreeMap();
	if ( NULL != m_pModuleProvider )
	{
		LLVMBackend::GetInstance()->GetExecEngine()->deleteModuleProvider(m_pModuleProvider);
	}
	if (NULL != m_pMemBuffer)
	{
		delete m_pMemBuffer;
	}
	delete this;
}

cl_int LLVMProgram::OptimizeProgram(Module *pModule)
{
	bool UnitAtATime = true;
	bool DisableSimplifyLibCalls = true;

    // Create a PassManager to hold and optimize the collection of passes we are
    // about to build...
    //
    PassManager Passes;

    // Add an appropriate TargetData instance for this module...
    Passes.add(new TargetData(pModule));

    FunctionPassManager *FPasses = NULL;
	FPasses = new FunctionPassManager(new ExistingModuleProvider(pModule));
	FPasses->add(new TargetData(pModule));

	FPasses->add(createCFGSimplificationPass());

	FPasses->add(createScalarReplAggregatesPass());
	FPasses->add(createInstructionCombiningPass());

	Passes.add(createVerifierPass());

	if (UnitAtATime)
		Passes.add(createRaiseAllocationsPass());      // call %malloc -> malloc inst
	Passes.add(createCFGSimplificationPass());       // Clean up disgusting code
	Passes.add(createPromoteMemoryToRegisterPass()); // Kill useless allocas
	if (UnitAtATime) {
		Passes.add(createGlobalOptimizerPass());       // OptLevel out global vars
		Passes.add(createGlobalDCEPass());             // Remove unused fns and globs
		Passes.add(createIPConstantPropagationPass()); // IP Constant Propagation
		Passes.add(createDeadArgEliminationPass());    // Dead argument elimination
	}
	Passes.add(createInstructionCombiningPass());    // Clean up after IPCP & DAE
	Passes.add(createCFGSimplificationPass());       // Clean up after IPCP & DAE
	if (UnitAtATime) {
		Passes.add(createPruneEHPass());               // Remove dead EH info
		Passes.add(createFunctionAttrsPass());         // Deduce function attrs
	}

	Passes.add(createFunctionInliningPass());      // Inline small functions
	Passes.add(createArgumentPromotionPass());   // Scalarize uninlined fn args
	if (!DisableSimplifyLibCalls)
		Passes.add(createSimplifyLibCallsPass());    // Library Call Optimizations
	Passes.add(createInstructionCombiningPass());  // Cleanup for scalarrepl.

	Passes.add(createJumpThreadingPass());         // Thread jumps.
	Passes.add(createCFGSimplificationPass());     // Merge & remove BBs
	Passes.add(createScalarReplAggregatesPass());  // Break up aggregate allocas
	Passes.add(createInstructionCombiningPass());  // Combine silly seq's

	Passes.add(createCondPropagationPass());       // Propagate conditionals
	Passes.add(createTailCallEliminationPass());   // Eliminate tail calls

	Passes.add(createCFGSimplificationPass());     // Merge & remove BBs
	Passes.add(createReassociatePass());           // Reassociate expressions
	Passes.add(createLoopRotatePass());            // Rotate Loop
	Passes.add(createLICMPass());                  // Hoist loop invariants
	Passes.add(createLoopUnswitchPass());
	Passes.add(createLoopIndexSplitPass());        // Split loop index
	Passes.add(createInstructionCombiningPass());  

	Passes.add(createIndVarSimplifyPass());        // Canonicalize indvars
	Passes.add(createLoopDeletionPass());          // Delete dead loops
	Passes.add(createLoopUnrollPass());          // Unroll small loops
	Passes.add(createInstructionCombiningPass());  // Clean up after the unroller

	Passes.add(createGVNPass());                   // Remove redundancies
	Passes.add(createMemCpyOptPass());             // Remove memcpy / form memset
	Passes.add(createSCCPPass());                  // Constant prop with SCCP

	// Run instcombine after redundancy elimination to exploit opportunities
	// opened up by them.
	Passes.add(createInstructionCombiningPass());

	Passes.add(createCondPropagationPass());       // Propagate conditionals
	Passes.add(createDeadStoreEliminationPass());  // Delete dead stores
	Passes.add(createAggressiveDCEPass());   // Delete dead instructions
	Passes.add(createCFGSimplificationPass());     // Merge & remove BBs

	if (UnitAtATime) {
		Passes.add(createStripDeadPrototypesPass());   // Get rid of dead prototypes
		Passes.add(createDeadTypeEliminationPass());   // Eliminate dead types
	}
  
	if (UnitAtATime)
		Passes.add(createConstantMergePass());       // Merge dup global constants 

	Pass *vectorizerPass = NULL;
	if( m_bUseVectorizer )
	{
		vectorizerPass = createVectorizerPass(g_BuiltinsModule);
		Passes.add(vectorizerPass);
	}
	Passes.add(createVerifierPass());

	Passes.add(createBuiltInImportPass(LLVMBackend::GetInstance()->GetRTModule())); // Inline BI function
	ModulePass* updatePass = createKernelUpdatePass(vectorizerPass);
	Passes.add(updatePass);
#ifdef _DEBUG
	//Passes.add(createVerifierPass());
#endif

	Passes.add(createFunctionInliningPass());		// Inline small functions
	Passes.add(createArgumentPromotionPass());		// Scalarize uninlined fn args
	if (!DisableSimplifyLibCalls)
		Passes.add(createSimplifyLibCallsPass());   // Library Call Optimizations
	Passes.add(createInstructionCombiningPass());	// Cleanup for scalarrepl.
	//Passes.add(createVerifierPass());

	for (Module::iterator I = pModule->begin(), E = pModule->end(); I != E; ++I)
		FPasses->run(*I);
	delete FPasses;

	Passes.run(*pModule);

	getKernelInfoMap(updatePass, m_mapKernelInfo);

	if( m_bUseVectorizer )
	{
		VectorizedFunctions.clear();

		SmallVector<Function*, 16> vectFunctions;
		SmallVector<int, 16>       vectWidths;

		getVectorizerFunctions((Vectorizer *)vectorizerPass, vectFunctions);
		getVectorizerWidths((Vectorizer *)vectorizerPass, vectWidths);
		assert(vectFunctions.size() == vectWidths.size());

		for(unsigned int i=0; i < vectFunctions.size(); i++)
		{
			VectorizedFunctions.push_back(FunctionWidthPair(vectFunctions[i], vectWidths[i]));
		}
	}

	return CL_DEV_SUCCESS;
}

LLVMKernel *LLVMProgram::CreateKernel(llvm::Function *pFunc, ConstantArray *pFuncArgs)
{
	// Check maximum number of arguments to kernel
	if ( (CPU_KERNEL_MAX_ARG_COUNT + KRNL_NUM_CONST_ARGS) < pFunc->arg_size() )
	{
		m_strLastError = "Too many arguments in function";
		LOG_ERROR("Build of function <%s> fail, <%s>", pFunc->getName().c_str(), m_strLastError.c_str());
		return NULL;
	}

	// Create new kernel container
	LLVMKernel* pKernel = new LLVMKernel(this);
	if ( NULL == pKernel )
	{
		m_strLastError = "Out of Memory";
		LOG_ERROR("Build of function <%s> fail, Cannot allocate LLVMKernel", pFunc->getName().c_str());
		return NULL;
	}

	cl_int rc;
	// Try to initialize kernel, on error
	try
	{
		rc = pKernel->Init(pFunc, pFuncArgs);
	}
	catch( std::string err )
	{
		m_strLastError = err;
		rc = CL_DEV_BUILD_ERROR;
	}
	if ( CL_DEV_FAILED(rc) )
	{
		pKernel->Release();
		LOG_ERROR("Build of function <%s> fail, <%s>", pFunc->getName().c_str(), m_strLastError.c_str());
		return NULL;
	}

	return pKernel;
}

cl_int LLVMProgram::BuildProgram(const char* pOptions)
{
	LOG_INFO("Entry");
	// Initiate log string to error
	m_strLastError = szError;

	cl_int rc;

	assert(NULL != m_pMemBuffer);

	// Create module to put IR in it
	Module *pModule = ParseBitcodeFile(m_pMemBuffer, &m_strLastError);
	if ( NULL == pModule )
	{
		LOG_ERROR("ParseBitcodeFile failed <%s>", m_strLastError.c_str());
		return CL_DEV_INVALID_BINARY;
	}

	rc =  OptimizeProgram(pModule);
	if ( CL_DEV_FAILED(rc) )
	{
		LOG_ERROR("FAILED to optimize module (first)");
		m_strLastError = "Optimization failed";
		delete pModule;
		return rc;
	}

	m_pModuleProvider = new ExistingModuleProvider(pModule);
	if ( NULL == m_pModuleProvider )
	{
		LOG_ERROR("FAILED to allocate ExistingModuleProvider");
		m_strLastError = "OutOfMemory";
		delete pModule;
		return CL_DEV_OUT_OF_MEMORY;
	}

	m_strLastError = szBuilding;

	LLVMBackend::GetInstance()->GetExecEngine()->addModuleProvider(m_pModuleProvider);

	// Now after JIT is up and setup with IR, we scan module for kernels
	LOG_DEBUG("Start iterating over kernels");
	GlobalVariable *annotation = pModule->getGlobalVariable("llvm.global.annotations");
	if ( NULL == annotation )
	{
		return CL_DEV_SUCCESS;
	}
	ConstantArray *init = dyn_cast<ConstantArray>(annotation->getInitializer());
	std::vector<FunctionWidthPair>::iterator vecIter = VectorizedFunctions.begin();
	for (unsigned i = 0, e = init->getType()->getNumElements(); i != e; ++i) 
	{
		// Obtain kernel function from annotation
		ConstantStruct *elt = cast<ConstantStruct>(init->getOperand(i));
		Function *pFunc = dyn_cast<Function>(elt->getOperand(0)->stripPointerCasts());
		if ( NULL == pFunc )
		{
			continue;	// Not a function pointer
		}

		// Obtain parameters definition
		GlobalVariable* pGlbVar = dyn_cast<GlobalVariable>(elt->getOperand(1)->stripPointerCasts());
		ConstantArray *pFuncArgs;
		if ( (NULL == pGlbVar) || (NULL == (pFuncArgs = dyn_cast<ConstantArray>(pGlbVar->getInitializer()))) )
		{
			m_strLastError = "Invalid argument's map";
			FreeMap();
			LOG_ERROR("Build of function <%s> fail, <%s>", pFunc->getName().c_str(), m_strLastError.c_str());
			return CL_DEV_BUILD_ERROR;
		}

		LLVMKernel *pKernel = CreateKernel(pFunc, pFuncArgs);
		if ( NULL == pKernel )
		{
			FreeMap();
			return CL_DEV_BUILD_ERROR;
		}

		// Check if vectorized kernel present
		// Store kernel to map
		m_mapKernels[pKernel->GetKernelName()] = pKernel;

		if(m_bUseVectorizer)
		{
			assert(vecIter != VectorizedFunctions.end());
			if(vecIter->first != NULL)
			{
				// We don't need to pass argument list here
				LLVMKernel *pVecKernel = CreateKernel(vecIter->first, NULL);
				if ( NULL == pKernel )
				{
					FreeMap();
					return CL_DEV_BUILD_ERROR;
				}

				pKernel->setVectorizerProperties(pVecKernel, vecIter->second);
			}
			vecIter++;
		}
	}
	LOG_DEBUG("Iterating completed");

	LOG_INFO("Exit");
	return CL_DEV_SUCCESS;
}

// ------------------------------------------------------------------------------
// Quarries program build log
cl_int	LLVMProgram::GetBuildLog(size_t* pSize, char* pLog) const
{
	size_t stLogSize = m_strLastError.length();
	if ( NULL == pLog )
	{
		*pSize = stLogSize;
		return CL_DEV_SUCCESS;
	}
	assert(*pSize >= stLogSize);

	memcpy_s(pLog, *pSize, m_strLastError.c_str(), stLogSize);

	return CL_DEV_SUCCESS;
}

// ------------------------------------------------------------------------------
// Copies internally stored container into provided buffer
cl_int LLVMProgram::GetContainer(size_t *pSize, cl_prog_container* pContainer) const
{
	if ( NULL == m_pMemBuffer )
	{
		return CL_DEV_INVALID_BINARY;
	}

	size_t stSize = m_pMemBuffer->getBufferSize() + sizeof(cl_prog_container);
	if ( NULL == pContainer)
	{
		assert(pSize);
		*pSize = stSize;
		return CL_DEV_SUCCESS;
	}
	assert(*pSize >= stSize);

	// Copy container header
	memcpy(pContainer, &m_ContainerInfo, sizeof(cl_prog_container));
	// Copy container content
	memcpy(((char*)pContainer)+sizeof(cl_prog_container), m_pMemBuffer->getBufferStart(), m_pMemBuffer->getBufferSize());
	// container pointer for user must be NULL
	((cl_prog_container*)pContainer)->container = NULL;

	return CL_DEV_SUCCESS;
}

// ------------------------------------------------------------------------------
// Retrieves a pointer to a function descriptor by function short name
cl_int LLVMProgram::GetKernel(const char* pName, const ICLDevBackendKernel* *pKernel) const
{
	TKernelMap::const_iterator	it;

	it = m_mapKernels.find(pName);
	if ( m_mapKernels.end() == it )
	{
		return CL_DEV_INVALID_KERNEL_NAME;
	}

	*pKernel = it->second;

	return CL_DEV_SUCCESS;
}

// ------------------------------------------------------------------------------
// Retrieves a vector of pointers to a function descriptors
cl_int LLVMProgram::GetAllKernels(const ICLDevBackendKernel* *pKernels, cl_uint *puiRetCount) const
{
	cl_uint uiCount = m_mapKernels.size();

	if ( NULL == pKernels )
	{
		assert (puiRetCount);
		*puiRetCount = uiCount;
		return CL_DEV_SUCCESS;
	}

	assert(m_mapKernels.size() <= *puiRetCount);

	TKernelMap::const_iterator it;
	unsigned int i = 0;
	for(it = m_mapKernels.begin(); it!=m_mapKernels.end();++it)
	{
		pKernels[i] = it->second;
		++i;
	}

	return CL_DEV_SUCCESS;
}

void LLVMProgram::FreeMap()
{
	// Free descriptors map
	TKernelMap::iterator it;
	for(it = m_mapKernels.begin(); it != m_mapKernels.end(); ++it)
	{
		it->second->Release();
	}

	m_mapKernels.clear();
}

bool LLVMProgram::IsKernel(const char* szFuncName)
{
	GlobalVariable *annotation = m_pModuleProvider->getModule()->getGlobalVariable("llvm.global.annotations");
	assert ( annotation );

	ConstantArray *init = dyn_cast<ConstantArray>(annotation->getInitializer());
	for (unsigned i = 0, e = init->getType()->getNumElements(); i != e; ++i) 
	{
		// Obtain kernel function from annotation
		ConstantStruct *elt = cast<ConstantStruct>(init->getOperand(i));
		Function *pFuncVal = dyn_cast<Function>(elt->getOperand(0)->stripPointerCasts());
		if ( NULL == pFuncVal )
		{
			continue;	// Not a function pointer
		}

		if ( pFuncVal->isName(szFuncName) )
		{
			return true;
		}
	}

	// Function not found
	return false;
}

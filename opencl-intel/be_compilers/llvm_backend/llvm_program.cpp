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
#include "llvm/DerivedTypes.h"
#include "llvm/Instructions.h"
#include "llvm/CallingConv.h"
#include "llvm/TypeSymbolTable.h"

#include "llvm/PassManager.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/support/StandardPasses.h"
#include "llvm/support/raw_ostream.h"

#include "Vectorizer.h"

#include "llvm_program.h"
#include "llvm_kernel.h"
#include "logger.h"

#include "cl_synch_objects.h"

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

extern DECLARE_LOGGER_CLIENT;

OclMutex LLVMProgram::s_serializingMutex;

// Constructor/Destructor
LLVMProgram::LLVMProgram(LLVMProgramConfig *pConfig) :
m_pMemBuffer(NULL), m_pModule(NULL),
m_bUseVectorizer(pConfig->bUseVectorizer), m_bUseVTune(pConfig->bUseVTune)
{
	memset(&m_ContainerInfo, 0, sizeof(cl_prog_container_header));
	memset(&m_ProgHeader, 0, sizeof(cl_llvm_prog_header));
	m_strLastError = szNone;
}

cl_int LLVMProgram::CreateProgram(const cl_prog_container_header* IN pContainer, ICLDevBackendProgram** OUT pProgram, LLVMProgramConfig *pConfig)
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

	pMyProg->m_pLLVMContext = LLVMBackend::GetInstance()->m_pLLVMContext;

	const char* pIR;		// Pointer to LLVM representation
	pIR = (const char*)pContainer+sizeof(cl_prog_container_header)+sizeof(cl_llvm_prog_header);
	size_t stIRsize = pContainer->container_size - sizeof(cl_llvm_prog_header);
	// Create Memory buffer to store IR data
	pMyProg->m_pMemBuffer = MemoryBuffer::getMemBufferCopy(pIR, pIR+stIRsize);
	if ( NULL == pMyProg->m_pMemBuffer )
	{
		LOG_ERROR("Failed to allocate container buffer, Exit");
		return CL_DEV_OUT_OF_MEMORY;
	}

	// Store container information
	memcpy(&pMyProg->m_ContainerInfo, pContainer, sizeof(cl_prog_container_header));
	memcpy(&pMyProg->m_ProgHeader, (const char*)pContainer+sizeof(cl_prog_container_header), sizeof(cl_llvm_prog_header));

	LOG_INFO("Exit");
	return CL_DEV_SUCCESS;
}

void LLVMProgram::Release()
{
	FreeMap();
	if ( NULL != m_pModule )
	{
		delete m_pModule;
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

	SmallVector<Function*, 16> vectFunctions;

	// Create a PassManager to hold and optimize the collection of passes we are
    // about to build...
    //
    PassManager Passes;

    // Add an appropriate TargetData instance for this module...
    Passes.add(new TargetData(pModule));

	FunctionPassManager *FPasses = NULL;
	FPasses = new FunctionPassManager(pModule);

	Pass *vectorizerPass = NULL;

	unsigned int uiOptLevel = (m_ProgHeader.bDisableOpt ? 0 : 3);

	llvm::createStandardFunctionPasses(FPasses, uiOptLevel);

	llvm::createStandardModulePasses(
		&Passes,
		uiOptLevel,
		false,
		true,
		true,
		false,
		false,
		NULL
		);

	Passes.add(createUnifyFunctionExitNodesPass());

	if( m_bUseVectorizer )
	{
		vectorizerPass = createVectorizerPass(LLVMBackend::GetInstance()->GetRTModule());
		Passes.add(vectorizerPass);
	}
	Passes.add(createVerifierPass());

	if( m_ProgHeader.bFastRelaxedMath )
	{
		Passes.add(createRelaxedPass());
	}

	Passes.add(createBuiltInImportPass(LLVMBackend::GetInstance()->GetRTModule())); // Inline BI function

	UndefinedExternalFunctions.clear();
	ModulePass* updatePass = createKernelUpdatePass(vectorizerPass, vectFunctions, m_pLLVMContext, UndefinedExternalFunctions);
	Passes.add(updatePass);

#ifdef _DEBUG
	Passes.add(createVerifierPass());
#endif
	Passes.add(createFunctionInliningPass());		// Inline small functions
	Passes.add(createArgumentPromotionPass());		// Scalarize uninlined fn args
	if (!DisableSimplifyLibCalls)
		Passes.add(createSimplifyLibCallsPass());   // Library Call Optimizations
	Passes.add(createInstructionCombiningPass());	// Cleanup for scalarrepl.

	Passes.add(createDeadStoreEliminationPass());  // Delete dead stores
	Passes.add(createAggressiveDCEPass());   // Delete dead instructions
	Passes.add(createCFGSimplificationPass());     // Merge & remove BBs
	Passes.add(createInstructionCombiningPass());	// Cleanup for scalarrepl.

	Passes.add(createVerifierPass());
	for (Module::iterator I = pModule->begin(), E = pModule->end(); I != E; ++I)
		FPasses->run(*I);

	if (NULL != FPasses)
	{
		delete FPasses;
	}

	Passes.run(*pModule);

	getKernelInfoMap(updatePass, m_mapKernelInfo);

	if(!UndefinedExternalFunctions.empty())
	{
		return CL_DEV_BUILD_ERROR;
	}

	if( m_bUseVectorizer )
	{
		VectorizedFunctions.clear();

		SmallVector<int, 16>       vectMinWidths;
		SmallVector<int, 16>       vectMaxWidths;

		getVectorizerWidths((Vectorizer *)vectorizerPass, vectMaxWidths, vectMinWidths);
		assert(vectFunctions.size() == vectMaxWidths.size());
		assert(vectFunctions.size() == vectMinWidths.size());

		for(unsigned int i=0; i < vectFunctions.size(); i++)
		{
			VectorizedFunctions.push_back(FunctionWidthPair(vectFunctions[i], vectMaxWidths[i]));
		}
	}

	return CL_DEV_SUCCESS;
}

LLVMKernel *LLVMProgram::CreateKernel(llvm::Function *pFunc, ConstantArray *pFuncArgs)
{
	if( NULL != pFuncArgs )
	{
		// Check maximum number of arguments to kernel
		std::string ArgStr = pFuncArgs->getAsString();

		int count = 0;

		if(ArgStr != "")
		{
			string::size_type pos = 0;

			pos = ArgStr.find(", ", 0);
			while(pos != string::npos)
			{
				count++;
				pos = ArgStr.find(", ", pos+1);
			}
			count ++;
		}

		if ( (CPU_KERNEL_MAX_ARG_COUNT + KRNL_NUM_CONST_ARGS) < count )
		{
			m_strLastError += "\nToo many arguments in kernel<";
			m_strLastError += pFunc->getNameStr().c_str();
			m_strLastError += ">";
			LOG_ERROR("Build of function <%s> fail, <%s>", pFunc->getNameStr().c_str(), m_strLastError.c_str());
			return NULL;
		}
	}

	// Create new kernel container
	LLVMKernel* pKernel = new LLVMKernel(this);
	if ( NULL == pKernel )
	{
		m_strLastError = "\nOut of memory";
		LOG_ERROR("Build of function <%s> fail, Cannot allocate LLVMKernel", pFunc->getNameStr().c_str());
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
		m_strLastError += "\n";
		m_strLastError += err;
		rc = CL_DEV_BUILD_ERROR;
	}
	if ( CL_DEV_FAILED(rc) )
	{
		pKernel->Release();
		LOG_ERROR("Build of function <%s> fail, <%s>", pFunc->getNameStr().c_str(), m_strLastError.c_str());
		return NULL;
	}

	return pKernel;
}

cl_int LLVMProgram::BuildProgram(const char* pOptions)
{
	LOG_INFO("Entry");
	cl_int rc;
	assert(NULL != m_pMemBuffer);
	
	OclAutoMutex CS(&s_serializingMutex);
	// Create module to put IR in it
	Module *pModule = ParseBitcodeFile(m_pMemBuffer, *m_pLLVMContext, &m_strLastError);
	if ( NULL == pModule )
	{
		m_strLastError = "Failed to parse IR";
		LOG_ERROR("ParseBitcodeFile failed <%s>", m_strLastError.c_str());
		return CL_DEV_INVALID_BINARY;
	}

	rc =  OptimizeProgram(pModule);
	if ( CL_DEV_FAILED(rc) )
	{
		if(!UndefinedExternalFunctions.empty())
		{
			LOG_ERROR("Unimplemented function(s) used" );
			m_strLastError = "Error: unimplemented function(s) used:\n";

			for(int i = 0; i < UndefinedExternalFunctions.size() ; i++)
			{
				m_strLastError += UndefinedExternalFunctions[i] + "\n";
			}

			return rc;
		}

		LOG_ERROR("FAILED to optimize module (first)");
		m_strLastError = "Program optimizations failed";
		delete pModule;
		return rc;
	}

	m_pModule = pModule;

	// if the user requested -dump-opt-llvm, print this module
	if((NULL != pOptions) && !strncmp(pOptions, "-dump-opt-llvm=", 15))
	{
		std::string err;
		std::string fname = pOptions;
		std::string::size_type pos1 = fname.find("\"", 0);
		std::string::size_type pos2 = fname.find("\"", pos1+1);

		if((pos1 != string::npos) && (pos2 != string::npos))
		{
			fname = fname.substr(pos1 + 1, pos2 - pos1 - 1);

			llvm::raw_fd_ostream ostr(fname.c_str(), err);
			if(err.empty())
			{
				m_pModule->print(ostr, 0);
			}
		}
	}

	// Now after JIT is up and setup with IR, we scan module for kernels
	LOG_DEBUG("Start iterating over kernels");
	GlobalVariable *metadata = pModule->getGlobalVariable("opencl_metadata");
	if ( NULL == metadata )
	{
		m_strLastError = "No kernels in program";
		return CL_DEV_SUCCESS;
	}

	m_strLastError = "Build started\n";

	ConstantArray *init = dyn_cast<ConstantArray>(metadata->getInitializer());
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
		GlobalVariable* pGlbVar = dyn_cast<GlobalVariable>(elt->getOperand(5)->stripPointerCasts());
		ConstantArray *pFuncArgs;
		if ( (NULL == pGlbVar) || (NULL == (pFuncArgs = dyn_cast<ConstantArray>(pGlbVar->getInitializer()))) )
		{
			m_strLastError = "Invalid argument's map";
			FreeMap();
			LOG_ERROR("Build of function <%s> fail, <%s>", pFunc->getNameStr().c_str(), m_strLastError.c_str());
			return CL_DEV_BUILD_ERROR;
		}

		LLVMKernel *pKernel = CreateKernel(pFunc, pFuncArgs);
		if ( NULL == pKernel )
		{
			m_strLastError += "\nCan't create kernel:";
			m_strLastError += pFunc->getName();
			FreeMap();
			return CL_DEV_BUILD_ERROR;
		}

		// Check if vectorized kernel present
		// Store kernel to map
		m_mapKernels[pKernel->GetKernelName()] = pKernel;

		const llvm::Type *vTypeHint = NULL;
		pGlbVar = dyn_cast<GlobalVariable>(elt->getOperand(1)->stripPointerCasts());

		if(pGlbVar)
		{
			if(dyn_cast<llvm::PointerType>(pGlbVar->getType()))
			{
				vTypeHint = dyn_cast<llvm::PointerType>(pGlbVar->getType())->getElementType();
			}
		}

		bool dontVectorize = false;

		if(NULL != vTypeHint)
		{
			//currently if the vector_type_hint attribute is set
			//we disable vectorizing if type is other than int or float
			if( !vTypeHint->isIntegerTy(32) && !vTypeHint->isFloatTy() )
			{
				dontVectorize = true;
			}
		}

		if(m_bUseVectorizer)
		{
			assert(vecIter != VectorizedFunctions.end());
			if(NULL != vecIter->first && !dontVectorize)
			{
				// We don't need to pass argument list here
				LLVMKernel *pVecKernel = CreateKernel(vecIter->first, NULL);
				if ( NULL == pVecKernel )
				{
					m_strLastError = "Can't create vectorized kernel:";
					m_strLastError += pFunc->getName();
					FreeMap();
					return CL_DEV_BUILD_ERROR;
				}

				pKernel->setVectorizerProperties(pVecKernel, vecIter->second);
				m_strLastError += "Kernel <";
				m_strLastError += pKernel->GetKernelName();
				m_strLastError += "> was successfully vectorized\n";
			}
			if ( NULL == vecIter->first )
			{
				m_strLastError += "Kernel <";
				m_strLastError += pKernel->GetKernelName();
				m_strLastError += "> was not vectorized\n";
			}
			else if ( dontVectorize )
			{
				m_strLastError += "Vectorization of kernel <";
				m_strLastError += pKernel->GetKernelName();
				m_strLastError += "> was disabled by the developer\n";
			}
			vecIter++;
		}
	}
	LOG_DEBUG("Iterating completed");

	m_strLastError += "Done.";
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
		*pSize = stLogSize + 1;
		return CL_DEV_SUCCESS;
	}
	assert(*pSize >= stLogSize);

	memcpy_s(pLog, *pSize, m_strLastError.c_str(), stLogSize + 1);

	return CL_DEV_SUCCESS;
}

// ------------------------------------------------------------------------------
// Copies internally stored container into provided buffer
cl_int LLVMProgram::GetContainer(size_t *pSize, cl_prog_container_header* pContainer) const
{
	if ( NULL == m_pMemBuffer )
	{
		return CL_DEV_INVALID_BINARY;
	}

	size_t stSize = m_pMemBuffer->getBufferSize() +
		sizeof(cl_prog_container_header)+ sizeof(cl_llvm_prog_header);
	if ( NULL == pContainer)
	{
		assert(pSize);
		*pSize = stSize;
		return CL_DEV_SUCCESS;
	}
	assert(*pSize >= stSize);

	// Copy container header
	memcpy(pContainer, &m_ContainerInfo, sizeof(cl_prog_container_header));
	// Copy LLVM program info
	memcpy(((char*)pContainer)+sizeof(cl_prog_container_header), &m_ProgHeader, sizeof(cl_llvm_prog_header));
	// Copy container content
	memcpy(((char*)pContainer)+sizeof(cl_prog_container_header)+sizeof(cl_llvm_prog_header),
		m_pMemBuffer->getBufferStart(), m_pMemBuffer->getBufferSize());

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
	GlobalVariable *metadata = m_pModule->getGlobalVariable("opencl_metadata");
	assert ( metadata );

	ConstantArray *init = dyn_cast<ConstantArray>(metadata->getInitializer());
	for (unsigned i = 0, e = init->getType()->getNumElements(); i != e; ++i) 
	{
		// Obtain kernel function from annotation
		ConstantStruct *elt = cast<ConstantStruct>(init->getOperand(i));
		Function *pFuncVal = dyn_cast<Function>(elt->getOperand(0)->stripPointerCasts());
		if ( NULL == pFuncVal )
		{
			continue;	// Not a function pointer
		}

		if ( !pFuncVal->getName().compare(szFuncName) )
		{
			return true;
		}
	}

	// Function not found
	return false;
}

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
#include "ocl_rt.h"

#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Constants.h"
#include "llvm/ModuleProvider.h"
#include "llvm/DerivedTypes.h"

#include "llvm_program.h"
#include "cpu_kernel.h"


using namespace Intel::OpenCL;
using namespace Intel::OpenCL::CPUDevice;
using namespace Intel::OpenCL::Utils;
using namespace llvm;

// Compilation Log strings
static const char szNone[] = "None";
static const char szBuilding[] = "Building";
static const char szError[] = "Error";

// List of the intrinsics to be substitued by internal function
static char*	szWIIntrinNames[] =
{
	"gid",
	"ndims",
	"gdim",
	"ldim",
	"lid",
	"tgdim",
	"tgid"
};
static unsigned int	uiWIIntrinCount = sizeof(szWIIntrinNames)/sizeof(char*);


// Contructor/Destructor
LLVMProgram::LLVMProgram() :
	m_clBuildStatus(CL_BUILD_NONE),	m_pBuildingThread(NULL), m_pMemBuffer(NULL), m_pExecEngine(NULL)
{
	memset(&m_ContainerInfo, 0, sizeof(cl_prog_container));
	m_strLastError = szNone;
}

LLVMProgram::~LLVMProgram()
{
	if ( NULL != m_pBuildingThread )
	{
		delete m_pBuildingThread;
	}

	FreeMap();
}

cl_int LLVMProgram::CreateProgram(const cl_prog_container*	pContainer)
{
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
	m_pMemBuffer = MemoryBuffer::getMemBufferCopy(pIR, pIR+pContainer->container_size);
	if ( NULL == m_pMemBuffer )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	// Store container inforamation
	memcpy(&m_ContainerInfo, pContainer, sizeof(cl_prog_container));
	m_ContainerInfo.container = m_pMemBuffer->getBufferStart();

	return CL_DEV_SUCCESS;
}

cl_int LLVMProgram::BuildProgram(fn_clDevBuildStatusUpdate* pfnCallBack, cl_dev_program progId, void* pUserData)
{
	// Initiate log string to error
	m_strLastError = szError;

	// Trying build the program again?
	if ( CL_BUILD_NONE != m_clBuildStatus )
	{
		return CL_DEV_INVALID_OPERATION;
	}

	if ( CL_BUILD_IN_PROGRESS == m_clBuildStatus )
	{
		return CL_DEV_STILL_BUILDING;
	}

	// Create building thread
	m_pBuildingThread = new LLVMProgramThread(this, pfnCallBack, progId, pUserData);
	if ( NULL == m_pBuildingThread )
	{
		m_clBuildStatus = CL_BUILD_ERROR;
		return CL_DEV_OUT_OF_MEMORY;
	}

	m_clBuildStatus = CL_BUILD_IN_PROGRESS;
	m_strLastError = szBuilding;
	int iRet = m_pBuildingThread->Start();
	if ( 0 != iRet )
	{
		m_strLastError = szError;
		m_clBuildStatus = CL_BUILD_ERROR;
		return CL_DEV_ERROR_FAIL;
	}


	return CL_DEV_SUCCESS;
}

// ------------------------------------------------------------------------------
// Quaries program build log
const char*	LLVMProgram::GetBuildLog() const
{
	return m_strLastError.c_str();
}

size_t LLVMProgram::GetContainerSize() const
{
	if ( NULL == m_pMemBuffer )
	{
		return 0;
	}

	return m_pMemBuffer->getBufferSize() + sizeof(cl_prog_container);
}

// ------------------------------------------------------------------------------
// Copies internally stored container into provided buffer
cl_int LLVMProgram::CopyContainer(void* pBuffer, size_t stSize) const
{
	if ( NULL == m_pMemBuffer )
	{
		return CL_DEV_INVALID_BINARY;
	}

	if ( (NULL == pBuffer) || (stSize < m_pMemBuffer->getBufferSize() + sizeof(cl_prog_container) ) )
	{
		return CL_DEV_INVALID_VALUE;
	}

	// Copy container header
	memcpy(pBuffer, &m_ContainerInfo, sizeof(cl_prog_container));
	// Copy container content
	memcpy(((char*)pBuffer)+sizeof(cl_prog_container), m_pMemBuffer->getBufferStart(), m_pMemBuffer->getBufferSize());
	// container pointer for user must be NULL
	((cl_prog_container*)pBuffer)->container = NULL;

	return CL_DEV_SUCCESS;
}

// ------------------------------------------------------------------------------
// Retrieves a pointer to a function descriptor by function short name
cl_int LLVMProgram::GetKernel(const char* pName, const ICLDevKernel* *pKernel) const
{
	if ( CL_BUILD_SUCCESS != m_clBuildStatus )
	{
		return CL_DEV_STILL_BUILDING;
	}

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
cl_int LLVMProgram::GetAllKernels(const ICLDevKernel* *pKernels, unsigned int uiCount, unsigned int *puiRetCount) const
{
	if ( CL_BUILD_SUCCESS != m_clBuildStatus )
	{
		return CL_DEV_STILL_BUILDING;
	}

	if ( NULL != puiRetCount )
	{
		*puiRetCount = m_mapKernels.size();
	}

	if ( (NULL == pKernels) &&  (0 == uiCount) )
	{
		if ( NULL == puiRetCount )
		{
			return CL_DEV_INVALID_VALUE;
		}

		return CL_DEV_SUCCESS;
	}

	if ( (NULL == pKernels) ||  (m_mapKernels.size() > uiCount) )
	{
		return CL_DEV_INVALID_VALUE;
	}

	TKernelMap::const_iterator it;
	unsigned int i = 0;
	for(it = m_mapKernels.begin(); it!=m_mapKernels.end();++it)
	{
		pKernels[i] = it->second;
		++i;
	}

	return CL_DEV_SUCCESS;
}

const char*	SubstituteIntrinName(const char* szName)
{
	if ( strncmp(szName, "llvm.x86.", 9) )
	{
		return NULL;
	}

	for(unsigned int i=0; i<uiWIIntrinCount; ++i)
	{
		if ( !strcmp(&szName[9], szWIIntrinNames[i] ) )
		{
			return szWIIntrinNames[i];
		}
	}

	return NULL;
}

// ------------------------------------------------------------------------------
//	Parses libary information and retrieves/builds function descripotrs
cl_int LLVMProgram::LoadProgram()
{
	cl_kernel_argument	pArgs[MAX_ARG_COUNT];
	unsigned int		uiArgCount;

	assert(NULL != m_pMemBuffer);

	// Create module to put IR in it
	Module *pModule = ParseBitcodeFile(m_pMemBuffer, &m_strLastError);
	if ( NULL == pModule )
	{
		m_clBuildStatus = CL_BUILD_ERROR;

		return CL_DEV_INVALID_BINARY;
	}

	// Now,  create JIT
	ExistingModuleProvider *pMP = new ExistingModuleProvider(pModule);
	if ( NULL == pMP )
	{
		m_clBuildStatus = CL_BUILD_ERROR;
		m_strLastError = szError;
		delete pModule;
		return CL_DEV_OUT_OF_MEMORY;
	}

	ExecutionEngine *m_pExecEngine = ExecutionEngine::create(pMP, false, &m_strLastError);
	if ( NULL == m_pExecEngine )
	{
		m_clBuildStatus = CL_BUILD_ERROR;
		delete pMP;
		return CL_DEV_INVALID_BINARY;
	}
	
	// Now after JIT is up and setup with IR, we scan module for kernels
	Module::FunctionListType::iterator func_it = pModule->getFunctionList().begin();
	while ( func_it != pModule->getFunctionList().end() )
	{
		if ( func_it->isDeclaration() || 
			!((GlobalValue::DLLExportLinkage == func_it->getLinkage()) ||
			(GlobalValue::ExternalLinkage == func_it->getLinkage())) )
		{
			++func_it;
			// Internal function -> not required
			continue;
		}

		// Create new kernel container
		CPUKernel* pKernel = new CPUKernel;
		if ( NULL == pKernel )
		{
			m_clBuildStatus = CL_BUILD_ERROR;
			FreeMap();
			return CL_DEV_OUT_OF_MEMORY;
		}

		// Set function name
		pKernel->SetName(func_it->getNameStart(), func_it->getNameLen() );
		uiArgCount = 0;

		Function::arg_iterator arg_it = func_it->arg_begin();
		while (arg_it != func_it->arg_end() )
		{
			// Set argument sizes
			switch (arg_it->getType()->getTypeID())
			{
			case Type::FloatTyID:
				pArgs[uiArgCount].type = CL_KRNL_ARG_FLOAT;
				pArgs[uiArgCount].size_in_bytes = sizeof(float);
				break;

			case Type::PointerTyID:
				{
				// Detect pointer qualifier
				const PointerType *PTy = cast<PointerType>(arg_it->getType());
				pArgs[uiArgCount].size_in_bytes = 0;
				switch (PTy->getAddressSpace())
				{
				case 0: case 1:	// Global Address space
					pArgs[uiArgCount].type = CL_KRNL_ARG_PTR_GLOBAL;
					break;
				case 2:
					pArgs[uiArgCount].type = CL_KRNL_ARG_PTR_CONST;
					break;
				case 3: // Local Address space
					pArgs[uiArgCount].type = CL_KRNL_ARG_PTR_LOCAL;
					break;

				default:
					assert(0);
				}}
				break;

			case Type::IntegerTyID:
				{
				const IntegerType *ITy = cast<IntegerType>(arg_it->getType());
				pArgs[uiArgCount].type = CL_KRNL_ARG_INT;
				pArgs[uiArgCount].size_in_bytes = ITy->getBitWidth();
				}
				break;
			}

			++uiArgCount;
			++arg_it;
		}
		// Set Kernel arguments
		pKernel->SetArgumentList(pArgs, uiArgCount);

		// Apple LLVM-IR workaround
		// Apple implemened some internal OCL function as intrisics (llvm.*)
		// We don't want to implement those commands in LLVM backend,
		// thus we need to substitute intrinsic calls to regular function calls
		Function::BasicBlockListType::iterator bb_it = func_it->getBasicBlockList().begin();
		while ( bb_it != func_it->getBasicBlockList().end() )
		{
			BasicBlock::InstListType::iterator inst_it = bb_it->getInstList().begin();
			while ( inst_it != bb_it->getInstList().end() )
			{
				if ( inst_it->getOpcode() == 0x2B )
				{
					const char* szNewName = SubstituteIntrinName(inst_it->getOperand(0)->getNameStart());
					if ( NULL != szNewName )
					{
						inst_it->getOperand(0)->setName(szNewName);
					} else if ( !strcmp(inst_it->getOperand(0)->getNameStart(), "__dotf4") )
					{
						inst_it->getOperand(0)->setName("@dotf4@32");
					}
					// TODO: Add another functions below
				}
				++inst_it;
			}
			++bb_it;
		}

		// Acquire pointer to function
		void *test = m_pExecEngine->getPointerToFunction(func_it);

		pKernel->SetFuncPtr(test);

		// Store kernel to map
		m_mapKernels[pKernel->GetKernelName()] = pKernel;

		++func_it;
	}

	m_clBuildStatus = CL_BUILD_SUCCESS;

	return CL_DEV_SUCCESS;
}

void LLVMProgram::FreeMap()
{
	// Free descriptors map
	TKernelMap::iterator it;
	for(it = m_mapKernels.begin(); it != m_mapKernels.end(); ++it)
	{
		delete it->second;
	}

	m_mapKernels.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////
// DLLProgramThread implementation
LLVMProgramThread::LLVMProgramThread(LLVMProgram* pProgram, fn_clDevBuildStatusUpdate* pfnCallBack,
	cl_dev_program progId, void* pUserData) : 
m_pProgram(pProgram), m_pfnCallBack(pfnCallBack), m_progId(progId), m_pUserData(pUserData)
{
}

int LLVMProgramThread::Run()
{
	cl_int ret = m_pProgram->LoadProgram();

	cl_build_status status = CL_DEV_SUCCEEDED(ret) ? CL_BUILD_SUCCESS : CL_BUILD_ERROR;
	m_pfnCallBack(m_progId, m_pUserData, status);

	Clean();

	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////
// OCL LLVM inline functions
extern "C" __declspec(dllexport) int gid(int i)
{
	return (int)get_global_id(i);
}

extern "C" __declspec(dllexport) int lid(int i)
{
	return (int)get_local_id(i);
}

extern "C" __declspec(dllexport) int ndims()
{
	return (int)get_work_dim();
}

extern "C" __declspec(dllexport) int gdim(int i)
{
	return (int)get_global_size(i);
}

extern "C" __declspec(dllexport) int ldim(int i)
{
	return (int)get_local_size(i);
}

extern "C" __declspec(dllexport) int tgdim(int i)
{
	return (int)get_num_groups(i);
}

extern "C" __declspec(dllexport) int tgid(int i)
{
	return (int)get_group_id(i);
}


extern "C" __declspec(dllexport) float __fastcall dotf4(__m128 a, __m128 b)
{
	a = _mm_mul_ps(a, b);
	a = _mm_hadd_ps(a, a);
	a = _mm_hadd_ps(a, a);
	return  _mm_cvtss_f32(a);
}

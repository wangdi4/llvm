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

#include "llvm_program.h"
#include "llvm_kernel.h"
#include "logger.h"

#include <string>
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

// External declarations
extern ExecutionEngine* g_pExecEngine;
extern DECLARE_LOGGER_CLIENT;

// Constructor/Destructor
LLVMProgram::LLVMProgram() :
m_pMemBuffer(NULL), m_pModuleProvider(NULL),
m_bBarrierDecl(false), m_bAsyncCopy(false), m_bPrefetchDecl(false)
{
	memset(&m_ContainerInfo, 0, sizeof(cl_prog_container));
	m_strLastError = szNone;
}

cl_int LLVMProgram::CreateProgram(const cl_prog_container* IN pContainer, ICLDevBackendProgram** OUT pProgram)
{
	LOG_INFO("Enter");
	// Allocate Program object
	LLVMProgram*	pMyProg = new LLVMProgram();
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
		g_pExecEngine->deleteModuleProvider(m_pModuleProvider);
	}
	if (NULL != m_pMemBuffer)
	{
		delete m_pMemBuffer;
	}
	delete this;
}

cl_int LLVMProgram::BuildProgram(const char* pOptions)
{
	LOG_INFO("Entry");
	// Initiate log string to error
	m_strLastError = szError;

	assert(NULL != m_pMemBuffer);

	// Create module to put IR in it
	Module *pModule = ParseBitcodeFile(m_pMemBuffer, &m_strLastError);
	if ( NULL == pModule )
	{
		LOG_ERROR("ParseBitcodeFile failed <%s>", m_strLastError.c_str());
		return CL_DEV_INVALID_BINARY;
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

	LOG_DEBUG("Before AddWIInfoDeclarations");
	// Add declarations of modified WI info functions
	AddWIInfoDeclarations();

	LOG_DEBUG("Before addModuleProvider()");
	g_pExecEngine->addModuleProvider(m_pModuleProvider);

	// Now after JIT is up and setup with IR, we scan module for kernels
	LOG_DEBUG("Start iterating over functions");
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

		// Check maximum number of argumnets to kernel
		if ( CPU_KERNEL_MAX_ARG_COUNT < func_it->arg_size() )
		{
			m_strLastError = "Too many arguments in function";
			FreeMap();
			LOG_ERROR("Build of function <%s> fail, <%s>", func_it->getName().c_str(), m_strLastError.c_str());
			return CL_DEV_BUILD_ERROR;
		}

		// Acquire pointer to function
		void *pKernelFunc = NULL;

		// Create new kernel container
		LLVMKernel* pKernel = new LLVMKernel(this);
		if ( NULL == pKernel )
		{
			m_strLastError = "Out of Memory";
			FreeMap();
			LOG_ERROR("Build of function <%s> fail, Cannot allocate LLVMKernel", func_it->getName().c_str());
			return CL_DEV_OUT_OF_MEMORY;
		}

		cl_int rc;
		// Try to create kernel, on error
		try
		{
			rc = pKernel->ParseLLVM(func_it);
		}
		catch( std::string err )
		{
			m_strLastError = err;
			rc = CL_DEV_BUILD_ERROR;
		}
		if ( CL_DEV_FAILED(rc) )
		{
			pKernel->Release();
			FreeMap();
			LOG_ERROR("Build of function <%s> fail, <%s>", func_it->getName().c_str(), m_strLastError.c_str());
			return rc;
		}
		// Store kernel to map
		m_mapKernels[pKernel->GetKernelName()] = pKernel;

		++func_it;
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

// ------------------------------------------------------------------------------
void LLVMProgram::AddWIInfoDeclarations()
{
	Module* pModule = m_pModuleProvider->getModule();

	// Detect size_t size
	unsigned int uiSizeT = pModule->getPointerSize()*32;
/*
	struct sLocalId 
	{
		size_t	Id[MAX_WORK_DIM];
	};
*/
	// Create Work Group/Work Item info structures
	std::vector<const Type*> members;
	members.push_back(ArrayType::get(IntegerType::get(uiSizeT), MAX_WORK_DIM)); // Local Id's
	StructType* pLocalId = StructType::get(members, true);
	pModule->addTypeName("struct.LocalId", pLocalId);

	/*
	struct sWorkInfo
	{
		unsigned int	uiWorkDim;
		size_t			GlobalOffset[MAX_WORK_DIM];
		size_t			GlobalSize[MAX_WORK_DIM];
		size_t			LocalSize[MAX_WORK_DIM];
		size_t			WGNumber[MAX_WORK_DIM];
	};
*/
	members.clear();
	members.push_back(IntegerType::get(32));
	members.push_back(ArrayType::get(IntegerType::get(uiSizeT), MAX_WORK_DIM)); // Global offset
	members.push_back(ArrayType::get(IntegerType::get(uiSizeT), MAX_WORK_DIM)); // Global size
	members.push_back(ArrayType::get(IntegerType::get(uiSizeT), MAX_WORK_DIM)); // WG size/Local size
	members.push_back(ArrayType::get(IntegerType::get(uiSizeT), MAX_WORK_DIM)); // Number of groups
	StructType* pWorkDimType = StructType::get(members, true);
	pModule->addTypeName("struct.WorkDim", pWorkDimType);
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


void LLVMProgram::AddBarrierDeclaration()
{
	if ( m_bBarrierDecl )
		return;
	llvm::Module* pModule = m_pModuleProvider->getModule();

	std::vector<const Type*> params;
	params.push_back(IntegerType::get(32));
	params.push_back(PointerType::get(IntegerType::get(32), 0));
	FunctionType* pNewType = FunctionType::get(Type::VoidTy, params, false);
	Function* pNewFunc = Function::Create(pNewType, (GlobalValue::LinkageTypes)0, "lbarrier", pModule);
	m_bBarrierDecl = true;
}

//event_t lasync_work_group_copy(void* pDst, void* pSrc, size_t numElem, event_t event,
//							   size_t elemSize, LLVMExecMultipleWIWithBarrier* *ppExec);
void LLVMProgram::AddAsyncCopyDeclaration()
{
	if ( m_bAsyncCopy )
		return;

	llvm::Module* pModule = m_pModuleProvider->getModule();

	unsigned int uiSizeT = pModule->getPointerSize()*32;

	std::vector<const Type*> params;
	params.push_back(PointerType::get(IntegerType::get(8), 0));
	params.push_back(PointerType::get(IntegerType::get(8), 0));
	params.push_back(IntegerType::get(uiSizeT));
	params.push_back(IntegerType::get(uiSizeT));
	params.push_back(IntegerType::get(uiSizeT));
	params.push_back(PointerType::get(IntegerType::get(32), 0));
	FunctionType* pNewType = FunctionType::get(IntegerType::get(uiSizeT), params, false);
	Function::Create(pNewType, (GlobalValue::LinkageTypes)0, "lasync_wg_copy_l2g", pModule);
	Function::Create(pNewType, (GlobalValue::LinkageTypes)0, "lasync_wg_copy_g2l", pModule);

	// Add wait
	params.clear();
	params.push_back(IntegerType::get(32));
	params.push_back(PointerType::get(IntegerType::get(32), 0));
	params.push_back(PointerType::get(IntegerType::get(32), 0));
	FunctionType* pWaitType = FunctionType::get(Type::VoidTy, params, false);
	Function::Create(pWaitType, (GlobalValue::LinkageTypes)0, "lwait_group_events", pModule);

	m_bAsyncCopy = true;
}

void LLVMProgram::AddPrefetchDeclaration()
{
	if ( m_bPrefetchDecl )
		return;
	llvm::Module* pModule = m_pModuleProvider->getModule();

	unsigned int uiSizeT = pModule->getPointerSize()*32;

	std::vector<const Type*> params;
	// Source Pointer
	params.push_back(PointerType::get(IntegerType::get(8), 0));
	// Number of elements
	params.push_back(IntegerType::get(uiSizeT));
	// Element size
	params.push_back(IntegerType::get(uiSizeT));
	FunctionType* pNewType = FunctionType::get(Type::VoidTy, params, false);
	Function* pNewFunc = Function::Create(pNewType, (GlobalValue::LinkageTypes)0, "lprefetch", pModule);
	m_bPrefetchDecl = true;
}

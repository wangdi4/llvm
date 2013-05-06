// Copyright (c) 2006-2012 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  mkl_kernels.cpp
///////////////////////////////////////////////////////////


#include "stdafx.h"

#include "builtin_kernels.h"
#include <assert.h>
#include <cl_sys_defines.h>
#include <string>

using namespace Intel::OpenCL::BuiltInKernels;
using namespace Intel::OpenCL::Utils;

cl_dev_err_code BuiltInProgram::ParseFunctionList(const char* szBuiltInKernelList)
{
	if ( 0 == strlen(szBuiltInKernelList) )
	{
		return CL_DEV_INVALID_OPERATION;
	}

	const char* pCurrFuncName = szBuiltInKernelList;
	const char* pNameListEnd = szBuiltInKernelList + strlen(szBuiltInKernelList);
	while ( pCurrFuncName < pNameListEnd )
	{
		const char* pNextNameSeparator = strchr(pCurrFuncName, ';');
		if ( NULL == pNextNameSeparator )
		{
			pNextNameSeparator = pCurrFuncName+strlen(pCurrFuncName);
		}

		const size_t stNameSize = (size_t)(pNextNameSeparator-pCurrFuncName);
		
		std::string funcName;
		
		funcName.resize(stNameSize);
		MEMCPY_S(&funcName[0], stNameSize, pCurrFuncName, stNameSize);

		if ( m_mapKernels.find(funcName) == m_mapKernels.end() )
		{
			IBuiltInKernel* pBIKernel;
			cl_dev_err_code err = BuiltInKernelRegistry::GetInstance()->CreateBuiltInKernel(funcName.c_str(), &pBIKernel);
			if ( CL_DEV_FAILED(err) )
			{
				return err;
			}
			m_mapKernels[funcName] = pBIKernel;
			m_listKernels.push_back(pBIKernel);
		}
		// Nothing to do on else. If kernel is not supported by the device, just don't add it to the list.
		// Other devices might support it

		pCurrFuncName = pNextNameSeparator+1;
	}

	return m_mapKernels.size() > 0 ? CL_DEV_SUCCESS : CL_DEV_INVALID_KERNEL_NAME;
}

cl_dev_err_code BuiltInProgram::GetKernelByName( const char* pKernelName, const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_** ppKernel) const
{
	if ( NULL == ppKernel )
	{
		assert(0 && "ppKernel is not expected to be NULL");
		return CL_DEV_INVALID_OPERATION;
	}

	BIKernelsMap_t::const_iterator it = m_mapKernels.find(pKernelName);
	if ( it == m_mapKernels.end() )
	{
		return CL_DEV_INVALID_KERNEL_NAME;
	}

	*ppKernel = it->second;
	return CL_DEV_SUCCESS;
}

cl_dev_err_code	BuiltInProgram::GetKernel( int kernelIndex, const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_** ppKernel) const
{
	if ( kernelIndex >= (int)m_listKernels.size() )
	{
		assert(0 && "Access OOB of kernel list");
		return CL_DEV_INVALID_OPERATION;
	}

	if ( NULL == ppKernel )
	{
		assert(0 && "ppKernel is not expected to be NULL");
		return CL_DEV_INVALID_OPERATION;
	}

	*ppKernel = m_listKernels[kernelIndex];
	return CL_DEV_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
BuiltInKernelRegistry*	BuiltInKernelRegistry::g_pMKLRegistery = NULL;

BuiltInKernelRegistry* BuiltInKernelRegistry::GetInstance()
{
	if ( NULL == g_pMKLRegistery )
	{
		g_pMKLRegistery = new BuiltInKernelRegistry();
	}

	return g_pMKLRegistery;
}

void BuiltInKernelRegistry::RegisterBuiltInKernel(const char* szBIKernelName, fn_BuiltInFunctionCreate* pCreator)
{
	assert(m_mapKernelCreators.find(szBIKernelName) == m_mapKernelCreators.end() && "The function can't be registered twice");

	m_stKernelNameStrLength += (strlen(szBIKernelName)+1);	// Count also for ';'
	m_mapKernelCreators[szBIKernelName] = pCreator;
}

void BuiltInKernelRegistry::GetBuiltInKernelList(char* szBIKernelList, size_t stSize) const
{
	assert(0 != stSize && "Input string can't be empty");
	if ( 0 == stSize )
	{
		return;
	}
	KernelCreatorMap_t::const_iterator it;
	size_t stTotalSize = 0;
	szBIKernelList[0] = '\0';
	for(it = m_mapKernelCreators.begin(); it != m_mapKernelCreators.end(); ++it)
	{
		if ( stTotalSize+(it->first.length()+1) > stSize )
			break;
		if ( '\0' != szBIKernelList[0] )
		{
			STRCPY_S(&szBIKernelList[stTotalSize], stSize-stTotalSize, ";");
			++stTotalSize;
		}
		STRNCPY_S(&szBIKernelList[stTotalSize], stSize-stTotalSize, it->first.c_str(), it->first.length());
		stTotalSize += it->first.length();
	}
}

cl_dev_err_code BuiltInKernelRegistry::CreateBuiltInKernel(const char* szMKLFuncName, IBuiltInKernel* *pMKLExecutor) const
{
	KernelCreatorMap_t::const_iterator it = m_mapKernelCreators.find(szMKLFuncName);
	if ( m_mapKernelCreators.end() == it )
	{
		return CL_DEV_INVALID_KERNEL_NAME;
	}

	cl_dev_err_code err = (it->second)(pMKLExecutor);

	return err;
}

cl_dev_err_code BuiltInKernelRegistry::CreateBuiltInProgram(const char* szKernelList, Intel::OpenCL::DeviceBackend::ICLDevBackendProgram_* *ppProgram)
{
	BuiltInProgram* pNewProgram = new BuiltInProgram;

	if ( NULL == pNewProgram )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	cl_dev_err_code err = pNewProgram->ParseFunctionList(szKernelList);
	if ( CL_DEV_FAILED(err) )
	{
		delete pNewProgram;
		return err;
	}

	assert(ppProgram != NULL && "NULL is not expected");
	*ppProgram = pNewProgram;

	return CL_DEV_SUCCESS;
}

cl_kernel_arg_address_qualifier Intel::OpenCL::BuiltInKernels::ArgType2AddrQual(cl_kernel_arg_type type)
{
	switch(type)
	{
	case CL_KRNL_ARG_INT: case CL_KRNL_ARG_UINT: case CL_KRNL_ARG_FLOAT: case CL_KRNL_ARG_DOUBLE:
	case CL_KRNL_ARG_VECTOR: case CL_KRNL_ARG_SAMPLER: case CL_KRNL_ARG_COMPOSITE:
		return CL_KERNEL_ARG_ADDRESS_PRIVATE;

	case CL_KRNL_ARG_PTR_LOCAL:
		return CL_KERNEL_ARG_ADDRESS_LOCAL;

	case CL_KRNL_ARG_PTR_CONST:
		return CL_KERNEL_ARG_ADDRESS_CONSTANT;

	case CL_KRNL_ARG_PTR_GLOBAL:
	default:
		return CL_KERNEL_ARG_ADDRESS_GLOBAL;

	}
}

#define INIT_NUM_OF_EVENTS 10

OMPExecutorThread::OMPExecutorThread(unsigned int uiNumOfThreads) :
	OclThread("MKLExecutor")
{
	m_StartEvent.Init(true); // Auto-reset event

	// Pre-allocate the pool of OS events
	for(int i=0;i<INIT_NUM_OF_EVENTS;++i)
	{
		OclOsDependentEvent* pEvent = new OclOsDependentEvent();
		if ( NULL != pEvent )
		{
			pEvent->Init(true);
			m_OSEventPool.PushBack(pEvent);
		}
	}
}

OMPExecutorThread::~OMPExecutorThread()
{
	OclOsDependentEvent* pEvent;
	while ( m_OSEventPool.TryPop(pEvent) )
	{
			delete pEvent;
	}
}

int OMPExecutorThread::Join()
{
	if(m_running)
	{
		if (0 != m_join.test_and_set(0, 1))
		{
			return THREAD_RESULT_FAIL;
		}
		m_StartEvent.Signal();
		return WaitForCompletion();
	}
	
	return THREAD_RESULT_SUCCESS;
}

RETURN_TYPE_ENTRY_POINT OMPExecutorThread::Run()
{
	while (!m_join)
	{
		ExecutionRecord mklTask;
		bool exists = m_ExecutionQueue.TryPop(mklTask);
		if ( exists )
		{
			mklTask.first->Execute();
			mklTask.second->Signal();
			continue;
		}
		m_StartEvent.Wait();
	}

	return 0;
}

cl_dev_err_code OMPExecutorThread::Execute(Intel::OpenCL::BuiltInKernels::IBuiltInKernelExecutor& kernelToExecute)
{
	// Check if we have allocted OS event
	OclOsDependentEvent* pEvent = NULL;

	bool exists = m_OSEventPool.TryPop(pEvent);
	if ( !exists )
	{
		pEvent = new OclOsDependentEvent();
		if ( NULL != pEvent )
		{
			pEvent->Init(true);
		}
	}
	if ( NULL == pEvent )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	m_ExecutionQueue.PushBack(ExecutionRecord(&kernelToExecute, pEvent));
	m_StartEvent.Signal();

	// Wait for execition
	pEvent->Wait();

	// Return event to the pool
	m_OSEventPool.PushBack(pEvent);
	return CL_DEV_SUCCESS;
}

Intel::OpenCL::BuiltInKernels::OMPExecutorThread* Intel::OpenCL::BuiltInKernels::OMPExecutorThread::Create(unsigned int uiNumOfWorkers)
{
	Intel::OpenCL::BuiltInKernels::OMPExecutorThread* pNewThread = new OMPExecutorThread(uiNumOfWorkers);
	return pNewThread;
}
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

using namespace Intel::OpenCL::CPUDevice;

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
		char* pFuncName = NULL;
		const char* pNextNameSeparator = strchr(pCurrFuncName, ';');
		if ( NULL != pNextNameSeparator )
		{
			size_t stNameSize = (size_t)(pNextNameSeparator-pCurrFuncName);
			pFuncName = (char*)alloca(stNameSize+1);
			assert(NULL!=pFuncName && "alloca() always MUST success");
			MEMCPY_S(pFuncName, stNameSize, pCurrFuncName, stNameSize);
			pFuncName[stNameSize]='\0';
		}
		else
		{
			pFuncName = (char*)pCurrFuncName;
			pNextNameSeparator = pFuncName+strlen(pCurrFuncName);
		}

		if ( m_mapKernels.find(pFuncName) == m_mapKernels.end() )
		{
			IBuiltInKernel* pBIKernel;
			cl_dev_err_code err = BuiltInKernelRegistry::GetInstance()->CreateBuiltInKernel(pFuncName, &pBIKernel);
			if ( CL_DEV_FAILED(err) )
			{
				return err;
			}
			m_mapKernels[pFuncName] = pBIKernel;
			m_listKernels.push_back(pBIKernel);
		}
		pCurrFuncName = pNextNameSeparator+1;
	}

	return m_mapKernels.size() > 0 ? CL_DEV_SUCCESS : CL_DEV_INVALID_KERNEL_NAME;
}

cl_dev_err_code BuiltInProgram::GetKernelByName( const char* pKernelName, const ICLDevBackendKernel_** ppKernel) const
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

cl_dev_err_code	BuiltInProgram::GetKernel( int kernelIndex, const ICLDevBackendKernel_** ppKernel) const
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
	KernelCreatorMap_t::const_iterator it;
	size_t stTotalSize = 0;
	szBIKernelList[0] = '\0';
	for(it = m_mapKernelCreators.begin(); it != m_mapKernelCreators.end(); ++it)
	{
		if ( stTotalSize+(it->first.length()+1) > stSize )
			break;
		if ( '\0' != szBIKernelList[0] )
		{
			STRCAT_S(&szBIKernelList[stTotalSize], stSize-stTotalSize, ";");
			++stTotalSize;
		}
		STRCAT_S(&szBIKernelList[stTotalSize], stSize, &(it->first.at(0)));
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

cl_dev_err_code BuiltInKernelRegistry::CreateBuiltInProgram(const char* szKernelList, ICLDevBackendProgram_* *ppProgram)
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

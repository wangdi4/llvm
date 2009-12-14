/////////////////////////////////////////////////////////////////////////
// llvm_executable.cpp:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2009 Intel Corporation All Rights Reserved.
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
#include "llvm_kernel.h"
#include "llvm_binary.h"
#include "llvm_executable.h"
#include <assert.h>

using namespace Intel::OpenCL::DeviceBackend;

size_t GCD(size_t a, size_t b)
{
	while( 1 )
	{
		a = a % b;
		if( a == 0 )
			return b;
		b = b % a;
		if( b == 0 )
			return a;
	}
}

LLVMBinary::LLVMBinary(const LLVMKernel* pKernel, void* IN pArgsBuffer,
			   size_t IN ArgBuffSize, cl_uint IN WorkDimension, const size_t* IN pGlobalOffset,
			   const size_t* IN pGlobalWorkSize, const size_t* IN pLocalWorkSize) :
					m_pKernel(pKernel), m_pEntryPoint(pKernel->m_pFuncPtr),
					m_pArgsBuffer(pArgsBuffer),
					m_ArgBuffSize(ArgBuffSize)
{
	memset(&m_WorkInfo, 0, sizeof(sWorkInfo));
	m_WorkInfo.uiWorkDim = WorkDimension;
	if ( NULL != pGlobalOffset )
	{
		memcpy_s(m_WorkInfo.GlobalOffset, MAX_WORK_DIM*sizeof(size_t), pGlobalOffset, MAX_WORK_DIM*sizeof(size_t));
	}
	memcpy_s(m_WorkInfo.GlobalSize, MAX_WORK_DIM*sizeof(size_t), pGlobalWorkSize, MAX_WORK_DIM*sizeof(size_t));

	for(unsigned int i=1; i<MAX_WORK_DIM; ++i)
	{
		m_WorkInfo.LocalSize[i] = 1;
	}

	bool bLocalZeros = true;
	for(unsigned int i=0; i<m_WorkInfo.uiWorkDim; ++i)
	{
		bLocalZeros &= ( (m_WorkInfo.LocalSize[i]=pLocalWorkSize[i]) == 0);
	}
	if ( bLocalZeros )
	{
		// Try hint size, find GDC for each dimension
		if ( m_pKernel->m_pHintWGSize )
		{
			for(unsigned int i=1; i<m_WorkInfo.uiWorkDim; ++i)
			{
				m_WorkInfo.LocalSize[i] = GCD(pGlobalWorkSize[i], m_pKernel->m_pHintWGSize[i]);
			}
		}
		else
		{
			// On the last use optimal size
			// Fill first dimension with WG size
			m_WorkInfo.LocalSize[0] = GCD(pGlobalWorkSize[0], pKernel->GetKernelPackSize());
			for(unsigned int i=1; i<m_WorkInfo.uiWorkDim; ++i)
			{
				m_WorkInfo.LocalSize[i] = 1;
			}
		}
	}

	// Calculate number of work groups
	for(unsigned int i=0; i<m_WorkInfo.uiWorkDim; ++i)
	{
		m_WorkInfo.WGNumber[i] = m_WorkInfo.GlobalSize[i]/m_WorkInfo.LocalSize[i];
	}
}

LLVMBinary::~LLVMBinary()
{
	if ( NULL != m_pLocalParams )
	{
		delete m_pLocalParams;
	}
}

cl_uint LLVMBinary::Execute( void* IN pMemoryBuffers, 
				const size_t* IN pBufferCount, 
				const size_t* IN pGlobalId, 
				const size_t* IN pLocalId, 
				const size_t* IN pItemsToProcess ) const
{
	return CL_DEV_SUCCESS;
}

cl_uint LLVMBinary::GetMemoryBuffersDescriptions(size_t* IN pBuffersSizes, 
									 cl_exec_mem_type* IN pBuffersTypes, 
									 size_t* INOUT pBufferCount ) const
{
	assert(pBufferCount);
	if ( (NULL == pBuffersSizes) )
	{
		*pBufferCount = m_uiLocalCount+(m_pKernel->GetImplicitLocalMemoryBufferSize() != 0);
		return CL_DEV_SUCCESS;
	}
	assert(pBuffersSizes);
	for (unsigned int i=0;i<m_uiLocalCount;++i)
	{
		pBuffersSizes[i] = *((size_t*)(m_pLocalParams+m_pLocalBufferOffsets[i]));
	}
	return CL_DEV_SUCCESS;
}

cl_uint LLVMBinary::CreateExecutable(void* IN *pMemoryBuffers, 
									  size_t IN pBufferCount, ICLDevBackendExecutable* OUT *pContext)
{
	// Calculate amount of actual WorkItems in context
	unsigned int uiWIcount = 1;
	for (unsigned int i=0; i<m_WorkInfo.uiWorkDim; ++i)
	{
		uiWIcount *= m_WorkInfo.LocalSize[i];
	}

	assert(pContext);
	LLVMExecutable*	pLLVMExecutable = NULL;
	if (1 == uiWIcount)
	{
		pLLVMExecutable = new LLVMExecSingleWI(this);
	} else if ( m_pKernel->m_bBarrier )
	{
		pLLVMExecutable = new LLVMExecMultipleWIWithBarrier(this);
	}
	else
	{
		pLLVMExecutable = new LLVMExecMultipleWINoBarrier(this);
		uiWIcount = 1;		// In this case we need single WI context
	}
	// Initial the context to be start of the stack frame
	if ( NULL == pLLVMExecutable )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	cl_uint res = pLLVMExecutable->Init(pMemoryBuffers, uiWIcount);
	*pContext = pLLVMExecutable;
	return res;
}

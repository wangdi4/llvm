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
#include "llvm_program.h"
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

LLVMBinary::LLVMBinary(const LLVMKernel* pKernel, cl_uint IN WorkDimension, const size_t* IN pGlobalOffset,
			   const size_t* IN pGlobalWorkSize, const size_t* IN pLocalWorkSize) :
					m_pKernel(pKernel), m_pEntryPoint(pKernel->m_pFuncPtr),
					m_stFormalParamSize(0), m_stStackSize(0),m_stKernelParamSize(0),
					m_uiLocalCount(0), m_pLocalParams(NULL), m_pVectEntryPoint(0)
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

	// Calculate number of work groups and WG size
	m_uiWGSize = 1;
	for(unsigned int i=0; i<m_WorkInfo.uiWorkDim; ++i)
	{
		m_WorkInfo.WGNumber[i] = m_WorkInfo.GlobalSize[i]/m_WorkInfo.LocalSize[i];
		m_uiWGSize *= m_WorkInfo.LocalSize[i];
	}
}

LLVMBinary::~LLVMBinary()
{
}

cl_uint	LLVMBinary::Init(char* IN pArgsBuffer, size_t IN ArgBuffSize)
{
	m_pLocalParams = (char*)(((size_t)m_pLocalParamsBase+15) & ~0xF);	// Make aligned to 16 byte
#ifdef _DEBUG
	memset(m_pLocalParams, 0x88, CPU_MAX_PARAMETER_SIZE);
#endif

	size_t	stTotalLocalSize = 0;
	size_t	stLocalOffset = 0;
	size_t	stArgsOffset = 0;

	// Calculate actual local buffer size
	// Store in local buffer in reverse order
	for(unsigned int i=0; i<m_pKernel->m_uiArgCount; ++i)
	{
		// Argument is buffer object or local memory size
		if ( CL_KRNL_ARG_PTR_GLOBAL <= m_pKernel->m_pArguments[i].type )

		{
			*(void**)(m_pLocalParams+stLocalOffset) = *(void**)(pArgsBuffer+stArgsOffset);
			stLocalOffset += sizeof(void*);
			stArgsOffset += sizeof(void*);
		}
		else if (CL_KRNL_ARG_PTR_LOCAL == m_pKernel->m_pArguments[i].type)
		{
			// Retrieve sizes of explicit local objects
			size_t origSize = *(((size_t*)(pArgsBuffer+stArgsOffset)));
			size_t locSize = ADJUST_SIZE_TO_DCU_LINE(origSize); 
			stTotalLocalSize += locSize;
			m_pLocalBufferOffsets[m_uiLocalCount] = stLocalOffset;	// the offset is from the end
			++m_uiLocalCount;
			*(((size_t*)(m_pLocalParams+stLocalOffset))) = locSize;
			stLocalOffset += sizeof(void*);
			stArgsOffset += sizeof(void*);
		}
		else if (CL_KRNL_ARG_VECTOR == m_pKernel->m_pArguments[i].type)
		{
			unsigned int elemSize = m_pKernel->m_pArguments[i].size_in_bytes >> 16;
			unsigned int numElements = (m_pKernel->m_pArguments[i].size_in_bytes) & 0xFFFF;
			unsigned int uiSize = elemSize * numElements;
			if( (elemSize < CPU_MIN_ACTUAL_PARAM_SIZE) && (uiSize < CPU_MIN_VECTOR_SIZE))
			{
				// Copy argument values one by one
				for(unsigned int j=0; j<numElements; ++j)
				{
					*(CPU_MIN_ACTUAL_PARAM_PTR)(m_pLocalParams+stLocalOffset) = 
						(*(CPU_MIN_ACTUAL_PARAM_PTR)(pArgsBuffer+stArgsOffset)) & (~(-1 <<(elemSize*8)));
					stArgsOffset += elemSize;
					stLocalOffset += CPU_MIN_ACTUAL_PARAM_SIZE;
				}
			}
			else
			{
				// Check if offset should be aligned
				if ( uiSize >= 16 )
				{
					stLocalOffset = ((stLocalOffset + 16-1) & ~(16-1));
				}

				// Copy arguments in one cycle
				memcpy(m_pLocalParams+stLocalOffset, pArgsBuffer+stArgsOffset, uiSize);
				stArgsOffset += uiSize;
				stLocalOffset += uiSize;
			}
		}
		else
		{
			memcpy(m_pLocalParams+stLocalOffset, pArgsBuffer+stArgsOffset, m_pKernel->m_pArguments[i].size_in_bytes);
			stArgsOffset += m_pKernel->m_pArguments[i].size_in_bytes;
			stLocalOffset += max(m_pKernel->m_pArguments[i].size_in_bytes, CPU_MIN_ACTUAL_PARAM_SIZE);
		}
	}

	m_stFormalParamSize = stLocalOffset;

	m_stKernelParamSize =	stLocalOffset +
							4*sizeof(void*)+MAX_WORK_DIM*sizeof(size_t) +	// OCL specific argument (WG Info, Global ID, ect)
							sizeof(void*); // Pointer to IDevExecutable

	m_stStackSize = m_pKernel->GetPrivateMemorySize() +				// Kernel stack area
						sizeof(void*) +								// Return address
						m_stKernelParamSize;						// Kernel call stack size
														

	return CL_DEV_SUCCESS;
}


cl_uint LLVMBinary::Execute( void* IN pMemoryBuffers, 
				const size_t* IN pBufferCount, 
				const size_t* IN pGlobalId, 
				const size_t* IN pLocalId, 
				const size_t* IN pItemsToProcess ) const
{
	return CL_DEV_ERROR_FAIL;
}

cl_uint LLVMBinary::GetMemoryBuffersDescriptions(size_t* IN pBufferSizes, 
									 cl_exec_mem_type* IN pBuffersTypes, 
									 size_t* INOUT pBufferCount ) const
{
	assert(pBufferCount);
	if ( (NULL == pBufferSizes) )
	{
		size_t buffCount = m_uiLocalCount;
		buffCount += (m_pKernel->GetImplicitLocalMemoryBufferSize() != 0);	// Implicit local buffers
		++buffCount;														// Stack size
		*pBufferCount = buffCount;
		return CL_DEV_SUCCESS;
	}
	assert(pBufferSizes);

	// Fill sizes of explicit local buffers
	unsigned int i;
	for (i=0;i<m_uiLocalCount;++i)
	{
		pBufferSizes[i] = *((size_t*)(m_pLocalParams+m_pLocalBufferOffsets[i]));
	}
	// Fill size of the implicit local buffer
	if ( m_pKernel->GetImplicitLocalMemoryBufferSize() != 0 )
	{
		pBufferSizes[i] = m_pKernel->GetImplicitLocalMemoryBufferSize();
		++i;
	}

	// Fill size of private area (stack)
	pBufferSizes[i] = m_stStackSize*m_uiWGSize +
						15;							// Request additional space for further aligment

	if ( NULL != pBuffersTypes )
	{
		pBuffersTypes[i] = CL_EXEC_PRIVATE_MEMORY_TYPE;
		while( i > 0 )
		{
			--i;
			pBuffersTypes[i] = CL_EXEC_LOCAL_MEMORY_TYPE;
		}
	}

	return CL_DEV_SUCCESS;
}

cl_uint LLVMBinary::CreateExecutable(void* IN *pMemoryBuffers, 
									  size_t IN stBufferCount, ICLDevBackendExecutable* OUT *pExec)
{
	unsigned int uiWGSizeLocal = m_uiWGSize;
	assert(pExec);

	if(m_bVectorized)
	{
		//get the vectorized kernel pointer
		const LLVMKernel *pVectKernel;
		if(CL_DEV_SUCCESS == m_pKernel->m_pProgram->GetVectorizedKernel(m_szVectorizedName, (const ICLDevBackendKernel **)&pVectKernel))
		{
			m_pVectEntryPoint = pVectKernel->m_pFuncPtr;
		}
		else
		{
			m_bVectorized = false;
			// we're not supposed to be here...
			assert(0);
		}
	}

	if(m_bVectorized && (m_WorkInfo.LocalSize[0] % m_uiVectorWidth))
	{
		// Disable vectorization for workgroup sizes that are not
		// a multiple of the vector width (Guy)
		m_bVectorized = false;
	}

	LLVMExecutable*	pLLVMExecutable = NULL;
	if (1 == uiWGSizeLocal)
	{
		pLLVMExecutable = new LLVMExecSingleWI(this);
	} else if ( m_pKernel->m_bBarrier )
	{
		pLLVMExecutable = new LLVMExecMultipleWIWithBarrier(this);
	}
	else if(m_bVectorized)
	{
		pLLVMExecutable = new LLVMExecVectorizedNoBarrier(this);
	}
	else
	{
		pLLVMExecutable = new LLVMExecMultipleWINoBarrier(this);
		uiWGSizeLocal = 1;		// In this case we need single WI context
	}
	// Initial the context to be start of the stack frame
	if ( NULL == pLLVMExecutable )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	// The last buffer is the stack region
	cl_uint res = pLLVMExecutable->Init(pMemoryBuffers, pMemoryBuffers[stBufferCount-1], uiWGSizeLocal);
	*pExec = pLLVMExecutable;
	return res;
}


bool LLVMBinary::isVectorized()
{
	return m_bVectorized;
}

unsigned int LLVMBinary::getVectorWidth()
{
	return m_uiVectorWidth;
}

void LLVMBinary::setVectorizerProperties(bool isVectorized, const char *vectorizedName, unsigned int vectorWidth)
{
	m_bVectorized      = isVectorized;
	m_szVectorizedName = vectorizedName;
	m_uiVectorWidth    = vectorWidth;
}

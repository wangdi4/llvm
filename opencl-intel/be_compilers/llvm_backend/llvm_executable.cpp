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
// or disclosed in any way without Intel�s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel�s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cl_device_api.h"
#include "llvm_kernel.h"
#include "llvm_binary.h"
#include "llvm_executable.h"

#include <assert.h>
#include <tmmintrin.h>

using namespace Intel::OpenCL::DeviceBackend;

#define DEBUG_CHECK_MASK_SIZE	16

LLVMExecutable::LLVMExecutable(const LLVMBinary* pBin):
	m_pBinary(pBin), m_pContext(NULL)
{
	m_pBaseGlobalId = (size_t*)_aligned_malloc(4*sizeof(size_t), 16);
	if ( NULL != m_pBaseGlobalId )
	{
		memset(m_pBaseGlobalId, 0, 4*sizeof(size_t));
	}
}

LLVMExecutable::~LLVMExecutable()
{
	if ( NULL != m_pBaseGlobalId)
	{
		_aligned_free(m_pBaseGlobalId);
	}
}
void LLVMExecutable::Release()
{
	delete this;
}

// Initialize context to with specific number of WorkItems 
cl_uint	LLVMExecutable::Init(void* *pLocalMemoryBuffers, void* pWGStackFrame, unsigned int uiWICount)
{
	if ( NULL == m_pBaseGlobalId)
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	m_uiWICount = uiWICount;

	size_t stParamSize = m_pBinary->GetKernelParametersSize();

	m_pContext = (char*)(((size_t)((char*)pWGStackFrame + m_pBinary->GetStackSize() - stParamSize + 15)) & ~0xF);

	// Copy parameters to context
	memcpy(m_pContext, m_pBinary->GetFormalParameters(), m_pBinary->GetFormalParametersSize());
	// Update pointers of the local buffers
	for (unsigned int i=0; i<m_pBinary->m_uiLocalCount; ++i)
	{
		*((void**)(m_pContext+m_pBinary->m_pLocalBufferOffsets[i])) = pLocalMemoryBuffers[i];
	}

	char* pWIParams = m_pContext + m_pBinary->GetFormalParametersSize();
	// Set implicit local buffer pointer
	if ( m_pBinary->m_pKernel->GetImplicitLocalMemoryBufferSize() )
	{
		// Is the next buffer after explicit locals
		*((void**)pWIParams) = pLocalMemoryBuffers[m_pBinary->m_uiLocalCount];
	}
	pWIParams += sizeof(void*);

	// Set Work Dimension Info pointer
	*((sWorkInfo**)pWIParams) = (sWorkInfo *)(&m_pBinary->m_WorkInfo);
	pWIParams += sizeof(sWorkInfo *);

	// Leave space for WorkGroup id
	pWIParams += sizeof(size_t*);

	// Set Base Global Id ptr
	*((size_t**)pWIParams) = m_pBaseGlobalId;
	pWIParams += sizeof(size_t**);

	// Set Local id to (0,0,0)
	memset(pWIParams, 0, 3*sizeof(size_t) );

	// Setup Context pointer
	pWIParams += 3*sizeof(size_t);
	*((void**)pWIParams) = this;

	return CL_DEV_SUCCESS;
}

cl_uint LLVMExecSingleWI::Execute(const size_t* IN pGroupId,
								  const size_t* IN pLocalOffset, 
								  const size_t* IN pItemsToProcess)
{
	// Set Work Group index pointer
	const size_t*	*pWGid = (const size_t* *)(m_pContext+m_pBinary->GetFormalParametersSize()+2*sizeof(void*));
	*pWGid = pGroupId;

	const void*			pEntryPoint = m_pBinary->m_pEntryPoint;

#ifdef __SSE4_1__
	__m128i xmmGroupId = _mm_lddqu_si128((__m128i*)pGroupId);
	__m128i xmmGroupSize = _mm_lddqu_si128((__m128i*)m_pBinary->m_WorkInfo.LocalSize);
	xmmGroupId = _mm_mullo_epi32(xmmGroupId, xmmGroupSize);
	_mm_store_si128((__m128i*)m_pBaseGlobalId, xmmGroupId);
#else
	m_pBaseGlobalId[0] = pGroupId[0]*m_pBinary->m_WorkInfo.LocalSize[0];
	if ( m_pBinary->m_WorkInfo.uiWorkDim > 1 )
		m_pBaseGlobalId[1] = pGroupId[1]*m_pBinary->m_WorkInfo.LocalSize[1];
	if ( m_pBinary->m_WorkInfo.uiWorkDim > 2 )
		m_pBaseGlobalId[2] = pGroupId[2]*m_pBinary->m_WorkInfo.LocalSize[2];
#endif

	void* pContext = m_pContext;
	__asm
	{
		push	edi
		mov		edi,	esp
		mov		esp,	pContext
		call	pEntryPoint
		mov		esp,	edi
		pop		edi
	}
	return CL_DEV_SUCCESS;
}

cl_uint LLVMExecMultipleWINoBarrier::Execute(const size_t* IN pGroupId,
											 const size_t* IN pLocalOffset, 
											 const size_t* IN pItemsToProcess)
{
	const size_t*	*pWGid = (const size_t* *)(m_pContext+m_pBinary->GetFormalParametersSize()+2*sizeof(void*));
	*pWGid = pGroupId;

	// Retrieve local id area
	size_t*	pLocalId = (size_t*)((const char*)pWGid+2*sizeof(void*));

	const void*		pEntryPoint = m_pBinary->m_pEntryPoint;
	void*			pContext = m_pContext;

#ifdef __SSE4_1__
	__m128i xmmGroupId;
	__m128i xmmGroupSize;
#endif

	m_bIsFirst = true;

	switch (m_pBinary->m_WorkInfo.uiWorkDim)
	{
	case 1:
		// Update base global id
		m_pBaseGlobalId[0] = pGroupId[0]*m_pBinary->m_WorkInfo.LocalSize[0];
		for (pLocalId[0]=0;pLocalId[0]<m_pBinary->m_WorkInfo.LocalSize[0];++pLocalId[0])
		{
			__asm
			{
				push	edi
				mov		edi,	esp
				mov		esp,	pContext
				call	pEntryPoint
				mov		esp,	edi
				pop		edi
			}
		}
		break;

	case 2:
#ifdef __SSE4_1__
		xmmGroupId = _mm_loadl_epi64((__m128i*)pGroupId);
		xmmGroupSize = _mm_loadl_epi64((__m128i*)m_pBinary->m_WorkInfo.LocalSize);
		xmmGroupId = _mm_mullo_epi32(xmmGroupId, xmmGroupSize);
		_mm_storel_epi64((__m128i*)m_pBaseGlobalId, xmmGroupId);
#else
		m_pBaseGlobalId[0] = pGroupId[0]*m_pBinary->m_WorkInfo.LocalSize[0];
		m_pBaseGlobalId[1] = pGroupId[1]*m_pBinary->m_WorkInfo.LocalSize[1];
#endif
		for (pLocalId[1]=0;pLocalId[1]<m_pBinary->m_WorkInfo.LocalSize[1];++pLocalId[1])
			for (pLocalId[0]=0;pLocalId[0]<m_pBinary->m_WorkInfo.LocalSize[0];++pLocalId[0])
			{
				__asm
				{
					push	edi
					mov		edi,	esp
					mov		esp,	pContext
					call	pEntryPoint
					mov		esp,	edi
					pop		edi
				}
			}
		break;

	case 3:
#ifdef __SSE4_1__
		xmmGroupId = _mm_lddqu_si128((__m128i*)pGroupId);
		xmmGroupSize = _mm_lddqu_si128((__m128i*)m_pBinary->m_WorkInfo.LocalSize);
		xmmGroupId = _mm_mullo_epi32(xmmGroupId, xmmGroupSize);
		_mm_store_si128((__m128i*)m_pBaseGlobalId, xmmGroupId);
#else
		m_pBaseGlobalId[0] = pGroupId[0]*m_pBinary->m_WorkInfo.LocalSize[0];
		m_pBaseGlobalId[1] = pGroupId[1]*m_pBinary->m_WorkInfo.LocalSize[1];
		m_pBaseGlobalId[2] = pGroupId[2]*m_pBinary->m_WorkInfo.LocalSize[2];
#endif
		for (pLocalId[2]=0;pLocalId[2]<m_pBinary->m_WorkInfo.LocalSize[2];++pLocalId[2])
			for (pLocalId[1]=0;pLocalId[1]<m_pBinary->m_WorkInfo.LocalSize[1];++pLocalId[1])
				for (pLocalId[0]=0;pLocalId[0]<m_pBinary->m_WorkInfo.LocalSize[0];++pLocalId[0])
				{
					__asm
					{
						push	edi
						mov		edi,	esp
						mov		esp,	pContext
						call	pEntryPoint
						mov		esp,	edi
						pop		edi
					}
				}
		break;

	default:
		return CL_DEV_ERROR_FAIL;
	}

	return CL_DEV_SUCCESS;
}

cl_uint LLVMExecVectorizedNoBarrier::Execute(const size_t* IN pGroupId,
											 const size_t* IN pLocalOffset, 
											 const size_t* IN pItemsToProcess)
{
	const size_t*	*pWGid = (const size_t* *)(m_pContext+m_pBinary->GetFormalParametersSize()+2*sizeof(void*));
	*pWGid = pGroupId;

	// Retrieve local id area
	size_t*	pLocalId = (size_t*)((const char*)pWGid+2*sizeof(void*));

	const void*		pStdEntryPoint = m_pBinary->m_pEntryPoint;
	const void*		pVecEntryPoint = m_pBinary->m_pVectEntryPoint;
	unsigned int    uiVectWidth    = m_pBinary->m_uiVectorWidth;
	void*			pContext = m_pContext;

#ifdef __SSE4_1__
	__m128i xmmGroupId;
	__m128i xmmGroupSize;
#endif

	m_bIsFirst = true;

	switch (m_pBinary->m_WorkInfo.uiWorkDim)
	{
	case 1:
		// Update base global id
		m_pBaseGlobalId[0] = pGroupId[0]*m_pBinary->m_WorkInfo.LocalSize[0];
		for (pLocalId[0]=0;(pLocalId[0] + uiVectWidth - 1)<m_pBinary->m_WorkInfo.LocalSize[0];pLocalId[0]+=uiVectWidth)
		{
			__asm
			{
				push	edi
				mov		edi,	esp
				mov		esp,	pContext
				call	pVecEntryPoint
				mov		esp,	edi
				pop		edi
			}
		}
		//
		// Comment out the treatment of the reminders - currently we disable
		// the vectorization of workgroups whit sizes that are not a multiple
		// of the vector width (Guy)
		//
		//for (;pLocalId[0]<m_pBinary->m_WorkInfo.LocalSize[0];++pLocalId[0])
		//{
		//	__asm
		//	{
		//		push	edi
		//		mov		edi,	esp
		//		mov		esp,	pContext
		//		call	pStdEntryPoint
		//		mov		esp,	edi
		//		pop		edi
		//	}
		//}
		break;
	case 2:
#ifdef __SSE4_1__
		xmmGroupId = _mm_loadl_epi64((__m128i*)pGroupId);
		xmmGroupSize = _mm_loadl_epi64((__m128i*)m_pBinary->m_WorkInfo.LocalSize);
		xmmGroupId = _mm_mullo_epi32(xmmGroupId, xmmGroupSize);
		_mm_storel_epi64((__m128i*)m_pBaseGlobalId, xmmGroupId);
#else
		m_pBaseGlobalId[0] = pGroupId[0]*m_pBinary->m_WorkInfo.LocalSize[0];
		m_pBaseGlobalId[1] = pGroupId[1]*m_pBinary->m_WorkInfo.LocalSize[1];
#endif
		for (pLocalId[1]=0;pLocalId[1]<m_pBinary->m_WorkInfo.LocalSize[1];++pLocalId[1])
		{
			for (pLocalId[0]=0;(pLocalId[0] + uiVectWidth - 1)<m_pBinary->m_WorkInfo.LocalSize[0];pLocalId[0]+=uiVectWidth)
			{
				__asm
				{
					push	edi
					mov		edi,	esp
					mov		esp,	pContext
					call	pVecEntryPoint
					mov		esp,	edi
					pop		edi
				}
			}
			//
			// Comment out the treatment of the reminders - currently we disable
			// the vectorization of workgroups whit sizes that are not a multiple
			// of the vector width (Guy)
			//
			//for (;pLocalId[0]<m_pBinary->m_WorkInfo.LocalSize[0];++pLocalId[0])
			//{
			//	__asm
			//	{
			//		push	edi
			//		mov		edi,	esp
			//		mov		esp,	pContext
			//		call	pStdEntryPoint
			//		mov		esp,	edi
			//		pop		edi
			//	}
			//}
		}
		break;
	case 3:
#ifdef __SSE4_1__
		xmmGroupId = _mm_lddqu_si128((__m128i*)pGroupId);
		xmmGroupSize = _mm_lddqu_si128((__m128i*)m_pBinary->m_WorkInfo.LocalSize);
		xmmGroupId = _mm_mullo_epi32(xmmGroupId, xmmGroupSize);
		_mm_store_si128((__m128i*)m_pBaseGlobalId, xmmGroupId);
#else
		m_pBaseGlobalId[0] = pGroupId[0]*m_pBinary->m_WorkInfo.LocalSize[0];
		m_pBaseGlobalId[1] = pGroupId[1]*m_pBinary->m_WorkInfo.LocalSize[1];
		m_pBaseGlobalId[2] = pGroupId[2]*m_pBinary->m_WorkInfo.LocalSize[2];
#endif
		for (pLocalId[2]=0;pLocalId[2]<m_pBinary->m_WorkInfo.LocalSize[2];++pLocalId[2])
			for (pLocalId[1]=0;pLocalId[1]<m_pBinary->m_WorkInfo.LocalSize[1];++pLocalId[1])
			{
				for (pLocalId[0]=0;(pLocalId[0] + uiVectWidth - 1)<m_pBinary->m_WorkInfo.LocalSize[0];pLocalId[0]+=uiVectWidth)
				{
					__asm
					{
						push	edi
						mov		edi,	esp
						mov		esp,	pContext
						call	pVecEntryPoint
						mov		esp,	edi
						pop		edi
					}
				}
				//
				// Comment out the treatment of the reminders - currently we disable
				// the vectorization of workgroups whit sizes that are not a multiple
				// of the vector width (Guy)
				//
				//for (;pLocalId[0]<m_pBinary->m_WorkInfo.LocalSize[0];++pLocalId[0])
				//{
				//	__asm
				//	{
				//		push	edi
				//		mov		edi,	esp
				//		mov		esp,	pContext
				//		call	pStdEntryPoint
				//		mov		esp,	edi
				//		pop		edi
				//	}
				//}
			}
		break;
	default:
		return CL_DEV_ERROR_FAIL;
	}

	return CL_DEV_SUCCESS;
}

//---------------------------------------------------------------------------------------
LLVMExecMultipleWIWithBarrier::~LLVMExecMultipleWIWithBarrier()
{
}

cl_uint	LLVMExecMultipleWIWithBarrier::Init(void* *pMemoryBuffers, void* pWGStackFrame, unsigned int uiWICount)
{
	cl_uint ret = LLVMExecutable::Init(pMemoryBuffers, pWGStackFrame, uiWICount);
	if ( CL_DEV_FAILED(ret))
	{
		return ret;
	}

	m_uiWIStackSize = m_pBinary->GetStackSize();

	// Setup local WG index pointer
	const size_t*	*pWGid = (const size_t* *)(m_pContext+m_pBinary->GetFormalParametersSize()+2*sizeof(void*));
	*pWGid = m_pLclGroupId;
	
	// Now setup rest of the WorkItems
	unsigned int i=0;
	unsigned int uiParamCopySize = m_pBinary->GetKernelParametersSize() -
										MAX_WORK_DIM*sizeof(size_t) - sizeof(void*);

	char* pCurrContext = m_pContext + m_uiWIStackSize;
	char* pLocald = pCurrContext + uiParamCopySize;

	for (unsigned int z=0; z<m_pBinary->m_WorkInfo.LocalSize[2]; ++z)
	{
		for (unsigned int y=0; y<m_pBinary->m_WorkInfo.LocalSize[1]; ++y)
		{
			for (unsigned int x=0; x<m_pBinary->m_WorkInfo.LocalSize[0]; ++x)
			{
				if ( i != 0 )
				{
					memcpy(pCurrContext, m_pContext, uiParamCopySize);
					// Update Local id
					((size_t*)pLocald)[0] = x;
					((size_t*)pLocald)[1] = y;
					((size_t*)pLocald)[2] = z;
					*((void**)(pLocald+MAX_WORK_DIM*sizeof(size_t))) = this;

					pCurrContext += m_uiWIStackSize;
					pLocald += m_uiWIStackSize;
				}
				++i;
			}
		}
	}
	return CL_DEV_SUCCESS;
}

cl_uint LLVMExecMultipleWIWithBarrier::Execute(const size_t* IN pGroupId,
											   const size_t* IN pLocalOffset, 
											   const size_t* IN pItemsToProcess)
{
	// Initialize internal states
	memset(m_pJmpBuf, 0, sizeof(_JUMP_BUFFER)*m_uiWICount);

	// Copy WG index
	for (unsigned int i=0;i<m_pBinary->m_WorkInfo.uiWorkDim;++i)
	{
		m_pLclGroupId[i] = pGroupId[i];
	}

#if 0
	m_setAsyncCopyCmds.clear();
#endif

	const void*		pEntryPoint = m_pBinary->m_pEntryPoint;
	char*			pContext = m_pContext;

#ifdef __SSE4_1__
	__m128i xmmGroupId = _mm_lddqu_si128((__m128i*)pGroupId);
	__m128i xmmGroupSize = _mm_lddqu_si128((__m128i*)m_pBinary->m_WorkInfo.LocalSize);
	xmmGroupId = _mm_mullo_epi32(xmmGroupId, xmmGroupSize);
	_mm_store_si128((__m128i*)m_pBaseGlobalId, xmmGroupId);
#else
	m_pBaseGlobalId[0] = pGroupId[0]*m_pBinary->m_WorkInfo.LocalSize[0];
	if ( m_pBinary->m_WorkInfo.uiWorkDim > 1 )
		m_pBaseGlobalId[1] = pGroupId[1]*m_pBinary->m_WorkInfo.LocalSize[1];
	if ( m_pBinary->m_WorkInfo.uiWorkDim > 2 )
		m_pBaseGlobalId[2] = pGroupId[2]*m_pBinary->m_WorkInfo.LocalSize[2];
#endif

	unsigned int iWICount = m_uiWICount;
	m_iCurrWI=0;

	// We have WI to execute
	while (iWICount > 0)
	{
		// Store main context
		// TODO: consider to use private Cntx switch
		int ret = setjmp((_JBTYPE*)&m_mainJmpBuf);
		if ( 0 == ret )
		{
			// We just stored main task call to WI
			_JUMP_BUFFER* pCurrJB = &m_pJmpBuf[m_iCurrWI];
			if ( 0 != pCurrJB->Esp )
			{
				// Execution of previously run WI
				longjmp((_JBTYPE*)pCurrJB, 0);
				assert(0);	// Never got here
			} else if ( -1 != pCurrJB->Esp)
			{
				pContext = m_pContext + m_iCurrWI*m_uiWIStackSize;
				__asm
				{
					push	edi
					mov		edi,		esp
					mov		esp,		pContext
					call	pEntryPoint
					mov		esp,		edi
					pop		edi
				}
				// When got here a WI was completely executed
				pCurrJB->Esp = -1;	// Mark as done
				--iWICount;
			}

			// Switch to next WI
			++m_iCurrWI;
			if ( m_iCurrWI >= m_uiWICount)
			{
				m_iCurrWI = 0;
			}
		}
	}

	return CL_DEV_SUCCESS;
}

void LLVMExecMultipleWIWithBarrier::SwitchToMain()
{
	// Store current state
	_JUMP_BUFFER* pCurrJB = &m_pJmpBuf[m_iCurrWI];
	int ret = setjmp((_JBTYPE*)pCurrJB);
	if ( 0 == ret )
	{
		// Here we should return to main routine
		// Calculate next WI to execute
		++m_iCurrWI;
		if ( m_iCurrWI >= m_uiWICount)
		{
			m_iCurrWI = 0;
		}
		longjmp((_JBTYPE*)&m_mainJmpBuf, 0);
		assert(0); // Never got here
	}
	// Here we are back from main routine
	// Just return to execution of WI
}

bool LLVMExecMultipleWIWithBarrier::SetAndCheckAsyncCopy(unsigned int uiKey)
{
#if 0
	AsyncCopyMap::iterator it = m_setAsyncCopyCmds.find(uiKey);

	if ( it != m_setAsyncCopyCmds.end())
	{
		return false;
	}

	m_setAsyncCopyCmds[uiKey] = m_uiWICount;

	return true;
#endif
	return (m_iCurrWI == 0);
}

bool LLVMExecMultipleWIWithBarrier::ResetAsyncCopy(unsigned int uiKey)
{
	return false;
#if 0
	AsyncCopyMap::iterator it = m_setAsyncCopyCmds.find(uiKey);

	if ( it == m_setAsyncCopyCmds.end())
	{
		return false;
	}

	unsigned int val = --(m_setAsyncCopyCmds[uiKey]);
	if ( 0 == val )
	{
		m_setAsyncCopyCmds.erase(it);
		return false;
	}

	return true;
#endif
}

//---------------------------------------------------------------------------------------
LLVMExecVectorizedWithBarrier::~LLVMExecVectorizedWithBarrier()
{
}

cl_uint	LLVMExecVectorizedWithBarrier::Init(void* *pMemoryBuffers, void* pWGStackFrame, unsigned int uiWICount)
{
	cl_uint ret = LLVMExecutable::Init(pMemoryBuffers, pWGStackFrame, uiWICount);
	if ( CL_DEV_FAILED(ret))
	{
		return ret;
	}

	m_uiWIStackSize = m_pBinary->GetStackSize();

	// Setup local WG index pointer
	const size_t*	*pWGid = (const size_t* *)(m_pContext+m_pBinary->GetFormalParametersSize()+2*sizeof(void*));
	*pWGid = m_pLclGroupId;
	
	// Now setup rest of the WorkItems
	unsigned int i=0;
	unsigned int uiParamCopySize = m_pBinary->GetKernelParametersSize() -
										MAX_WORK_DIM*sizeof(size_t) - sizeof(void*);

	char* pCurrContext = m_pContext + m_uiWIStackSize;
	char* pLocald = pCurrContext + uiParamCopySize;

	for (unsigned int z=0; z<m_pBinary->m_WorkInfo.LocalSize[2]; ++z)
	{
		for (unsigned int y=0; y<m_pBinary->m_WorkInfo.LocalSize[1]; ++y)
		{
			for (unsigned int x=0; x<m_pBinary->m_WorkInfo.LocalSize[0]; ++x)
			{
				if ( i != 0 )
				{
					memcpy(pCurrContext, m_pContext, uiParamCopySize);
					// Update Local id
					((size_t*)pLocald)[0] = x;
					((size_t*)pLocald)[1] = y;
					((size_t*)pLocald)[2] = z;
					*((void**)(pLocald+MAX_WORK_DIM*sizeof(size_t))) = this;

					pCurrContext += m_uiWIStackSize;
					pLocald += m_uiWIStackSize;
				}
				++i;
			}
		}
	}
	return CL_DEV_SUCCESS;
}

cl_uint LLVMExecVectorizedWithBarrier::Execute(const size_t* IN pGroupId,
											   const size_t* IN pLocalOffset, 
											   const size_t* IN pItemsToProcess)
{

	// Initialize internal states
	memset(m_pJmpBuf, 0, sizeof(_JUMP_BUFFER)*m_uiWICount);

	// Copy WG index
	for (unsigned int i=0;i<m_pBinary->m_WorkInfo.uiWorkDim;++i)
	{
		m_pLclGroupId[i] = pGroupId[i];
	}

#if 0
	m_setAsyncCopyCmds.clear();
#endif

	const void*		pEntryPoint = m_pBinary->m_pEntryPoint;
	const void*		pVecEntryPoint = m_pBinary->m_pVectEntryPoint;
	unsigned int    uiVectWidth    = m_pBinary->m_uiVectorWidth;
	char*			pContext = m_pContext;

#ifdef __SSE4_1__
	__m128i xmmGroupId = _mm_lddqu_si128((__m128i*)pGroupId);
	__m128i xmmGroupSize = _mm_lddqu_si128((__m128i*)m_pBinary->m_WorkInfo.LocalSize);
	xmmGroupId = _mm_mullo_epi32(xmmGroupId, xmmGroupSize);
	_mm_store_si128((__m128i*)m_pBaseGlobalId, xmmGroupId);
#else
	m_pBaseGlobalId[0] = pGroupId[0]*m_pBinary->m_WorkInfo.LocalSize[0];
	if ( m_pBinary->m_WorkInfo.uiWorkDim > 1 )
		m_pBaseGlobalId[1] = pGroupId[1]*m_pBinary->m_WorkInfo.LocalSize[1];
	if ( m_pBinary->m_WorkInfo.uiWorkDim > 2 )
		m_pBaseGlobalId[2] = pGroupId[2]*m_pBinary->m_WorkInfo.LocalSize[2];
#endif

	unsigned int iWICount = m_uiWICount;
	m_iCurrWI=0;

	// We have WI to execute
	while (iWICount > 0)
	{
		// Store main context
		// TODO: consider to use private Cntx switch
		int ret = setjmp((_JBTYPE*)&m_mainJmpBuf);
		if ( 0 == ret )
		{
			// We just stored main task call to WI
			_JUMP_BUFFER* pCurrJB = &m_pJmpBuf[m_iCurrWI];
			if ( 0 != pCurrJB->Esp )
			{
				// Execution of previously run WI
				longjmp((_JBTYPE*)pCurrJB, 0);
				assert(0);	// Never got here
			} else if ( -1 != pCurrJB->Esp)
			{
				pContext = m_pContext + m_iCurrWI*m_uiWIStackSize;
				__asm
				{
					push	edi
					mov		edi,		esp
					mov		esp,		pContext
					call	pVecEntryPoint
					mov		esp,		edi
					pop		edi
				}
				// When got here a WI was completely executed
				pCurrJB->Esp = -1;	// Mark as done
				iWICount -= uiVectWidth;
			}

			// Switch to next WI
			m_iCurrWI += uiVectWidth;
			if ( m_iCurrWI >= m_uiWICount)
			{
				m_iCurrWI = 0;
			}
		}
	}

	return CL_DEV_SUCCESS;
}

void LLVMExecVectorizedWithBarrier::SwitchToMain()
{
	unsigned int    uiVectWidth    = m_pBinary->m_uiVectorWidth;

	// Store current state
	_JUMP_BUFFER* pCurrJB = &m_pJmpBuf[m_iCurrWI];
	int ret = setjmp((_JBTYPE*)pCurrJB);
	if ( 0 == ret )
	{
		// Here we should return to main routine
		// Calculate next WI to execute
		m_iCurrWI+=uiVectWidth;
		if ( m_iCurrWI >= m_uiWICount)
		{
			m_iCurrWI = 0;
		}
		longjmp((_JBTYPE*)&m_mainJmpBuf, 0);
		assert(0); // Never got here
	}
	// Here we are back from main routine
	// Just return to execution of WI
}


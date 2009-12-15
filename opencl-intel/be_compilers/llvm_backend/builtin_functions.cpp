/////////////////////////////////////////////////////////////////////////
// wi_info_functions.cpp:
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

#include "llvm_binary.h"
#include "llvm_executable.h"
#include "cl_types.h"
#include "cpu_dev_limits.h"

#include <stdio.h>
#include <setjmp.h>
#include <smmintrin.h>

using namespace Intel::OpenCL::DeviceBackend;

/*
/*****************************************************************************************************************************
*		Synchronization functions (Section 6.11.9)
*****************************************************************************************************************************/
extern "C" __declspec(dllexport) void lbarrier(unsigned flags, LLVMExecutable* *ppExec)
{
	// Memory fence operation
	_mm_mfence();

	LLVMExecutable* pExec = *ppExec;

	assert(pExec && "Invalid context pointer");

	if ( !pExec->IsMultipleWIs() )
	{
		// We don't have active context, empty barrier
		return;
	}
	__asm{
		push		edi
		mov			edi, esp
		and			esp, 0xFFFFFFF0
		sub	esp,	512		;	// Leave space for XMM registers
		fxsave		[esp]	;	// Save XMM state
		pusha 				;	// Save general purpose registers
	}

	((LLVMExecMultipleWIWithBarrier*)pExec)->SwitchToMain();	// Returns control to the main execution routine

	// Here we are back, so we can continue
	__asm{
		popa
		fxrstor [esp]
		mov		esp, edi
		pop		edi
	}
}

extern "C" __declspec(dllexport) void dbg_print(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);

	vprintf(fmt, va);
	va_end( va );
}

typedef size_t event_t;
extern "C" __declspec(dllexport) void lwait_group_events(int num_events, event_t *event_list, LLVMExecutable* *ppExec)
{
	LLVMExecutable* pExec = *ppExec;
	assert(pExec && "Invalid context pointer");

	bool bBarrier = false;
	for (int i=0; i<num_events; ++i)
	{
		bBarrier |= pExec->ResetAsyncCopy(event_list[i]);
	}

	if ( bBarrier )
	{
		lbarrier(0, ppExec);
	}
}

extern "C" __declspec(dllexport) __declspec(noinline) event_t lasync_wg_copy_l2g(char* pDst, char* pSrc, size_t numElem, event_t event,
																size_t elemSize, LLVMExecutable* *ppExec)
{
	LLVMExecutable* pExec = *ppExec;
	assert(pExec && "Invalid context pointer");

	if ( 0 == event )
	{
		event = (size_t)_ReturnAddress();
	}
#if 0
	// Compound the key
	unsigned int uiKey = (((unsigned int)_ReturnAddress() & 0xFFFF) << 16) |
		((unsigned int)pDst & 0xFFFF);
#endif
	// Check if copy is required
	if ( !pExec->SetAndCheckAsyncCopy(event) )
		return event;

	size_t	uiBytesToCopy = numElem*elemSize;
	bool bUseSSE = (!(((size_t)pDst) & 0xF)) && (!((uiBytesToCopy) & 0xF));
	if ( bUseSSE )
	{
		for (unsigned int i=0; i<uiBytesToCopy; i+=16)
		{
			__m128i	xmmTmp = _mm_lddqu_si128((__m128i*)(pSrc+i));
			_mm_stream_si128((__m128i*)(pDst+i), xmmTmp);			// TODO: check performance implication of streaming instruction
		}
		return event;
	}

	// else use memcpy
	memcpy(pDst, pSrc, uiBytesToCopy);

	return event;
}

extern "C" __declspec(dllexport) __declspec(noinline) event_t lasync_wg_copy_g2l(char* pDst, char* pSrc, size_t numElem, event_t event,
															size_t elemSize, LLVMExecutable* *ppExec)
{
	LLVMExecutable* pExec = *ppExec;
	assert(pExec && "Invalid context pointer");

	if ( 0 == event )
	{
		event = (size_t)_ReturnAddress();
	}
#if 0
	// Compound the key
	unsigned int uiKey = (((unsigned int)_ReturnAddress() & 0xFFFF) << 16) |
		((unsigned int)pSrc & 0xFFFF);
#endif
	// Check if copy is required
	if ( !pExec->SetAndCheckAsyncCopy(event) )
		return event;

	size_t	uiBytesToCopy = numElem*elemSize;
	bool bUseSSE = (!(((size_t)pDst) & 0xF)) && (!(((size_t)pSrc) & 0xF)) && (!((uiBytesToCopy) & 0xF));
	if ( bUseSSE )
	{
		for (unsigned int i=0; i<uiBytesToCopy; i+=16)
		{
#ifdef __SSE4_1__
			__m128i	xmmTmp = _mm_stream_load_si128((__m128i*)(pSrc+i)); // TODO: check performance implication of streaming instruction
#else
			__m128i	xmmTmp = _mm_load_si128((__m128i*)(pSrc+i));
#endif
			_mm_store_si128((__m128i*)(pDst+i), xmmTmp);
		}
		return event;
	}

	// else use memcpy
	memcpy(pDst, pSrc, uiBytesToCopy);
	return event;
}

extern "C" __declspec(dllexport) void lprefetch(const char* ptr, size_t numElements, size_t elmSize)
{
	size_t totalLines = ((numElements * elmSize) + CPU_DCU_LINE_SIZE - 1) / CPU_DCU_LINE_SIZE;

	for (size_t i=0; i<totalLines; ++i)
	{
		_mm_prefetch(ptr, _MM_HINT_T0);
		ptr += CPU_DCU_LINE_SIZE;
	}
}

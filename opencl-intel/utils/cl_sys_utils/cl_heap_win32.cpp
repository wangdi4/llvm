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

#include <assert.h>
#include <windows.h>

#include "cl_heap.h"
#include "cl_synch_objects.h"

namespace Intel { namespace OpenCL { namespace Utils {

typedef struct
{
	HANDLE heapHandle;
	size_t maxSize;
	size_t userAllocatedSpace;
	size_t actualAllocatedSpace;

	OclMutex critSectionMtx;
} ClHeapInfo_t;

typedef struct
{
	void*  actualPtr;
	size_t userAllocatedSize;
	size_t actualAllocatedSize;
} ClHeapEntry_t;

// Create local heap
int	clCreateHeap(int node, size_t maxHeapSize, ClHeap* phHeap)
{
	assert(phHeap);
	// Let the heap generate exceptions.
	// Make the heap non-thread safe. Use our own serialization.
	HANDLE hHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS | HEAP_NO_SERIALIZE, 0, 0);

	if ( NULL == hHeap)
	{
		return -1;
	}

	ClHeapInfo_t *heapInfo = new ClHeapInfo_t();
	heapInfo->heapHandle = hHeap;
	heapInfo->maxSize = maxHeapSize;
	heapInfo->userAllocatedSpace   = 0;
	heapInfo->actualAllocatedSpace = 0;

	*phHeap = heapInfo;
	return 0;
}

int	clDeleteHeap(ClHeap hHeap)
{
	ClHeapInfo_t *heapInfo = (ClHeapInfo_t*)hHeap;

	BOOL retVal = HeapDestroy(heapInfo->heapHandle);
	if (0 == retVal)
	{
		DWORD dw = GetLastError();
		// Construct error string for debug purposes:
		LPVOID buffer;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &buffer, 0, NULL );
		LocalFree(buffer);
	} else {
		delete heapInfo;
	}
	return retVal;
}


/**
 *__try/__except pattern conflicts with objects DTOR, like CLAutoPtr. So
 * I put the allocation in a separate function.
 * #pragma warning(disable : 4509) // MSVC warns about return after HeapValidate because AutoMutex.
 */
static void *__safeHeapAlloc__(HANDLE heap, const size_t allocSize)
{
	void* ptr;
	DWORD ecode;

	__try
	{
		ptr = HeapAlloc(heap, 0, allocSize);
	}
	__except(ecode = GetExceptionCode())
	{
		if (STATUS_NO_MEMORY == ecode)
		{
			/* The allocation attempt failed because of a 
			 * lack of available memory or heap corruption. */
			int x = 1; // breakpoint for debug
		} else if (STATUS_ACCESS_VIOLATION == ecode){
			/* The allocation attempt failed because of 
			 * heap corruption or improper function parameters. */
			int x = 1; // breakpoint for debug
		}
		HeapValidate(heap, 0, NULL);
	}

	return ptr;
}

void* clAllocateFromHeap(ClHeap hHeap, size_t allocSize, size_t alignment)
{
	ClHeapInfo_t *heapInfo = (ClHeapInfo_t*)hHeap;

	OclAutoMutex critSection(&heapInfo->critSectionMtx);

	if (heapInfo->maxSize < heapInfo->userAllocatedSpace + allocSize)
	{
		return NULL;
	}

	// Allocate with extra space for meta-data and alignment.
	size_t newSize = allocSize + sizeof(ClHeapEntry_t) + alignment - 1;

	void *ptr = __safeHeapAlloc__(heapInfo->heapHandle, newSize);
	if (NULL == ptr)
	{
		return NULL;
	}

	// Find aligned user pointer position.
	size_t uiPtr = (size_t)ptr;
	uiPtr += sizeof(ClHeapEntry_t) + alignment - 1;
	uiPtr &= ~(alignment-1);
	
	void* retPtr = (void*)uiPtr;

	// Fill meta-data.
	ClHeapEntry_t* entryInfo = (ClHeapEntry_t*)(uiPtr - sizeof(ClHeapEntry_t));
	entryInfo->actualPtr = ptr;
	entryInfo->userAllocatedSize   = allocSize;
	entryInfo->actualAllocatedSize = newSize;

	heapInfo->userAllocatedSpace   += allocSize;
	heapInfo->actualAllocatedSpace += newSize;

	return retPtr;
}

int	clFreeHeapPointer(ClHeap hHeap, void* ptr)
{
	ClHeapInfo_t *heapInfo = (ClHeapInfo_t*)hHeap;

	ClHeapEntry_t* entryInfo = (ClHeapEntry_t*)((size_t)ptr - sizeof(ClHeapEntry_t));

	OclAutoMutex critSection(&heapInfo->critSectionMtx);

	heapInfo->userAllocatedSpace   -= entryInfo->userAllocatedSize;
	heapInfo->actualAllocatedSpace -= entryInfo->actualAllocatedSize;

	void* realPtr = entryInfo->actualPtr;
	BOOL retVal = TRUE;

	retVal = HeapFree(heapInfo->heapHandle, 0, realPtr) == TRUE ? 0 : -1;

	if (0 == retVal)
	{
		DWORD dw = GetLastError();
		// Make error visible for debug purposes:
		LPVOID buffer;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &buffer, 0, NULL );
		LocalFree(buffer);
		// no return
	}

	return retVal;
}

}}}

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
#include "cl_sys_defines.h"
#include <map>

using namespace std;

#define MEM_LARGE_ALLOC_TRIGGER (2*PAGE_4K_SIZE)

namespace Intel { namespace OpenCL { namespace Utils {

enum AllocApproach_t {
	USE_HEAP = 0,
	USE_VIRTUAL_ALLOC
};

struct ClHeapEntry_t
{
	void*			actualPtr;
	size_t			userAllocatedSize;
	size_t			actualAllocatedSize;
	AllocApproach_t	use_approach;

};

typedef map<size_t, ClHeapEntry_t> EntryMap_t;

struct ClHeapInfo_t
{
	HANDLE		heapHandle;
	size_t		maxSize;
	size_t		userAllocatedSpace;
	size_t		actualAllocatedSpace;
	
	EntryMap_t	entryMap;
	size_t		numVirtualAllocs;

	OclMutex	critSectionMtx;
};

//////////////////////////////////////////////////////////////////
//
// Internal functions
//
/////////////////////////////////////////////////////////////////

// 4K page alinged
inline void* LargeAlloc( size_t size )
{
	return VirtualAlloc( NULL, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE );
}

inline BOOL LargeFree( void* ptr )
{
	return VirtualFree( ptr, 0, MEM_RELEASE );
}

// remove all entrys 
static void RemoveAll( ClHeapInfo_t* phHeap )
{
	assert( NULL != phHeap );

	if (0 == phHeap->numVirtualAllocs)
	{
		return;
	}

	EntryMap_t::const_iterator it     = phHeap->entryMap.begin();
	EntryMap_t::const_iterator it_end = phHeap->entryMap.end();

	for (; it != it_end; ++it)
	{
		const ClHeapEntry_t& entry = it->second;

		if (USE_VIRTUAL_ALLOC == entry.use_approach)
		{
			LargeFree( entry.actualPtr );
		}
	}

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

//////////////////////////////////////////////////////////////////
//
// Interface functions
//
/////////////////////////////////////////////////////////////////

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

	heapInfo->heapHandle		   = hHeap;
	heapInfo->maxSize			   = (0 != maxHeapSize) ? maxHeapSize : (size_t)(-1);
	heapInfo->userAllocatedSpace   = 0;
	heapInfo->actualAllocatedSpace = 0;
	heapInfo->numVirtualAllocs	   = 0;

	*phHeap = heapInfo;
	return 0;
}

int	clDeleteHeap(ClHeap hHeap)
{
	assert(hHeap);

	ClHeapInfo_t *heapInfo = (ClHeapInfo_t*)hHeap;

	RemoveAll( heapInfo );

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

void* clAllocateFromHeap(ClHeap hHeap, size_t allocSize, size_t alignment)
{
	assert(hHeap);
	assert( IS_ALIGNED_ON(alignment, alignment) && "Alignment is not power of 2" );

	ClHeapInfo_t *heapInfo = (ClHeapInfo_t*)hHeap;

	OclAutoMutex critSection(&heapInfo->critSectionMtx);

	if (heapInfo->maxSize < heapInfo->userAllocatedSpace + allocSize)
	{
		return NULL;
	}

	bool   used_large_alloc;
	size_t real_allocated_size;
	void   *ptr;
	void   *user_ptr;

	if ((allocSize >= MEM_LARGE_ALLOC_TRIGGER) && (alignment <= PAGE_4K_SIZE))
	{
		used_large_alloc = true;

		// align to page boundary
		real_allocated_size = (IS_ALIGNED_ON( allocSize, PAGE_4K_SIZE )) ? 
										allocSize : ALIGN_UP( allocSize, PAGE_4K_SIZE );

		ptr = LargeAlloc( real_allocated_size );
		user_ptr = ptr;
	}
	else
	{
		used_large_alloc	= false;

		// add extra space to allow alignment
		real_allocated_size = allocSize + alignment -1;

		ptr = __safeHeapAlloc__(heapInfo->heapHandle, real_allocated_size);
		user_ptr = (IS_ALIGNED_ON( (size_t)ptr, alignment )) ? 
										ptr : (void*)ALIGN_UP( (size_t)ptr, alignment );

		assert( (((size_t)user_ptr - (size_t)ptr) + allocSize) <= real_allocated_size );
	}

	if (NULL == ptr)
	{
		return NULL;
	}

	// Fill meta-data.
	ClHeapEntry_t& entry = heapInfo->entryMap[ (size_t)user_ptr ];

	entry.actualPtr			  = ptr;
	entry.userAllocatedSize   = allocSize;
	entry.actualAllocatedSize = real_allocated_size;
	entry.use_approach		  = used_large_alloc ? USE_VIRTUAL_ALLOC : USE_HEAP;

	heapInfo->userAllocatedSpace   += allocSize;
	heapInfo->actualAllocatedSpace += real_allocated_size;

	if (used_large_alloc)
	{
		++(heapInfo->numVirtualAllocs);
	}

	return user_ptr;
}

int	clFreeHeapPointer(ClHeap hHeap, void* ptr)
{
	assert(hHeap);

	ClHeapInfo_t *heapInfo = (ClHeapInfo_t*)hHeap;
	BOOL	     retVal;

	OclAutoMutex critSection(&heapInfo->critSectionMtx);

	EntryMap_t::iterator it = heapInfo->entryMap.find( (size_t)ptr );

	if (heapInfo->entryMap.end() != it )
	{
		ClHeapEntry_t& entryInfo = it->second;

		heapInfo->userAllocatedSpace   -= entryInfo.userAllocatedSize;
		heapInfo->actualAllocatedSpace -= entryInfo.actualAllocatedSize;

		BOOL ok;

		if (USE_VIRTUAL_ALLOC == entryInfo.use_approach)
		{
			--(heapInfo->numVirtualAllocs);
			ok = LargeFree( entryInfo.actualPtr );
		}
		else
		{
			ok = HeapFree(heapInfo->heapHandle, 0, entryInfo.actualPtr);
		}

		retVal = (FALSE != ok) ? ERROR_SUCCESS : GetLastError();
		heapInfo->entryMap.erase( it );
	}
	else
	{
		retVal = ERROR_INVALID_ADDRESS; 
	}
	
	if (ERROR_SUCCESS != retVal)
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

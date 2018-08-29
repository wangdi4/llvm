// INTEL CONFIDENTIAL
//
// Copyright 2007-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include <assert.h>
#include <windows.h>

#include "cl_heap.h"
#include "cl_synch_objects.h"
#include "cl_sys_defines.h"
#include <map>
#include <tchar.h>

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
 * DEPRICATED notice, left here for future debugging (if required):
 *__try/__except pattern conflicts with objects DTOR, like CLAutoPtr. So
 * I put the allocation in a separate function.
 * #pragma warning(disable : 4509) // MSVC warns about return after HeapValidate because AutoMutex.
 */
static void *__safeHeapAlloc__(HANDLE heap, const size_t allocSize)
{
	void* ptr;

    // No exceptions. Errors will simply return NULL to ptr.
    ptr = HeapAlloc(heap, 0, allocSize);

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
    HANDLE hHeap = NULL;

#if defined(_M_X64) || defined(__x86_64__)
    // No exceptions, with no internal serialization.
    hHeap = HeapCreate(HEAP_NO_SERIALIZE , 0, 0);
    if (NULL == hHeap)
	{
		return -1;
	}
#else // FOR 32 BIT WINDOWS
    // No exceptions, with internal serialization (for LFH).
    hHeap = HeapCreate(0 , 0, 0);
	if (NULL == hHeap)
	{
		return -1;
	}

    // To enable the LFH for the specified heap, set the variable pointed to by the HeapInformation parameter to 2.
    ULONG heapSetInfo = 2; 
    bool bResult = HeapSetInformation(hHeap, HeapCompatibilityInformation, &heapSetInfo, sizeof(ULONG));
    if (bResult != FALSE) {
        //_tprintf(TEXT("The low-fragmentation heap has been enabled.\n"));
    } else {
		char cBuf[256];
        SPRINTF_S(cBuf, 256, "'CPU Runtime': Failed to enable the low-fragmentation heap with LastError %d."
            " If running from debugger you need to set environment variable _NO_DEBUG_HEAP=1\n",
            GetLastError());
        OutputDebugString(cBuf);
    }
#endif

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

void* clAllocateFromHeap(ClHeap hHeap, size_t allocSize, size_t alignment, bool force_dedicated_pages)
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

	if (force_dedicated_pages || ((allocSize >= MEM_LARGE_ALLOC_TRIGGER) && (alignment <= PAGE_4K_SIZE)))
	{
		used_large_alloc = true;

		// align to page boundary
		real_allocated_size = ALIGN_UP( allocSize, PAGE_4K_SIZE );

		ptr = LargeAlloc( real_allocated_size );
		user_ptr = ptr;
	}
	else
	{
		used_large_alloc	= false;

		// add extra space to allow alignment
		real_allocated_size = allocSize + alignment -1;

		ptr = __safeHeapAlloc__(heapInfo->heapHandle, real_allocated_size);
		user_ptr = (void*)ALIGN_UP( (size_t)ptr, alignment );

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

////////////////////////////////////////////////////////////////////
//
// There is no special treatment on Windows to allow DMA access to memory
//
////////////////////////////////////////////////////////////////////
int clHeapMarkSafeForDMA( void* start, size_t size )
{
	return 0;
}

int clHeapUnmarkSafeForDMA( void* start, size_t size )
{
	return 0;
}

}}}


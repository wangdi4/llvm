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

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

#include "cl_heap.h"
#include "cl_synch_objects.h"
#include "cl_sys_defines.h"
#include <map>

using namespace std;

#define MEM_LARGE_ALLOC_TRIGGER (2*PAGE_4K_SIZE)

namespace Intel { namespace OpenCL { namespace Utils {

enum AllocApproach_t {
	USE_HEAP = 0,
	USE_MMAP
};

struct ClHeapEntry_t
{
	size_t			userAllocatedSize;
	size_t			actualAllocatedSize;
	AllocApproach_t	use_approach;

};

typedef map<size_t, ClHeapEntry_t> EntryMap_t;

struct ClHeapInfo_t
{
	size_t		maxSize;
	size_t		userAllocatedSpace;
	size_t		actualAllocatedSpace;
	
	EntryMap_t	entryMap;
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
	void* ptr = mmap( nullptr, size, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0 );
	return (MAP_FAILED == ptr) ? nullptr : ptr;
}

inline void LargeFree( void* ptr, size_t size )
{
	munmap( ptr, size );
}

// remove all entrys 
static void RemoveAll( ClHeapInfo_t* phHeap )
{
	assert( nullptr != phHeap );

	EntryMap_t::const_iterator it     = phHeap->entryMap.begin();
	EntryMap_t::const_iterator it_end = phHeap->entryMap.end();

	for (; it != it_end; ++it)
	{
		const ClHeapEntry_t& entry = it->second;
		void* actualPtr			   = (void*)(it->first);

		if (USE_MMAP == entry.use_approach)
		{
			LargeFree( actualPtr, entry.actualAllocatedSize );
		}
		else
		{
			ALIGNED_FREE( actualPtr );
		}
	}
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

	ClHeapInfo_t *heapInfo = new ClHeapInfo_t();

	heapInfo->maxSize			   = (0 != maxHeapSize) ? maxHeapSize : (size_t)(-1);
	heapInfo->userAllocatedSpace   = 0;
	heapInfo->actualAllocatedSpace = 0;

	*phHeap = heapInfo;
	return 0;
}

int	clDeleteHeap(ClHeap hHeap)
{
	assert(hHeap);

	ClHeapInfo_t *heapInfo = (ClHeapInfo_t*)hHeap;

	RemoveAll( heapInfo );
	delete heapInfo;

	return 0;
}

void* clAllocateFromHeap(ClHeap hHeap, size_t allocSize, size_t alignment, bool force_dedicated_pages)
{
	assert(hHeap);
	assert( IS_ALIGNED_ON(alignment, alignment) && "Alignment is not power of 2" );

	ClHeapInfo_t *heapInfo = (ClHeapInfo_t*)hHeap;

	OclAutoMutex critSection(&heapInfo->critSectionMtx);

	if (heapInfo->maxSize < heapInfo->userAllocatedSpace + allocSize)
	{
		return nullptr;
	}

	bool   used_large_alloc;
	size_t real_allocated_size;
	void   *ptr;

	if (force_dedicated_pages || ((allocSize >= MEM_LARGE_ALLOC_TRIGGER) && (alignment <= PAGE_4K_SIZE)))
	{
		used_large_alloc = true;

		// align to page boundary
		real_allocated_size = ALIGN_UP( allocSize, PAGE_4K_SIZE );

		ptr = LargeAlloc( real_allocated_size );
	}
	else
	{
		used_large_alloc	= false;
		real_allocated_size = allocSize;

		ptr = ALIGNED_MALLOC( real_allocated_size, alignment );
	}

	if (nullptr == ptr)
	{
		return nullptr;
	}

	// Fill meta-data.
	ClHeapEntry_t& entry = heapInfo->entryMap[ (size_t)ptr ];

	entry.userAllocatedSize   = allocSize;
	entry.actualAllocatedSize = real_allocated_size;
	entry.use_approach		  = used_large_alloc ? USE_MMAP : USE_HEAP;

	heapInfo->userAllocatedSpace   += allocSize;
	heapInfo->actualAllocatedSpace += real_allocated_size;

	return ptr;
}

int	clFreeHeapPointer(ClHeap hHeap, void* ptr)
{
	assert(hHeap);

	ClHeapInfo_t *heapInfo = (ClHeapInfo_t*)hHeap;

	OclAutoMutex critSection(&heapInfo->critSectionMtx);

	EntryMap_t::iterator it = heapInfo->entryMap.find( (size_t)ptr );

	if (heapInfo->entryMap.end() == it )
	{
		return -1;
	}

	ClHeapEntry_t& entryInfo = it->second;

	heapInfo->userAllocatedSpace   -= entryInfo.userAllocatedSize;
	heapInfo->actualAllocatedSpace -= entryInfo.actualAllocatedSize;

	if (USE_MMAP == entryInfo.use_approach)
	{
		LargeFree( ptr, entryInfo.actualAllocatedSize );
	}
	else
	{
		ALIGNED_FREE( ptr );
	}

	heapInfo->entryMap.erase( it );	
	return 0;
}

////////////////////////////////////////////////////////////////////
//
// On Linux when new process is created all parent memory is marked as copy-on-write.
// When one of processes parent or child access memory marked as copy-on-write, this page 
// is copied aside to another physical address. This may create problems for external devices
// that cache page physical addresses for DMA.
//
////////////////////////////////////////////////////////////////////
int clHeapMarkSafeForDMA( void* start, size_t size )
{
	assert( nullptr != start );

	char* end = (char*)start + size;
	char* begin = (char*)ALIGN_DOWN( start, PAGE_4K_SIZE );

	int ret = madvise( begin, end - begin, MADV_DONTFORK ); 
	assert( 0 == ret );
	return ret;
}

int clHeapUnmarkSafeForDMA( void* start, size_t size )
{
	assert( nullptr != start );

	char* end = (char*)start + size;
	char* begin = (char*)ALIGN_DOWN( start, PAGE_4K_SIZE );

	int ret = madvise( begin, end - begin, MADV_DOFORK ); 
	assert( 0 == ret );
	return ret;
}

}}}

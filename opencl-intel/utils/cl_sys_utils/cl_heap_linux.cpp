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

#include "cl_heap.h"

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

namespace Intel { namespace OpenCL { namespace Utils {

// Create local heap
int	clCreateHeap(int node, size_t maxHeapSize, ClHeap* phHeap)
{
	assert(phHeap);

	*phHeap = (ClHeap*)(node+0x10000);
	return 0;
}

int	clDeleteHeap(ClHeap hHeap)
{
	return 0;
}

void* clAllocateFromHeap(ClHeap hHeap, size_t allocSize, size_t aligment)
{
	size_t newSize = allocSize + sizeof(void*) + aligment-1;
	void* ptr = malloc(newSize);

	if ( NULL == ptr )
	{
		return NULL;
	}
	size_t uiPtr = (size_t)ptr;
	uiPtr += sizeof(void*) + aligment-1;
	uiPtr &= ~(aligment-1);
	
	void* retPtr = (void*)uiPtr;
	void** origPtr = (void**)(uiPtr-sizeof(void*));
	*origPtr = ptr;
	return retPtr;
}

int	clFreeHeapPointer(ClHeap hHeap, void* ptr)
{
	size_t uiPtr = (size_t)ptr;
	void** pOrigPtr = (void**)(uiPtr-sizeof(void*));
	void* relPtr = *pOrigPtr;

	free(relPtr);
	return 0;
}

}}}

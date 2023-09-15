// INTEL CONFIDENTIAL
//
// Copyright 2007 Intel Corporation.
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

#pragma once

#include <stddef.h>

namespace Intel {
namespace OpenCL {
namespace Utils {

typedef void *ClHeap;

// Create local heap
//   maxHeapSize == 0 means unlimited heap
int clCreateHeap(int node, size_t maxHeapSize, ClHeap *pHeap);
int clDeleteHeap(ClHeap hHeap);

// Allocate a buffer from heap, assumption alligment is power of 2
void *clAllocateFromHeap(ClHeap hHeap, size_t allocSize, size_t allignent,
                         bool force_dedicated_pages);
int clFreeHeapPointer(ClHeap hHeap, void *ptr);

// Mark pages as safe to be used by DMA HW engines. Required in Linux to avoid
// copy-on-write behavior during fork() return 0 on success
int clHeapMarkSafeForDMA(void *start, size_t size);
int clHeapUnmarkSafeForDMA(void *start, size_t size);

/// Return true if size is considered as large for memory allocation.
bool isLargeAllocSize(size_t size);
} // namespace Utils
} // namespace OpenCL
} // namespace Intel

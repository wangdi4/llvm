// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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

#include "tbb/scalable_allocator.h"

namespace Intel {
namespace OpenCL {
namespace TaskExecutor {

class ScalableMemAllocator {
public:
  static void *scalableMalloc(size_t size) { return scalable_malloc(size); };
  static void scalableFree(void *ptr) { scalable_free(ptr); };
  static void *scalableAlignedMalloc(size_t size, size_t alignment) {
    return scalable_aligned_malloc(size, alignment);
  };
  static void scalableAlignedFree(void *ptr) { scalable_aligned_free(ptr); };
};

} // namespace TaskExecutor
} // namespace OpenCL
} // namespace Intel

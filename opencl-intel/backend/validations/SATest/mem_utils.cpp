// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
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

#include "mem_utils.h"

namespace Validation {

void *align_malloc(size_t size, size_t alignment) {
  void *ptr = NULL;
#if defined(_WIN32) && defined(_MSC_VER)
  ptr = _aligned_malloc(size, alignment);
#elif defined(__linux__) || defined(linux)
  // The value of alignment shall be a multiple of sizeof(void*)
  {
    size_t linuxAlignment;
    size_t remainder = alignment % (sizeof(void *));
    if (remainder) {
      size_t quotient = alignment / (sizeof(void *));
      linuxAlignment = (quotient + 1) * (sizeof(void *));
    } else {
      linuxAlignment = alignment;
    }
    if (0 != posix_memalign(&ptr, linuxAlignment, size)) {
      ptr = NULL;
    }
  }
#elif defined(__MINGW32__)
  ptr = __mingw_aligned_malloc(size, alignment);
#else
#error "Please add support OS for aligned malloc"
#endif
  if (NULL == ptr) {
    throw std::bad_alloc();
  }
  return ptr;
}

// This function implementation must ignore NULL ptr value.
void align_free(void *ptr) {
#if defined(_WIN32) && defined(_MSC_VER)
  _aligned_free(ptr);
#elif defined(__linux__) || defined(linux)
  return free(ptr);
#elif defined(__MINGW32__)
  return __mingw_aligned_free(ptr);
#else
#error "Please add OS support for aligned free"
#endif
}

} // namespace Validation

// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#include "Atomics.h"

#if defined(_MSC_VER)
#include <windows.h>
#endif

#if defined(__GNUC__) || (defined(__INTEL_COMPILER) && !defined(_WIN32))
#define GNU_ATOMICS
#endif

namespace intel {
// short implementation of atomics for thread-safe reference counter
// written on base of llvm/Support/Atomic.h
// our implementation is needed since this file is used in ocl_executor
// that is NOT using/linking LLVM libraries. That's the reason
// to implement own atomics
namespace atomics {

atomic_type AtomicIncrement(atomic_type *ptr) {
#if defined(GNU_ATOMICS)
  return __sync_add_and_fetch(ptr, 1);
#elif defined(_MSC_VER)
  return InterlockedIncrement(ptr);
#else
#error No atomic increment implementation for your platform!
#endif
}

atomic_type AtomicDecrement(atomic_type *ptr) {
#if defined(GNU_ATOMICS)
  return __sync_sub_and_fetch(ptr, 1);
#elif defined(_MSC_VER)
  return InterlockedDecrement(ptr);
#else
#error No atomic decrement implementation for your platform!
#endif
}

} // namespace atomics
} // namespace intel

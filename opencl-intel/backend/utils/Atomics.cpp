/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "Atomics.h"

#if defined(_MSC_VER)
#include <windows.h>
#endif

#if defined(__GNUC__) || (defined(__INTEL_COMPILER) && !defined(_WIN32))
#define GNU_ATOMICS
#endif

namespace intel{
// short implementation of atomics for thread-safe reference counter
// written on base of llvm/Support/Atomic.h
// our implementation is needed since this file is used in ocl_executor
// that is NOT using/linking LLVM libraries. That's the reason
// to implement own atomics
namespace atomics {

atomic_type AtomicIncrement(atomic_type* ptr) {
#if defined(GNU_ATOMICS)
    return __sync_add_and_fetch(ptr, 1);
#elif defined(_MSC_VER)
    return InterlockedIncrement(ptr);
#else
#  error No atomic increment implementation for your platform!
#endif
  }

atomic_type AtomicDecrement(atomic_type* ptr) {
#if defined(GNU_ATOMICS)
    return __sync_sub_and_fetch(ptr, 1);
#elif defined(_MSC_VER)
    return InterlockedDecrement(ptr);
#else
#  error No atomic decrement implementation for your platform!
#endif
}

} // namespace atomics
}// namepace intel

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

#ifndef __ATOMICS_H__
#define __ATOMICS_H__

namespace intel {
// short implementation of atomics for thread-safe reference counter
// written on base of llvm/Support/Atomic.h
// our implementation is needed since this file is used in ocl_mic_executor
// ocl_mic_executor is NOT using/linking LLVM libraries. That's the reason
// to implement our own atomics
namespace atomics {
#ifdef _MSC_VER
typedef long atomic_type;
#else
typedef unsigned atomic_type;
#endif
atomic_type AtomicIncrement(atomic_type *ptr);
atomic_type AtomicDecrement(atomic_type *ptr);
} // namespace atomics
} // namespace intel
#endif // __ATOMICS_H__

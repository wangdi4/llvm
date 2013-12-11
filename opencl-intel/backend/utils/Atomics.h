/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __ATOMICS_H__
#define __ATOMICS_H__


namespace intel{
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
  atomic_type AtomicIncrement(atomic_type* ptr);
  atomic_type AtomicDecrement(atomic_type* ptr);
} // namespace atomics
} // namespace intel
#endif // __ATOMICS_H__

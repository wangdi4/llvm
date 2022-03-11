//===--- DemangleConfig.h ---------------------------------------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a variety of feature test macros copied from
// include/llvm/Support/Compiler.h so that LLVMDemangle does not need to take
// a dependency on LLVMSupport.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_DEMANGLE_DEMANGLECONFIG_H
#define LLVM_DEMANGLE_DEMANGLECONFIG_H

#if INTEL_CUSTOMIZATION
// This is a temporary change to avoid prod build failures.
// This file is part of an ongoing refactoring in llorg which renames this
// file to DemangleConfig.h and modifies it significantly.
// When that change is merged, this customization can be discarded.
#include "llvm/Config/llvm-config.h"
#endif // INTEL_CUSTOMIZATION

#ifndef __has_feature
#define __has_feature(x) 0
#endif

#ifndef __has_cpp_attribute
#define __has_cpp_attribute(x) 0
#endif

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#ifndef DEMANGLE_GNUC_PREREQ
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
#define DEMANGLE_GNUC_PREREQ(maj, min, patch)                           \
  ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) + __GNUC_PATCHLEVEL__ >=          \
   ((maj) << 20) + ((min) << 10) + (patch))
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
#define DEMANGLE_GNUC_PREREQ(maj, min, patch)                           \
  ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) >= ((maj) << 20) + ((min) << 10))
#else
#define DEMANGLE_GNUC_PREREQ(maj, min, patch) 0
#endif
#endif

#if __has_attribute(used) || DEMANGLE_GNUC_PREREQ(3, 1, 0)
#define DEMANGLE_ATTRIBUTE_USED __attribute__((__used__))
#else
#define DEMANGLE_ATTRIBUTE_USED
#endif

#if __has_builtin(__builtin_unreachable) || DEMANGLE_GNUC_PREREQ(4, 5, 0)
#define DEMANGLE_UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER)
#define DEMANGLE_UNREACHABLE __assume(false)
#else
#define DEMANGLE_UNREACHABLE
#endif

#if __has_attribute(noinline) || DEMANGLE_GNUC_PREREQ(3, 4, 0)
#define DEMANGLE_ATTRIBUTE_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define DEMANGLE_ATTRIBUTE_NOINLINE __declspec(noinline)
#else
#define DEMANGLE_ATTRIBUTE_NOINLINE
#endif

#if !defined(NDEBUG)
#define DEMANGLE_DUMP_METHOD DEMANGLE_ATTRIBUTE_NOINLINE DEMANGLE_ATTRIBUTE_USED
#else
#define DEMANGLE_DUMP_METHOD DEMANGLE_ATTRIBUTE_NOINLINE
#endif

#if __cplusplus > 201402L && __has_cpp_attribute(fallthrough)
#define DEMANGLE_FALLTHROUGH [[fallthrough]]
#elif __has_cpp_attribute(gnu::fallthrough)
#define DEMANGLE_FALLTHROUGH [[gnu::fallthrough]]
#elif !__cplusplus
// Workaround for llvm.org/PR23435, since clang 3.6 and below emit a spurious
// error when __has_cpp_attribute is given a scoped attribute in C mode.
#define DEMANGLE_FALLTHROUGH
#elif __has_cpp_attribute(clang::fallthrough)
#define DEMANGLE_FALLTHROUGH [[clang::fallthrough]]
#else
#define DEMANGLE_FALLTHROUGH
#endif

#define DEMANGLE_NAMESPACE_BEGIN namespace llvm { namespace itanium_demangle {
#define DEMANGLE_NAMESPACE_END } }

#endif

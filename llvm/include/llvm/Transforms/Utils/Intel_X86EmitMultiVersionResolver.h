//===-- X86EmitMultiVersionResolver -----------------------------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2022 Intel Corporation
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
// This file implements utilities to generate code used for CPU dispatch code.
// INTEL: Upstreaming attempt is at https://reviews.llvm.org/D108424.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_UTILS_X86EMITMULTIVERSIONRESOLVER_H
#define LLVM_TRANSFORMS_UTILS_X86EMITMULTIVERSIONRESOLVER_H

#include "llvm/ADT/APSInt.h" // INTEL
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"

namespace llvm {
class Value;
class Type;
class Function;
class IRBuilderBase;
class GlobalVariable;

struct MultiVersionResolverOption {
  Function *Fn;
  struct Conds {
    StringRef Architecture;
    llvm::SmallVector<StringRef, 8> Features;

    Conds(StringRef Arch, ArrayRef<StringRef> Feats)
        : Architecture(Arch), Features(Feats.begin(), Feats.end()) {}
  } Conditions;

  MultiVersionResolverOption(Function *Fn, StringRef Arch,
                             ArrayRef<StringRef> Feats)
      : Fn(Fn), Conditions(Arch, Feats) {}
};

// Emits the body of a multiversion function's resolver. Assumes that the
// options are already sorted in the proper order, with the 'default' option
// last (if it exists).
#if INTEL_CUSTOMIZATION
void emitMultiVersionResolver(Function *Resolver,
                              ArrayRef<MultiVersionResolverOption> Options,
                              bool UseIFunc, bool UseLibIRC);
Value *formResolverCondition(IRBuilderBase &Builder,
                             const MultiVersionResolverOption &RO,
                             bool UseLibIRC);
#endif // INTEL_CUSTOMIZATION
namespace X86 {
void emitCPUInit(IRBuilderBase &Builder, bool UseIFunc);
Value *emitCpuIs(IRBuilderBase &Builder, StringRef CPUStr);
Value *emitCpuSupports(IRBuilderBase &Builder, uint64_t FeaturesMask);
Value *emitCpuSupports(IRBuilderBase &Builder, ArrayRef<StringRef> FeatureStrs);
#if INTEL_CUSTOMIZATION
void emitCpuFeaturesInit(IRBuilderBase &Builder, bool UseIFunc);
Value *mayIUseCpuFeatureHelper(IRBuilderBase &Builder,
                               ArrayRef<llvm::APSInt> Pages);
#endif // INTEL_CUSTOMIZATION
} // namespace X86
} // namespace llvm

#endif

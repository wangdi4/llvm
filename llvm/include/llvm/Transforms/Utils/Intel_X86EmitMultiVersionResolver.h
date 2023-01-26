//===--------  Intel_X86EmitMultiVersionResolver.h ------------------------===//
//
// Copyright (C) Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_UTILS_X86EMITMULTIVERSIONRESOLVER_H
#define LLVM_TRANSFORMS_UTILS_X86EMITMULTIVERSIONRESOLVER_H

#include "llvm/ADT/APSInt.h"
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
void emitMultiVersionResolver(Function *Resolver,
                              ArrayRef<MultiVersionResolverOption> Options,
                              bool UseIFunc, bool UseLibIRC);
Value *formResolverCondition(IRBuilderBase &Builder,
                             const MultiVersionResolverOption &RO,
                             bool UseLibIRC);
namespace X86 {
void emitCPUInit(IRBuilderBase &Builder, bool UseIFunc);
Value *emitCpuIs(IRBuilderBase &Builder, StringRef CPUStr);
Value *emitCpuSupports(IRBuilderBase &Builder, uint64_t FeaturesMask);
Value *emitCpuSupports(IRBuilderBase &Builder, ArrayRef<StringRef> FeatureStrs);
void emitCpuFeaturesInit(IRBuilderBase &Builder, bool UseIFunc);
Value *mayIUseCpuFeatureHelper(IRBuilderBase &Builder,
                               ArrayRef<llvm::APSInt> Pages);
} // namespace X86
} // namespace llvm

#endif

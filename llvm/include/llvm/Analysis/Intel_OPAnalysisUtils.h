//===-------------------- Intel_OPAnalysisUtils.h -------------------------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Analysis routines helpful in working with IR that uses opaque pointers.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_OPANALYSISUTILS_H
#define LLVM_ANALYSIS_INTEL_OPANALYSISUTILS_H

#include "llvm/IR/Instructions.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/APInt.h"

namespace llvm {
//
// If the type of 'V' is a pointer type, infer and return its pointer element
// type, or return 'nullptr'.
//
extern Type *inferPtrElementType(Value &V);


// The class below is a helper which analyses opaque pointer
// element types and maps Value(opaque pointer)->Type(element type).
class OpaquePointerTypeMapper {
private:
  using TypeAggregator = Optional<const Type *>;
  using MapType = DenseMap<const Value *, TypeAggregator>;
  MapType PtrETypeMap;

public:
  OpaquePointerTypeMapper(Module &M);
  const Type *getPointerElementType(const Value *) const;
};

} // namespace llvm

#endif // LLVM_ANALYSIS_INTEL_OPANALYSISUTILS_H

//=------------------------ SGFunctionWiden.h -*- C++ -*---------------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===---------------------------------------------------------------------===//

#ifndef BACKEND_SUBGROUP_EMULATION_SG_FUNCTION_WIDEN_H
#define BACKEND_SUBGROUP_EMULATION_SG_FUNCTION_WIDEN_H

#include "SGHelper.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/Analysis/Intel_VectorVariant.h"
#include "llvm/IR/Function.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

#include <vector>

using namespace llvm;

namespace intel {

class FunctionWidener {
private:
  bool EnableDebug;

  SGHelper Helper;

  /// Remove byval attribute for vector parameters.
  void RemoveByValAttr(Function &F);

  /// Make a copy of the function if it is marked as SIMD.
  Function *CloneFunction(Function &F, VectorVariant &V,
                          ValueToValueMapTy &Vmap);

  /// Update uses for widened parameters.
  void expandVectorParameters(Function *Clone, VectorVariant &V,
                              ValueToValueMapTy &Vmap);

  /// Update return value.
  void expandReturn(Function *Clone);

  bool isWideCall(Instruction *I);

  Instruction *getInsertPoint(Instruction *I, Value *V);

  FuncSet FunctionsToWiden;

public:
  explicit FunctionWidener(bool EnableDebug) : EnableDebug(EnableDebug) {}

  void run(FuncSet &Fns, DenseMap<Function *, Function *> &FuncMap);
};

} // namespace intel

#endif

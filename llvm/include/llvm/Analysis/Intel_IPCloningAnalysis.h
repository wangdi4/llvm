//===------- Intel_IPCloningAnalysis.h - IP CloningAnalysis -*------===//
//
// Copyright (C) 2016-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// Intel IP CloningAnalysis
//===----------------------------------------------------------------------===//
//
#ifndef LLVM_ANALYSIS_INTELIPCLONING_H
#define LLVM_ANALYSIS_INTELIPCLONING_H

#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/ValueHandle.h"

namespace llvm {

extern cl::opt<bool> IPCloningTrace;

extern GetElementPtrInst* getAnyGEPAsIncomingValueForPhi(Value *Phi);

extern bool isConstantArgWorthyForSpecializationClone(Value *Arg);

extern bool collectPHIsForSpecialization(Function &F, CallSite CS,
                                       SmallPtrSet<Value *, 8>& PhiValues);

extern bool findPotentialConstsAndApplyHeuristics(Value *V, LoopInfo* LI);

extern bool applyHeuristicsForSpecialization(Function &F, CallSite CS,
                      SmallPtrSet<Value *, 8>& PhiValues, LoopInfo* LI);

extern bool isCallCandidateForSpecialization(CallSite& CS, LoopInfo* LI);

}

#endif

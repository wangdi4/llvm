//===------- Intel_IPCloningAnalysis.h - IP CloningAnalysis -*------===//
//
// Copyright (C) 2016-2019 Intel Corporation. All rights reserved.
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
namespace llvm_cloning_analysis {

extern cl::opt<bool> IPCloningTrace;

extern bool isPointerToCharArray(Type* PTy);

extern GetElementPtrInst* getAnyGEPAsIncomingValueForPhi(Value *Phi);

extern bool isConstantArgWorthyForSpecializationClone(Value *Arg);

extern bool collectPHIsForSpecialization(Function &F, CallBase &CB,
                                       SmallPtrSet<Value *, 8>& PhiValues);

extern bool findPotentialConstsAndApplyHeuristics(Function &F, Value *V,
                                                  LoopInfo* LI, bool AfterInl,
                                                  bool IFSwitchHeuristic,
                                                  unsigned *IFCount = nullptr,
                                                  unsigned *SwitchCount
                                                      = nullptr);

extern bool applyHeuristicsForSpecialization(Function &F, CallBase &CB,
                      SmallPtrSet<Value *, 8>& PhiValues, LoopInfo* LI);

extern bool isCallCandidateForSpecialization(CallBase& CB, LoopInfo* LI);

}
}

#endif

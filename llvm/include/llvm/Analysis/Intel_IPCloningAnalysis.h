#if INTEL_FEATURE_SW_ADVANCED
//===------- Intel_IPCloningAnalysis.h - IP CloningAnalysis -*------===//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
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

extern bool IPCloningTrace;

extern bool isCharArray(Type* ATy);

extern GetElementPtrInst* getAnyGEPAsIncomingValueForPhi(Value *Phi);

extern bool isConstantArgWorthyForSpecializationClone(Argument *FormalV,
                                                      Value *ActualV);

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

#endif // LLVM_ANALYSIS_INTELIPCLONING_H
#endif // INTEL_FEATURE_SW_ADVANCED

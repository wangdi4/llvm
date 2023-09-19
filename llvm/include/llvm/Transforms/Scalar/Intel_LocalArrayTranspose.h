//===-- Intel_LocalArrayTranspose.h - LocalArrayTransposePass -------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This optimization intechanges the dimensions of 2D local dope-vector based
// arrays when doing so improves the locality of indexing.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_LOCAL_ARRAY_TRANSPOSE_H
#define INTEL_LOCAL_ARRAY_TRANSPOSE_H

#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class LoopInfo;

class LocalArrayTransposePass : public PassInfoMixin<LocalArrayTransposePass> {
  // AllocaInsts for local dope-vector based arrays that are candidates for
  // transposing.
  SetVector<AllocaInst *> Candidates;

public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

private:
  // Return 'true' if 'AI' is square, i.e. it is a 2D array where the lower
  // bounds and extents match.
  bool isSquareAllocatableArray(
      AllocaInst *AI,
      std::function<const TargetLibraryInfo &(Function &)> GetTLI);
  // Return the call to for_alloc_allocatable_handle() which allocates
  // the array pointed to by 'GEPI'.
  CallBase *findAllocHandle(GetElementPtrInst *GEPI,
                            const TargetLibraryInfo &TLI);
  // Return the call to for_dealloc_allocatable_handle() which allocates
  // the array pointed to by 'GEPI'.
  CallBase *findDeallocHandle(GetElementPtrInst *GEPI,
                              const TargetLibraryInfo &TLI);
  // Return 'true' if 'SBI' is a SubscriptInst with 'LI' as its pointer
  // operand and which has one user which is also a SubscriptInst. In
  // this case, it is the base of a canonical indexing of a 2D array.
  bool isValidSubscriptInst(SubscriptInst *SBI, LoadInst *LI);
  // Return 'true' if 'PHIN' has incoming values, each of which is null
  // or a Value whose use is 'CB1'. When 'CB1' is the deallocation call,
  // 'PHIN' is used to form an array reference post-deallocation.
  bool isValidPHINode(PHINode *PHIN, CallBase *CB1);
  // Return 'true' if 'SI' has 'GEPI' as its pointer operand and is either
  // an initializing store, or is a clearing store performed after deallocation.
  // In either case, the store will not affect the values in the array
  // referenced by 'GEPI' while the array is allocated.
  bool isValidStoreInst(StoreInst *SI, GetElementPtrInst *GEPI,
                        const TargetLibraryInfo &TLI);
  // Indicate 'AI' is a candidate for local array transpose.
  void addCandidate(AllocaInst *AI);
  // Return 'true' if 'LI' is dominated by 'CB0' and postdominated by 'CB1'.
  // Here 'CB0' will be the call to for_alloc_allocatable_handle() and
  // 'CB1' will be the call to 'for_dealloc_allocatble_handle(). If 'true'
  // is returned, we know the array accessed by 'LI' is accessed while
  // the array is allocated.
  bool PassesDominanceCheck(LoadInst *LI, CallBase *CB0, CallBase *CB1,
                            DominatorTree &DT, PostDominatorTree &PDT);
  // Return 'true' if 'AI' passes all legality checks needed for it to
  // be a candidate for local array transpose.
  bool
  isValidCandidate(AllocaInst *AI, DominatorTree &DT, PostDominatorTree &PDT,
                   std::function<const TargetLibraryInfo &(Function &)> GetTLI);
  // Find legal candidates for local array transpose for 'F'.
  unsigned findValidCandidates(
      Function &F, DominatorTree &DT, PostDominatorTree &PDT,
      std::function<const TargetLibraryInfo &(Function &)> GetTLI);
  // Print the current set of local array transpose candidates for 'F'.
  void printCandidates(Function &F, StringRef Banner);
  // Return 'true' if 'AI' is a profitable candidate for local array transpose.
  bool isProfitableCandidate(AllocaInst *AI, LoopInfo &LI);
  // Remove candidates which are not profitable.
  unsigned findProfitableCandidates(LoopInfo &LI);
  // Transform 'AI' by transposing its indices.
  void transformCandidate(AllocaInst *AI, LoopInfo &LI);
  // Transpose the indices of all profitable candidates.
  void transformCandidates(LoopInfo &LI);
  // Run local array transpose  on 'F'.
  bool runImpl(Function &F, DominatorTree &DT, PostDominatorTree &PDT,
               LoopInfo &LI,
               std::function<const TargetLibraryInfo &(Function &)> GetTLI);
};

} // end namespace llvm

#endif // INTEL_LOCAL_ARRAY_TRANSPOSE_H

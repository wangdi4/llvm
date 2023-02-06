//===- JumpThreading.h - thread control through conditional BBs -*- C++ -*-===//
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
/// \file
/// See the comments on JumpThreadingPass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_JUMPTHREADING_H
#define LLVM_TRANSFORMS_SCALAR_JUMPTHREADING_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/IR/ValueHandle.h"
#include <utility>

namespace llvm {

class AAResults;
class BasicBlock;
class BinaryOperator;
class BranchInst;
class CmpInst;
class Constant;
class DomTreeUpdater;
class Function;
class Instruction;
class IntrinsicInst;
class LazyValueInfo;
class LoadInst;
class PHINode;
class SelectInst;
class SwitchInst;
class TargetLibraryInfo;
class TargetTransformInfo;
class Value;

/// A private "module" namespace for types and utilities used by
/// JumpThreading.
/// These are implementation details and should not be used by clients.
namespace jumpthreading {

// These are at global scope so static functions can use them too.
using PredValueInfo = SmallVectorImpl<std::pair<Constant *, BasicBlock *>>;
using PredValueInfoTy = SmallVector<std::pair<Constant *, BasicBlock *>, 8>;

#if INTEL_CUSTOMIZATION
  ///   A simple extension to jump threading across a single BB is threading
  /// across a multi-block region described by a <BBTop, BBBottom> pair,
  /// where BBBottom ends in a multi-way branch that can be pre-determined
  /// based on an incoming edge to BBTop and where BBTop dominates BBBottom.
  /// (BBTop needn't strictly dominate BBBottom, i.e. BBTop == BBBottom is ok.)
  /// We can duplicate every block on every path from BBTop to BBBottom and
  /// adjust the control flow in the same way as for the single BB case, i.e.
  /// BBTop's pred of interest is redirected to BBTop.thread, and the multi-way
  /// branch at the end of BBBottom.thread is replaced by an unconditional
  /// branch to the pre-determined successor. There can be other exits from the
  /// region, but IF control gets from BBTop.thread to BBBottom.thread, we save
  /// the overhead of the multi-way branch.
  ///
  ///   Taking the idea one step further, we can put multiple <BBTop, BBBottom>
  /// pairs together, e.g. <BBTop2, BBBottom2>, <BBTop1, BBBottom1>, to
  /// describe a thread region headed by BBTop1 and ending with BBBottom2 where
  /// BBTop1 does not dominate BBBottom2. There is an edge from BBBottom1 to
  /// BBTop2 that will be preserved in the threaded code, but there might be
  /// other incoming edges to BBTop2 that will not. The individual pairs (call
  /// them sub-regions) have the same control flow properties described above,
  /// i.e. BBTopN dominates BBBottomN & all blocks on all paths from BBTopN to
  /// BBBottomN get duplicated as part of the jump threading transformation.
  /// The sub-regions are in reverse order since it is convenient to construct
  /// them that way.
  ///
  typedef SmallVectorImpl<std::pair<BasicBlock*,BasicBlock*> >
      ThreadRegionInfo;
  typedef SmallVector<std::pair<BasicBlock*,BasicBlock*>, 4>
      ThreadRegionInfoTy;
  typedef SmallVectorImpl<std::pair<BasicBlock*,BasicBlock*> >::const_iterator
    ThreadRegionInfoIterator;
#endif // INTEL_CUSTOMIZATION

// This is used to keep track of what kind of constant we're currently hoping
// to find.
enum ConstantPreference { WantInteger, WantBlockAddress };

} // end namespace jumpthreading

/// This pass performs 'jump threading', which looks at blocks that have
/// multiple predecessors and multiple successors.  If one or more of the
/// predecessors of the block can be proven to always jump to one of the
/// successors, we forward the edge from the predecessor to the successor by
/// duplicating the contents of this block.
///
/// An example of when this can occur is code like this:
///
///   if () { ...
///     X = 4;
///   }
///   if (X < 3) {
///
/// In this case, the unconditional branch at the end of the first if can be
/// revectored to the false side of the second if.
class JumpThreadingPass : public PassInfoMixin<JumpThreadingPass> {
  TargetLibraryInfo *TLI;
  TargetTransformInfo *TTI;
  LazyValueInfo *LVI;
  AAResults *AA;
  DomTreeUpdater *DTU;
  PostDominatorTree *PDT; // INTEL
  std::unique_ptr<BlockFrequencyInfo> BFI;
  std::unique_ptr<BranchProbabilityInfo> BPI;
  bool HasProfileData = false;
  bool HasGuards = false;
#ifndef LLVM_ENABLE_ABI_BREAKING_CHECKS
  SmallPtrSet<const BasicBlock *, 16> LoopHeaders;
  SmallPtrSet<const BasicBlock *, 16> CountableSingleExitLoopHeaders; // INTEL
  SmallPtrSet<const BasicBlock *, 16> CountableSingleExitLoopLatches; // INTEL
#else
  SmallSet<AssertingVH<const BasicBlock>, 16> LoopHeaders;
  SmallSet<AssertingVH<const BasicBlock>, 16>       // INTEL
                    CountableSingleExitLoopHeaders; // INTEL
  SmallSet<AssertingVH<const BasicBlock>, 16>       // INTEL
                    CountableSingleExitLoopLatches;  // INTEL
#endif

  unsigned BBDupThreshold;
  unsigned DefaultBBDupThreshold;

#if INTEL_CUSTOMIZATION
  // Jump threading performs several CFG simplifications that are not
  // in themselves jump threading but rather are attempts to expose more jump
  // threading opportunities. These simplifications can interfere with
  // optimizations in the simplify CFG pass, specifically the if-to-switch
  // conversion, so we suppress them when this pass is run prior to CFG
  // simplification.
  bool DoCFGSimplifications;

  // Count the number of times that each block has been threaded, and stop
  // threading across a block once this count reaches the threshold. This is
  // a fail-safe to ensure that jump threading terminates when we allow
  // threading across loop headers.
  DenseMap<BasicBlock*, int> BlockThreadCount;
  static const int MaxThreadsPerBlock = 10;
#endif // INTEL_CUSTOMIZATION

public:
  JumpThreadingPass(int T = -1, // INTEL
                    bool AllowCFGSimps = true); // INTEL

  // Glue for old PM.
  bool runImpl(Function &F, TargetLibraryInfo *TLI, TargetTransformInfo *TTI,
               LazyValueInfo *LVI, AAResults *AA, DomTreeUpdater *DTU,
               bool HasProfileData, std::unique_ptr<BlockFrequencyInfo> BFI,
               std::unique_ptr<BranchProbabilityInfo> BPI, // INTEL
               PostDominatorTree *PDT_);  // INTEL

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  void releaseMemory() {
    BFI.reset();
    BPI.reset();
  }

  void findLoopHeaders(Function &F);
  bool processBlock(BasicBlock *BB);
  bool maybeMergeBasicBlockIntoOnlyPred(BasicBlock *BB);
  void updateSSA(BasicBlock *BB, BasicBlock *NewBB,
                 DenseMap<Instruction *, Value *> &ValueMapping);
  DenseMap<Instruction *, Value *> cloneInstructions(BasicBlock::iterator BI,
                                                     BasicBlock::iterator BE,
                                                     BasicBlock *NewBB,
                                                     BasicBlock *PredBB);
  bool tryThreadEdge(const jumpthreading::ThreadRegionInfo &RegionInfo, // INTEL
                     const SmallVectorImpl<BasicBlock*> &PredBBs,
                     BasicBlock *SuccBB);
  void threadEdge(const jumpthreading::ThreadRegionInfo &RegionInfo,  // INTEL
                  const SmallVectorImpl<BasicBlock *> &RegionBlocks,  // INTEL
                  bool ThreadingLoopHeader,                           // INTEL
                  const SmallVectorImpl<BasicBlock*> &PredBBs,        // INTEL
                  BasicBlock *SuccBB);
  bool duplicateCondBranchOnPHIIntoPred(
      BasicBlock *BB, const SmallVectorImpl<BasicBlock *> &PredBBs);

  bool computeValueKnownInPredecessorsImpl(
      Value *V, BasicBlock *BB, jumpthreading::PredValueInfo &Result,
      jumpthreading::ThreadRegionInfo &RegionInfo, // INTEL
      jumpthreading::ConstantPreference Preference,
      DenseSet<Value *> &RecursionSet, Instruction *CxtI = nullptr);
  bool
  computeValueKnownInPredecessors(Value *V, BasicBlock *BB,
                                  jumpthreading::PredValueInfo &Result,
                                  jumpthreading::ThreadRegionInfo &RegionInfo, // INTEL
                                  jumpthreading::ConstantPreference Preference,
                                  Instruction *CxtI = nullptr) {
    DenseSet<Value *> RecursionSet;
    return computeValueKnownInPredecessorsImpl(V, BB, Result, RegionInfo,       // INTEL
                                               Preference, RecursionSet, CxtI); // INTEL
  }

  Constant *evaluateOnPredecessorEdge(BasicBlock *BB, BasicBlock *PredPredBB,
                                      Value *cond);
  bool maybethreadThroughTwoBasicBlocks(BasicBlock *BB, Value *Cond);
  void threadThroughTwoBasicBlocks(BasicBlock *PredPredBB, BasicBlock *PredBB,
                                   BasicBlock *BB, BasicBlock *SuccBB);
  bool processThreadableEdges(Value *Cond, BasicBlock *BB,
                              jumpthreading::ConstantPreference Preference,
                              Instruction *CxtI = nullptr);

  bool processBranchOnPHI(PHINode *PN);
  bool processBranchOnXOR(BinaryOperator *BO);
  bool processBranchOnOr(BasicBlock *BB); // INTEL
  bool processImpliedCondition(BasicBlock *BB);

  bool simplifyPartiallyRedundantLoad(LoadInst *LI);
  void unfoldSelectInstr(BasicBlock *Pred, BasicBlock *BB, SelectInst *SI,
                         PHINode *SIUse, unsigned Idx);

  bool tryToUnfoldSelect(CmpInst *CondCmp, BasicBlock *BB);
  bool tryToUnfoldSelect(SwitchInst *SI, BasicBlock *BB);
  bool tryToUnfoldSelectInCurrBB(BasicBlock *BB);

  bool processGuards(BasicBlock *BB);
  bool threadGuard(BasicBlock *BB, IntrinsicInst *Guard, BranchInst *BI);

private:
  BasicBlock *splitBlockPreds(BasicBlock *BB, ArrayRef<BasicBlock *> Preds,
                              const char *Suffix);
#if INTEL_CUSTOMIZATION
    void updateRegionBlockFreqAndEdgeWeight(BasicBlock *PredBB,
                       BasicBlock *SuccBB,
                       const jumpthreading::ThreadRegionInfo &RegionInfo,
                       const SmallVectorImpl<BasicBlock*> &RegionBlocks,
                       DenseMap<BasicBlock*, BasicBlock*> &BlockMapping);
#endif // INTEL_CUSTOMIZATION
  /// Check if the block has profile metadata for its outgoing edges.
  bool doesBlockHaveProfileData(BasicBlock *BB);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_JUMPTHREADING_H

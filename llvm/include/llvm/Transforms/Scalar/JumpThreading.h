//===- JumpThreading.h - thread control through conditional BBs -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
/// See the comments on JumpThreadingPass.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_JUMPTHREADING_H
#define LLVM_TRANSFORMS_SCALAR_JUMPTHREADING_H

#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BlockFrequencyInfoImpl.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/LazyValueInfo.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/ValueHandle.h"

namespace llvm {

/// A private "module" namespace for types and utilities used by
/// JumpThreading.
/// These are implementation details and should not be used by clients.
namespace jumpthreading {
// These are at global scope so static functions can use them too.
typedef SmallVectorImpl<std::pair<Constant *, BasicBlock *>> PredValueInfo;
typedef SmallVector<std::pair<Constant *, BasicBlock *>, 8> PredValueInfoTy;

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
}

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
///
class JumpThreadingPass : public PassInfoMixin<JumpThreadingPass> {
  TargetLibraryInfo *TLI;
  LazyValueInfo *LVI;
  AliasAnalysis *AA;
  std::unique_ptr<BlockFrequencyInfo> BFI;
  std::unique_ptr<BranchProbabilityInfo> BPI;
  bool HasProfileData = false;
  bool HasGuards = false;
#ifdef NDEBUG
  SmallPtrSet<const BasicBlock *, 16> LoopHeaders;
#else
  SmallSet<AssertingVH<const BasicBlock>, 16> LoopHeaders;
#endif
  DenseSet<std::pair<Value *, BasicBlock *>> RecursionSet;

  unsigned BBDupThreshold;

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

  // RAII helper for updating the recursion stack.
  struct RecursionSetRemover {
    DenseSet<std::pair<Value *, BasicBlock *>> &TheSet;
    std::pair<Value *, BasicBlock *> ThePair;

    RecursionSetRemover(DenseSet<std::pair<Value *, BasicBlock *>> &S,
                        std::pair<Value *, BasicBlock *> P)
        : TheSet(S), ThePair(P) {}

    ~RecursionSetRemover() { TheSet.erase(ThePair); }
  };

public:
  JumpThreadingPass(int T = -1, bool AllowCFGSimps = true); // INTEL

  // Glue for old PM.
  bool runImpl(Function &F, TargetLibraryInfo *TLI_, LazyValueInfo *LVI_,
               AliasAnalysis *AA_, bool HasProfileData_,
               std::unique_ptr<BlockFrequencyInfo> BFI_,
               std::unique_ptr<BranchProbabilityInfo> BPI_);

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  void releaseMemory() {
    BFI.reset();
    BPI.reset();
  }

  void FindLoopHeaders(Function &F);
  bool ProcessBlock(BasicBlock *BB);
  bool ThreadEdge(const jumpthreading::ThreadRegionInfo &RegionInfo,  // INTEL
                  const SmallVectorImpl<BasicBlock*> &PredBBs,        // INTEL
                  BasicBlock *SuccBB);
  bool DuplicateCondBranchOnPHIIntoPred(
      BasicBlock *BB, const SmallVectorImpl<BasicBlock *> &PredBBs);

  bool
  ComputeValueKnownInPredecessors(Value *V, BasicBlock *BB,
                                  jumpthreading::PredValueInfo &Result,
                           jumpthreading::ThreadRegionInfo &RegionInfo, // INTEL
                                  jumpthreading::ConstantPreference Preference,
                                  Instruction *CxtI = nullptr);
  bool ProcessThreadableEdges(Value *Cond, BasicBlock *BB,
                              jumpthreading::ConstantPreference Preference,
                              Instruction *CxtI = nullptr);

  bool ProcessBranchOnPHI(PHINode *PN);
  bool ProcessBranchOnXOR(BinaryOperator *BO);
  bool ProcessImpliedCondition(BasicBlock *BB);

  bool SimplifyPartiallyRedundantLoad(LoadInst *LI);
  bool TryToUnfoldSelect(CmpInst *CondCmp, BasicBlock *BB);
  bool TryToUnfoldSelectInCurrBB(BasicBlock *BB);

  bool ProcessGuards(BasicBlock *BB);
  bool ThreadGuard(BasicBlock *BB, IntrinsicInst *Guard, BranchInst *BI);

private:
  BasicBlock *SplitBlockPreds(BasicBlock *BB, ArrayRef<BasicBlock *> Preds,
                              const char *Suffix);
#if INTEL_CUSTOMIZATION
    void UpdateRegionBlockFreqAndEdgeWeight(BasicBlock *PredBB,
                       BasicBlock *SuccBB,
                       const jumpthreading::ThreadRegionInfo &RegionInfo,
                       const SmallVectorImpl<BasicBlock*> &RegionBlocks,
                       DenseMap<BasicBlock*, BasicBlock*> &BlockMapping);
#endif // INTEL_CUSTOMIZATION
  /// Check if the block has profile metadata for its outgoing edges.
  bool doesBlockHaveProfileData(BasicBlock *BB);
};

} // end namespace llvm

#endif

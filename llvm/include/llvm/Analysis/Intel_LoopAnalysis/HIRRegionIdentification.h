//===---- HIRRegionIdentification.h - Identifies HIR regions ---*- C++ --*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This analysis is used to identify sections(regions) of LLVM IR on which loop
// transformations can be applied.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_REGIONIDENTIFICATION_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_REGIONIDENTIFICATION_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"

#include "llvm/Pass.h"

#include "llvm/IR/Intel_LoopIR/IRRegion.h"

namespace llvm {

class Value;
class Function;
class Instruction;
class PHINode;
class Loop;
class LoopInfo;
class DominatorTree;
struct PostDominatorTree;
class ScalarEvolution;
class GetElementPtrInst;
class GEPOperator;
class SCEV;
class TargetLibraryInfo;

namespace loopopt {

/// This analysis is the first step in creating HIR. We start by
/// identiyfing regions as a set of basic blocks in the incoming IR. This
/// information is then used by HIRCreation pass to create and populate
/// HIR regions.
class HIRRegionIdentification : public FunctionPass {
public:
  typedef SmallVector<IRRegion, 16> IRRegionsTy;

  /// Iterators to iterate over regions
  typedef IRRegionsTy::iterator iterator;
  typedef IRRegionsTy::const_iterator const_iterator;
  typedef IRRegionsTy::reverse_iterator reverse_iterator;
  typedef IRRegionsTy::const_reverse_iterator const_reverse_iterator;

private:
  /// Vector of IRRegion.
  IRRegionsTy IRRegions;

  /// The loop information for the function we are currently analyzing.
  LoopInfo *LI;

  /// The dominator tree.
  DominatorTree *DT;

  /// The post-dominator tree.
  PostDominatorTree *PDT;

  /// Scalar Evolution analysis for the function.
  ScalarEvolution *SE;

  /// Target library information for the target.
  TargetLibraryInfo *TLI;

  /// CostModelAnalyzer - Used to determine whether creating HIR for a loop
  /// would be profitable.
  class CostModelAnalyzer;

  /// Returns true if this loop should be throttled based on cost model
  /// analysis.
  bool shouldThrottleLoop(const Loop &Lp, bool IsUnknown) const;

  /// Returns true if bblocks of \p Lp are generable.
  bool areBBlocksGenerable(const Loop &Lp) const;

  /// Returns true if Lp appears to be generable without looking at the sub
  /// loops.
  bool isSelfGenerable(const Loop &Lp, unsigned LoopnestDepth) const;

  /// Returns true if this instruction represents simd begin/end directive. \p
  /// BeginDir flag indicates whether to look for begin or end directive.
  static bool isSIMDDirective(const Instruction *Inst, bool BeginDir);

  /// Returns true if this bblock contains simd begin/end directive. \p BeginDir
  /// flag indicates whether to look for begin or end directive.
  static bool containsSIMDDirective(const BasicBlock *BB, bool BeginDir);

  /// Traces a chain of single predecessor/successor bblocks starting from \p BB
  /// and looks for simd begin/end directive. Returns the bblock containing the
  /// directive.
  static BasicBlock *findSIMDDirective(BasicBlock *BB, bool BeginDir);

  /// Inserts chain of bblocks from BeginBB to EndBB inclusive, to RegBBlocks.
  void addBBlocks(const BasicBlock *BeginBB, const BasicBlock *EndBB,
                  IRRegion::RegionBBlocksTy &RegBBlocks) const;

  /// Returns true if Lp is a SIMD loop. If RegBBlocks is non-null, it adds
  /// simd loop predecess/successor bblocks to it. Entry/Exit bblocks for the
  /// simd loop region are returned via \p EntryBB and \p ExitBB.
  bool isSIMDLoop(const Loop &Lp,
                  IRRegion::RegionBBlocksTy *RegBBlocks = nullptr,
                  BasicBlock **RegEntryBB = nullptr,
                  BasicBlock **RegExitBB = nullptr) const;

  /// Creates a Region out of Lp's basic blocks.
  void createRegion(const Loop &Lp);

  /// Returns true if we can form a region around this loop. Returns the max
  /// loopnest depth in LoopnestDepth.
  bool formRegionForLoop(const Loop &Lp, unsigned *LoopnestDepth);

  /// Identifies regions in the incoming LLVM IR.
  void formRegions();

  /// Returns true if \p GEPOp contains a type not supported by HIR.
  static bool containsUnsupportedTy(const GEPOperator *GEPOp);

  /// Returns true if \p Inst contains a type not supported by HIR.
  static bool containsUnsupportedTy(const Instruction *Inst);

  /// Implements isReachableFrom().
  bool
  isReachableFromImpl(const BasicBlock *BB,
                      const SmallPtrSetImpl<const BasicBlock *> &EndBBs,
                      const SmallPtrSetImpl<const BasicBlock *> &FromBBs,
                      SmallPtrSetImpl<const BasicBlock *> &VisitedBBs) const;

  /// Returns true if dominator children of \p BB in \p Lp are involved in a
  /// cycle which doesn't go through backedges. This indicates irreducible CFG.
  bool containsCycle(const BasicBlock *BB, const Loop &Lp) const;

  /// Returns true if metadata node \p Node contains only debug metadata or
  /// equals null.
  static bool isDebugMetadataOnly(MDNode *Node);

public:
  static char ID; // Pass identification
  HIRRegionIdentification();

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;
  void verifyAnalysis() const override;

  /// IRRegion iterator methods
  iterator begin() { return IRRegions.begin(); }
  const_iterator begin() const { return IRRegions.begin(); }
  iterator end() { return IRRegions.end(); }
  const_iterator end() const { return IRRegions.end(); }

  reverse_iterator rbegin() { return IRRegions.rbegin(); }
  const_reverse_iterator rbegin() const { return IRRegions.rbegin(); }
  reverse_iterator rend() { return IRRegions.rend(); }
  const_reverse_iterator rend() const { return IRRegions.rend(); }

  unsigned getNumRegions() const { return IRRegions.size(); }

  /// Returns true if this type is supported. Currently returns false for
  /// structure and function types.
  static bool isSupported(Type *Ty);

  // NOTE: Following functions were moved here so they can be shared between
  // HIRParser and SSADeconstruction. Is there a better way?

  /// Returns the primary element type of PtrTy. Primary element type is the
  /// underlying type which this type is pointing to. For example, the primary
  /// element type of both i32* && [100 x [100 x i32]]* is i32.
  Type *getPrimaryElementType(Type *PtrTy) const;

  /// Returns true if Phi occurs in the header of a loop.
  bool isHeaderPhi(const PHINode *Phi) const;

  /// Returns IV definition PHINode of the loop.
  const PHINode *findIVDefInHeader(const Loop &Lp,
                                   const Instruction *Inst) const;

  /// Returns true if \p BB can be reached from any of the \p FromBBs before
  /// hitting any \p EndBBs and without going through any backedges.
  bool
  isReachableFrom(const BasicBlock *BB,
                  const SmallPtrSetImpl<const BasicBlock *> &EndBBs,
                  const SmallPtrSetImpl<const BasicBlock *> &FromBBs) const;
};

} // End namespace loopopt

} // End namespace llvm

#endif

//===---- HIRRegionIdentification.h - Identifies HIR regions ---*- C++ --*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/IR/IRRegion.h"
#include "llvm/Analysis/ScalarEvolution.h"

namespace llvm {

class Value;
class Function;
class Instruction;
class PHINode;
class Loop;
class AssumptionCache;
class LoopInfo;
class DominatorTree;
class PostDominatorTree;
class GetElementPtrInst;
class GEPOrSubsOperator;
class SCEV;
class TargetLibraryInfo;
class MDNode;
class Type;

namespace loopopt {

/// This analysis is the first step in creating HIR. We start by
/// identiyfing regions as a set of basic blocks in the incoming IR. This
/// information is then used by HIRCreation pass to create and populate
/// HIR regions.
class HIRRegionIdentification {
  const unsigned MaxFusionTripCountDiff = 3;
  const unsigned MaxIntermediateBBsForFusion = 10;
  const unsigned FusedRegionTCThreshold = 16;

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

  /// Set of all basic blocks which are present in regions created for loop(s).
  DenseSet<const BasicBlock *> AllLoopBasedRegionsBBlocks;

  /// The loop information for the function we are currently analyzing.
  LoopInfo &LI;

  /// The dominator tree.
  DominatorTree &DT;

  /// The post-dominator tree.
  PostDominatorTree &PDT;

  /// Scalar Evolution analysis for the function.
  ScalarEvolution &SE;

  /// Target library information for the target.
  TargetLibraryInfo &TLI;

  /// Scoped Scalar Evolution analysis for the function.
  std::unique_ptr<ScopedScalarEvolution> ScopedSE;

  unsigned OptLevel;

  /// CostModelAnalyzer - Used to determine whether creating HIR for a loop
  /// would be profitable.
  class CostModelAnalyzer;

  /// Returns true if \p Inst contains a type not supported by HIR.
  static bool containsUnsupportedTy(const Instruction *Inst,
                                    const Loop *Lp = nullptr);

  /// Implements isReachableFrom().
  bool
  isReachableFromImpl(const BasicBlock *BB,
                      const SmallPtrSetImpl<const BasicBlock *> &EndBBs,
                      const SmallPtrSetImpl<const BasicBlock *> &FromBBs,
                      SmallPtrSetImpl<const BasicBlock *> &VisitedBBs) const;

  /// Returns true if \p BB is generable (can be handled by HIR). \p Lp is
  /// passed as null for function level region mode.
  static bool isGenerable(const BasicBlock *BB, const Loop *Lp);

  /// Returns true if bblocks of \p Lp are generable.
  bool areBBlocksGenerable(const Loop &Lp) const;

  /// Returns true if Lp appears to be generable without looking at the sub
  /// loops. \p IsFunctionRegionMode is set to true when we want to form a
  /// single function level region.
  bool isSelfGenerable(const Loop &Lp, unsigned LoopnestDepth,
                       bool IsFunctionRegionMode,
                       bool &ThrottleParentLoop) const;

  /// Returns true if this loop should be throttled based on cost model
  /// analysis.
  bool shouldThrottleLoop(const Loop &Lp, const SCEV *BECount,
                          bool &ThrottleParentLoop) const;

  /// Creates a Region out of Loops' and \p IntermediateBlocks basic blocks.
  void
  createRegion(const ArrayRef<const Loop *> &Loops,
               const SmallPtrSetImpl<const BasicBlock *> *IntermediateBlocks);

  /// Returns true if we can form a region around this loop. Returns the max
  /// loopnest depth in \p LoopnestDepth. \p GenerableLoops contains \p Lp if it
  /// is generable, otherwise it contains generable children loops of \p Lp.
  bool isGenerableLoopnest(const Loop &Lp, unsigned &LoopnestDepth,
                           SmallVectorImpl<const Loop *> &GenerableLoops);

  typedef std::pair<SmallVector<const Loop *, 4>,
                    SmallPtrSet<const BasicBlock *, 4>>
      LoopSpanTy;

  /// Compute \p Spans vector with successive loops together with intermediate
  /// basic blocks to be combined into a single region.
  void computeLoopSpansForFusion(const SmallVectorImpl<const Loop *> &Loops,
                                 SmallVectorImpl<LoopSpanTy> &Spans);

  /// Collect all basic blocks (\p BBs) that may be reached from the \p Loops
  /// exits until it reaches another loop.
  /// Returns true if the single Region may be created for \p Loops and \p BBs.
  bool collectIntermediateBBs(const Loop *Loop1, const Loop *Loop2,
                              SmallPtrSetImpl<const BasicBlock *> &BBs);

  /// Returns the lexical insertion position of \p BB in IRRegions.
  HIRRegionIdentification::iterator
  getLexicalInsertionPos(const BasicBlock *BB);

  /// Forms regions out of function bblocks which are not part of any loop based
  /// regions and contain loop materialization candidates.
  void formRegionsForLoopMaterialization(Function &Func);

  /// Identifies regions in the incoming LLVM IR.
  void formRegions(Function &Func);

  /// Returns true if bblocks of the function are generable.
  bool areBBlocksGenerable(Function &Func) const;

  /// Returns true if we can form a function level region.
  bool canFormFunctionLevelRegion(Function &F);

  /// Creates a function level region out of all the function bblocks, except
  /// the entry bblock.
  void createFunctionLevelRegion(Function &F);

  /// Checks whether this loop basic block is loop concatenation candidate.
  static bool isLoopConcatenationCandidate(BasicBlock *BB);

  /// Checks whether the current function is loop concatenation candidate.
  bool isLoopConcatenationCandidate() const;

  void runImpl(Function &F);

  /// Returns either the inital or update operand of header phi corresponding to
  /// the passed in boolean argument.
  const Value *getHeaderPhiOperand(const PHINode *Phi, bool IsInit) const;

  /// Tries to trace though some instruction types to get to a GEPOperator.
  const GEPOperator *
  tracebackToGEPOp(const Value *Val,
                   SmallPtrSetImpl<const Value *> &VisitedInsts) const;

public:
  HIRRegionIdentification(Function &F, LoopInfo &LI, DominatorTree &DT,
                          PostDominatorTree &PDT, AssumptionCache &AC,
                          ScalarEvolution &SE, TargetLibraryInfo &TLI,
                          unsigned OptLevel);
  HIRRegionIdentification(const HIRRegionIdentification &) = delete;
  HIRRegionIdentification(HIRRegionIdentification &&RI);

  void print(raw_ostream &OS) const;

  ScopedScalarEvolution &getScopedSE() { return *ScopedSE; }

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

  /// Returns true if this type is supported.
  static bool isSupported(Type *Ty, bool IsGEPRelated,
                          const Instruction *Inst = nullptr,
                          const Loop *Lp = nullptr,
                          bool IsLastIndexedType = false);

  /// Returns true if \p GEPOp contains a type not supported by HIR.
  static bool containsUnsupportedTy(const GEPOrSubsOperator *GEPOp,
                                    const Loop *Lp = nullptr);

  // NOTE: Following functions were moved here so they can be shared between
  // HIRParser and SSADeconstruction. Is there a better way?

  /// Returns true if Phi occurs in the header of a loop.
  bool isHeaderPhi(const PHINode *Phi) const;

  /// Returns the header phi operand which corresponds to the initial value of
  /// phi (value coming from outside the loop).
  const Value *getHeaderPhiInitVal(const PHINode *Phi) const {
    return getHeaderPhiOperand(Phi, true);
  }

  /// Returns the header phi operand which corresponds to phi update (value
  /// coming from loop's backedge).
  const Value *getHeaderPhiUpdateVal(const PHINode *Phi) const {
    return getHeaderPhiOperand(Phi, false);
  }

  /// Returns true if \p BB can be reached from any of the \p FromBBs before
  /// hitting any \p EndBBs and without going through any backedges.
  bool
  isReachableFrom(const BasicBlock *BB,
                  const SmallPtrSetImpl<const BasicBlock *> &EndBBs,
                  const SmallPtrSetImpl<const BasicBlock *> &FromBBs) const;

  /// Erases all the formed regions.
  /// NOTE: Only used by HIRSSADeconstruction pass in opt-bisect mode.
  void discardRegions() { IRRegions.clear(); }

  /// Tries to find the element type of an AddRec phi by tracing it back to a
  /// GEPOperator. Returns null if element type cannot be found.
  Type *findPhiElementType(const PHINode *AddRecPhi) const;

  /// Traces back \p Inst's operands to a phi in \p Lp's header block which
  /// would act as loop's IV. Returns null if no such instruction is found.
  const PHINode *
  findIVDefInHeader(const Loop &Lp, const Instruction *Inst,
                    SmallPtrSetImpl<const Instruction *> &VisitedInsts) const;
};

class HIRRegionIdentificationAnalysis
    : public AnalysisInfoMixin<HIRRegionIdentificationAnalysis> {
  friend struct AnalysisInfoMixin<HIRRegionIdentificationAnalysis>;

  static AnalysisKey Key;

public:
  using Result = HIRRegionIdentification;

  HIRRegionIdentification run(Function &F, FunctionAnalysisManager &AM);
};

class HIRRegionIdentificationPrinterPass
    : public PassInfoMixin<HIRRegionIdentificationPrinterPass> {
  raw_ostream &OS;

public:
  explicit HIRRegionIdentificationPrinterPass(raw_ostream &OS) : OS(OS) {}

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    AM.getResult<HIRRegionIdentificationAnalysis>(F).print(OS);
    return PreservedAnalyses::all();
  }
};

class HIRRegionIdentificationWrapperPass : public FunctionPass {
  std::unique_ptr<HIRRegionIdentification> RI;

public:
  static char ID;

  HIRRegionIdentificationWrapperPass();

  HIRRegionIdentification &getRI() { return *RI; }
  const HIRRegionIdentification &getRI() const { return *RI; }

  bool runOnFunction(Function &F) override;

  void releaseMemory() override { RI.reset(); }

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  void print(raw_ostream &OS, const Module * = nullptr) const override {
    RI->print(OS);
  }
};

} // End namespace loopopt

} // End namespace llvm

#endif

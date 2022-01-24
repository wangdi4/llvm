//===- WorkItemAnalysis.h - Work item dependency analysis -------*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_WORKITEM_ANALYSIS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_WORKITEM_ANALYSIS_H

#include "llvm/IR/PassManager.h"
#include <map>
#include <queue>

namespace llvm {

class AllocaInst;
class BinaryOperator;
class BranchInst;
class CallInst;
class CastInst;
class GetElementPtrInst;
class SelectInst;
class UnaryOperator;
class VAArgInst;

class DominatorTree;
class LoopInfo;
class PostDominatorTree;
class RuntimeService;
class SoaAllocaInfo;

/// WorkItemInfo provides work item dependency info.
class WorkItemInfo {
public:
  explicit WorkItemInfo(const Function &F, RuntimeService *RTService,
                        DominatorTree *DT, PostDominatorTree *PDT, LoopInfo *LI,
                        SoaAllocaInfo *SA)
      : F(F), DT(DT), PDT(PDT), LI(LI), SA(SA), RTService(RTService),
        FullJoin(nullptr) {}

  using SchdConstMap = std::map<BasicBlock *, std::vector<BasicBlock *>>;

  /// Type of dependency on work item.
  enum Dependency {
    UNIFORM = 0,         /// All elements in vector are constant.
    CONSECUTIVE = 1,     /// Elements are consecutive.
    PTR_CONSECUTIVE = 2, /// Elements are pointers which are consecutive.
    STRIDED = 3,         /// Elements are in strides.
    RANDOM = 4,          /// Unknown or non consecutive order.
    NumDeps = 5,         /// Overall amount of dependencies.
  };

  /// Returns a map with scheduling constraints.
  SchdConstMap &getSchedulingConstraints() { return SchedulingConstraints; }

  /// Returns true if \p BB is a divergent block.
  bool isDivergentBlock(BasicBlock *BB) { return DivBlocks.contains(BB); }

  /// Returns true if the Phi's in \p BB are divergent.
  bool isDivergentPhiBlocks(BasicBlock *BB) {
    return DivPhiBlocks.contains(BB);
  }

  /// Inform analysis that \p V was invalidated as pointer many later be reused.
  void invalidateDepend(const Value *V) { Deps.erase(V); }

  /// Set the type of dependency an instruction should have and update the data
  /// structures accordingly.
  /// \param From source instruction.
  /// \param To targetinstruction.
  void setDepend(const Instruction *From, const Instruction *To);

  /// Returns the type of dependency the instruction has on the work item.
  Dependency whichDepend(const Value *V);

  void print(raw_ostream &OS) const;

  /// Compute dependency along dimension \p Dim.
  void compute(unsigned Dim);

private:
  /// Calculate dependency of \p V.
  void calculateDep(const Value *V);

  /// Dependency calculation functions.
  /// \param I instruction to inspect.
  /// \return depedency type.
  ///@{
  Dependency calculateDep(const AllocaInst *I);
  Dependency calculateDep(const BinaryOperator *I);
  Dependency calculateDep(const CallInst *I);
  Dependency calculateDep(const CastInst *I);
  Dependency calculateDep(const GetElementPtrInst *I);
  Dependency calculateDep(const PHINode *I);
  Dependency calculateDep(const SelectInst *I);
  Dependency calculateDep(const UnaryOperator *I);
  Dependency calculateDep(const VAArgInst *I);
  Dependency calculateDepTerminator(const Instruction *I);
  ///@}

  /// Trivally check work-item dependency.
  /// \param I instruction to check.
  /// \return dependency type. Returns Uniform if all operands are Uniform,
  /// Random otherwise.
  Dependency calculateDepSimple(const Instruction *I);

  /// Calculates the influence region, divergent loops, divergent blocks and
  /// partial joins for the branch instruction. PDT is given for post dominance
  /// info.
  /// \param I divergent branch.
  void calcInfoForBranch(const Instruction *I);

  /// Look for partial joins reachable from two different successors s.t. the
  /// path from each successor accesses the partial join from a predecessor.
  void findDivergePartialJoins(const Instruction *I);

  /// Provide known dependency type for requested value.
  /// \param V value to examine.
  /// \return Dependency type. Returns Uniform for unknown type.
  Dependency getDependency(const Value *V);

  /// Return true if there is calculated dependency type for requested value.
  /// \param V value to examine.
  bool hasDependency(const Value *V);

  /// Mark all the Phi nodes in full/partial joins as random.
  void markDependentPhiRandom();

  /// Update the work-item dependency from a divergent branch.
  /// Affected instructions are added to ChangedNew.
  /// \param I the divergent branch.
  /// \param Dep the dependency.
  void updateDepMap(const Instruction *I, Dependency Dep);

  /// Update dependency relations between all values.
  void updateDeps();

  /// The main function to handle control flow divergence.
  /// \param I the divergent branch.
  void updateCfDependency(const Instruction *I);

  /// The function that the WorkItemInfo is for.
  const Function &F;

  /// Analyses needed by control flow divergence propagation.
  DominatorTree *DT;
  PostDominatorTree *PDT;
  LoopInfo *LI;
  SoaAllocaInfo *SA;

  RuntimeService *RTService;

  /// The dimension over which we vectorize (usually 0).
  unsigned VectorizeDim;

  /// Iteratively one set holds the changed from the previous iteration and the
  /// other holds the new changed values from the current iteration.
  SetVector<const Value *> Changed[2];
  // Ptr to current changed set.
  SetVector<const Value *> *ChangedNew;

  /// Stores an updated list of all dependencies.
  DenseMap<const Value *, Dependency> Deps;

  /// Fields for the control flow divergence propagation.

  /// Block info - these are general for the kernel and not branch specific.

  /// Stores the divergent blocks - ones that have an input mask
  DenseSet<const BasicBlock *> DivBlocks;

  /// Stores the divergent phi blocks - ones that has divergent output due to
  /// the control flow.
  DenseSet<const BasicBlock *> DivPhiBlocks;

  /// Holds the divergent branches waiting to propagate their divergency.
  std::queue<const BranchInst *> DivBranchesQueue;

  /// Branch specific info.

  /// Immediate post-dominator.
  /// In case where the immediate post-dominator of a branch is inside a loop,
  /// the latch node is not an exiting node, and the latch node is in the
  /// influence region.
  /// Then FullJoin is moved to be the first post-dominator outside the loop.
  BasicBlock *FullJoin;

  /// Influence region - blocks that exist in a path from cbr to FullJoin.
  SetVector<BasicBlock *> InfluenceRegion;

  /// Blocks in InfluenceRegion that are reachable from cbr by two different
  /// successors.
  SetVector<BasicBlock *> PartialJoins;

  /// Blocks in InfluenceRegion that are reachable from cbr by its two
  /// successors and there exists a path from each successors that accesses the
  /// block from a different predecessor.
  SetVector<BasicBlock *> DivergePartialJoins;

  /// A map that maps a block terminating with a divergent branch to a vector
  /// containing the divergent branch basic block together with its influence
  /// region and its immediate post-dominator. Later on we use it to add
  /// scheduling constraints for the linearizer.
  SchdConstMap SchedulingConstraints;
};

/// Analysis pass which detects values that depend on work-item and describes
/// their dependency.
/// The algorithm used is recursive and new instructions are updated according
/// to their operands (which are already calculated).
class WorkItemAnalysis : public AnalysisInfoMixin<WorkItemAnalysis> {
  friend AnalysisInfoMixin<WorkItemAnalysis>;
  static AnalysisKey Key;

public:
  using Result = WorkItemInfo;

  Result run(Function &F, FunctionAnalysisManager &FAM);
};

/// Printer pass for WorkItemAnalysis.
class WorkItemAnalysisPrinter : public PassInfoMixin<WorkItemAnalysisPrinter> {
  raw_ostream &OS;

public:
  explicit WorkItemAnalysisPrinter(raw_ostream &OS) : OS(OS) {}

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    AM.getResult<WorkItemAnalysis>(F).print(OS);
    return PreservedAnalyses::all();
  }
};

/// For legacy pass manager.
class WorkItemAnalysisLegacy : public FunctionPass {
  std::unique_ptr<WorkItemInfo> WIInfo;
  unsigned VectorizeDim;

public:
  static char ID;

  WorkItemAnalysisLegacy(unsigned VectorizeDim = 0);

  bool runOnFunction(Function &F) override;

  StringRef getPassName() const override { return "WorkItemAnalysisLegacy"; }

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  void print(raw_ostream &OS, const Module *) const override {
    WIInfo->print(OS);
  }

  WorkItemInfo &getResult() { return *WIInfo; }
  const WorkItemInfo &getResult() const { return *WIInfo; }
};

} // namespace llvm
#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_WORKITEM_ANALYSIS_H

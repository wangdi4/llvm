//===--- HIRDeadStoreElimination.h ---------------------------*- C++ -*---===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRDSEIMPL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRDSEIMPL_H

#include "llvm/Transforms/Intel_LoopTransforms/HIRDeadStoreEliminationPass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGrouping.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/InitializePasses.h"

namespace llvm {

class Value;
class DominatorTree;
class FieldModRefResult;

namespace loopopt {

typedef DDRefGrouping::RefGroupTy<const RegDDRef *> RefGroupTy;
typedef DDRefGrouping::RefGroupVecTy<const RegDDRef *> RefGroupVecTy;
typedef SmallVector<const RegDDRef *, 4> RefVecTy;

namespace dse {

class HIRDeadStoreEliminationLegacyPass : public HIRTransformPass {
public:
  static char ID;
  HIRDeadStoreEliminationLegacyPass() : HIRTransformPass(ID) {
    initializeHIRDeadStoreEliminationLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.setPreservesAll();
  }
};

class HIRDeadStoreElimination {
  HIRDDAnalysis &HDDA;
  HIRLoopStatistics &HLS;
  HLNodeUtils &HNU;

  // Region-level memref groups and symbases:
  HIRLoopLocality::RefGroupVecTy EqualityGroups;
  SmallSet<unsigned, 8> UniqueGroupSymbases;

  // Region-level AddressOf Refs:
  SmallVector<RegDDRef *, 4> AddressOfRefVec;

  // Map of base ptr value to a boolean indicating whether all uses of the
  // base are in single region.
  DenseMap<Value *, bool> AllUsesInSingleRegion;

  bool isValidParentChain(const HLNode *PostDomNode, const HLNode *PrevNode,
                          const RegDDRef *PostDomRef);

  void releaseMemory(void) {
    EqualityGroups.clear();
    UniqueGroupSymbases.clear();
    AddressOfRefVec.clear();
  }

  // Collects memrefs and addressOf refs in the region. Returns false if no
  // memrefs were found.
  bool doCollection(HLRegion &Region);

  /// Returns true if \p Ref escapes via an AddressOf ref and cannot be proven
  /// to be safe for analysis.
  bool basePtrEscapesAnalysis(const RegDDRef *Ref) const;

  /// Returns true if all uses of the base ptr of \p Ref are within \p Region.
  bool hasAllUsesWithinRegion(HLRegion &Region, const RegDDRef *Ref);

public:
  HIRDeadStoreElimination(HIRFramework &HIRF, HIRDDAnalysis &HDDA,
                          HIRLoopStatistics &HLS)
      : HDDA(HDDA), HLS(HLS), HNU(HIRF.getHLNodeUtils()) {}

  bool run(HLRegion &Region);
};

// AddressOf Ref Collector:
// - collect all AddressOf Ref(s) into AddressOfRefVec
class AddressOfRefCollector final : public HLNodeVisitorBase {
private:
  SmallVectorImpl<RegDDRef *> &AddressOfRefVec;

public:
  AddressOfRefCollector(SmallVectorImpl<RegDDRef *> &AddressOfRefVec)
      : AddressOfRefVec(AddressOfRefVec) {}

  void visit(HLDDNode *Node) {
    for (auto *Ref : make_range(Node->op_ddref_begin(), Node->op_ddref_end())) {
      if (Ref->isAddressOf()) {
        AddressOfRefVec.push_back(Ref);
      }
    }
  }

  void visit(HLGoto *Goto){};
  void visit(HLLabel *Label){};
  void visit(HLNode *Node) {
    llvm_unreachable(" visit(HLNode *) - Node not supported\n");
  }
  void postVisit(const HLNode *Node) {}
};

class UnsafeCallVisitor final : public HLNodeVisitorBase {
  HIRLoopStatistics &HLS;
  const HLNode *StartNode;
  const HLNode *EndNode;
  bool FoundStartNode;
  bool FoundEndNode;
  bool FoundUnsafeCall;

public:
  UnsafeCallVisitor(HIRLoopStatistics &HLS, const HLNode *StartNode,
                    const HLNode *EndNode)
      : HLS(HLS), StartNode(StartNode), EndNode(EndNode), FoundStartNode(false),
        FoundEndNode(false), FoundUnsafeCall(false) {
    assert((isa<HLLoop>(StartNode) || isa<HLInst>(StartNode)) &&
           "Invalid start node!");
    assert((isa<HLLoop>(EndNode) || isa<HLInst>(EndNode)) &&
           "Invalid end node!");
  }

  bool isNodeRelevant(const HLNode *Node) {
    if (Node == StartNode) {
      FoundStartNode = true;
    } else if (Node == EndNode) {
      FoundEndNode = true;
    }

    return FoundStartNode;
  }

  void visit(const HLInst *Inst) {
    if (!isNodeRelevant(Inst)) {
      return;
    }

    // TODO: check mayThrow() as well
    FoundUnsafeCall = Inst->isUnknownAliasingCallInst();
  }

  void visit(const HLLoop *Loop) {
    if (!isNodeRelevant(Loop)) {
      return;
    }

    FoundUnsafeCall =
        HLS.getTotalLoopStatistics(Loop).hasCallsWithUnknownAliasing();
  }

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}

  bool foundUnsafeCall() const { return FoundUnsafeCall; }
  bool isDone() const { return FoundEndNode || FoundUnsafeCall; }
};

} // namespace dse
} // namespace loopopt
} // namespace llvm

#endif

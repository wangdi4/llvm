//===------ ParVecDirectiveInsertion.h -------------------*-- C++ --*------===//
//                         Implements ParVecDirectiveInsertion class
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
// This file implements ParVecDirectiveInsertion class.
// This is the common parent class of ParDirInsert/VecDirInsert passes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_PARVECDIRECTIVEINSERTION_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_PARVECDIRECTIVEINSERTION_H

#include "llvm/Analysis/Intel_Directives.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRParVecAnalysis.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

namespace llvm {
namespace loopopt {

/// \brief Abstract parent class for ParDirectiveInsertion/VecDirectiInsertion.
class ParVecDirectiveInsertion : public HIRTransformPass {
  ParVecInfo::AnalysisMode Mode;

  /// \brief Worker class for directive insertion.
  class Visitor final : public HLNodeVisitorBase {
    Function &Func;
    HIRFramework *HIRF;
    HIRParVecAnalysis *PVA;
    ParVecInfo::AnalysisMode Mode;
    /// \brief Status flag to indicate whether we modified the HIR or not.
    bool Inserted;
    HLNode *SkipNode;

    /// \brief Insert auto-vec directives to the loop.
    void insertVecDirectives(HLLoop *L, const ParVecInfo *Info);
    /// \brief Insert auto-par directives to the loop.
    void insertParDirectives(HLLoop *L, const ParVecInfo *Info);
    /// \brief Returns metadata RegDDRef for the OpenMP directive.
    RegDDRef *createRegDDRef(OMP_DIRECTIVES Dir);
    /// \brief Returns metadata RegDDRef for the OpenMP clause.
    RegDDRef *createRegDDRef(OMP_CLAUSES Qual);
    /// \brief Prepend/Append a directive call to the Loop.
    HLInst *insertDirective(HLLoop *L, OMP_DIRECTIVES Dir, bool Append);

  public:
    Visitor(Function &Func, HIRFramework *HIRF, HIRParVecAnalysis *PVA,
            ParVecInfo::AnalysisMode Mode)
        : Func(Func), HIRF(HIRF), PVA(PVA), Mode(Mode), Inserted(false) {}
    void visit(HLNode *N) {}
    void postVisit(HLNode *N) {}
    /// \brief Checks if directive insertion is needed for the loop
    /// and invokes insertDirective() function.
    void visit(HLLoop *L);
    /// \brief Returns true if directive is inserted for at least one loop.
    bool getInserted() { return Inserted; }

    bool skipRecursion(const HLNode *Node) const override {
      return Node == SkipNode;
    }
  };

public:
  ParVecDirectiveInsertion(char &ID, ParVecInfo::AnalysisMode Mode)
      : HIRTransformPass(ID), Mode(Mode) {
    // Be sure to take the ID parameter as reference, not by value.
    // Otherwise, it will be hard to diagnose bugs.
  }

  /// \brief Analyze auto-parallelizability/auto-vectorizability of the loops
  /// in the function and insert directives for auto-parallelization/
  /// auto-vectorization.
  bool runOnFunction(Function &Func) override;

  void releaseMemory() override {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFramework>();
    AU.addRequiredTransitive<HIRParVecAnalysis>();
    AU.setPreservesAll();
  }
};

} // vpo namespace
} // llvm namespace

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_PARVECDIRECTIVEINSERTION_H

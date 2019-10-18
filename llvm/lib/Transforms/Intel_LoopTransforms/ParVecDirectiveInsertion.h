//===------ ParVecDirectiveInsertion.h -------------------*-- C++ --*------===//
//                         Implements ParVecDirectiveInsertion class
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Directives.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRParVecAnalysis.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

namespace llvm {
namespace loopopt {

/// \brief Abstract parent class for ParDirectiveInsertion/VecDirectiInsertion.
class ParVecDirectiveInsertion {
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
    /// \brief Prepend region entry directive call to the Loop.
    HLInst *insertBeginRegion(HLLoop *Lp, OMP_DIRECTIVES Dir);
    /// \brief Append a region exit directive call to the Loop.
    HLInst *insertEndRegion(HLLoop *Lp, OMP_DIRECTIVES Dir, HLInst *Entry);

  public:
    Visitor(Function &Func, HIRFramework *HIRF, HIRParVecAnalysis *PVA,
            ParVecInfo::AnalysisMode Mode)
        : Func(Func), HIRF(HIRF), PVA(PVA), Mode(Mode), Inserted(false), 
          SkipNode(nullptr) {}
    void visit(HLNode *N) {}
    void postVisit(HLNode *N) {}
    /// \brief Checks if directive insertion is needed for the loop
    /// and invokes insertDirective() function.
    void visit(HLLoop *L);
    /// \brief Returns true if directive is inserted for at least one loop.
    bool getInserted() { return Inserted; }

    bool skipRecursion(const HLNode *Node) const {
      return Node == SkipNode;
    }
  };

public:
  ParVecDirectiveInsertion(ParVecInfo::AnalysisMode Mode) : Mode(Mode) {
    // Be sure to take the ID parameter as reference, not by value.
    // Otherwise, it will be hard to diagnose bugs.
  }

  /// \brief Analyze auto-parallelizability/auto-vectorizability of the loops
  /// in the function and insert directives for auto-parallelization/
  /// auto-vectorization.
  bool runOnFunction(Function &Func, HIRFramework *HIRF, HIRParVecAnalysis *HPVA);
};

} // vpo namespace
} // llvm namespace

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_PARVECDIRECTIVEINSERTION_H

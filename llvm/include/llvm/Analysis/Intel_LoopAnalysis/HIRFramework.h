//===--- HIRFramework.h - public interface of HIR framework ---*-- C++ --*-===//
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
// This analysis is the public interface for the HIR framework. HIR
// transformations should add it as a dependency.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRFRAMEWORK_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRFRAMEWORK_H

#include "llvm/Pass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"
#include "llvm/Analysis/Intel_LoopAnalysis/SymbaseAssignment.h"
// Required for making INVALID_SYMBASE and CONSTANT_SYMBASE available.
#include "llvm/Analysis/Intel_LoopAnalysis/ScalarSymbaseAssignment.h"

namespace llvm {

namespace loopopt {

/// \brief This analysis is the public interface for the HIR framework.
///
/// The overall sequence of building the HIR is as follows-
///
/// 1) RegionIdentification - identifies regions in IR.
/// 2) SCCFormation - identifies SCCs in regions.
/// 3) SSADeconstruction - deconstructs SSA for HIR by inserting copies.
/// 4) HIRCreation - populates HIR regions with a sequence of HLNodes (without
///    HIR loops).
/// 5) HIRCleanup - removes redundant gotos/labels from HIR.
/// 6) LoopFormation - Forms HIR loops within HIR regions.
/// 7) ScalarSymbaseAssignment - Assigns symbases to livein/liveout values.
/// 8) HIRParser - Creates DDRefs and parses SCEVs into CanonExprs. Also assigns
///    symbases to non livein/liveout scalars using ScalarSymbaseAssignment's
///    interface.
/// 9) SymbaseAssignment - Assigns symbases to memory DDRefs.
///
class HIRFramework : public FunctionPass {
public:
  /// Iterators to iterate over regions
  typedef HLContainerTy::iterator iterator;
  typedef HLContainerTy::const_iterator const_iterator;
  typedef HLContainerTy::reverse_iterator reverse_iterator;
  typedef HLContainerTy::const_reverse_iterator const_reverse_iterator;

private:
  // Expose insertHIRLval()/getNewSymbase().
  friend class CanonExprUtils;
  friend class DDRefUtils;

  /// HIRP - parser for the function.
  HIRParser *HIRP;

  /// SA - symbase assignment for the function.
  SymbaseAssignment *SA;


  /// \brief Registers new lval/symbase pairs created by HIR transformations.
  /// Only used for printing.
  void insertHIRLval(const Value *Lval, unsigned Symbase) {
    HIRP->insertHIRLval(Lval, Symbase);
  }

  unsigned getNewSymbase() { return SA->getNewSymbase(); }

public:
  static char ID; // Pass identification
  HIRFramework();

  bool runOnFunction(Function &F) override;
  void releaseMemory() override {}
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;
  void verifyAnalysis() const override;

  /// Region iterator methods
  HIRCreation::iterator hir_begin() { return HIRP->hir_begin(); }
  HIRCreation::const_iterator hir_cbegin() const { return HIRP->hir_cbegin(); }
  HIRCreation::iterator hir_end() { return HIRP->hir_end(); }
  HIRCreation::const_iterator hir_cend() const { return HIRP->hir_cend(); }

  HIRCreation::reverse_iterator hir_rbegin() { return HIRP->hir_rbegin(); }
  HIRCreation::const_reverse_iterator hir_crbegin() const {
    return HIRP->hir_crbegin();
  }
  HIRCreation::reverse_iterator hir_rend() { return HIRP->hir_rend(); }
  HIRCreation::const_reverse_iterator hir_crend() const {
    return HIRP->hir_crend();
  }

  /// \brief Returns Function object.
  Function &getFunction() const { return HIRP->getFunction(); }
  
  /// \brief Returns Module object.
  Module &getModule() const { return HIRP->getModule(); }

  /// \brief Returns LLVMContext object.
  LLVMContext &getContext() const { return HIRP->getContext(); }

  /// \brief Returns DataLayout object.
  const DataLayout &getDataLayout() const { return HIRP->getDataLayout(); }
};

} // End namespace loopopt

} // End namespace llvm

#endif

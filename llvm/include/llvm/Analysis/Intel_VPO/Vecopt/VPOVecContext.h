//===------ VPOVecContext.h -------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This header file declares vectorizer classes to hold the vectorization
/// context.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALISYS_VPO_VPOVECCONTEXT_H
#define LLVM_ANALISYS_VPO_VPOVECCONTEXT_H

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"

namespace llvm {
namespace vpo {

/// Holds information about the current context underwhich vectorization
/// is considered, including the specific loop that is being considered, and
/// the Vectorization Factor.
/// Currently  the VectorContext is serving multiple clients:
/// 1) IR CodeGen out of AVR: VectorContext indicates which Loops in
/// the region to vectorize, and using which Vectorization Factor (in essence,
/// make up for what is not coded explicitly in AVR).
/// 2) VPO Cost Model: Same. (Additional data-structures, such as VLS grouping
/// information and memory access information, are passed separately).
/// 3) VLS: In addition to the VF and loop, the VectorContext also holds
/// the data-dependence graph for the loop, in order to be able to answer
/// quesionts such as "can memref1 be moved to the location of memref2".
// TODO: Revisit what information the VectorContext should hold.
// FIXME?: Hold the AVRLoop in VPOVecContextBase instead of the loop construct
// of the underlying IR in the derived classes?
// FIXME?: Change name to VPOVecScenario?
class VPOVecContextBase {
public:
  VPOVecContextBase(unsigned VF) : VectFactor(VF) {}
  VPOVecContextBase() {}

  /// \name Functions to get/set the vectorization factor.
  /// @{
  unsigned getVectFactor() const { return VectFactor; }
  void setCurrentVF(unsigned VF) { VectFactor = VF; }
  /// @}

private:
  /// \brief Vectorization Factor used in the current scenario.
  unsigned VectFactor;
};

/// LLVMIR VectorContext (just an empty implementation so far).
// TODO.
class VPOVecContext : public VPOVecContextBase {
public:
  VPOVecContext(unsigned VF) : VPOVecContextBase(VF) {}
  VPOVecContext() {}
};

/// HIR VectorContext
class VPOVecContextHIR : public VPOVecContextBase {
public:
  VPOVecContextHIR(unsigned VF, loopopt::DDGraph DDG, const loopopt::HLLoop *L)
      : VPOVecContextBase(VF), DDG(DDG), Loop(L) {}
  VPOVecContextHIR(loopopt::DDGraph DDG, const loopopt::HLLoop *L)
      : DDG(DDG), Loop(L) {}
  VPOVecContextHIR(unsigned VF)
      : VPOVecContextBase(VF), DDG(nullptr, nullptr), Loop(nullptr) {}
  VPOVecContextHIR() : DDG(nullptr, nullptr), Loop(nullptr) {}

  /// \name Functions to get the DDG, Loop and Loop level.
  /// @{
  loopopt::DDGraph getDDG() const { return DDG; }
  const loopopt::HLLoop *getLoop() const { return Loop; }
  unsigned getLoopLevel() const { return Loop->getNestingLevel(); }
  /// @}

private:
  /// \brief Data-Dependence and def-use information. Currently used to check
  /// the legality of code-movement during VLS-grouping analysis.
  loopopt::DDGraph DDG;

  /// \brief The HIR Loop that is currently considered for vectorization.
  const loopopt::HLLoop *Loop;
};

} // End namespace vpo
} // End namespace llvm

#endif // LLVM_ANALYSIS_VPO_VPOVECCONTEXT_H

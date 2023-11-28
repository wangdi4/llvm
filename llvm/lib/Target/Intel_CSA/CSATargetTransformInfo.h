//===-- CSATargetTransformInfo.h - CSA specific TTI -------------*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file a TargetTransformInfo::Concept conforming object specific to the
/// CSA target machine. It uses the target's detailed information to
/// provide more precise answers to certain TTI queries, while letting the
/// target independent and default TTI implementations handle the rest.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSATARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_CSA_CSATARGETTRANSFORMINFO_H

#include "CSA.h"
#include "CSATargetMachine.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/BasicTTIImpl.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

class CSATTIImpl : public BasicTTIImplBase<CSATTIImpl> {
  typedef BasicTTIImplBase<CSATTIImpl> BaseT;
  typedef TargetTransformInfo TTI;
  friend BaseT;

  const CSASubtarget *ST;
  const CSATargetLowering *TLI;

  const CSASubtarget *getST() const { return ST; }
  const CSATargetLowering *getTLI() const { return TLI; }

public:
  explicit CSATTIImpl(const CSATargetMachine *TM, const Function &F)
      : BaseT(TM, F.getParent()->getDataLayout()), ST(TM->getSubtargetImpl(F)),
        TLI(ST->getTargetLowering()) {}

  /// \name Scalar TTI Implementations
  /// @{

  bool areInlineCompatible(const Function *Caller,
                           const Function *Callee) const;

  bool needsStructuredCFG() const { return true; }
  /// @}

  /// \name Vector TTI Implementations
  /// @{

  unsigned getNumberOfRegisters(bool Vector) const;
  unsigned getRegisterBitWidth(bool Vector) const;
  unsigned getMinVectorRegisterBitWidth() const { return 64; }
  int getShuffleCost(TTI::ShuffleKind Kind, VectorType *Tp, int Index,
                     VectorType *SubTp);
  /// @}
};

} // end namespace llvm

#endif

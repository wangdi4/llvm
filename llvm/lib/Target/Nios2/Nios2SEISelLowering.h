//===-- Nios2ISEISelLowering.h - Nios2ISE DAG Lowering Interface ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Subclass of Nios2ITargetLowering specialized for nios232/64.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_NIOS2_NIOS2SEISELLOWERING_H
#define LLVM_LIB_TARGET_NIOS2_NIOS2SEISELLOWERING_H

#include "Nios2ISelLowering.h"
#include "Nios2RegisterInfo.h"

namespace llvm {
  class Nios2SETargetLowering : public Nios2TargetLowering  {
  public:
    explicit Nios2SETargetLowering(const Nios2TargetMachine &TM,
                                  const Nios2Subtarget &STI);

    SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;
  private:
    bool isEligibleForTailCallOptimization(const Nios2CC &Nios2CCInfo,
                                     unsigned NextStackOffset,
                                     const Nios2FunctionInfo& FI) const override;
  };
}

#endif // Nios2ISEISELLOWERING_H

//===- IntelVPLoopAnalysis.cpp --------------------------------------------===//
//
//   Copyright (C) 2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// \file
/// This file provides VPLoop-based analysis. Right now VPLoopAnalysisBase can
/// only be used to compute min, known, estimated or max trip counts for a
/// VPLoopRegion.
//
//===----------------------------------------------------------------------===//

#include "IntelVPLoopAnalysis.h"
#include "IntelVPlan.h"

using namespace llvm;
using namespace llvm::vpo;

unsigned int VPInduction::getInductionOpcode() const {
  return getInductionBinOp() ? getInductionBinOp()->getOpcode() : BinOpcode;
}

void VPLoopEntities::addReduction(VPInstruction *Instr, VPValue *Incoming,
                                  VPInstruction *Exit, RecurrenceKind Kind, FastMathFlags FMF,
                                  MinMaxRecurrenceKind MKind, Type *RedTy,
                                  bool Signed, VPValue *AI) {

  assert(Instr && "null instruction");
  VPReduction *Red =
      new VPReduction(Incoming, Exit, Kind, FMF, MKind, RedTy, Signed);
  ReductionMap[Instr] = std::unique_ptr<VPReduction>(Red);
  createMemDescFor(Red, AI);
}
void VPLoopEntities::addIndexReduction(VPInstruction *Instr,
                                       VPReduction *Parent, VPValue *Incoming,
                                       VPInstruction *Exit, Type *RedTy,
                                       bool Signed, bool ForLast, VPValue *AI) {
  assert(Instr && "null instruction");
  assert(Parent && "null parent in index-reduction");
  VPIndexReduction *Red =
      new VPIndexReduction(Parent, Incoming, Exit, RedTy, Signed, ForLast);
  ReductionMap[Instr] = std::unique_ptr<VPIndexReduction>(Red);
  MinMaxIndexes[Parent] = Red;
  createMemDescFor(Red, AI);
}
void VPLoopEntities::addInduction(VPInstruction *Start, VPValue *Incoming,
                                  InductionKind Kind, VPValue *Step,
                                  VPInstruction *InductionBinOp,
                                  unsigned int Opc, VPValue *AI) {
  assert(Start && "null starting instruction");
  VPInduction *Ind = new VPInduction(Incoming, Kind, Step, InductionBinOp, Opc);
  InductionMap[Start] = std::unique_ptr<VPInduction>(Ind);
  createMemDescFor(Ind, AI);
}
void VPLoopEntities::addPrivate(VPInstruction *Assign, bool isConditional,
                                bool Explicit, VPValue *AI) {
  assert(Assign && "null assign");
  VPPrivate *Priv = new VPPrivate(Assign, isConditional, Explicit);
  PrivateMap[Assign] = std::unique_ptr<VPPrivate>(Priv);
  createMemDescFor(Priv, AI);
}

void VPLoopEntities::createMemDescFor(VPLoopEntity *E, VPValue *AI) {
  if (AI) {
    VPInMemoryEntity *Mem = new VPInMemoryEntity(AI);
    MemoryDescriptors[E] = std::unique_ptr<VPInMemoryEntity>(Mem);
  }
}

/// Returns identity corresponding to the RecurrenceKind.
VPValue *VPLoopEntities::getReductionIdentiy(const VPReduction *Red) const {
  switch (Red->getRecurrenceKind()) {
  case RecurrenceKind::RK_IntegerXor:
  case RecurrenceKind::RK_IntegerAdd:
  case RecurrenceKind::RK_IntegerOr:
  case RecurrenceKind::RK_IntegerMult:
  case RecurrenceKind::RK_IntegerAnd:
  case RecurrenceKind::RK_FloatMult:
  case RecurrenceKind::RK_FloatAdd: {
    Constant *C = VPReduction::getRecurrenceIdentity(Red->getRecurrenceKind(),
                                                     Red->getRecurrenceType());
    return Plan->getVPConstant(C);
  }
  case RecurrenceKind::RK_IntegerMinMax:
  case RecurrenceKind::RK_FloatMinMax:
    return Red->getRecurrenceStartValue();
  default:
    llvm_unreachable("Unknown recurrence kind");
  }
}


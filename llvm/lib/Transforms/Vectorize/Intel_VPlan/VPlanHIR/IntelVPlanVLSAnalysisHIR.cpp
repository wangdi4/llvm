//===- IntelVPlanVLSAnalysisHIR.cpp - --------------------------------------===/
//
//   Copyright (C) 2018 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements VPlanVLSAnalysisHIR.
///
//===----------------------------------------------------------------------===//
#include "IntelVPlanVLSAnalysisHIR.h"

namespace llvm {

namespace vpo {

VPlanVLSAnalysis::MemAccessTy
VPlanVLSAnalysisHIR::getAccessType(const RegDDRef *Ref, const unsigned Level,
                                   int64_t *Stride) {
  if (!Ref->isMemRef())
    return MemAccessTy::Unknown;

  if (!Ref->getConstStrideAtLevel(Level, Stride) || !*Stride)
    return Ref->isStructurallyInvariantAtLevel(Level) ? MemAccessTy::Uniform
                                                      : MemAccessTy::Indexed;

  // Compute stride in terms of number of elements
  auto DL = Ref->getDDRefUtils().getDataLayout();
  auto RefSizeInBytes = DL.getTypeSizeInBits(Ref->getDestType()) >> 3;
  *Stride /= RefSizeInBytes;
  return MemAccessTy::Strided;
}

// TODO: Replace this function with a call to divergence analysis when it is
// ready.
bool VPlanVLSAnalysisHIR::isUnitStride(const RegDDRef *Ref, unsigned Level) {
  int64_t ConstStride;
  return getAccessType(Ref, Level, &ConstStride) == MemAccessTy::Strided &&
         ConstStride == 1;
}

OVLSMemref *VPlanVLSAnalysisHIR::createVLSMemref(const VPInstruction *Inst,
                                                 const MemAccessTy &AccTy,
                                                 const unsigned Level,
                                                 const unsigned VF) const {
  unsigned Size =
  // FIXME: Move getMemInstValueType into VPlanValueUitls.
      VPlanCostModel::getMemInstValueType(Inst)->getScalarSizeInBits();
  // TODO: Shouldn't see struct types here.
  if (!Size)
    return nullptr;

  OVLSType Ty = OVLSType(Size, VF);
  unsigned Opcode = Inst->getOpcode();

  if (AccTy == MemAccessTy::Strided && Opcode == Instruction::Load)
    return new VPVLSClientMemrefHIR(OVLSAccessType::getStridedLoadTy(), Ty,
                                    Inst, Level, DDA);
  if (AccTy == MemAccessTy::Strided && Opcode == Instruction::Store)
    return new VPVLSClientMemrefHIR(OVLSAccessType::getStridedStoreTy(), Ty,
                                    Inst, Level, DDA);
  if (AccTy == MemAccessTy::Indexed && Opcode == Instruction::Load)
    return new VPVLSClientMemrefHIR(OVLSAccessType::getIndexedLoadTy(), Ty,
                                    Inst, Level, DDA);
  if (AccTy == MemAccessTy::Indexed && Opcode == Instruction::Store)
    return new VPVLSClientMemrefHIR(OVLSAccessType::getIndexedStoreTy(), Ty,
                                    Inst, Level, DDA);
  return nullptr;
}

} // namespace vpo

} // namespace llvm

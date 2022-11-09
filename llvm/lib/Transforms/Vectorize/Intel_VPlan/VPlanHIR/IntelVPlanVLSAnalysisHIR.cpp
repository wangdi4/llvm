//===- IntelVPlanVLSAnalysisHIR.cpp - --------------------------------------===/
//
//   Copyright (C) 2018-2022 Intel Corporation. All rights reserved.
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
  return MemAccessTy::Strided;
}

OVLSMemref *
VPlanVLSAnalysisHIR::createVLSMemref(const VPLoadStoreInst *Inst,
                                     const unsigned VF,
                                     const VPlanScalarEvolution *VPSE) {

  const HLNode *Node = Inst->HIR().getUnderlyingNode();
  const HLDDNode *DDNode = cast_or_null<HLDDNode>(Node);
  if (!DDNode)
    return nullptr;

  int Level = TheLoop->getNestingLevel();
  const RegDDRef *Ref = Inst->getHIRMemoryRef();
  if (!Ref)
    return nullptr;

  unsigned Size = DL.getTypeAllocSizeInBits(Ref->getDestType());
  if (!Size)
    return nullptr;

  OVLSType Ty = OVLSType(Size, VF);

  int64_t Stride = 0;
  // Overrides incoming AT.
  // TODO: remove getAccessType() from this place.
  MemAccessTy AccTy = getAccessType(Ref, Level, &Stride);
  unsigned Opcode = Inst->getOpcode();

  if (AccTy == MemAccessTy::Strided && Opcode == Instruction::Load)
    return VPVLSClientMemrefHIR::create(OptVLSContext, OVLSAccessKind::SLoad,
                                        Ty, Inst, TheLoop, DDA, Ref);
  if (AccTy == MemAccessTy::Strided && Opcode == Instruction::Store)
    return VPVLSClientMemrefHIR::create(OptVLSContext, OVLSAccessKind::SStore,
                                        Ty, Inst, TheLoop, DDA, Ref);
  if (AccTy == MemAccessTy::Indexed && Opcode == Instruction::Load)
    return VPVLSClientMemrefHIR::create(OptVLSContext, OVLSAccessKind::ILoad,
                                        Ty, Inst, TheLoop, DDA, Ref);
  if (AccTy == MemAccessTy::Indexed && Opcode == Instruction::Store)
    return VPVLSClientMemrefHIR::create(OptVLSContext, OVLSAccessKind::IStore,
                                        Ty, Inst, TheLoop, DDA, Ref);
  return nullptr;
}

} // namespace vpo

} // namespace llvm

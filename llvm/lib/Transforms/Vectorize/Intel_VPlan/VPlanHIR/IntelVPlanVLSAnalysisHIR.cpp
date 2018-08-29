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
                                                 const MemAccessTy &AT,
                                                 const unsigned Level,
                                                 const unsigned VF) const {
  unsigned Size = 0;
  if (Inst->getOpcode() == Instruction::Load ||
      Inst->getOpcode() == Instruction::Store)
    Size = VPlanCostModel::getMemInstValueType(Inst)->getScalarSizeInBits();
  else if (Inst->getType())
    Size = Inst->getType()->getScalarSizeInBits();

  // TODO: Shouldn't see struct types here.
  if (!Size)
    return nullptr;

  OVLSType Ty = OVLSType(Size, VF);
  unsigned Opcode = Inst->getOpcode();

  const HLDDNode *DDNode = cast<HLDDNode>(Inst->HIR.getUnderlyingNode());
  assert(DDNode && "HLDDNode is expected.");
  const RegDDRef *Ref = nullptr;

  // FIXME: This code is not needed with proper decomposition and DA.
  // Assume we have original HLInst
  //  a[i] = b[i] + (c[i] - d[i]);
  // Currently, there's no way to understand if incoming VPInstruction was
  // constructed for d[i] or for c[i], due to absence link to original RegDDRef.
  // In the code we're looking for first unvisited RegDDRef and construct
  // VPVLSClientMemrefHIR for this Ref. It's definitely not accurate, but still
  // better than nothing.
  for (auto I = DDNode->op_ddref_begin(), E = DDNode->op_ddref_end(); I != E;
       ++I) {
    Ref = *I;
    auto DDNodeIt = DDNodeRefs.find(DDNode);
    if (DDNodeIt == DDNodeRefs.end())
      DDNodeIt = DDNodeRefs.insert({DDNode, {}}).first;

    if (DDNodeIt != DDNodeRefs.end()) {
      auto RefIt = DDNodeIt->second.find(Ref);
      if (Ref->isMemRef() && !Ref->isStructurallyInvariantAtLevel(Level) &&
          RefIt == DDNodeIt->second.end()) {
        DDNodeIt->second.insert(Ref);
        break;
      }
    }
  }

  assert(Ref && "Unvisited RegDDRef must exist.");

  int64_t Stride = 0;
  // Overrides incoming AT.
  // TODO: remove getAccessType() from this place.
  MemAccessTy AccTy = getAccessType(Ref, Level, &Stride);
  Opcode = Ref->isRval() ? Instruction::Load : Instruction::Store;

  if (AccTy == MemAccessTy::Strided && Opcode == Instruction::Load)
    return new VPVLSClientMemrefHIR(OVLSAccessType::getStridedLoadTy(), Ty,
                                    Inst, Level, DDA, Ref);
  if (AccTy == MemAccessTy::Strided && Opcode == Instruction::Store)
    return new VPVLSClientMemrefHIR(OVLSAccessType::getStridedStoreTy(), Ty,
                                    Inst, Level, DDA, Ref);
  if (AccTy == MemAccessTy::Indexed && Opcode == Instruction::Load)
    return new VPVLSClientMemrefHIR(OVLSAccessType::getIndexedLoadTy(), Ty,
                                    Inst, Level, DDA, Ref);
  if (AccTy == MemAccessTy::Indexed && Opcode == Instruction::Store)
    return new VPVLSClientMemrefHIR(OVLSAccessType::getIndexedStoreTy(), Ty,
                                    Inst, Level, DDA, Ref);
  return nullptr;
}

} // namespace vpo

} // namespace llvm

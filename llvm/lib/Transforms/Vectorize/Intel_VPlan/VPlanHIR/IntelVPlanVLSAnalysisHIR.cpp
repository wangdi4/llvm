//===- IntelVPlanVLSAnalysisHIR.cpp - --------------------------------------===/
//
//   Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
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

// TODO: Replace this function with a call to divergence analysis when it is
// ready.
bool VPlanVLSAnalysisHIR::isUnitStride(const RegDDRef *Ref, unsigned Level) {
  int64_t ConstStride;
  auto DL = Ref->getDDRefUtils().getDataLayout();
  auto RefSizeInBytes = DL.getTypeAllocSize(Ref->getDestType());
  return getAccessType(Ref, Level, &ConstStride) == MemAccessTy::Strided &&
    ConstStride > 0 && static_cast<uint64_t>(ConstStride) == RefSizeInBytes;
}

OVLSMemref *VPlanVLSAnalysisHIR::createVLSMemref(const VPLoadStoreInst *Inst,
                                                 const unsigned VF) const {

  const HLNode *Node = Inst->HIR().getUnderlyingNode();
  const HLDDNode *DDNode = cast_or_null<HLDDNode>(Node);
  if (!DDNode)
    return nullptr;

  int Level = 0;
  if (HLLoop *Lp = Node->getParentLoop())
    Level = Lp->getNestingLevel();

  // TODO: Masked case is not supported right now by VPOCG. As soon as OVLS
  // still groups such masked memrefs, CM will try to reduce costs for them,
  // thus it's better to disable collection of masked memrefs here by now.
  // FIXME: This check is not complete for outer loop vectorization.
  if (isa<HLIf>(DDNode->getParent()))
    return nullptr;

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
    return new VPVLSClientMemrefHIR(OVLSAccessKind::SLoad, Ty, Inst, Level, DDA,
                                    Ref);
  if (AccTy == MemAccessTy::Strided && Opcode == Instruction::Store)
    return new VPVLSClientMemrefHIR(OVLSAccessKind::SStore, Ty, Inst, Level,
                                    DDA, Ref);
  if (AccTy == MemAccessTy::Indexed && Opcode == Instruction::Load)
    return new VPVLSClientMemrefHIR(OVLSAccessKind::ILoad, Ty, Inst, Level, DDA,
                                    Ref);
  if (AccTy == MemAccessTy::Indexed && Opcode == Instruction::Store)
    return new VPVLSClientMemrefHIR(OVLSAccessKind::IStore, Ty, Inst, Level,
                                    DDA, Ref);
  return nullptr;
}

const DDGraph VPVLSClientMemrefHIR::getDDGraph() const {
  const HLNode *Node = getInstruction()->HIR().getUnderlyingNode();
  assert(Node && "Expected underlying HLNode!");
  const HLLoop *Loop = Node->getParentLoop();
  const DDGraph &DDG = DDA->getGraph(Loop);
  return DDG;
}

} // namespace vpo

} // namespace llvm

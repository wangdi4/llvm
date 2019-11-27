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

  // Compute stride in terms of number of elements
  auto DL = Ref->getDDRefUtils().getDataLayout();
  auto RefSizeInBytes = DL.getTypeAllocSize(Ref->getDestType());
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
                                                 const unsigned VF) const {
  unsigned Opcode = Inst->getOpcode();

  const HLNode *Node = Inst->HIR.getUnderlyingNode();
  const HLDDNode *DDNode = cast_or_null<HLDDNode>(Node);
  if (!DDNode)
    return nullptr;

  int Level = 0;
  if (HLLoop *Lp = Node->getParentLoop())
    Level = Lp->getNestingLevel();

  // Restrict creation of VLSMemrefs to load/store instructions only
  // TODO: Remove this bailout when VLS is made an explicit transformation in
  // VPlan (JR : CMPLRLLVM-7613)
  auto *HInst = dyn_cast<HLInst>(DDNode);
  if (!HInst || !(isa<LoadInst>(HInst->getLLVMInstruction()) ||
                  isa<StoreInst>(HInst->getLLVMInstruction())))
    return nullptr;

  // VLS codegen currently handles cases where the underlying HLInst has only
  // one memref and the instruction is load/store. Cases where HIR Temp Cleanup
  // pass introduces extra load memrefs within the same HLInst is not correctly
  // handled.
  // TODO: Remove this bailout when VLS is made an explicit transformation in
  // VPlan (JR : CMPLRLLVM-7613)

  int CountMemref = llvm::count_if(
      make_range(DDNode->op_ddref_begin(), DDNode->op_ddref_end()),
      [&](const RegDDRef *Ref) { return Ref->isMemRef(); });

  if (CountMemref > 1)
    return nullptr;

  // TODO: Masked case is not supported right now by VPOCG. As soon as OVLS
  // still groups such masked memrefs, CM will try to reduce costs for them,
  // thus it's better to disable collection of masked memrefs here by now.
  // FIXME: This check is not complete for outer loop vectorization.
  if (isa<HLIf>(DDNode->getParent()))
    return nullptr;
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

  unsigned Size = DL.getTypeAllocSizeInBits(Ref->getDestType());
  if (!Size)
    return nullptr;

  OVLSType Ty = OVLSType(Size, VF);

  int64_t Stride = 0;
  // Overrides incoming AT.
  // TODO: remove getAccessType() from this place.
  MemAccessTy AccTy = getAccessType(Ref, Level, &Stride);
  Opcode = Ref->isRval() ? Instruction::Load : Instruction::Store;

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
  const HLNode *Node = getInstruction()->HIR.getUnderlyingNode();
  assert(Node && "Expected underlying HLNode!");
  const HLLoop *Loop = Node->getParentLoop();
  const DDGraph &DDG = Loop->getParentLoop()
                           ? DDA->getGraph(Loop->getParentLoop())
                           : DDA->getGraph(Loop->getParentRegion());
  return DDG;
}

} // namespace vpo

} // namespace llvm

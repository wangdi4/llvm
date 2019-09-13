//===---- HIRVLAAnalysis.cpp - Computes VLS Analysis ---------------------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements an HIR VLS-Client.
///
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"

#include "llvm/IR/Type.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRVLSClient.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-vls-analysis"

// TODO: Current implementation is largely HIR generic, but partly vectorizer
// specific (namely the exact conditions we check for each DDG edge).
// Can change this to work with an edge visitor that other users of this
// utility can define.
// TODO: vectorizer will want to take the distance into account to prune
// irrelevant dependences.
// TODO: vectorizer will want to prune edges from irrlevant levels.
// TODO: This function is conservative in the presence of unstructured code
// (specifically the dominance checks).
// TODO: This function relies on the topological sort numbers to tell if some
// RegDDRef is accessed between the accesses to Ref and AtRef, where Ref and
// AtRef dominate/potdominate one another. This maybe generally unsafe except
// for straight line code.
bool HIRVLSClientMemref::canAccessWith(const RegDDRef *Ref,
                                       const RegDDRef *AtRef,
                                       const VectVLSContext *VectContext) {

  // LLVM_DEBUG(dbgs() << "\ncanMove: "; Ref->dump());
  // LLVM_DEBUG(dbgs() << " To: "; AtRef->dump());

  DDGraph DDG = VectContext->getDDG();
  const HLDDNode *DDNode = Ref->getHLDDNode();
  const HLDDNode *AtDDNode = AtRef->getHLDDNode();

  //(1) Check Control Flow: In terms of CFG Ref and AtRef need to be
  //"equivalent": In the context of optVLS (which calls this utility), if
  // canAccessWith returns true then Ref and AtRef will be loaded together in
  // one location (using a load+shuffle sequence) instead of loaded separately
  // in two different locations (by two gather instructions). This means that
  // now anytime Ref is accessed AtRef will be accessed and vice versa. So
  // if there is a scenario/path in which Ref is accessed and AtRef isn't,
  // or the other way around, we have to return false.
  if (!HLNodeUtils::canAccessTogether(DDNode, AtDDNode))
    return false;

  //(2) Check Aliasing:
  // If there's a true/anti/output forward dependence (loop carried or not)
  // between Ref and any other Ref that may be accessed between the Ref
  // and AtRef: then it's illegal to move the Ref access to the location of
  // the AtRef access (because that will result in a backward dependence
  // which will make vectorization illegal). For example:
  //
  // r0 A[4*i] = ...
  // r1 ... = A[4*i+1]
  // r2 ... = A[4*i+4]
  // r3 A[4*i] = ...
  // r4 ... = A[4*i+2]
  // r5 ... = A[4*i+3]
  // r6 A[4*i] = ...
  //
  // It is safe to move r4 and r5 up past r3 to the location of r1 and r2
  // (loop will still be vectorizable), but it is not safe to move r1 and r2
  // down past r3 to the location of r4 and r5 (because the forward anti
  // dependence between r1/r2 and r3 will become a backward dependence).
  //
  // FIXME: This assumes that if there are any nodes past which it is not safe
  // to move Ref, then there will be a DDG edge between any such nodes
  // and Ref. If that's not the case we need to explicitly check for such
  // nodes (e.f. function calls?).
  RegDDRef *RegRef = const_cast<RegDDRef *>(Ref);
  for (auto II = DDG.outgoing_edges_begin(RegRef),
            EE = DDG.outgoing_edges_end(RegRef);
       II != EE; ++II) {
    const DDEdge *Edge = *II;
    DDRef *SinkRef = Edge->getSink();
    HLDDNode *SinkNode = SinkRef->getHLDDNode();
    // LLVM_DEBUG(dbgs() << "\nmove past loc " << HNode->getTopSortNum() << ".
    // "); Assuming structured code (no labels/gotos), we rely on the
    // topological sort number to reflect if sink is accessed between Ref and
    // AtRef. We don't care about a sink that is outside the range bordered by
    // Ref and AtRef. In the example above, if Ref,AtRef are r1,r5 then the sink
    // r3 is relevant, but the sink r0 and r6 are not relevant.
    // FIXME: Probably this check holds only for straight line code? may need a
    // stronger check for the general case
    if (!HLNodeUtils::isInTopSortNumRange(SinkNode, DDNode, AtDDNode) &&
        !HLNodeUtils::isInTopSortNumRange(SinkNode, AtDDNode, DDNode)) {
      continue;
    }
    // Lastly: Check the dependence edge.
    if (Edge->getSrc() == Edge->getSink()) {
      continue;
    }
    if (Edge->isInput()) {
      continue;
    }
    if (Edge->isForwardDep()) {
      return false;
    }
  }
  return true;
}

//"Strided Access" includes unit stride (Stride=1)
bool HIRVLSClientMemref::setStridedAccess() {
  assert(VectContext && "No Context");
  unsigned LoopLevel = VectContext->getLoopLevel();

  assert(LoopLevel > 0 && LoopLevel <= MaxLoopNestLevel &&
         " Level is invalid.");
  LLVM_DEBUG(dbgs() << "\nsetStridedAccess: Examine Ref "; Ref->dump());

  // CHECKME: Not supposed to find invariant refs... turn into an assert?
  if (Ref->isStructurallyInvariantAtLevel(LoopLevel)) {
    LLVM_DEBUG(dbgs() << "\n  Invariant at Level\n");
    return false;
  }

  Stride = Ref->getStrideAtLevel(LoopLevel);
  if (!Stride) {
    return false;
  }
  LLVM_DEBUG(dbgs() << "\n  Stride at Level is "; Stride->dump(1));

  if (Stride->isIntConstant(&ConstStride) && !ConstStride) {
    Stride->getCanonExprUtils().destroy(Stride);
    return false;
  }

  if (Ref->isRval()) {
    setAccessType(OVLSAccessType::getStridedLoadTy());
  } else {
    assert(Ref->isLval() && " Ref is invalid."); // CHECKME: drop this assert?
    setAccessType(OVLSAccessType::getStridedStoreTy());
  }
#if 0
  LLVM_DEBUG(dbgs() << "   Strided Access at Level " << LoopLevel << ".");
  if (ConstStride) {
    LLVM_DEBUG(dbgs() << " Constant Stride = " << ConstStride << ".\n"); 
  }
  else {
    LLVM_DEBUG(dbgs() << ". Non Constant Stride.\n");
  }
#endif
  return true;
}

// --------- client specific parts of the TTI Cost Model utilities

// Helper function: Return the data type of the pointer that accesses Mrf.
PointerType *getPtrType(const OVLSMemref &Mrf) {
  assert(isa<HIRVLSClientMemref>(Mrf) && "Expecting HIR Memref.\n");
  const RegDDRef *DDRef = (cast<HIRVLSClientMemref>(Mrf)).getRef();
  assert(DDRef->hasGEPInfo() && "Expecting a memref DDReft, not a terminal");
  const CanonExpr *CE = DDRef->getBaseCE();
  PointerType *BaseTy = cast<PointerType>(CE->getSrcType());
  return BaseTy;
}

// Helper function: Return the underlying LLVMIR Value of the pointer that
// accesses Mrf.
Value *getPtrVal(const OVLSMemref &Mrf) {
  assert(isa<HIRVLSClientMemref>(Mrf) && "Expecting HIR Memref.\n");
  const RegDDRef *DDRef = (cast<HIRVLSClientMemref>(Mrf)).getRef();
  assert(DDRef->hasGEPInfo() && "Expecting a memref DDReft, not a terminal");
  const HLNode *Node = DDRef->getHLDDNode();
  const HLInst *INode = dyn_cast<HLInst>(Node);
  assert(INode && "not an HLIInst Node");
  const Instruction *ConstInst = INode->getLLVMInstruction();
  Instruction *I = const_cast<Instruction *>(ConstInst);
  StoreInst *SI = dyn_cast<StoreInst>(I);
  LoadInst *LI = dyn_cast<LoadInst>(I);
  Value *PtrVal = SI ? SI->getPointerOperand() : LI->getPointerOperand();
  return PtrVal;
}

unsigned OVLSTTICostModelHIR::getMrfAddressSpace(const OVLSMemref &Mrf) const {
  PointerType *BaseTy = getPtrType(Mrf);
  return BaseTy->getPointerAddressSpace();
}

uint64_t
OVLSTTICostModelHIR::getGatherScatterOpCost(const OVLSMemref &Mrf) const {
  bool isLoad = Mrf.getAccessType().isStridedLoad();
  uint64_t GatherScatterCost;
  PointerType *BaseTy = getPtrType(Mrf);
  Type *DataTy = BaseTy->getElementType();
  bool isGatherOrScatterLegal = (isLoad && TTI.isLegalMaskedGather(DataTy)) ||
                                (!isLoad && TTI.isLegalMaskedScatter(DataTy));
  if (!isGatherOrScatterLegal)
    return 0;
  unsigned Opcode = isLoad ? Instruction::Load : Instruction::Store;
  uint32_t NumElements = Mrf.getType().getNumElements();
  assert(NumElements > 1 && "Unexpected NumElements");
  Type *VectorTy = VectorType::get(DataTy, NumElements);
  Value *PtrVal = getPtrVal(Mrf);
  bool isMaskRequired = false; // TODO
  unsigned Alignment = 0;      // TODO
  GatherScatterCost = TTI.getGatherScatterOpCost(Opcode, VectorTy, PtrVal,
                                                 isMaskRequired, Alignment);
  GatherScatterCost += TTI.getAddressComputationCost(VectorTy);
  return GatherScatterCost;
}

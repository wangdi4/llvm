//===---- HIRVLAAnalysis.cpp - Computes VLS Analysis ---------------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_LoopAnalysis/HIRVLSClient.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-vls-analysis"

// FIXME: Temporarily here. To be moved to RegDDRef.cpp
bool HIRVLSClientMemref::isInvariantAtLevel(const RegDDRef *RegDD,
                                            unsigned LoopLevel) {
  // Check the Base CE.
  if (RegDD->hasGEPInfo() &&
      !RegDD->getBaseCE()->isInvariantAtLevel(LoopLevel)) {
    return false;
  }

  // Check canon expr of the ddrefs to see if level exist.
  for (auto Iter = RegDD->canon_begin(), End = RegDD->canon_end(); Iter != End;
       ++Iter) {

    const CanonExpr *Canon = *Iter;
    // Check if CanonExpr is invariant i.e. IV is not present in any form inside
    // the canon expr.
    if (!Canon->isInvariantAtLevel(LoopLevel)) {
      return false;
    }
  }

  // Level doesn't exist in any of the canon exprs.
  return true;
}

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
    DDRef *SinkRef = II->getSink();
    HLDDNode *SinkNode = SinkRef->getHLDDNode();
    // DEBUG(dbgs() << "\nmove past loc " << HNode->getTopSortNum() << ". ");
    // Assuming structured code (no labels/gotos), we rely on the topological
    // sort number to reflect if sink is accessed between Ref and AtRef. We
    // don't care about a sink that is outside the range bordered by Ref and
    // AtRef. In the example above, if Ref,AtRef are r1,r5 then the sink r3 is
    // relevant, but the sink r0 and r6 are not relevant.
    // FIXME: Probably this check holds only for straight line code? may need a
    // stronger check for the general case
    if (!HLNodeUtils::isInTopSortNumRange(SinkNode, DDNode, AtDDNode) &&
        !HLNodeUtils::isInTopSortNumRange(SinkNode, AtDDNode, DDNode)) {
      continue;
    }
    // Lastly: Check the dependence edge.
    const DDEdge *Edge = &(*II);
    if (Edge->getSrc() == Edge->getSink()) {
      continue;
    }
    if (Edge->isINPUTdep()) {
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
  DEBUG(dbgs() << "\nsetStridedAccess: Examine Ref "; Ref->dump());

  // CHECKME: Not supposed to find invariant refs... turn into an assert?
  if (isInvariantAtLevel(Ref, LoopLevel)) {
    DEBUG(dbgs() << "\n  Invariant at Level\n");
    return false;
  }

  Stride = Ref->getStrideAtLevel(LoopLevel);
  if (!Stride) {
    return false;
  }
  DEBUG(dbgs() << "\n  Stride at Level is "; Stride->dump(1));

  if (Stride->isIntConstant(&ConstStride) && !ConstStride) {
    CanonExprUtils::destroy(Stride);
    return false;
  }

  if (Ref->isRval()) {
    setAccessType(OVLSAccessType::getStridedLoadTy());
  } else {
    assert(Ref->isLval() && " Ref is invalid."); // CHECKME: drop this assert?
    setAccessType(OVLSAccessType::getStridedStoreTy());
  }
#if 0
  DEBUG(dbgs() << "   Strided Access at Level " << LoopLevel << ".");
  if (ConstStride) {
    DEBUG(dbgs() << " Constant Stride = " << ConstStride << ".\n"); 
  }
  else {
    DEBUG(dbgs() << ". Non Constant Stride.\n");
  }
#endif
  return true;
}

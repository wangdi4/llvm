//===- IntelVPlanVLSClientHIR.h - ------------------------------------------===/
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
/// Implements HIR client version of VPVLSClientMemref, which is used by OptVLS.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVLSCLIENTHIR_H
#define LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVLSCLIENTHIR_H

#include "Intel_VPlan/IntelVPlanVLSClient.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"

using namespace llvm::loopopt;

namespace llvm {

namespace vpo {

class VPVLSClientMemrefHIR : public VPVLSClientMemref {
private:
  /// That is necessary to keep information about parent loop of the current
  /// memory reference, because whether it can or cannot be moved depends on
  /// it.
  const unsigned LoopLevel;
  HIRDDAnalysis *DDA;

  unsigned getLoopLevel() const { return LoopLevel; }
  const DDGraph getDDGraph() const {
    const HLLoop *Loop = cast<HLInst>(getInstruction()->HIR.getUnderlyingNode())
                             ->getParentLoop();
    const DDGraph &DDG = Loop->getParentLoop()
                             ? DDA->getGraph(Loop->getParentLoop())
                             : DDA->getGraph(Loop->getParentRegion());
    return DDG;
  }

  const RegDDRef *getRegDDRef() const {
    auto I = cast<HLInst>(Inst->HIR.getUnderlyingNode());
    return Inst->getOpcode() == Instruction::Load ? I->getOperandDDRef(1)
                                                  : I->getLvalDDRef();
  }

public:
  explicit VPVLSClientMemrefHIR(const OVLSAccessType &AccTy, const OVLSType &Ty,
                                const VPInstruction *Inst,
                                const unsigned LoopLevel, HIRDDAnalysis *DDA)
      : VPVLSClientMemref(VLSK_VPlanHIRVLSClientMemref, AccTy, Ty, Inst),
        LoopLevel(LoopLevel), DDA(DDA) {}

  virtual ~VPVLSClientMemrefHIR() {}

  virtual bool isAConstDistanceFrom(const OVLSMemref &From,
                                    int64_t *Dist) final {
    auto FromMem = dyn_cast<const VPVLSClientMemrefHIR>(&From);
    assert(FromMem && "Invalid OVLSMemref");
    return DDRefUtils::getConstByteDistance(getRegDDRef(),
                                            FromMem->getRegDDRef(), Dist);
  }

  virtual bool haveSameNumElements(const OVLSMemref &Memref) final {
    return getNumElements() == Memref.getNumElements();
  }

  virtual bool canMoveTo(const OVLSMemref &To) final {
    const RegDDRef *ToRef =
        dyn_cast<const VPVLSClientMemrefHIR>(&To)->getRegDDRef();
    const HLDDNode *ToDDNode = ToRef->getHLDDNode();

    const RegDDRef *FromRef = getRegDDRef();
    const HLDDNode *FromDDNode = FromRef->getHLDDNode();

    //(1) Check Control Flow: In terms of CFG FromRef and ToRef need to be
    //"equivalent": In the context of optVLS (which calls this utility), if
    // canAccessWith returns true then FromRef and ToRef will be loaded together
    // in one location (using a load+shuffle sequence) instead of loaded
    // separately in two different locations (by two gather instructions). This
    // means that now anytime FromRef is accessed ToRef will be accessed and
    // vice versa. So if there is a scenario/path in which FromRef is accessed
    // and ToRef isn't, or the other way around, we have to return false.
    if (!HLNodeUtils::canAccessTogether(FromDDNode, ToDDNode))
      return false;

    //(2) Check Aliasing:
    // If there's a true/anti/output forward dependence (loop carried or not)
    // between FromRef and any other Ref that may be accessed between the
    // FromRef and ToRef: then it's illegal to move the FromRef access to the
    // location of the ToRef access (because that will result in a backward
    // dependence which will make vectorization illegal). For example:
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
    // to move FromRef, then there will be a DDG edge between any such nodes
    // and FromRef. If that's not the case we need to explicitly check for such
    // nodes (e.f. function calls?).
    RegDDRef *RegRef = const_cast<RegDDRef *>(FromRef);
    const DDGraph &DDG = getDDGraph();
    for (auto II = DDG.outgoing_edges_begin(RegRef),
              EE = DDG.outgoing_edges_end(RegRef);
         II != EE; ++II) {
      const DDEdge *Edge = *II;
      DDRef *SinkRef = Edge->getSink();
      HLDDNode *SinkNode = SinkRef->getHLDDNode();
      // DEBUG(dbgs() << "\nmove past loc " << HNode->getTopSortNum() << ". ");
      // Assuming structured code (no labels/gotos), we rely on the topological
      // sort number to reflect if sink is accessed between FromRef and ToRef.
      // We don't care about a sink that is outside the range bordered by
      // FromRef and ToRef. In the example above, if FromRef,ToRef are r1,r5
      // then the sink r3 is relevant, but the sink r0 and r6 are not relevant.
      // FIXME: Probably this check holds only for straight line code? may need
      // a stronger check for the general case
      if (!HLNodeUtils::isInTopSortNumRange(SinkNode, FromDDNode, ToDDNode) &&
          !HLNodeUtils::isInTopSortNumRange(SinkNode, ToDDNode, FromDDNode)) {
        continue;
      }
      // Lastly: Check the dependence edge.
      if (Edge->getSrc() == Edge->getSink())
        continue;
      if (Edge->isINPUTdep())
        continue;
      if (Edge->isForwardDep())
        return false;
    }
    return true;
  }

  virtual bool hasAConstStride(int64_t *Stride) const final {
    return getRegDDRef()->getConstStrideAtLevel(getLoopLevel(), Stride);
  }

  virtual unsigned getLocation() const final { return 0; }

  static bool classof(const OVLSMemref *Memref) {
    return Memref->getKind() == VLSK_VPlanHIRVLSClientMemref;
  }
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVLSCLIENTHIR_H

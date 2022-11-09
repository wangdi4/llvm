//===- IntelVPlanVLSClientHIR.h - ------------------------------------------===/
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
/// Implements HIR client version of VPVLSClientMemref, which is used by OptVLS.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVLSCLIENTHIR_H
#define LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVLSCLIENTHIR_H

#include "../IntelVPlanVLSClient.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"

using namespace llvm::loopopt;

namespace llvm {

namespace vpo {

class VPVLSClientMemrefHIR final : public VPVLSClientMemref {
private:
  /// Keep information about loop being vectorized, so that we can determine
  /// whether memory reference can or cannot be moved using the same.
  const HLLoop *TheLoop = nullptr; // Loop being vectorized
  HIRDDAnalysis *DDA;
  const RegDDRef *Ref;

  unsigned getLoopLevel() const { return TheLoop->getNestingLevel(); }
  const DDGraph getDDGraph() const { return DDA->getGraph(TheLoop); }

protected:
  friend class llvm::OVLSContext;
  explicit VPVLSClientMemrefHIR(OVLSContext &Context, OVLSAccessKind AccKind,
                                const OVLSType &Ty, const VPLoadStoreInst *Inst,
                                const HLLoop *Loop, HIRDDAnalysis *DDA,
                                const RegDDRef *Ref)
      : VPVLSClientMemref(Context, VLSK_VPlanHIRVLSClientMemref, AccKind, Ty,
                          Inst,
                          /*VLSA=*/nullptr),
        TheLoop(Loop), DDA(DDA), Ref(Ref) {}

public:
  static VPVLSClientMemrefHIR *
  create(OVLSContext &Context, OVLSAccessKind AccKind, const OVLSType &Ty,
         const VPLoadStoreInst *Inst, const HLLoop *Loop, HIRDDAnalysis *DDA,
         const RegDDRef *Ref) {
    return Context.create<VPVLSClientMemrefHIR>(AccKind, Ty, Inst, Loop, DDA,
                                                Ref);
  }

  Optional<int64_t>
  getConstDistanceFrom(const OVLSMemref &From) const override {
    auto *FromMem = cast<VPVLSClientMemrefHIR>(&From);
    int64_t Dist;
    if (DDRefUtils::getConstByteDistance(getRegDDRef(), FromMem->getRegDDRef(),
                                         &Dist))
      return Dist;
    return None;
  }

  bool canMoveTo(const OVLSMemref &To) override {
    const VPVLSClientMemrefHIR *ToHIR = cast<const VPVLSClientMemrefHIR>(&To);
    const RegDDRef *ToRef = ToHIR->getRegDDRef();
    const HLDDNode *ToDDNode = ToRef->getHLDDNode();

    const RegDDRef *FromRef = getRegDDRef();
    const HLDDNode *FromDDNode = FromRef->getHLDDNode();

    // OVLS analysis when forming a new group with the given memref, sets
    // the OVLSMemref as the insertion point. When adding the memref to
    // this new group subsequently, it later asserts to see if it is
    // safe to insert the memref at the insertion point. This essentially
    // causes canMoveTo be called with the same ToRef and FromRef. DD based
    // checks will fail for this case and so we check and return true for
    // this case here. Note that LLVM IR based VLS implementation also has
    // the same check. TODO - see if we can special case this check in OptVLS
    // analysis.
    const VPLoadStoreInst *ToInst = ToHIR->getInstruction();
    const VPLoadStoreInst *FromInst = getInstruction();
    if (ToInst == FromInst)
      return true;

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
    const DDGraph &DDG = getDDGraph();
    auto hasDependency = [](const HLDDNode *FromDDNode,
                            const HLDDNode *ToDDNode, const DDEdge *Edge,
                            const bool IsOutgoingEdge) -> bool {
      DDRef *Ref = IsOutgoingEdge ? Edge->getSink() : Edge->getSrc();
      HLDDNode *Node = Ref->getHLDDNode();

      // DEBUG(dbgs() << "\nmove past loc " << HNode->getTopSortNum() << ". ");
      // Assuming structured code (no labels/gotos), we rely on the topological
      // sort number to reflect if sink is accessed between FromRef and ToRef.
      // We don't care about a sink that is outside the range bordered by
      // FromRef and ToRef. In the example above, if FromRef,ToRef are r1,r5
      // then the sink r3 is relevant, but the sink r0 and r6 are not relevant.
      // FIXME: Probably this check holds only for straight line code? may need
      // a stronger check for the general case
      if (!HLNodeUtils::isInTopSortNumRange(Node, FromDDNode, ToDDNode) &&
          !HLNodeUtils::isInTopSortNumRange(Node, ToDDNode, FromDDNode)) {
        return false;
      }
      // Lastly: Check the dependence edge.
      if (Edge->getSrc() == Edge->getSink())
        return false;
      if (Edge->isInput())
        return false;
      return true;
    };
    for (auto RefIt = FromDDNode->all_dd_begin(),
              RefEndIt = FromDDNode->all_dd_end(); RefIt != RefEndIt; ++RefIt) {
      for (auto II = DDG.outgoing_edges_begin(*RefIt),
                EE = DDG.outgoing_edges_end(*RefIt);
           II != EE; ++II) {
        if (hasDependency(FromDDNode, ToDDNode, *II, true))
          return false;
      }

      for (auto II = DDG.incoming_edges_begin(*RefIt),
                EE = DDG.incoming_edges_end(*RefIt);
           II != EE; ++II) {
        if (hasDependency(FromDDNode, ToDDNode, *II, false))
          return false;
      }
    }
    return true;
  }

  Optional<int64_t> getConstStride() const override {
    int64_t Stride;
    if (getRegDDRef()->getConstStrideAtLevel(getLoopLevel(), &Stride))
      return Stride;
    return None;
  }

  static bool classof(const OVLSMemref *Memref) {
    return Memref->getKind() == VLSK_VPlanHIRVLSClientMemref;
  }

  const RegDDRef *getRegDDRef() const { return Ref; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &Os, unsigned Indent = 0) const override {
    VPVLSClientMemref::print(Os, Indent);
    Os << " | ";
    formatted_raw_ostream Fos(Os);
    getRegDDRef()->print(Fos);
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVLSCLIENTHIR_H

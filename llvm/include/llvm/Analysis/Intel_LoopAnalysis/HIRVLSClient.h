//===------ HIRVLSAnalysis.h - Provides Locality Analysis -------*- C++-*-===//
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
/// This file provides the utilities required by the OptVLS analyzer.
/// OptVLS operates on OVLSMemrefs -- an IR independent representation of
/// memory references. This file defines the HIR implementation of OVLSMemrefs,
/// for the purpose of the vectorizer.
///
/// An HIR analysis/transformation pass that wants to use the services of
/// OptVLS, namely an HIR VLS-Client pass, such as the vectorizer, needs to
/// provide an OVLSMemref implementation.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRVLSCLIENT_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRVLSCLIENT_H

#include "llvm/IR/Intel_LoopIR/HLNode.h"

#include "llvm/Analysis/Intel_OptVLS.h"
#include "llvm/Analysis/Intel_OptVLSClientUtils.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOVecContext.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/Pass.h"
#include <map>

using namespace llvm::vpo;

namespace llvm {
namespace loopopt {

class HLLoop;
class DDRefUtils;

typedef SmallVector<RegDDRef *, 16> LoopMemrefsVector;
typedef LoopMemrefsVector::iterator LoopMemrefsIt;
typedef SmallDenseMap<const RegDDRef *, OVLSMemref *, 16> HIRToVLSMemrefsMap;

/// Holds information about the current context underwhich the memrefs will be
/// analyzed, namely the specific loop that is being considered, the DDG
/// for that loop, and the Vectorization Factor.
///
/// Normally we expect to deal with Memrefs that all point to the same context.
/// The utilities defined usually operate on pairs of Memrefs, both of which
/// belong to the same context.
///
/// The client analysis pass may consider several contexts. Specifically the
/// vectorizer will consider different loops in a loop nest and different VFs.
/// Therefore the context the Memrefs point to may change.
typedef class VPOVecContextHIR VectVLSContext;

/// A memory reference that is considered for Vector-Load-Store (VLS)
/// optimization.
/// It provides an abstraction of the underlying HIR-based memory reference
/// representation (namely, a RegDDRef).
/// Reflects an access pattern under a specific vectorization context. For
/// example:
/// for (i=0; i<N; i++) {
///    for (j=0; i<M; j++) {
///       a[b[i] + 2*j] = x;
///    }
/// }
/// The HIRVLSClientMemref for the 'a' access represents a strided access with
/// Constant stride = 2 elements when Loop is the j-loop; OTHO when Loop is
/// the i-loop it represents an indexed access with index array b[i];
class HIRVLSClientMemref : public OVLSMemref {
public:
  /// Base class is initialized with NumElements=0
  HIRVLSClientMemref(RegDDRef *Ref, VectVLSContext *Cntxt)
      : OVLSMemref(VLSK_HIRVLSClientMemref,
                   OVLSType(Ref->getCanonExprUtils().getTypeSizeInBits(
                                Ref->getSrcType()),
                            1),
                   OVLSAccessType::getUnknownTy()),
        Ref(Ref), VectContext(Cntxt), Stride(nullptr), ConstStride(0) {}

  ~HIRVLSClientMemref(){};

  static bool classof(const OVLSMemref *Mrf) {
    return Mrf->getKind() == VLSK_HIRVLSClientMemref;
  }

  const RegDDRef *getRef() const { return Ref; }

  /// \name Functions to get the context: e.g. under which loop level
  /// this memref is now being considered.
  /// @{
  int getLoopLevelContext() const {
    assert(VectContext);
    return VectContext->getLoopLevel();
  }
  bool sameVectContext(const VectVLSContext *OtherContext) const {
    return (OtherContext == VectContext);
  }
  /// @}

  /// \name Functions to query/get the stride of the memory access, in the loop
  /// level that is currently being considered, if it is constant. For example:
  /// for (i=0; i<N; i++) {
  ///    for (j=0; i<M; j++) {
  ///       a[B*i+5*j] = x;
  ///    }
  /// }
  /// If LoopLevel is the i-loop, Stride is the size of B elements in bytes;
  /// B is not a constant and therefore 0/false will be returned by
  /// getConstStride/hasConstStride, respectively.
  /// If LoopLevel is the j-loop, Stride is the size of 5 elements in bytes;
  /// This time it is a constant, and therefore 5*ElemSize and true
  /// will be returned by getConstStride and hasConstStride, respectively.
  /// @{
  int64_t getConstStride() const { return ConstStride; }
  bool hasConstStride() const { return (ConstStride != 0); }
  /// @}

  /// \brief Tries to compute the distance between this memref and \p Mrf.
  /// If it is able to compute a constant distance between the two Memrefs
  /// it returns true, otherwise it returns false.
  /// \param [out] Distance holds the constant Distance if found.
  /// This method will be called by the OptVLS engine when the engine is asked
  /// to form groups from Memrefs of a certain client. It operates on the base
  /// class OVLSMemref, which different clients can derive. Clients of the
  /// engine, such as this HIRVLSClient, will always pass the engine Memrefs of
  /// the same type (client type). Normally no usage scenario will mix \p Mrfs
  /// from two different clients. This is why this method has to accept the
  /// base type pointer, but always expects to get an HIRVLSClientMemref.
  bool isAConstDistanceFrom(const OVLSMemref &Mrf, int64_t *Distance) {
    assert(isa<HIRVLSClientMemref>(&Mrf) && "Expected HIRVLSClientMemref");
    const HIRVLSClientMemref *CLMrf = cast<const HIRVLSClientMemref>(&Mrf);
    const RegDDRef *Ref2 = CLMrf->getRef();
    // In the context of VLS grouping, if the two Memrefs don't have the same
    // access type they will never end up in the same group so don't bother
    // computing the distance.
    if (!(this->getAccessType() == CLMrf->getAccessType()))
      return false;
    // Another shortcut: if these are strided accesses with different strides,
    // don't bother computing the distance (they will not end up in the same
    // group, and anyway the distance will not be constant in this case).
    if ((this->getAccessType()).isStridedAccess() &&
        (this->hasConstStride() && CLMrf->hasConstStride() &&
         !(this->getConstStride() == CLMrf->getConstStride())))
      return false;

    return DDRefUtils::getConstByteDistance(Ref, Ref2, Distance);
  }

  /// \bried Returns true if this and Memref have the same number of elements.
  bool haveSameNumElements(const OVLSMemref &Memref) {
    // TODO: Verify that have same VecContext?
    return true; // Assuming same VF. CHECKME.
  }

  /// \brief Checks if this memref can be moved to the location of \p Mrf.
  /// Both aliasing and control flow conditions are checked, to make sure
  /// that the semantics of the vectorized loop will be preserved (namely
  /// no forward dependences will turn into backward dependences, etc.).
  /// This method will be called by the OptVLS engine when the engine is asked
  /// to form groups from Memrefs of a certain client. It operates on the base
  /// class OVLSMemref, which different clients can derive. Clients of the
  /// engine, such as this HIRVLSClient, will always pass the engine Memrefs of
  /// the same type (client type). Normally no usage scenario will mix \p Mrfs
  /// from two different clients. This is why this method has to accept the
  /// base type pointer, but always expects to get an HIRVLSClientMemref.
  bool canMoveTo(const OVLSMemref &Mrf) {
    assert(isa<HIRVLSClientMemref>(&Mrf) && "Expected HIRVLSClientMemref");
    const HIRVLSClientMemref *CLMrf = cast<const HIRVLSClientMemref>(&Mrf);
    const RegDDRef *Ref2 = CLMrf->getRef();
    assert(Ref2);
    assert(CLMrf->sameVectContext(VectContext));
    return canAccessWith(Ref, Ref2, VectContext);
  }

  bool hasAConstStride(int64_t *Stride) const {
    if (!hasConstStride())
      return false;

    *Stride = getConstStride();
    return true;
  }

  /// \brief Return the location of this in the code. The location should be
  /// relative to other Memrefs sent to the VLS engine. Here it is obtained
  /// by the topological sort number. Assuming structured code
  /// (no labels/gotos), we rely on the topological sort number to reflect
  /// the relative ordering between the different OVLSMemref accesses.
  /// This assumption may only hold within straight line code segments;
  /// indeed the vectorizer will only send the VLS engine Mrfs that all reside
  /// in the same straight line code segement.
  unsigned getLocation() const {
    const RegDDRef *Ref = getRef();
    assert(Ref);
    const HLDDNode *DDNode = Ref->getHLDDNode();
    return DDNode->getTopSortNum();
  }

  /// \brief Checks if \p Ref can be moved to the location of \p AtRef.
  /// This function returns true when it is safe to access \p Ref together
  /// with \p AtRef at the location of \p AtRef.
  /// Both aliasing and control flow conditions are checked, to make sure
  /// that the semantics of the vectorized loop containing Ref and AtRef
  /// will be preserved (namely no forward dependences will turn into backward
  /// dependences, etc.).
  /// \param VectContext holds the vectorization context, namely the
  /// data-dependence graph in the relevant loop, and the VF (which determines
  /// the relevant dependence distances).
  static bool canAccessWith(const RegDDRef *Ref, const RegDDRef *AtRef,
                            const VectVLSContext *VectContext);

  /// \brief Analyzes the access pattern of this memref under the current
  /// context (namely, in the loop VectContext->LoopLevel). If the analysis is
  /// successful and the access pattern is identified as a strided access
  /// with an invariant stride (in the loop in question) it returns true
  /// and sets the access pattern information, namely: Stride, constStride and
  /// AccType.
  /// Returns false otherwise.
  bool setStridedAccess();

  // FIXME: Need to decide if this has to be associated with VF, and if so to
  // allow setting it only when VF is set.
  void setNumElements(unsigned NumElements) {
    OVLSMemref::setNumElements(NumElements);
  }

private:
  /// \brief The memref that this object represents.
  const RegDDRef *Ref;

  /// \brief The Client Context (Loop, Vectorization-Factor) under which this
  /// memref representation is obtained. The Context may change as the client
  /// pass examines different nesting levels to vectorize and different
  /// Vectorization Factor.
  VectVLSContext *VectContext;

  /// \brief If this is a strided access (under VectContext) then \p Stride is
  /// the Stride in bytes of this memref in VectContext->LoopLevel. Can change
  /// if VectContext changes.
  CanonExpr *Stride;

  /// \brief If this is a strided access (under VectContext) then \p ConstStride
  /// is the Stride in bytes of this memref in VectContext->LoopLevel, if it
  /// is a known constant number. Can change if VectContext changes.
  int64_t ConstStride;

  CanonExpr *getStride() const { return Stride; }

  // TODO: Add verify: valid memref, element-size and access-type match Ref,
  // NumElements matches VF
};

/// Implementation for the HIR vectorizer client, operating on
/// HIRVLSClientMemrefs (scalar memrefs).
class OVLSTTICostModelHIR : public OVLSTTICostModel {
public:
  OVLSTTICostModelHIR(const TargetTransformInfo &TTI, LLVMContext &C)
      : OVLSTTICostModel(TTI, C) {}

  Type *getMrfDataType(const OVLSMemref *Mrf) const override {
    assert(isa<HIRVLSClientMemref>(Mrf) && "Expecting HIR Memref.\n");
    return (cast<HIRVLSClientMemref>(Mrf))->getRef()->getSrcType();
  }
  unsigned getMrfAddressSpace(const OVLSMemref &Mrf) const override;
  uint64_t getGatherScatterOpCost(const OVLSMemref &Mrf) const override;
};

} // End namespace loopopt
} // End namespace llvm

#endif

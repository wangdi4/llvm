//===- IntelVPlanVLSClient.h - ---------------------------------------------===/
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
/// This class implements OVLSMemref for VPlan.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVLSCLIENT_H
#define LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVLSCLIENT_H

#include "Intel_VPlan/IntelVPlan.h"
#include "Intel_VPlan/IntelVPlanCostModel.h"
#include "llvm/Analysis/Intel_OptVLS.h"

namespace llvm {
namespace vpo {

class VPVLSClientMemref : public OVLSMemref {
#if INTEL_CUSTOMIZATION
protected:
  explicit VPVLSClientMemref(const OVLSMemrefKind &Kind,
                             const OVLSAccessType &AccTy, const OVLSType &Ty,
                             const VPInstruction *Inst)
      : OVLSMemref(Kind, Ty, AccTy), Inst(Inst) {}
#endif // INTEL_CUSTOMIZATION

  const VPInstruction *Inst;

  static llvm::OVLSType getOVLSType(const VPInstruction *Inst,
                                    const unsigned VF) {
    return llvm::OVLSType();
  }

  static llvm::OVLSAccessType getOVLSAccessType(const VPInstruction *Inst) {
    // FIXME: The type should be known at this point.
    return OVLSAccessType::getUnknownTy();
  }

public:
  explicit VPVLSClientMemref(const VPInstruction *Inst, const unsigned VF)
      : OVLSMemref(VLSK_VPlanVLSClientMemref, getOVLSType(Inst, VF),
                   getOVLSAccessType(Inst)),
        Inst(Inst) {}

  virtual ~VPVLSClientMemref() {}

  /// Return true if constant distance between current memref and \p From
  /// can be computed and assign this distance in \p Dist.
  /// If distance cannot be computed or it's non-constant, return false.
  virtual bool isAConstDistanceFrom(const OVLSMemref &From,
                                    int64_t *Dist) override {
    // FIXME: Implement this function.
    return false;
  }

  /// Return true if number of elements in \p Memref is identical to number of
  /// elements in current memref.
  virtual bool haveSameNumElements(const OVLSMemref &Memref) override {
    // FIXME: Implement this function.
    return false;
  }

  /// Return true if current memref can be moved to memref \p To.
  virtual bool canMoveTo(const OVLSMemref &To) override {
    // FIXME: Implement this function.
    return false;
  }

  /// Return true if current memref has constant stride and return this stride
  /// in \p Stride.
  virtual bool hasAConstStride(int64_t *Stride) const override {
    // FIXME: Implement this function.
    return false;
  }

  const VPInstruction *getInstruction(void) const { return Inst; }

  void dump() const {
    print(errs());
    errs() << '\n';
  }

  void print(raw_ostream &Os, const Twine Indent = "") const {
    Os << Indent;
    Os << "OVLSMemref for VPInst ";
    Inst->print(Os);
    Os << "[ AccessType: ";
    getAccessType().print(Os);
    Os << " | VLSType = ";
    getType().print(Os);
    int64_t Stride;
    Os << " | Stride = ";
    if (hasAConstStride(&Stride))
      Os << Stride;
    else
      Os << "unknown";
    Os << " ]\n";
  }

  static bool classof(const OVLSMemref *Memref) {
    return Memref->getKind() == VLSK_VPlanVLSClientMemref ||
           Memref->getKind() == VLSK_VPlanHIRVLSClientMemref;
  }
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVLSCLIENT_H

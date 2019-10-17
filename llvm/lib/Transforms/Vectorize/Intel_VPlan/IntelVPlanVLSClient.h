//===- IntelVPlanVLSClient.h - ---------------------------------------------===/
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
/// This class implements OVLSMemref for VPlan.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVLSCLIENT_H
#define LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVLSCLIENT_H

#include "Intel_VPlan/IntelVPlan.h"
#include "llvm/Analysis/Intel_OptVLS.h"

namespace llvm {
namespace vpo {

class VPlanVLSAnalysis;

class VPVLSClientMemref : public OVLSMemref {
  const VPInstruction *Inst;
  const VPlanVLSAnalysis *VLSA;
  const SCEV *ScevExpr = nullptr;

public:
  VPVLSClientMemref(const OVLSMemrefKind &Kind, OVLSAccessKind AccKind,
                    const OVLSType &Ty, const VPInstruction *Inst,
                    const VPlanVLSAnalysis *VLSA);

  Optional<int64_t> getConstDistanceFrom(const OVLSMemref &From) override;

  bool canMoveTo(const OVLSMemref &To) override;

  Optional<int64_t> getConstStride() const override;

  bool dominates(const OVLSMemref &Mrf) const override;
  bool postDominates(const OVLSMemref &Mrf) const override;

  const VPInstruction *getInstruction(void) const { return Inst; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &Os, unsigned Indent) const override;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  static bool classof(const OVLSMemref *Memref) {
    return Memref->getKind() == VLSK_VPlanVLSClientMemref ||
           Memref->getKind() == VLSK_VPlanHIRVLSClientMemref;
  }

private:
  const SCEV *getSCEVForVPValue(const VPValue *Val) const;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVLSCLIENT_H

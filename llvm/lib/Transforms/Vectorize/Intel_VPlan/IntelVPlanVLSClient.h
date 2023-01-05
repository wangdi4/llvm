//===- IntelVPlanVLSClient.h - ---------------------------------------------===/
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
/// This class implements OVLSMemref for VPlan.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVLSCLIENT_H
#define LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVLSCLIENT_H

#include "IntelVPlan.h"
#include "llvm/Analysis/Intel_OptVLS.h"

namespace llvm {
namespace vpo {

class VPlanVector;
class VPlanVLSAnalysis;

class VPVLSClientMemref : public OVLSMemref {
  const VPLoadStoreInst *Inst;
  const VPlanVLSAnalysis *VLSA;

protected:
  friend class llvm::OVLSContext;
  VPVLSClientMemref(OVLSContext &Context, const OVLSMemrefKind &Kind,
                    OVLSAccessKind AccKind, const OVLSType &Ty,
                    const VPLoadStoreInst *Inst, const VPlanVLSAnalysis *VLSA)
      : OVLSMemref(Context, Kind, Ty, AccKind), Inst(Inst), VLSA(VLSA) {}

public:
  static VPVLSClientMemref *create(OVLSContext &Context,
                                   const OVLSMemrefKind &Kind,
                                   OVLSAccessKind AccKind, const OVLSType &Ty,
                                   const VPLoadStoreInst *Inst,
                                   const VPlanVLSAnalysis *VLSA) {
    return Context.create<VPVLSClientMemref>(Kind, AccKind, Ty, Inst, VLSA);
  }

  Optional<int64_t> getConstDistanceFrom(const OVLSMemref &From) const override;

  bool canMoveTo(const OVLSMemref &To) override;

  bool static isConstStride(const VPLoadStoreInst *Inst,
                            const VPlanScalarEvolutionLLVM *VPSE);
  Optional<int64_t> getConstStride() const override;

  bool dominates(const OVLSMemref &Mrf) const override;
  bool postDominates(const OVLSMemref &Mrf) const override;

  const VPLoadStoreInst *getInstruction(void) const { return Inst; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &Os, unsigned Indent) const override;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  static bool classof(const OVLSMemref *Memref) {
    return Memref->getKind() == VLSK_VPlanVLSClientMemref ||
           Memref->getKind() == VLSK_VPlanHIRVLSClientMemref;
  }

private:
  VPlanScalarEvolutionLLVM &getVPSE() const {
    // FIXME: Get VPSE from VLSA once VLSA is moved into VPlan object as well.
    const VPlanVector *Plan =
        cast<const VPlanVector>(Inst->getParent()->getParent());
    return *static_cast<VPlanScalarEvolutionLLVM *>(Plan->getVPSE());
  }

  static const SCEV *getAddressSCEV(const VPLoadStoreInst *LSI) {
    return VPlanScalarEvolutionLLVM::toSCEV(LSI->getAddressSCEV());
  }
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVLSCLIENT_H

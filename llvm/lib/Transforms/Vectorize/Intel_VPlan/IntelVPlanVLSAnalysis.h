//===- IntelVPlanVLSAnalysis.h - -------------------------------------------===/
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
/// This file defines the VPlanVLSAnalysis class that is used to collect and
/// store memory references for different VPlans.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLANVLSANALYSIS_H
#define LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLANVLSANALYSIS_H

#include "Intel_VPlan/IntelVPlanVLSClient.h"
#if INTEL_CUSTOMIZATION
#include "VPlanHIR/IntelVPlanVLSClientHIR.h"
#endif // INTEL_CUSTOMIZATION

namespace llvm {

namespace vpo {

class VPlan;

/// VPlanVLSAnalysis class is used to collect all memory references in VPlan,
/// pass them to OptVLS interface and store result internally, so there's no need
/// to recollect and recompute grouping information for same HCFG and VF.
class VPlanVLSAnalysis {
protected:
  enum class MemAccessTy {
    Unknown,
    Uniform,
    Strided,
    Indexed,
  };

  // FIXME: With Type information this context is not needed. Moreover,
  // it's here because it's easier to pass context downstream using
  // this class, rather then obtain the context from deep inside of
  // vectorizer.
  LLVMContext &Context;

  /// Finds a group for a given VPInstruction.
  OVLSGroup *getGroupForInstruction(const VPlan *Plan,
                                    const VPInstruction *Inst) const {
    const VLSInfo &VlsInfoForVPlan = Plan2VLSInfo.find(Plan)->second;
    auto MemrefIt = llvm::find_if(
        VlsInfoForVPlan.Memrefs, [&Inst](const OVLSMemref *Memref) {
          return cast<VPVLSClientMemref>(Memref)->getInstruction() == Inst;
        });
    if (MemrefIt != VlsInfoForVPlan.Memrefs.end()) {
      auto GIt = VlsInfoForVPlan.Mem2Group.find(*MemrefIt);
      if (GIt != VlsInfoForVPlan.Mem2Group.end())
        return GIt->second;
    }
    return nullptr;
  }

  static MemAccessTy getInstructionAccessType(const VPInstruction *Inst,
                                              const unsigned Level);

  virtual OVLSMemref *createVLSMemref(const VPInstruction *Inst,
                                      const MemAccessTy &AccTy,
                                      const unsigned Level,
                                      const unsigned VF) const {
    llvm_unreachable("Is not implemented yet.");
  }

private:
  /// To call OptVLSInterface, vectorizer has to pass maximum physical
  /// vector length for a given target. From vectorization point of view,
  /// this is redundant argument and OVLSCostModel + VPlanCostModel will
  /// decide which VF is profitable regardless of physical vector length.
  const unsigned MaxVectorWidthInBytes = 64;

  /// Data structure to keep all needed information for each VPlan.
  struct VLSInfo {
    OVLSMemrefVector Memrefs;
    OVLSGroupVector Groups;
    OVLSMemrefToGroupMap Mem2Group;

    void eraseGroups() {
      for (OVLSGroup *X : Groups)
        delete X;
      Groups.clear();
      Mem2Group.clear();
    }

    void erase() {
      for (OVLSMemref *X : Memrefs)
        delete X;
      eraseGroups();
    }
  };

  SmallDenseMap<const VPlan *, VLSInfo> Plan2VLSInfo;

public:
  explicit VPlanVLSAnalysis(LLVMContext &Context) : Context(Context) {}
  virtual ~VPlanVLSAnalysis() {}
  /// Collect all memrefs within given VPlan and reflect given VF in
  /// each collected memref.
  /// \p Force parameter must be used if VPlan was modified so that
  /// memrefs were added, moved or removed.
  void getOVLSMemrefs(const VPlan *Plan, const unsigned VF,
                      const bool Force = false);

  /// Returns OVLSGroup for a given VPlan and VPInstruction.
  OVLSGroup *getGroupsFor(const VPlan *Plan, const VPInstruction *Inst) const {
    assert(Plan2VLSInfo.count(Plan) && "OVLSMemrefs were not collected.");
    return getGroupForInstruction(Plan, Inst);
  }

  void dump(const VPlan *Plan) const;
  void dump() const;

  LLVMContext &getContext() const { return Context; }
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLANVLSANALYSIS_H

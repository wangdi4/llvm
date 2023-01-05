//===- IntelVPlanVLSAnalysis.h - -------------------------------------------===/
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
/// This file defines the VPlanVLSAnalysis class that is used to collect and
/// store memory references for different VPlans.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLANVLSANALYSIS_H
#define LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLANVLSANALYSIS_H

#include "IntelVPlanVLSClient.h"
#if INTEL_CUSTOMIZATION
#include "VPlanHIR/IntelVPlanVLSClientHIR.h"
#endif // INTEL_CUSTOMIZATION

namespace llvm {

namespace vpo {

class VPlan;
class VPlanScalarEvolutionLLVM;
class VPlanVLSCostModel;

/// VPlanVLSAnalysis class is used to collect all memory references in VPlan,
/// pass them to OptVLS interface and store result internally, so there's no
/// need to recollect and recompute grouping information for same HCFG and VF.
class VPlanVLSAnalysis {
protected:
  enum class MemAccessTy {
    Unknown,
    Uniform,
    Strided,
    Indexed,
  };

  /// The loop that we are vectorizing.
  const Loop *MainLoop;

  // FIXME: With Type information this context is not needed. Moreover,
  // it's here because it's easier to pass context downstream using
  // this class, rather then obtain the context from deep inside of
  // vectorizer.
  LLVMContext &Context;
  const DataLayout &DL;
  TargetTransformInfo *TTI;

  // Context that serves as backing storage for OptVLS data.
  OVLSContext OptVLSContext;

  /// Finds a group for a given VPInstruction.
  OVLSGroup *getGroupForInstruction(const VPlan *Plan,
                                    const VPInstruction *Inst) const {
    const VLSInfo &VlsInfoForVPlan = Plan2VLSInfo.find(Plan)->second;
    const auto *MemrefIt = llvm::find_if(
        VlsInfoForVPlan.Memrefs, [Inst](const OVLSMemref *Memref) {
          return cast<VPVLSClientMemref>(Memref)->getInstruction() == Inst;
        });
    if (MemrefIt != VlsInfoForVPlan.Memrefs.end()) {
      auto GIt = VlsInfoForVPlan.Mem2Group.find(*MemrefIt);
      if (GIt != VlsInfoForVPlan.Mem2Group.end())
        return GIt->second;
    }
    return nullptr;
  }

  virtual OVLSMemref *createVLSMemref(const VPLoadStoreInst *Inst,
                                      const unsigned VF,
                                      const VPlanScalarEvolution *VPSE);

private:
  void collectMemrefs(OVLSMemrefVector &MemrefVector, const VPlan *Plan,
                      unsigned VF);

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

    void erase() {
      Memrefs.clear();
      Groups.clear();
      Mem2Group.clear();
    }
  };

  SmallDenseMap<const VPlan *, VLSInfo> Plan2VLSInfo;

public:
  VPlanVLSAnalysis(const Loop *MainLoop, LLVMContext &Context,
                   const DataLayout &DL, TargetTransformInfo *TTI)
      : MainLoop(MainLoop), Context(Context), DL(DL), TTI(TTI) {}
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

  const OVLSGroupVector &groups(const VPlan *Plan) {
    return Plan2VLSInfo[Plan].Groups;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(const VPlan *Plan) const;
  void dump() const;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  const Loop *getMainLoop() const { return MainLoop; }
  LLVMContext &getContext() const { return Context; }
  OVLSContext &getOVLSContext() { return OptVLSContext; }
  const DataLayout &getDL() const { return DL; }
};

int computeInterleaveIndex(const OVLSMemref *Memref, OVLSGroup *Group);
int computeInterleaveFactor(const OVLSMemref *Memref);

inline const VPLoadStoreInst *instruction(const OVLSMemref *Memref) {
  return cast<VPLoadStoreInst>(
      cast<VPVLSClientMemref>(Memref)->getInstruction());
}

inline auto instructions(OVLSGroup *Group) {
  return map_range(*Group,
                   [](OVLSMemref *Memref) { return instruction(Memref); });
}

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLANVLSANALYSIS_H

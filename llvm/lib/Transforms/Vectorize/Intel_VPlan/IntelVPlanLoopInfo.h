//===-- IntelVPlanLoopInfo.h ------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines VPLoopInfo analysis and VPLoop class. VPLoopInfo is a
/// specialization of LoopInfoBase for VPBlockBase. VPLoops is a specialization
/// of LoopBase that is used to hold loop metadata from VPLoopInfo. Further
/// information can be found in VectorizationPlanner.rst.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANLOOPINFO_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANLOOPINFO_H

#include "llvm/Analysis/LoopInfoImpl.h"

namespace llvm {
namespace vpo {
class VPBlockBase;
class VPBasicBlock;
class VPValue;
class VPInstruction;
class VPlanDivergenceAnalysis;
class VPLoop;

struct TripCountInfo {
  using TripCountTy = uint64_t;
  TripCountTy MinTripCount = 0;
  static constexpr TripCountTy UnknownMaxTripCount =
      // unsigned is due to inconsistency in interfaces we use to get known
      // trip count vs. get maximum trip count.
      std::numeric_limits<unsigned>::max();
  TripCountTy MaxTripCount = UnknownMaxTripCount;
  TripCountTy TripCount = 0;
  bool IsEstimated = true;

  void calculateEstimatedTripCount();
  static TripCountInfo getKnownTripCountInfo(TripCountTy TripCount) {
    return {TripCount, TripCount, TripCount, false};
  }
};

/// VPLoopInfo provides analysis of natural loop for VPBlockBase-based
/// Hierarchical CFG. It is a specialization of LoopInfoBase class.
typedef LoopInfoBase<VPBlockBase, VPLoop> VPLoopInfo;

/// A VPLoop holds analysis information for every loop detected by VPLoopInfo.
/// It is an instantiation of LoopBase.
class VPLoop : public LoopBase<VPBlockBase, VPLoop> {
private:
  friend class LoopInfoBase<VPBlockBase, VPLoop>;
  explicit VPLoop() : LoopBase<VPBlockBase, VPLoop>() {}
  explicit VPLoop(VPBlockBase *VPB) : LoopBase<VPBlockBase, VPLoop>(VPB) {}

public:
  bool isLiveIn(const VPValue* VPVal) const;
  bool isLiveOut(const VPValue* VPVal) const;

  using LoopBase<VPBlockBase, VPLoop>::contains;

  // LoopBase doesn't expect different types of VPBlockBase's and treats
  // VPBasicBlock as a template type parameter for an Instruction version. Need
  // to process these manually.
  bool contains(const VPBasicBlock *BB) const;
  // LoopBase's contains isn't virtual so its I->getParent can't call our
  // overload, have to re-implement it too.
  bool contains(const VPInstruction *I) const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printRPOT(raw_ostream &OS, const VPLoopInfo *VPLI = nullptr,
                 unsigned Indent = 0,
                 const VPlanDivergenceAnalysis *DA = nullptr) const;

  LLVM_DUMP_METHOD void dump() const { print(dbgs()); }
  LLVM_DUMP_METHOD void dumpVerbose() const {
    print(dbgs(), /*Depth=*/0, /*Verbose=*/true);
  }
  LLVM_DUMP_METHOD void dumpRPOT() const { printRPOT(dbgs()); };
  LLVM_DUMP_METHOD void dumpRPOT(const VPLoopInfo *VPLI) const {
    printRPOT(dbgs(), VPLI);
  };
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  using TripCountTy = TripCountInfo::TripCountTy;
  void setTripCountInfo(TripCountInfo TCInfo);
  TripCountInfo getTripCountInfo();
  void setKnownTripCount(TripCountTy TripCount) {
    setTripCountInfo(TripCountInfo::getKnownTripCountInfo(TripCount));
  }
};
} // namespace vpo

template <> struct GraphTraits<vpo::VPLoop *> {
  using NodeRef = vpo::VPLoop *;
  using ChildIteratorType = vpo::VPLoopInfo::iterator;

  static NodeRef getEntryNode(NodeRef N) { return N; }

  static inline ChildIteratorType child_begin(NodeRef N) {
    return N->begin();
  }

  static inline ChildIteratorType child_end(NodeRef N) {
    return N->end();
  }
};

template <> struct GraphTraits<const vpo::VPLoop *> {
  using NodeRef = const vpo::VPLoop *;
  using ChildIteratorType = vpo::VPLoopInfo::iterator;

  static NodeRef getEntryNode(NodeRef N) { return N; }

  static inline ChildIteratorType child_begin(NodeRef N) {
    return N->begin();
  }

  static inline ChildIteratorType child_end(NodeRef N) {
    return N->end();
  }
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANLOOPINFO_H

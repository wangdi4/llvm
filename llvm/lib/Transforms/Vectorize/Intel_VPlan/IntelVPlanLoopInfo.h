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
/// specialization of LoopInfoBase for VPBasicBlock. VPLoops is a specialization
/// of LoopBase that is used to hold loop metadata from VPLoopInfo. Further
/// information can be found in VectorizationPlanner.rst.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANLOOPINFO_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANLOOPINFO_H

#include "llvm/Analysis/Intel_OptReport/OptReport.h"
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
#include "llvm/Analysis/LoopInfoImpl.h"

namespace llvm {
namespace vpo {
class VPBasicBlock;
class VPValue;
class VPInstruction;
class VPCmpInst;
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

/// VPLoopInfo provides analysis of natural loop for VPlan. It is a
/// specialization of LoopInfoBase class.
class VPLoopInfo;
class VPDominatorTree;

/// A VPLoop holds analysis information for every loop detected by VPLoopInfo.
/// It is an instantiation of LoopBase.
class VPLoop : public LoopBase<VPBasicBlock, VPLoop> {
private:
  friend class LoopInfoBase<VPBasicBlock, VPLoop>;
  explicit VPLoop() : LoopBase<VPBasicBlock, VPLoop>() {}
  explicit VPLoop(VPBasicBlock *VPB) : LoopBase<VPBasicBlock, VPLoop>(VPB) {}

public:
  // Return true if \p VPVal is defined outside of the loop. Constants and
  // MetadataAsValue don't have any def so the function returns false for them.
  bool isDefOutside(const VPValue *VPVal) const;

  // Return true if \p VPVal is liveout in the loop, i.e. it is defined inside
  // the loop and has a use outside.
  bool isLiveOut(const VPInstruction *VPVal) const;

  bool isLCSSAForm() const;
  bool isRecursivelyLCSSAForm(const VPLoopInfo &LI) const;

  /// Return pair: a value which represents upper bound of the loop and
  /// the latch compare instruction. The upper bound value is one of the compare
  /// instruction operands.
  /// The argument \p AssumeNormalizedIV tells whether we honor that the
  /// original loop must have normalized IV. When set to true that effectively
  /// means that normalized IV code has been added to VPlan but the original
  /// loop does not necessarily have IV normalized, so that we need to bypass
  /// that assertion.
  std::pair<VPValue *, VPCmpInst *>
  getLoopUpperBound(bool AssumeNormalizedIV = false) const;

  /// Return the comparison used for loop latch condition.
  /// If not found, return nullptr.
  VPCmpInst *getLatchComparison() const;

  /// Return the loop id metadata node for this VPLoop if present. This is
  /// obtained by checking loop latch's terminator instruction.
  // TODO: Do we need a setter for LoopID?
  MDNode *getLoopID() const;

  /// Return true if the loop has normalized induction.
  bool hasNormalizedInduction() const {
    assert(HasNormalizedInduction.has_value() && "The flag is unset");
    return HasNormalizedInduction.value();
  }

  /// Return true if the loop trip count is equal to upper bound.
  bool exactUB() const {
    assert(HasNormalizedInduction.has_value() && "The flag is unset");
    return ExactUB;
  }


  void setHasNormalizedInductionFlag(bool Val, bool EUB) {
    assert(!HasNormalizedInduction.has_value() && "The flag is already set");
    HasNormalizedInduction = Val;
    ExactUB = EUB;
  }

  void copyHasNormalizedInductionFlag(const VPLoop* L) {
    HasNormalizedInduction = L->HasNormalizedInduction;
    ExactUB = L->ExactUB;
  }

  OptReport getOptReport() const { return OR; }
  void setOptReport(OptReport R) { OR = R; }
  void eraseOptReport() { OR = nullptr; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printRPOT(raw_ostream &OS, const VPLoopInfo *VPLI = nullptr,
                 unsigned Indent = 0) const;

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
  TripCountInfo getTripCountInfo() const;
  void setKnownTripCount(TripCountTy TripCount) {
    setTripCountInfo(TripCountInfo::getKnownTripCountInfo(TripCount));
  }

private:
  Optional<bool> HasNormalizedInduction;
  // Flag indicating how the loop iteration count is related to the
  // upper bound (invariant operand of the latch condition). False means
  // trip count is equal to upper bound + 1, true means trip count is
  // equal to upper bound exactly.
  bool ExactUB = true;
  // Track opt-report remarks for this VPLoop.
  OptReport OR = nullptr;
};
class VPLoopInfo : public LoopInfoBase<VPBasicBlock, VPLoop> {
  using Base = LoopInfoBase<VPBasicBlock, VPLoop>;

  // Remove interface from public. We prohibit VPLoop's recalculating because
  // they are used as keys in multiple maps. That is needed because we want to
  // decouple CFG from the VPlan, so loop-specific VPlan data needs some map
  // key. VPloop seems to be the best candidate so far.
  using Base::releaseMemory;
public:
  void analyze(const VPDominatorTree &DomTree);
};
} // namespace vpo

template <> struct GraphTraits<vpo::VPLoop *> {
  using NodeRef = vpo::VPLoop *;
  using ChildIteratorType = vpo::VPLoopInfo::iterator;

  static NodeRef getEntryNode(NodeRef N) { return N; }

  static inline ChildIteratorType child_begin(NodeRef N) { return N->begin(); }

  static inline ChildIteratorType child_end(NodeRef N) { return N->end(); }
};

template <> struct GraphTraits<const vpo::VPLoop *> {
  using NodeRef = const vpo::VPLoop *;
  using ChildIteratorType = vpo::VPLoopInfo::iterator;

  static NodeRef getEntryNode(NodeRef N) { return N; }

  static inline ChildIteratorType child_begin(NodeRef N) { return N->begin(); }

  static inline ChildIteratorType child_end(NodeRef N) { return N->end(); }
};

// Traits of VPLoop for OptReportBuilder.
template <> struct OptReportTraits<vpo::VPLoop> {
  using ObjectHandleTy = std::pair<vpo::VPLoop &, vpo::VPLoopInfo &>;

  static OptReport getOptReport(const ObjectHandleTy &Handle) {
    return Handle.first.getOptReport();
  }

  static void setOptReport(const ObjectHandleTy &Handle, OptReport OR) {
    Handle.first.setOptReport(OR);
  }

  static void eraseOptReport(const ObjectHandleTy &Handle) {
    Handle.first.eraseOptReport();
  }

  static DebugLoc getDebugLoc(const ObjectHandleTy &Handle) {
    // TODO: Missing DbgLoc for VPLoops.
    return DebugLoc();
  }

  static Optional<std::string> getOptReportTitle(const ObjectHandleTy &Handle) {
    return None;
  }

  static OptReport getOrCreatePrevOptReport(const ObjectHandleTy &Handle,
                                            const OptReportBuilder &Builder) {
    auto &L = Handle.first;
    vpo::VPLoop *PrevSiblingLoop = nullptr;

    if (L.getParentLoop())
      for (auto *ChildLoop : L.getParentLoop()->getSubLoops()) {
        if (ChildLoop == &L)
          break;

        PrevSiblingLoop = ChildLoop;
      }
    else {
      auto &LI = Handle.second;

      for (vpo::VPLoopInfo::reverse_iterator I = LI.rbegin(), E = LI.rend();
           I != E; ++I) {
        if (*I == &L)
          break;

        PrevSiblingLoop = *I;
      }
    }

    if (!PrevSiblingLoop)
      return nullptr;

    return Builder(*PrevSiblingLoop, Handle.second).getOrCreateOptReport();
  }

  static OptReport getOrCreateParentOptReport(const ObjectHandleTy &Handle,
                                              const OptReportBuilder &Builder) {
    auto &L = Handle.first;
    // Attach to the parent Loop, if it exists.
    if (vpo::VPLoop *Dest = L.getParentLoop())
      return Builder(*Dest, Handle.second).getOrCreateOptReport();

    // TODO: Extend VPlan to be the parent of outermost VPLoop.
    llvm_unreachable("Failed to find a parent.");
  }

  using ChildNodeTy = vpo::VPLoop;
  using ChildHandleTy = typename OptReportTraits<ChildNodeTy>::ObjectHandleTy;
  using NodeVisitorTy = std::function<void(ChildHandleTy)>;
  static void traverseChildNodesBackward(const ObjectHandleTy &Handle,
                                         NodeVisitorTy Func) {
    auto &L = Handle.first;
    std::for_each(L.rbegin(), L.rend(), [&Handle, &Func](vpo::VPLoop *CL) {
      Func(ChildHandleTy(*CL, Handle.second));
    });
  }
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANLOOPINFO_H

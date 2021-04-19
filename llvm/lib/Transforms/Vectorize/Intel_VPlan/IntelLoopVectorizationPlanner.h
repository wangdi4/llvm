//===-- LoopVectorizationPlanner.h ------------------------------*- C++ -*-===//
//
//   Copyright (C) 2016-2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines LoopVectorizationPlanner.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELLOOPVECTORIZATIONPLANNER_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELLOOPVECTORIZATIONPLANNER_H

#if INTEL_CUSTOMIZATION
#include "IntelVPlan.h"
#include "IntelVPlanLoopUnroller.h"
#else
#include "VPlan.h"
#endif
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallSet.h"

namespace llvm {
class Loop;
class ScalarEvolution;
class TargetLibraryInfo;
class TargetTransformInfo;

namespace loopopt {
class HLLoop;
class DDGraph;
}

using namespace llvm::loopopt;

namespace vpo {
#if INTEL_CUSTOMIZATION
class VPlanVLSAnalysis;
class WRNVecLoopNode;
#endif
class VPOCodeGen;
class VPOVectorizationLegality;
class WRNVecLoopNode;
class VPlanHCFGBuilder;
class VPlanCostModel;
class VPlanRemainderEvaluator;
class VPlanPeelEvaluator;
class VPlanCFGMerger;
class CfgMergerPlanDescr;

extern bool PrintSVAResults;
extern bool PrintAfterCallVecDecisions;
extern bool LoopMassagingEnabled;
extern bool EnableSOAAnalysis;
extern bool EnableNewCFGMerge;

/// Auxiliary class to keep vectorization scenario for a single loop
/// vectorization. It describes which variants of the loops are selected for
/// peel, remainder, and main loop. For peel we can select either scalar or
/// masked vector loop or none of them, for remainder we can have four
/// variants: none, scalar, masked vector, unmasked vector plus scalar. For main
/// loop we can select either unmasked or masked vector variants. All vector
/// variants can have their own VF, the main loop can have also an unroll
/// factor. The selection is done by cost model and trip count analysis.
class SingleLoopVecScenario {
public:
  friend class LoopVectorizationPlanner; // to use set() methods

  enum AuxLoopKind {
    LKNone = 0,
    LKScalar,
    LKMasked,
    LKVector,
  };

  struct AuxLoopDescr {
    AuxLoopKind Kind = LKNone;
    unsigned VF = 0;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    void print(raw_ostream &OS) const {
      switch (Kind) {
      case LKNone:
        OS << "none";
        break;
      case LKScalar:
        OS << "scalar";
        break;
      case LKMasked:
        OS << "masked, VF=" << VF;
        break;
      case LKVector:
        OS << "unmasked, VF=" << VF;
        break;
      }
    }
    friend raw_ostream &operator<<(raw_ostream &OS, const AuxLoopDescr &D);
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  };

  const AuxLoopDescr &getMain() const { return Main; }
  const AuxLoopDescr &getPeel() const { return Peel; }
  auto rem_begin() const { return Remainders.begin(); }
  auto rem_end() const { return Remainders.end(); }
  unsigned getMainUF() const { return MainUF; }

  AuxLoopKind getMainKind() const { return Main.Kind; }
  unsigned getMainVF() const { return Main.VF; }
  AuxLoopKind getPeelKind() const { return Peel.Kind; }
  unsigned getPeelVF() const { return Peel.VF; }

  /// Convenience methods
  bool hasPeel() const { return Peel.Kind != LKNone; }
  bool hasMaskedPeel() const { return Peel.Kind == LKMasked; }

  /// Return list of vector factors used for the current selection.
  void getUsedVFs(SmallSet<unsigned, 4> &Ret) {
    Ret.insert(Main.VF);
    if (Peel.Kind != LKNone)
      Ret.insert(Peel.VF);
    for (auto &Rem : Remainders)
      if (Rem.Kind != LKNone)
        Ret.insert(Rem.VF);
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const { dump(dbgs()); }
  void dump(raw_ostream &OS) const {
    OS << "Single loop scenario:\n";
    OS << " MainLoop: " << Main << "\n";
    OS << " PeelLoop: " << Peel << "\n";
    OS << " Remainders: ";
    if (!Remainders.size())
      OS << "none";
    else
      for (auto &Rem : Remainders)
        OS << Rem << ",";
    OS << "\n";
  }

  // For debug purposes, allow setting from a string.
  // The supported string format is:
  //  Spec:= <Peel>;<Main>;<Remainder>
  //  Peel:= <Loop>
  //  Main:= <Loop>
  //  Remainder:= <Loops>
  //  Loops:= <Loop> {<Loops>}
  //  Loop := <LoopKind><VF>
  //  VF := a positive number
  //  LoopKind := n | s | v | m
  //    n := none, means no loop
  //    s := scalar
  //    v := non-masked vector
  //    m := masked vector
  // E.g."n0;v8;v4s1" means "no peel, unmasked vector main loop with VF=8,
  // unmasked vector remainder with VF=4 and scalar remainder".
  void fromString(StringRef S);
#endif // !NDEBUG || LLVM_ENABLE_DUMP

private:
  void setMainUF(unsigned N) {MainUF = N;}
  void resetRemainders() { Remainders.clear(); }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Keep it for debug purpose only (used by fromString())
  void addRemainder(const AuxLoopDescr RD) { Remainders.emplace_back(RD); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  void resetPeel() { Peel = {LKNone, 0}; }
  void resetMain() {
    Main = {LKScalar, 1};
    MainUF = 1;
  }
  void setScalarPeel() { Peel = {LKScalar, 1}; }
  void setMaskedPeel(unsigned VF) { Peel = {LKMasked, VF}; }

  void addScalarRemainder() {
    Remainders.emplace_back(AuxLoopDescr{LKScalar, 1});
  }
  void addUnmaskedRemainder(unsigned VF) {
    Remainders.emplace_back(AuxLoopDescr{LKVector, VF});
  }
  void addMaskedRemainder(unsigned VF) {
    Remainders.emplace_back(AuxLoopDescr{LKMasked, VF});
  }
  void setVectorMain(unsigned VF, unsigned UF) {
    Main = {LKVector, VF};
    MainUF = UF;
  }
  void setMaskedMain(unsigned VF) {
    Main = {LKMasked, VF};
    MainUF = 1;
  }


  AuxLoopDescr Main;
  AuxLoopDescr Peel;
  SmallVector<AuxLoopDescr, 1> Remainders;
  unsigned MainUF = 1; // Unroll factor of the main loop.
};

inline raw_ostream &operator<<(raw_ostream &OS,
                               const SingleLoopVecScenario::AuxLoopDescr &D) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  D.print(OS);
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  return OS;
}

// Auxiliary class to describe VPlan for CFG merge.
// After selection of vectorization scenario, the list of the desriptors
// is created and CFGMerger is run on that list, creating merged CFG.
class CfgMergerPlanDescr {
public:
  enum LoopType {
    LTRemainder,
    LTMain,
    LTPeel,
  };
  CfgMergerPlanDescr(LoopType LT, unsigned F, VPlan *P)
      : Type(LT), VF(F), Plan(P) {}


  VPlan *getVPlan() const { return Plan; }
  unsigned getVF() const { return VF; }

  LoopType getLoopType() const { return Type; }

private:
  LoopType Type; // Loop-type.
  unsigned VF;   // vector factor
  VPlan *Plan;   // VPlan

  // Basic blocks used during merge
  VPBasicBlock *AdapterBB = nullptr;
  VPBasicBlock *PrevMerge = nullptr;
  VPBasicBlock *MergeBefore = nullptr;

  friend class VPlanCFGMerger;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const { dump(dbgs()); }
  void dump(raw_ostream &OS) const {
    auto getLoopTypeName = [](LoopType LT) -> StringRef {
      switch (LT) {
      case LTRemainder:
        return "remainder";
      case LTMain:
        return "main";
      case LTPeel:
        return "peel";
      };
      return "";
    };
    OS << "VPlan: " << Plan->getName() << "\n";
    OS << "  Kind: " << getLoopTypeName(Type) << " VF:" << VF << "\n";
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

/// LoopVectorizationPlanner - builds and optimizes the Vectorization Plans
/// which record the decisions how to vectorize the given loop.
/// In particular, represent the control-flow of the vectorized version,
/// the replication of instructions that are to be scalarized, and interleave
/// access groups.
class LoopVectorizationPlanner {
  friend class VPlanTestBase;

public:
#if INTEL_CUSTOMIZATION
  LoopVectorizationPlanner(WRNVecLoopNode *WRL, Loop *Lp, LoopInfo *LI,
                           const TargetLibraryInfo *TLI,
                           const TargetTransformInfo *TTI, const DataLayout *DL,
                           class DominatorTree *DT,
                           VPOVectorizationLegality *Legal,
                           VPlanVLSAnalysis *VLSA)
      : WRLp(WRL), TLI(TLI), TTI(TTI), DL(DL), Legal(Legal), TheLoop(Lp),
        LI(LI), DT(DT), VLSA(VLSA) {}
#endif // INTEL_CUSTOMIZATION

  virtual ~LoopVectorizationPlanner() {}
  /// Build initial VPlans according to the information gathered by Legal
  /// when it checked if it is legal to vectorize this loop.
  /// Returns the number of VPlans built, zero if failed.
  unsigned buildInitialVPlans(MDNode *MD, LLVMContext *Context,
                              const DataLayout *DL, std::string VPlanName,
                              ScalarEvolution *SE = nullptr);


  /// On VPlan construction, each instruction marked for predication by Legal
  /// gets its own basic block guarded by an if-then. This initial planning
  /// is legal, but is not optimal. This function attempts to leverage the
  /// necessary conditional execution of the predicated instruction in favor
  /// of other related instructions. The function applies these optimizations
  /// to all VPlans.
  // void optimizePredicatedInstructions();

  /// Select the best peeling variant for every VPlan.
  void selectBestPeelingVariants();

  /// Record CM's decision and dispose of all other VPlans.
  // void setBestPlan(unsigned VF, unsigned UF);

  // Preprocess best VPlan before CG, creating the needed auxiliary loops
  // (peel/remainder of different kinds) and merging them into flattened
  // cfg. \p UF and \p VF are selected unroll factor and vector factor,
  // correspondingly, for main VPlan.
  void emitPeelRemainderVPLoops(unsigned VF, unsigned UF);

  // Create VPlans that are needed for CFG merge by the selected scenario.
  void createMergerVPlans(VPAnalysesFactory &VPAF);

  /// Generate the IR code for the body of the vectorized loop according to the
  /// best selected VPlan.
  void executeBestPlan(VPOCodeGen &LB);

  /// Feed information from explicit clauses to the loop Legality.
  /// This information is necessary for initial loop analysis in the CodeGen.
#if INTEL_CUSTOMIZATION
  template <class VPOVectorizationLegality>
#endif
  static void EnterExplicitData(WRNVecLoopNode *WRLp, VPOVectorizationLegality &Legal);

  /// Post VPlan FrontEnd pass to verify that we can process the VPlan that
  /// was constructed. There are some limitations in CG, CM, and other parts of
  /// VPlan vectorizer on which we better gracefully bail out than assert.
  bool canProcessVPlan(const VPlanVector &Plan);

  /// Select the best plan and dispose all other VPlans.
  /// \Returns the selected vectorization factor and corresponding VPlan.
  template <typename CostModelTy = VPlanCostModel>
  std::pair<unsigned, VPlanVector *> selectBestPlan();

  /// \Returns the VPlan for selected best vectorization factor.
  VPlanVector *getBestVPlan();

  /// \Returns the selected best vectorization factor.
  unsigned getBestVF() {return VecScenario.getMainVF();}

  /// \Returns the selected best unroll factor.
  unsigned getBestUF() {return VecScenario.getMainUF();}

  /// Extracts VFs from "llvm.loop.vector.vectorlength" metadata
  void extractVFsFromMetadata(MDNode *MD, unsigned SafeLen);

  /// Returns vector of allowed VFs
  ArrayRef<unsigned> getVectorFactors();

  /// Predicate all unique non-scalar VPlans
  void predicate(void);

  /// Insert all-zero bypasses for \p Plan.
  void insertAllZeroBypasses(VPlanVector *Plan, unsigned VF);

  /// Return Loop Unroll Factor either forced by option or pragma
  /// or advised by optimizations.
  /// \p Forced indicates that Unroll Factor is forced.
  virtual unsigned getLoopUnrollFactor(bool *Forced = nullptr);

  /// Perform VPlan loop unrolling if needed
  void
  unroll(VPlanVector &Plan,
         VPlanLoopUnroller::VPInstUnrollPartTy *VPInstUnrollPart = nullptr);

  template <typename CostModelTy = VPlanCostModel>
  void printCostModelAnalysisIfRequested(const std::string &Header);

  virtual bool isNewCFGMergeEnabled() const { return EnableNewCFGMerge; }

  /// Generate the IR code for the body of the vectorized loop according to the
  /// best selected VPlan.
  // void executeBestPlan(InnerLoopVectorizer &LB);

  /// Return non-masked variant of VPlan for given VF.
  VPlanVector *getVPlanForVF(unsigned VF) const {
    auto It = VPlans.find(VF);
    return It != VPlans.end() ? It->second.MainPlan.get() : nullptr;
  }

  /// Return masked variant of VPlan for given VF.
  VPlanMasked *getMaskedVPlanForVF(unsigned VF) const {
    auto It = VPlans.find(VF);
    return It != VPlans.end() ? It->second.MaskedModeLoop.get() : nullptr;
  }

  bool hasVPlanForVF(const unsigned VF) const { return VPlans.count(VF) != 0; }

  auto getAllVPlans() const {
    return make_range(VPlans.begin(), VPlans.end());
  }

  struct VPlanPair {
    std::shared_ptr<VPlanVector> MainPlan;
    std::shared_ptr<VPlanMasked> MaskedModeLoop;
  };

  void appendVPlanPair(unsigned VF, const VPlanPair &PlanPair) {
    VPlans[VF] = PlanPair;
  }

  VPlan *addAuxiliaryVPlan(VPlan &Plan) {
    AuxVPlans.push_back(std::unique_ptr<VPlan>(&Plan));
    return AuxVPlans.back().get();
  }

  // Return the list-range of VPlans created by the merger to the clients.
  inline decltype(auto) mergerVPlans() const {
    return make_range(MergerVPlans.begin(), MergerVPlans.end());
  }

  /// Returns true if the loop has normalized induction:
  /// - the main induction is integer
  /// - the induction is incremented with step 1
  /// - start value is 0
  /// - upper bound is invariant
  /// - the update instruction is used only in latch condition and
  ///   in the header phi
  /// - the latch condition is used only as back-edge condition.
  /// - the latch condition and back edge are in a form that allows
  ///   us to use the upper bound as the loop trip count.
  ///   One example of normalized case:
  ///     cond = cmp lt iv_incr, ub
  ///     br cond loopheader, loopexit
  ///   One example of non-normalized case:
  ///     cond = cmp le iv_incr, ub
  ///     br cond loopheader, loopexit
  static bool hasLoopNormalizedInduction(const VPLoop *Loop);

protected:
  /// Build an initial VPlan according to the information gathered by Legal
  /// when it checked if it is legal to vectorize this loop. \return a VPlan
  /// that corresponds to vectorization factors starting from the given
  /// \p StartRangeVF and up to \p EndRangeVF, exclusive, possibly decreasing
  /// the given \p EndRangeVF.
  // TODO: If this function becomes more complicated, move common code to base
  // class.
  virtual std::shared_ptr<VPlanVector>
  buildInitialVPlan(VPExternalValues &Ext,
                    VPUnlinkedInstructions &UnlinkedVPInsts,
                    std::string VPlanName, ScalarEvolution *SE = nullptr);

  /// If FoorcedVF if specified, puts it into vector of VFs. Else if
  /// "llvm.loop.vector.vectorlength" metadata is specified, fills vector of VFs
  /// with values from metadata. Else if "llvm.loop.vector.vectorlength"
  /// metadata is not specified, defines MinVF and MaxVF and fills vector of VFs
  /// with default vector values between MinVF and MaxVF.
  int setDefaultVectorFactors(MDNode *MD);

  /// Transform to emit explict uniform Vector loop iv.
  virtual void emitVecSpecifics(VPlanVector *Plan);

  /// \Returns a pair of the <min, max> types' width used in the underlying loop.
  /// Doesn't take into account i1 type.
  virtual std::pair<unsigned, unsigned> getTypesWidthRangeInBits() const;

  /// Create VPLiveIn/VPLiveOut lists for VPEntities.
  virtual void createLiveInOutLists(VPlanVector &Plan);

  /// Check whether everything in the loop body is supported at the moment.
  /// We can have some unimplemented things and it's better to gracefully
  /// bailout in such cases than assert or generate incorrect code.
  virtual bool canProcessLoopBody(const VPlanVector &Plan,
                                  const VPLoop &Loop) const;

  /// Register the choosen vectorization scenario: peel/remainder configuration,
  /// vector and unroll factors for main loop
  void updateVecScenario(VPlanPeelEvaluator const &PE,
                         VPlanRemainderEvaluator const &RE, unsigned VF,
                         unsigned UF);

  /// Select simplest vectorization scenario: no peel, non-masked main loop with
  /// specified vector and unroll factors, scalar remainder.
  void selectSimplestVecScenario(unsigned VF, unsigned UF);

  /// WRegion info of the loop we evaluate. It can be null.
  WRNVecLoopNode *WRLp;

  /// Target Library Info.
  const TargetLibraryInfo *TLI;

  /// Target Transform Info.
  const TargetTransformInfo *TTI;

  /// Data Layout
  const DataLayout *DL;

  /// The legality analysis.
  // TODO: Turn into a reference when supported for HIR.
  LoopVectorizationLegality *Legal;

  /// This class is copied from open-source LoopVectorize.cpp and it's supposed
  /// to be temporal. VPO doesn't need it but we have it to minimize divergency
  /// with TransformState.
  struct VPCallbackILV : public VPCallback {

    ~VPCallbackILV() override {}

    Value *getOrCreateVectorValues(Value *V, unsigned Part) override {
      llvm_unreachable("Not implemented");
      return nullptr;
    }
  };
  SmallVector<unsigned, 5> VFs;
  SingleLoopVecScenario VecScenario;

  // Storage for common external data (VPExternalDefs, Uses, Consts etc).
  std::unique_ptr<VPExternalValues> Externals;

  // Storage for VPInstructions that have been removed from VPlan and unlinked.
  std::unique_ptr<VPUnlinkedInstructions> UnlinkedVPInsts;

private:
  /// Determine whether \p I will be scalarized in a given range of VFs.
  /// The returned value reflects the result for a prefix of the range, with \p
  /// EndRangeVF modified accordingly.
  // bool willBeScalarized(Instruction *I, unsigned StartRangeVF,
  //                      unsigned &EndRangeVF);

  /// Iteratively sink the scalarized operands of a predicated instruction into
  /// the block that was created for it.
  // void sinkScalarOperands(Instruction *PredInst, VPlan *Plan);

  void runInitialVecSpecificTransforms(VPlanVector *Plan);

  /// Main function that canonicalizes the CFG and applyies loop massaging
  /// transformations like mergeLoopExits transform.
  void doLoopMassaging(VPlanVector *Plan);

  /// Use results from VPEntity analysis to emit explicit VPInstruction-based
  /// representation. The analysis results can be invalidated/stale after this
  /// transform.
  void emitVPEntityInstrs(VPlanVector *Plan);

  /// Emit uniform IV for the vector loop and rewrite backedge condition to use
  /// it.
  //
  //   header:
  //     %iv = phi [ 0, preheader ], [ iv.next, latch ]
  //
  //   latch:
  //     %iv.next = %iv + VF
  //     %cond = %iv.next `icmp` TripCount
  //     br i1 %cond
  //
  // The order of latch's successors isn't changed which is ensured by selecting
  // proper icmp predicate (eq/ne). Original latch's CondBit is erased if there
  // are no remaining uses of it after the transformation above.
  void emitVectorLoopIV(VPlanVector *Plan, VPValue *TripCount, VPValue *VF);

  /// Utility to dump and verify VPlan details after initial set of transforms.
  void printAndVerifyAfterInitialTransforms(VPlan *Plan);

  /// The loop that we evaluate.
  Loop *TheLoop;

  /// Loop Info analysis.
  LoopInfo *LI;

  /// The dominators tree.
  class DominatorTree *DT;

#if INTEL_CUSTOMIZATION
  /// VPlan VLS Analysis.
  VPlanVLSAnalysis *VLSA;
#endif // INTEL_CUSTOMIZATION

  /// The profitablity analysis.
  // LoopVectorizationCostModel *CM;

  // TODO: Move to base class
  VPOCodeGen *ILV = nullptr;

  // InnerLoopVectorizer *ILV = nullptr;

  /// VPlans are shared between VFs, use smart pointers.
  DenseMap<unsigned, VPlanPair> VPlans;

  /// A list of other additional VPlans, created during peel/remainders
  /// creation and cloning.
  SmallVector<std::unique_ptr<VPlan>, 2> AuxVPlans;
  std::list<CfgMergerPlanDescr> MergerVPlans;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELLOOPVECTORIZATIONPLANNER_H

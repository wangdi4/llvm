//===-- IntelLoopVectorizationPlanner.h -------------------------*- C++ -*-===//
//
//   Copyright (C) 2016 Intel Corporation. All rights reserved.
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

#include "Driver.h"
#include "IntelVPlan.h"
#include "IntelVPlanLoopUnroller.h"
#include "IntelVPlanVerifier.h"
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
class VPlanVLSAnalysis;
class WRNVecLoopNode;
class CodeGenLLVM;
class LegalityLLVM;
class WRNVecLoopNode;
class VPlanHCFGBuilder;
class VPlanRemainderEvaluator;
class VPlanPeelEvaluator;
class VPlanCFGMerger;
class CfgMergerPlanDescr;

extern bool PrintSVAResults;
extern bool PrintAfterCallVecDecisions;
extern bool LoopMassagingEnabled;
extern bool EnableSOAAnalysis;
extern bool EnableSOAAnalysisHIR;
extern unsigned DefaultTripCount;
// Flag to indicate if dynamic peeling is enabled. Flag is set based on
// appropriate value of command line option for the IR kind being processed.
extern bool VPlanEnablePeeling;
// Flag to indicate if peeling is enabled in general, including static peeling.
extern bool VPlanEnableGeneralPeeling;
extern bool EnableIntDivRemBlendWithSafeValue;

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
  friend class LoopVectorizationPlannerHIR; // to use set() methods

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
  auto remainders() const { return make_range(rem_begin(), rem_end()); }
  unsigned getMainUF() const { return MainUF; }

  AuxLoopKind getMainKind() const { return Main.Kind; }
  unsigned getMainVF() const { return Main.VF; }
  AuxLoopKind getPeelKind() const { return Peel.Kind; }
  unsigned getPeelVF() const { return Peel.VF; }

  /// Convenience methods
  bool hasPeel() const { return Peel.Kind != LKNone; }
  bool hasMaskedPeel() const { return Peel.Kind == LKMasked; }
  bool hasRemainder() const { return !Remainders.empty(); }
  bool isMainMasked() const { return Main.Kind == LKMasked; }

  uint64_t getMinimumProfitablePeelTC() const {
    return MinimumProfitablePeelTC;
  }

  uint64_t getMinProfitableMaskedRemTC() const {
    return MinProfitableMaskedRemTC;
  }

  /// Simple main vector loop and scalar remainder scenario of a
  /// a constant trip count loop. The main vector and scalar remainder
  /// loops can be added for such scenarios without any checks to see
  /// if they should be exercised.
  bool isSimpleConstTCScenario() {
    if (!IsConstTC)
      return false;
    if (Main.Kind != LKVector)
      return false;
    if (hasPeel())
      return false;
    if (!hasRemainder() || Remainders.size() > 1)
      return false;
    return Remainders[0].Kind == LKScalar;
  }

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
  void setIsConstTC(bool ConstTC) { IsConstTC = ConstTC; }
  void setMainUF(unsigned N) {MainUF = N;}
  void resetRemainders() {
    Remainders.clear();
    MinProfitableMaskedRemTC = 0;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Keep it for debug purpose only (used by fromString())
  void addRemainder(const AuxLoopDescr RD) { Remainders.emplace_back(RD); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  void resetPeel() {
    Peel = {LKNone, 0};
    MinimumProfitablePeelTC = 0;
  }
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
  void setMinimumProfitablePeelTC(uint64_t N) { MinimumProfitablePeelTC = N; }
  void setMinProfitableMaskedRemTC(uint64_t N) { MinProfitableMaskedRemTC = N; }

  AuxLoopDescr Main;
  AuxLoopDescr Peel;
  SmallVector<AuxLoopDescr, 1> Remainders;
  unsigned MainUF = 1; // Unroll factor of the main loop.

  // Is the loop we are dealing with a constant trip loop?
  bool IsConstTC = false;

  // For dynamic peeling: what is the minimum trip count for peeling to become
  // profitable?
  uint64_t MinimumProfitablePeelTC = 0;

  // Jump to a scalar remainder loop if runtime TC is less than this value.
  uint64_t MinProfitableMaskedRemTC = 0;
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
class CfgMergerPlanDescr : public VPlanLoopDescr {
public:
  using LoopType = VPlanLoopDescr::LoopType;

  CfgMergerPlanDescr(LoopType LT, unsigned VF, unsigned UF, VPlan *P)
      : VPlanLoopDescr(LT, VF, UF, isa<VPlanMasked>(P)), Plan(P) {}

  VPlan *getVPlan() const { return Plan; }

  bool isMaskedRemainder() const {
    return getLoopType() == LoopType::LTRemainder && isMasked();
  }

  bool isNonMaskedVecRemainder() const {
    return getLoopType() == LoopType::LTRemainder && !isMasked();
  }

  bool isScalarRemainder() const {
    return getLoopType() == LoopType::LTRemainder && isa<VPlanScalar>(Plan);
  }

  bool isMaskedOrScalarRemainder() const {
    return getLoopType() == LoopType::LTRemainder &&
           (isMasked() || isa<VPlanScalar>(Plan));
  }

private:
  VPlan *Plan;   // VPlan

  // Basic blocks used during merge.
  // For main VPlan these are the existing start and end blocks respectively,
  // before merge starts. For other VPlans both fields point to the same
  // newly created basic block with VPlanAdapter.
  VPBasicBlock *FirstBB = nullptr;
  VPBasicBlock *LastBB = nullptr;
  // Merge blocks created around Plan. The PrevMerge is placed after LastBB and
  // MergeBefore - before FirstBB.
  VPBasicBlock *PrevMerge = nullptr;
  VPBasicBlock *MergeBefore = nullptr;

  friend class VPlanCFGMerger;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const { dump(dbgs()); }
  void dump(raw_ostream &OS) const {
    auto getLoopTypeName = [](LoopType LT) -> StringRef {
      switch (LT) {
      case LoopType::LTRemainder:
        return "remainder";
      case LoopType::LTMain:
        return "main";
      case LoopType::LTPeel:
        return "peel";
      };
      return "";
    };
    OS << "VPlan: " << Plan->getName() << "\n";
    OS << " Kind: " << getLoopTypeName(getLoopType()) << " VF:" << getVF();
    if (getUF() > 1)
      OS << " UF:" << getUF();
    OS << " TC:" << getTC() << "\n";
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

protected:
  // PlannerType depends on global setting of -x knob and input type: LLVM-IR
  // or HIR. PlannerType drives the approach of VPlan planning and cost
  // modelling.
  enum class PlannerType {
    Base,        // Basic pipeline of planning is used for LLVM-IR input, which
                 // currently kicks in for Intel and non-Intel targets for
                 // #pragma forced vectorization loops only that bypassed HIR
                 // vectorization (either HIR path is supressed or HIR pass
                 // does not support such loops).
    LightWeight, // Light weight type is used for non-Intel targets with HIR
                 // VPlan input is activated.
    Full         // The full mode is for Intel targets and HIR input.
  };

public:
  LoopVectorizationPlanner(WRNVecLoopNode *WRL, Loop *Lp, LoopInfo *LI,
                           const TargetLibraryInfo *TLI,
                           const TargetTransformInfo *TTI, const DataLayout *DL,
                           class DominatorTree *DT, LegalityLLVM *Legal,
                           VPlanVLSAnalysis *VLSA, LLVMContext *C,
                           BlockFrequencyInfo *BFI = nullptr)
      : VectorlengthMD(nullptr), WRLp(WRL), TLI(TLI), TTI(TTI), DL(DL),
        Legal(Legal), VLSA(VLSA), DT(DT), TheLoop(Lp), LI(LI), Context(C),
        BFI(BFI) {
    clearBailoutRemark();
  }

  virtual ~LoopVectorizationPlanner() {}
  /// Build initial VPlans according to the information gathered by Legal
  /// when it checked if it is legal to vectorize this loop.
  /// Returns the number of VPlans built, zero if failed.
  unsigned buildInitialVPlans(Module *M, std::string VPlanName,
                              AssumptionCache &AC, VPAnalysesFactoryBase &VPAF,
                              ScalarEvolution *SE = nullptr,
                              bool IsLegalToVec = true);

  /// On VPlan construction, each instruction marked for predication by Legal
  /// gets its own basic block guarded by an if-then. This initial planning
  /// is legal, but is not optimal. This function attempts to leverage the
  /// necessary conditional execution of the predicated instruction in favor
  /// of other related instructions. The function applies these optimizations
  /// to all VPlans.
  // void optimizePredicatedInstructions();

  /// Select the best peeling variant for every VPlan.
  void selectBestPeelingVariants();

  /// Detects and returns the current type of planning.
  virtual PlannerType getPlannerType() const;

  /// Interface to get DDGraph, which has non-empty implementation for HIR
  /// pipeline only.
  virtual const loopopt::DDGraph *getDDGraph() const {
    return nullptr;
  }

  /// Create and return Plan/VF/UF specific CostModel object based on global
  /// compilation settings such as presence of -x knob in command line.
  std::unique_ptr<VPlanCostModelInterface> createCostModel(
    const VPlanVector *Plan, unsigned VF, unsigned UF = 1) const;

  /// Create and return Plan/VF/UF specific CostModel object based on global
  /// compilation settings such as presence of -x knob in command line.
  /// Do not include the SLP-breakage heuristic.
  std::unique_ptr<VPlanCostModelInterface> createNoSLPCostModel(
    const VPlanVector *Plan, unsigned VF, unsigned UF = 1) const;

  /// Record CM's decision and dispose of all other VPlans.
  // void setBestPlan(unsigned VF, unsigned UF);

  // Preprocess best VPlan before CG, creating the needed auxiliary loops
  // (peel/remainder of different kinds) and merging them into flattened
  // cfg. \p UF and \p VF are selected unroll factor and vector factor,
  // correspondingly, for main VPlan.
  virtual void emitPeelRemainderVPLoops(unsigned VF, unsigned UF);

  // Create VPlans that are needed for CFG merge by the selected scenario.
  virtual void createMergerVPlans(VPAnalysesFactoryBase &VPAF);

  /// Generate the IR code for the body of the vectorized loop according to the
  /// best selected VPlan.
  void executeBestPlan(CodeGenLLVM &LB);

  /// Post VPlan FrontEnd pass to verify that we can process the VPlan that
  /// was constructed. There are some limitations in CG, CM, and other parts of
  /// VPlan vectorizer on which we better gracefully bail out than assert.
  bool canProcessVPlan(const VPlanVector &Plan);

  /// Pre VPlan CodeGen pass to verify that we can lower the final VPlan into
  /// corresponding vector code.
  bool canLowerVPlan(const VPlanVector &Plan, unsigned VF);

#ifndef NDEBUG
  /// Go through all VPlans and run the VPlan verifier on them
  // TODO: VerifyLoopInfo should change to be flags for skipping/running
  //       checks once verifyVPlan uses that
  void verifyAllVPlans(const Loop *Lp, unsigned int Flags = 0) {
    SmallPtrSet<VPlan *, 2> Visited;

    for (auto &Pair : VPlans) {
      VPlanVector *P = Pair.second.MainPlan.get();
      if (Visited.insert(P).second)
        VPlanVerifier::verify(P, Lp, Flags);
      P = Pair.second.MaskedModeLoop.get();
      if (P && Visited.insert(P).second)
        VPlanVerifier::verify(P, Lp, Flags);
    }
  }
#endif

  /// Select the best plan and dispose all other VPlans.
  /// \Returns the selected vectorization factor and corresponding VPlan.
  std::pair<unsigned, VPlanVector *> selectBestPlan();

  /// \Returns the VPlan for selected best vectorization factor.
  VPlanVector *getBestVPlan();

  /// \Returns the selected best vectorization factor.
  unsigned getBestVF() {return VecScenario.getMainVF();}

  /// \Returns the selected best unroll factor.
  unsigned getBestUF() {return VecScenario.getMainUF();}

  bool peelWasSelected() const { return VecScenario.hasPeel(); }

  /// Reads all metadata specified by pragmas
  void readLoopMetadata() {
    VectorlengthMD =
        findOptionMDForLoop(TheLoop, "llvm.loop.intel.vector.vectorlength");
    IsVecRemainder = readVecRemainderEnabled();
    IsVecAlign = readVecAlignEnabled();
    IsDynAlign = readDynAlignEnabled();
    UnrollCount =
        getOptionalIntLoopAttribute(TheLoop, "llvm.loop.unroll.count");
  }

  void disableVecRemainder() { IsVecRemainder = false; }

  bool isVecRemainderDisabled() const {
    return IsVecRemainder.has_value() ? !IsVecRemainder.value() : false;
  }
  bool isVecRemainderEnforced() const {
    return IsVecRemainder.value_or(false);
  }

  bool isDynAlignEnabled() const { return IsDynAlign; }

  /// Extracts VFs from "llvm.loop.vector.vectorlength" metadata
  void extractVFsFromMetadata(unsigned SafeLen);

  /// Check if VF=1 provided in "llvm.loop.vector.vectorlength" metadata
  bool hasVFOneInMetadata() const;

  /// Returns vector of allowed VFs
  ArrayRef<unsigned> getVectorFactors();

  /// Predicate all unique non-scalar VPlans
  void predicate(void);

  /// Insert all-zero bypasses for \p Plan.
  void insertAllZeroBypasses(VPlanVector *Plan, unsigned VF);

  /// Transform integer div/rem masked instructions blending
  /// the second operand with 1.
  void blendWithSafeValue(void);

  /// Go through all VPLoadStoreInst in all masked mode VPlans and disable
  /// -1-stride optimization.
  void disableNegOneStrideOptInMaskedModeVPlans();

  /// Return Loop Unroll Factor either forced by option or pragma
  /// or advised by optimizations.
  /// \p Forced indicates that Unroll Factor is forced.
  virtual unsigned getLoopUnrollFactor(bool *Forced = nullptr);

  /// Perform VPlan loop unrolling if needed
  virtual bool unroll(VPlanVector &Plan);

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

  bool hasMaskedVPlanForVF(const unsigned VF) const {
    return getMaskedVPlanForVF(VF) != nullptr;
  }

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

  /// Return the list-range of VPlans created by the merger to the clients.
  inline decltype(auto) mergerVPlans() const {
    return make_range(MergerVPlans.begin(), MergerVPlans.end());
  }

  VPLoopDescrMap &getLoopDescrs() { return TopLoopDescrs; }

  /// Execute peephole optimizations before predicator.
  void runPeepholeBeforePredicator();

  static int getVPlanOrderNumber() { return VPlanOrderNumber; }

  /// Initialize cached bailout remark data.
  void clearBailoutRemark() const { BR.BailoutRemark = OptRemark(); }

  /// Store a variadic remark indicating the reason for not vectorizing a loop.
  /// Clients should pass string constants as std::string to avoid extra
  /// instantiations of this template function.
  template <typename... Args>
  void setBailoutRemark(OptReportVerbosity::Level BailoutLevel,
                        OptRemarkID BailoutID, Args &&...BailoutArgs) const {
    BR.BailoutLevel = BailoutLevel;
    BR.BailoutRemark =
        OptRemark::get(*Context, BailoutID, std::forward<Args>(BailoutArgs)...);
  }

  /// Access the cached bailout remark.
  VPlanBailoutRemark &getBailoutRemark() const { return BR; }

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
                    std::string VPlanName, AssumptionCache &AC,
                    ScalarEvolution *SE = nullptr);

  /// If FoorcedVF if specified, puts it into vector of VFs. Else if
  /// "llvm.loop.vector.vectorlength" metadata is specified, fills vector of VFs
  /// with values from metadata. Else if "llvm.loop.vector.vectorlength"
  /// metadata is not specified, defines MinVF and MaxVF and fills vector of VFs
  /// with default vector values between MinVF and MaxVF.
  void setDefaultVectorFactors();

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
  /// If second returned value \p ExactUB set to true, it means that the
  /// upper bound value can be directly used as the exact trip count of the
  /// loop. That basically depends on the loop latch comparison predicate:
  /// ExactUB is true => UB is the exact loop trip count:
  ///     iv = add iv, 1
  ///     cond = cmp lt iv, ub
  ///     br cond loopheader, loopexit
  /// ExactUB is false ==> the loop trip count is "UB + 1":
  ///     iv = add iv, 1
  ///     cond = cmp le iv, ub
  ///     br cond loopheader, loopexit
  static bool hasLoopNormalizedInduction(const VPLoop *Loop, bool &ExactUB);

  /// Contains metadata slecified by "llvm.loop.vector.vectorlength"
  MDNode *VectorlengthMD;

  /// Contains true or false value from "llvm.loop.vector.vecremainder" metadata
  /// if it was set on the loop or false if forced by disableVecRemainder call.
  /// Otherwise, it is std::nullopt.
  std::optional<bool> IsVecRemainder;

  /// Contains true if "llvm.loop.intel.vector.aligned" metadata is specified
  /// and false if "llvm.loop.intel.vector.unaligned" metadata is specified.
  /// Otherwise, it is std::nullopt.
  std::optional<bool> IsVecAlign;

  /// Contains true or false value from "llvm.loop.vectorize.dynamic_align"
  /// metadata
  bool IsDynAlign = true;

  /// Contains unroll factor value from "llvm.loop.unroll.count" metadata.
  std::optional<int> UnrollCount;

  /// Returns true/false value if "llvm.loop.intel.vector.vecremainder"/
  /// "llvm.loop.intel.vector.novecremainder" metadata is specified. If there is
  ///  no such metadata, returns std::nullopt.
  std::optional<bool> readVecRemainderEnabled();

  /// Returns true if "llvm.loop.intel.vector.aligned" metadata is specified
  /// and false if "llvm.loop.intel.vector.unaligned" metadata is specified.
  /// Otherwise, it is std::nullopt.
  std::optional<bool> readVecAlignEnabled();

  /// Returns true/false value if "llvm.loop.intel.vector.dynamic_align"/
  /// "llvm.loop.intel.vector.nodynamic_align" metadata is specified. If there
  /// is no such metadata, returns true.
  bool readDynAlignEnabled();

  /// Transform to emit explict uniform Vector loop iv.
  virtual void emitVecSpecifics(VPlanVector *Plan);

  /// \Returns a pair of the <min, max> types' width used in the underlying loop.
  /// Doesn't take into account i1 type.
  virtual std::pair<unsigned, unsigned> getTypesWidthRangeInBits() const;

  /// Create VPLiveIn/VPLiveOut lists for VPEntities.
  virtual void createLiveInOutLists(VPlanVector &Plan);

  // Checks whether a call is to an OMP construct, and if so whether it's valid
  // inside of a SIMD region
  bool isInvalidOMPConstructInSIMD(VPCallInstruction* VPCall) const;

  /// Check whether everything in the loop body is supported at the moment.
  /// We can have some unimplemented things and it's better to gracefully
  /// bailout in such cases than assert or generate incorrect code.
  virtual bool canProcessLoopBody(const VPlanVector &Plan,
                                  const VPLoop &Loop) const;

  /// Check if vectorizer supports SOA privates.
  virtual bool isSOACodegenSupported() const { return true; }

  /// Register the choosen vectorization scenario: peel/remainder configuration,
  /// vector and unroll factors for main loop
  void updateVecScenario(VPlanPeelEvaluator const &PE,
                         VPlanRemainderEvaluator const &RE, unsigned VF,
                         unsigned UF, bool MainIsMasked);

  /// Select simplest vectorization scenario: no peel, non-masked main loop with
  /// specified vector and unroll factors, scalar remainder.
  void selectSimplestVecScenario(unsigned VF, unsigned UF, bool IsMainMasked,
                                 bool AddRemainder);

  /// Fill in the map of top loops descriptors (see TopLoopDescrs and its type
  /// for details). The scalar loops are skipped due to we don't have VPLoops
  /// for them and they are created only during CG
  void fillLoopDescrs();

  /// Reports a reason for vectorization bailout.
  /// \p Message will appear both in the debug dump and the opt report remark.
  template <typename... Args>
  void bailout(OptReportVerbosity::Level Level, OptRemarkID ID,
               std::string Message, Args &&...BailoutArgs) const {
    DEBUG_WITH_TYPE("LoopVectorizationPlanner", dbgs() << Message << '\n');
    setBailoutRemark(Level, ID, Message, std::forward<Args>(BailoutArgs)...);
  }

  /// Reports a reason for vectorization bailout.
  /// The auxiliary remark string for \p MessageID will appear both in the debug
  /// dump and the opt report remark.
  template <typename... Args>
  void bailout(OptReportVerbosity::Level Level, OptRemarkID ID,
               AuxRemarkID MessageID, Args &&...BailoutArgs) const {
    DEBUG_WITH_TYPE("LoopVectorizationPlanner",
                    dbgs() << OptReportAuxDiag::getMsg(MessageID) << '\n');
    setBailoutRemark(Level, ID, MessageID, std::forward<Args>(BailoutArgs)...);
  }

  /// Reports a reason for vectorization bailout. Always returns false.
  /// \p Debug will appear in the debug dump, but not in the opt report remark.
  template <typename... Args>
  void bailoutWithDebug(OptReportVerbosity::Level Level, OptRemarkID ID,
                        std::string Debug, Args &&...BailoutArgs) const {
    DEBUG_WITH_TYPE("LoopVectorizationPlanner", dbgs() << Debug << '\n');
    setBailoutRemark(Level, ID, std::forward<Args>(BailoutArgs)...);
  }

  /// Go through all VPlans and run \p ProcessPlan on each of them.
  template <class F> void transformAllVPlans(F &ProcessPlan) {
    SmallPtrSet<VPlan *, 2> Visited;

    auto TransformPlan = [&Visited, &ProcessPlan](VPlanVector &P) -> void {
      if (!Visited.insert(&P).second)
        return;
      ProcessPlan(P);
    };

    for (auto &Pair : VPlans) {
      if (Pair.first == 1) // skip VF==1
        continue;
      TransformPlan(*Pair.second.MainPlan);
      if (Pair.second.MaskedModeLoop)
        TransformPlan(*Pair.second.MaskedModeLoop);
    }
  }

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

  /// VPlan VLS Analysis.
  VPlanVLSAnalysis *VLSA;

  /// The dominator tree.
  class DominatorTree *DT;

  /// A list of other additional VPlans, created during peel/remainders
  /// creation and cloning.
  std::list<CfgMergerPlanDescr> MergerVPlans;

  // Map of the descriptors for top vector loops. Is built after CFG merger
  // creates VPlan list for merging. No info for inner loops is kept here.
  VPLoopDescrMap TopLoopDescrs;

  // Bail-out reason data.
  mutable VPlanBailoutRemark BR;

  struct VPPeelSummary {
    std::string Scenario;
    std::string Formula;
    bool PeelingWasPerformed;
    std::string PeelKind;
    VPInstructionCost GainWithPeel;
    VPInstructionCost GainWithoutPeel;
  };

  struct VPCostSummary {
    VPInstructionCost ScalarIterationCost;
    VPInstructionCost VectorIterationCost;
    VPInstructionCost Speedup;
    VPInstructionCost LoopOverhead;

    VPPeelSummary PeelSummary;

    VPCostSummary() = default;

    VPCostSummary(VPInstructionCost ScalarIterationCost,
                  VPInstructionCost VectorIterationCost,
                  VPInstructionCost Speedup, VPInstructionCost VectorInitFini,
                  VPPeelSummary PeelSummary)
        : ScalarIterationCost(ScalarIterationCost),
          VectorIterationCost(VectorIterationCost), Speedup(Speedup),
          PeelSummary(PeelSummary) {
      if (VectorIterationCost.isValid() && VectorIterationCost != 0)
        LoopOverhead = VectorInitFini / VectorIterationCost;
      else
        LoopOverhead = VPInstructionCost::getInvalid();
    }
  };

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

  /// Check for fences after initial transforms and bail out if found.
  bool fencesFound(VPlan *Plan);

  /// Main function that canonicalizes the CFG and applyies loop massaging
  /// transformations like mergeLoopExits transform.
  void doLoopMassaging(VPlanVector *Plan);

  /// Use results from VPEntity analysis to emit explicit VPInstruction-based
  /// representation. The analysis results can be invalidated/stale after this
  /// transform.
  void emitVPEntityInstrs(VPlanVector *Plan);

  /// Produce optimization report remarks for VPReductions.
  void reportReductions(VPlanVector *Plan, VPLoop *MainLoop,
                        VPLoopEntityList *LE);

  /// Clear NSW/NUW flags from reduction instructions if necessary.
  void clearWrapFlagsForReductions(VPlanVector *Plan);

  /// Exchange input and scan phases for exclusive scan.
  void exchangeExclusiveScanLoopInputScanPhases(VPlanVector *Plan);

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
  void emitVectorLoopIV(VPlanVector *Plan, VPValue *TripCount, VPValue *VF,
                        VPValue *IVUpdate, bool ExactUB);

  /// Utility to dump and verify VPlan details after initial set of transforms.
  void printAndVerifyAfterInitialTransforms(VPlan *Plan);

  /// The loop that we evaluate.
  Loop *TheLoop;

  /// Loop Info analysis.
  LoopInfo *LI;

  // Context for the current function.
  LLVMContext *Context;

  /// Block frequency info
  BlockFrequencyInfo *BFI;

  /// The profitablity analysis.
  // LoopVectorizationCostModel *CM;

  // TODO: Move to base class
  CodeGenLLVM *ILV = nullptr;

  // InnerLoopVectorizer *ILV = nullptr;

  /// VPlans are shared between VFs, use smart pointers.
  DenseMap<unsigned, VPlanPair> VPlans;

  /// A list of other additional VPlans, created during peel/remainders
  /// creation and cloning.
  SmallVector<std::unique_ptr<VPlan>, 2> AuxVPlans;

  /// The Cost of a single iteration of scalar VPlan. Calculated far before
  /// VF selection and kept in this member. Is not expected to be updated
  /// once calculated.
  VPInstructionCost ScalarIterationCost;

  /// Internal loop number. The loops are numbered sequentially throughout
  /// one compilation. The loop numbers can be used for debugging.
  static int VPlanOrderNumber;

  /// True iff fences were detected during canProcessLoopBody().
  mutable bool FencesInInput = false;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELLOOPVECTORIZATIONPLANNER_H

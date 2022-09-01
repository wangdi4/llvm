//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines the VPlanCostModel class that is used for all cost
/// estimations performed in the VPlan-based vectorizer. The class provides both
/// the interfaces to calculate different costs (e.g., a single VPInstruction or
/// the whole VPlan) and the dedicated printing methods used exclusively for
/// testing purposes.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODEL_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODEL_H

#include "IntelVPlanCallVecDecisions.h"
#include "IntelVPlanCostModelHeuristics.h"
#include "IntelVPlanUtils.h"
#include "IntelVPlanVLSAnalysis.h"
#include "IntelVPlanVLSTransform.h"
#include "llvm/Analysis/Intel_OptVLS.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Support/SaveAndRestore.h"

namespace llvm {
class DataLayout;
class TargetTransformInfo;
class Type;
class Value;
class raw_ostream;

namespace vpo {
class VPlan;
class VPBasicBlock;
class VPInstruction;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#define CM_DEBUG(OS, X) { if (OS) { X; }}
#else  // !NDEBUG || LLVM_ENABLE_DUMP
#define CM_DEBUG(OS, X)
#endif // !NDEBUG || LLVM_ENABLE_DUMP

#if INTEL_CUSTOMIZATION
class VPlanVLSCostModel : public OVLSCostModel {
public:
  explicit VPlanVLSCostModel(unsigned VF,
                             const TargetTransformInfo &TTI, LLVMContext &Cntx)
      : OVLSCostModel(TTI, Cntx), VF(VF) {}
  /// Generic function to get a cost of OVLSInstruction. Internally it has
  /// dispatch functionality to return cost for OVLSShuffle
  virtual uint64_t getInstructionCost(const OVLSInstruction *I) const final;

  /// Return cost of a gather or scatter instruction.
  virtual uint64_t getGatherScatterOpCost(const OVLSMemref &Memref) const final;

protected:
  unsigned VF;
};
#endif // INTEL_CUSTOMIZATION

// VPlanTTICostModel defines interface and its implementation, that is to be
// used by Cost Model heuristics.  Also all Heuristics independent code goes
// into VPlanTTICostModel.
//
// We don't create objects of this type, a general Cost Model type object
// rather passed to Heuristics.
class VPlanTTICostModel {
public:
  /// Get TTI based cost of a single instruction \p VPInst.
  VPInstructionCost getTTICost(const VPInstruction *VPInst);

  /// Return cost of all-zero check instruction
  VPInstructionCost getAllZeroCheckInstrCost(Type *VecSrcTy, Type *DestTy);

  // getLoadStoreCost(LoadOrStore, Alignment, VF) interface returns the Cost
  // of Load/Store VPInstruction given VF and Alignment on input. For
  // compress-store/expand-load instructions if GSCostOnly parameter is set only
  // gather/scatter cost will be returned (usual loads/stores are not affected).
  VPInstructionCost getLoadStoreCost(const VPLoadStoreInst *LoadStore,
                                     Align Alignment, unsigned VF,
                                     bool GSCostOnly = false) const;
  // The Cost of Load/Store using underlying IR/HIR Inst Alignment. For
  // compress-store/expand-load instructions if GSCostOnly parameter is set only
  // gather/scatter cost will be returned (usual loads/stores are not affected).
  VPInstructionCost getLoadStoreCost(const VPLoadStoreInst *LoadStore,
                                     unsigned VF,
                                     bool GSCostOnly = false) const;

  const VPlanVector *const Plan;
  const unsigned VF;
  const TargetLibraryInfo *const TLI;
  const DataLayout *const DL;
  const TargetTransformInfo &TTI;
  const VPlanVLSAnalysis *const VLSA;

  /// \Returns the alignment of the load/store \p LoadStore.
  ///
  /// This method guarantees to never return zero by returning default alignment
  /// for the base type in case of zero alignment in the underlying IR, so this
  /// method can freely be used even for widening of the \p VPInst.
  Align getMemInstAlignment(const VPLoadStoreInst *LoadStore) const;

  /// \Returns True iff \p VPInst is Uniform load or store.
  bool isUniformLoadStore(const VPLoadStoreInst *LoadStore) const;

  /// \Returns True iff \p VPInst is Unit Strided load or store.
  /// When load/store is strided NegativeStride is set to true if the stride is
  /// negative (-1 in number of elements) or to false otherwise.
  bool isUnitStrideLoadStore(const VPLoadStoreInst *LoadStore,
                             bool &NegativeStride) const;

  /// \Returns true if VPInst is part of an optimized VLS group.
  bool isOptimizedVLSGroupMember(const VPInstruction *VPInst) const {
    if (!VLSA)
      return false;

    auto *Group = VLSA->getGroupsFor(Plan, VPInst);
    if (!Group)
      return false;

    return isTransformableVLSGroup(Group);
  }

  /// \Returns the cost of one operand or two operands arithmetics instructions.
  /// For one operand case Op2 is expected to be null.  Op1 is never expected to
  /// be null. VF specifies custom vector length.
  VPInstructionCost getArithmeticInstructionCost(const unsigned Opcode,
                                                 const VPValue *Op1,
                                                 const VPValue *Op2,
                                                 const Type *ScalarTy,
                                                 const unsigned VF);

protected:
  VPlanPeelingVariant* DefaultPeelingVariant = nullptr;

  VPlanTTICostModel(const VPlanVector *Plan, const unsigned VF,
                    const TargetTransformInfo *TTIin,
                    const TargetLibraryInfo *TLI, const DataLayout *DL,
                    VPlanVLSAnalysis *VLSAin)
    : Plan(Plan), VF(VF), TLI(TLI), DL(DL), TTI(*TTIin), VLSA(VLSAin),
      VPAA(*Plan->getVPSE(), *Plan->getVPVT(), VF) {

    // CallVecDecisions analysis invocation.
    VPlanCallVecDecisions CallVecDecisions(*const_cast<VPlanVector *>(Plan));

    // Pass native TTI into CallVecDecisions analysis.
    CallVecDecisions.runForVF(VF, TLI, &TTI);

    // Compute SVA results for current VPlan in order to compute cost
    // accurately in CM.
    const_cast<VPlanVector *>(Plan)->runSVA(VF);

    // Collect VLS Groups once VLSA is specified. Heuristics can query VLS
    // Groups when VLSA is available.
    if (VLSAin)
      VLSAin->getOVLSMemrefs(Plan, VF);
  }

  // We prefer protected dtor over virtual one as there is no plan to
  // manipulate with objects through VPlanTTICostModel type handler.
  ~VPlanTTICostModel() {};

private:
  VPlanAlignmentAnalysis VPAA;

  // The utility checks whether the Cost Model can assume that 32-bit indexes
  // will be used instead of 64-bit indexes for gather/scatter HW instructions.
  unsigned getLoadStoreIndexSize(const VPLoadStoreInst *LoadStore) const;

  // Calculates the sum of the cost of extracting VF elements of Ty type
  // or the cost of inserting VF elements of Ty type into a vector.
  VPInstructionCost getInsertExtractElementsCost(unsigned Opcode,
                                                 Type *Ty, unsigned VF);

  // Get intrinsic corresponding to provided call. This is purely meant for
  // internal cost computation purposes and not for general purpose
  // functionality (unlike llvm::getIntrinsicForCallSite).
  // TODO: This is a temporary solution to avoid CM from choosing inefficient
  // VFs, complete solution would be to introduce a general scheme in TTI to
  // provide costs for different SVML calls. Check JIRA : CMPLRLLVM-23527.
  Intrinsic::ID getIntrinsicForLibFuncCall(const VPCallInstruction *VPCall) const;

  // Get Cost for Intrinsic (ID) call.
  VPInstructionCost getIntrinsicInstrCost(
    Intrinsic::ID ID, const VPCallInstruction *VPCall, unsigned VF);

  // Returns TTI Cost in VPInst arbitrary VF.
  VPInstructionCost getTTICostForVF(const VPInstruction *VPInst, unsigned VF);

  // Determine cost adjustment for a memref with specific Alignment.
  VPInstructionCost getNonMaskedMemOpCostAdj(
    unsigned Opcode, Type *Src, Align Alignment) const;

  // Estimate the cost of memory operation taking into account align/misalign
  // probability.
  VPInstructionCost getMemoryOpCost(
    unsigned Opcode, Type *Src, Align Alignment, unsigned AddressSpace,
    TTI::TargetCostKind CostKind = TTI::TCK_RecipThroughput,
    const Instruction *I = nullptr) const;

  // Return cost of VPConflictInsn
  VPInstructionCost getConflictInsnCost(unsigned VF, unsigned ElementSizeBits);

  // The Cost of Compress/Expand Idiom Load/Store instruction.
  // If GSCostOnly parameter is set only gather/scatter cost will be returned.
  VPInstructionCost
  getCompressExpandLoadStoreCost(const VPLoadStoreInst *LoadStore, unsigned VF,
                                 bool GSCostOnly = false) const;
};

// Class HeuristicsList is designed to hold Heuristics objects. It is a
// template class and should be specialized with Heuristics types.  An object
// of HeuristicsList<Scope, HTy1, ... , HTyn> creates Heuristics objects of
// specified types HTy1 ... HTyn on the specified Scope, which in turn can be
// VPlan, VPBlock or VPInstruction.
//
// If a heuristics is created on scope 'Scope' it means that such heuristic is
// applied on that scope (i.e. for VPlan/VPBlock/VPInstruction).
// HeuristicsList implements facility to apply all heuristics it contains.
template <typename Scope, typename... Ts> class HeuristicsList;

// Recursive declaration for arbitrary number of Heuristics types on input.
template <typename Scope, typename HTy, typename... HTys>
class HeuristicsList<Scope, HTy, HTys...>
    : public HeuristicsList<Scope, HTys...> {
  using Base = HeuristicsList<Scope, HTys...>;
  HTy H;
public:
  HeuristicsList() = delete;
  HeuristicsList(VPlanTTICostModel *CM) : Base(CM), H(CM) {};
  /// initForVPlan() method invokes VPlan level initialization routines for
  /// every heuristic in the container.
  void initForVPlan() {
    H.initForVPlan();
    this->Base::initForVPlan();
  }
  void apply(const VPInstructionCost &TTICost, VPInstructionCost &Cost,
             Scope *S, raw_ostream *OS = nullptr) const {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    VPInstructionCost RefCost = Cost;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

    H.apply(TTICost, Cost, S, OS);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    H.printCostChange(RefCost, Cost, S, OS);
#endif // !NDEBUG || LLVM_ENABLE_DUMP

    // Once any heuristics in the pipeline returns Unknown/Invalid cost
    // return it immediately.
    if (!Cost.isValid())
      return;
    this->Base::apply(TTICost, Cost, S, OS);
  }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Note that ScopeTy and Scope are different types.
  template <typename ScopeTy>
  void dump(raw_ostream &OS, ScopeTy *S) const {
    H.dump(OS, S);
    this->Base::dump(OS, S);
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

// Specialization for empty Heuristics types list.
template <typename Scope>
class HeuristicsList<Scope> {
public:
  HeuristicsList() = delete;
  HeuristicsList(VPlanTTICostModel *CM) {}
  // There is no heuristics to apply, thus just be transparent.
  void apply(const VPInstructionCost &TTICost, VPInstructionCost &Cost,
             Scope *S, raw_ostream *OS = nullptr) const {}
  void initForVPlan() {}
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // No heuristics to emit dump.
  template <typename ScopeTy>
  void dump(raw_ostream &OS, ScopeTy *S) const {}
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

// VPlanCostModelInterface is pure virtual class defining the interface all
// VPlan CMs are expected to implement. External users can only use methods
// listed in this class.
class VPlanCostModelInterface {
public:
  virtual ~VPlanCostModelInterface() = default;

  /// Get Cost for the range of basic blocks specified through \p Begin and
  /// \p End with optionally specified peeling \p PeelingVariant and optionally
  /// specified pointer to output stream \p OS if debug output is requested.
  virtual VPInstructionCost
  getBlockRangeCost(const VPBasicBlock *Begin, const VPBasicBlock *End,
                    VPlanPeelingVariant *PeelingVariant = nullptr,
                    raw_ostream *OS = nullptr, StringRef RangeName = "") = 0;

  /// Get the pair of Costs (iteration cost and preheader/postexit cost) for
  /// VPlan with optionally specified peeling \p PeelingVariant and optionally
  /// specified pointer to output stream \p OS if debug output is requested.
  virtual VPlanCostPair getCost(VPlanPeelingVariant *PeelingVariant = nullptr,
                                 raw_ostream *OS = nullptr) = 0;

  /// Return the cost of Load/Store VPInstruction given VF and Alignment
  /// on input.
  virtual VPInstructionCost getLoadStoreCost(
    const VPLoadStoreInst *LoadOrStore,
    Align Alignment, unsigned VF) const = 0;
};

// Definition of 'Cost Model with Heuristics' template class. As the name
// implies the class extends TTI Cost Model class with a set of Heuristics
// that are specified through their types rather than object, exercising static
// polymorphism. Cost Model object creates Heuristics objects per set of
// Heuristics type provided.
//
// Heuristics types are provided through the three list of heuristics
// HeuristicsList: separate list of Heuristics for VPlan, VPBasicBlock and
// VPInstruction levels.
//
// 'Cost Model with Heuristics' class also features debug printing facility for
// its getCost() methods. Debug printing happens when getCost() methods are
// invoked with specified output stream argument.
//
// The template class implements VPlanCostModelInterface virtual functions
// and internal *Impl functions are used in imlementation that are not virtual.
//
template <typename HeuristicsListVPInstTy,
          typename HeuristicsListVPBlockTy,
          typename HeuristicsListVPlanTy>
class VPlanCostModelWithHeuristics :
    public VPlanTTICostModel, public VPlanCostModelInterface {
private:
  using BlockPair = std::pair<const VPBasicBlock*, const VPBasicBlock*>;

  HeuristicsListVPInstTy HeuristicsListVPInst;
  HeuristicsListVPBlockTy HeuristicsListVPBlock;
  HeuristicsListVPlanTy HeuristicsListVPlan;

  // The method applies the heuristics from input heuristics list modifing
  // the input Cost and returns adjusted cost.
  template <typename HeuristicsListTy, typename ScopeTy>
  VPInstructionCost applyHeuristics(
    HeuristicsListTy HeuristicsList, ScopeTy *Scope,
    const VPInstructionCost &Cost, raw_ostream *OS = nullptr) {
    VPInstructionCost RetCost = Cost;
    HeuristicsList.apply(Cost, RetCost, Scope, OS);
    return RetCost;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // The method invokes debug dump() facility of each heuristics list on
  // given input Scope.
  template <typename ScopeTy>
  void dumpAllHeuristics(raw_ostream &OS, ScopeTy *Scope) {
    HeuristicsListVPInst.dump(OS, Scope);
    HeuristicsListVPBlock.dump(OS, Scope);
    HeuristicsListVPlan.dump(OS, Scope);
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // VPInstruction level internal getCostImpl().
  VPInstructionCost getCostImpl(const VPInstruction *VPInst,
                                raw_ostream *OS = nullptr) {
    VPInstructionCost TTICost = getTTICost(VPInst);

    CM_DEBUG(OS, *OS << "  Cost " << TTICost << " for ";
             VPInst->printWithoutAnalyses(*OS););

    VPInstructionCost AdjCost = applyHeuristics(HeuristicsListVPInst, VPInst,
                                                TTICost, OS);

    CM_DEBUG(OS, dumpAllHeuristics(*OS, VPInst);
             if (TTICost != AdjCost)
               *OS << " AdjCost: " << AdjCost;
             *OS << '\n';);

    return AdjCost;
  }

  // VPBasicBlock level internal getCostImpl().
  VPInstructionCost getCostImpl(const VPBasicBlock *VPBB,
                                raw_ostream *OS = nullptr) {

    CM_DEBUG(OS, *OS << "Analyzing VPBasicBlock " << VPBB->getName() << '\n');

    VPInstructionCost BaseCost = 0;
    for (const VPInstruction &VPInst : *VPBB) {
      VPInstructionCost InstCost = getCostImpl(&VPInst, OS);
      // TODO:
      // For now we skip Unknown costs. Eventually we may want to allow Unknown
      // cost to propage to the final VPlan cost once every instructions
      // is modelled.
      if (InstCost.isUnknown())
        continue;
      BaseCost += InstCost;
    }

    CM_DEBUG(OS, *OS << VPBB->getName() << ": base cost: "
                     << BaseCost << '\n';);

    VPInstructionCost AdjCost = applyHeuristics(HeuristicsListVPBlock, VPBB,
                                               BaseCost, OS);
    CM_DEBUG(OS, dumpAllHeuristics(*OS, VPBB);
             if(BaseCost != AdjCost)
               *OS << " Adjusted Cost for BasicBlock: " <<
                 VPBB->getName() << AdjCost << '\n';);
    return AdjCost;
  }

  // Internal helper function to reduce code duplication.
  template <typename RangeTy>
  VPInstructionCost getRangeCost(RangeTy Range, raw_ostream *OS) {
    VPInstructionCost RangeCost =
      std::accumulate(Range.begin(), Range.end(), VPInstructionCost(0),
                      [=](VPInstructionCost Cost, const VPBasicBlock *VPBlk) {
                        return Cost + getCostImpl(VPBlk, OS);});
    return RangeCost;
  }

  inline BlockPair getVPlanPreLoopBeginEndBlocks() const {
    const VPLoop *L = Plan->getMainLoop(true);
    const VPBasicBlock *Preheader = L->getLoopPreheader();
    return std::make_pair(&Plan->getEntryBlock(), Preheader);
  }

  inline BlockPair getVPlanAfterLoopBeginEndBlocks() const {
    const VPLoop *L = Plan->getMainLoop(true);

    const VPBasicBlock *Latch = L->getLoopLatch();
    assert(Latch->getNumSuccessors() == 2 && "Expected two latch successors");

    const VPBasicBlock *ExitBB = Latch->getSuccessor(0);
    if (ExitBB == L->getHeader())
      ExitBB = Latch->getSuccessor(1);

    const VPBasicBlock *LastBB = &*Plan->getExitBlock();
    return std::make_pair(ExitBB, LastBB);
  }

  inline decltype(auto) getVPlanLoopRange() const {
    const VPLoop *L = Plan->getMainLoop(true);
    return sese_depth_first(L->getHeader(), L->getLoopLatch());
  }

  VPInstructionCost
  getBlockRangeCost(BlockPair BeginEnd,
                    VPlanPeelingVariant *PeelingVariant = nullptr,
                    raw_ostream *OS = nullptr, StringRef RangeName = "") {
    return getBlockRangeCost(BeginEnd.first, BeginEnd.second, PeelingVariant,
                             OS, RangeName);
  }


  // Ctor is private.
  // CM objects should be created using special interface of planner class
  // rather than created directly through the ctor.
  VPlanCostModelWithHeuristics(const VPlanVector *Plan, const unsigned VF,
                               const TargetTransformInfo *TTI,
                               const TargetLibraryInfo *TLI,
                               const DataLayout *DL,
                               VPlanVLSAnalysis *VLSA = nullptr) :
    VPlanTTICostModel(Plan, VF, TTI, TLI, DL, VLSA),
    HeuristicsListVPInst(this), HeuristicsListVPBlock(this),
    HeuristicsListVPlan(this) {}

protected:
  // Planners are allowed to create CostModel objects through
  // makeUniquePtr call.
  friend class LoopVectorizationPlanner;
  friend class LoopVectorizationPlannerHIR;

  // Protected wrapper around ctor to create unique_ptr and to hide unique_ptr
  // creating code and simplify caller's code.
  static auto makeUniquePtr(const VPlanVector *Plan, const unsigned VF,
                            const TargetTransformInfo *TTI,
                            const TargetLibraryInfo *TLI,
                            const DataLayout *DL,
                            VPlanVLSAnalysis *VLSA = nullptr) {
    std::unique_ptr<VPlanCostModelWithHeuristics> CMPtr(
      new VPlanCostModelWithHeuristics(Plan, VF, TTI, TLI, DL, VLSA));
    return CMPtr;
  }

public:
  VPlanCostModelWithHeuristics() = delete;

  VPInstructionCost
  getBlockRangeCost(const VPBasicBlock *Begin, const VPBasicBlock *End,
                    VPlanPeelingVariant *PeelingVariant = nullptr,
                    raw_ostream *OS = nullptr, StringRef RangeName = "") final {
    // Assume no peeling if it is not specified.
    SaveAndRestore<VPlanPeelingVariant*> RestoreOnExit(
        DefaultPeelingVariant,
        PeelingVariant ? PeelingVariant : &VPlanStaticPeeling::NoPeelLoop);

    VPInstructionCost Cost = getRangeCost(sese_depth_first(Begin, End), OS);
    CM_DEBUG(OS,
             StringRef Hdr = RangeName.empty() ? "Block range" : RangeName;
             *OS << "Cost Model for " << Hdr << " " << Begin->getName() << " : "
                 << End->getName() << " for VF = " << VF
                 << " resulted Cost = " << Cost << '\n';);

    // TODO: Consider which (and how) VPlan heuristics we should run here.
    return Cost;
  }

  VPlanCostPair getCost(VPlanPeelingVariant *PeelingVariant = nullptr,
                        raw_ostream *OS = nullptr) final {
    // Assume no peeling if it is not specified.
    SaveAndRestore<VPlanPeelingVariant*> RestoreOnExit(
        DefaultPeelingVariant,
        PeelingVariant ? PeelingVariant : &VPlanStaticPeeling::NoPeelLoop);

    // Initialize heuristics VPlan level data for each VPlan level getCost
    // call.
    // Note that VPBlock and VPInstruction level getCost interfaces and
    // getBlockRangeCost interface bypass the initialization call.
    HeuristicsListVPlan.initForVPlan();

    CM_DEBUG(OS, *OS << "Cost Model for VPlan " << Plan->getName() <<
             " with VF = " <<VF << ":\n";);

    VPInstructionCost PreHdrCost =
        getBlockRangeCost(getVPlanPreLoopBeginEndBlocks(),
                          nullptr /* peeling */, OS, "Loop preheader");

    VPInstructionCost BaseCost = getRangeCost(getVPlanLoopRange(), OS);

    CM_DEBUG(OS, *OS << "Base Cost: " << BaseCost << '\n';);

    VPInstructionCost TotCost = applyHeuristics(HeuristicsListVPlan, Plan,
                                                BaseCost, OS);

    CM_DEBUG(OS, dumpAllHeuristics(*OS, Plan);
             if (BaseCost != TotCost)
               *OS << "Total Cost: " << TotCost << '\n';);

    VPInstructionCost PostExitCost =
        getBlockRangeCost(getVPlanAfterLoopBeginEndBlocks(),
                          nullptr /* peeling */, OS, "Loop postexit");

    return std::make_pair(TotCost, PostExitCost + PreHdrCost);
  }

  /// This method is proxy to implementation in TTI cost model, which returns
  /// TTI cost (unmodified by VPInst-level heuristics).
  /// Currently we are Okay with that but may want to reconsider in future.
  VPInstructionCost getLoadStoreCost(const VPLoadStoreInst *LoadOrStore,
                                     Align Alignment, unsigned VF) const final {
    return VPlanTTICostModel::getLoadStoreCost(LoadOrStore, Alignment, VF);
  }
};

#if INTEL_FEATURE_SW_ADVANCED
using VPlanCostModelBase = VPlanCostModelWithHeuristics<
  HeuristicsList<const VPInstruction>, // empty list
  HeuristicsList<const VPBasicBlock>,  // empty list
  HeuristicsList<const VPlanVector,
                 VPlanCostModelHeuristics::HeuristicSLP,
                 VPlanCostModelHeuristics::HeuristicSpillFill>>;

// TODO: lightweight mode CostModel heuristics set to be tuned yet.
using VPlanCostModelLite = VPlanCostModelWithHeuristics<
  HeuristicsList<const VPInstruction>, // empty list
  HeuristicsList<const VPBasicBlock>,  // empty list
  HeuristicsList<
    const VPlanVector,
    VPlanCostModelHeuristics::HeuristicSLP,
    VPlanCostModelHeuristics::HeuristicGatherScatter,
    VPlanCostModelHeuristics::HeuristicSpillFill,
    VPlanCostModelHeuristics::HeuristicPsadbw>>;

using VPlanCostModelFull = VPlanCostModelWithHeuristics<
  HeuristicsList<
    const VPInstruction,
    VPlanCostModelHeuristics::HeuristicOVLSMember,
    VPlanCostModelHeuristics::HeuristicSVMLIDivIRem>,
  HeuristicsList<const VPBasicBlock>, // empty list
  HeuristicsList<
    const VPlanVector,
    VPlanCostModelHeuristics::HeuristicSLP,
    VPlanCostModelHeuristics::HeuristicGatherScatter,
    VPlanCostModelHeuristics::HeuristicSpillFill,
    VPlanCostModelHeuristics::HeuristicPsadbw>>;

#else // INTEL_FEATURE_SW_ADVANCED

using VPlanCostModelBase = VPlanCostModelWithHeuristics<
  HeuristicsList<const VPInstruction>, // empty list
  HeuristicsList<const VPBasicBlock>,  // empty list
  HeuristicsList<const VPlanVector>>;  // empty list

using VPlanCostModelLite = VPlanCostModelBase;
using VPlanCostModelFull = VPlanCostModelLite;

#endif // INTEL_FEATURE_SW_ADVANCED

} // namespace vpo

} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODEL_H

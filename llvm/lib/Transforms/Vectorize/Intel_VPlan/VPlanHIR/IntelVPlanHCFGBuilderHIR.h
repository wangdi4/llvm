//===-- IntelVPlanHCFGBuilderHIR.h ------------------------------*- C++ -*-===//
//
//   Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines VPlanHCFGBuilderHIR class which extends
/// VPlanHCFGBuilderBase with support to build a hierarchical CFG from HIR.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANHCFGBUILDER_HIR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANHCFGBUILDER_HIR_H

#include "../IntelLoopVectorizationLegality.h"
#include "../IntelVPlanHCFGBuilder.h"
#include "../IntelVPlanLegalityDescr.h"
#include "IntelVPlanVerifierHIR.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRVecIdioms.h"

using namespace llvm::loopopt;

namespace llvm {

// Forward declarations
namespace loopopt {
class DDGraph;
class HLLoop;
class HIRSafeReductionAnalysis;
class HLInst;
class HIRDDAnalysis;
class RegDDRef;
class DDRef;
class DDRefUtils;
} // namespace loopopt

namespace vpo {

// High-level class to capture and provide loop vectorization legality analysis
// for incoming HIR. Currently various loop entities like reductions, inductions
// and privates are identified and stored within this class.
class HIRVectorizationLegality final
    : public VectorizationLegalityBase<HIRVectorizationLegality> {
  // Explicit vpo:: to workaround gcc bug
  // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52625
  template <typename LegalityTy> friend class vpo::VectorizationLegalityBase;

public:
  struct CompareByDDRefSymbase {
    bool operator()(const DDRef *Ref1, const DDRef *Ref2) const {
      return Ref1->getSymbase() < Ref2->getSymbase();
    }
  };

  using DescrValueTy = DescrValue<DDRef>;
  using DescrWithAliasesTy = DescrWithAliases<DDRef>;

  // Class used to store aliases and initvalue required for loop vectorization
  // legality analysis for incoming HIR.
  class DescrWithInitValue : public DescrWithAliasesTy {
    using DescrKind = typename DescrValueTy::DescrKind;
    // NOTE: InitValue holds only DDRefs for which VPExternalDefs were created
    // for a descriptor/alias. DDRefs with VPConstants are not accounted for.
    // Each descriptor/alias may have multiple updating HLInsts within the loop.
    const DDRef *InitValue;

  public:
    DescrWithInitValue(DDRef *RefV)
        : DescrWithAliasesTy(RefV, DescrKind::DK_WithInitValue),
          InitValue(nullptr) {}
    // Move constructor
    DescrWithInitValue(DescrWithInitValue &&Other) = default;

    void setInitValue(DDRef *Val) { InitValue = Val; }
    const DDRef *getInitValue() const { return InitValue; }

    bool isValidAlias() const override {
      return InitValue && getUpdateInstructions().size() > 0;
    }

    static bool classof(const DescrWithAliasesTy *Descr) {
      return Descr->getKind() == DescrKind::DK_WithInitValue;
    }

    static bool classof(const DescrValueTy *Descr) {
      return Descr->getKind() == DescrKind::DK_WithInitValue;
    }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    void print(raw_ostream &OS, unsigned Indent = 0) const override {
      DescrWithAliasesTy::print(OS);
      if (InitValue) {
        OS.indent(Indent + 2) << "InitValue: ";
        InitValue->dump();
        OS << "\n";
      }
    }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  };

  // Specialized class to represent reduction descriptors specified explicitly
  // via SIMD reduction clause. The reduction's kind and signed datatype
  // information is also stored within this class.
  struct RedDescr : public DescrWithInitValue {
    RedDescr(RegDDRef *RegV, RecurKind KindV, bool Signed)
        : DescrWithInitValue(RegV), Kind(KindV), IsSigned(Signed) {}
    // Move constructor
    RedDescr(RedDescr &&Other) = default;

    RecurKind Kind;
    bool IsSigned;
  };
  using ReductionListTy = SmallVector<RedDescr, 8>;

  using PrivDescrTy = PrivDescr<DDRef>;
  using PrivDescrNonPODTy = PrivDescrNonPOD<DDRef>;
  using PrivatesListTy = SmallVector<PrivDescrTy, 8>;
  using PrivatesNonPODListTy = SmallVector<PrivDescrNonPODTy, 8>;
  // Specialized class to represent linear descriptors specified explicitly via
  // SIMD linear clause. The linear's Step value is also stored within this
  // class.
  struct LinearDescr : public DescrWithInitValue {
    LinearDescr(RegDDRef *RegV, const RegDDRef *StepV)
        : DescrWithInitValue(RegV), Step(StepV) {}
    // Move constructor
    LinearDescr(LinearDescr &&Other) = default;

    const DDRef *Step;
  };
  using LinearListTy = SmallVector<LinearDescr, 8>;

  // Delete copy/assignment/move operations
  HIRVectorizationLegality(const HIRVectorizationLegality &) = delete;
  HIRVectorizationLegality &
  operator=(const HIRVectorizationLegality &) = delete;
  HIRVectorizationLegality(HIRVectorizationLegality &&) = delete;
  HIRVectorizationLegality &operator=(HIRVectorizationLegality &&) = delete;

  HIRVectorizationLegality(const TargetTransformInfo *TTI,
                           HIRSafeReductionAnalysis *SafeReds,
                           HIRDDAnalysis *DDA)
      : TTI(TTI), SRA(SafeReds), DDAnalysis(DDA) {}

  /// Returns true if it is legal to vectorize this loop.
  bool canVectorize(const WRNVecLoopNode *WRLp);

  HIRSafeReductionAnalysis *getSRA() const { return SRA; }
  const HIRVectorIdioms *getVectorIdioms(HLLoop *Loop) const;

  const PrivatesListTy &getPrivates() const { return PrivatesList; }
  const PrivatesNonPODListTy &getNonPODPrivates() const {
    return PrivatesNonPODList;
  }
  const LinearListTy &getLinears() const { return LinearList; }
  const ReductionListTy &getReductions() const { return ReductionList; }

  PrivDescrTy *getPrivateDescr(const DDRef *Ref) const {
    return findDescr<PrivDescrTy>(PrivatesList, Ref);
  }
  PrivDescrNonPODTy *getPrivateDescrNonPOD(const DDRef *Ref) const {
    return findDescr<PrivDescrNonPODTy>(PrivatesNonPODList, Ref);
  }
  LinearDescr *getLinearDescr(const DDRef *Ref) const {
    return findDescr<LinearDescr>(LinearList, Ref);
  }
  RedDescr *getReductionDescr(const DDRef *Ref) const {
    return findDescr<RedDescr>(ReductionList, Ref);
  }

  /// Check if the given temp ref \p Ref is part of minmax+index idiom
  /// recognized for a loop \p HLoop.
  bool isMinMaxIdiomTemp(const DDRef *Ref, HLLoop *HLoop) const;

  /// Identify any DDRefs in the \p HLoop's pre/post-loop nodes which alias the
  /// OMP SIMD clause descriptor DDRefs
  void findAliasDDRefs(HLNode *BeginNode, HLNode *EndNode, HLLoop *HLoop);

  /// Check if the given DDRef \p Ref corresponds to any linear/reduction
  /// HIRLegality descriptors. If found, then update the corresponding
  /// descriptor with \p Ref as its initialization value since it is directly
  /// used inside loop. NOTE: The default InitValue for all descriptors/aliases
  /// is nullptr since it may never be actually used within the loop.
  void recordPotentialSIMDDescrUse(DDRef *Ref);

  /// Check if the given HLInst \p UpdateInst writes in an LVal DDRef that
  /// potentially corresponds to any linear/reduction HIRLegality descriptors.
  /// If found, then update the descriptor with \p UpdateInst as its updating
  /// instruction.
  void recordPotentialSIMDDescrUpdate(HLInst *UpdateInst);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// Debug print utility to display contents of the descriptor lists
  void dump(raw_ostream &OS) const;
  void dump() const { dump(errs()); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

private:
  /// Reports a reason for vectorization bailout. Always returns false.
  bool bailout(BailoutReason Code);

  /// Add an explicit non-POD private to PrivatesList
  /// TODO: Use Constr, Destr and CopyAssign for non-POD privates.
  void addLoopPrivate(RegDDRef *PrivVal, Type *PrivTy, Function *Constr,
                      Function *Destr, Function *CopyAssign, PrivateKindTy Kind,
                      bool IsF90NonPod) {
    assert(PrivVal->isAddressOf() && "Private ref is not address of type.");
    PrivatesNonPODList.emplace_back(PrivVal, PrivTy, Kind, Constr, Destr,
                                    CopyAssign, IsF90NonPod);
  }

  /// Add an explicit POD private to PrivatesList
  void addLoopPrivate(RegDDRef *PrivVal, Type *PrivTy, PrivateKindTy Kind) {
    assert(PrivVal->isAddressOf() && "Private ref is not address of type.");
    PrivatesList.emplace_back(PrivVal, PrivTy, Kind);
  }

  /// Add an explicit linear.
  void addLinear(RegDDRef *LinearVal, Type * /* LinearTy */, RegDDRef *Step) {
    assert(LinearVal->isAddressOf() && "Linear ref is not address of type.");

    // TODO: Type information is not used thus far but is here for consistency
    // of the legality interface(vs LLVM IR path). LinearListCvt converter has
    // problems and is disabled. Once enabled this likely has to be revisited
    // too.
    LinearList.emplace_back(LinearVal, Step);
  }
  /// Add an explicit reduction variable
  void addReduction(RegDDRef *V, RecurKind Kind,
                    Optional<InscanReductionKind> InscanDescr) {
    assert(V->isAddressOf() && "Reduction ref is not an address-of type.");

    // TODO: Consider removing IsSigned field from RedDescr struct since it is
    // unused and can basically be deducted from the recurrence kind.
    ReductionList.emplace_back(V, Kind, false /*IsSigned*/);
  }

  /// Check if the given \p Ref is an explicit SIMD descriptor variable of type
  /// \p DescrType in the list \p List, if yes then return the descriptor object
  /// corresponding to it, else nullptr
  template <typename DescrType>
  DescrType *findDescr(ArrayRef<DescrType> List, const DDRef *Ref) const;

  /// Return the descriptor object corresponding to the input \p Ref, if it
  /// represents a reduction or linear SIMD variable (original or aliases). If
  /// \p Ref is not a SIMD descriptor variable nullptr is returned.
  DescrWithInitValue *getLinearRednDescriptors(DDRef *Ref) {
    // Check if Ref is a linear descriptor
    DescrWithInitValue *Descr = getLinearDescr(Ref);

    // If Ref is not linear, check if it is a reduction variable
    if (!Descr)
      Descr = getReductionDescr(Ref);

    return Descr;
  }

  const TargetTransformInfo *TTI;
  HIRSafeReductionAnalysis *SRA;
  HIRDDAnalysis *DDAnalysis;
  PrivatesListTy PrivatesList;
  PrivatesNonPODListTy PrivatesNonPODList;
  LinearListTy LinearList;
  ReductionListTy ReductionList;

  using IdiomListTy = std::unique_ptr<HIRVectorIdioms>;
  // List of idioms recognized for each corresponding HLLoop.
  // NOTE: Map is made mutable since the const getter method repopulates the
  // list of idioms on the fly if no entry is found for a given loop. Check
  // getVectorIdioms(HLLoop*).
  mutable std::map<HLLoop *, IdiomListTy> VecIdioms;
};

class VPlanHCFGBuilderHIR : public VPlanHCFGBuilder {

private:
  /// The outermost loop to be vectorized.
  HLLoop *TheLoop;

  /// HIR DDGraph that contains DD information for the incoming loop nest.
  const DDGraph &DDG;

  HIRVectorizationLegality *HIRLegality;

  /// Loop header VPBasicBlock to HLLoop map.
  SmallDenseMap<VPBasicBlock *, HLLoop *, 4> Header2HLLoop;

  bool buildPlainCFG(VPLoopEntityConverterList &CvtVec) override;

  void populateVPLoopMetadata(VPLoopInfo *VPLInfo) override;

  void passEntitiesToVPlan(VPLoopEntityConverterList &Cvts) override;

public:
  VPlanHCFGBuilderHIR(const WRNVecLoopNode *WRL, HLLoop *Lp, VPlanVector *Plan,
                      HIRVectorizationLegality *Legality, const DDGraph &DDG);
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANHCFGBUILDER_HIR_H

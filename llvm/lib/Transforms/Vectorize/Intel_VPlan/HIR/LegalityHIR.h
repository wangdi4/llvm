//===-- LegalityHIR.h -------------------------------------------*- C++ -*-===//
//
//   INTEL CONFIDENTIAL
//
//   Copyright (C) 2017 Intel Corporation
//
//   This software and the related documents are Intel copyrighted materials,
//   and your use of them is governed by the express license under which they
//   were provided to you ("License").  Unless the License provides otherwise,
//   you may not use, modify, copy, publish, distribute, disclose or treansmit
//   this software or the related documents without Intel's prior written
//   permission.
//
//   This software and the related documents are provided as is, with no
//   express or implied warranties, other than those that are expressly
//   stated in the License.
//
//===----------------------------------------------------------------------===//
///
/// \file LegalityHIR.h
/// VPlan vectorizer's HIR legality analysis.
///
/// Split from IntelVPlanHCFGBuilderHIR.h on 2023-10-03.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_HIR_LEGALITYHIR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_HIR_LEGALITYHIR_H

#include "../IntelVPlanLegalityDescr.h"
#include "../Legality.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRVecIdioms.h"

namespace llvm {

namespace loopopt {
class HLLoop;
class HIRSafeReductionAnalysis;
class HLInst;
class HIRDDAnalysis;
class RegDDRef;
class DDRef;
} // namespace loopopt

namespace vpo {

/// \class LegalityHIR
///
/// This class has the following purposes:
///
///  1) To import loop entities (such as reductions, inductions, and
///     privates) from the WRNVecLoopNode associated with an HIR loop.
///  2) To perform legality testing on those entities.  If any entity
///     has characteristics that the VPlan vectorizer cannot currently
///     support, VPlan will bail out and provide a reason in the
///     optimization report.
///
/// These operations are performed by the canVectorize method.  Afterwards,
/// accessor methods provided by LegalityHIR can be used to examine the
/// imported entities.
///
/// In contrast with the LegalityLLVM class, there is no need to perform
/// additional legality testing concerning control flow, possibly unsafe
/// aliasing, unvectorizable data types, and so on, because this has
/// already been done prior to invoking the vectorizer.
///
class LegalityHIR final : public LegalityBase<LegalityHIR> {
  // Explicit vpo:: to workaround gcc bug
  // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52625
  template <typename LegalityTy> friend class vpo::LegalityBase;

public:
  struct CompareByDDRefSymbase {
    bool operator()(const DDRef *Ref1, const DDRef *Ref2) const {
      return Ref1->getSymbase() < Ref2->getSymbase();
    }
  };

  using DescrValueTy = DescrValue<DDRef>;
  using DescrWithAliasesTy = DescrWithAliases<DDRef>;
  using DescrWithInitValueTy = DescrWithInitValue<DDRef>;

  using RedDescrTy = RedDescr<DDRef>;
  using ReductionListTy = SmallVector<RedDescrTy, 8>;
  using RedDescrUDRTy = RedDescrUDR<DDRef>;
  using UDRListTy = SmallVector<RedDescrUDRTy, 8>;

  using PrivDescrTy = PrivDescr<DDRef>;
  using PrivDescrNonPODTy = PrivDescrNonPOD<DDRef>;
  using PrivDescrF90DVTy = PrivDescrF90DV<DDRef>;
  using PrivatesListTy = SmallVector<PrivDescrTy, 8>;
  using PrivatesNonPODListTy = SmallVector<PrivDescrNonPODTy, 8>;
  using PrivatesF90DVListTy = SmallVector<PrivDescrF90DVTy, 8>;
  /// Specialized class to represent linear descriptors specified explicitly via
  /// SIMD linear clause. The linear's Step value is also stored within this
  /// class.
  struct LinearDescr : public DescrWithInitValueTy {
    using DescrKind = typename DescrValueTy::DescrKind;
    LinearDescr(RegDDRef *RegV, Type *LinearTyV, Type *PointeeTyV,
                const RegDDRef *StepV)
        : DescrWithInitValueTy(RegV, DescrKind::DK_LinearDescr),
          LinearTy(LinearTyV), PointeeTy(PointeeTyV), Step(StepV) {}
    // Move constructor
    LinearDescr(LinearDescr &&Other) = default;

    Type *LinearTy;
    Type *PointeeTy;
    const DDRef *Step;
    /// Method to support type inquiry through isa, cast, and dyn_cast.
    static bool classof(const DescrValueTy *Descr) {
      return Descr->getKind() == DescrKind::DK_LinearDescr;
    }
  };
  using LinearListTy = SmallVector<LinearDescr, 8>;

  LegalityHIR(const LegalityHIR &) = delete;

  LegalityHIR &operator=(const LegalityHIR &) = delete;

  LegalityHIR(LegalityHIR &&) = delete;

  LegalityHIR &operator=(LegalityHIR &&) = delete;

  LegalityHIR(const TargetTransformInfo *TTI,
              HIRSafeReductionAnalysis *SafeReds, HIRDDAnalysis *DDA,
              LLVMContext *C)
      : LegalityBase(C), TTI(TTI), SRA(SafeReds), DDAnalysis(DDA) {}

  /// Check whether Fortran90 dope vectors are supported for HIR.
  bool isF90DVSupported() override {
    return EnableF90DVSupport && EnableHIRF90DVSupport;
  }

  /// Check whether array privates are supported for HIR.
  bool isPrivateArraySupported() override { return EnableHIRPrivateArrays; }

  /// Return true if we don't need to consult memory aliases for this
  /// reduction.  Otherwise set a bailout message and return false;
  bool reductionOkayForMemoryAliases(const ReductionItem *Item) override {
    bool OrigIsAllocaInst = false;
    if (auto *OrigI = dyn_cast<Instruction>(Item->getOrig()))
      OrigIsAllocaInst = isa<AllocaInst>(OrigI);

    if (!OrigIsAllocaInst)
      return bailout(OptReportVerbosity::High,
                     OptRemarkID::VecFailGenericBailout,
                     INTERNAL("Non-alloca instruction in reduction clause."));
    if (Item->getIsArraySection())
      return bailout(OptReportVerbosity::High,
                     OptRemarkID::VecFailGenericBailout,
                     INTERNAL("Array sections with offsets not supported."));
    return true;
  }

  /// Returns true if it is legal to vectorize this loop.
  bool canVectorize(const WRNVecLoopNode *WRLp);

  /// Returns the safe reduction analysis object.
  HIRSafeReductionAnalysis *getSRA() const { return SRA; }

  /// Returns vector idioms determined by analysis.
  const HIRVectorIdioms *getVectorIdioms(HLLoop *Loop) const;

  /// Returns list of privates in the loop.
  const PrivatesListTy &getPrivates() const { return PrivatesList; }

  /// Returns list of non-POD private in the loop.
  const PrivatesNonPODListTy &getNonPODPrivates() const {
    return PrivatesNonPODList;
  }

  /// Returns list of Fortran 90 dope vector privates in the loop.
  const PrivatesF90DVListTy &getF90DVPrivates() const {
    return PrivatesF90DVList;
  }

  /// Returns list of linears in the loop.
  const LinearListTy &getLinears() const { return LinearList; }

  /// Returns list of reductions in the loop.
  const ReductionListTy &getReductions() const { return ReductionList; }

  /// Returns list of user-defined reductions in the loop.
  const UDRListTy &getUDRs() const { return UDRList; }

  /// Returns a descriptor for the private \p Ref.
  PrivDescrTy *getPrivateDescr(const DDRef *Ref) const {
    return findDescr<PrivDescrTy>(PrivatesList, Ref);
  }

  /// Returns a descriptor for the non-POD private \p Ref.
  PrivDescrNonPODTy *getPrivateDescrNonPOD(const DDRef *Ref) const {
    return findDescr<PrivDescrNonPODTy>(PrivatesNonPODList, Ref);
  }

  /// Returns a descriptor for the Fortran 90 dope vector private \p Ref.
  PrivDescrF90DVTy *getPrivateDescrF90DV(const DDRef *Ref) const {
    return findDescr<PrivDescrF90DVTy>(PrivatesF90DVList, Ref);
  }

  /// Returns a descriptor for the linear \p Ref.
  LinearDescr *getLinearDescr(const DDRef *Ref) const {
    return findDescr<LinearDescr>(LinearList, Ref);
  }

  /// Returns a descriptor for the reduction associated with \p Ref.
  RedDescrTy *getReductionDescr(const DDRef *Ref) const {
    return findDescr<RedDescrTy>(ReductionList, Ref);
  }

  /// Returns a descriptor for the user-defined reduction associated
  /// with \p Ref.
  RedDescrUDRTy *getUDRDescr(const DDRef *Ref) const {
    return findDescr<RedDescrUDRTy>(UDRList, Ref);
  }

  /// Check if the given temp ref \p Ref is part of minmax+index idiom
  /// recognized for a loop \p HLoop.
  bool isMinMaxIdiomTemp(const DDRef *Ref, HLLoop *HLoop) const;

  /// Identify any DDRefs in the \p HLoop's pre/post-loop nodes which alias the
  /// OMP SIMD clause descriptor DDRefs.
  void findAliasDDRefs(HLNode *BeginNode, HLNode *EndNode, HLLoop *HLoop);

  /// Check if the given DDRef \p Ref corresponds to any linear/reduction
  /// HIRLegality descriptors. If found, then update the corresponding
  /// descriptor with \p Ref as its initialization value since it is directly
  /// used inside loop. NOTE: The default InitValue for all descriptors/aliases
  /// is nullptr since it may never be actually used within the loop.
  void recordPotentialSIMDDescrUse(DDRef *Ref);

  bool mapsToSIMDDescriptor(const DDRef *Ref);

  /// Check if the given HLInst \p UpdateInst writes in an LVal DDRef that
  /// potentially corresponds to any linear/reduction HIRLegality descriptors.
  /// If found, then update the descriptor with \p UpdateInst as its updating
  /// instruction.
  void recordPotentialSIMDDescrUpdate(HLInst *UpdateInst);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// Debug print utility to display contents of the descriptor lists.
  void dump(raw_ostream &OS) const;
  void dump() const { dump(errs()); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

private:
  /// Add an explicit non-POD private to PrivatesList.
  void addLoopPrivate(RegDDRef *PrivVal, Type *PrivTy, Function *Constr,
                      Function *Destr, Function *CopyAssign, PrivateKindTy Kind,
                      bool IsF90) {
    assert(PrivVal->isAddressOf() && "Private ref is not address of type.");
    PrivatesNonPODList.emplace_back(PrivVal, PrivTy, Kind, IsF90, Constr, Destr,
                                    CopyAssign);
  }

  /// Add an explicit POD private to PrivatesList or PrivatesF90DVList.
  void addLoopPrivate(RegDDRef *PrivVal, Type *PrivTy, PrivateKindTy Kind,
                      Type *F90DVElementType) {
    assert(PrivVal->isAddressOf() && "Private ref is not address of type.");
    if (F90DVElementType)
      PrivatesF90DVList.emplace_back(PrivVal, PrivTy, Kind, F90DVElementType);
    else
      PrivatesList.emplace_back(PrivVal, PrivTy, Kind, false /*IsF90*/);
  }

  /// Add an explicit linear.
  void addLinear(RegDDRef *LinearVal, Type *LinearTy, Type *PointeeTy,
                 RegDDRef *Step, bool IsIV) {
    assert(LinearVal->isAddressOf() && "Linear ref is not address of type.");
    LinearList.emplace_back(LinearVal, LinearTy, PointeeTy, Step);
  }

  /// Add an explicit reduction variable.
  void addReduction(RegDDRef *V, Type *Ty, RecurKind Kind,
                    std::optional<InscanReductionKind> InscanDescr,
                    bool IsComplex) {
    assert(V->isAddressOf() && "Reduction ref is not an address-of type.");
    // Following TODO is tracked in CMPLRLLVM-52293.
    assert(!InscanDescr && "TODO: Inscan for HIR is not supported!");

    // TODO: Consider removing IsSigned field from RedDescr struct since it is
    // unused and can basically be deduced from the recurrence kind.  Tracked
    // in CMPLRLLVM-52295.
    ReductionList.emplace_back(V, Kind, false /*IsSigned*/, IsComplex, Ty);
  }

  /// Add an explicit user-defined reduction variable.
  void addReduction(
      RegDDRef *V, Type *Ty, Function *Combiner, Function *Initializer,
      Function *Constr, Function *Destr,
      std::optional<InscanReductionKind> InscanRedKind = std::nullopt) {
    UDRList.emplace_back(V, Ty, Combiner, Initializer, Constr, Destr,
                         InscanRedKind);
  }

  /// Check if the incoming \p Ref matches the original SIMD descriptor DDRef
  /// \p DescrRef.
  bool isSIMDDescriptorDDRef(const RegDDRef *DescrRef, const DDRef *Ref,
                             bool isF90DV = false) const;

  /// Check if the given \p Ref is an explicit SIMD descriptor variable of type
  /// \p DescrType in the list \p List, if yes then return the descriptor object
  /// corresponding to it, else nullptr.
  template <typename DescrType>
  DescrType *findDescr(ArrayRef<DescrType> List, const DDRef *Ref) const {
    for (auto &Descr : List) {
      // TODO: try to avoid returning the non-const ptr.  Tracked in
      // CMPLRLLVM-52295.
      DescrType *CurrentDescr = const_cast<DescrType *>(&Descr);
      assert(isa<RegDDRef>(CurrentDescr->getRef()) &&
             "The original SIMD descriptor Ref is not a RegDDRef.");
      if (isSIMDDescriptorDDRef(cast<RegDDRef>(CurrentDescr->getRef()), Ref,
                                isa<PrivDescrF90DVTy>(CurrentDescr)))
        return CurrentDescr;

      // Check if Ref matches any aliases of current descriptor's ref.
      if (CurrentDescr->findAlias(Ref))
        return CurrentDescr;
    }

    return nullptr;
  }

  /// Check if the given \p Ref is an explicit SIMD descriptor variable of type
  /// \p DescrType in the list \p List, if yes then return the descriptor object
  /// corresponding to it, else nullptr. In addition to fidDescr this function
  /// also checks, whether \p Ref is in the list \p List.
  template <typename DescrType>
  DescrType *findDescrThatUsesSymbase(ArrayRef<DescrType> List,
                                      const DDRef *Ref) const {
    for (auto &Descr : List) {
      // TODO: try to avoid returning the non-const ptr.
      DescrType *CurrentDescr = const_cast<DescrType *>(&Descr);
      assert(isa<RegDDRef>(CurrentDescr->getRef()) &&
             "The original SIMD descriptor Ref is not a RegDDRef.");
      if (isSIMDDescriptorDDRef(cast<RegDDRef>(CurrentDescr->getRef()), Ref))
        return CurrentDescr;

      // Check if Ref matches any aliases of current descriptor's ref
      if (CurrentDescr->findAlias(Ref))
        return CurrentDescr;

      // Check if some alias is already using it
      for (auto *Alias : CurrentDescr->aliases()) {
        if (cast<RegDDRef>(Ref)->usesSymbase(Alias->getRef()->getSymbase()))
          return CurrentDescr;
      }
    }

    return nullptr;
  }

  /// Return the descriptor object corresponding to the input \p Ref, if it
  /// represents a reduction or linear SIMD variable (original or aliases). If
  /// \p Ref is not a SIMD descriptor variable nullptr is returned.
  DescrWithInitValueTy *getLinearRednDescriptors(DDRef *Ref) {
    // Check if Ref is a linear descriptor
    DescrWithInitValueTy *Descr = getLinearDescr(Ref);

    // If Ref is not linear, check if it is a reduction variable
    if (!Descr)
      Descr = getReductionDescr(Ref);

    if (!Descr)
      Descr = getUDRDescr(Ref);

    return Descr;
  }

  const TargetTransformInfo *TTI;
  HIRSafeReductionAnalysis *SRA;
  HIRDDAnalysis *DDAnalysis;
  PrivatesListTy PrivatesList;
  PrivatesNonPODListTy PrivatesNonPODList;
  PrivatesF90DVListTy PrivatesF90DVList;
  LinearListTy LinearList;
  ReductionListTy ReductionList;
  UDRListTy UDRList;

  using IdiomListTy = std::unique_ptr<HIRVectorIdioms>;
  // List of idioms recognized for each corresponding HLLoop.
  // NOTE: Map is made mutable since the const getter method repopulates the
  // list of idioms on the fly if no entry is found for a given loop. Check
  // getVectorIdioms(HLLoop*).
  mutable std::map<HLLoop *, IdiomListTy> VecIdioms;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_HIR_LEGALITYHIR_H

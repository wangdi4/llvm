//===-Legality.h-------------------------------------------------*- C++ -*-===//
//
//   INTEL CONFIDENTIAL
//
//   Copyright (C) 2019 Intel Corporation
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
///   \file Legality.h
///   VPlan vectorizer legality analysis.
///
///   Defines the base class LegalityBase that is the superclass of
///   LegalityLLVM and LegalityHIR.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LEGALITY_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LEGALITY_H

#include "Driver.h"
#include "IntelVPlan.h"
#include "IntelVPlanLegalityDescr.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"

using namespace llvm::loopopt;
namespace llvm {

class Function;

namespace vpo {
class LegalityLLVM;

extern bool ForceUDSReductionVec;
extern bool EnableHIRPrivateArrays;
extern bool EnableF90DVSupport;
extern bool EnableHIRF90DVSupport;

/// \class LegalityBase
///
/// LegalityBase uses subclass awareness for the purpose of static type
/// polymorphism, so that the same logic can be performed on Value objects
/// (LLVM IR) and DDRef objects (HIR).  The primary service provided by
/// LegalityBase is the EnterExplicitData method, which imports explicit
/// loop entities (reductions, inductions, privates, etc.) from the provided
/// WRNVecLoopNode object, and bails out for any characteristics of these
/// entities that the vectorizer is currently unable to support.
///
template <typename LegalityTy> class LegalityBase {
  /// Infer the IR kind for use in static type polymorphism.  This variable
  /// should not be tested to enable IR-specific logic.  Use virtual functions
  /// instead.
  static constexpr IRKind IR = std::is_same<LegalityTy, LegalityLLVM>::value
                                   ? IRKind::LLVMIR
                                   : IRKind::HIR;

public:
  using ValueTy =
      typename std::conditional<IR == IRKind::LLVMIR, Value, RegDDRef>::type;
  using PrivateKindTy =
      typename std::conditional<IR == IRKind::LLVMIR,
                                PrivDescr<Value>::PrivateKind,
                                PrivDescr<DDRef>::PrivateKind>::type;

  /// Return true if requested to vectorize a loop with inscan reduction.
  static bool forceUDSReductionVec() { return ForceUDSReductionVec; }

  /// Return the reason for bailing out.
  VPlanBailoutRemark &getBailoutRemark() { return BR; }

protected:
  LegalityBase(LLVMContext *C) : Context(C) {}

  LegalityBase(const LegalityBase &) noexcept = delete;

  LegalityBase &operator=(const LegalityBase &) = delete;

  virtual ~LegalityBase() = default;

  /// Import explicit data from WRLoop.
  bool EnterExplicitData(const WRNVecLoopNode *WRLp) {
    if (!WRLp)
      return true;
    return visitPrivates(WRLp) && visitLinears(WRLp) && visitReductions(WRLp);
  }

  /// Reports a reason for vectorization bailout. Always returns false.
  /// \p Message will appear both in the debug dump and the opt report remark.
  template <typename... Args>
  bool bailout(OptReportVerbosity::Level Level, OptRemarkID ID,
               std::string Message, Args &&...BailoutArgs) {
    DEBUG_WITH_TYPE("VPlanLegality", dbgs() << Message << '\n');
    setBailoutRemark(Level, ID, Message, std::forward<Args>(BailoutArgs)...);
    return false;
  }

  /// Reports a reason for vectorization bailout. Always returns false.
  /// \p Debug will appear in the debug dump, but not in the opt report remark.
  template <typename... Args>
  bool bailoutWithDebug(OptReportVerbosity::Level Level, OptRemarkID ID,
                        std::string Debug, Args &&...BailoutArgs) {
    DEBUG_WITH_TYPE("VPlanLegality", dbgs() << Debug << '\n');
    setBailoutRemark(Level, ID, std::forward<Args>(BailoutArgs)...);
    return false;
  }

  /// Initialize cached bailout remark data.
  void clearBailoutRemark() { BR.BailoutRemark = OptRemark(); }

  /// Store a variadic remark indicating the reason for not vectorizing a loop.
  /// Clients should pass string constants as std::string to avoid extra
  /// instantiations of this template function.
  template <typename... Args>
  void setBailoutRemark(OptReportVerbosity::Level BailoutLevel,
                        OptRemarkID BailoutID, Args &&...BailoutArgs) {
    BR.BailoutLevel = BailoutLevel;
    BR.BailoutRemark =
        OptRemark::get(*Context, BailoutID, std::forward<Args>(BailoutArgs)...);
  }

  /// Cached bailout reason remark.
  VPlanBailoutRemark BR;

  /// Context for creating optimization report remarks.
  LLVMContext *Context;

  /// Check whether Fortran90 dope vectors are supported for this IR.
  virtual bool isF90DVSupported() { return EnableF90DVSupport; }

  /// Check whether array privates are supported for this IR.
  virtual bool isPrivateArraySupported() { return true; }

  /// Return true if we don't need to consult memory aliases for this
  /// reduction.  Otherwise set a bailout message and return false;
  virtual bool reductionOkayForMemoryAliases(const ReductionItem *Item) {
    return true;
  }

private:
  /// Import any SIMD loop private amd lastprivate information into Legality.
  /// Return true on success.
  bool visitPrivates(const WRNVecLoopNode *WRLp) {
    for (LastprivateItem *Item : WRLp->getLpriv().items()) {
      if (Item->getIsF90DopeVector()) {
        // See CMPLRLLVM-10783.
        return bailout(OptReportVerbosity::High,
                       OptRemarkID::VecFailGenericBailout,
                       INTERNAL("F90 dope vector privates are not supported."));
      }
      if (!visitLastPrivate(Item)) {
        assert(BR.BailoutRemark && "visitLastPrivate didn't set bailout data!");
        return false;
      }
    }

    for (PrivateItem *Item : WRLp->getPriv().items()) {
      if (Item->getIsF90DopeVector() && !isF90DVSupported())
        // See CMPLRLLVM-10783.
        return bailout(OptReportVerbosity::High,
                       OptRemarkID::VecFailGenericBailout,
                       INTERNAL("F90 dope vector privates are not supported."));
      if (!visitPrivate(Item)) {
        assert(BR.BailoutRemark && "visitPrivate didn't set bailout data!");
        return false;
      }
    }
    return true;
  }
  /// Import information about loop linears to Legality.
  /// Returns true (always success).
  bool visitLinears(const WRNVecLoopNode *WRLp) {
    for (LinearItem *Item : WRLp->getLinear().items())
      visitLinear(Item);
    return true;
  }

  /// Import information about loop reductions into Legality.
  /// Return true on success.
  bool visitReductions(const WRNVecLoopNode *WRLp) {
    auto IsSupportedReduction = [this, WRLp](const ReductionItem *Item) {
      if (Item->getIsF90DopeVector())
        // See CMPLRLLVM-10783.
        return bailout(
            OptReportVerbosity::High, OptRemarkID::VecFailGenericBailout,
            INTERNAL("F90 dope vector reductions are not supported."));
      switch (Item->getType()) {
      case ReductionItem::WRNReductionMin:
      case ReductionItem::WRNReductionMax:
      case ReductionItem::WRNReductionAdd:
      case ReductionItem::WRNReductionSub:
      case ReductionItem::WRNReductionMult:
      case ReductionItem::WRNReductionBor:
      case ReductionItem::WRNReductionBxor:
      case ReductionItem::WRNReductionBand:
      case ReductionItem::WRNReductionUdr:
        return true;
      default:
        return bailoutWithDebug(
            OptReportVerbosity::Medium, OptRemarkID::VecFailBadReduction,
            INTERNAL("A reduction of this operation is not supported"),
            WRLp && WRLp->isOmpSIMDLoop() ? AuxRemarkID::SimdLoop
                                          : AuxRemarkID::Loop);
      }
    };

    for (ReductionItem *Item : WRLp->getRed().items())
      if (!IsSupportedReduction(Item) || !visitReduction(Item, WRLp)) {
        assert(BR.BailoutRemark && "visitReduction didn't set bailout data!");
        return false;
      }
    return true;
  }

  /// Produces recurrence kind given a reduction and type.
  static RecurKind getReductionRecurKind(const ReductionItem *Item,
                                         const Type *ElType) {
    switch (Item->getType()) {
    case ReductionItem::WRNReductionMin:
      return ElType->isIntegerTy()
                 ? (Item->getIsUnsigned() ? RecurKind::UMin : RecurKind::SMin)
                 : RecurKind::FMin;
    case ReductionItem::WRNReductionMax:
      return ElType->isIntegerTy()
                 ? (Item->getIsUnsigned() ? RecurKind::UMax : RecurKind::SMax)
                 : RecurKind::FMax;
    case ReductionItem::WRNReductionAdd:
    case ReductionItem::WRNReductionSub:
      return ElType->isIntegerTy() ? RecurKind::Add : RecurKind::FAdd;
    case ReductionItem::WRNReductionMult:
      return ElType->isIntegerTy() ? RecurKind::Mul : RecurKind::FMul;
    case ReductionItem::WRNReductionBor:
      return RecurKind::Or;
    case ReductionItem::WRNReductionBxor:
      return RecurKind::Xor;
    case ReductionItem::WRNReductionBand:
      return RecurKind::And;
    case ReductionItem::WRNReductionUdr:
      return RecurKind::Udr;
    default:
      llvm_unreachable("Unsupported reduction type");
    }
  };

  /// When NumElements is null (aka 1 element) return ElType.
  /// When it is a constant, construct an array type <NumElements x ElType>.
  /// Otherwise return nullptr.
  Type *adjustTypeIfArray(Type *ElType, const Value *NumElements) {
    if (!NumElements)
      return ElType;
    // For opaque pointers this is a new way an array type being communicated
    // into vectorizer so we need to re-construct its type as 1D array
    // [NumElements x ElType].
    // Eg. "QUAL.OMP.PRIVATE:TYPED"([10 x i32]* %20, [10 x i32]
    // zeroinitializer, i32 2520) will give us [2520 x [10 x i32]] array
    // type. Another way of representing an array:
    // "QUAL.OMP.PRIVATE:TYPED"([10 x i32]* %20, [10 x i32] zeroinitializer,
    // i32 1) will give us [10 x i32] array type (is likely to be deprecated).
    // The resulting array then will be vectorized by our algorithms.
    // Usually it will be transformed into [VF x ElType] array (unless SOA
    // transformation is applied to it).
    if (auto *CI = dyn_cast<ConstantInt>(NumElements))
      if (CI->getValue().ugt(1))
        return ArrayType::get(ElType, CI->getZExtValue());

    return nullptr;
  }

  /// Register an explicit private variable.
  /// Return true if successfully consumed.
  bool visitPrivate(const PrivateItem *Item) {
    Type *PrivType = nullptr;
    Value *NumElements = nullptr;
    std::tie(PrivType, NumElements, /* AddrSpace */ std::ignore) =
        VPOParoptUtils::getItemInfo(Item);
    Type *F90DVElementType = nullptr;
    if (Item->getIsF90DopeVector())
      std::tie(std::ignore, F90DVElementType) =
          VPOParoptUtils::getF90DVItemInfo(Item);

    assert(PrivType && "Missed OMP clause item type!");

    PrivType = adjustTypeIfArray(PrivType, NumElements);
    if (!PrivType)
      return bailout(OptReportVerbosity::High,
                     OptRemarkID::VecFailGenericBailout,
                     INTERNAL("Cannot handle array privates yet."));

    ValueTy *Val = Item->getOrig<IR>();

    if (Item->getIsNonPod()) {
      addLoopPrivate(Val, PrivType, Item->getConstructor(),
                     Item->getDestructor(), nullptr /* no CopyAssign */,
                     PrivateKindTy::NonLast, Item->getIsF90NonPod());
      return true;
    }

    if (isa<ArrayType>(PrivType) && !isPrivateArraySupported())
      return bailout(OptReportVerbosity::High,
                     OptRemarkID::VecFailGenericBailout,
                     INTERNAL("Private array is not supported"));

    addLoopPrivate(Val, PrivType, PrivateKindTy::NonLast, F90DVElementType);
    return true;
  }

  /// Register an explicit last private variable.
  /// Return true if successfully consumed.
  bool visitLastPrivate(const LastprivateItem *Item) {
    Type *LPrivType = nullptr;
    Value *NumElements = nullptr;
    std::tie(LPrivType, NumElements, /* AddrSpace */ std::ignore) =
        VPOParoptUtils::getItemInfo(Item);
    assert(LPrivType && "Missed OMP clause item type!");
    Type *F90DVElementType = nullptr;
    if (Item->getIsF90DopeVector())
      std::tie(std::ignore, F90DVElementType) =
          VPOParoptUtils::getF90DVItemInfo(Item);

    LPrivType = adjustTypeIfArray(LPrivType, NumElements);
    if (!LPrivType)
      return bailout(OptReportVerbosity::High,
                     OptRemarkID::VecFailGenericBailout,
                     INTERNAL("Cannot handle array privates yet."));

    ValueTy *Val = Item->getOrig<IR>();

    if (Item->getIsNonPod()) {
      addLoopPrivate(Val, LPrivType, Item->getConstructor(),
                     Item->getDestructor(), Item->getCopyAssign(),
                     PrivateKindTy::Last, Item->getIsF90NonPod());
      return true;
    }

    if (isa<ArrayType>(LPrivType) && !isPrivateArraySupported())
      return bailout(OptReportVerbosity::High,
                     OptRemarkID::VecFailGenericBailout,
                     INTERNAL("Private array is not supported"));

    // Until CG to extract vector by non-const index is implemented.
    if (Item->getIsConditional() && LPrivType->isVectorTy())
      return bailout(OptReportVerbosity::High,
                     OptRemarkID::VecFailGenericBailout,
                     INTERNAL("Conditional lastprivate of a vector type is "
                              "not supported."));

    addLoopPrivate(Val, LPrivType,
                   Item->getIsConditional() ? PrivateKindTy::Conditional
                                            : PrivateKindTy::Last,
                   F90DVElementType);
    return true;
  }

  /// Register explicit linear variable.
  void visitLinear(const LinearItem *Item) {
    Type *PointeeTy = nullptr;
    Type *LinType = nullptr;
    Value *NumElements = nullptr;
    std::tie(LinType, NumElements, /* AddrSpace */ std::ignore) =
        VPOParoptUtils::getItemInfo(Item);
    // TODO: Move to VPOParoptUtils::getItemInfo.  Tracked in
    // CMPLRLLVM-52295.
    if (Item->getIsTyped() && Item->getIsPointerToPointer()) {
      PointeeTy = Item->getPointeeElementTypeFromIR();
    }
    assert(LinType && "Missed OMP clause item type!");

    // NumElements == nullptr by convention means the number is 1.
    // Arrays are not allowed by OMP standard thus any values including
    // a constant are illegal for linears.
    assert(!isa<ArrayType>(LinType) && !NumElements &&
           "Unexpected number of elements");

    ValueTy *Val = Item->getOrig<IR>();
    ValueTy *Step = Item->getStep<IR>();
    bool IsIV = Item->getIsIV();
    addLinear(Val, LinType, PointeeTy, Step, IsIV);
  }

  /// Register explicit reduction variable.
  /// Return true if successfully consumed.
  bool visitReduction(const ReductionItem *Item, const WRNVecLoopNode *WRLp) {
    Type *RedType = nullptr;
    Value *NumElements = nullptr;
    std::tie(RedType, NumElements, /* AddrSpace */ std::ignore) =
        VPOParoptUtils::getItemInfo(Item);
    assert(RedType && "Missed OMP clause item type!");

    RedType = adjustTypeIfArray(RedType, NumElements);
    // Bailout for unknown array size.
    if (!RedType)
      // CMPLRLLVM-20621.
      return bailout(OptReportVerbosity::High,
                     OptRemarkID::VecFailGenericBailout,
                     INTERNAL("Cannot handle array reductions."));

    Type *ElemType = RedType;
    // Other temporary bailouts for array reductions.
    if (auto *ArrTy = dyn_cast<ArrayType>(RedType)) {
      // Prototype supported only for POD type arrays.
      ElemType = ArrTy->getElementType();
      if (!ElemType->isSingleValueType())
        return bailout(OptReportVerbosity::High,
                       OptRemarkID::VecFailGenericBailout,
                       INTERNAL("Cannot handle array reduction with "
                                "non-single value type."));

      // Bailouts for cases where memory aliases concept is needed.
      // So far, these are needed only on the HIR path and include:
      //   1. Non-alloca instruction being used in reduction clause.
      //   2. Array sections with offsets.
      if (!reductionOkayForMemoryAliases(Item))
        return false;

      // VPEntities framework can only handle single-element allocas. This check
      // works for both LLVM-IR and HIR.
      auto *OrigAlloca = dyn_cast<AllocaInst>(Item->getOrig());
      if (OrigAlloca && OrigAlloca->isArrayAllocation())
        return bailout(OptReportVerbosity::High,
                       OptRemarkID::VecFailGenericBailout,
                       INTERNAL("Array alloca detected."));
    }

    ValueTy *Val = Item->getOrig<IR>();
    RecurKind Kind = getReductionRecurKind(Item, ElemType);

    if (!forceUDSReductionVec() && Kind == RecurKind::Udr &&
        Item->getIsInscan())
      return bailout(OptReportVerbosity::High,
                     OptRemarkID::VecFailGenericBailout,
                     INTERNAL("Scan reduction with user-defined operation "
                              "is not supported."));

    // We currently don't support mul/div reduction of complex types. TODO:
    // Remove this bailout when complex intrinsics are enabled by default in FE
    // and VPlan CGs are updated to emit these intrinsics during finalization.
    // Tracked by CMPLRLLVM-29705.
    if (Item->getIsComplex() && Kind == RecurKind::FMul)
      return bailout(
          OptReportVerbosity::High, OptRemarkID::VecFailGenericBailout,
          INTERNAL("Complex mul/div type reductions are not supported."));

    if (Kind == RecurKind::Udr) {
      // Check for UDR and inscan flags, that would make this UDS.
      std::optional<InscanReductionKind> InscanRedKind = std::nullopt;
      if (Item->getIsInscan()) {
        InscanRedKind =
            isa<InclusiveItem>(
                WRegionUtils::getInclusiveExclusiveItemForReductionItem(WRLp,
                                                                        Item))
                ? InscanReductionKind::Inclusive
                : InscanReductionKind::Exclusive;
      }
      // Capture functions for init/finalization for UDRs.
      addReduction(Val, RedType, Item->getCombiner(), Item->getInitializer(),
                   Item->getConstructor(), Item->getDestructor(),
                   InscanRedKind);
    } else if (Item->getIsInscan()) {
      // Add an ordinary inscan reduction.
      addReduction(Val, RedType, Kind,
                   isa<InclusiveItem>(
                       WRegionUtils::getInclusiveExclusiveItemForReductionItem(
                           WRLp, Item))
                       ? InscanReductionKind::Inclusive
                       : InscanReductionKind::Exclusive,
                   Item->getIsComplex());
    } else
      addReduction(Val, RedType, Kind, std::nullopt, Item->getIsComplex());
    return true;
  }

  // Thunk to call subclass method to add a loop private.
  void addLoopPrivate(ValueTy *Val, Type *Ty, Function *Constr, Function *Destr,
                      Function *CopyAssign, PrivateKindTy Kind, bool IsF90) {
    return static_cast<LegalityTy *>(this)->addLoopPrivate(
        Val, Ty, Constr, Destr, CopyAssign, Kind, IsF90);
  }

  // Thunk to call subclass method to add a loop private for Fortran 90
  // dope vectors.
  void addLoopPrivate(ValueTy *Val, Type *Ty, PrivateKindTy Kind,
                      Type *F90DVElementType) {
    return static_cast<LegalityTy *>(this)->addLoopPrivate(Val, Ty, Kind,
                                                           F90DVElementType);
  }

  // Thunk to call subclass method to add a loop linear.
  void addLinear(ValueTy *Val, Type *Ty, Type *PointeeType, ValueTy *Step,
                 bool IsIV) {
    return static_cast<LegalityTy *>(this)->addLinear(Val, Ty, PointeeType,
                                                      Step, IsIV);
  }

  // Thunk to call subclass method to add a loop reduction.
  void addReduction(ValueTy *V, Type *Ty, RecurKind Kind,
                    std::optional<InscanReductionKind> InscanRedKind,
                    bool IsComplex) {
    return static_cast<LegalityTy *>(this)->addReduction(
        V, Ty, Kind, InscanRedKind, IsComplex);
  }

  // Thunk to call subclass method to add a user-defined loop reduction.
  void addReduction(ValueTy *V, Type *Ty, Function *Combiner,
                    Function *Initializer, Function *Constr, Function *Destr,
                    std::optional<InscanReductionKind> InscanRedKind) {
    return static_cast<LegalityTy *>(this)->addReduction(
        V, Ty, Combiner, Initializer, Constr, Destr, InscanRedKind);
  }
};
} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LEGALITY_H

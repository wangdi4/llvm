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
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LEGALITY_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LEGALITY_H

#include "IntelVPlan.h"
#include "IntelVPlanDriver.h"
#include "IntelVPlanLegalityDescr.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"

using namespace llvm::loopopt;
namespace llvm {

class Function;

namespace vpo {
class VPOVectorizationLegality;

extern bool ForceUDSReductionVec;
extern bool EnableHIRPrivateArrays;
extern bool EnableF90DVSupport;
extern bool EnableHIRF90DVSupport;

template <typename LegalityTy> class VectorizationLegalityBase {
  static constexpr IRKind IR =
      std::is_same<LegalityTy, VPOVectorizationLegality>::value ? IRKind::LLVMIR
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

protected:
  VectorizationLegalityBase() = default;

  /// Import explicit data from WRLoop.
  bool EnterExplicitData(const WRNVecLoopNode *WRLp) {
    if (!WRLp)
      return true;
    return visitPrivates(WRLp) && visitLinears(WRLp) && visitReductions(WRLp);
  }

  /// Cached bailout reason data.
  VPlanBailoutRemark BR;

private:
  /// Imports any SIMD loop private amd listprivate information into Legality
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
      if ((!EnableF90DVSupport ||
           (!EnableHIRF90DVSupport && IR == IRKind::HIR)) &&
          Item->getIsF90DopeVector())
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
  /// Import information about loop linears to Legality
  /// Returns true (always success).
  bool visitLinears(const WRNVecLoopNode *WRLp) {
    for (LinearItem *Item : WRLp->getLinear().items())
      visitLinear(Item);
    return true;
  }

  /// Import information about loop reductions into Legality
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

  /// Figures recurrence kind given a reduction and type.
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

  // When NumElements is null (aka 1 element) return ElType.
  // When it is a constant, construct an array type <NumElements x ElType>
  // Otherwise return nullptr.
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
    // i32 1) will give us [10 x i32] array type( is likely to be deprecated).
    // The resulting array then will be vectorized by our algorithms.
    // Usually it will be transformed into [VF x ElType] array (unless SOA
    // transformation is applied to it).
    if (auto *CI = dyn_cast<ConstantInt>(NumElements))
      if (CI->getValue().ugt(1))
        return ArrayType::get(ElType, CI->getZExtValue());

    return nullptr;
  }

  /// Register an explicit private variable
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

    if (!EnableHIRPrivateArrays && IR == IRKind::HIR &&
        isa<ArrayType>(PrivType))
      return bailout(OptReportVerbosity::High,
                     OptRemarkID::VecFailGenericBailout,
                     INTERNAL("Private array is not supported"));

    addLoopPrivate(Val, PrivType, PrivateKindTy::NonLast, F90DVElementType);
    return true;
  }

  /// Register an explicit last private variable
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

    if (!EnableHIRPrivateArrays && IR == IRKind::HIR &&
        isa<ArrayType>(LPrivType))
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

  /// Register explicit linear variable
  void visitLinear(const LinearItem *Item) {
    Type *PointeeTy = nullptr;
    Type *LinType = nullptr;
    Value *NumElements = nullptr;
    std::tie(LinType, NumElements, /* AddrSpace */ std::ignore) =
        VPOParoptUtils::getItemInfo(Item);
    // TODO: Move to VPOParoptUtils::getItemInfo
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

  /// Register explicit reduction variable
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

      // Bailouts from HIR path for cases where memory aliases concept is
      // needed. So far, these include -
      // 1. Non-alloca instruction being used in reduction clause.
      // 2. Array sections with offsets.
      if (IR == IRKind::HIR) {
        bool OrigIsAllocaInst = false;
        if (auto *OrigI = dyn_cast<Instruction>(Item->getOrig()))
          OrigIsAllocaInst = isa<AllocaInst>(OrigI);

        if (!OrigIsAllocaInst)
          return bailout(
              OptReportVerbosity::High, OptRemarkID::VecFailGenericBailout,
              INTERNAL("Non-alloca instruction in reduction clause."));
        if (Item->getIsArraySection())
          return bailout(
              OptReportVerbosity::High, OptRemarkID::VecFailGenericBailout,
              INTERNAL("Array sections with offsets not supported."));
      }

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
    if (Item->getIsComplex() && Kind == RecurKind::FMul)
      // TODO: Better is to add a medium remark of type VecFailBadComplexFloatOp
      // or VecFailBadComplexDoubleOp, depending on the underlying type.  These
      // remarks also require passing a string identifying the reduction kind.
      // There's some complexity in getting this information from "Item" for
      // all possible cases.
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

  // Set of thunks to a parent methods
  template <typename... Args>
  bool bailout(OptReportVerbosity::Level Level, OptRemarkID ID,
               std::string Message, Args &&...BailoutArgs) {
    return static_cast<LegalityTy *>(this)->bailout(
        Level, ID, Message, std::forward<Args>(BailoutArgs)...);
  }

  template <typename... Args>
  bool bailoutWithDebug(OptReportVerbosity::Level Level, OptRemarkID ID,
                        std::string Debug, Args &&...BailoutArgs) {
    return static_cast<LegalityTy *>(this)->bailoutWithDebug(
        Level, ID, Debug, std::forward<Args>(BailoutArgs)...);
  }

  void addLoopPrivate(ValueTy *Val, Type *Ty, Function *Constr, Function *Destr,
                      Function *CopyAssign, PrivateKindTy Kind, bool IsF90) {
    return static_cast<LegalityTy *>(this)->addLoopPrivate(
        Val, Ty, Constr, Destr, CopyAssign, Kind, IsF90);
  }

  void addLoopPrivate(ValueTy *Val, Type *Ty, PrivateKindTy Kind,
                      Type *F90DVElementType) {
    return static_cast<LegalityTy *>(this)->addLoopPrivate(Val, Ty, Kind,
                                                           F90DVElementType);
  }

  void addLinear(ValueTy *Val, Type *Ty, Type *PointeeType, ValueTy *Step,
                 bool IsIV) {
    return static_cast<LegalityTy *>(this)->addLinear(Val, Ty, PointeeType,
                                                      Step, IsIV);
  }

  void addReduction(ValueTy *V, Type *Ty, RecurKind Kind,
                    std::optional<InscanReductionKind> InscanRedKind,
                    bool IsComplex) {
    return static_cast<LegalityTy *>(this)->addReduction(
        V, Ty, Kind, InscanRedKind, IsComplex);
  }

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

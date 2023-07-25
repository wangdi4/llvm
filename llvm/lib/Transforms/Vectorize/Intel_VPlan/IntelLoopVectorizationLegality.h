//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   IntelLoopVectorizationLegality.h -- LLVM IR loop vectorization legality
//   analysis.
//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELLOOPVECTORIZERLEGALITY_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELLOOPVECTORIZERLEGALITY_H

#include "IntelVPlan.h"
#include "IntelVPlanLegalityDescr.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/IVDescriptors.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"
#include "llvm/Transforms/Vectorize/IntelVPlanDriver.h"

using namespace llvm::loopopt;
namespace llvm {

class PredicatedScalarEvolution;
class TargetTransformInfo;
class TargetLibraryInfo;
class Loop;
class LoopInfo;
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

  /// Convenience function for optimization remark substitution strings.
  std::string getAuxMsg(AuxRemarkID ID) { return OptReportAuxDiag::getMsg(ID); }

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
            WRLp && WRLp->isOmpSIMDLoop() ? getAuxMsg(AuxRemarkID::SimdLoop)
                                          : getAuxMsg(AuxRemarkID::Loop));
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
  bool visitReduction(const ReductionItem *Item,
                      const WRNVecLoopNode *WRLp) {
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

class VPOVectorizationLegality final
    : public VectorizationLegalityBase<VPOVectorizationLegality> {
  // Explicit vpo:: to workaround gcc bug
  // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52625
  template <typename LegalityTy> friend class vpo::VectorizationLegalityBase;

public:
  VPOVectorizationLegality(Loop *L, PredicatedScalarEvolution &PSE, Function *F,
                           LLVMContext *C)
      : TheLoop(L), PSE(PSE), Context(C), Induction(nullptr),
        WidestIndTy(nullptr) {}

  struct ExplicitReductionDescr {
    RecurrenceDescriptor RD; // Contains type info.
    Value *RedVarPtr;
    std::optional<InscanReductionKind> InscanRedKind;
  };
  struct InMemoryReductionDescr {
    RecurKind Kind;
    std::optional<InscanReductionKind> InscanRedKind;
    Instruction *UpdateInst;
    bool IsComplex;
    Type *Ty;
  };

  /// Returns true if it is legal to vectorize this loop.
  bool canVectorize(DominatorTree &DT, const WRNVecLoopNode *WRLp);

  using DescrValueTy = DescrValue<Value>;
  using DescrWithAliasesTy = DescrWithAliases<Value>;
  using PrivDescrTy = PrivDescr<Value>;
  using PrivDescrNonPODTy = PrivDescrNonPOD<Value>;
  using PrivDescrF90DVTy = PrivDescrF90DV<Value>;
  using UDRDescrTy = RedDescrUDR<Value>;

  /// Container-class for storing the different types of Privates
  using PrivatesListTy = MapVector<const Value *, std::unique_ptr<PrivDescrTy>>;

  /// ReductionList contains the reduction descriptors for all
  /// of the reductions that were found in the loop.
  using ReductionList = MapVector<PHINode *, RecurrenceDescriptor>;

  /// The list of explicit reduction variables.
  using ExplicitReductionList = MapVector<PHINode *, ExplicitReductionDescr>;
  /// The list of in-memory reductions. Store instruction that updates the
  /// reduction is also tracked.
  using InMemoryReductionList = MapVector<Value *, InMemoryReductionDescr>;
  /// The list of user-defined reductions. Stores the Legality descriptor that
  /// captures the SIMD reduction variable.
  using UDRList = SmallVector<std::unique_ptr<UDRDescrTy>, 2>;

  /// InductionList saves induction variables and maps them to the
  /// induction descriptor.
  using InductionList = MapVector<PHINode *, InductionDescriptor>;

  /// Linear list contains explicit linear specifications, mapping linear values
  /// to their strides and a type of the linear.
  using LinearListTy =
      MapVector<Value *,
                std::tuple<Type * /*EltType*/, Type * /* EltPointeeTy */,
                           Value * /*Step*/, bool /*IsIV*/>>;

  /// Returns the Induction variable.
  PHINode *getInduction() { return Induction; }

  /// Returns the induction variables found in the loop.
  InductionList *getInductionVars() { return &Inductions; }

  /// Returns the reduction variables found in the loop.
  ReductionList *getReductionVars() { return &Reductions; }

  /// Returns the explicit reduction variables.
  ExplicitReductionList *getExplicitReductionVars() {
    return &ExplicitReductions;
  }

  /// Return the list of reduction variables that are being
  /// calculated using memory in each iteration.
  InMemoryReductionList *getInMemoryReductionVars() {
    return &InMemoryReductions;
  }

  /// Returns the list of user-defined reductions (legality descriptors) found
  /// in loop.
  UDRList *getUDRVars() { return &UserDefinedReductions; }

  /// Return a recurrence descriptor for the given \p Phi node.
  /// (For explicit reduction variables)
  RecurrenceDescriptor &getRecurrenceDescrByPhi(PHINode *Phi) {
    assert(ExplicitReductions.count(Phi) && "Exp reduction var is not found");
    return ExplicitReductions[Phi].RD;
  }

  /// Return a pointer to reduction var using the \p Phi node.
  /// (Explicit only)
  Value *getReductionPtrByPhi(PHINode *Phi) {
    assert(ExplicitReductions.count(Phi) && "Exp reduction var is not found");
    return ExplicitReductions[Phi].RedVarPtr;
  }

  /// Returns True if V is an induction variable in this loop.
  bool isInductionVariable(const Value *V);

  /// Returns True if PN is a reduction variable in this loop.
  bool isReductionVariable(PHINode *PN) const {
    return ExplicitReductions.count(PN) || Reductions.count(PN);
  }

  /// Return true if \p PN is a Phi node for an explicitly specified
  /// reduction variable in this loop
  bool isExplicitReductionVariable(PHINode *PN) {
    return ExplicitReductions.count(PN);
  }

  /// Return true if \p PN is a Phi node for a reduction variable
  /// that was auto-detected duiring loop analysis.
  bool isImplicitReductionVariable(PHINode *PN) { return Reductions.count(PN); }

  Loop *getLoop() { return TheLoop; }

  PredicatedScalarEvolution &getPSE() { return PSE; }

  /// Adds \p Phi node to the list of induction variables.
  void addInductionPhi(PHINode *Phi, const InductionDescriptor &ID,
                       SmallPtrSetImpl<Value *> &AllowedExit);

  /// Returns the widest induction type.
  Type *getWidestInductionType() { return WidestIndTy; }

private:
  /// The loop that we evaluate.
  Loop *TheLoop;
  /// A wrapper around ScalarEvolution used to add runtime SCEV checks.
  /// Applies dynamic knowledge to simplify SCEV expressions in the context
  /// of existing SCEV assumptions. The analysis will also add a minimal set
  /// of new predicates if this is required to enable vectorization and
  /// unrolling.
  PredicatedScalarEvolution &PSE;
  /// Context object for the current function.
  LLVMContext *Context;
  /// Holds the integer induction variable. This is the counter of the
  /// loop.
  PHINode *Induction;
  /// Holds the reduction variables.
  ReductionList Reductions;
  /// Holds the explicitly-specified reduction variables.
  ExplicitReductionList ExplicitReductions;
  /// Holds the explicitly-specified reduction variables that
  /// calculated in loop using memory.
  InMemoryReductionList InMemoryReductions;
  /// Holds descriptors that describe user-defined reduction variables.
  UDRList UserDefinedReductions;

  /// Holds all of the induction variables that we found in the loop.
  /// Notice that inductions don't need to start at zero and that induction
  /// variables can be pointers.
  InductionList Inductions;

  /// Holds the widest induction type encountered.
  Type *WidestIndTy;

  /// A set of Phi nodes that may be used outside the loop.
  SmallPtrSet<Value *, 4> AllowedExit;

  /// Vector of in memory loop private values(allocas)
  PrivatesListTy Privates;

  /// List of explicit linears.
  LinearListTy Linears;

  /// Map of pointer values and stride
  DenseMap<Value *, int> PtrStrides;

  bool IsSimdLoop = false;

public:
  /// Add stride information for pointer \p Ptr.
  // Used for a temporary solution of teaching legality based on DA.
  // TODO: Is it okay to overwrite existing stride value?
  void addPtrStride(Value *Ptr, int Stride) { PtrStrides[Ptr] = Stride; }

  /// Erase pointer \p Ptr from the stride information map.
  // Used for a temporary solution of teaching legality based on DA.
  void erasePtrStride(Value *Ptr) { PtrStrides.erase(Ptr); }

  bool isExplicitReductionPhi(PHINode *Phi);

  // Return True if the specified value \p Val is reduction variable that
  // is written to the memory in each iteration.
  bool isInMemoryReduction(Value *Val) const;

  // Return true if the specified value \p Val is private.
  bool isLoopPrivate(Value *Val) const;

  // Return pointer to Linears map
  LinearListTy *getLinears() { return &Linears; }

  // Analyze all instruction between the SIMD clause and the loop to identify
  // any aliasing variables to the explicit SIMD descriptors.
  void collectPreLoopDescrAliases();

  // Analyze all instructions in loop post-exit to identify any aliasing
  // variables to the explicit SIMD descriptor.
  void collectPostExitLoopDescrAliases();

  VPlanBailoutRemark &getBailoutRemark() { return BR; }

  // Return the iterator-range to the list of privates loop-entities.
  // TODO: Windows compiler explicitly doesn't allow for const type specifier.
  inline decltype(auto) privates() const {
    return map_range(
        make_range(Privates.begin(), Privates.end()), [](auto &PrivatePair) {
          return const_cast<const PrivDescrTy *>(PrivatePair.second.get());
        });
  }

  // Return the iterator-range to the list of privates loop-entities values.
  inline decltype(auto) privateVals() const {
    return make_first_range(Privates);
  }

  // Return the iterator-range to the list of explicit reduction variables which
  // are of 'Pointer Type'.
  inline decltype(auto) explicitReductionVals() const {
    return make_filter_range(
        map_range(make_second_range(ExplicitReductions),
                  [](auto &Descr) { return Descr.RedVarPtr; }),
        [](auto *Val) { return Val->getType()->isPointerTy(); });
  }

  // Return the iterator-range to the list of in-memory reduction variables.
  inline decltype(auto) inMemoryReductionVals() const {
    return make_first_range(InMemoryReductions);
  }

  // Return the iterator-range to the list of user-defined reduction variables.
  inline decltype(auto) userDefinedReductions() const {
    return make_range(UserDefinedReductions.begin(),
                      UserDefinedReductions.end());
  }

  // Return the iterator-range to the list of 'linear' variables.
  inline decltype(auto) linearVals() const { return make_first_range(Linears); }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS) const;
  void dump() const { dump(errs()); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

private:
  /// Reports a reason for vectorization bailout. Always returns false.
  /// \p Message will appear both in the debug dump and the opt report remark.
  template <typename... Args>
  bool bailout(OptReportVerbosity::Level Level, OptRemarkID ID,
               std::string Message, Args &&...BailoutArgs);

  /// Reports a reason for vectorization bailout. Always returns false.
  /// \p Debug will appear in the debug dump, but not in the opt report remark.
  template <typename... Args>
  bool bailoutWithDebug(OptReportVerbosity::Level Level, OptRemarkID ID,
                        std::string Debug, Args &&...BailoutArgs);

  /// Initialize cached bailout remark data.
  void clearBailoutRemark() { BR.BailoutRemark = OptRemark(); }

  /// Store a variadic remark indicating the reason for not vectorizing a loop.
  /// Clients should pass string constants as std::string to avoid extra
  /// instantiations of this template function.
  template <typename... Args>
  void setBailoutRemark(OptReportVerbosity::Level BailoutLevel,
                        OptRemarkID BailoutID, Args &&...BailoutArgs) {
    BR.BailoutLevel = BailoutLevel;
    BR.BailoutRemark = OptRemark::get(
        *Context, static_cast<unsigned>(BailoutID),
        OptReportDiag::getMsg(BailoutID), std::forward<Args>(BailoutArgs)...);
  }

  /// Add an in memory non-POD private to the vector of private values.
  void addLoopPrivate(Value *PrivVal, Type *PrivTy, Function *Constr,
                      Function *Destr, Function *CopyAssign, PrivateKindTy Kind,
                      bool IsF90) {
    Privates.insert({PrivVal, std::make_unique<PrivDescrNonPODTy>(
                                  PrivVal, PrivTy, Kind, IsF90, Constr, Destr,
                                  CopyAssign)});
  }

  /// Add an in memory POD private to the vector of private values.
  void addLoopPrivate(Value *PrivVal, Type *PrivTy, PrivateKindTy Kind,
                      Type *F90DVElementType) {
    if (F90DVElementType)
      Privates.insert({PrivVal, std::make_unique<PrivDescrF90DVTy>(
                                    PrivVal, PrivTy, Kind, F90DVElementType)});
    else
      Privates.insert({PrivVal, std::make_unique<PrivDescrTy>(
                                    PrivVal, PrivTy, Kind, false /* isF90 */)});
  }

  /// Add linear value to Linears map
  void addLinear(Value *LinearVal, Type *EltTy, Type *EltPointeeTy,
                 Value *StepValue, bool IsIV) {
    assert(TheLoop->isLoopInvariant(StepValue) &&
           "Unexpected step value in linear clause");
    Linears[LinearVal] = std::make_tuple(EltTy, EltPointeeTy, StepValue, IsIV);
  }

  /// Add an explicit reduction variable \p V and the reduction recurrence kind.
  /// Additionally track if this is an inscan or complex type reduction.
  void addReduction(Value *V, Type *Ty, RecurKind Kind,
                    std::optional<InscanReductionKind> InscanRedKind,
                    bool IsComplex);

  /// Add a user-defined reduction variable \p V and functions that are needed
  /// for its initialization/finalization.
  void addReduction(Value *V, Type *Ty, Function *Combiner,
                    Function *Initializer, Function *Constr, Function *Destr,
                    std::optional<InscanReductionKind> InscanRedKind) {
    UserDefinedReductions.emplace_back(std::make_unique<UDRDescrTy>(
        V, Ty, Combiner, Initializer, Constr, Destr, InscanRedKind));
  }

  /// Parsing Min/Max reduction patterns.
  void parseMinMaxReduction(Value *V, RecurKind Kind,
                            std::optional<InscanReductionKind> InscanRedKind,
                            Type *Ty);
  /// Parsing arithmetic reduction patterns.
  void parseBinOpReduction(Value *V, RecurKind Kind,
                           std::optional<InscanReductionKind> InscanRedKind,
                           bool IsComplex, Type *Ty);

  /// Return true if the explicit reduction uses Phi nodes.
  bool doesReductionUsePhiNodes(Value *RedVarPtr, PHINode *&LoopHeaderPhiNode,
                                Value *&StartV);

  /// Return true if the reduction variable \p RedVarPtr is updated via
  /// in-memory reduction pattern. This is identified by checking if variable is
  /// stored inside the loop or used by a call. The found store or call
  /// instruction is captured in \p CallOrStore. A store is preferred, so if
  /// there are both store and call, the store is returned.
  bool isInMemoryReductionPattern(Value *RedVarPtr, Instruction *&CallOrStore);

  /// Check whether \p I is liveout.
  bool isLiveOut(const Instruction *I) const;

  /// Return operand of the \p Phi which is live-out. Live out phi or
  /// a Recurrence phi is expected.
  const Instruction *getLiveOutPhiOperand(const PHINode *Phi) const;
  Instruction *getLiveOutPhiOperand(PHINode *Phi) const {
    return const_cast<Instruction *>(
        // std::as_const is C++17.
        getLiveOutPhiOperand(const_cast<const PHINode *>(Phi)));
  }

  /// Check whether Phi can be private or private alias, update private
  /// descriptor and return true if so. Otherwise return false. Live out phi or
  /// a Recurrence phi is expected.
  bool checkAndAddAliasForSimdLastPrivate(const PHINode *Phi);

  /// If the \p Candidate is declared as private or is an alias for a private
  /// then return the descriptor of private. Otherwise nullptr is returned.
  PrivDescrTy *findPrivateOrAlias(const Value *Candidate) const;

  /// Set \p ExitI as instruction for private.
  void updatePrivateExitInst(PrivDescrTy *Priv, const Instruction *ExitI);

  /// Check the safety of aliasing of OMP clause variables outside of the loop.
  bool isAliasingSafe(DominatorTree &DT, const CallInst *RegionEntry);

  /// Check the safety of aliasing of particular class of clause-variables in \p
  /// LERange outside of the loop.
  template <typename LoopEntitiesRange>
  bool isEntityAliasingSafe(
      const LoopEntitiesRange &LERange,
      function_ref<bool(const Instruction *)> IsAliasInRelevantScope);

  /// Utility function to check whether given Instruction is end directive of
  /// OMP.SIMD directive.
  bool isEndDirective(Instruction *I) const;
};
} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELLOOPVECTORIZERLEGALITY_H

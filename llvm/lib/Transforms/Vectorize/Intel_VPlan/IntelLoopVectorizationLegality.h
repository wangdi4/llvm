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

#include "IntelVPlanLegalityDescr.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/IVDescriptors.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"

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
extern bool ForceComplexTyReductionVec;
extern bool ForceInscanReductionVec;

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

  // This enum lists reasons why vectorizer may decide not to vectorize a loop.
  enum class BailoutReason {
    ComplexTyReduction,
    // Fortran dope vectors support not implemented (CMPLRLLVM-10783)
    F90DopeVectorPrivate,
    F90DopeVectorReduction,
    // Loop entities framework does not support array reductions idiom. Bailout
    // to prevent incorrect vector code generatiion. Check - CMPLRLLVM-20621.
    ArrayReduction,
    // Loop entities framework does not support nonPOD [last]privates array.
    // Bailout to prevent incorrect vector code generatiion.
    // TODO: CMPLRLLVM-30686.
    ArrayLastprivateNonPod,
    ArrayPrivateNonPod,
    ArrayPrivate,
    UnsupportedReductionOp,
    InscanReduction,
    VectorCondLastPrivate, // need CG implementation
  };

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  static const char *getBailoutReasonStr(BailoutReason Code) {
    switch (Code) {
    case BailoutReason::ComplexTyReduction:
      return "Complex type reductions are not supported.\n";
    case BailoutReason::F90DopeVectorPrivate:
      return "F90 dope vector privates are not supported.\n";
    case BailoutReason::F90DopeVectorReduction:
      return "F90 dope vector reductions are not supported.\n";
    case BailoutReason::ArrayReduction:
      return "Cannot handle array reductions.\n";
    case BailoutReason::ArrayLastprivateNonPod:
      return "Cannot handle nonPOD array lastprivates.\n";
    case BailoutReason::ArrayPrivateNonPod:
      return "Cannot handle nonPOD array privates.\n";
    case BailoutReason::ArrayPrivate:
      return "Cannot handle array privates yet.\n";
    case BailoutReason::UnsupportedReductionOp:
      return "A reduction of this operation is not supported.\n";
    case BailoutReason::InscanReduction:
      return "Inscan reduction is not supported.\n";
    case BailoutReason::VectorCondLastPrivate:
      return "Conditional lastprivate of a vector type is not supported.\n";
    }
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  /// Return true if requested to proceed with vectorizing a complex type
  /// reduction
  static bool forceComplexTyReductionVec() {
    return ForceComplexTyReductionVec;
  }

  /// Return true if requested to vectorize a loop with inscan reduction.
  static bool forceInscanReductionVec() {
    return ForceInscanReductionVec;
  }

protected:
  VectorizationLegalityBase() = default;

  /// Import explicit data from WRLoop.
  bool EnterExplicitData(const WRNVecLoopNode *WRLp) {
    if (!WRLp)
      return true;
    return visitPrivates(WRLp) && visitLinears(WRLp) && visitReductions(WRLp);
  }

private:
  /// Imports any SIMD loop private amd listprivate information into Legality
  /// Return true on success.
  bool visitPrivates(const WRNVecLoopNode *WRLp) {
    for (LastprivateItem *Item : WRLp->getLpriv().items()) {
      if (Item->getIsF90DopeVector())
        return bailout(BailoutReason::F90DopeVectorPrivate);
      if (!visitLastPrivate(Item))
        return false;
    }

    for (PrivateItem *Item : WRLp->getPriv().items()) {
      if (Item->getIsF90DopeVector())
        return bailout(BailoutReason::F90DopeVectorPrivate);
      if (!visitPrivate(Item))
        return false;
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
    auto IsSupportedReduction = [this](const ReductionItem *Item) {
      if (Item->getIsArraySection())
        return bailout(BailoutReason::ArrayReduction);
      if (Item->getIsF90DopeVector())
        return bailout(BailoutReason::F90DopeVectorReduction);
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
        return bailout(BailoutReason::UnsupportedReductionOp);
      }
    };

    assert(
        (std::count_if(WRLp->getChildren().begin(), WRLp->getChildren().end(),
                       [](const WRegionNode *WRNode) {
                         return isa<WRNScanNode>(WRNode);
                       }) <= 1) &&
        "Not more than one scan region is expected!");
    DenseMap<uint64_t, InscanReductionKind> InscanReductionMap;
    // Locate the scan region if present.
    WRNScanNode *WRScan = nullptr;
    for (auto WRNode : WRLp->getChildren())
      if (WRNScanNode *WRSc = dyn_cast<WRNScanNode>(WRNode)) {
        WRScan = WRSc;
        break;
      }
    if (WRScan) {
      for (const InclusiveItem *Item : WRScan->getInclusive().items()) {
        InscanReductionMap.insert(
            {Item->getInscanIdx(), InscanReductionKind::Inclusive});
      }
      for (const ExclusiveItem *Item : WRScan->getExclusive().items()) {
        InscanReductionMap.insert(
            {Item->getInscanIdx(), InscanReductionKind::Exclusive});
      }
    }

    for (ReductionItem *Item : WRLp->getRed().items())
      if (!IsSupportedReduction(Item) ||
          !visitReduction(Item, InscanReductionMap))
        return false;
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
    Type *Type = nullptr;
    Value *NumElements = nullptr;
    std::tie(Type, NumElements, /* AddrSpace */ std::ignore) =
        VPOParoptUtils::getItemInfo(Item);

    assert(Type && "Missed OMP clause item type!");

    Type = adjustTypeIfArray(Type, NumElements);
    if (!Type)
      return bailout(BailoutReason::ArrayPrivate);

    ValueTy *Val = Item->getOrig<IR>();

    if (Item->getIsNonPod()) {
      if (isa<ArrayType>(Type) || NumElements)
        return bailout(BailoutReason::ArrayPrivateNonPod);
      addLoopPrivate(Val, Type, Item->getConstructor(), Item->getDestructor(),
                     nullptr /* no CopyAssign */, PrivateKindTy::NonLast,
                     Item->getIsF90NonPod());
      return true;
    }
    addLoopPrivate(Val, Type, PrivateKindTy::NonLast,
                   Item->getIsF90DopeVector());
    return true;
  }

  /// Register an explicit last private variable
  /// Return true if successfully consumed.
  bool visitLastPrivate(const LastprivateItem *Item) {
    Type *Type = nullptr;
    Value *NumElements = nullptr;
    std::tie(Type, NumElements, /* AddrSpace */ std::ignore) =
        VPOParoptUtils::getItemInfo(Item);
    assert(Type && "Missed OMP clause item type!");

    Type = adjustTypeIfArray(Type, NumElements);
    if (!Type)
      return bailout(BailoutReason::ArrayPrivate);

    ValueTy *Val = Item->getOrig<IR>();

    if (Item->getIsNonPod()) {
      if (isa<ArrayType>(Type) || NumElements)
        return bailout(BailoutReason::ArrayLastprivateNonPod);
      addLoopPrivate(Val, Type, Item->getConstructor(), Item->getDestructor(),
                     Item->getCopyAssign(), PrivateKindTy::Last,
                     Item->getIsF90NonPod());
      return true;
    }

    // Until CG to extract vector by non-const index is implemented.
    if (Item->getIsConditional() && Type->isVectorTy())
      return bailout(BailoutReason::VectorCondLastPrivate);

    addLoopPrivate(Val, Type,
                   Item->getIsConditional() ? PrivateKindTy::Conditional
                                            : PrivateKindTy::Last,
                   Item->getIsF90DopeVector());
    return true;
  }

  /// Register explicit linear variable
  void visitLinear(const LinearItem *Item) {
    Type *PointeeTy = nullptr;
    Type *Type = nullptr;
    Value *NumElements = nullptr;
    std::tie(Type, NumElements, /* AddrSpace */ std::ignore) =
        VPOParoptUtils::getItemInfo(Item);
    // TODO: Move to VPOParoptUtils::getItemInfo
    if (Item->getIsTyped() && Item->getIsPointerToPointer()) {
      PointeeTy = Item->getPointeeElementTypeFromIR();
    }
    assert(Type && "Missed OMP clause item type!");

    // NumElements == nullptr by convention means the number is 1.
    // Arrays are not allowed by OMP standard thus any values including
    // a constant are illegal for linears.
    assert(!isa<ArrayType>(Type) && !NumElements &&
           "Unexpected number of elements");

    ValueTy *Val = Item->getOrig<IR>();
    ValueTy *Step = Item->getStep<IR>();
    addLinear(Val, Type, PointeeTy, Step);
  }

  /// Register explicit reduction variable
  /// Return true if successfully consumed.
  bool visitReduction(const ReductionItem *Item,
                      DenseMap<uint64_t, InscanReductionKind> &InscanMap) {
    if (!forceComplexTyReductionVec() && Item->getIsComplex())
      return bailout(BailoutReason::ComplexTyReduction);

    if (!forceInscanReductionVec() && Item->getIsInscan())
      return bailout(BailoutReason::InscanReduction);

    Type *Type = nullptr;
    Value *NumElements = nullptr;
    std::tie(Type, NumElements, /* AddrSpace */ std::ignore) =
        VPOParoptUtils::getItemInfo(Item);
    assert(Type && "Missed OMP clause item type!");

    if (isa<ArrayType>(Type) || NumElements)
      return bailout(BailoutReason::ArrayReduction);

    ValueTy *Val = Item->getOrig<IR>();
    RecurKind Kind = getReductionRecurKind(Item, Type);
    // Capture functions for init/finalization for UDRs.
    if (Kind == RecurKind::Udr) {
      addReduction(Val, Item->getCombiner(), Item->getInitializer(),
                   Item->getConstructor(), Item->getDestructor());
    } else if (Item->getIsInscan()) {
      assert(InscanMap.count(Item->getInscanIdx()) &&
             "The inscan item must be present in the separating pragma");
      addReduction(Val, Kind, InscanMap[Item->getInscanIdx()]);
    } else
      addReduction(Val, Kind, None);
    return true;
  }

  // Set of thunks to a parent methods
  bool bailout(BailoutReason Code) {
    return static_cast<LegalityTy *>(this)->bailout(Code);
  }

  void addLoopPrivate(ValueTy *Val, Type *Ty, Function *Constr, Function *Destr,
                      Function *CopyAssign, PrivateKindTy Kind, bool IsF90) {
    return static_cast<LegalityTy *>(this)->addLoopPrivate(
        Val, Ty, Constr, Destr, CopyAssign, Kind, IsF90);
  }

  void addLoopPrivate(ValueTy *Val, Type *Ty, PrivateKindTy Kind, bool IsF90) {
    return static_cast<LegalityTy *>(this)->addLoopPrivate(Val, Ty, Kind,
                                                           IsF90);
  }

  void addLinear(ValueTy *Val, Type *Ty, Type *PointeeType, ValueTy *Step) {
    return static_cast<LegalityTy *>(this)->addLinear(Val, Ty, PointeeType,
                                                      Step);
  }

  void addReduction(ValueTy *V, RecurKind Kind,
                    Optional<InscanReductionKind> InscanRedKind) {
    return static_cast<LegalityTy *>(this)->addReduction(
      V, Kind, InscanRedKind);
  }

  void addReduction(ValueTy *V, Function *Combiner, Function *Initializer,
                    Function *Constr, Function *Destr) {
    return static_cast<LegalityTy *>(this)->addReduction(
        V, Combiner, Initializer, Constr, Destr);
  }
};

class VPOVectorizationLegality final
    : public VectorizationLegalityBase<VPOVectorizationLegality> {
  // Explicit vpo:: to workaround gcc bug
  // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52625
  template <typename LegalityTy> friend class vpo::VectorizationLegalityBase;

public:
  VPOVectorizationLegality(Loop *L, PredicatedScalarEvolution &PSE, Function *F)
      : TheLoop(L), PSE(PSE), Induction(nullptr), WidestIndTy(nullptr) {}

  struct ExplicitReductionDescr {
    RecurrenceDescriptor RD;
    Value *RedVarPtr;
    Optional<InscanReductionKind> InscanRedKind;
  };
  struct InMemoryReductionDescr {
    RecurKind Kind;
    Optional<InscanReductionKind> InscanRedKind;
    Instruction *UpdateInst;
  };

  /// Returns true if it is legal to vectorize this loop.
  bool canVectorize(DominatorTree &DT, const WRNVecLoopNode *WRLp);

  using DescrValueTy = DescrValue<Value>;
  using DescrWithAliasesTy = DescrWithAliases<Value>;
  using PrivDescrTy = PrivDescr<Value>;
  using PrivDescrNonPODTy = PrivDescrNonPOD<Value>;
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
                           Value * /*Step*/>>;

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
  bool bailout(BailoutReason Code);

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
                      bool IsF90) {
    Privates.insert(
        {PrivVal, std::make_unique<PrivDescrTy>(PrivVal, PrivTy, Kind, IsF90)});
  }

  /// Add linear value to Linears map
  void addLinear(Value *LinearVal, Type *EltTy, Type *EltPointeeTy,
                 Value *StepValue) {
    assert(TheLoop->isLoopInvariant(StepValue) &&
           "Unexpected step value in linear clause");
    Linears[LinearVal] = std::make_tuple(EltTy, EltPointeeTy, StepValue);
  }

  /// Add an explicit reduction variable \p V and the reduction recurrence kind.
  void addReduction(Value *V, RecurKind Kind,
                    Optional<InscanReductionKind> InscanRedKind);

  /// Add a user-defined reduction variable \p V and functions that are needed
  /// for its initialization/finalization.
  void addReduction(Value *V, Function *Combiner, Function *Initializer,
                    Function *Constr, Function *Destr) {
    UserDefinedReductions.emplace_back(
        std::make_unique<UDRDescrTy>(V, Combiner, Initializer, Constr, Destr));
  }

  /// Parsing Min/Max reduction patterns.
  void parseMinMaxReduction(Value *V, RecurKind Kind);
  /// Parsing arithmetic reduction patterns.
  void parseBinOpReduction(Value *V, RecurKind Kind,
                           Optional<InscanReductionKind> InscanRedKind);

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

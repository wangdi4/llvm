//
//   Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// This file provides VPLoop-based analyses:
///  - VPLoopEntity, which is a base class for loop entities like inductions,
///    reductions, private.
///  - VPlanScalVecAnalysis, which is used to keep information about code
///    generation type for each VPValue: scalar and/or vector.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLOOPANALYSIS_H
#define LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLOOPANALYSIS_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Analysis/IVDescriptors.h"
#include <map>

using namespace llvm;

namespace llvm {

class ScalarEvolution;
class LoopInfo;

namespace vpo {

class VPValue;
class VPlanVector;
class IntelVPlanUtils;
class VPLoop;
class VPlan;
class VPValue;
class VPConstant;
class VPInstruction;
class VPPHINode;
class VPBasicBlock;
class VPBuilder;
class VPAllocatePrivate;
class VPReductionInitScalar;
class VPReductionFinal;
class VPLoadStoreInst;
class VPDominatorTree;
class VPCompressExpandInit;
class VPCompressExpandFinal;

/// Base class for loop entities
class VPLoopEntity {
public:
  using LinkedVPValuesTy = SetVector<VPValue *>;

  enum {
    Reduction,
    IndexReduction,
    InscanReduction,
    UserDefinedReduction,
    Induction,
    Private,
    PrivateNonPOD,
    CompressExpand,
  };
  unsigned char getID() const { return SubclassID; }

  virtual ~VPLoopEntity() = 0;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const { dump(errs()); }
  virtual void dump(raw_ostream &OS) const = 0;
#endif

  void setIsMemOnly(bool V) { IsMemOnly = V; }
  bool getIsMemOnly() const { return IsMemOnly; }

  const LinkedVPValuesTy &getLinkedVPValues() const { return LinkedVPValues; }
  void addLinkedVPValue(VPValue *Val) {
    if (Val != nullptr)
      LinkedVPValues.insert(Val);
  }

  virtual Type *getAllocatedType() const = 0;

protected:
  // No need for public constructor.
  explicit VPLoopEntity(unsigned char Id, bool IsMem)
      : IsMemOnly(IsMem), SubclassID(Id){};

  bool IsMemOnly;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printLinkedValues(raw_ostream &OS) const;
#endif // NDEBUG || LLVM_ENABLE_DUMP

private:
  const unsigned char SubclassID;
  LinkedVPValuesTy LinkedVPValues;
};

/// Recurrence descriptor
class VPReduction
    : public VPLoopEntity,
      public RecurrenceDescriptorTempl<VPValue, VPInstruction, VPValue *> {
  using RDTempl = RecurrenceDescriptorTempl<VPValue, VPInstruction, VPValue *>;

public:
  VPReduction(VPValue *Start, VPInstruction *Exit, RecurKind RdxKind,
              FastMathFlags FMF, Type *RT, bool Signed, bool IsMemOnly = false,
              unsigned char Id = Reduction)
      : VPLoopEntity(Id, IsMemOnly),
        RDTempl(Start, Exit, RdxKind, FMF, RT, Signed, false) {}

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPLoopEntity *V) {
    return V->getID() == Reduction;
  }

  unsigned getReductionOpcode() const {
    return getReductionOpcode(getRecurrenceKind());
  }

  Type *getAllocatedType() const override { return getRecurrenceType(); }

  bool isMinMax() const {
    return RecurrenceDescriptorData::isMinMaxRecurrenceKind(
        getRecurrenceKind());
  }

  bool isSelectCmp() const {
    return RecurrenceDescriptorData::isSelectCmpRecurrenceKind(
        getRecurrenceKind());
  }

  virtual StringRef getNameSuffix() const {
    return isMinMax() ? "minmax.red" : "red";
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  virtual void dump(raw_ostream &OS) const override;
#endif
  static unsigned getReductionOpcode(RecurKind K);
};

/// Descriptor for user-defined reduction.
class VPUserDefinedReduction : public VPReduction {
public:
  VPUserDefinedReduction(Function *Combiner, Function *Initializer,
                         Function *Ctor, Function *Dtor, VPValue *Start,
                         FastMathFlags FMF, Type *RT, bool Signed,
                         bool IsMemOnly = false)
      : VPReduction(Start, nullptr /*Exit*/, RecurKind::Udr, FMF, RT, Signed,
                    IsMemOnly, UserDefinedReduction),
        Combiner(Combiner), Initializer(Initializer), Ctor(Ctor), Dtor(Dtor) {}

  Function *getCombiner() const { return Combiner; }
  Function *getInitializer() const { return Initializer; }
  Function *getCtor() const { return Ctor; }
  Function *getDtor() const { return Dtor; }

  virtual StringRef getNameSuffix() const override { return "udr"; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS) const override;
#endif

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPLoopEntity *V) {
    return V->getID() == UserDefinedReduction;
  }

private:
  // Functions for initialization/finalization of UDRs.
  Function *Combiner;
  Function *Initializer;
  Function *Ctor;
  Function *Dtor;
};

/// Descriptor for inscan reduction.
class VPInscanReduction : public VPReduction {
public:
  VPInscanReduction(InscanReductionKind InscanRedKind, VPValue *Start,
                    VPInstruction *Exit, RecurKind RdxKind, FastMathFlags FMF,
                    Type *RT, bool Signed, bool IsMemOnly = false)
    : VPReduction(Start, Exit, RdxKind, FMF, RT, Signed, IsMemOnly,
                  InscanReduction),
      InscanRedKind(InscanRedKind) {}

  InscanReductionKind getInscanKind() const { return InscanRedKind; }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPLoopEntity *V) {
    return V->getID() == InscanReduction;
  }

  virtual StringRef getNameSuffix() const override {
    return "inscan.red";
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS) const override;
#endif

private:
  InscanReductionKind InscanRedKind;
};

/// Descriptor of the index part of min/max+index reduction.
/// In addition to VPReduction, contains pointer to a parent min/max reduction
/// descriptor.
class VPIndexReduction : public VPReduction {
public:
  VPIndexReduction(const VPReduction *Parent, VPValue *Start,
                   VPInstruction *Exit, Type *RT, bool Signed, bool ForLast,
                   bool IsLinIndex, bool IsMemOnly = false)
      : VPReduction(Start, Exit,
                    ForLast ? (Signed ? RecurKind::SMax : RecurKind::UMax)
                            : (Signed ? RecurKind::SMin : RecurKind::UMin),
                    FastMathFlags(), RT, Signed, IsMemOnly,
                    IndexReduction),
        ParentRed(Parent), IsLinearIndex(IsLinIndex) {
    assert((Parent && Parent->isMinMax()) && "Incorrect parent reduction");
  }

  const VPReduction *getParentReduction() const { return ParentRed; }
  void replaceParentReduction(const VPReduction *NewParent) {
    assert((NewParent && NewParent->isMinMax()) &&
           "Incorrect parent reduction");
    ParentRed = NewParent;
  }

  bool isLinearIndex() const { return IsLinearIndex; }
  void setIsLinearIndex(bool V) { IsLinearIndex = V; }

  // Return true if it's last index, false otherwise.
  bool isForLast() const {
    return getRecurrenceKind() == RecurKind::SMax ||
           getRecurrenceKind() == RecurKind::UMax;
  }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPLoopEntity *V) {
    return V->getID() == IndexReduction;
  }

  StringRef getNameSuffix() const override{
    return isLinearIndex() ? "mono.idx.red" : "idx.red";
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS) const override;
#endif
private:
  // Parent reduction, either min or max.
  const VPReduction *ParentRed;

  // Flag shows that this entity is created for a linear index. This rules
  // the last value calculation. If the flag is true then the value is simply
  // calculated as horizontal min or max value of indexes in the vector that
  // correspond to parent reduction last value. Otherwise, the last value is
  // taken from vector by position that corresponds to the min/max linear index.
  // For example, we have the following values for min+min_index reduction
  //    if (b>a[i]) {
  //      b = a[i];  // min value
  //      n = i;     // min linear index
  //      t = log(c[i/3]);  // other non-linear index
  //    }
  // main value : {1, 2, 1, 3}  -> last value is 1.
  // linear index: {10, 4, 6, 2} -> last value is 6, min index of value 1.
  // non-linear index {20, 2, 60, 10} -> last value is 60, at position of
  //                                     min index.
  // A min/max+index idiom should contain at least one linear index. If
  // there is no such indexes then it is added using main induction
  // variable.
  //
  // The following relations are established between main reduction and linear
  // and non-linear indexes:
  // - main reduction is a parent for all linear indexes.
  // - the first encountered linear index is a parent for all non-linear indexes.
  //
  bool IsLinearIndex;
};

/// Induction descriptor.
class VPInduction
    : public VPLoopEntity,
      public InductionDescriptorTempl<VPValue, VPInstruction, VPValue *> {
  friend class VPLoopEntityList;

public:
  VPInduction(VPValue *Start, InductionKind K, VPValue *Step,
              int StepMultiplier, Type *StepTy, const SCEV *StepSCEV,
              VPValue *StartVal, VPValue *EndVal, VPInstruction *InductionOp,
              bool IsMemOnly = false,
              unsigned int Opc = Instruction::BinaryOpsEnd)
      : VPLoopEntity(Induction, IsMemOnly),
        InductionDescriptorTempl(Start, K, InductionOp), Step(Step),
        StepMultiplier(StepMultiplier), StepTy(StepTy), StepSCEV(StepSCEV),
        StartVal(StartVal), EndVal(EndVal), IndOpcode(Opc) {
    assert((getInductionBinOp() || IndOpcode != Instruction::BinaryOpsEnd) &&
           "Induction opcode should be set");
  }

  /// Return stride
  VPValue *getStep() const { return Step; }
  int getStepMultiplier() const { return StepMultiplier; }
  Type *getStepType() const { return StepTy; }
  const SCEV *getStepSCEV() const { return StepSCEV; }

  /// Return lower/upper range of values for induction
  VPValue *getStartVal() const { return StartVal; }
  VPValue *getEndVal() const { return EndVal; }

  Type *getAllocatedType() const override { return getInductionType(); }

  /// Return true if the induction needs a close-form generation at the
  /// beginning of the loop body. These cases are: explicit linears w/o
  /// explicit increments inside loop body and inductions that have a use
  /// after increment. E.g.
  ///
  /// The case when no close-form is needed.
  /// for(...) {
  ///   a[i] = i;  // i is up-to-date={0, step, 2*step, ...}
  ///   i += step; // this is converted to i += VF*step;
  /// }
  /// The two cases when the close form is needed.
  /// for(...) {
  ///   i += step; // can't convert this into i += step*VF; because
  ///   a[i] = i;  // here we should have {step, 2*step, 3*step,...}
  /// }
  /// #pragma omp simd linear(i:step)
  /// for(...) {
  ///   foo(&i);   // it's supposed that i is updated inside foo()
  ///   a[i] = i;  // here we should have {step, 2*step, 3*step,...}
  /// }
  ///
  bool needCloseForm() const { return NeedCloseForm; }

  void setNeedCloseForm(bool Val) { NeedCloseForm = Val; }

  /// Returns binary opcode of the induction operator. Hides parent's method.
  unsigned int getInductionOpcode() const;

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPLoopEntity *V) {
    return V->getID() == Induction;
  }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS) const override;
#endif
private:
  VPValue *Step;
  int StepMultiplier; // For linear pointer, holds pointee type
  Type *StepTy;       // For linear pointer, holds step type
  const SCEV *StepSCEV; // Holds auto-detected SCEV
  VPValue *StartVal;
  VPValue *EndVal;
  unsigned int IndOpcode; // Explicitly set opcode.
  bool NeedCloseForm = false;
};

/// Private descriptor. Privates can be declared explicitly or detected
/// by analysis.
class VPPrivate : public VPLoopEntity {
  friend class VPLoopEntityList;

public:
  enum class PrivateKind {
    NonLast,    // A simple non-live out private.
    Last,       // The lastvalue is used outside the loop or it was declared as
                // "lastprivate" in directive.
    Conditional // The assignment of last private is under condition or it's
                // declared as "lastprivate:conditional" in the directive.
  };

  // An additional classification flag for privates without assigned exit
  // instruction.
  enum class PrivateTag {
    PTRegisterized, // Not specified.
    PTArray,        // Private of an array type.
    PTInMemory,     // In-memory allocated private.
    PTNonPod,       // Non-POD private.
  };

  // Explicit destructor to drop references to alias VPInstructions. This is
  // needed to avoid memory leak in cases where aliases are not lowered into
  // instructions in VPlan CFG.
  ~VPPrivate();

  // Currently only used for VPPrivates. In future, this can be hoisted to
  // VPLoopEntity. The map links a VPValue (typically VPExternalDef), that was
  // created for a private memory alias, with the pair <VPInstruction *,
  // Instruction *>. The Instruction in this pair is underlying instruction that
  // creates the alias in underlying IR, VPInstruction is its VPlan equivalent.
  using VPEntityAliasesTy =
      MapVector<VPValue *, std::pair<VPInstruction *, const Instruction *>>;

  VPPrivate(VPInstruction *ExitI, VPEntityAliasesTy &&InAliases, PrivateKind K,
            bool Explicit, Type *AllocatedTy, bool IsMemOnly = false,
            bool IsF90 = false, unsigned char Id = Private)
      : VPLoopEntity(Id, IsMemOnly), Kind(K), IsExplicit(Explicit),
        TagOrExit(ExitI), Aliases(std::move(InAliases)),
        AllocatedType(AllocatedTy), IsF90(IsF90) {}

  VPPrivate(PrivateTag PTag, VPEntityAliasesTy &&InAliases, PrivateKind K,
            bool Explicit, Type *AllocatedType, bool IsMemOnly = false,
            bool IsF90 = false, unsigned char Id = Private)
      : VPLoopEntity(Id, IsMemOnly), Kind(K), IsExplicit(Explicit),
        TagOrExit(PTag), Aliases(std::move(InAliases)),
        AllocatedType(AllocatedType), IsF90(IsF90) {}

  bool isConditional() const { return Kind == PrivateKind::Conditional; }
  bool isLast() const { return Kind != PrivateKind::NonLast; }
  bool isExplicit() const { return IsExplicit; }
  bool isF90() const { return IsF90; }

  PrivateTag getPrivateTag() const {
    assert(hasPrivateTag() && "expected tag");
    return TagOrExit.T;
  }
  VPInstruction *getExitInst() const {
    assert(hasExitInstr() && "expected instruction");
    return TagOrExit.I;
  }

  Type *getAllocatedType() const override { return AllocatedType; }

  void setExitInst(VPInstruction *I) { TagOrExit.assignInstr(I); }

  bool hasExitInstr() const { return TagOrExit.IsInstr; }
  bool hasPrivateTag() const { return !TagOrExit.IsInstr; }

  // TODO: Consider making this method virtual and hence available to all
  // entities.
  // Iterator-range for the aliases-set.
  iterator_range<VPEntityAliasesTy::const_iterator> aliases() const {
    return iterator_range<VPEntityAliasesTy::const_iterator>(Aliases.begin(),
                                                             Aliases.end());
  }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPLoopEntity *V) {
    return V->getID() == Private || V->getID() == PrivateNonPOD;
  }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS) const override;
#endif

private:
  struct PrivData {
    bool IsInstr : 1;
    // Storage for the private classification flag or exit instruction.
    union {
      PrivateTag T;
      VPInstruction *I;
    };
    PrivData(PrivateTag AT) : IsInstr(false), T(AT) {}
    PrivData(VPInstruction *AI) : IsInstr(true), I(AI) {}

    void assignInstr(VPInstruction *AI) {
      assert(IsInstr && "expected instruction");
      I = AI;
    }
  };

  // Kind of private;
  PrivateKind Kind;

  // Is defined explicitly.
  bool IsExplicit;

  // The Exit Instruction or an additional classification flag
  PrivData TagOrExit;

  // Map that stores the VPExternalDef->VPInstruction mapping. These are alias
  // instructions to existing loop-private, which are outside the loop-region.
  VPEntityAliasesTy Aliases;

  // Type of the allocated memory.
  Type *AllocatedType;

  // Is private F90 directive.
  bool IsF90;
};

class VPPrivateNonPOD : public VPPrivate {
public:
  VPPrivateNonPOD(VPEntityAliasesTy &&InAliases, PrivateKind K, bool IsExplicit,
                  Function *Ctor, Function *Dtor, Type *AllocatedTy,
                  Function *CopyAssign, bool IsF90)
      : VPPrivate(PrivateTag::PTNonPod, std::move(InAliases), K, IsExplicit,
                  AllocatedTy, true /*IsMemOnly*/, IsF90, PrivateNonPOD),
        Ctor(Ctor), Dtor(Dtor), CopyAssign(CopyAssign) {}

  Function *getCtor() const { return Ctor; }
  Function *getDtor() const { return Dtor; }
  Function *getCopyAssign() const { return CopyAssign; }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPLoopEntity *V) {
    return V->getID() == PrivateNonPOD;
  }

  static inline bool classof(const VPPrivate *V) {
    return V->getID() == PrivateNonPOD;
  }

private:
  Function *Ctor;
  Function *Dtor;
  Function *CopyAssign;
};

class VPCompressExpandIdiom : public VPLoopEntity {
  friend class VPLoopEntityList;

public:
  VPCompressExpandIdiom(VPPHINode *RecurrentPhi, VPValue *LiveIn,
                        VPInstruction *LiveOut, int64_t TotalStride,
                        const SmallVectorImpl<VPInstruction *> &Increments,
                        const SmallVectorImpl<VPLoadStoreInst *> &Stores,
                        const SmallVectorImpl<VPLoadStoreInst *> &Loads,
                        const SmallVectorImpl<VPInstruction *> &Indices)
      : VPLoopEntity(CompressExpand, false), RecurrentPhi(RecurrentPhi),
        LiveIn(LiveIn), LiveOut(LiveOut), TotalStride(TotalStride),
        Increments(Increments.begin(), Increments.end()),
        Stores(Stores.begin(), Stores.end()), Loads(Loads.begin(), Loads.end()),
        Indices(Indices.begin(), Indices.end()) {}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS) const override;
#endif

  Type *getAllocatedType() const override;

  VPPHINode *getRecurrentPhi() const { return RecurrentPhi; }
  VPValue *getLiveIn() const { return LiveIn; }
  VPInstruction *getLiveOut() const { return LiveOut; }
  int64_t getTotalStride() const { return TotalStride; }

  VPCompressExpandInit *getInit() const { return Init; }
  VPCompressExpandFinal *getFinal() const { return Final; }

  auto increments() const {
    return make_range(Increments.begin(), Increments.end());
  }
  auto stores() const { return make_range(Stores.begin(), Stores.end()); }
  auto loads() const { return make_range(Loads.begin(), Loads.end()); }
  auto indices() const { return make_range(Indices.begin(), Indices.end()); }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPLoopEntity *V) {
    return V->getID() == CompressExpand;
  }

  static inline bool classof(const VPPrivate *V) {
    return V->getID() == CompressExpand;
  }

private:
  VPPHINode *RecurrentPhi;
  VPValue *LiveIn;
  VPInstruction *LiveOut;
  int64_t TotalStride;

  VPCompressExpandInit *Init = nullptr;
  VPCompressExpandFinal *Final = nullptr;

  SmallSetVector<VPInstruction *, 4> Increments;
  SmallVector<VPLoadStoreInst *, 4> Stores;
  SmallVector<VPLoadStoreInst *, 4> Loads;
  SmallSetVector<VPInstruction *, 4> Indices;
};

/// Complimentary class that describes memory locations of the loop entities.
/// The VPLoopEntity is linked with this descriptor through its ExitingValue
/// field.
class VPLoopEntityMemoryDescriptor {
public:
  VPLoopEntityMemoryDescriptor(VPLoopEntity *LoopEn, VPValue *AllocaInst)
      : LE(LoopEn), MemoryPtr(AllocaInst) {}

  /// Return pointer to the linked VPLoopEntity
  VPLoopEntity *getVPLoopEntity() const { return LE; }

  /// Return memory address of this var.
  VPValue *getMemoryPtr() const { return MemoryPtr; }

  /// Sometime keeping an entity in memory is unnecessary, i.e. when it's
  /// address is not escaping either to a function call or through a pointer.
  /// This bit shows that we can generate code using a register, eliminating
  /// load/store inside the loop and storing just last value.
  bool canRegisterize() const { return CanRegisterize; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS) const;
#endif
private:
  friend class VPLoopEntityList;

  // Pointer to the VPLoopEntity. A VPLoopEntity need not have a corresponding
  // VPLoopEntityMemoryDescriptor, but a VPLoopEntityMemoryDescriptor must
  // always have a corresponding VPLoopEntity.
  VPLoopEntity *LE;

  // Interface for analyzer to set the bits.
  void setCanRegisterize(bool Val) { CanRegisterize = Val; }

  // Allocation instruction.
  VPValue *MemoryPtr;

  bool CanRegisterize = false;

};

/// Class to hold/analyze VPLoop enitites - reductions, inductions, and
/// privates. This is a proxy for data classified by legalization analysis, to
/// be able to operate in terms of VPValue. It encapsulates also some additional
/// analysis that can be done on VPLoop body.
class VPLoopEntityList {
  using InductionKind = VPInduction::InductionKind;
  using VPEntityAliasesTy = VPPrivate::VPEntityAliasesTy;

public:
  /// Importing error indicators.
  enum class ImportError {
    None = 0,
    Reduction,
    Induction,
    Private,
    ComressExpand,
  };

  /// Add reduction described by \p K, \p MK, and \p Signed,
  /// with starting instruction \p Instr, incoming value \p Incoming, exiting
  /// instruction \p Exit and alloca-instruction \p AI.
  VPReduction *addReduction(VPInstruction *Instr, VPValue *Incoming,
                            VPInstruction *Exit, RecurKind K,
                            FastMathFlags FMF, Type *RT, bool Signed,
                            Optional<InscanReductionKind> InscanRedKind,
                            VPValue *AI = nullptr, bool ValidMemOnly = false);

  /// Add index part of min/max+index reduction with parent (min/max) reduction
  /// \p Parent, starting instruction \pInstr, incoming value \p Incoming,
  /// exiting instruction \p Exit, \p Signed data type \p RT, and
  /// alloca-instruction \p AI. The \p ForLast flag indicates whether we need
  /// the last index or the first one.
  VPIndexReduction *addIndexReduction(VPInstruction *Instr,
                                      const VPReduction *Parent,
                                      VPValue *Incoming, VPInstruction *Exit,
                                      Type *RT, bool Signed, bool ForLast,
                                      bool IsLinNdx, VPValue *AI = nullptr,
                                      bool ValidMemOnly = false);

  /// Add user-defined reduction described by \p Combiner, \p Initializer, \p
  /// RedTy and \p Signed, with incoming value \p Incoming and
  /// alloca-instruction \p AI.
  VPUserDefinedReduction *
  addUserDefinedReduction(Function *Combiner, Function *Initializer,
                          Function *Ctor, Function *Dtor, VPValue *Incoming,
                          FastMathFlags FMF, Type *RedTy, bool Signed,
                          VPValue *AI, bool ValidMemOnly);

  /// Add induction of kind \p K, with opcode \p Opc or binary operation
  /// \p InductionOp, starting instruction \pStart, incoming value
  /// \p Incoming, stride \p Step, and alloca-instruction \p AI.
  VPInduction *addInduction(VPInstruction *Start, VPValue *Incoming,
                            InductionKind K, VPValue *Step, int StepMultiplier,
                            Type *StepTy, const SCEV *StepSCEV,
                            VPValue *StartVal, VPValue *EndVal,
                            VPInstruction *InductionOp, unsigned int Opc,
                            VPValue *AI = nullptr, bool ValidMemOnly = false);

  /// Add private corresponding to \p Alloca along with the final store
  /// instruction which writes to the private memory witin the for-loop. Also
  /// store other relavant attributes of the private like the conditional, last
  /// and explicit.
  VPPrivate *addPrivate(VPInstruction *ExitI, VPEntityAliasesTy &PtrAliases,
                        VPPrivate::PrivateKind K, bool Explicit,
                        Type *AllocatedTy, VPValue *AI = nullptr,
                        bool ValidMemOnly = false, bool IsF90 = false);

  /// Add private corresponding to \p Alloca along with the specified private
  /// tag. Also store other relavant attributes of the private like the
  /// conditional, last and explicit.
  VPPrivate *addPrivate(VPPrivate::PrivateTag Tag,
                        VPEntityAliasesTy &PtrAliases, VPPrivate::PrivateKind K,
                        bool Explicit, Type *AllocatedTy, VPValue *AI = nullptr,
                        bool ValidMemOnly = false, bool IsF90 = false);

  VPPrivateNonPOD *addNonPODPrivate(VPEntityAliasesTy &PtrAliases,
                                    VPPrivate::PrivateKind K, bool Explicit,
                                    Function *Ctor, Function *Dtor,
                                    Function *CopyAssign, bool IsF90,
                                    Type *AllocatedTy = nullptr,
                                    VPValue *AI = nullptr);

  VPCompressExpandIdiom *
  addCompressExpandIdiom(VPPHINode *RecurrentPhi, VPValue *LiveIn,
                         VPInstruction *LiveOut, int64_t TotalStride,
                         const SmallVectorImpl<VPInstruction *> &Increments,
                         const SmallVectorImpl<VPLoadStoreInst *> &Stores,
                         const SmallVectorImpl<VPLoadStoreInst *> &Loads,
                         const SmallVectorImpl<VPInstruction *> &Indices);

  /// Final stage of importing data from IR. Go through all imported descriptors
  /// and check/create links to VPInstructions.
  void finalizeImport(void);

  /// Return reduction descriptor if \p VPVal is used in calculations of
  /// reduction. Not only starting phis are mapped, the instructions inside loop
  /// body should check this for proper masking of reduction operations.
  const VPReduction *getReduction(const VPValue *VPVal) const {
    return find(ReductionMap, VPVal);
  }

  /// Return induction descriptor by \p VPVal. Shows whether VPVal is used in
  /// calculations of induction.
  const VPInduction *getInduction(const VPValue *VPVal) const {
    return find(InductionMap, VPVal);
  }

  /// Return private descriptor by \p VPVal.
  const VPPrivate *getPrivate(const VPValue *VPVal) const {
    return find(PrivateMap, VPVal);
  }

  /// Return compress/expand idiom descriptor by \p VPVal.
  const VPCompressExpandIdiom *getCompressExpandIdiom(const VPValue *VPVal) const {
    return find(ComressExpandIdiomMap, VPVal);
  }

  /// Get descriptor for an entity's memory.
  const VPLoopEntityMemoryDescriptor *
  getMemoryDescriptor(const VPLoopEntity *E) const {
    auto It = MemoryDescriptors.find(E);
    if (It != MemoryDescriptors.end())
      return It->second.get();
    return nullptr;
  }

  /// Get original memory pointer for an entity. Returns nullptr for
  /// in-register entities.
  const VPValue *getOrigMemoryPtr(const VPLoopEntity *E) const {
    const VPLoopEntityMemoryDescriptor *Descr = getMemoryDescriptor(E);
    return Descr ? Descr->getMemoryPtr() : nullptr;
  }

  /// Get descriptor for a memory descriptor by its alloca instruction.
  const VPLoopEntityMemoryDescriptor *
  getMemoryDescriptor(const VPValue *AI) const {
    auto It = MemInstructions.find(AI);
    if (It != MemInstructions.end())
      return It->second;
    return nullptr;
  }

  /// Get mutable descriptor for a memory descriptor by its alloca instruction.
  VPLoopEntityMemoryDescriptor *getMemoryDescriptor(const VPValue *AI) {
    return const_cast<VPLoopEntityMemoryDescriptor *>(
        static_cast<const VPLoopEntityList *>(this)->getMemoryDescriptor(AI));
  }

  /// Entities lists. We need a predictable way of iterating through lists so
  /// the init/fini code is generated always the same for the same loops.
  using VPReductionList = SmallVector<std::unique_ptr<VPReduction>, 4>;
  using VPInductionList = SmallVector<std::unique_ptr<VPInduction>, 4>;
  using VPPrivatesList = SmallVector<std::unique_ptr<VPPrivate>, 4>;
  using VPComressExpandIdiomList = SmallVector<std::unique_ptr<VPCompressExpandIdiom>, 4>;

  /// Mapping of VPValues to entities. Created after entities lists are formed
  /// to ensure correct masking.
  using VPReductionMap = DenseMap<const VPValue *, const VPReduction *>;
  using VPInductionMap = DenseMap<const VPValue *, const VPInduction *>;
  using VPPrivateMap = DenseMap<const VPValue *, const VPPrivate *>;
  using VPComressExpandIdiomMap = DenseMap<const VPValue *, const VPCompressExpandIdiom *>;

  // Return the iterator-range to the list of privates loop-entities.
  inline decltype(auto) vpprivates() const {
    return map_range(make_range(PrivatesList.begin(), PrivatesList.end()),
                     getRawPointer<VPPrivate>);
  }
  // Return the iterator-range to the list of reduction loop-entities.
  inline decltype(auto) vpreductions() const {
    return map_range(make_range(ReductionList.begin(), ReductionList.end()),
                     getRawPointer<VPReduction>);
  }

  // Return the iterator-range to the list of induction loop-entities.
  inline decltype(auto) vpinductions() const {
    return map_range(make_range(InductionList.begin(), InductionList.end()),
                     getRawPointer<VPInduction>);
  }

  // Return the iterator-range to the list of compress/expand idiom
  // loop-entities.
  inline decltype(auto) vpceidioms() const {
    return map_range(make_range(ComressExpandIdiomList.begin(),
                                ComressExpandIdiomList.end()),
                     getRawPointer<VPCompressExpandIdiom>);
  }

  VPIndexReduction *getMinMaxIndex(const VPReduction *Red) const {
    MinMaxIndexTy::const_iterator It = MinMaxIndexes.find(Red);
    if (It != MinMaxIndexes.end()) {
      assert(It->second->isLinearIndex() && "expected linear index reduction");
      return It->second;
    }
    return nullptr;
  }

  VPLoopEntityList(VPlanVector &P, VPLoop &L) : Plan(P), Loop(L) {}

  VPValue *getReductionIdentity(const VPReduction *Red) const;
  bool isMinMaxLastItem(const VPReduction &Red) const;

  void insertVPInstructions(VPBuilder &Builder);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS, const VPBasicBlock *LoopHeader = nullptr) const;
  void dump() const { dump(errs()); }
#endif
  const VPLoop &getLoop() const { return Loop; }

  VPPHINode *findInductionStartPhi(const VPInduction *Induction) const;

  // Record that PHI node \p Duplicate is exactly identical to the original
  // induction PHI node \p Orig.
  void addDuplicateInductionPHIs(VPPHINode *Duplicate, VPPHINode *Orig) {
    DuplicateInductionPHIs.push_back(std::make_pair(Duplicate, Orig));
  }

  // Replace all uses of duplicate induction PHI nodes with their corresponding
  // original induction PHIs within the current loop. This method also clears
  // DuplicateInductionPHIs after replacement. NOTE: The duplicate PHI is not
  // removed from HCFG.
  void replaceDuplicateInductionPHIs();

  /// Return VPPHINode that corresponds to a recurrent entity.
  VPPHINode *getRecurrentVPHINode(const VPLoopEntity &E) const;

  // Return VPInstTy corresponding to a VPLoopEntity.
  template<class VPInstTy>
  VPInstTy *getLinkedInstruction(const VPLoopEntity *E) const {
    for (auto *VPInst : E->getLinkedVPValues())
      if (auto *Inst = dyn_cast<VPInstTy>(VPInst))
        return Inst;
    return nullptr;
  }

  /// Return true if the \p VPhi is recurrence Phi of a reduction.
  /// Recurrence phi is phi that resides in loop header and merges
  /// initial value and value coming from loop latch.
  bool isReductionPhi(const VPPHINode* VPhi) const;

  bool hasInMemoryEntity() const {
    for (auto &It : MemoryDescriptors)
      // Return true if we have an entity with a memory descriptor
      // that cannot be registerized.
      if (!It.second.get()->canRegisterize())
        return true;

    return false;
  }

  /// Same as hasInMemoryEntity but restrict to inductions.
  bool hasInMemoryInduction() const {
    for (auto &It : MemoryDescriptors) {
      auto *VPEntity = It.first;
      auto *MemoryDescr = It.second.get();
      // Return true if we have a induction with a memory descriptor
      // that cannot be registerized.
      if (isa<VPInduction>(VPEntity) && !MemoryDescr->canRegisterize())
        return true;
    }

    return false;
  }

  /// Same as hasInMemoryEntity but restrict to liveout privates.
  bool hasInMemoryLiveoutPrivate() const {
    for (auto &It : MemoryDescriptors) {
      auto *PrivEntity = dyn_cast<VPPrivate>(It.first);
      auto *MemoryDescr = It.second.get();
      bool IsLastPriv = PrivEntity && PrivEntity->isLast();
      // Return true if we have a liveout private with a memory descriptor
      // that cannot be registerized.
      if (IsLastPriv && !MemoryDescr->canRegisterize())
        return true;
    }

    return false;
  }

  void setImportingError(ImportError ErrC) { ImportErr = ErrC; }
  ImportError getImportingError() const { return ImportErr; }

  // Find implicit last privates in the loop and add their descriptors.
  // Implicit last private is a live out value which is assigned in the
  // loop and is not known as reduction/induction.
  void analyzeImplicitLastPrivates();

  void linkValue(VPLoopEntity *E, VPValue *Val) {
    if (auto Red = dyn_cast<VPReduction>(E))
      linkValue(ReductionMap, Red, Val);
    else if (auto Red = dyn_cast<VPIndexReduction>(E))
      linkValue(ReductionMap, Red, Val);
    else if (auto Red = dyn_cast<VPInscanReduction>(E))
      linkValue(ReductionMap, Red, Val);
    else if (auto Red = dyn_cast<VPUserDefinedReduction>(E))
      linkValue(ReductionMap, Red, Val);
    else if (auto Ind = dyn_cast<VPInduction>(E))
      linkValue(InductionMap, Ind, Val);
    else if (auto Priv = dyn_cast<VPPrivate>(E))
      linkValue(PrivateMap, Priv, Val);
    else if (auto CEIdiom = dyn_cast<VPCompressExpandIdiom>(E))
      linkValue(ComressExpandIdiomMap, CEIdiom, Val);
    else
      llvm_unreachable("Unknown loop entity");
  }

  bool hasReduction() const { return !ReductionList.empty(); }

private:
  VPlanVector &Plan;
  VPLoop &Loop;

  ImportError ImportErr = ImportError::None;

  VPReductionList ReductionList;
  VPInductionList InductionList;
  VPPrivatesList PrivatesList;
  VPComressExpandIdiomList ComressExpandIdiomList;

  VPReductionMap ReductionMap;
  VPInductionMap InductionMap;
  VPPrivateMap PrivateMap;
  VPComressExpandIdiomMap ComressExpandIdiomMap;

  // Mapping of VPLoopEntity to VPLoopEntityMemoryDescriptor.
  using MemDescrTy =
      DenseMap<VPLoopEntity *, std::unique_ptr<VPLoopEntityMemoryDescriptor>>;
  MemDescrTy MemoryDescriptors;

  // Mapping alloca instructions to memory descriptors.
  using MemInstDescrMapTy = DenseMap<VPValue *, VPLoopEntityMemoryDescriptor *>;
  MemInstDescrMapTy MemInstructions;

  // MinMax reduction to index reduction
  using MinMaxIndexTy = DenseMap<const VPReduction *, VPIndexReduction *>;
  MinMaxIndexTy MinMaxIndexes;

  // Collection of duplicate induction PHI nodes. First element of the pair
  // represents the duplicate PHI node and the second element represents the
  // original PHI.
  SmallVector<std::pair<VPPHINode *, VPPHINode *>, 4> DuplicateInductionPHIs;

  // Find an item in the map defined as T<K,item>
  template <typename T, class K>
  typename T::mapped_type find(T &Map, K Key) const {
    typename T::const_iterator It = Map.find(Key);
    if (It != Map.end())
      return It->second;
    return nullptr;
  }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  template <class T>
  void dumpList(const char *Header, const T &List, raw_ostream &OS) const {
    OS << Header;
    for (auto &Item : List) {
      Item.get()->dump(OS);
      const VPLoopEntityMemoryDescriptor *Mem =
          getMemoryDescriptor(Item.get());
      if (Mem) {
        OS << " Memory: ";
        Mem->dump(OS);
      }
      OS << "\n";
    }
  }
#endif

  void createMemDescFor(VPLoopEntity *E, VPValue *AI);

  template <class T, class M> void linkValue(M &Map, T *Descr, VPValue *Val) {
    if (Val && !isa<VPConstant>(Val)) {
      auto Iter = Map.find(Val); (void)Iter;
      assert((Iter == Map.end() || Iter->second == Descr) &&
             "Inconsistency in VPValue->Descriptor mapping");
      Map[Val] = Descr;
      Descr->addLinkedVPValue(Val);
    }
  }

  // Create private memory allocator for VPLoopEntity if the corresponding
  // memory descriptor exists.
  // If the memory descriptor exists, stores the descriptor's alloca instruction
  // into \p AI, and returns a newly created VPAllocatePrivate. In case of
  // in-register entity (i.e. no descriptor exists), the nullptr is stored in \p
  // AI and nullptr is returned.
  // If the VPAlloca is created it is placed at the beginning of the
  // \p Preheader.
  VPValue *createPrivateMemory(VPLoopEntity &E, VPBuilder &Builder,
                               VPValue *&AI, VPBasicBlock* Preheader);

  // Process initial value \p Init of entity \p E.
  // If the private memory \p PrivateMem is not null then an instruction
  // to store the \p Init into \p PrivateMem is created. Then, all occurences of
  // the scalar memory \p AI are replaced, in the loop, by \p PrivateMemory.
  // Also all relevant occurences of the start value \p Start are replaced by
  // the new \p Init.
  void processInitValue(VPLoopEntity &E, VPValue *AI, VPValue *PrivateMem,
                        VPBuilder &Builder, VPValue &Init, Type *Ty,
                        VPValue &Start, bool IsInscanInit = false);

  // Process final value \p Final of entity \p E. The store to original memory
  // \p AI is created and original exit value \p Exit ocurrences are replaced by
  // the new final value \p Final, in all external uses.
  void processFinalValue(VPLoopEntity &E, VPValue *AI, VPBuilder &Builder,
                         VPValue &Final, Type *Ty, VPValue *Exit);

  // Create "close-form calculation" for induction. The close-form
  // calculation is calculation by the formula v = i MUL step OP v0. In case of
  // the loop inductions, the need of the close-form means that we need
  // up-to-date induction value at the beginning of each loop iteration. This
  // cases are, for example, when induction is used after its increment or is
  // updated inside a function. The example of the first case is below.
  // DO
  //   %ind = phi(init, %inc_ind)
  //   ... uses of %ind
  //   %inc_ind = %ind OP step   // we can't replace step with VF here due to
  //   ... uses of inc_ind       // the followed uses.
  // ENDDO
  // The close-form calculation can be done in two ways: insert calculation
  // exactly by the formula at the beginning of the loop, or insert increment of
  // the original induction in the end of the loop, ignoring any updates inside
  // loop body. The second way increases register pressure but seems more
  // effective in terms of run-time.
  void createInductionCloseForm(VPInduction *Induction, VPBuilder &Builder,
                                VPValue &Init, VPValue &InitStep,
                                VPValue *PrivateMem);

  VPInstruction *getInductionLoopExitInstr(const VPInduction *Induction) const;

  /// Return induction descriptor that corresponds to the main loop IV.
  const VPInduction *getLoopInduction() const;

  /// Return true if live out value of the induction \p Ind is calculated on the
  /// penultimate iteration of the loop.
  bool isInductionLastValPreInc(const VPInduction *Ind) const;

  // Insert VPInstructions related to VPReductions.
  void insertReductionVPInstructions(VPBuilder &Builder,
                                     VPBasicBlock *Preheader,
                                     VPBasicBlock *PostExit);

  // Insert VPInstructions related to VPInductions.
  void insertInductionVPInstructions(VPBuilder &Builder,
                                     VPBasicBlock *Preheader,
                                     VPBasicBlock *PostExit);

  // Insert VPInstructions related to VPPrivates.
  void insertPrivateVPInstructions(VPBuilder &Builder, VPBasicBlock *Preheader,
                                   VPBasicBlock *PostExit);

  // Insert VPInstructions related to VPCompressExpandIdioms.
  void insertCompressExpandVPInstructions(VPBuilder &Builder,
                                          VPBasicBlock *Preheader,
                                          VPBasicBlock *PostExit);

  // Each update/store in the chain from the outer vectorized loop header to
  // liveout instruction is accompanied by the assignment/store of the loop
  // induction to an additional variable. Each phi in this chain is accompanied
  // by the phi for that additional variable. The variable keeps the index of
  // the iteration at which the value was assigned. In SSA form, the explicit
  // assignments are unneeded and only phis with loop-induction operands from
  // corresponding blocks are inserted.
  // That is done for the last value calculation: after the loop, we find the
  // maximum index and extract the vector element from the same position where
  // that maximum index resides.
  void insertConditionalLastPrivateInst(VPPrivate &Private, VPBuilder &Builder,
                                        VPBasicBlock *Preheader,
                                        VPBasicBlock *PostExit,
                                        VPValue *PrivateMem, VPValue *AI);

  // Mapping function that returns the underlying raw pointer.
  template <typename EntityType>
  static inline EntityType *
  getRawPointer(const std::unique_ptr<EntityType> &En) {
    return En.get();
  }

  // Preprocess entities before instructions insertion.
  // - Identify/fix indexes of index reductions
  void preprocess();

  // Insert VPInstructions (init/final) for the reduction \p Reduction,
  // keeping its final and exit instructions in a special map \p RedFinalMap,
  // and inserting the reduction into list of processed reductions
  // \p ProcessedReductions.
  void insertOneReductionVPInstructions(
      VPReduction *Reduction, VPBuilder &Builder, VPBasicBlock *PostExit,
      VPBasicBlock *Preheader,
      // The map contains, for each reduction, a ReductionExit
      // and is used to obtain parent reduction values needed for
      // the index part of min/max+index last value code generation.
      DenseMap<const VPReduction *, VPInstruction *> &RedExitMap,
      SmallPtrSetImpl<const VPReduction *> &ProcessedReductions);

  // Insert VPInstructions to initialize/finalize user-defined reduction \p UDR
  // and inserting the reduction into list of processed reductions \p
  // ProcessedReductions.
  void insertUDRVPInstructions(
      VPUserDefinedReduction *UDR, VPBuilder &Builder, VPBasicBlock *PostExit,
      VPBasicBlock *Preheader,
      SmallPtrSetImpl<const VPReduction *> &ProcessedReductions);

  // Insert inscan-related reduction instructions and process
  // inclusive/exclusive pragmas in the loop body.
  void insertRunningInscanReductionInstrs(
    const SmallVectorImpl<const VPInscanReduction *> &InscanReductions,
    VPBuilder &Builder);

  // Look through min/max+index reductions and identify which ones
  // are linears. See comment for VPIndexReduction::isLinearIndex().
  void identifyMinMaxLinearIdxs();

  // Create linear index for min/max + index idiom.
  // 1) Emit the following set of VPInstructions:
  // Suppose we have
  // %loop_header:
  //    %NonLinPhi = phi[%NonLinStart,%preheader], [%NonLinUpdate,%loop_latch]
  //    ...
  //  %loop_body:
  //    %NonLinUpdate = select %NonLin.cond, %NonLinVal, %NonLinPhi
  //
  // The result of the transformation will be:
  // %loop_header:
  //   %phi_val = phi [-1, %preheader], [%select, %loop_latch] // new phi
  //   %NonLinPhi = phi[%NonLinStart,%preheader], [%NonLinUpdate,%loop_latch]
  //
  //  %loop_body:
  //    %NonLinUpdate = select %NonLin.cond, %NonLinVal, %NonLinPhi
  //    %select = select %NonLin.cond, %loop_induction, %phi_val // new select
  //
  // 2) Create VPIndexReduction descriptor for these instructions, setting
  //    parent reduction to the parent of NonLinNdx.
  // 3) Set "IsLinearIndex" flag on the new reduction and set parent's index
  //    reduction to the newly created descriptor.
  VPIndexReduction *createLinearIndexReduction(VPIndexReduction *NonLinNdx,
                                               VPDominatorTree &DomTree);
};

class VPEntityImportDescr {
  struct DescrAlias {
    DescrAlias() = default;
    VPValue *Start = nullptr;
    SmallVector<VPInstruction *, 4> UpdateVPInsts;
  };

protected:
  VPEntityImportDescr() = default;

public:
  virtual ~VPEntityImportDescr() {}

  virtual bool isDuplicate(const VPlanVector *Plan, const VPLoop *Loop) const;

  VPValue *getAllocaInst() const { return AllocaInst; }
  bool getValidMemOnly() const { return ValidMemOnly; }
  bool getImporting() const { return Importing; }

  void setAllocaInst(VPValue *V) { AllocaInst = V; }
  void setValidMemOnly(bool V) { ValidMemOnly = V; }
  void setImporting(bool V) { Importing = V; }

  void setAlias(VPValue *Start, ArrayRef<VPInstruction *> UpdateVPInsts) {
    DescrAlias NewAlias;
    NewAlias.Start = Start;
    NewAlias.UpdateVPInsts.assign(UpdateVPInsts.begin(), UpdateVPInsts.end());
    Alias = Optional<DescrAlias>(NewAlias);
    HasAlias = true;
  }

  // Return true if \p Val is used in the \p Loop or in its
  // preheader, not in lifetimestart/end.
  static bool hasRealUserInLoop(VPValue *Val, const VPLoop *Loop);

  iterator_range<SmallVectorImpl<VPInstruction *>::iterator>
  getUpdateVPInsts() {
    return make_range(UpdateVPInsts.begin(), UpdateVPInsts.end());
  }
  void addUpdateVPInst(VPInstruction *V) { UpdateVPInsts.push_back(V); }

protected:
  VPValue *findMemoryUses(VPValue *Start, const VPLoop *Loop);

  virtual void clear() {
    AllocaInst = nullptr;
    ValidMemOnly = false;
    Importing = true;
    Alias.reset();
    HasAlias = false;
    UpdateVPInsts.clear();
  }
  VPValue *AllocaInst = nullptr;
  bool ValidMemOnly = false;
  bool Importing = true;
  // NOTE: We assume that a descriptor can have only one valid alias
  Optional<DescrAlias> Alias;
  bool HasAlias = false;
  /// Instruction(s) in VPlan that update the variable
  SmallVector<VPInstruction *, 4> UpdateVPInsts;
};

/// Intermediate reduction descriptor. This is a temporary data to keep
/// descriptors between two stages, gathering and importing. We can't use final
/// descriptors for that purpose as they require some consistency but in some
/// cases it can be obtained only after some analysis, at the second stage of
/// importing.
class ReductionDescr : public VPEntityImportDescr {
  using BaseT = VPEntityImportDescr;
public:
  ReductionDescr() = default;

  VPInstruction *getStartPhi() const { return StartPhi; }
  VPValue *getStart() const { return Start; }
  VPInstruction *getExit() const { return Exit; }
  RecurKind getKind() const { return K; }
  Type *getRecType() const { return RT; }
  bool getSigned() const { return Signed; }
  VPInstruction *getLinkPhi() const { return LinkPhi; }
  bool getLinearIndex() const { return IsLinearIndex; }
  Optional<InscanReductionKind> getInscanReductionKind() const {
    return InscanRedKind;
  }
  bool isUDR() const { return K == RecurKind::Udr; }
  Function *getCombiner() const { return Combiner; }
  Function *getInitializer() const { return Initializer; }
  Function *getCtor() const { return Ctor; }
  Function *getDtor() const { return Dtor; }

  void setStartPhi(VPInstruction *V) { StartPhi = V; }
  void setStart(VPValue *V) { Start = V; }
  void setExit(VPInstruction *V) { Exit = V; }
  void setKind(RecurKind V) { K = V; }
  void setRecType(Type *V) { RT = V; }
  void setSigned(bool V) { Signed = V; }
  void setLinkPhi(VPInstruction *V) { LinkPhi = V; }
  void setIsLinearIndex(bool V) { IsLinearIndex = V; }
  void setInscanReductionKind(Optional<InscanReductionKind> V) {
    InscanRedKind = V;
  }
  void addLinkedVPValue(VPValue *V) { LinkedVPVals.push_back(V); }
  void setCombiner(Function *CombinerFn) { Combiner = CombinerFn; }
  void setInitializer(Function *InitFn) { Initializer = InitFn; }
  void setCtor(Function *CtorFn) { Ctor = CtorFn; }
  void setDtor(Function *DtorFn) { Dtor = DtorFn; }

  /// Clear the content.
  void clear() override {
    BaseT::clear();
    StartPhi = nullptr;
    Start = nullptr;
    Exit = nullptr;
    K = RecurKind::None;
    RT = nullptr;
    Signed = false;
    LinkPhi = nullptr;
    LinkedVPVals.clear();
    IsLinearIndex = false;
    InscanRedKind = None;
    Combiner = nullptr;
    Initializer = nullptr;
    Ctor = nullptr;
    Dtor = nullptr;
  }
  /// Check for that all non-null VPInstructions in the descriptor are in the \p
  /// Loop.
  void checkParentVPLoop(const VPLoop *Loop) const;

  /// Return true if not all data is completed.
  bool isIncomplete() const;
  /// Attempt to fix incomplete data using VPlan and VPLoop.
  void tryToCompleteByVPlan(VPlanVector *Plan, const VPLoop *Loop);
  /// Pass the data to VPlan
  void passToVPlan(VPlanVector *Plan, const VPLoop *Loop);
  /// Check if current reduction descriptor duplicates another that is already
  /// imported.
  bool isDuplicate(const VPlanVector *Plan, const VPLoop *Loop) const override;

private:
  // Some analysis methods used by tryToCompleteByVPlan
  /// Utility that replaces current descriptor's properties with that from
  /// alias, if needed. Returns false if analysis bailed which means descriptor
  /// is not being imported, else returns true.
  bool replaceOrigWithAlias();
  /// Utility to get the loop exit VPInstruction for given reduction descriptor
  /// by analyzing its UpdateVPInsts in light of loop LiveOut analysis. It
  /// returns nullptr for InMemory reduction.
  VPInstruction *getLoopExitVPInstr(const VPLoop *Loop);
  /// Utility to invalidate underlying IR for all VPInstructions involved in
  /// current reduction. Currently we invalidate instructions in StartPhi, Exit
  /// and LinkedVPVals.
  void invalidateReductionInstructions();
  /// Utility to identify last non-header PHI node in the users-chain of VPInst.
  /// Checks are performed recursively using worklist approach to account for
  /// nested control flow.
  VPPHINode *getLastNonheaderPHIUser(VPInstruction *VPInst, const VPLoop *Loop);

  VPInstruction *StartPhi = nullptr; // TODO: Consider changing to VPPHINode.
  VPValue *Start = nullptr;
  VPInstruction *Exit = nullptr;
  RecurKind K = RecurKind::None;
  Type *RT = nullptr;
  bool Signed = false;
  VPInstruction *LinkPhi = nullptr; // TODO: Consider changing to VPPHINode.
  bool IsLinearIndex = false;
  // To avoid having 'None' inscan reduction kind in an enum, use Optional.
  // If set, this means this is an inscan reduction.
  Optional<InscanReductionKind> InscanRedKind;

  /// VPValues that are associated with reduction variable
  /// NOTE: This list is accessed and populated internally within the descriptor
  /// object, hence no getters or setters.
  // Example for reduction with LinkedVPValues:
  //
  //  %init = load %red.var
  //  loop.body:
  //   %red.phi = phi [ %init, PHBB ], [ %update, loop.body ]
  //   ...
  //   %update = add %red.phi, %abc
  //   store %update, %red.var
  //   ...
  //
  // Here for the reduction descriptor %red.var we initially collect only the
  // store as its updating instruction. However through alias analysis we find
  // out the actual PHI and add upate instruction. Before overwriting update
  // instructions for any analyses, the store is saved in linked VPValues for
  // later stage analyses or corrections (example would be private memory).
  SmallVector<VPValue *, 4> LinkedVPVals;
  // Functions needed for initialization/finalization of UDRs.
  Function *Combiner = nullptr;
  Function *Initializer = nullptr;
  Function *Ctor = nullptr;
  Function *Dtor = nullptr;
};

/// Intermediate induction descriptor. Same as ReductionDescr above but for
/// inductions.
class InductionDescr : public VPEntityImportDescr {
  using InductionKind = VPInduction::InductionKind;
  using BaseT = VPEntityImportDescr;

public:
  InductionDescr() = default;

  VPInstruction *getStartPhi() const { return StartPhi; }
  InductionKind getKind() const { return K; }
  VPValue *getStart() const { return Start; }
  VPValue *getStep() const { return Step; }
  int getStepMultiplier() const { return StepMultiplier; }
  Type *getStepType() const { return StepTy; }
  const SCEV *getStepSCEV() const { return StepSCEV; }
  VPValue *getStartVal() const { return StartVal; }
  VPValue *getEndVal() const { return EndVal; }
  VPInstruction *getInductionOp() const { return InductionOp; }
  unsigned getIndOpcode() const { return IndOpcode; }

  void setStartPhi(VPInstruction *V) { StartPhi = V; }
  void setKind(InductionKind V) { K = V; }
  void setStart(VPValue *V) { Start = V; }
  void setStep(VPValue *V) { Step = V; }
  void setStepMultiplier(int multiplier) { StepMultiplier = multiplier; }
  void setStepType(Type *Ty) { StepTy = Ty; }
  void setStepSCEV(const SCEV *Scev) { StepSCEV = Scev; }
  void setStartVal(VPValue *V) { StartVal = V; }
  void setEndVal(VPValue *V) { EndVal = V; }
  void setInductionOp(VPInstruction *V) { InductionOp = V; }
  void setIndOpcode(unsigned V) { IndOpcode = V; }
  void setIsExplicitInduction(bool V) { IsExplicitInduction = V; }
  void addUpdateVPInst(VPInstruction *V) { UpdateVPInsts.push_back(V); }

  // Get InductionKind and default induction opcode for the data type \p IndTy.
  static std::pair<unsigned, InductionKind>
  getKindAndOpcodeFromTy(Type *IndTy) {
    if (IndTy->isIntegerTy()) {
      return std::make_pair(Instruction::Add,
                            InductionDescriptor::IK_IntInduction);
    } else if (IndTy->isPointerTy()) {
      return std::make_pair(Instruction::GetElementPtr,
                            InductionDescriptor::IK_PtrInduction);
    } else {
      assert(IndTy->isFloatingPointTy() && "unexpected induction type");
      return std::make_pair(Instruction::FAdd,
                            InductionDescriptor::IK_FpInduction);
    }
  }

  // Set InductionKind and default induction opcode using data type \p IndTy.
  void setKindAndOpcodeFromTy(Type *IndTy) {
    unsigned Opc = 0;
    VPInduction::InductionKind Kind;
    std::tie(Opc, Kind) = getKindAndOpcodeFromTy(IndTy);
    setKind(Kind);
    setIndOpcode(Opc);
  }

  /// Clear the content.
  void clear() override {
    BaseT::clear();
    StartPhi = nullptr;
    K = InductionKind::IK_NoInduction;
    Start = nullptr;
    Step = nullptr;
    StepMultiplier = 1;
    StepSCEV = nullptr;
    StepTy = nullptr;
    StartVal = nullptr;
    EndVal = nullptr;
    InductionOp = nullptr;
    IndOpcode = Instruction::BinaryOpsEnd;
    IsExplicitInduction = false;
  }
  /// Check for all non-null VPInstructions in the descriptor are in the \p
  /// Loop.
  void checkParentVPLoop(const VPLoop *Loop) const;

  /// Return true if not all data is completed.
  bool isIncomplete() const;
  /// Attemp to fix incomplete data using VPlan and VPLoop.
  void tryToCompleteByVPlan(VPlanVector *Plan, const VPLoop *Loop);
  /// Pass the data to VPlan
  void passToVPlan(VPlanVector *Plan, const VPLoop *Loop);
  /// Check if current induction descriptor duplicates another that is already
  /// imported.
  bool isDuplicate(const VPlanVector *Plan, const VPLoop *Loop) const override;
  /// Check if induction needs close form representation in vector code.
  // NOTE: This analysis is done for both auto-recognized and explicit
  // inductions.
  bool inductionNeedsCloseForm(const VPLoop *Loop) const;

private:
  /// Check if the VPlan instruction \p IncrementVPI which increments the
  /// induction variable has any non-whitelist users within \p Loop. Whitelist
  /// can be found in the function's implementation.
  bool hasUserOfIndIncrement(VPInstruction *IncrementVPI,
                             SmallPtrSetImpl<VPInstruction *> &AnalyzedVPIs,
                             const VPLoop *Loop) const;

  VPInstruction *StartPhi = nullptr;
  InductionKind K = InductionKind::IK_NoInduction;
  VPValue *Start = nullptr;
  VPValue *Step = nullptr;
  int StepMultiplier = 1;
  Type *StepTy = nullptr;
  const SCEV *StepSCEV = nullptr;
  VPValue *StartVal = nullptr;
  VPValue *EndVal = nullptr;
  VPInstruction *InductionOp =nullptr;
  unsigned IndOpcode = Instruction::BinaryOpsEnd;
  bool IsExplicitInduction = false;
};

/// Intermediate private descriptor. Same as ReductionDescr above but for
/// privates.
class PrivateDescr : public VPEntityImportDescr {
  using BaseT = VPEntityImportDescr;
  using VPEntityAliasesTy = VPPrivate::VPEntityAliasesTy;

public:
  PrivateDescr() = default;

  Type *getAllocatedType() const { return AllocatedType; }
  bool isConditional() const { return IsConditional; }
  bool isLast() const { return IsLast; }
  bool isExplicit() const { return IsExplicit; }
  bool isMemOnly() const { return getValidMemOnly(); }
  Function *getCtor() const { return Ctor; }
  Function *getDtor() const { return Dtor; }
  Function *getCopyAssign() const { return CopyAssign; }

  /// Clear the content.
  void clear() override {
    BaseT::clear();
    PtrAliases.clear();
    ExitInst = nullptr;
    IsConditional = false;
    IsLast = false;
    IsExplicit = false;
    IsF90 = false;
    PTag = VPPrivate::PrivateTag::PTRegisterized;
  }
  /// Check for all non-null VPInstructions in the descriptor are in the \p
  /// Loop.
  void checkParentVPLoop(const VPLoop *Loop) const;

  /// Return true if not all data is completed.
  bool isIncomplete() const { return ExitInst == nullptr; }

  /// Attemp to fix incomplete data using VPlan and VPLoop.
  void tryToCompleteByVPlan(VPlanVector *Plan, const VPLoop *Loop);

  /// Pass the data to VPlan
  void passToVPlan(VPlanVector *Plan, const VPLoop *Loop);

  void setAllocatedType(Type *Ty) { AllocatedType = Ty; }
  void setIsConditional(bool IsCond) { IsConditional = IsCond; }
  void setIsLast(bool IsLastPriv) { IsLast = IsLastPriv; }
  void setIsExplicit(bool IsExplicitVal) { IsExplicit = IsExplicitVal; }
  void setIsMemOnly(bool IsMem) { setValidMemOnly(IsMem); }
  void setExitInst(VPInstruction *EI) { ExitInst = EI; }
  void addAlias(VPValue *Alias, VPInstruction *VPI, const Instruction *I) {
    PtrAliases[Alias] = std::make_pair(VPI, I);
  }
  void setCtor(Function *CtorFn) { Ctor = CtorFn; }
  void setDtor(Function *DtorFn) { Dtor = DtorFn; }
  void setCopyAssign(Function *CopyAssignFn) { CopyAssign = CopyAssignFn; }
  void setIsF90(bool F90) { IsF90 = F90; }

private:
  /// Set fields to define PrivateKind for the imported private.
  /// Sometimes it can differ from one that was set by user. E.g. the
  /// "conditional" modifier can be set on the private that is assigned in both
  /// branches of an IF statement but those assignments can be merged into one
  /// select instruction and the assignment becomes unconditional. This is a
  /// simple correction of the user declaration to generate more effective code.
  /// Returns true if the update was successful and false if we were unable
  /// to recognize it.
  bool updateKind(VPLoopEntityList *LE);

  // Type of memory.
  Type *AllocatedType = nullptr;
  VPInstruction *ExitInst = nullptr;
  // These are Pointer-aliases. Each Loop-private memory descriptor can have
  // multiple aliases as opposed to memory descriptors for inductions or
  // reductions. Hence, we have a separate field. TODO: consider using a
  // single/same field for every memory descriptor.
  VPEntityAliasesTy PtrAliases;
  bool IsConditional = false;
  bool IsLast = false;
  bool IsExplicit = false;
  bool IsF90 = false;
  Function *Ctor = nullptr;
  Function *Dtor = nullptr;
  Function *CopyAssign = nullptr;
  VPPrivate::PrivateTag PTag = VPPrivate::PrivateTag::PTRegisterized;
};

class CompressExpandIdiomDescr : public VPEntityImportDescr {

  SmallVector<VPInstruction *> Increments;
  SmallVector<VPLoadStoreInst *, 4> Stores;
  SmallVector<VPLoadStoreInst *, 4> Loads;
  SmallVector<VPInstruction *, 4> Indices;

  int64_t TotalStride = 0;
  VPPHINode *RecurrentPhi = nullptr;
  VPValue *LiveIn = nullptr;
  VPInstruction *LiveOut = nullptr;

  bool IsIncomplete = true;

public:
  void addIncrement(VPInstruction *VPInst, int64_t Stride) {
    Increments.push_back(VPInst);
    TotalStride += Stride;
  }
  void addStore(VPLoadStoreInst *VPInst) { Stores.push_back(VPInst); }
  void addLoad(VPLoadStoreInst *VPInst) { Loads.push_back(VPInst); }
  void addIndex(VPInstruction *VPVal) { Indices.push_back(VPVal); }

  /// Check for all non-null VPInstructions in the descriptor are in the \p
  /// Loop.
  void checkParentVPLoop(const VPLoop *Loop) const {}

  /// Return true if not all data is completed.
  bool isIncomplete() const { return IsIncomplete; }

  /// Attemp to fix incomplete data using VPlan and VPLoop.
  void tryToCompleteByVPlan(VPlanVector *Plan, const VPLoop *Loop);

  /// Pass the data to VPlan
  void passToVPlan(VPlanVector *Plan, const VPLoop *Loop);
};

// Base class for loop entities converter. Used to create a list of converters
// during first conversion phase. Those converters are then used at the second
// conversion phase. This class is needed due to implementation of converters
// is templatized. But we can't have templatized virtual functions, we can have
// a virtual function in templatized class.
class VPLoopEntitiesConverterBase {
public:
  enum C_Kind { C_Unknown = 0, C_Templ, C_Final };
  explicit VPLoopEntitiesConverterBase(C_Kind K) : Kind(K) {}
  VPLoopEntitiesConverterBase(const VPLoopEntitiesConverterBase &) = delete;

  virtual ~VPLoopEntitiesConverterBase() {}
  C_Kind getKind() const { return Kind; }

private:
  C_Kind Kind;
};

// Base abstract converter template class with virtual function.
template <class MapperT>
class VPLoopEntitiesConverterTempl : public VPLoopEntitiesConverterBase {
public:
  explicit VPLoopEntitiesConverterTempl(C_Kind K)
      : VPLoopEntitiesConverterBase(K) {}
  VPLoopEntitiesConverterTempl(const VPLoopEntitiesConverterTempl &) = delete;

  virtual void passToVPlan(VPlanVector *Plan, MapperT &M) = 0;

  static bool classof(const VPLoopEntitiesConverterBase *B) {
    return B->getKind() != C_Unknown;
  }
};

/// Loop entities converter. Consists of two passes. On the first pass, using
/// externally created iterator, goes through the input sequence and creates a
/// list of converted entities. This pass is called for each loop encountered in
/// underlying IR on which VPlan is built. During the second pass, all gathered
/// entities are passed into VPlan grouping by VPLoop-s.
///  Template parameters:
///    IteratorT - input iterator.
///    LoopT - underlying IR loop class.
///    MapperT - object to map underlying IR loop to VPLoop.
template <class DescrT, class LoopT, class MapperT>
class VPLoopEntitiesConverter : public VPLoopEntitiesConverterTempl<MapperT> {
  typedef SmallVector<DescrT, 2> DescrList;
  typedef std::pair<LoopT *, DescrList> LoopListItemT;
  typedef SmallVector<LoopListItemT, 2> LoopListT;

public:
  explicit VPLoopEntitiesConverter(VPlanVector *P)
      : VPLoopEntitiesConverterTempl<MapperT>(
            VPLoopEntitiesConverterBase::C_Final),
        Plan(P) {}
  VPLoopEntitiesConverter(const VPLoopEntitiesConverter &) = delete;

  static bool classof(const VPLoopEntitiesConverterBase *B) {
    return B->getKind() == VPLoopEntitiesConverterBase::C_Final;
  }

  /// Gathering pass.
  template<typename... OtherItersT>
  void createDescrList(LoopT *Loop, OtherItersT&&... Args) {
    LoopList.emplace_back(Loop, DescrList());
    DescrList &Lst = LoopList.back().second;
    // Expand the parameter pack.
    (void)std::initializer_list<int>{(processIterator(Lst, std::move(Args)), 0)...};
  }

  /// Sending pass.
  void passToVPlan(VPlanVector *Plan, MapperT &M) override {
    for (auto &LLItem : LoopList) {
      const VPLoop *Loop = M[LLItem.first];
      assert(Loop != nullptr && "Can't find corresponding VPLoop");
      for (auto &Descr : LLItem.second) {
        Descr.checkParentVPLoop(Loop);
        if (Descr.isIncomplete())
          Descr.tryToCompleteByVPlan(Plan, Loop);
        if (Descr.isDuplicate(Plan, Loop))
          continue; // Skip duplication
        Descr.passToVPlan(Plan, Loop);
      }
    }
  }

private:
  template <typename RangeTy, typename ConverterTy>
  static void processIterator(DescrList &Lst,
                              std::pair<RangeTy, ConverterTy> &&Iterator) {
    auto &ValRange = Iterator.first;
    auto &ConverterFunc = Iterator.second;
    for (const auto &Iter : ValRange) {
      Lst.push_back(DescrT());
      auto &Item = Lst.back();
      ConverterFunc(Item, Iter);
    }
  }

  VPlanVector *Plan;
  LoopListT LoopList;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLOOPANALYSIS_H

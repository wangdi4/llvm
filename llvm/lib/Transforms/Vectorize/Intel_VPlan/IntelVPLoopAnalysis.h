//===- IntelVPLoopAnalysis.h ----------------------------------------------===//
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
// \file
/// This file provides VPLoop-based analysis. Right now VPLoopAnalysisBase can
/// only be used to compute min, known, estimated or max trip counts for a
/// VPLoopRegion.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLOOPANALYSIS_H
#define LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLOOPANALYSIS_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/IVDescriptors.h"
#include <map>

using namespace llvm;

namespace llvm {

class ScalarEvolution;
class LoopInfo;

namespace vpo {

class VPLoopRegion;
class IntelVPlanUtils;
class VPLoop;
class VPlan;
class VPValue;
class VPInstruction;

class VPLoopAnalysisBase {
protected:
#if INTEL_CUSTOMIZATION
  using TripCountTy = uint64_t;
#else
  // SCEV's getConstantTripCount() is limited to trip counts that fit into
  // uint32_t
  using TripCountTy = unsigned;
#endif // INTEL_CUSTOMIZATION
  typedef struct TripCountInfo {
    TripCountTy MinTripCount;
    TripCountTy MaxTripCount;
    TripCountTy TripCount;
    bool IsEstimated;

    explicit TripCountInfo(void)
        : MinTripCount(0), MaxTripCount(0), TripCount(0), IsEstimated(true) {}
    explicit TripCountInfo(const TripCountTy MinTC, const TripCountTy MaxTC,
                           const TripCountTy TC, const bool IsEstimated)
        : MinTripCount(MinTC), MaxTripCount(MaxTC), TripCount(TC),
          IsEstimated(IsEstimated) {}
  } TripCount;

  std::map<const VPLoopRegion *, TripCountInfo> LoopTripCounts;

  // Trip count for loops with unknown loop count
  const TripCountTy DefaultTripCount;

  TripCountInfo computeAndReturnTripCountInfo(const VPLoopRegion *Lp) {
    if (LoopTripCounts.count(Lp))
      return LoopTripCounts.find(Lp)->second;
    computeTripCount(Lp);
    assert(LoopTripCounts.count(Lp) &&
           "Cannot compute trip count for this loop");
    return LoopTripCounts.find(Lp)->second;
  }

  virtual void computeTripCountImpl(const VPLoopRegion *Lp) = 0;

public:
  explicit VPLoopAnalysisBase(const TripCountTy DefaultTripCount)
      : DefaultTripCount(DefaultTripCount) {}

  virtual ~VPLoopAnalysisBase(){}

  void computeTripCount(const VPLoopRegion *Lp) { computeTripCountImpl(Lp); }

  /// Return true if trip count for the loop is known in compile time.
  bool isKnownTripCountFor(const VPLoopRegion *Lp) {
    return !computeAndReturnTripCountInfo(Lp).IsEstimated;
  }

  /// Return minimal trip count for the loop, which was either computed by some
  /// analysis or was provided by user.
  TripCountTy getMinTripCountFor(const VPLoopRegion *Lp) {
    return computeAndReturnTripCountInfo(Lp).MinTripCount;
  }

  /// Return minimal trip count for the loop, which was either computed by some
  /// analysis or was provided by user.
  TripCountTy getMaxTripCountFor(const VPLoopRegion *Lp) {
    return computeAndReturnTripCountInfo(Lp).MaxTripCount;
  }

  /// Return trip count for the loop. It's caller's responsibility to check
  /// whether this trip count was estimated or was known in compile time.
  TripCountTy getTripCountFor(const VPLoopRegion *Lp) {
    return computeAndReturnTripCountInfo(Lp).TripCount;
  }

  /// Set known trip count for a given VPLoop. This function also sets MinTC and
  /// MaxTC to same value.
  void setKnownTripCountFor(const VPLoopRegion *Lp,
                            const TripCountTy TripCount) {
    LoopTripCounts[Lp] = TripCountInfo(TripCount, TripCount, TripCount, false);
  }

  /// Set estimated trip count for a given VPLoop. This function doesn't touch
  /// MinTC or MaxTC.
  void setEstimatedTripCountFor(const VPLoopRegion *Lp,
                                const TripCountTy TripCount) {
    LoopTripCounts[Lp].TripCount = TripCount;
    LoopTripCounts[Lp].IsEstimated = true;
  }

  /// Set MinTC for a given VPLoop. This function doesn't touch TC or MaxTC.
  void setMinTripCountFor(const VPLoopRegion *Lp, const TripCountTy TripCount) {
    LoopTripCounts[Lp].MinTripCount = TripCount;
    LoopTripCounts[Lp].IsEstimated = true;
    if (TripCount > getMaxTripCountFor(Lp))
      setMaxTripCountFor(Lp, TripCount);
  }

  /// Set MaxTC for a given VPLoop. This function doesn't touch TC or MinTC.
  void setMaxTripCountFor(const VPLoopRegion *Lp, const TripCountTy TripCount) {
    LoopTripCounts[Lp].MaxTripCount = TripCount;
    LoopTripCounts[Lp].IsEstimated = true;
    if (TripCount < getMinTripCountFor(Lp))
      setMinTripCountFor(Lp, TripCount);
  }

  void setTripCountsFromPragma(const VPLoopRegion *Lp, uint64_t MinTripCount,
                               uint64_t MaxTripCount, uint64_t AvgTripCount);
};

class VPLoopAnalysis : public VPLoopAnalysisBase {
private:
  ScalarEvolution *SE;
  LoopInfo *LI;
  // TODO: templatizing of getSmallConstantMaxTripCount() is required to support
  // VPLoop.

  void computeTripCountImpl(const VPLoopRegion *Lp) final;

public:
  explicit VPLoopAnalysis(ScalarEvolution *SE,
                          const TripCountTy DefaultTripCount, LoopInfo *LI)
      : VPLoopAnalysisBase(DefaultTripCount), SE(SE), LI(LI) {}
};

/// Base class for loop entities
class VPLoopEntity {
public:
  enum { Reduction, IndexReduction, Induction, Private };
  unsigned char getID() const { return SubclassID; }

protected:
  // No need for public constructor.
  explicit VPLoopEntity(unsigned char Id) : SubclassID(Id){};

private:
  const unsigned char SubclassID;
};

/// Recurrence descriptor
class VPReduction
    : public VPLoopEntity,
      public RecurrenceDescriptorTempl<VPValue, VPInstruction, VPValue *> {
  using RDTempl = RecurrenceDescriptorTempl<VPValue, VPInstruction, VPValue *>;

public:
  VPReduction(VPValue *Start, VPInstruction *Exit, RecurrenceKind K, FastMathFlags FMF,
              MinMaxRecurrenceKind MK, Type *RT, bool Signed,
              unsigned char Id = Reduction)
      : VPLoopEntity(Id), RDTempl(Start, Exit, K, FMF, MK, RT, Signed) {}

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPLoopEntity *V) {
    return V->getID() == Reduction;
  }
};

/// Descriptor of the index part of min/max+index reduction.
/// In addition to VPReduction, contains pointer to a parent min/max reduction
/// descriptor.
class VPIndexReduction : public VPReduction {
public:
  VPIndexReduction(VPReduction *Parent, VPValue *Start, VPInstruction *Exit,
                   Type *RT, bool Signed, bool ForLast)
      : VPReduction(Start, Exit, RK_IntegerMinMax, FastMathFlags::getFast(),
                    ForLast ? (Signed ? MRK_SIntMax : MRK_UIntMax)
                            : (Signed ? MRK_SIntMin : MRK_UIntMin),
                    RT, Signed, IndexReduction),
        ParentRed(Parent) {
    assert((Parent && Parent->getMinMaxRecurrenceKind() != MRK_Invalid) &&
           "Incorrect parent reduction");
  }

  const VPReduction *getParentReduction() const { return ParentRed; }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPLoopEntity *V) {
    return V->getID() == IndexReduction;
  }

private:
  // Parent reduction, either min or max.
  const VPReduction *ParentRed;
};

/// Induction descriptor.
class VPInduction
    : public VPLoopEntity,
      public InductionDescriptorTempl<VPValue, VPInstruction, VPValue *> {
  friend class VPLoopEntities;
  void setNeedCloseForm(bool Val) { NeedCloseForm = Val; }

public:
  VPInduction(VPValue *Start, InductionKind K, VPValue *Step,
              VPInstruction *InductionBinOp,
              unsigned int Opc = Instruction::BinaryOpsEnd)
      : VPLoopEntity(Induction),
        InductionDescriptorTempl(Start, K, InductionBinOp), Step(Step),
        BinOpcode(Opc) {
    assert((getInductionBinOp() || BinOpcode != Instruction::BinaryOpsEnd) &&
           "Induction opcode should be set");
  }

  /// Return stride
  const VPValue *getStep() const { return Step; }

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

  /// Returns binary opcode of the induction operator. Hides parent's method.
  unsigned int getInductionOpcode() const;

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPLoopEntity *V) {
    return V->getID() == Induction;
  }

private:
  const VPValue *Step;
  unsigned int BinOpcode; // Explicitly set opcode.
  bool NeedCloseForm = false;
};

/// Private descriptor. Privates can be declared explicitly or detected
/// by analysis.
class VPPrivate : public VPLoopEntity {
  // The assignment is under condition.
  bool IsConditional;
  // Is defined explicitly.
  bool IsExplicit;
  // The assignment instruction.
  VPInstruction *AssignInst;

public:
  VPPrivate(VPInstruction *Inst, bool Conditional, bool Explicit)
      : VPLoopEntity(Private), IsConditional(Conditional),
        IsExplicit(Explicit), AssignInst(Inst) {}

  bool isConditional() const { return IsConditional; }
  bool isExplicit() const { return IsExplicit; }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPLoopEntity *V) {
    return V->getID() == Private;
  }
};

/// Complimentary class that describes memory locations of the loop entities.
/// The VPLoopEntity is linked with this descriptor through its ExitingValue
/// field.
class VPInMemoryEntity {
public:
  VPInMemoryEntity(const VPValue *AllocaInst) : MemoryPtr(AllocaInst) {}

  /// Return memory address of this var.
  const VPValue *getMemoryPtr() const { return MemoryPtr; }

  /// Sometime keeping an entity in memory is unnecessary, i.e. when it's
  /// address is not escaping either to a function call or through a pointer.
  /// This bit shows that we can generate code using a register, eliminating
  /// load/store inside the loop and storing just last value.
  bool canRegisterize() const { return CanRegisterize; }

  /// Return true if memory is safe for SOA, i.e. all uses inside the loop
  /// are known and there are no layout-casts.
  bool isSafeSOA() const { return SafeSOA; }

  /// Return true if it's profitable to make SOA transformation, i.e. there
  /// is at least one unit-stride load/store to that memory (in case of
  /// private array), or the memory is a scalar structure.
  bool isProfitableSOA() const { return ProfitableSOA; }

private:
  friend class VPLoopEntities;
  // Interface for analyzer to set the bits.
  void setCanRegisterize(bool Val) { CanRegisterize = Val; }
  void setSafeSOA(bool Val) { SafeSOA = Val; }
  void setProfitableSOA(bool Val) { ProfitableSOA = Val; }

  // Allocation instruction.
  const VPValue *MemoryPtr;

  bool CanRegisterize = false;
  bool SafeSOA = false;
  bool ProfitableSOA = false;
};

/// Class to hold/analyze VPLoop enitites - reductions, inductions, and
/// privates. This is a proxy for data classified by legalization analysis, to
/// be able to operate in terms of VPValue. It encapsulates also some additional
/// analysis that can be done on VPLoop body.
class VPLoopEntities {
  using RecurrenceKind = VPReduction::RecurrenceKind;
  using MinMaxRecurrenceKind = VPReduction::MinMaxRecurrenceKind;
  using InductionKind = VPInduction::InductionKind;

public:
  /// Add reduction described by \p K, \p MK, and \p Signed,
  /// with starting instruction \p Instr, incoming value \p Incoming, exiting
  /// instruction \p Exit and alloca-instruction \p AI.
  void addReduction(VPInstruction *Instr, VPValue *Incoming,
                    VPInstruction *Exit, RecurrenceKind K, FastMathFlags FMF,
                    MinMaxRecurrenceKind MK, Type *RT, bool Signed,
                    VPValue *AI = nullptr);
  /// Add index part of min/max+index reduction with parent (min/max) reduction
  /// \p Parent, starting instruction \pInstr, incoming value \p Incoming,
  /// exiting instruction \p Exit, \p Signed data type \p RT, and
  /// alloca-instruction \p AI. The \p ForLast flag indicates whether we need
  /// the last index or the first one.
  void addIndexReduction(VPInstruction *Instr, VPReduction *Parent,
                         VPValue *Incoming, VPInstruction *Exit, Type *RT,
                         bool Signed, bool ForLast, VPValue *AI = nullptr);

  /// Add induction of kind \p K, with opcode \p Opc or binary operation
  /// \p InductionBinOp, starting instruction \pStart, incoming value
  /// \p Incoming, stride \p Step, and alloca-instruction \p AI.
  void addInduction(VPInstruction *Start, VPValue *Incoming, InductionKind K,
                    VPValue *Step, VPInstruction *InductionBinOp,
                    unsigned int Opc, VPValue *AI = nullptr);

  /// Add private for instruction \p Assign, which is \p Conditional and
  /// \p Explicit, and alloca-instruction \p AI.
  void addPrivate(VPInstruction *Assign, bool isConditional, bool Explicit,
                  VPValue *AI = nullptr);

  /// Return reduction descriptor if instruction \p Instr is used in
  /// calculations of reduction. Not only starting phis are mapped, the
  /// instructions inside loop body should check this for proper masking of
  /// reduction operations.
  VPReduction *getReduction(const VPInstruction *Instr) const {
    return find(ReductionMap, Instr);
  }

  /// Return induction descriptor by \p Instr.
  VPInduction *getInduction(const VPInstruction *Instr) const {
    return find(InductionMap, Instr);
  }

  /// Return private descriptor by instruction.
  VPPrivate *getPrivate(const VPInstruction *Instr) const {
    return find(PrivateMap, Instr);
  }

  /// Get descriptor for an entity's memory.
  VPInMemoryEntity *getMemoryDescriptor(VPLoopEntity *E);

  /// Entities lists
  typedef DenseMap<const VPInstruction *, std::unique_ptr<VPReduction>> VPReductionMap;
  typedef DenseMap<const VPInstruction *, std::unique_ptr<VPInduction>> VPInductionMap;
  typedef DenseMap<const VPInstruction *, std::unique_ptr<VPPrivate>> VPPrivateMap;

  /// Iterators to iterate through entities lists.
  VPInductionMap::iterator inductionsBegin();
  VPInductionMap::iterator inductionsEnd();

  VPReductionMap::iterator reductionsBegin();
  VPReductionMap::iterator reductionsEnd();

  VPPrivateMap::iterator privatesBegin();
  VPPrivateMap::iterator privatesEnd();

  VPIndexReduction *getMinMaxIndex(const VPReduction *Red) {
    MinMaxIndexTy::const_iterator It = MinMaxIndexes.find(Red);
    if (It != MinMaxIndexes.end())
      return It->second;
    return nullptr;
  }

  VPLoopEntities(VPlan *P) : Plan(P) {}

  VPValue *getReductionIdentiy(const VPReduction *Red) const;

private:
  VPlan *Plan;

  VPReductionMap ReductionMap;
  VPInductionMap InductionMap;
  VPPrivateMap PrivateMap;

  // Mapping of VPLoopEntity to VPInMemoryEntity.
  typedef DenseMap<VPLoopEntity *, std::unique_ptr<VPInMemoryEntity>> MemDescrTy;
  MemDescrTy MemoryDescriptors;

  // MinMax reduction to index reduction
  typedef DenseMap<VPReduction *, VPIndexReduction *> MinMaxIndexTy;
  MinMaxIndexTy MinMaxIndexes;

  // Find an item in the map defined as T<K, std::unique_ptr<item>>
  template <typename T, class K>
  typename T::mapped_type::element_type *find(T &Map, K Key) const {
    typename T::const_iterator It = Map.find(Key);
    if (It != Map.end())
      return It->second.get();
    return nullptr;
  }
  void createMemDescFor(VPLoopEntity *E, VPValue *AI);
};
} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLOOPANALYSIS_H

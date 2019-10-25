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
#include "llvm/Analysis/IVDescriptors.h"
#include <map>

using namespace llvm;

extern cl::opt<bool> LoopEntityImportEnabled;
extern cl::opt<bool> VPlanUseVPEntityInstructions;

namespace llvm {

class ScalarEvolution;
class LoopInfo;

namespace vpo {

class VPValue;
class VPLoopRegion;
class VPlan;
class IntelVPlanUtils;
class VPLoop;
class VPlan;
class VPValue;
class VPConstant;
class VPInstruction;
class VPPHINode;
class VPBlockBase;
class VPBuilder;

/// Base class for loop entities
class VPLoopEntity {
public:
  enum { Reduction, IndexReduction, Induction, Private };
  unsigned char getID() const { return SubclassID; }

  virtual ~VPLoopEntity() = 0;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const { dump(errs()); }
  virtual void dump(raw_ostream &OS) const = 0;
#endif

  void setIsMemOnly(bool V) { IsMemOnly = V; }
  bool getIsMemOnly() const { return IsMemOnly; }

  const SmallVectorImpl<VPValue *> &getLinkedVPValues() const {
    return LinkedVPValues;
  }
  void addLinkedVPValue(VPValue *Val) {
    if (Val != nullptr)
      LinkedVPValues.push_back(Val);
  }

protected:
  // No need for public constructor.
  explicit VPLoopEntity(unsigned char Id, bool IsMem)
      : IsMemOnly(IsMem), SubclassID(Id){};

  bool IsMemOnly;

private:
  const unsigned char SubclassID;
  SmallVector<VPValue *, 2> LinkedVPValues;
};

/// Recurrence descriptor
class VPReduction
    : public VPLoopEntity,
      public RecurrenceDescriptorTempl<VPValue, VPInstruction, VPValue *> {
  using RDTempl = RecurrenceDescriptorTempl<VPValue, VPInstruction, VPValue *>;

public:
  VPReduction(VPValue *Start, VPInstruction *Exit, RecurrenceKind K,
              FastMathFlags FMF, MinMaxRecurrenceKind MK, Type *RT, bool Signed,
              bool IsMemOnly = false, unsigned char Id = Reduction)
      : VPLoopEntity(Id, IsMemOnly),
        RDTempl(Start, Exit, K, FMF, MK, RT, Signed) {}

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPLoopEntity *V) {
    return V->getID() == Reduction;
  }

  unsigned getReductionOpcode() const {
    return getReductionOpcode(getRecurrenceKind(), getMinMaxRecurrenceKind());
  }

  bool isMinMax() const { return MinMaxKind != MRK_Invalid; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  virtual void dump(raw_ostream &OS) const;
#endif
  static unsigned getReductionOpcode(RecurrenceKind K, MinMaxRecurrenceKind MK);
};

/// Descriptor of the index part of min/max+index reduction.
/// In addition to VPReduction, contains pointer to a parent min/max reduction
/// descriptor.
class VPIndexReduction : public VPReduction {
public:
  VPIndexReduction(const VPReduction *Parent, VPValue *Start,
                   VPInstruction *Exit, Type *RT, bool Signed, bool ForLast,
                   bool IsMemOnly = false)
      : VPReduction(Start, Exit, RK_IntegerMinMax, FastMathFlags::getFast(),
                    ForLast ? (Signed ? MRK_SIntMax : MRK_UIntMax)
                            : (Signed ? MRK_SIntMin : MRK_UIntMin),
                    RT, Signed, IsMemOnly, IndexReduction),
        ParentRed(Parent) {
    assert((Parent && Parent->getMinMaxRecurrenceKind() != MRK_Invalid) &&
           "Incorrect parent reduction");
  }

  const VPReduction *getParentReduction() const { return ParentRed; }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPLoopEntity *V) {
    return V->getID() == IndexReduction;
  }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS) const override;
#endif
private:
  // Parent reduction, either min or max.
  const VPReduction *ParentRed;
};

/// Induction descriptor.
class VPInduction
    : public VPLoopEntity,
      public InductionDescriptorTempl<VPValue, VPInstruction, VPValue *> {
  friend class VPLoopEntityList;

public:
  VPInduction(VPValue *Start, InductionKind K, VPValue *Step,
              VPInstruction *InductionBinOp, bool IsMemOnly = false,
              unsigned int Opc = Instruction::BinaryOpsEnd)
      : VPLoopEntity(Induction, IsMemOnly),
        InductionDescriptorTempl(Start, K, InductionBinOp), Step(Step),
        BinOpcode(Opc) {
    assert((getInductionBinOp() || BinOpcode != Instruction::BinaryOpsEnd) &&
           "Induction opcode should be set");
  }

  /// Return stride
  VPValue *getStep() const { return Step; }

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
  unsigned int BinOpcode; // Explicitly set opcode.
  bool NeedCloseForm = false;
};

/// Private descriptor. Privates can be declared explicitly or detected
/// by analysis.
class VPPrivate : public VPLoopEntity {
  friend class VPLoopEntityList;

public:
  // Currently only used for VPPrivates. In future, this can be hoisted to
  // VPLoopEntity.
  using VPEntityAliasesTy = MapVector<VPValue *, VPInstruction *>;

  VPPrivate(VPInstruction *FinalI, VPEntityAliasesTy &&InAliases,
            bool Conditional, bool Last, bool Explicit, bool IsMemOnly = false)
      : VPLoopEntity(Private, IsMemOnly), IsConditional(Conditional),
        IsLast(Last), IsExplicit(Explicit), FinalInst(FinalI),
        Aliases(std::move(InAliases)) {}

  bool isConditional() const { return IsConditional; }
  bool isLast() const { return IsLast; }
  bool isExplicit() const { return IsExplicit; }

  // TODO: Consider making this method virtual and hence available to all
  // entities.
  // Iterator-range for the aliases-set.
  iterator_range<VPEntityAliasesTy::const_iterator> aliases() const {
    return iterator_range<VPEntityAliasesTy::const_iterator>(Aliases.begin(),
                                                             Aliases.end());
  }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPLoopEntity *V) {
    return V->getID() == Private;
  }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS) const override;
#endif

private:
  // The assignment is under condition.
  bool IsConditional;

  // The last-value is used outside the loop.
  bool IsLast;

  // Is defined explicitly.
  bool IsExplicit;

  // The assignment instruction.
  VPInstruction *FinalInst;

  // Map that stores the VPExternalDef->VPInstruction mapping. These are alias
  // instructions to existing loop-private, which are outside the loop-region.
  VPEntityAliasesTy Aliases;
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

  /// Return true if memory is safe for SOA, i.e. all uses inside the loop
  /// are known and there are no layout-casts.
  bool isSafeSOA() const { return SafeSOA; }

  /// Return true if it's profitable to make SOA transformation, i.e. there
  /// is at least one unit-stride load/store to that memory (in case of
  /// private array), or the memory is a scalar structure.
  bool isProfitableSOA() const { return ProfitableSOA; }
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
  void setSafeSOA(bool Val) { SafeSOA = Val; }
  void setProfitableSOA(bool Val) { ProfitableSOA = Val; }

  // Allocation instruction.
  VPValue *MemoryPtr;

  bool CanRegisterize = false;
  bool SafeSOA = false;
  bool ProfitableSOA = false;
};

/// Class to hold/analyze VPLoop enitites - reductions, inductions, and
/// privates. This is a proxy for data classified by legalization analysis, to
/// be able to operate in terms of VPValue. It encapsulates also some additional
/// analysis that can be done on VPLoop body.
class VPLoopEntityList {
  using RecurrenceKind = VPReduction::RecurrenceKind;
  using MinMaxRecurrenceKind = VPReduction::MinMaxRecurrenceKind;
  using InductionKind = VPInduction::InductionKind;
  using VPEntityAliasesTy = VPPrivate::VPEntityAliasesTy;

public:
  /// Add reduction described by \p K, \p MK, and \p Signed,
  /// with starting instruction \p Instr, incoming value \p Incoming, exiting
  /// instruction \p Exit and alloca-instruction \p AI.
  VPReduction *addReduction(VPInstruction *Instr, VPValue *Incoming,
                    VPInstruction *Exit, RecurrenceKind K, FastMathFlags FMF,
                    MinMaxRecurrenceKind MK, Type *RT, bool Signed,
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
                                      VPValue *AI = nullptr,
                                      bool ValidMemOnly = false);

  /// Add induction of kind \p K, with opcode \p Opc or binary operation
  /// \p InductionBinOp, starting instruction \pStart, incoming value
  /// \p Incoming, stride \p Step, and alloca-instruction \p AI.
  VPInduction *addInduction(VPInstruction *Start, VPValue *Incoming,
                            InductionKind K, VPValue *Step,
                            VPInstruction *InductionBinOp, unsigned int Opc,
                            VPValue *AI = nullptr, bool ValidMemOnly = false);

  /// Add private corresponding to \p Alloca along with the final store
  /// instruction which writes to the private memory witin the for-loop. Also
  /// store other relavant attributes of the private like the conditional, last
  /// and explicit.
  VPPrivate *addPrivate(VPInstruction *FinalI, VPEntityAliasesTy &PtrAliases,
                        bool IsConditional, bool IsLast, bool Explicit,
                        VPValue *AI = nullptr, bool ValidMemOnly = false);

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

  /// Mapping of VPValues to entities. Created after entities lists are formed
  /// to ensure correct masking.
  using VPReductionMap = DenseMap<const VPValue *, const VPReduction *>;
  using VPInductionMap = DenseMap<const VPValue *, const VPInduction *>;
  using VPPrivateMap = DenseMap<const VPValue *, const VPPrivate *>;

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

  VPIndexReduction *getMinMaxIndex(const VPReduction *Red) {
    MinMaxIndexTy::const_iterator It = MinMaxIndexes.find(Red);
    if (It != MinMaxIndexes.end())
      return It->second;
    return nullptr;
  }

  VPLoopEntityList(VPlan &P, VPLoop &L) : Plan(P), Loop(L) {}

  VPValue *getReductionIdentity(const VPReduction *Red) const;
  static bool isMinMaxInclusive(const VPReduction &Red);

  void insertVPInstructions(VPBuilder &Builder);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS, const VPBlockBase *LoopHeader = nullptr) const;
  void dump() const { dump(errs()); }
#endif
  const VPLoop &getLoop() const { return Loop; }

  /// Return true if live out value of the induction \p Ind is calculated on the
  /// penultimate iteration of the loop.
  bool isInductionLastValPreInc(const VPInduction *Ind) const;

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

  // Do Escape analysis on loop-privates.
  void doEscapeAnalysis();

  /// Return VPPHINode that corresponds to a recurrent entity.
  VPPHINode *getRecurrentVPHINode(const VPLoopEntity &E) const;

private:
  VPlan &Plan;
  VPLoop &Loop;

  VPReductionList ReductionList;
  VPInductionList InductionList;
  VPPrivatesList PrivatesList;

  VPReductionMap ReductionMap;
  VPInductionMap InductionMap;
  VPPrivateMap PrivateMap;

  // Mapping of VPLoopEntity to VPLoopEntityMemoryDescriptor.
  typedef DenseMap<VPLoopEntity *,
                   std::unique_ptr<VPLoopEntityMemoryDescriptor>>
      MemDescrTy;
  MemDescrTy MemoryDescriptors;

  // Mapping alloca instructions to memory descriptors.
  typedef DenseMap<VPValue *, VPLoopEntityMemoryDescriptor *> MemInstDescrMapTy;
  MemInstDescrMapTy MemInstructions;

  // MinMax reduction to index reduction
  typedef DenseMap<const VPReduction *, VPIndexReduction *> MinMaxIndexTy;
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
             "Inconsistensy in VPValue->Descriptor mapping");
      Map[Val] = Descr;
      Descr->addLinkedVPValue(Val);
    }
  }

  void linkValue(VPLoopEntity *E, VPValue *Val) {
    if (auto Red = dyn_cast<VPReduction>(E))
      linkValue(ReductionMap, Red, Val);
    else if (auto Red = dyn_cast<VPIndexReduction>(E))
      linkValue(ReductionMap, Red, Val);
    else if (auto Ind = dyn_cast<VPInduction>(E))
      linkValue(InductionMap, Ind, Val);
    else if (auto Priv = dyn_cast<VPPrivate>(E))
      linkValue(PrivateMap, Priv, Val);
    else
      llvm_unreachable("Unknown loop entity");
  }

  // Create private memory allocator for VPLoopEntity if the corresponding
  // memory descriptor exists.
  // If the memory descriptor exists, stores the descriptor's alloca instruction
  // into \p AI, and returns a newly created VPAllocatePrivate. In case of
  // in-register entity (i.e. no descriptor exists), the nullptr is stored in \p
  // AI and nullptr is returned.
  VPValue *createPrivateMemory(VPLoopEntity &E, VPBuilder &Builder,
                               VPValue *&AI);

  // Process initial value \p Init of entity \p E.
  // If the private memory \p PrivateMem is not null then an instruction
  // to store the \p Init into \p PrivateMem is created. Then, all occurences of
  // the scalar memory \p AI are replaced, in the loop, by \p PrivateMemory.
  // Also all relevant occurences of the start value \p Start are replaced by
  // the new \p Init.
  void processInitValue(VPLoopEntity &E, VPValue *AI, VPValue *PrivateMem,
                        VPBuilder &Builder, VPValue &Init, Type *Ty,
                        VPValue &Start);

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
                                VPValue &PrivateMem);

  VPInstruction *getInductionLoopExitInstr(const VPInduction *Induction) const;

  // Mapping function that returns the underlying raw pointer.
  template <typename EntityType>
  static inline EntityType *
  getRawPointer(const std::unique_ptr<EntityType> &En) {
    return En.get();
  }
};

class VPEntityImportDescr {
  struct DescrAlias {
    DescrAlias() = default;
    VPValue *Start = nullptr;
    SmallVector<VPInstruction *, 4> UpdateVPInsts;
  };

protected:
  VPEntityImportDescr()
      : AllocaInst(nullptr), ValidMemOnly(false), Importing(true),
        HasAlias(false){};

public:
  virtual ~VPEntityImportDescr() {}

  virtual bool isDuplicate(const VPlan *Plan, const VPLoop *Loop) const;

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

protected:
  VPValue *findMemoryUses(VPValue *Start, const VPLoop *Loop);

  virtual void clear() {
    AllocaInst = nullptr;
    ValidMemOnly = false;
    Importing = true;
    Alias.reset();
    HasAlias = false;
  }
  VPValue *AllocaInst;
  bool ValidMemOnly;
  bool Importing;
  // NOTE: We assume that a descriptor can have only one valid alias
  Optional<DescrAlias> Alias;
  bool HasAlias;
};

/// Intermediate reduction descriptor. This is a temporary data to keep
/// descriptors between two stages, gathering and importing. We can't use final
/// descriptors for that purpose as they require some consistency but in some
/// cases it can be obtained only after some analysis, at the second stage of
/// importing.
class ReductionDescr : public VPEntityImportDescr {
  using RecurrenceKind = VPReduction::RecurrenceKind;
  using MinMaxRecurrenceKind = VPReduction::MinMaxRecurrenceKind;
  using BaseT = VPEntityImportDescr;
public:
  ReductionDescr() {clear();}

  VPInstruction *getStartPhi() const { return StartPhi; }
  VPValue *getStart() const { return Start; }
  VPInstruction *getExit() const { return Exit; }
  RecurrenceKind getKind() const { return K; }
  MinMaxRecurrenceKind getMinMaxKind() const { return MK; }
  Type *getRecType() const { return RT; }
  bool getSigned() const { return Signed; }
  VPInstruction *getLinkPhi() const { return LinkPhi; }
  iterator_range<SmallVectorImpl<VPInstruction *>::iterator>
  getUpdateVPInsts() {
    return make_range(UpdateVPInsts.begin(), UpdateVPInsts.end());
  }

  void setStartPhi(VPInstruction *V) { StartPhi = V; }
  void setStart(VPValue *V) { Start = V; }
  void setExit(VPInstruction *V) { Exit = V; }
  void setKind(RecurrenceKind V) { K = V; }
  void setMinMaxKind(MinMaxRecurrenceKind V) { MK = V; }
  void setRecType(Type *V) { RT = V; }
  void setSigned(bool V) { Signed = V; }
  void setLinkPhi(VPInstruction *V) { LinkPhi = V; }
  void addUpdateVPInst(VPInstruction *V) { UpdateVPInsts.push_back(V); }

  /// Clear the content.
  void clear() override {
    BaseT::clear();
    StartPhi = nullptr;
    Start = nullptr;
    UpdateVPInsts.clear();
    Exit = nullptr;
    K = RecurrenceKind::RK_NoRecurrence;
    MK = MinMaxRecurrenceKind::MRK_Invalid;
    RT = nullptr;
    Signed = false;
    LinkPhi = nullptr;
    LinkedVPVals.clear();
  }
  /// Check for that all non-null VPInstructions in the descriptor are in the \p
  /// Loop.
  void checkParentVPLoop(const VPlan *Plan, const VPLoop *Loop) const;

  /// Return true if not all data is completed.
  bool isIncomplete() const;
  /// Attempt to fix incomplete data using VPlan and VPLoop.
  void tryToCompleteByVPlan(const VPlan *Plan, const VPLoop *Loop);
  /// Pass the data to VPlan
  void passToVPlan(VPlan *Plan, const VPLoop *Loop);
  /// Check if current reduction descriptor duplicates another that is already
  /// imported.
  bool isDuplicate(const VPlan *Plan, const VPLoop *Loop) const override;

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

  VPInstruction *StartPhi = nullptr; // TODO: Consider changing to VPPHINode.
  VPValue *Start = nullptr;
  /// Instruction(s) in VPlan that update the reduction variable
  SmallVector<VPInstruction *, 4> UpdateVPInsts;
  VPInstruction *Exit = nullptr;
  RecurrenceKind K = RecurrenceKind::RK_NoRecurrence;
  MinMaxRecurrenceKind MK = MinMaxRecurrenceKind::MRK_Invalid;
  Type *RT = nullptr;
  bool Signed = false;
  VPInstruction *LinkPhi = nullptr; // TODO: Consider changing to VPPHINode.
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
};

/// Intermediate induction descriptor. Same as ReductionDescr above but for
/// inductions.
class InductionDescr : public VPEntityImportDescr {
  using InductionKind = VPInduction::InductionKind;
  using BaseT = VPEntityImportDescr;

public:
  InductionDescr() { clear(); }

  VPInstruction *getStartPhi() const { return StartPhi; }
  InductionKind getKind() const { return K; }
  VPValue *getStart() const { return Start; }
  VPValue *getStep() const { return Step; }
  VPInstruction *getInductionBinOp() const { return InductionBinOp; }
  unsigned getBinOpcode() const { return BinOpcode; }

  void setStartPhi(VPInstruction *V) { StartPhi = V; }
  void setKind(InductionKind V) { K = V; }
  void setStart(VPValue *V) { Start = V; }
  void setStep(VPValue *V) { Step = V; }
  void setInductionBinOp(VPInstruction *V) { InductionBinOp = V; }
  void setBinOpcode(unsigned V) { BinOpcode = V; }
  void setIsExplicitInduction(bool V) { IsExplicitInduction = V; }

  /// Clear the content.
  void clear() override {
    BaseT::clear();
    StartPhi = nullptr;
    K = InductionKind::IK_NoInduction;
    Start = nullptr;
    Step = nullptr;
    InductionBinOp = nullptr;
    BinOpcode = Instruction::BinaryOpsEnd;
  }
  /// Check for all non-null VPInstructions in the descriptor are in the \p
  /// Loop.
  void checkParentVPLoop(const VPlan *Plan, const VPLoop *Loop) const;

  /// Return true if not all data is completed.
  bool isIncomplete() const;
  /// Attemp to fix incomplete data using VPlan and VPLoop.
  void tryToCompleteByVPlan(const VPlan *Plan, const VPLoop *Loop);
  /// Pass the data to VPlan
  void passToVPlan(VPlan *Plan, const VPLoop *Loop);
  /// Check if current induction descriptor duplicates another that is already
  /// imported.
  bool isDuplicate(const VPlan *Plan, const VPLoop *Loop) const override;
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
  VPInstruction *InductionBinOp =nullptr;
  SmallVector<VPInstruction *, 4> UpdateVPInsts; // TODO: unpopulated
  unsigned BinOpcode = Instruction::BinaryOpsEnd;
  bool IsExplicitInduction = false; // TODO: unpopulated
};

/// Intermediate private descriptor. Same as ReductionDescr above but for
/// privates.
class PrivateDescr : public VPEntityImportDescr {
  using BaseT = VPEntityImportDescr;
  using VPEntityAliasesTy = VPPrivate::VPEntityAliasesTy;

public:
  PrivateDescr() { clear(); }

  VPValue *getAllocaInst() const { return AllocaInst; }
  bool isConditional() const { return IsConditional; }
  bool isLast() const { return IsLast; }
  bool isExplicit() const { return IsExplicit; }
  bool isMemOnly() const { return getValidMemOnly(); }

  /// Clear the content.
  void clear() override {
    BaseT::clear();
    PtrAliases.clear();
    AllocaInst = nullptr;
    FinalInst = nullptr;
    IsConditional = false;
    IsLast = false;
    IsExplicit = false;
  }
  /// Check for all non-null VPInstructions in the descriptor are in the \p
  /// Loop.
  void checkParentVPLoop(const VPlan *Plan, const VPLoop *Loop) const;

  /// Return true if not all data is completed.
  bool isIncomplete() const { return FinalInst == nullptr; }

  /// Attemp to fix incomplete data using VPlan and VPLoop.
  void tryToCompleteByVPlan(const VPlan *Plan, const VPLoop *Loop);

  /// Pass the data to VPlan
  void passToVPlan(VPlan *Plan, const VPLoop *Loop);

  void setAllocaInst(VPValue *AllocaI) { AllocaInst = AllocaI; }
  void setIsConditional(bool IsCond) { IsConditional = IsCond; }
  void setIsLast(bool IsLastPriv) { IsLast = IsLastPriv; }
  void setIsExplicit(bool IsExplicitVal) { IsExplicit = IsExplicitVal; }
  void setIsMemOnly(bool IsMem) { setValidMemOnly(IsMem); }
  void addAlias(VPValue *Alias, VPInstruction *I) { PtrAliases[Alias] = I; }

private:
  VPValue *AllocaInst = nullptr;
  VPInstruction *FinalInst = nullptr;
  // These are Pointer-aliases. Each Loop-private memory descriptor can have
  // multiple aliases as opposed to memory descriptors for inductions or
  // reductions. Hence, we have a separate field. TODO: consider using a
  // single/same field for every memory descriptor.
  VPEntityAliasesTy PtrAliases;
  bool IsConditional;
  bool IsLast;
  bool IsExplicit;
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

  virtual void passToVPlan(VPlan *Plan, MapperT &M) = 0;

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
  explicit VPLoopEntitiesConverter(VPlan *P)
      : VPLoopEntitiesConverterTempl<MapperT>(
            VPLoopEntitiesConverterBase::C_Final),
        Plan(P) {}
  VPLoopEntitiesConverter(const VPLoopEntitiesConverter &) = delete;

  static bool classof(const VPLoopEntitiesConverterBase *B) {
    return B->getKind() == VPLoopEntitiesConverterBase::C_Final;
  }

  /// Gathering pass.
  template<typename... OtherItersT>
  void createDescrList(LoopT *Loop, OtherItersT&... Args) {
    LoopList.emplace_back(Loop, DescrList());
    DescrList &Lst = LoopList.back().second;
    processIterators(Lst, Args...);
  }

  /// Sending pass.
  void passToVPlan(VPlan *Plan, MapperT &M) override {
    for (auto &LLItem : LoopList) {
      const VPLoop *Loop = M[LLItem.first];
      assert(Loop != nullptr && "Can't find corresponding VPLoop");
      for (auto &Descr : LLItem.second) {
        Descr.checkParentVPLoop(Plan, Loop);
        if (Descr.isIncomplete())
          Descr.tryToCompleteByVPlan(Plan, Loop);
        if (Descr.isDuplicate(Plan, Loop))
          continue; // Skip duplication
        Descr.passToVPlan(Plan, Loop);
      }
    }
  }

private:
  template <typename FirstIteratorT, typename... OtherItersT>
  static void processIterators(DescrList &Lst, FirstIteratorT &FirstIter,
                               OtherItersT &... Args) {
    processIterators(Lst, FirstIter);
    processIterators(Lst, Args...);
  }

  template <typename IteratorT>
  static void processIterators(DescrList &Lst, IteratorT &Iterator) {
    auto &ValRange = Iterator.first;
    auto &ConverterFunc = Iterator.second;
    for (auto &Iter : ValRange) {
      Lst.push_back(DescrT());
      auto &Item = Lst.back();
      ConverterFunc(Item, Iter);
    }
  }

  void processIterators(DescrList &Lst) {}

  VPlan *Plan;
  LoopListT LoopList;
};

/// This class is responsible to find and keep whether VPValue must be widened
/// or it can be kept scalar. One obivous example when widening is not needed
/// is for bottom-test, which is always scalar regardless to vectorization type.
/// Another case is related to uniform values and uniform instructions that
/// can be computed as scalar and broadcasted in vector context.
class VPlanScalVecAnalysis {
public:
  explicit VPlanScalVecAnalysis() = default;
  ~VPlanScalVecAnalysis() = default;

  bool needsScalarCode(const VPValue *Value) const {
    return (getCGCodeForValue(Value) & VectorCodeGenKind::Scalar) != 0;
  }

  bool needVectorCode(const VPValue *Value) const {
    return (getCGCodeForValue(Value) & VectorCodeGenKind::Vector) != 0;
  }

  void setNeedScalarCode(const VPValue *Value) {
    resetCGCodeForValue(Value);
    addNeedScalarCode(Value);
  }

  void setNeedVectorCode(const VPValue *Value) {
    resetCGCodeForValue(Value);
    addNeedVectorCode(Value);
  }

  void setNeedScalarAndVectorCode(const VPValue *Value) {
    resetCGCodeForValue(Value);
    addNeedScalarAndVector(Value);
  }

  void addNeedScalarCode(const VPValue *Value) {
    addCGCodeForValue(Value, VectorCodeGenKind::Scalar);
  }

  void addNeedVectorCode(const VPValue *Value) {
    addCGCodeForValue(Value, VectorCodeGenKind::Vector);
  }

  void addNeedScalarAndVector(const VPValue *Value) {
    addCGCodeForValue(Value, VectorCodeGenKind::ScalarAndVector);
  }

  /// This interface is used to mark that some \p Value must not be vectorized.
  /// NOTE: The Scalar property is not reset by resetCGCodeForValue() or by
  /// clear().
  void setAlwaysScalar(const VPValue *Value) {
    AlwaysScalar.insert(Value);
  }

  void clear(void) { ValuesCGKinds.clear(); }

private:
  enum VectorCodeGenKind {
    // Unknown kind means that code generation kind is not yet known. By
    // default that means Vector, which should be correct for all cases.
    Unknown = 0,
    // Scalar kind means that code generation must generate scalar instruction
    // for a given VPValue.
    Scalar = 1,
    // Vector kind means that code generation must widen the VPValue. In case
    // if value is uniform, its scalar value will be broadcasted.
    Vector = 2,
    // Combination of Scalar and Vector kinds. Code generation must generate
    // scalar value and vector value simultaneously.
    ScalarAndVector = 3,
  };

  SmallDenseMap<const VPValue *, VectorCodeGenKind> ValuesCGKinds;
  SmallDenseSet<const VPValue *> AlwaysScalar;

  VectorCodeGenKind getCGCode(const unsigned Bits) const {
    switch (Bits) {
    case 0: return VectorCodeGenKind::Unknown;
    case 1: return VectorCodeGenKind::Scalar;
    case 2: return VectorCodeGenKind::Vector;
    case 3: return VectorCodeGenKind::ScalarAndVector;
    default:
      llvm_unreachable("[0, 3] values are expected.");
    }
  }

  VectorCodeGenKind getCGCodeForValue(const VPValue *Value) const {
    if (AlwaysScalar.count(Value))
      return VectorCodeGenKind::Scalar;

    auto ValuesIt = ValuesCGKinds.find(Value);
    if (ValuesIt == ValuesCGKinds.end())
      return VectorCodeGenKind::Vector;

    VectorCodeGenKind CG = getCGCode(ValuesIt->second);

    return CG == VectorCodeGenKind::Unknown ? VectorCodeGenKind::Vector : CG;
  }

  void addCGCodeForValue(const VPValue *Value, const VectorCodeGenKind &CG) {
    auto ValueIt = ValuesCGKinds.find(Value);
    if (ValueIt == ValuesCGKinds.end()) {
      ValuesCGKinds.insert({Value, CG});
      return;
    }
    ValueIt->second = static_cast<VectorCodeGenKind>(
        static_cast<unsigned>(ValueIt->second) | static_cast<unsigned>(CG));
  }

  void resetCGCodeForValue(const VPValue *Value) {
    auto ValueIt = ValuesCGKinds.find(Value);
    if (ValueIt == ValuesCGKinds.end())
      return;

    ValueIt->second = VectorCodeGenKind::Unknown;
  }
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLOOPANALYSIS_H

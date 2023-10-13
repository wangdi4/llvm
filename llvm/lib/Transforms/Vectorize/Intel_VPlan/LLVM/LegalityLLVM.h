//===- LegalityLLVM.h -------------------------------------------*- C++ -*-===//
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
///   \file LegalityLLVM.h
///   VPlan vectorizer's LLVM IR legality analysis.
///
///   Split from Legality.h on 2023-09-27.
///
///   Defines the LegalityLLVM class, which performs loop entity import and
///   legality testing on an LLVM IR loop.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LLVM_LEGALITYLLVM_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LLVM_LEGALITYLLVM_H

#include "../IntelVPlanUtils.h"
#include "../Legality.h"

using namespace llvm::loopopt;

namespace llvm {

class Loop;
class PredicatedScalarEvolution;

namespace vpo {

/// \class LegalityLLVM
///
/// This class has the following purposes:
///
///   1) To import loop entities (such as reductions, inductions, and
///      privates) from the WRNVecLoopNode associated with an lLVM IR loop.
///   2) To perform legality testing on these entities.  If any entity
///      has characteristics that the VPlan vectorizer cannot currently
///      support, VPlan will bail out and provide a reason in the
///      optimization report.
///   3) To perform additional legality testing regarding the loop
///      control flow, possible unsafe aliasing, unvectorizable data
///      data types, and so forth.
///
/// These operations are performed by the canVectorize method.  Afterwards,
/// accessor methods provided by LegalityLLVM can be used to examine the
/// imported entities.
///
class LegalityLLVM final : public LegalityBase<LegalityLLVM> {
  // Explicit vpo:: to workaround gcc bug
  // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52625
  template <typename LegalityTy> friend class vpo::LegalityBase;

public:
  // Constructor.
  LegalityLLVM(Loop *L, PredicatedScalarEvolution &PSE, Function *F,
               LLVMContext *C)
      : LegalityBase(C), TheLoop(L), PSE(PSE), Induction(nullptr),
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

  /// Returns true iff nothing about \p Phi in basic block \p BB will
  /// invalidate vectorization of \p WRLp.
  bool isPHIOkayForVectorization(PHINode *Phi, BasicBlock *BB,
                                 const WRNVecLoopNode *WRLp,
                                 BasicBlock *Header);

  /// Returns true iff nothing about \p Call will invalidate vectorization
  /// of \p WRLp.
  bool isCallOkayForVectorization(CallInst *Call, const WRNVecLoopNode *WRLp);

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

  /// Vector of in-memory loop private values (allocas).
  PrivatesListTy Privates;

  /// List of explicit linears.
  LinearListTy Linears;

  /// Map of pointer values and stride.
  DenseMap<Value *, int> PtrStrides;

  /// Is this an explicit SIMD loop?
  bool IsSimdLoop = false;

public:
  /// Add stride information for pointer \p Ptr.
  /// Used for a temporary solution of teaching legality based on DA.
  // TODO: Is it okay to overwrite existing stride value?  Tracked in
  // CMPLRLLVM-52295.
  void addPtrStride(Value *Ptr, int Stride) { PtrStrides[Ptr] = Stride; }

  /// Erase pointer \p Ptr from the stride information map.
  /// Used for a temporary solution of teaching legality based on DA.
  void erasePtrStride(Value *Ptr) { PtrStrides.erase(Ptr); }

  /// Test whether this \p Phi is associated with an explicit reduction.
  bool isExplicitReductionPhi(PHINode *Phi);

  /// Return true if the specified value \p Val is a reduction variable that
  /// is written to the memory in each iteration.
  bool isInMemoryReduction(Value *Val) const;

  /// Return true if the specified value \p Val is private.
  bool isLoopPrivate(Value *Val) const;

  /// Return pointer to Linears map.
  LinearListTy *getLinears() { return &Linears; }

  /// Analyze all instructions between the SIMD clause and the loop to identify
  /// any aliasing variables to the explicit SIMD descriptors.
  void collectPreLoopDescrAliases();

  /// Analyze all instructions in loop post-exit to identify any aliasing
  /// variables to the explicit SIMD descriptor.
  void collectPostExitLoopDescrAliases();

  /// Return the iterator-range to the list of privates loop-entities.
  // Note: Windows compiler explicitly doesn't allow for const type specifier.
  inline decltype(auto) privates() const {
    return map_range(
        make_range(Privates.begin(), Privates.end()), [](auto &PrivatePair) {
          return const_cast<const PrivDescrTy *>(PrivatePair.second.get());
        });
  }

  /// Return the iterator-range to the list of privates loop-entities values.
  inline decltype(auto) privateVals() const {
    return make_first_range(Privates);
  }

  /// Return the iterator-range to the list of explicit reduction variables
  /// which are of 'Pointer Type'.
  inline decltype(auto) explicitReductionVals() const {
    return make_filter_range(
        map_range(make_second_range(ExplicitReductions),
                  [](auto &Descr) { return Descr.RedVarPtr; }),
        [](auto *Val) { return Val->getType()->isPointerTy(); });
  }

  /// Return the iterator-range to the list of in-memory reduction variables.
  inline decltype(auto) inMemoryReductionVals() const {
    return make_first_range(InMemoryReductions);
  }

  /// Return the iterator-range to the list of user-defined reduction variables.
  inline decltype(auto) userDefinedReductions() const {
    return make_range(UserDefinedReductions.begin(),
                      UserDefinedReductions.end());
  }

  /// Return the iterator-range to the list of 'linear' variables.
  inline decltype(auto) linearVals() const { return make_first_range(Linears); }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// Debug print utility to display contents of the descriptor lists.
  void dump(raw_ostream &OS) const;
  void dump() const { dump(errs()); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

private:
  /// Add an in-memory non-POD private to the vector of private values.
  void addLoopPrivate(Value *PrivVal, Type *PrivTy, Function *Constr,
                      Function *Destr, Function *CopyAssign, PrivateKindTy Kind,
                      bool IsF90) {
    Privates.insert({PrivVal, std::make_unique<PrivDescrNonPODTy>(
                                  PrivVal, PrivTy, Kind, IsF90, Constr, Destr,
                                  CopyAssign)});
  }

  /// Add an in-memory POD private to the vector of private values.
  void addLoopPrivate(Value *PrivVal, Type *PrivTy, PrivateKindTy Kind,
                      Type *F90DVElementType) {
    if (F90DVElementType)
      Privates.insert({PrivVal, std::make_unique<PrivDescrF90DVTy>(
                                    PrivVal, PrivTy, Kind, F90DVElementType)});
    else
      Privates.insert({PrivVal, std::make_unique<PrivDescrTy>(
                                    PrivVal, PrivTy, Kind, false /* isF90 */)});
  }

  /// Add linear value to Linears map.
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
      function_ref<bool(const Instruction *)> IsAliasInRelevantScope) {
    for (auto *En : LERange) {
      SetVector<const Value *> WL;
      WL.insert(En);
      while (!WL.empty()) {
        auto *HeadI = WL.pop_back_val();
        for (auto *Use : HeadI->users()) {
          const auto *UseInst = cast<Instruction>(Use);

          // We only want to analyze the blocks between the region-entry and the
          // loop-block (typically just simd.loop.preheader). This means we
          // won't loop on cycle-causing PHIs.
          if (!IsAliasInRelevantScope(UseInst))
            continue;

          // If this is a store of private pointer or any of its alias to an
          // external memory, treat the loop as unsafe for vectorization and
          // return false.
          if (const auto *SI = dyn_cast<StoreInst>(UseInst))
            if (SI->getValueOperand() == HeadI)
              return false;
          if (isTrivialPointerAliasingInst(UseInst))
            WL.insert(UseInst);
        }
      }
    }
    return true;
  }

  /// Utility function to check whether given Instruction is end directive of
  /// OMP.SIMD directive.
  bool isEndDirective(Instruction *I) const;
};
} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LLVM_LEGALITYLLVM_H

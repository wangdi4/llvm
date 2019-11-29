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

#include "llvm/ADT/MapVector.h"
#include "llvm/Analysis/IVDescriptors.h"

namespace llvm {

class PredicatedScalarEvolution;
class TargetTransformInfo;
class TargetLibraryInfo;
class Loop;
class LoopInfo;
class Function;

namespace vpo {

class VPOVectorizationLegality {
public:
  VPOVectorizationLegality(Loop *L, PredicatedScalarEvolution &PSE, Function *F)
      : TheLoop(L), PSE(PSE), Induction(nullptr), WidestIndTy(nullptr) {}

  /// Returns true if it is legal to vectorize this loop.
  bool canVectorize();

  void collectLoopUniformsForAnyVF();

  bool isUniformForTheLoop(Value *V) {
    // Each vector lane gets a different private value
    if (isLoopPrivate(V))
      return false;

    return !isa<Instruction>(V) || UniformForAnyVF.count(cast<Instruction>(V));
  }

  /// Container-class for storing the different types of Privates
  using PrivatesListTy = SetVector<Value *>;

  /// ReductionList contains the reduction descriptors for all
  /// of the reductions that were found in the loop.
  using ReductionList = MapVector<PHINode *, RecurrenceDescriptor>;

  /// The list of explicit reduction variables.
  using ExplicitReductionList = MapVector<PHINode *, std::pair<RecurrenceDescriptor, AllocaInst *>>;
  using InMemoryReductionList = MapVector<AllocaInst *,
                   std::pair<RecurrenceDescriptor::RecurrenceKind,
                             RecurrenceDescriptor::MinMaxRecurrenceKind>>;

  /// InductionList saves induction variables and maps them to the
  /// induction descriptor.
  using InductionList = MapVector<PHINode *, InductionDescriptor>;

  /// Linear list contains explicit linear specifications, mapping linear values
  /// and their strides.
  using LinearListTy = MapVector<Value *, int>;

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

  /// Return a recurrence descriptor for the given \p Phi node.
  /// (For explicit reduction variables)
  RecurrenceDescriptor &getRecurrenceDescrByPhi(PHINode *Phi) {
    assert(ExplicitReductions.count(Phi) && "Exp reduction var is not found");
    return ExplicitReductions[Phi].first;
  }

  /// Return a pointer to reduction var using the \p Phi node.
  /// (Explicit only)
  AllocaInst *getReductionPtrByPhi(PHINode *Phi) {
    assert(ExplicitReductions.count(Phi) && "Exp reduction var is not found");
    return ExplicitReductions[Phi].second;
  }

  /// Returns True if V is an induction variable in this loop.
  bool isInductionVariable(const Value *V);

  /// Returns true if the value \p V is loop invariant.
  bool isLoopInvariant(Value *V);

  /// Returns true if the access through \p Ptr is consecutive.
  int isConsecutivePtr(Value *Ptr);

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
  PrivatesListTy LastPrivates;
  PrivatesListTy CondLastPrivates;

  /// List of explicit linears.
  LinearListTy Linears;

  /// Map of pointer values and stride
  DenseMap<Value *, int> PtrStrides;

  /// Set of loop invariants - currently only GEP operands and select condition
  /// are checked for invariance.
  SmallPtrSet<Value *, 8> LoopInvariants;

  /// Map of linear items in original loop and the scalar linear item in the
  /// vector loop along with linear Step. This map is maintained only for
  /// step values 1 and -1 and is used to generate unit-stride loads/stores
  /// when possible
  std::map<Value *, std::pair<Value *, int>> UnitStepLinears;

public:
  /// Add stride information for pointer \p Ptr.
  // Used for a temporary solution of teaching legality based on DA.
  // TODO: Is it okay to overwrite existing stride value?
  void addPtrStride(Value *Ptr, int Stride) { PtrStrides[Ptr] = Stride; }

  /// Erase pointer \p Ptr from the stride information map.
  // Used for a temporary solution of teaching legality based on DA.
  void erasePtrStride(Value *Ptr) { PtrStrides.erase(Ptr); }

  /// Holds the instructions known to be uniform after vectorization for any VF.
  SmallPtrSet<Instruction *, 4> UniformForAnyVF;

  /// Add an in memory private to the vector of private values.
  void addLoopPrivate(Value *PrivVal, bool IsLast = false,
                      bool IsConditional = false) {
    Privates.insert(PrivVal);
    if (IsConditional)
      CondLastPrivates.insert(PrivVal);
    else if (IsLast)
      LastPrivates.insert(PrivVal);
  }

  /// Register explicit reduction variables provided from outside.
  void addReductionMin(Value *V, bool IsSigned);
  void addReductionMax(Value *V, bool IsSigned);
  void addReductionAdd(Value *V);
  void addReductionMult(Value *V);
  void addReductionAnd(Value *V) {
    return parseExplicitReduction(V, RecurrenceDescriptor::RK_IntegerAnd);
  }
  void addReductionXor(Value *V) {
    return parseExplicitReduction(V, RecurrenceDescriptor::RK_IntegerXor);
  }
  void addReductionOr(Value *V) {
    return parseExplicitReduction(V, RecurrenceDescriptor::RK_IntegerOr);
  }

  bool isExplicitReductionPhi(PHINode *Phi);

  // Return True if the specified value \p Val is reduction variable that
  // is written to the memory in each iteration.
  bool isInMemoryReduction(Value *Val) const;

  // Return true if the specified value \p Val is private.
  bool isLoopPrivate(Value *Val) const;

  // Return true if the specified value \p Val is private.
  bool isLoopPrivateAggregate(Value *Val) const;

  // Return True if the specified value \p Val is (unconditional) last private.
  bool isLastPrivate(Value *Val) const;

  // Return True if the specified value \p Val is conditional last private.
  bool isCondLastPrivate(Value *Val) const;

  // Add linear value to Linears map
  void addLinear(Value *LinearVal, Value *StepValue) {
    assert(isa<ConstantInt>(StepValue) &&
           "Non constant LINEAR step is not yet supported");
    ConstantInt *CI = cast<ConstantInt>(StepValue);
    int Step = *((CI->getValue()).getRawData());
    Linears[LinearVal] = Step;
  }

  // Add unit step linear value to UnitStepLinears map
  void addUnitStepLinear(Value *LinearVal, Value *NewVal, int Step) {
    UnitStepLinears[LinearVal] = std::make_pair(NewVal, Step);
  }

  // Return true if \p Val is a linear and return linear step in \p Step if
  // non-null
  bool isLinear(Value *Val, int *Step = nullptr);

  // Return true if \p Val is a unit step linear item and return linear step in
  // \p Step if non-null and New scalar value in NewScal if non-null
  bool isUnitStepLinear(Value *Val, int *Step = nullptr,
                        Value **NewScal = nullptr);

  // Return pointer to Linears map
  LinearListTy *getLinears() {
    return &Linears;
  }

  // Return Pointer to Privates map
  const PrivatesListTy &getPrivates() const { return Privates; }

  // Return Pointer to CondPrivates map
  const PrivatesListTy &getCondPrivates() const { return CondLastPrivates; }

  // Return Pointer to LastPrivates map
  const PrivatesListTy &getLastPrivates() const { return LastPrivates; }

private:
  // Find pattern inside the loop for matching the explicit
  // reduction variable \p V.
  void parseExplicitReduction(Value *V,
                              RecurrenceDescriptor::RecurrenceKind Kind,
                              RecurrenceDescriptor::MinMaxRecurrenceKind Mrk =
                                  RecurrenceDescriptor::MRK_Invalid);
  /// Parsing Min/Max reduction patterns.
  void parseMinMaxReduction(AllocaInst *V,
                            RecurrenceDescriptor::RecurrenceKind Kind,
                            RecurrenceDescriptor::MinMaxRecurrenceKind Mrk);
  /// Parsing arithmetic reduction patterns.
  void parseBinOpReduction(AllocaInst *V,
                           RecurrenceDescriptor::RecurrenceKind Kind);

  /// Return true if the explicit reduction uses Phi nodes.
  bool doesReductionUsePhiNodes(Value *RedVarPtr, PHINode *&LoopHeaderPhiNode,
                                Value *&StartV);
  /// Return true if the explicit reduction variable uses private memory on
  /// each iteration.
  bool isReductionVarStoredInsideTheLoop(Value *RedVarPtr);
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELLOOPVECTORIZERLEGALITY_H

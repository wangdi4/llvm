//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   LoopVectorizerCodeGen.h -- LLVM IR Code generation
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_VPLAN_LOOPVECTORIZERCODEGEN_H
#define LLVM_TRANSFORMS_VECTORIZE_VPLAN_LOOPVECTORIZERCODEGEN_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

namespace llvm { // LLVM Namespace

class TargetTransformInfo;

class VPOVectorizationLegality {
public:
  VPOVectorizationLegality(Loop *L, PredicatedScalarEvolution& PSE,
                           TargetLibraryInfo *TLI, TargetTransformInfo *TTI,
                           Function *F, LoopInfo *LI)
    : TheLoop(L), PSE(PSE), TLI(TLI), TTI(TTI), LI(LI),
    Induction(nullptr), WidestIndTy(nullptr) {}

  /// Returns true if it is legal to vectorize this loop.
  bool canVectorize();

  /// ReductionList contains the reduction descriptors for all
  /// of the reductions that were found in the loop.
  typedef DenseMap<PHINode *, RecurrenceDescriptor> ReductionList;

  /// InductionList saves induction variables and maps them to the
  /// induction descriptor.
  typedef MapVector<PHINode *, InductionDescriptor> InductionList;

  /// Returns the Induction variable.
  PHINode *getInduction() { return Induction; }

  /// Returns the induction variables found in the loop.
  InductionList *getInductionVars() { return &Inductions; }

  /// Returns True if V is an induction variable in this loop.
  bool isInductionVariable(const Value *V);

  /// Returns true if the value \p V is loop invariant.
  bool isLoopInvariant(Value *V);

  /// Returns true if the access through \p Ptr is consecutive.
  bool isConsecutivePtr(Value *Ptr);

  /// Returns True if PN is a reduction variable in this loop.
  bool isReductionVariable(PHINode *PN) { return Reductions.count(PN); }

  Loop *getLoop() { return TheLoop; }

  PredicatedScalarEvolution& getPSE() { return PSE; }

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
  PredicatedScalarEvolution PSE;
  /// Target Library Info.
  TargetLibraryInfo *TLI;
  /// Target Transform Info
  const TargetTransformInfo *TTI;
  LoopInfo *LI;
  /// Holds the integer induction variable. This is the counter of the
  /// loop.
  PHINode *Induction;
  /// Holds the reduction variables.
  ReductionList Reductions;
  /// Holds all of the induction variables that we found in the loop.
  /// Notice that inductions don't need to start at zero and that induction
  /// variables can be pointers.
  InductionList Inductions;

  /// Holds the widest induction type encountered.
  Type *WidestIndTy;

  /// A set of Phi nodes that may be used outside the loop.
  SmallPtrSet<Value *, 4> AllowedExit;
};

// LVCodeGen generates vector code by widening of scalars into
// appropriate length vectors.

class VPOCodeGen {
public:
  VPOCodeGen(Loop *OrigLoop, PredicatedScalarEvolution &PSE,
                    LoopInfo *LI, DominatorTree *DT,
                    TargetLibraryInfo *TLI,
                    const TargetTransformInfo *TTI, unsigned VecWidth,
                    unsigned UnrollFactor, VPOVectorizationLegality *LVL)
    : OrigLoop(OrigLoop), PSE(PSE), LI(LI), DT(DT), TLI(TLI), TTI(TTI),
    Legal(LVL), TripCount(nullptr), VectorTripCount(nullptr),
    Induction(nullptr), OldInduction(nullptr), VF(VecWidth), UF(UnrollFactor),
    Builder(PSE.getSE()->getContext()), StartValue(nullptr),
    StrideValue(nullptr), LoopVectorPreHeader(nullptr),
    LoopScalarPreHeader(nullptr), LoopMiddleBlock(nullptr),
    LoopExitBlock(nullptr), LoopVectorBody(nullptr), LoopScalarBody(nullptr){
    }

  ~VPOCodeGen() {}

  // Take care of phi's to fix: reduction, 1st-order-recurrence, loop-closed.
  bool finalizeLoop();

  // Create the vector loop skeleton which iterates from StartIndex
  // to StartIndex +  VL * Stride * TripCount. We also setup the control
  // flow such that the scalar loop is skipped.
  void createEmptyLoop();

  // Widen the given instruction to VL wide vector instruction
  void vectorizeInstruction(Instruction *Inst);

  IRBuilder<>& getBuilder() { return Builder; }

  BasicBlock *getLoopVectorPH() { return LoopVectorPreHeader;  }

  static void collectTriviallyDeadInstructions(
    Loop *OrigLoop, VPOVectorizationLegality *Legal,
    SmallPtrSetImpl<Instruction *> &DeadInstructions);

private:

  /// Emit blocks of vector loop
  /// Emit a bypass check to see if we have enough iterations \p Count to
  /// execute one vector loop.
  void  emitMinimumIterationCountCheck(Loop *L, Value *Count);

  /// Emit a bypass check to see if the vector trip count is nonzero.
  void  emitVectorLoopEnteredCheck(Loop *L, BasicBlock *Bypass);

  /// Emit resume block for combining bypassed values and the values coming
  /// from the vector loop.
  void  emitResume(Value *CountRoundDown);

  /// Check whether the original loop trip count \p Count is equal to vector
  /// loop trip count \p CountRoundDown. In this case we can bypass the scalar
  /// remainder.
  void  emitEndOfVectorLoop(Value *Count, Value *CountRoundDown);

  // Return the trip count for the scalar loop. Returns 0 for non-constant trip
  // count loops.
  uint64_t getConstTripCount() const;

  // Return the trip count for the scalar loop. 
  Value *getTripCount() const { return TripCount; }

  /// Returns (and creates if needed) the original loop trip count.
  Value *getOrCreateTripCount(Loop *L);

  /// Returns (and creates if needed) the trip count of the widened loop.
  Value *getOrCreateVectorTripCount(Loop *L);

  /// Reverse vector elements
  Value *reverseVector(Value *Vec);

  /// Create the primary induction variable for vector loop.
  PHINode *createInductionVariable(Loop *L, Value *Start,
                                   Value *End, Value *Step);

  /// Insert the new loop to the loop hierarchy and pass manager
  /// and update the analysis passes.
  void updateAnalysis();

  /// This function adds (StartIdx, StartIdx + Step, StartIdx + 2*Step, ...)
  /// to each vector element of Val. The sequence starts at StartIndex.
  Value *getStepVector(Value *Val, int StartIdx, Value *Step);

  /// Create vector and scalar version of the same induction variable.
  /// We don't need always the both, cost model may provide information
  /// about this.
  void widenIntInduction(PHINode *IV);

  /// Create a vector version of FP induction.
  void widenFpInduction(PHINode *IV);

  /// Create a vector version of induction.
  void createVectorIntInductionPHI(PHINode *IV, Instruction *& VectorInd);

  /// The original loop.
  Loop *OrigLoop;
  /// A wrapper around ScalarEvolution used to add runtime SCEV checks. Applies
  /// dynamic knowledge to simplify SCEV expressions and converts them to a
  /// more usable form.
  PredicatedScalarEvolution &PSE;
  /// Loop Info.
  LoopInfo *LI;
  /// Dominator Tree.
  DominatorTree *DT;

  /// Target Library Info.
  TargetLibraryInfo *TLI;
  /// Target Transform Info.
  const TargetTransformInfo *TTI;

  VPOVectorizationLegality *Legal;

  // Loop trip count
  Value *TripCount;
  // Vectorized loop trip count.
  Value *VectorTripCount;

  /// The new Induction variable which was added to the new block.
  PHINode *Induction;
  /// The induction variable of the old basic block.
  PHINode *OldInduction;

  // Vector factor or vector length to use. Each scalar instruction is widened
  // to operate on this number of operands.
  unsigned VF;

  // Unroll factor
  unsigned UF;

  // IR Builder to use to generate instructions
  IRBuilder<> Builder;

  // Starting value of loop induction variable
  Value *StartValue;

  // Loop increment
  ConstantInt *StrideValue;

  // Map of widened value.
  std::map<Value *, Value *> WidenMap;
  // Map of scalar values.
  std::map<Value *, Value *> ScalarMap;

  // Holds the end values for each induction variable. We save the end values
  // so we can later fix-up the external users of the induction variables.
  DenseMap<PHINode *, Value *> IVEndValues;

  // --- Vectorization state ---

  /// The vector-loop preheader.
  BasicBlock *LoopVectorPreHeader;
  /// The scalar-loop preheader.
  BasicBlock *LoopScalarPreHeader;
  /// Middle Block between the vector and the scalar.
  BasicBlock *LoopMiddleBlock;
  /// The ExitBlock of the scalar loop.
  BasicBlock *LoopExitBlock;
  /// The vector loop body.
  BasicBlock *LoopVectorBody;
  /// The scalar loop body.
  BasicBlock *LoopScalarBody;
  /// A list of all bypass blocks. The first block is the entry of the loop.
  SmallVector<BasicBlock *, 4> LoopBypassBlocks;

  // Widen the given load instruction. EmitIntrinsic needs to be set to true
  // when we can start emitting masked_gather intrinsic once we have support
  // in code gen. Without code gen support, we will serialize the intrinsic.
  // As a result, we simply serialize the instruction for now.
  void vectorizeLoadInstruction(Instruction *Inst, bool EmitIntrinsic = false);

  // Vectorize the given loop invariant load.
  void vectorizeLoopInvariantLoad(Instruction *Inst);

  // Widen the given store instruction. EmitIntrinsic needs to be set to true
  // when we can start emitting masked_scatter intrinsic once we have support
  // in code gen. Without code gen support, we will serialize the intrinsic.
  // As a result, we simply serialize the instruction for now.
  void vectorizeStoreInstruction(Instruction *Inst, bool EmitIntrinsic = false);

  // Widen call instruction parameters and return. Currently, this is limited
  // to svml function support that is hooked in to TLI. Later, this can be
  // extended to user-defined vector functions.
  void vectorizeCallInstruction(CallInst *Call);

  // Widen the given PHI instruction. For now we assume this corresponds to
  // the Induction PHI.
  void vectorizePHIInstruction(Instruction *Inst);

  void vectorizeReductionPHI(PHINode *Inst);

  // Vectorize the given instruction that cannot be widened using serialization.
  // This is done using a sequence of extractelement, Scalar Op, InsertElement
  // instructions.
  void serializeInstruction(Instruction *Inst);

  // Get the widened vector value for given value V. If the scalar value
  // has not been widened, we widen it by VL and store it in WidenMap
  // before returning the widened value
  Value *getVectorValue(Value *V);

  Value *getScalarValue(Value *V);

  // Return a vector Vl wide: <Val, Val + Stride,
  // ... VAL + (VL - 1) * Stride>
  Value *getStrideVector(Value *Val, Value *Stride);
};

} // End LLVM Namespace

#endif // LLVM_TRANSFORMS_VPO_VECOPT_VPOAVRLLVMCODEGEN_H

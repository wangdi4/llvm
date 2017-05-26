//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/Intel_VectorVariant.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

namespace llvm { // LLVM Namespace

class VPOVectorizationLegality {
public:
  VPOVectorizationLegality(Loop *L, PredicatedScalarEvolution& PSE,
                           TargetLibraryInfo *TLI, TargetTransformInfo *TTI,
                           Function *F, LoopInfo *LI, DominatorTree *DT)
    : TheLoop(L), PSE(PSE), TLI(TLI), TTI(TTI), LI(LI), DT(DT),
    Induction(nullptr), WidestIndTy(nullptr) {}

  /// Returns true if it is legal to vectorize this loop.
  bool canVectorize();

  void collectLoopUniformsForAnyVF();

  bool isUniformForTheLoop(Value *V) {
    // Each vector lane gets a different private value
    if (isLoopPrivate(V))
      return false;
    
    return !isa<Instruction>(V) || UniformForAnyVF.count(cast<Instruction>(V));
  }

  //iterator_range<Instruction *> uniforms() const{
  //  return make_range(UniformForAnyVF.begin(), UniformForAnyVF.end());
  //}
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

  /// Returns the reduction variables found in the loop.
  ReductionList *getReductionVars() { return &Reductions; }

  /// Returns True if V is an induction variable in this loop.
  bool isInductionVariable(const Value *V);

  /// Returns true if the value \p V is loop invariant.
  bool isLoopInvariant(Value *V);

  /// Returns true if the access through \p Ptr is consecutive.
  int isConsecutivePtr(Value *Ptr);

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
  /// Dominator Tree.
  DominatorTree *DT;
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
  
  /// Vector of in memory loop private values(allocas)
  SmallPtrSet<Value *, 8> Privates;
  SmallPtrSet<Value *, 8> LastPrivates;
  SmallPtrSet<Value *, 8> CondLastPrivates;

public:
  /// Holds the instructions known to be uniform after vectorization for any VF.
  SmallPtrSet<Instruction *, 4> UniformForAnyVF;

  void addLoopPrivate(Value *PrivVal, bool IsLast, bool IsConditional) {
    Privates.insert(PrivVal);
    if (IsConditional)
      CondLastPrivates.insert(PrivVal);
    else if (IsLast)
      LastPrivates.insert(PrivVal);
  }
  
  // Return true if the specified value \p Val is private.
  bool isLoopPrivate(Value *Val) const;

  // Return True if the specified value \p Val is (uncoditional) last private.
  bool isLastPrivate(Value *Val) const;

  // Return True if the specified value \p Val is conditional last private.
  bool isCondLastPrivate(Value *Val) const;
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
    LoopExitBlock(nullptr), LoopVectorBody(nullptr), LoopScalarBody(nullptr),
    MaskValue(nullptr) {
    }

  ~VPOCodeGen() {}

  // Take care of phi's to fix: reduction, 1st-order-recurrence, loop-closed.
  void finalizeLoop();

  // Create the vector loop skeleton which iterates from StartIndex
  // to StartIndex +  VF * Stride * TripCount. We also setup the control
  // flow such that the scalar loop is skipped.
  void createEmptyLoop();

  // Widen the given instruction to VF wide vector instruction
  void vectorizeInstruction(Instruction *Inst);
  // Vectorize the given instruction that cannot be widened using serialization.
  // This is done using a sequence of extractelement, Scalar Op, InsertElement
  // instructions.
  void serializeInstruction(Instruction *Inst);

  /// Collect Uniform and Scalar values for the given \p VF.
  void collectUniformsAndScalars(unsigned VF);

  IRBuilder<>& getBuilder() { return Builder; }

  BasicBlock *getLoopVectorPH() { return LoopVectorPreHeader;  }

  static void collectTriviallyDeadInstructions(
    Loop *OrigLoop, VPOVectorizationLegality *Legal,
    SmallPtrSetImpl<Instruction *> &DeadInstructions);

  // Get the widened vector value for given value V. If the scalar value
  // has not been widened, we widen it by VF and store it in WidenMap
  // before returning the widened value
  Value *getVectorValue(Value *V);

  // Get widened base pointer of in-memory private variable
  Value *getVectorPrivateBase(Value *V);

  // Get a vector of pointers corresponding to the private variable for each
  // vector lane.
  Value *getVectorPrivatePtrs(Value *ScalarPrivate);

  /// Return a value in the new loop corresponding to \p V from the original
  /// loop at vector index \p Lane. If the value has
  /// been vectorized but not scalarized, the necessary extractelement
  /// instruction will be generated.
  Value *getScalarValue(Value *V, unsigned Lane);

  /// MaskValue setter
  void setMaskValue(Value *MV) { MaskValue = MV; }

  /// Write down the condition mask of the Edge between block \p From
  /// and block \p To.
  void setEdgeMask(BasicBlock *From, BasicBlock *To, Value *Mask);

  /// Get a condition mask between block \p From and block \p To.
  Value *getEdgeMask(BasicBlock *From, BasicBlock *To);

  /// Find the best simd function variant. 
  VectorVariant* matchVectorVariant(Function *CalledFunc, bool Masked);

  /// Vectorize call arguments, or for simd functions scalarize if the arg
  /// is linear or uniform.
  void vectorizeCallArgs(CallInst *Call, VectorVariant *VecVariant,
                         SmallVectorImpl<Value*> &VecArgs,
                         SmallVectorImpl<Type*> &VecArgTys);
  
  /// Add an in memory private to the vector of private values.
  void addLoopPrivate(Value *PrivVal, bool IsLastP = false,
                      bool IsConditional = false) {
    Legal->addLoopPrivate(PrivVal, IsLastP, IsConditional);
  }
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

  /// Serialize instruction that requires predication.
  void serializeWithPredication(Instruction *Inst);

  /// Predicate conditional instructions that require predication on their
  /// respective conditions.
  void predicateInstructions();

  /// Reverse vector elements
  Value *reverseVector(Value *Vec);

  /// Create the primary induction variable for vector loop.
  PHINode *createInductionVariable(Loop *L, Value *Start,
                                   Value *End, Value *Step);

  /// Handle all cross-iteration phis in the header.
  void fixCrossIterationPHIs();

  /// Fix a reduction cross-iteration phi. This is the second phase of
  /// vectorizing this phi node.
  void fixReduction(PHINode *Phi);

  /// Set up the values of the IVs correctly when exiting the vector loop.
  void fixupIVUsers(PHINode *OrigPhi, const InductionDescriptor &II,
                    Value *CountRoundDown, Value *EndValue,
                    BasicBlock *MiddleBlock);

  /// Fix instructions that use loop private values outside the vectorized loop.
  void fixupLoopPrivates();

  /// Write last value of unconditional private variable.
  void writePrivateValAfterLoop(Value *OrigPrivate);

  /// Write last value of conditional private.
  void writeCondPrivateValAfterLoop(Value *OrigPrivate);

  /// Get an index of last written lane using Mask value.
  Value *getLastLaneFromMask(Value *MaskPtr);

  /// \brief The Loop exit block may have single value PHI nodes where the
  /// incoming value is 'Undef'. While vectorizing we only handled real values
  /// that were defined inside the loop. Here we fix the 'undef case'.
  void fixLCSSAPHIs();

  /// Insert the new loop to the loop hierarchy and pass manager
  /// and update the analysis passes.
  void updateAnalysis();

  /// This function adds (StartIdx, StartIdx + Step, StartIdx + 2*Step, ...)
  /// to each vector element of Val. The sequence starts at StartIndex.
  Value *getStepVector(Value *Val, int StartIdx, Value *Step,
                       Instruction::BinaryOps BinOp);

  /// Create a broadcast instruction. This method generates a broadcast
  /// instruction (shuffle) for loop invariant values and for the induction
  /// value. If this is the induction variable then we extend it to N, N+1, ...
  /// this is needed because each iteration in the loop corresponds to a SIMD
  /// element.
  Value *getBroadcastInstrs(Value *V);

  /// Create vector and scalar version of the same induction variable.
  void widenIntOrFpInduction(PHINode *IV);

  /// Widen Phi node, which is not an induction variable. This Phi node
  /// is a result of merging blocks ruled out by uniform branch.
  void widenNonInductionPhi(PHINode *Phi);

  void fixNonInductionPhis();

  /// Compute scalar induction steps. \p ScalarIV is the scalar induction
  /// variable on which to base the steps, \p Step is the size of the step, and
  /// \p EntryVal is the value from the original loop that maps to the steps.
  void buildScalarSteps(Value *ScalarIV, Value *Step, Value *EntryVal,
                        const InductionDescriptor &ID);

  /// Create a vector version of induction.
  void createVectorIntOrFpInductionPHI(const InductionDescriptor &II,
                                       Value *Step, Instruction *&VectorInd);

  /// Collect the instructions that are uniform after vectorization. An
  /// instruction is uniform if we represent it with a single scalar value in
  /// the vectorized loop corresponding to each vector iteration. Examples of
  /// uniform instructions include pointer operands of consecutive or
  /// interleaved memory accesses. Note that although uniformity implies an
  /// instruction will be scalar, the reverse is not true. In general, a
  /// scalarized instruction will be represented by VF scalar values in the
  /// vectorized loop, each corresponding to an iteration of the original
  /// scalar loop.
  void collectLoopUniforms(unsigned VF);

  /// Returns true if \p I is known to be uniform after vectorization.
  bool isUniformAfterVectorization(Instruction *I, unsigned VF) const;

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

  /// Holds the instructions known to be uniform after vectorization.
  /// The data is collected per VF.
  DenseMap<unsigned, SmallPtrSet<Instruction *, 4>> Uniforms;

  /// Holds the instructions known to be scalar after vectorization.
  /// The data is collected per VF.
  DenseMap<unsigned, SmallPtrSet<Instruction *, 4>> Scalars;

  // Map of widened value.
  std::map<Value *, Value *> WidenMap;
  // Map of scalar values.
  std::map<Value *, DenseMap<unsigned, Value *>> ScalarMap;

  // Map of widened private values. Unlike WidenMap, this is
  // pointer-to-pointer map.
  std::map<Value *, Value *> LoopPrivateWidenMap;

  // Keeps last non-zero mask
  std::map<Value *, Value *> LoopPrivateLastMask;

  // Holds the end values for each induction variable. We save the end values
  // so we can later fix-up the external users of the induction variables.
  DenseMap<PHINode *, Value *> IVEndValues;

  SmallVector<PHINode *, 8> OrigInductionPhisToFix;

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
  /// Current mask value for instructions being generated
  Value *MaskValue;
  /// A list of all bypass blocks. The first block is the entry of the loop.
  SmallVector<BasicBlock *, 4> LoopBypassBlocks;

  // Widen the given load instruction. EmitIntrinsic needs to be set to true
  // when we can start emitting masked_gather intrinsic once we have support
  // in code gen. Without code gen support, we will serialize the intrinsic.
  // As a result, we simply serialize the instruction for now.
  void vectorizeLoadInstruction(Instruction *Inst, bool EmitIntrinsic = false);

  // Widen the given store instruction. EmitIntrinsic needs to be set to true
  // when we can start emitting masked_scatter intrinsic once we have support
  // in code gen. Without code gen support, we will serialize the intrinsic.
  // As a result, we simply serialize the instruction for now.
  void vectorizeStoreInstruction(Instruction *Inst, bool EmitIntrinsic = false);

  // Widen a BitCast instruction
  void vectorizeBitCast(Instruction *Inst);

  // Widen call instruction parameters and return. Currently, this is limited
  // to svml function support that is hooked in to TLI. Later, this can be
  // extended to user-defined vector functions.
  void vectorizeCallInstruction(CallInst *Call);

  // Widen Select instruction.
  void vectorizeSelectInstruction(Instruction *Inst);

  // Widen the given PHI instruction. For now we assume this corresponds to
  // the Induction PHI.
  void vectorizePHIInstruction(Instruction *Inst);

  void vectorizeReductionPHI(PHINode *Inst);

  // Return a vector Vl wide: <Val, Val + Stride,
  // ... VAL + (VF - 1) * Stride>
  Value *getStrideVector(Value *Val, Value *Stride);

  // Map Edge between blocks to a mask value.
  std::map< std::pair<BasicBlock *, BasicBlock *>, Value *> EdgeToMaskMap;

  /// Store instructions that should be predicated, as a pair
  ///   <StoreInst, Predicate>
  SmallVector<std::pair<Instruction *, Value *>, 4> PredicatedInstructions;

};

} // End LLVM Namespace

#endif // LLVM_TRANSFORMS_VPO_VECOPT_VPOAVRLLVMCODEGEN_H

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
//   VPOAvrLLVMCodeGen.h -- LLVM IR Code generation from AVR
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_VECOPT_VPOAVRLLVMCODEGEN_H
#define LLVM_TRANSFORMS_VPO_VECOPT_VPOAVRLLVMCODEGEN_H

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrGenerate.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/MapVector.h"


namespace llvm { // LLVM Namespace
namespace vpo {  // VPO Vectorizer Namespace

// Class ReductionMngr generates code for loop reductions
class ReductionMngr {
public:
  ReductionMngr(AVR *Avr);

  // Save loop bbs for future constructions.
  void saveLoopEntryExit(BasicBlock *Preheader, BasicBlock *Exit);

  // Complete Phi nodes after vectorization.
  // Build the loop tail.
  void completeReductionPhis(std::map<Value *, Value *> &WidenMap);

  // Return true if the instruction is a Phi node marked as reduction
  bool isReductionPhi(const PHINode *Phi);

  bool isReductionVariable(const Value *Val);

  ReductionItem *getReductionInfo(const Value *Val);

  void saveInsertPointForReductionPhis(Instruction *Inst) {
    PhiInsertPt = Inst;
  }

  static Value *getRecurrenceIdentityVector(ReductionItem *RedItem, Type *Ty,
                                            unsigned VL);

  // Widening reduction Phi node
  Instruction *vectorizePhiNode(PHINode *RdxPhi, unsigned VL);

private:
  // Reduction map
  typedef DenseMap<const Value *, ReductionItem *> ReductionValuesMap;
  ReductionValuesMap ReductionMap;
  DenseMap<PHINode *, ReductionItem *> ReductionPhiMap;
  Instruction *PhiInsertPt;
  BasicBlock *LoopPreheader;
  BasicBlock *VectorBody;
  BasicBlock *LoopExit;
};

class AVRLoopVectorizationLegality {
public:
  AVRLoopVectorizationLegality(Loop *L, ScalarEvolution *SE,
    TargetLibraryInfo *TLI, Function *F, LoopInfo *LI)
    : TheLoop(L), PSE(*SE, *L), TLI(TLI), TTI(nullptr), LI(LI),
    Induction(nullptr), WidestIndTy(nullptr) {}

  /// ReductionList contains the reduction descriptors for all
  /// of the reductions that were found in the loop.
  typedef DenseMap<PHINode *, RecurrenceDescriptor> ReductionList;

  /// InductionList saves induction variables and maps them to the
  /// induction descriptor.
  typedef MapVector<PHINode *, InductionDescriptor> InductionList;

  /// Returns the Induction variable.
  PHINode *getInduction() { return Induction; }

  /// Returns True if V is an induction variable in this loop.
  bool isInductionVariable(const Value *V);

  /// Returns true if the value \p V is loop invariant.
  bool isLoopInvariant(Value *V);

  /// Returns true if the access through \p Ptr is consecutive.
  bool isConsecutivePtr(Value *Ptr);

  /// Returns True if PN is a reduction variable in this loop.
  bool isReductionVariable(PHINode *PN) { return Reductions.count(PN); }

  bool canVectorizeLoop(AVRLoopIR *ALoop, ReductionMngr *RM);

  Loop *getLoop() { return TheLoop; }

  /// Adds \p Phi node to the list of induction variables.
  void addInductionPhi(PHINode *Phi, const InductionDescriptor &ID,
                       SmallPtrSetImpl<Value *> &AllowedExit);
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
  /// Dominator Tree.
  DominatorTree *DT;
  /// A set of Phi nodes that may be used outside the loop.
  SmallPtrSet<Value *, 4> AllowedExit;
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

};

// AVRCodeGen generates vector code by widening of scalars into
// appropriate length vectors.
// TBI - In stress mode generate scalar code by cloning
// instructions.
class AVRCodeGen {
public:
  AVRCodeGen(AVR *Avr, DominatorTree *DT, ScalarEvolution *SE, LoopInfo *LI,
             TargetLibraryInfo *TLI, Function *F)
      : Avr(Avr), F(F), SE(SE), DT(DT), LI(LI), TLI(TLI), ALoop(nullptr),
        OrigLoop(nullptr), Legal (nullptr), TripCount(0), VL(0),
        Builder(F->getContext()), LoopBackEdge(nullptr),
        InductionPhi(nullptr), InductionCmp(nullptr), StartValue(nullptr),
        StrideValue(nullptr), NewInductionVal(nullptr), RM(Avr) {}

  ~AVRCodeGen() {
    WidenMap.clear();
    delete Legal;
  }

  // Perform the actual loop widening (vectorization) using VF as the
  // vectorization factor.
  bool vectorize(unsigned int VF);

  // Check if loop is currently supported by AVRCodeGen. If \p VF is 0 ignore 
  // it (which means that checks such as whether the trip count is evenly
  // divisible by VF will not be done).
  bool loopIsHandled(unsigned int VF);

  // Set AVR Loop node.
  void setALoop(AVRLoop *L) { ALoop = L; }

  // Return the trip count for the scalar loop. Returns 0 for unknown trip
  // count loops
  uint64_t getTripCount() const {return TripCount;}

  /// Reverse vector elements
  Value *reverseVector(Value *Vec);

private:
  AVR *Avr;
  Function *F;
  ScalarEvolution *SE;
  DominatorTree *DT;
  LoopInfo *LI;
  TargetLibraryInfo *TLI;

  // AVRLoop in AVR region
  AVRLoop *ALoop;

  // Original LLVM loop corresponding to this Avr region
  Loop *OrigLoop;

  AVRLoopVectorizationLegality *Legal;

  // Loop trip count
  unsigned int TripCount;

  // Vector factor or vector length to use. Each scalar instruction is widened
  // to operate on this number of operands.
  unsigned VL;

  // IR Builder to use to generate instructions
  IRBuilder<> Builder;

  // Loop backedge
  AVRBranchIR *LoopBackEdge;

  // Induction variable PHI
  AVRPhiIR *InductionPhi;

  // Induction variable cmp
  AVRCompare *InductionCmp;

  // Starting value of loop induction variable
  Value *StartValue;

  // Loop increment
  ConstantInt *StrideValue;

  // New induction var
  Value *NewInductionVal;

  // Map of scalar, widened value
  std::map<Value *, Value *> WidenMap;

  // Reductions handling
  ReductionMngr RM;

  void setOrigLoop(Loop *L) { OrigLoop = L; }
  void setTripCount(unsigned int TC) { TripCount = TC; }
  void setVL(unsigned V) { VL = V; }
  void setLoopBackEdge(AVRBranchIR *FB) { LoopBackEdge = FB; }
  void setInductionPhi(AVRPhiIR *P) { InductionPhi = P; }
  void setInductionCmp(AVRCompare *C) { InductionCmp = C; }
  void setStartValue(Value *SV) { StartValue = SV; }
  void setStrideValue(ConstantInt *SV) { StrideValue = SV; }
  void setNewInductionVal(Value *V) { NewInductionVal = V; }

  // Check for currently handled loops. Initial implementations
  // punts on seeing any control flow. 
  // The output parameter \p TripCount holds the tripCount of the loop if it is
  // a constant, zero otherwise.
  bool loopIsHandledImpl(unsigned int &TripCount);

  // Create the vector loop skeleton which iterates from StartIndex
  // to StartIndex +  VL * Stride * TripCount. We also setup the control
  // flow such that the scalar loop is skipped.
  void createEmptyLoop();

  // Widen the given instruction to VL wide vector instruction
  void vectorizeInstruction(Instruction *Inst);

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
  void completeReductions();

  // Vectorize the given instruction that cannot be widened using serialization.
  // This is done using a sequence of extractelement, Scalar Op, InsertElement
  // instructions.
  void serializeInstruction(Instruction *Inst);

  // Get the widened vector value for given value V. If the scalar value
  // has not been widened, we widen it by VL and store it in WidenMap
  // before returning the widened value
  Value *getVectorValue(Value *V);

  Value *getScalarValue(Value *V);

  // Get a vector value by broadcasting given value to a vector that is VL
  // wide.
  Value *getBroadcastInstrs(Value *V);

  // Return a vector Vl wide: <Val, Val + Stride,
  // ... VAL + (VL - 1) * Stride>
  Value *getStrideVector(Value *Val, Value *Stride);
};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_TRANSFORMS_VPO_VECOPT_VPOAVRLLVMCODEGEN_H

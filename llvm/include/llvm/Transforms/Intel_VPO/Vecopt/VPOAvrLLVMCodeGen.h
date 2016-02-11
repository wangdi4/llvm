//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
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

#include <map>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrGenerate.h"

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
  static Constant *getRecurrenceIdentity(ReductionItem::WRNReductionKind RKind,
                                         Type *Ty);
  // Widening reduction Phi node
  Instruction *vectorizePhiNode(PHINode *RdxPhi, unsigned VL);

private:
  // Reduction map
  typedef std::map<const Value *, ReductionItem *> ReductionValuesMap;
  ReductionValuesMap ReductionMap;
  std::map<PHINode *, ReductionItem *> ReductionPhiMap;
  Instruction *PhiInsertPt;
  BasicBlock *LoopPreheader;
  BasicBlock *VectorBody;
  BasicBlock *LoopExit;
};

// AVRCodeGen generates vector code by widening of scalars into
// appropriate length vectors.
// TBI - In stress mode generate scalar code by cloning
// instructions.
class AVRCodeGen {
public:
  AVRCodeGen(AVR *Avr, ScalarEvolution *SE, LoopInfo *LI, Function *F)
      : Avr(Avr), F(F), SE(SE), LI(LI), OrigLoop(nullptr), TripCount(0), VL(0),
        Builder(F->getContext()), LoopBackEdge(nullptr), InductionPhi(nullptr),
        InductionCmp(nullptr), StartValue(nullptr), StrideValue(nullptr),
        NewInductionVal(nullptr), RM(Avr) {}

  ~AVRCodeGen() { WidenMap.clear(); }

  // Perform the actual loop widening (vectorization).
  bool vectorize();

private:
  AVR *Avr;
  Function *F;
  ScalarEvolution *SE;
  LoopInfo *LI;

  // AVRLoop in AVR region
  AVRLoop *ALoop;

  // Original LLVM loop corresponding to this Avr region
  Loop *OrigLoop;

  // Loop trip count
  unsigned int TripCount;

  // Vector factor or vector length to use. Each scalar instruction is widened
  // to operate on this number of operands.
  int VL;

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

  void setALoop(AVRLoop *L) { ALoop = L; }
  void setOrigLoop(Loop *L) { OrigLoop = L; }
  void setTripCount(unsigned int TC) { TripCount = TC; }
  void setVL(int V) { VL = V; }
  void setLoopBackEdge(AVRBranchIR *FB) { LoopBackEdge = FB; }
  void setInductionPhi(AVRPhiIR *P) { InductionPhi = P; }
  void setInductionCmp(AVRCompare *C) { InductionCmp = C; }
  void setStartValue(Value *SV) { StartValue = SV; }
  void setStrideValue(ConstantInt *SV) { StrideValue = SV; }
  void setNewInductionVal(Value *V) { NewInductionVal = V; }

  // Check for currently handled loops. Initial implementations
  // punts on seeing any control flow.
  bool loopIsHandled();

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

  // Widen the given store instruction. EmitIntrinsic needs to be set to true
  // when we can start emitting masked_scatter intrinsic once we have support
  // in code gen. Without code gen support, we will serialize the intrinsic.
  // As a result, we simply serialize the instruction for now.
  void vectorizeStoreInstruction(Instruction *Inst, bool EmitIntrinsic = false);

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

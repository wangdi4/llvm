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
//   IntelVectorizeIndirectCall.h -- LLVM IR Code generation
//
//===----------------------------------------------------------------------===//
#include "IntelVPOCodeGen.h"
#include "IntelVPlan.h"
#include "llvm/Analysis/VectorUtils.h"

#ifndef INTEL_VPLAN_VECTORIZE_INDIRECT_CALL_H
#define INTEL_VPLAN_VECTORIZE_INDIRECT_CALL_H

namespace llvm {
namespace vpo {
class IndirectCallCodeGenerator {
public:
  IndirectCallCodeGenerator(VPOCodeGen *CodeGen, LoopInfo *LI, unsigned VF,
                            VPTransformState *State, Value *MaskValue,
                            const VPlanVector *Plan)
      : CodeGen(CodeGen), LI(LI), VF(VF), State(State), MaskValue(MaskValue),
        Plan(Plan), OrigCall(nullptr){};

  /// Returns true if the indirect call was vectorized
  bool vectorize(VPCallInstruction *VPCallInst);

private:
  VPOCodeGen *CodeGen = nullptr;
  LoopInfo *LI = nullptr;
  unsigned VF = 0;
  VPTransformState *State = nullptr;
  Value *MaskValue = nullptr;
  const VPlanVector *Plan = nullptr;

  // Keeps the call arguments.
  SmallVector<Value *, 4> CallArgs;
  // Keeps the type of call arguments.
  SmallVector<Type *, 4> CallArgsTy;
  SmallVector<AttributeSet, 2> VecArgAttrs;
  // Call to scalar function pointer.
  const CallInst *OrigCall;
  // Original vector of function pointers.
  Value *OriginalVectorOfFuncPtrs = nullptr;
  // Split the basic block of the indirect call in two: the current.basic.block
  // and the indirect.call.loop.exit.
  BasicBlock *CurrentBB = nullptr;
  // The BB that follows CurrentBB.
  BasicBlock *NextBB = nullptr;
  // This is the first basic block (indirect.call.loop.entry) of the loop.
  BasicBlock *IndirectCallLoopEntryBB = nullptr;
  // This is the second basic block (vector.indirect.call) of the loop.
  BasicBlock *VectorIndirectCallBB = nullptr;
  // This is the third basic block (indirect.call.loop.latch) of the loop.
  BasicBlock *IndirectCallLoopLatchBB = nullptr;
  // After the indirect call, code gen continues to emit the code in
  // indirect.call.loop.exit.
  BasicBlock *IndirectCallLoopExitBB = nullptr;
  // Vector of function pointers which is updated in the loop.
  PHINode *VectorOfFuncPtrs = nullptr;
  // The return value of the indirect call.
  PHINode *CurIndirectCallReturn = nullptr;
  // The final vector of the return value of the indirect call.
  PHINode *FinalIndirectCallReturn = nullptr;
  // The index of the vector of function pointers.
  PHINode *Index = nullptr;
  // The function pointer of the vector of function pointers that we
  // currently process.
  Value *CurrentFPtr = nullptr;
  // The return value of the indirect call is updated in each iteration.
  Value *IndirectCallReturnUpdated = nullptr;
  // The vector of function pointers is updated in each iteration.
  Value *VectorOfFuncPtrsUpdated = nullptr;
  // The following functions fill the newly emitted basic blocks with the
  // right instructions.
  void fillIndirectCallLoopEntryBB(VPCallInstruction *VPCallInst);
  void fillVectorIndirectCallBB(VPCallInstruction *VPCallInst);
  void fillIndirectCallLoopLatchBB(VPCallInstruction *VPCallInst);
  void fillIndirectCallLoopExitBB(VPCallInstruction *VPCallInst);
  Value *generateIndirectCall(VPCallInstruction *VPCallInst,
                              Value *CurrentFPtr);

  /// For uniform function pointers, we just have to call the vectorized
  /// version of the function pointer along with the vectorized arguments
  /// and the mask (if it is needed).
  void generateCodeForUniformIndirectCall(VPCallInstruction *VPCallInst);

  /// For non uniform function pointers, we need to geenrate a loop which
  /// will fill the vector of function pointers at runtime.
  void generateCodeForNonUniformIndirectCall(VPCallInstruction *VPCallInst);
};
} // namespace vpo
} // namespace llvm

#endif

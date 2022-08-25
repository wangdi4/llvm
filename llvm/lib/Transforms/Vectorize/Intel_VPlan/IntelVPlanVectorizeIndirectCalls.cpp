//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2019-2022 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   IntelVectorizeIndirectCall.cpp -- LLVM IR Code generation from VPlan
//
//===----------------------------------------------------------------------===//
//
// This file shows how to vectorize indirect calls that are marked with
// __intel_indirect_call. The first argument of these calls point to a table
// with all the possible vectorization scenarios for the current pointer. The
// best variant is selected by using matchVectorVariant().
//
// To vectorize an indirect call, we need to emit as many calls as the unique
// function pointers in the vector. The functions where the function pointers
// are pointing are vectorized and they have their arguments vectorized as well.
// The function pointers point to the masked vectorized versions of the original
// functions. The mask is needed to turn-off the lanes that do not correspond to
// the current function pointer. At compile-time, we do not know the contents of
// the vector of function pointers. Therefore, we need to generate the mask and
// the right number of function calls using a loop. Below is the code that we
// generate in order to support indirect call vectorization:
//
// Let's assume that we want to vectorize the following indirect call:
//      %initial_indirect_call_return = call %fptr (%arg1)
// Initially, we split the basic block of the indirect call in two: the
// current.basic.block and the indirect.call.loop.exit. The indirect call is
// replaced with the loop. The loop consists of three basic blocks
// (indirect.call.loop.entry, vector.indirect.call, indirect.call.loop.latch).
// current.basic.block:
// ...
//      // The following instruction is emitted only when there is a mask.
//      %original_vector_of_func_ptrs = select %mask_value, %fptr, 0
//      br %indirect.call.loop.entry
//
// indirect.call.loop.entry:
//      # Iterate over the vector of function pointers
//      %vector_of_func_ptrs = phi (%original_vector_of_func_ptrs,
//                                 %current_vector_of_func_ptrs)
//      %indirect_call_return = phi (%initial_indirect_call_return,
//                                  %indirect_call_return_updated)
//      %indx = phi(0, %indx_updated)
//      # Get a function pointer from the vector
//      %current_fptr = extract_elem ent(%vector_of_func_ptrs, %indx)
//      # Every time a function pointer is processed, it is replaced with a zero
//      # in the vector of function pointers. Here, we check the current
//      # function pointer is zero. If so, we skip it and we jump to
//      indirect.call.loop.latch. %is_visited = icmpeq %current_fptr, 0 br
//      %is_visited, %indirect.call.loop.latch, %vector.indirect.call
//
// vector.indirect.call:
//      %broadcast = broadcast(%current_fptr)
//      %func_ptr_mask = icmpeq %broadcast, %vector_of_func_ptrs
//      # The following instruction is emitted only when the %mask_value is
//      # defined i.e. the call is under condition
//      %final_mask = func_ptr_mask &
//      %mask_value %vec_mask = sext %func_ptr_mask, INT_PTR_TY # Call the
//      vectorized version of the function pointer along with a # correct mask.
//      %tmp_indirect_call_return = call *current_fptr, %arg1,
//      %vec_mask
//      # Add the return value of each function pointer in a vector.
//      %indirect_call_return_updated = select %final_mask,
//                                             %tmp_indirect_call_return,
//                                             %indirect_call_return
//      # Replace the instances of the vector of the current function pointer
//      with zero.
//      %vector_of_func_ptrs_updated = select %final_mask, 0,
//                                            %vector_of_func_ptrs
//      br %indirect.call.loop.latch
//
// indirect.call.loop.latch:
//      %final_indirect_call_return = phi (%indirect_call_return_updated,
//                                        %indirect_call_return)
//      %current_vector_of_func_ptrs = phi (%vector_of_func_ptrs_updated,
//                                         %vector_of_func_ptrs)
//      %indx_updated = add %indx, 1
//      %exit_cond = icmp lt %indx_updated, VF
//      br %exit_cond, %indirect.call.loop.entry, %indirect.call.loop.exit
// indirect.call.loop.exit:
//      phi (%final_indirect_call_return)
//
#include "IntelVPlanVectorizeIndirectCalls.h"
#include "IntelVPlanCallVecDecisions.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "IndirectCallCodeGenerator"

using namespace llvm;
using namespace llvm::vpo;

bool IndirectCallCodeGenerator::vectorize(VPCallInstruction *VPCallInst) {
  assert(VPCallInst->isIntelIndirectCall() &&
         "Intel indirect call is expected.");
  OrigCall = VPCallInst->getOriginalCall();

  assert(OrigCall && "OriginalCall is expected.");

  // Check if there is a matched vector vaariant.
  const VFInfo *MatchedVecVariant = VPCallInst->getVectorVariant();
  assert(MatchedVecVariant &&
         "There is not any attribute for the current indirect call. The "
         "indirect call will be serialized.");

  // Get the vector version of the arguments if they are available.
  CodeGen->vectorizeCallArgs(
      VPCallInst, MatchedVecVariant, Intrinsic::not_intrinsic, 0,
      VPCallInst->getPumpFactor(), CallArgs, CallArgsTy, VecArgAttrs);

  // Check whether the function pointer is uniform.
  if (!Plan->getVPlanDA()->isDivergent(*VPCallInst->getOperand(0)))
    generateCodeForUniformIndirectCall(VPCallInst);
  else
    generateCodeForNonUniformIndirectCall(VPCallInst);
  return true;
}

// Create the call to the vector version of the function pointer.
// MatchedVecVariantIdx indicates which function pointer of the table with the
// function pointers we should select.
Value *
IndirectCallCodeGenerator::generateIndirectCall(VPCallInstruction *VPCallInst,
                                                Value *CurrentFuncPtr) {
  unsigned MatchedVecVariantIdx = VPCallInst->getVectorVariantIndex();
  auto EC = ElementCount::getFixed(VF);
  Type *CallTy = VPCallInst->getType();
  Type *VecRetTy = CallTy->isVoidTy() ? CallTy :  VectorType::get(CallTy, EC);
  FunctionType *VecFuncTy = FunctionType::get(VecRetTy, CallArgsTy, false);
  unsigned AS = OrigCall->getArgOperand(0)->getType()->getPointerAddressSpace();
  Type *VecFuncPtrTy = VecFuncTy->getPointerTo(AS);
  Value *BitCast = State->Builder.CreateBitCast(
      CurrentFuncPtr, VecFuncPtrTy->getPointerTo(AS));

  Value *VariantGep = State->Builder.CreateConstGEP1_32(VecFuncPtrTy, BitCast,
                                                        MatchedVecVariantIdx);
  LoadInst *LoadVariant =
      State->Builder.CreateLoad(VecFuncTy->getPointerTo(AS), VariantGep);

  FunctionCallee VariantCallee(VecFuncTy, LoadVariant);
  CallInst *VariantCall = State->Builder.CreateCall(VariantCallee, CallArgs);
  return VariantCall;
}

// For uniform function pointers, we just have to call the vectorized version of
// the function pointer.
void IndirectCallCodeGenerator::generateCodeForUniformIndirectCall(
    VPCallInstruction *VPCallInst) {
  Value *IndirectCallReturn = generateIndirectCall(
      VPCallInst, CodeGen->getScalarValue(VPCallInst->getArgOperand(0), 0));
  CodeGen->addToWidenMap(VPCallInst, IndirectCallReturn);
}

// Emit the loop that will fill-in the vector of the function pointers at
// runtime.
void IndirectCallCodeGenerator::generateCodeForNonUniformIndirectCall(
    VPCallInstruction *VPCallInst) {
  bool IsMasked = MaskValue != nullptr;
  auto EC = ElementCount::getFixed(VF);
  Constant *NullPtrVec = ConstantVector::getSplat(
      EC, Constant::getNullValue(OrigCall->getArgOperand(0)->getType()));
  CurrentBB = State->Builder.GetInsertBlock();
  Function *CurFunc = CurrentBB->getParent();
  // Call to the vectorized function pointer.
  Value *VectorPtr = CodeGen->getVectorValue(VPCallInst->getArgOperand(0));
  OriginalVectorOfFuncPtrs = VectorPtr;
  if (IsMasked)
    OriginalVectorOfFuncPtrs = State->Builder.CreateSelect(
        MaskValue, VectorPtr, NullPtrVec, "original_vector_of_func_ptr");

  // Create the new basic blocks of the loop as it is shown in the example
  // above.
  NextBB = CurrentBB->getNextNode();
  IndirectCallLoopEntryBB = BasicBlock::Create(
      *Plan->getLLVMContext(), "indirect.call.loop.entry", CurFunc, NextBB);
  VectorIndirectCallBB = BasicBlock::Create(
      *Plan->getLLVMContext(), "vector.indirect.call", CurFunc, NextBB);
  IndirectCallLoopLatchBB = BasicBlock::Create(
      *Plan->getLLVMContext(), "indirect.call.loop.latch", CurFunc, NextBB);
  IndirectCallLoopExitBB = BasicBlock::Create(
      *Plan->getLLVMContext(), "indirect.call.loop.exit", CurFunc, NextBB);

  // At this point, the basic blocks do not have terminators. Instead, there is
  // an unreachable instruction at the end of the basic block. Here, the
  // unreachable is removed and a branch is emitted at the bottom of the
  // CurrentBB.
  State->Builder.CreateBr(IndirectCallLoopEntryBB);
  Instruction *Term = CurrentBB->getTerminator();
  assert(Term->getOpcode() == Instruction::Unreachable &&
         "Basic block's terminator should be the Unreachable instruction!");
  Term->removeFromParent();

  // Fill-in the new basic-blocks.
  fillIndirectCallLoopEntryBB(VPCallInst);
  fillVectorIndirectCallBB(VPCallInst);
  fillIndirectCallLoopLatchBB(VPCallInst);
  fillIndirectCallLoopExitBB(VPCallInst);
}

// Fill-in entry.indirect.call.loop basic block. This basic block iterates
// over the values of the vector of function pointers. Every time, a
// function pointer is visited, the instances of the function pointers are
// replaced with zero.
void IndirectCallCodeGenerator::fillIndirectCallLoopEntryBB(
    VPCallInstruction *VPCallInst) {
  auto EC = ElementCount::getFixed(VF);

  State->Builder.SetInsertPoint(IndirectCallLoopEntryBB);
  VectorOfFuncPtrs = State->Builder.CreatePHI(
      VectorType::get(OrigCall->getArgOperand(0)->getType(), EC), 2,
      "vector_of_func_ptrs");
  VectorOfFuncPtrs->addIncoming(OriginalVectorOfFuncPtrs, CurrentBB);

  if (!VPCallInst->getType()->isVoidTy()) {
    Constant *NullPtrVec = ConstantVector::getSplat(
        EC, Constant::getNullValue(VPCallInst->getType()));
    CurIndirectCallReturn = State->Builder.CreatePHI(
        NullPtrVec->getType(), 2, "cur_indirect_call_return");
    CurIndirectCallReturn->addIncoming(NullPtrVec, CurrentBB);
  }

  Index = State->Builder.CreatePHI(Type::getInt64Ty(*Plan->getLLVMContext()), 2,
                                   "indx");
  Constant *Zero =
      ConstantInt::get(Type::getInt64Ty(*Plan->getLLVMContext()), 0);
  Index->addIncoming(Zero, CurrentBB);
  // Get a function pointer from the vector of the function pointers.
  CurrentFPtr = State->Builder.CreateExtractElement(VectorOfFuncPtrs, Index,
                                                    "current_fptr");
  Value *IsVisited = State->Builder.CreateICmpEQ(
      CurrentFPtr, Constant::getNullValue(CurrentFPtr->getType()),
      "is_visited");
  State->Builder.CreateCondBr(IsVisited, IndirectCallLoopLatchBB,
                              VectorIndirectCallBB);
}

// Fill-in vector.indirect.call basic block where the call to the vectorized
// function pointer is emitted.
void IndirectCallCodeGenerator::fillVectorIndirectCallBB(
    VPCallInstruction *VPCallInst) {
  auto EC = ElementCount::getFixed(VF);
  Constant *NullPtrVec = ConstantVector::getSplat(
      EC, Constant::getNullValue(OrigCall->getArgOperand(0)->getType()));
  bool IsMasked = MaskValue != nullptr;
  State->Builder.SetInsertPoint(VectorIndirectCallBB);

  Value *Splat =
      State->Builder.CreateVectorSplat(VF, CurrentFPtr, "current.fptr");
  // Calculate the mask that activates the lanes that have the
  // instances of the current function pointer.
  Value *FuncPtrMask =
      State->Builder.CreateICmpEQ(Splat, VectorOfFuncPtrs, "func_ptr_mask");
  // If the loop has a mask (MaskValue), then the function pointer
  // mask (FuncPtrMask) should be blended with the look mask (MaskValue).
  Value *FinalMask = FuncPtrMask;
  if (IsMasked)
    FinalMask = State->Builder.CreateAnd(FuncPtrMask, MaskValue, "final_mask");

  const VFInfo *MatchedVariant = VPCallInst->getVectorVariant();
  assert(MatchedVariant && "Unexpected null matched vector variant");

  // If matched vector variant has a mask, then this mask will have already been
  // added in indirect call's arguments by vectorizeCallArgs. Thus, we have to
  // pop it out from the arguments list.
  if (MatchedVariant->isMasked()) {
    CallArgs.pop_back();
    CallArgsTy.pop_back();
  }

  CodeGen->createVectorMaskArg(VPCallInst, MatchedVariant, CallArgs, CallArgsTy,
                               VF, FinalMask);

  // Generate the call to the vectorized version of the function pointer.
  Value *IndirectCallReturn = generateIndirectCall(VPCallInst, CurrentFPtr);

  // Blend the return value of the current function pointer in a vector.
  if (!VPCallInst->getType()->isVoidTy())
    IndirectCallReturnUpdated = State->Builder.CreateSelect(
        FinalMask, IndirectCallReturn, CurIndirectCallReturn,
        "indirect_call_return_updated");

  // Add zeros in the vector lanes that we have the same function pointer as
  // the one that we currently process.
  VectorOfFuncPtrsUpdated = State->Builder.CreateSelect(
      FinalMask, NullPtrVec, VectorOfFuncPtrs, "vector_of_func_ptrs_updated");
  State->Builder.CreateBr(IndirectCallLoopLatchBB);
}

// Fill-in indirect.call.loop.latch.
void IndirectCallCodeGenerator::fillIndirectCallLoopLatchBB(
    VPCallInstruction *VPCallInst) {
  auto EC = ElementCount::getFixed(VF);
  Constant *NullptrVec = ConstantVector::getSplat(
      EC, Constant::getNullValue(OrigCall->getArgOperand(0)->getType()));

  State->Builder.SetInsertPoint(IndirectCallLoopLatchBB);
  // FinalIndirectCallReturn has the complete blended vector with the return
  // value of the indirect call.
  if (!VPCallInst->getType()->isVoidTy()) {
    Constant *NullValueVec = ConstantVector::getSplat(
        EC, Constant::getNullValue(VPCallInst->getType()));
    FinalIndirectCallReturn = State->Builder.CreatePHI(
        NullValueVec->getType(), 2, "final_indirect_call_return");
    FinalIndirectCallReturn->addIncoming(IndirectCallReturnUpdated,
                                         VectorIndirectCallBB);
    FinalIndirectCallReturn->addIncoming(CurIndirectCallReturn,
                                         IndirectCallLoopEntryBB);
    CurIndirectCallReturn->addIncoming(FinalIndirectCallReturn,
                                       IndirectCallLoopLatchBB);
  }

  PHINode *CurVectorOfFuncPtrs = State->Builder.CreatePHI(
      NullptrVec->getType(), 2, "current_vector_of_func_ptrs");
  CurVectorOfFuncPtrs->addIncoming(VectorOfFuncPtrsUpdated,
                                   VectorIndirectCallBB);
  CurVectorOfFuncPtrs->addIncoming(VectorOfFuncPtrs, IndirectCallLoopEntryBB);
  VectorOfFuncPtrs->addIncoming(CurVectorOfFuncPtrs, IndirectCallLoopLatchBB);
  Value *IndexUpdated = State->Builder.CreateAdd(
      Index, ConstantInt::get(Type::getInt64Ty(*Plan->getLLVMContext()), 1),
      "indx_updated");
  Index->addIncoming(IndexUpdated, IndirectCallLoopLatchBB);
  Value *ExitCond = State->Builder.CreateICmpEQ(
      IndexUpdated,
      ConstantInt::get(Type::getInt64Ty(*Plan->getLLVMContext()), VF),
      "exitcond");
  State->Builder.CreateCondBr(ExitCond, IndirectCallLoopExitBB,
                              IndirectCallLoopEntryBB);
}

// VPlan's code gen will continue to generate the rest of the code
// from this point.
void IndirectCallCodeGenerator::fillIndirectCallLoopExitBB(
    VPCallInstruction *VPCallInst) {
  State->Builder.SetInsertPoint(IndirectCallLoopExitBB);
  // To keep the LCSSA form of the loop, we emit a phi node for the return value
  // of the indirect call.
  if (!VPCallInst->getType()->isVoidTy()) {
    auto EC = ElementCount::getFixed(VF);
    PHINode *IndirectCallReturnLCSSAPhi =
        State->Builder.CreatePHI(VectorType::get(VPCallInst->getType(), EC), 1,
                                 "indirect_call_return_lcssa_phi");
    IndirectCallReturnLCSSAPhi->addIncoming(FinalIndirectCallReturn,
                                            IndirectCallLoopLatchBB);
    CodeGen->addToWidenMap(VPCallInst, IndirectCallReturnLCSSAPhi);
  }

  UnreachableInst *Terminator = State->Builder.CreateUnreachable();
  State->Builder.SetInsertPoint(Terminator);
  State->CFG.PrevBB = IndirectCallLoopExitBB;
}

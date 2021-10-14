#if INTEL_COLLAB
//===------ VPOParoptAtomics.cpp - Transformation of Atomic WRegions ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Author(s):
// --------
// Abhinav Gaba (abhinav.gaba@intel.com)
//
// Major Revisions:
// ----------------
// Apr 2016: Initial Implementation for OMP Atomic Read, Write. (Abhinav Gaba)
// May 2016: Added support for OMP Atomic Update. (Abhinav Gaba)
// Jun 2016: Added support for OMP Atomic Capture. (Abhinav Gaba)
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains code for handling the OpenMP Atomic construct, including
/// selection as well as emitting of intrinsic calls to the KMPC runtime
/// library, based on the atomic operation and the data type(s) of the
/// operand(s).
///
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/VPO/Paropt/VPOParoptAtomics.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;
using namespace llvm::vpo;
using namespace llvm::vpo::intrinsics;

#define DEBUG_TYPE "vpo-paropt-atomics"

cl::opt<bool> Enable64BitOpenCLAtomics(
  "vpo-paropt-enable-64bit-opencl-atomics", cl::Hidden, cl::init(false),
  cl::desc("Enables usage of 64-bit atomic OpenCL RTL API."));

// Main driver for handling a WRNAtomicNode.
bool VPOParoptAtomics::handleAtomic(WRNAtomicNode *AtomicNode,
                                    StructType *IdentTy,
                                    Constant *TidPtr,
                                    DominatorTree *DT,
                                    LoopInfo *LI,
                                    bool IsTargetSPIRV) {
  bool handled;
  assert(AtomicNode != nullptr && "AtomicNode is null.");

  AtomicNode->populateBBSet();

  if (AtomicNode->getBBSetSize() < 3) {
    // It's possible that the middle BBlock is empty, in which case,
    // we don't need to do anything.
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": AtomicNode has less than 3 BBlocks. Skipping...\n");
    handled = true;
  } else {
    switch (AtomicNode->getAtomicKind()) {
    case WRNAtomicRead:
      handled = handleAtomicRW<WRNAtomicRead>(AtomicNode, IdentTy, TidPtr,
                                              IsTargetSPIRV);
      break;
    case WRNAtomicWrite:
      handled = handleAtomicRW<WRNAtomicWrite>(AtomicNode, IdentTy, TidPtr,
                                               IsTargetSPIRV);
      break;
    case WRNAtomicUpdate:
      handled = handleAtomicUpdate(AtomicNode, IdentTy, TidPtr, IsTargetSPIRV);
      break;
    case WRNAtomicCapture:
      handled = handleAtomicCapture(AtomicNode, IdentTy, TidPtr, IsTargetSPIRV);
      break;
    }
  }

  if (!handled)
    handled =
        VPOParoptUtils::genKmpcCriticalSection(AtomicNode, IdentTy, TidPtr,
                                               DT, LI, IsTargetSPIRV);
  assert(handled == true && "Handling of AtomicNode failed.\n");

  AtomicNode->resetBBSet(); // Invalidate BBSet if transformed
  LLVM_DEBUG(dbgs() << __FUNCTION__
                    << ": Handling of AtomicNode successful.\n");

  return handled;
}

// Functions for handling different WRNAtomicNodes.

// Handles generation of KMPC intrinsics for Atomic Read and Write.
template <WRNAtomicKind AtomicKind>
bool VPOParoptAtomics::handleAtomicRW(WRNAtomicNode *AtomicNode,
                                      StructType *IdentTy, Constant *TidPtr,
                                      bool IsTargetSPIRV) {
  assert((AtomicKind == WRNAtomicRead || AtomicKind == WRNAtomicWrite) &&
         "Unsupported AtomicKind for handleAtomicReadAndWrite.");
  assert(AtomicNode != nullptr && "AtomicNode is null.");
  assert(IdentTy != nullptr && "IdentTy is null.");
  assert(TidPtr != nullptr && "TidPtr is null.");

  if (AtomicNode->getBBSetSize() != 3) {
    LLVM_DEBUG(
        dbgs() << __FUNCTION__
               << ": AtomicNode for Read/Write does not have 3 BBlocks.\n");
    return false;
  }

  bool AtomicRead = (AtomicKind == WRNAtomicRead);

  // The first and last BasicBlocks contain directive intrinsic calls.
  // We're interested in only the middle one here.
  BasicBlock *BB = *(AtomicNode->bbset_begin() + 1);
  Instruction *Inst = nullptr;

  if (AtomicRead)
    Inst = getLoneInstructionOfTypeInBB<LoadInst>(*BB);
  else
    Inst = getLoneInstructionOfTypeInBB<StoreInst>(*BB);

  if (!Inst) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": No lone Load/Store in the BB "
                      << "for AtomicRead/Write. Returning...\n");
    return false; // Handle using critical section.
  }

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Source Instruction: " << *Inst
                    << "\n");

  // Now that we have the load/store instruction, we need to extract the
  // operands from it. Load/Store instructions are of form:
  //     store <type> val, <type*> ptr
  //     <ret> = load <type*> ptr
  // And the KMPC atomic read/write calls are of form:
  //     __kmpc_atomic_<dtype>_rd(loc, tid, ptr)
  //     __kmpc_atomic_<dtype>_wr(loc, tid, ptr, val)
  // So, only the ptr operand is used for both kmpc_atomic_rd, and both ptr and
  // val are needed for kmpc_atomic_wr.
  Value *Ptr = AtomicRead ? Inst->getOperand(0) : Inst->getOperand(1);
  Type *OpndTy = AtomicRead ? Inst->getType() :
      cast<StoreInst>(Inst)->getValueOperand()->getType();
  assert(OpndTy != nullptr && "Operand Type is null.");
  if (IsTargetSPIRV)
    Ptr =
        VPOParoptUtils::genAddrSpaceCast(Ptr, Inst, vpo::ADDRESS_SPACE_GENERIC);
  SmallVector<Value*, 2> FnArgs;
  AtomicRead ? FnArgs.assign({Ptr})                       // {ptr}
             : FnArgs.assign({Ptr, Inst->getOperand(0)}); // {ptr, val}

  // We have the val and ptr operands now. So we can use them to get the
  // operand type, and find the KMPC intrinsic name, if it exists for the type.
  assert(isa<PointerType>(Ptr->getType()) && "Unexpected type for operand.");
  const std::string Name =
      getAtomicRWSIntrinsicName<AtomicKind>(*(Inst->getParent()), *OpndTy);
  if (Name.empty())
    return false; // No intrinsic found. Handle using critical sections.

  // The return type is the Type of the operand for load, and void for store.
  Type *ReturnTy =
      AtomicRead ? OpndTy : Type::getVoidTy(BB->getParent()->getContext());

  // Now try to generate the call for kmpc_atomic_rd/kmpc_atomic_wr of type:
  //     __kmpc_atomic_<type>_rd(loc, tid, ptr)
  //     __kmpc_atomic_<type>_wr(loc, tid, ptr, val)
  CallInst *AtomicCall = genAtomicCall(AtomicNode, IdentTy, TidPtr, Inst, Name,
                                       ReturnTy, FnArgs, IsTargetSPIRV);
  assert(AtomicCall != nullptr && "Generated Atomic R/W call is null.");

  ReplaceInstWithInst(Inst, AtomicCall);
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic call inserted.\n");
  return true;
}

bool VPOParoptAtomics::handleAtomicUpdate(
    WRNAtomicNode *AtomicNode, StructType *IdentTy, Constant *TidPtr,
    bool IsTargetSPIRV) {
  assert(AtomicNode && "AtomicNode is null.");

  if (AtomicNode->getBBSetSize() != 3) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": AtomicNode for Update does not have 3 BBlocks.\n");
    return false;
  }

  // The first and last BasicBlocks contain directive intrinsic calls.
  // We're interested in only the middle one here.
  BasicBlock *BB = *(AtomicNode->bbset_begin() + 1);

  return handleAtomicUpdateInBlock(AtomicNode, BB, IdentTy, TidPtr,
                                   IsTargetSPIRV);
}

// Handles generation of KMPC intrinsics for Atomic Update.
Instruction *VPOParoptAtomics::handleAtomicUpdateInBlock(
    WRegionNode *W, BasicBlock *BB, StructType *IdentTy, Constant *TidPtr,
    bool IsTargetSPIRV) {

  if (!IsTargetSPIRV) {
    assert(IdentTy && "IdentTy is null.");
    assert(TidPtr && "TidPtr is null.");
  }

  // We maintain a list of Instructions that will be deleted from BB, when the
  // KMPC call is added.
  SmallVector<Instruction*, ApproxNumInstsToDeleteForUpdate> InstsToDelete;

  // Make sure that the BBlock has enough Instructions to start with.
  if (BB->size() <= 3) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": Atomic update BBlock has less than 4"
                      << " Instructions. Returning.\n");
    return nullptr; // Handle using critical section.
  }

  // The last store in the BBlock should be to the atomic operand. In the
  // future, to improve flexibility, the frontend could send in the atomic
  // operand as part of the intrinsic.
  StoreInst *OpndStore = VPOParoptAtomics::getLastStoreInBB(*BB);
  if (!OpndStore) {
    LLVM_DEBUG(
        dbgs() << __FUNCTION__
               << ": Unable to find store to atomic operand. Returning.\n");
    return nullptr; // Handle using critical section.
  }

  // A call to atomic update intrinsic is of form:
  //     call void __kmpc_atomic_<...>(loc, tid, <type1*> atomic_opnd, <type2>
  //     value_opnd)
  // So aside from tid and loc, we need the two operands, as well as the
  // BinaryOperator Instruction whose result is stored to `atomic_opnd` in the
  // incoming IR (which is used to determine the intrinsic to be called).
  Instruction *OpInst;
  Value *AtomicOpnd, *ValueOpnd;
  bool Reversed; // e.g.: x = expr - x;
  AtomicOpnd = OpndStore->getPointerOperand();
  StoreInst* AtomicOpndStore;

  // Now using AtomicOpnd, we try to extract other information about the atomic
  // update operation.
  AtomicUpdateOp OpKind = extractAtomicUpdateOp(
      BB, AtomicOpnd,                                              // In
      OpInst, ValueOpnd, Reversed, AtomicOpndStore,                // Out
      InstsToDelete);                                              // Out

  bool UpdateOpFound = (OpKind != AtomicUpdateNone);
  if (!UpdateOpFound)
    return nullptr; // Handle using critical sections.

  assert(OpndStore == AtomicOpndStore &&
         "Invalid atomic operand store found.");

  removeDuplicateInstsFromList(InstsToDelete);

  if (instructionsAreUsedOutsideBB(InstsToDelete, BB))
    return nullptr; // Handle using critical section.

  // At this point, we may need to generate a CastInst for ValueOpnd, in case
  // the types of AtomicOpnd and ValueOpnd are not the same.
  Type* AtomicOpndElemTy = OpndStore->getValueOperand()->getType();
  CastInst *ValueOpndCast = genCastForValueOpnd<WRNAtomicUpdate>(
      OpInst, Reversed, AtomicOpndElemTy, ValueOpnd);

  ValueOpnd = ValueOpndCast ? ValueOpndCast : ValueOpnd;
  // Note that we have not yet inserted this Cast into the IR. We do that only
  // after we find a matching intrinsic.

  // Now we know the atomic operand, the value operand, and the operation. We
  // now check whether an intrinsic exists which supports the given combination
  // of operation and operands.
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Atomic Opnd: " << *AtomicOpnd
                    << "\n");
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Value Opnd: " << *ValueOpnd << "\n");
  const std::string Name = getAtomicUCIntrinsicName<WRNAtomicUpdate>(
      *OpInst, OpKind, Reversed, AtomicOpndElemTy, *ValueOpnd, IsTargetSPIRV);
  if (Name.empty()) {
    if (ValueOpndCast)
      delete ValueOpndCast;
    // No intrinsic found. Handle using critical sections.
    return nullptr;
  }

  // We found the matching intrinsic. So, it's safe to insert ValueOpndCast
  // into the IR.
  if (ValueOpndCast)
    ValueOpndCast->insertBefore(OpndStore);

  if (IsTargetSPIRV)
    AtomicOpnd = VPOParoptUtils::genAddrSpaceCast(AtomicOpnd, OpndStore,
                                                  vpo::ADDRESS_SPACE_GENERIC);

  // Next, generate and insert the KMPC call for atomic update. It looks like:
  //     call void __kmpc_atomic_<...>(loc, tid, atomic_opnd, value_opnd)
  CallInst *AtomicCall =
      genAtomicCall(W, IdentTy, TidPtr, OpndStore, Name,
                    Type::getVoidTy(BB->getParent()->getContext()),
                    {AtomicOpnd, ValueOpnd}, IsTargetSPIRV);

  assert(AtomicCall && "Generated Atomic Update call is null.");

  AtomicCall->insertBefore(OpndStore);
  AtomicCall->setDebugLoc(OpndStore->getDebugLoc());

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic call inserted.\n");

  // And finally, delete the instructions that are no longer needed.
  deleteInstructionsInList(InstsToDelete);

  return AtomicCall;
}

// Handles generation of KMPC intrinsics for Atomic Capture.
bool VPOParoptAtomics::handleAtomicCapture(WRNAtomicNode *AtomicNode,
                                           StructType *IdentTy,
                                           Constant *TidPtr,
                                           bool IsTargetSPIRV) {
  assert(AtomicNode != nullptr && "AtomicNode is null.");
  assert(IdentTy != nullptr && "IdentTy is null.");
  assert(TidPtr != nullptr && "TidPtr is null.");

  if (AtomicNode->getBBSetSize() != 3) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": AtomicNode for Capture does not have 3 BBlocks.\n");
    return false;
  }

  // The first and last BasicBlocks contain directive intrinsic calls.
  // We're interested in only the middle one here.
  BasicBlock *BB = *(AtomicNode->bbset_begin() + 1);

  return handleAtomicCaptureInBlock(AtomicNode, BB, IdentTy, TidPtr,
                                    IsTargetSPIRV);
}

Instruction *VPOParoptAtomics::handleAtomicCaptureInBlock(WRegionNode *W,
                                                          BasicBlock *BB,
                                                          StructType *IdentTy,
                                                          Constant *TidPtr,
                                                          bool IsTargetSPIRV) {
  // Make sure that the BBlock has enough Instructions to start with.
  if (BB->size() <= 3) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": Atomic Capture BBlock has less than 4"
                      << " Instructions. Returning.\n");
    return nullptr; // Handle using critical section.
  }

  // We use the last statement of this BB as the anchor for new instructions.
  Instruction* Anchor = &*(BB->rbegin());

  // A call to atomic capture intrinsic is of form:
  //
  //    capture_opnd = __kmpc_atomic_<...>(loc, tid, <type*> atomic_opnd, <type>
  //    value_opnd [, capture_flag])
  //
  // We need the three operands, as well as the operation Instruction whose
  // result is stored to `atomic_opnd` in the incoming IR (which is used to
  // determine the intrinsic to be called). `capture_flag` represents the
  // kind of capture operation:
  // * (A) Capture type: CaptureBeforeOp. capture_flag = 0.
  // * (B) Capture type: CaptureSwap. capture_flag: not present.
  // * (C) Capture type: CaptureAfterOp. capture_flag = 1.
  Instruction *OpInst;
  StoreInst * AtomicStore;
  Value *AtomicOpnd, *ValueOpnd, *CaptureOpnd;
  AtomicUpdateOp OpKind;
  bool Reversed; // true if update operation is reversed. (e.g. x = v - x).
  CastInst *CaptureOpndCast; // Cast to CaptureOpnd's data type from
                             // AtomicOpnd's type before the store to
                             // CaptureOpnd.
  SmallVector<Instruction *, ApproxNumInstsToDeleteForCapture>
      InstsToDelete; // Instructions that will be
                     // deleted from BB when the KMPC
                     // call is generated.

  AtomicCaptureKind CaptureKind =
      extractAtomicCaptureOp(BB,                                         // In
                             OpInst, AtomicOpnd, ValueOpnd, CaptureOpnd, // Out
                             Reversed, AtomicStore, CaptureOpndCast,     // Out
                             OpKind,                                     // Out
                             InstsToDelete); // In, Out

  if (CaptureKind == CaptureUnknown)
    return nullptr; // Handle using critical section.

  removeDuplicateInstsFromList(InstsToDelete);

  if (instructionsAreUsedOutsideBB(InstsToDelete, BB))
    return nullptr; // Handle using critical section.

  // At this point, we may need to generate a CastInst for ValueOpnd, in case
  // the types of AtomicOpnd and ValueOpnd are not the same.
  Type* AtomicOpndElemTy = AtomicStore->getValueOperand()->getType();
  CastInst *ValueOpndCast = genCastForValueOpnd<WRNAtomicCapture>(
      OpInst, Reversed, AtomicOpndElemTy, ValueOpnd);

  ValueOpnd = ValueOpndCast != nullptr ? ValueOpndCast : ValueOpnd;
  // Note that we have not yet inserted this Cast into the IR. We do that only
  // after we find a matching intrinsic.

  // We know the atomic operand, the value operand, and the operation. We can
  // find a compatible KMPC intrinsic now.
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Atomic Opnd: " << *AtomicOpnd
                    << "\n");
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Capture Opnd: " << *CaptureOpnd
                    << "\n");
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Value Opnd: " << *ValueOpnd << "\n");
  assert(AtomicOpnd->getType()->isPointerTy() && "Unexpected AtomicOpnd.");
  assert(CaptureOpnd->getType()->isPointerTy() && "Unexpected CaptureOpnd.");

  const std::string Name = getAtomicCaptureIntrinsicName(
      CaptureKind, BB, OpInst, OpKind, Reversed, AtomicOpndElemTy, ValueOpnd,
      IsTargetSPIRV);
  if (Name.empty()) {
    if (ValueOpndCast != nullptr)
      delete ValueOpndCast;
    // No intrinsic found. Handle using critical sections.
    return nullptr;
  }

  // We found the matching intrinsic. So, it's safe to insert ValueOpndCast
  // into the IR.
  if (ValueOpndCast != nullptr)
    ValueOpndCast->insertBefore(Anchor);

  // Now we will generate the KMPC call to intrinsic `Name`.
  // The final IR should look like:
  //     %capture.val =  __kmpc_atomic_<...>(loc, tid, atomic_opnd, value_opnd
  //     [, capture_flag])
  //     %capture.val.cast = <cast> %capture.val
  //     store %capture.val, capture_opnd

  // First, we need the Function args.
  if (IsTargetSPIRV)
    AtomicOpnd = VPOParoptUtils::genAddrSpaceCast(AtomicOpnd, Anchor,
                                                  vpo::ADDRESS_SPACE_GENERIC);
  SmallVector<Value*, 3> FnArgs = {AtomicOpnd, ValueOpnd};
  if (CaptureKind != CaptureSwap) {

    Function *F = BB->getParent();
    LLVMContext &C = F->getContext();

    Value *CaptureFlag = ConstantInt::get(
        Type::getInt32Ty(C), CaptureKind == CaptureBeforeOp ? 0 : 1);
    FnArgs.push_back(CaptureFlag);
  }

  // Second, the return type.
  Type* ReturnTy = AtomicStore->getValueOperand()->getType();
  assert(ReturnTy != nullptr && "Invalid return type for KMPC call.");

  // Now we can generate the call.
  CallInst *AtomicCall = genAtomicCall(W, IdentTy, TidPtr, Anchor, Name,
                                       ReturnTy, FnArgs, IsTargetSPIRV);
  assert(AtomicCall != nullptr && "Generated Atomic Capture call is null.");

  AtomicCall->insertBefore(Anchor);
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic call inserted.\n");

  // Now we need to store the result of this call to the capture operand,
  // which may have a different type than the value returned by the call.
  Value* CaptureVal = AtomicCall;
  if (CaptureOpndCast != nullptr)
    CaptureVal =
        CastInst::Create(CaptureOpndCast->getOpcode(), AtomicCall,
                         CaptureOpndCast->getType(),
                         "cpt.opnd.cast", Anchor);

  // Now generate the store to CaptureOpnd.
  StoreInst *CaptureStore =
      new StoreInst(CaptureVal, CaptureOpnd, false /*volatile*/, Anchor);
  (void)CaptureStore;

  // And finally, delete the instructions that are no longer needed.
  deleteInstructionsInList(InstsToDelete);

  return AtomicCall;
}

// Omit the Ident and Tid parameters if IsTargetSPIRV is true.
// Note that this function does not emit the call, and InsertPt is just
// used to obtain the Bblock where the atomic pragma resides.
CallInst *VPOParoptAtomics::genAtomicCall(WRegionNode *W,
                                          StructType *IdentTy, Constant *TidPtr,
                                          Instruction *InsertPt, StringRef Name,
                                          Type *ReturnTy,
                                          ArrayRef<Value *> Args,
                                          bool IsTargetSPIRV) {
  CallInst *AtomicCall = nullptr;

  if (!IsTargetSPIRV) {
    AtomicCall = VPOParoptUtils::genKmpcCallWithTid(
        W, IdentTy, TidPtr, InsertPt, Name, ReturnTy, Args);
    assert(AtomicCall && "Generated KMPC Atomic call is null.");
  } else {
    BasicBlock *B = InsertPt->getParent();
    Function *F = B->getParent();
    Module *M = F->getParent();
    AtomicCall = VPOParoptUtils::genCall(M, Name, ReturnTy, Args, nullptr);
    VPOParoptUtils::setFuncCallingConv(AtomicCall, M);
    assert(AtomicCall && "Generated Atomic call for GPU offloading is null.");
  }
  return AtomicCall;
}

// Methods for identifying the op and opnds for Atomic update and capture.

// Extract the update operation (OpInst) and its value operand (ValueOpnd) for
// a given AtomicOpnd.
//
// Example:
//     x = x - expr; // `x` is int, `expr` is float
//                   // `x` is atomic opnd, `expr` is value opnd.
// IR:
//     %2 = load float, float* %expr, align 4
//     %3 = load i32, i32* %x, align 4          ; <- (i)
//     %conv = sitofp i32 %3 to float           ; <- (ii)
//     %sub = fsub float %2, %conv              ; <- (iii)
//     %conv1 = fptosi float %sub to i32c       ; <- (iv)
//     store i32 %conv1, i32* %x, align 4       ; <- (v)
//
//  Instructions (i) to (v) will be added to InstsToDelete.
//  Details are in the header file.
VPOParoptAtomics::AtomicUpdateOp VPOParoptAtomics::extractAtomicUpdateOp(
    BasicBlock *BB, Value *AtomicOpnd,                       // In
    Instruction *&OpInst, Value *&ValueOpnd, bool &Reversed, // Out
    StoreInst *&AtomicOpndStore,                             // Out
    SmallVectorImpl<Instruction *> &InstsToDelete) {         // In, Out

  assert(BB != nullptr && "BasicBlock is null.");
  assert(AtomicOpnd != nullptr && "AtomicOpnd is null.");
  OpInst = nullptr;
  ValueOpnd = nullptr;
  Reversed = false;
  AtomicOpndStore = nullptr;
  AtomicUpdateOp OpKind = AtomicUpdateNone;

  // We have AtomicOpnd. We look for a store to it within the BasicBlock BB.
  // There should only be one.
  StoreInst *AtomicStore = getStoreToOpndIfUnique(*BB, *AtomicOpnd);
  if (AtomicStore == nullptr) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": More than one stores in BB for AtomicOpnd:"
                      << *AtomicOpnd << "\n");
    return AtomicUpdateNone; // Handle using critical sections.
  }

  AtomicOpndStore = AtomicStore;
  InstsToDelete.push_back(AtomicStore); // (v)

  Instruction* Op; // atomic update operation e.g. `sub` for `x = x - expr`.
  // Now, the value being stored in AtomicStore will give us the atomic
  // operation (Op). But to get that, we need to strip off any casts, if
  // present.
  Value *OpResult = AtomicStore->getValueOperand();
  OpResult = VPOUtils::stripCasts(OpResult, InstsToDelete); // (iv)

  Op = dyn_cast<Instruction>(OpResult);

  if (Op == nullptr || (!isa<BinaryOperator>(Op) && !isa<SelectInst>(Op))) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Unexpected update Op:" << *OpResult
                      << "\n");
    InstsToDelete.clear();
    return AtomicUpdateNone;
  }

  if (isa<SelectInst>(Op)) {
    // Recognize min/max updates.
    //
    // TODO: add recognition for boolean and/or and .eqv./.neqv. updates.
    //       Note that reduction(.eqv.:) is recognized as AtomicUpdateXor
    //       due to the its lowering in Paropt, but we may need to support
    //       the atomic update pattern, if FE passes it to us (right now,
    //       all FEs are lowering atomic updates themselves).
    SmallVector<Instruction *, 8> CastInstructions;

    CmpInst *PredicateInst = dyn_cast<CmpInst>(
        VPOUtils::stripCasts(Op->getOperand(0), CastInstructions));
    if (!PredicateInst) {
      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Unexpected update Op:" << *Op
                 << "\n");
      InstsToDelete.clear();
      return AtomicUpdateNone;
    }

    Value *LhsCmp =
        VPOUtils::stripCasts(PredicateInst->getOperand(0), CastInstructions);
    Value *RhsCmp =
        VPOUtils::stripCasts(PredicateInst->getOperand(1), CastInstructions);
    CmpInst::Predicate Pred = PredicateInst->getPredicate();
    Value *TrueOp = VPOUtils::stripCasts(Op->getOperand(1), CastInstructions);
    Value *FalseOp = VPOUtils::stripCasts(Op->getOperand(2), CastInstructions);

    // MAX:
    //   x = x <sgt> y ? x : y
    //   x = x <ugt> y ? x : y
    //   x = x <ogt> y ? x : y
    if ((Pred == CmpInst::ICMP_SGT || Pred == CmpInst::ICMP_UGT ||
         Pred == CmpInst::FCMP_OGT) &&
        LhsCmp == TrueOp && RhsCmp == FalseOp && isa<LoadInst>(LhsCmp) &&
        cast<LoadInst>(LhsCmp)->getPointerOperand() == AtomicOpnd) {
      InstsToDelete.push_back(cast<LoadInst>(LhsCmp));
      ValueOpnd = RhsCmp;
      if (PredicateInst->isFPPredicate())
        OpKind = AtomicUpdateFMax;
      else if (PredicateInst->isUnsigned())
        OpKind = AtomicUpdateUMax;
      else
        OpKind = AtomicUpdateSMax;
    }

    // MIN:
    //   x = x <sgt> y ? y : x
    //   x = x <ugt> y ? y : x
    //   x = x <ogt> y ? y : x
    if ((Pred == CmpInst::ICMP_SGT || Pred == CmpInst::ICMP_UGT ||
         Pred == CmpInst::FCMP_OGT) &&
        LhsCmp == FalseOp && RhsCmp == TrueOp && isa<LoadInst>(LhsCmp) &&
        cast<LoadInst>(LhsCmp)->getPointerOperand() == AtomicOpnd) {
      InstsToDelete.push_back(cast<LoadInst>(LhsCmp));
      ValueOpnd = RhsCmp;
      if (PredicateInst->isFPPredicate())
        OpKind = AtomicUpdateFMin;
      else if (PredicateInst->isUnsigned())
        OpKind = AtomicUpdateUMin;
      else
        OpKind = AtomicUpdateSMin;
    }

    if (OpKind == AtomicUpdateNone) {
      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Unexpected update Op:" << *Op
                 << "\n");
      InstsToDelete.clear();
      return AtomicUpdateNone;
    }

    InstsToDelete.append(CastInstructions.begin(), CastInstructions.end());
    InstsToDelete.push_back(PredicateInst);
    OpInst = Op;
    Reversed = false;

    return OpKind;
  }

  InstsToDelete.push_back(Op); // (iii)

  // Now, we have the operation. One of its two operands should be a load from
  // AtomicOpnd, (or a cast of it). Then, the other operand will be ValueOpnd.
  unsigned Idx;
  for (Idx = 0; Idx <= 1; ++Idx) {
    Value *Opnd = Op->getOperand(Idx);

    SmallVector<Instruction*, 2> OpndLoadCasts;
    Opnd = VPOUtils::stripCasts(Opnd, OpndLoadCasts);

    auto *OpndLoad = dyn_cast<LoadInst>(Opnd);
    if (OpndLoad == nullptr)
      continue;

    if (OpndLoad->getPointerOperand() == AtomicOpnd) {
      if (!OpndLoadCasts.empty())
        InstsToDelete.append(OpndLoadCasts.begin(),
                             OpndLoadCasts.end()); // (ii)
      InstsToDelete.push_back(OpndLoad);           // (i)
      break;
    }
  }

  if (Idx > 1) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": Load of AtomicOpnd not used in Op:" << *Op << "\n");
    InstsToDelete.clear();
    return AtomicUpdateNone;
  }
  // A load for AtomicOpnd was found at index `Idx`. So, the operand at position
  // `1-Idx` is ValueOpnd.
  ValueOpnd = Op->getOperand(1 - Idx);
  assert(ValueOpnd != nullptr && "ValueOpnd is null.");
  // Also, if Op is a non-commutative operation, like div and sub, and
  // AtomicOpnd is the second operand, then the update operation is reversed.
  Reversed = Idx == 1 && !Op->isCommutative();
  OpInst = Op;

  Type *AtomicOpndTy  = AtomicOpndStore->getValueOperand()->getType();
  Type *ValueOpndTy = ValueOpnd->getType();
  return getAtomicUpdateOpForBinaryOp(Op, Reversed, AtomicOpndTy, ValueOpndTy);
}

VPOParoptAtomics::AtomicUpdateOp VPOParoptAtomics::getAtomicUpdateOpForBinaryOp(
    const Instruction *Operation, bool Reversed,
    const Type *AtomicOpndTy, const Type *ValueOpndTy) {
  static std::map<unsigned, AtomicUpdateOp> UpdateOpsMap = {
      {Instruction::AShr, AtomicUpdateAShr},
      {Instruction::Add, AtomicUpdateAdd},
      {Instruction::And, AtomicUpdateAnd},
      {Instruction::FAdd, AtomicUpdateFAdd},
      {Instruction::FDiv, AtomicUpdateFDiv},
      {Instruction::FMul, AtomicUpdateFMul},
      {Instruction::FSub, AtomicUpdateFSub},
      {Instruction::LShr, AtomicUpdateLShr},
      {Instruction::Mul, AtomicUpdateMul},
      {Instruction::Or, AtomicUpdateOr},
      {Instruction::SDiv, AtomicUpdateSDiv},
      {Instruction::Shl, AtomicUpdateShl},
      {Instruction::Sub, AtomicUpdateSub},
      {Instruction::UDiv, AtomicUpdateUDiv},
      {Instruction::Xor, AtomicUpdateXor}
  };

  auto It = UpdateOpsMap.find(Operation->getOpcode());
  if (It == UpdateOpsMap.end())
    return AtomicUpdateNone;

  AtomicUpdateOp OpKind = It->second;

  // Special case for atomic update:
  // For the case: x = x/v, when x is an integer or unsigned, and y is
  // float128, the IR looks like:
  //     %3 = load fp128, fp128* %v, align 16
  //     %div = fdiv fp128 %conv, %3
  //
  // where, %conv is different, based on whether x is an integer, or an
  // unsigned:
  //     %2 = load i32, i32* %x, align 4
  //     %conv = sitofp i32 %2 to fp128  ; (1) x is signed integer
  //     %conv = uitofp i32 %2 to fp128  ; (2) x is unsigned
  //
  // The KMPC intrinsic is different for the two cases, so to differentiate the
  // two in OpToUpdateIntrinsicMap, Opcode for `fdiv` is used for case (1),
  // and op code `udiv` is used for case (2).
  if (OpKind == AtomicUpdateFDiv && ValueOpndTy->isFP128Ty() &&
      isUIToFPCast(*(Operation->getOperand(Reversed ? 1 : 0)))) {
    OpKind = AtomicUpdateUDiv;
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Changed FDiv to UDiv.\n");
  }

  // Similarly for multiplication of unsigned ints with F128, we use Mul
  // instead of FMul.
  if (OpKind == AtomicUpdateFMul && ValueOpndTy->isFP128Ty() &&
      isUIToFPCast(*(Operation->getOperand(Reversed ? 1 : 0)))) {
    OpKind = AtomicUpdateMul;
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Changed FMul to Mul.\n");
  }

  // Division, shift operations on uints may use the same operator as ints.
  // uint32, uint64 use lshr/udiv instead of ashr/sdiv if both atomic and value
  // operands are of the same type. Otherwise, sdiv/ashr are used for both int,
  // uint. To identify the kmpc routines correctly, we need to check if there is
  // a zext/sext on the atomic operand before the actual sdiv/ashr operation.
  //
  // -----------------------------------+---------------------------------------
  //   int8_t i1; uint8_t u1; uint64_t rhs;
  //                                    |
  //   #pragma omp atomic               | #pragma omp atomic
  //   i1 >>= rhs;                      | u1 >>= rhs;
  // -----------------------------------+---------------------------------------
  //               [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
  //                                    |
  //  %25 = load i64, i64* %rhs.addr    | %28 = load i64, i64* %rhs.addr
  //  %26 = load i8, i8* @i1            | %29 = load i8, i8* @u1
  //  %conv20 = sext i8 %26 to i32      | %conv22 = zext i8 %29 to i32
  //  %sh_prom = trunc i64 %25 to i32   | %sh_prom23 = trunc i64 %28 to i32
  //  %shl.mask = and i32 %sh_prom, 31  | %shl.mask24 = and i32 %sh_prom23, 31
  //  %shr = ashr i32 %conv20, %shl.mask| %shr25 = ashr i32 %conv22, %shl.mask24
  //  %conv21 = trunc i32 %shr to i8    | %conv26 = trunc i32 %shr25 to i8
  //  store i8 %conv21, i8* @i1         | store i8 %conv26, i8* @u1
  //                                    |
  //                        [ "DIR.OMP.END.ATOMIC"() ]
  //                                    |

  if (OpKind == AtomicUpdateSDiv &&
      (AtomicOpndTy->isIntegerTy(8) || AtomicOpndTy->isIntegerTy(16) ||
       AtomicOpndTy->isIntegerTy(32) || AtomicOpndTy->isIntegerTy(64)) &&
      isa<ZExtInst>(*(Operation->getOperand(Reversed ? 1 : 0)))) {
    OpKind = AtomicUpdateUDiv;
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Changed SDiv to UDiv.\n");
  }

  if (OpKind == AtomicUpdateAShr &&
      (AtomicOpndTy->isIntegerTy(8) || AtomicOpndTy->isIntegerTy(16) ||
       AtomicOpndTy->isIntegerTy(32) || AtomicOpndTy->isIntegerTy(64)) &&
      isa<ZExtInst>(*(Operation->getOperand(Reversed ? 1 : 0)))) {
    OpKind = AtomicUpdateLShr;
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Changed AShr to LShr.\n");
  }

  return OpKind;
}

// For a given BB, try to find the capture op and operands.
VPOParoptAtomics::AtomicCaptureKind VPOParoptAtomics::extractAtomicCaptureOp(
    BasicBlock *BB,                                                   // In
    Instruction *&OpInst,  Value *&AtomicOpnd, Value *&ValueOpnd,      // Out
    Value *&CaptureOpnd, bool &Reversed, StoreInst *&AtomicOpndStore, // Out
    CastInst *&CaptureOpndCast, AtomicUpdateOp &OpKind,               // Out
    SmallVectorImpl<Instruction *> &InstsToDelete) {                  // In, Out

  assert(BB != nullptr && "BB is null.");

  // A store to AtomicOpnd/CaptureOpnd could only be the first, second-last or
  // the last StoreInst within BB.
  SmallVector<StoreInst *, 3> StoreCandidates =
      gatherFirstSecondToLastAndLastStores(*BB);

  if (StoreCandidates.size() < 3) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": Unexpected: Less than two stores found in BB.\n");
    return CaptureUnknown;
  }

  AtomicCaptureKind CaptureKind = CaptureUnknown;

  // (A) CaptureBeforeOp:
  // First, try to process the capture-before-op case. Here, the last store of
  // BB should be the store to x, and the first store should be a store to v.
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Processing as CaptureBeforeOp...\n");
  AtomicOpnd = (*StoreCandidates.rbegin())->getPointerOperand();
  CaptureOpnd = (*StoreCandidates.begin())->getPointerOperand();

  OpKind =
      extractAtomicUpdateOp(BB, AtomicOpnd,                               // In
                            OpInst, ValueOpnd, Reversed, AtomicOpndStore, // Out
                            InstsToDelete); // In, Out

  bool UpdateOpFound = (OpKind != AtomicUpdateNone);
  if (UpdateOpFound)
    CaptureKind =
        identifyNonSwapCaptureKind(BB, AtomicOpndStore, CaptureOpnd, // In
                                   CaptureOpndCast,                  // Out
                                   InstsToDelete);                   // In, Out

  if (CaptureKind != CaptureUnknown) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": CaptureBeforeOp identified. Op: "
                      << *OpInst << "\n");
    return CaptureKind;
  } else if (UpdateOpFound) {
    LLVM_DEBUG(
        dbgs() << __FUNCTION__
               << ": Skipping processing as CaptureSwap/CaptureAfterOp since "
                  "we found an Update Operation on the operand of the last "
                  "store ("
               << *AtomicOpnd << ").\n");
    return CaptureUnknown;
  }

  // (B) CaptureSwap
  // Second, try to process CaptureSwap. Here again, first store of BB will be
  // to v, and the last store to x.
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Processing as CaptureSwap...\n");
  AtomicOpnd = (*StoreCandidates.rbegin())->getPointerOperand();
  CaptureOpnd = (*StoreCandidates.begin())->getPointerOperand();

  bool SwapOpFound =
      extractSwapOp(BB, AtomicOpnd, CaptureOpnd,                 // In
                    ValueOpnd, AtomicOpndStore, CaptureOpndCast, // Out
                    InstsToDelete);                              // In, Out

  if (SwapOpFound) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": CaptureSwap identified.\n");
    return CaptureSwap;
  }

  // (C) CaptureAfterOp
  // Last, try to process CaptureAfterOp. Here Last store of BB will be to v,
  // and second last to x.
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Processing as CaptureAfterOp...\n");
  AtomicOpnd =(*(StoreCandidates.rbegin() + 1))->getPointerOperand();
  CaptureOpnd =(*StoreCandidates.rbegin())->getPointerOperand();

  OpKind =
      extractAtomicUpdateOp(BB, AtomicOpnd,                               // In
                            OpInst, ValueOpnd, Reversed, AtomicOpndStore, // Out
                            InstsToDelete); // In, Out

  UpdateOpFound = (OpKind != AtomicUpdateNone);
  if (UpdateOpFound)
    CaptureKind =
        identifyNonSwapCaptureKind(BB, AtomicOpndStore, CaptureOpnd, // In
                                   CaptureOpndCast,                  // Out
                                   InstsToDelete);                   // In, Out

  if (CaptureKind != CaptureUnknown) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": CaptureAfterOp identified. Op: "
                      << *OpInst << "\n");
    return CaptureKind;
  }

  // (D) No Capture operation found.
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Capture Op not found.\n");
  return CaptureUnknown;
}

// Identify the kind of capture operation for the given atomic and capture
// operands. This is called for non-swap type atomic captures.

// Example: CaptureAfterOp:
//     x = x - expr; v = x;
//  Here, `x` is float, `expr` is double, `v` is uint64_t. `x` is atomic opnd,
//  `expr` is value opnd, `v` is capture opnd.
//
// IR:
//     %2 = load float, float* %x, align 4
//     %conv = fpext float %2 to double
//     %3 = load double, double* %expr, align 8
//     %sub = fsub double %conv, %3
//     %conv1 = fptrunc double %sub to float
//     store float %conv1, float* %x, align 4
//     %4 = load float, float* %x, align 4      ; <- (a)
//     %conv2 = fptoui float %4 to i64          ; <- (b)
//     store i64 %conv2, i64* %v, align 8       ; <- (c)
//
//  Details on InstsToDelete are in the header file.
VPOParoptAtomics::AtomicCaptureKind
VPOParoptAtomics::identifyNonSwapCaptureKind(
    BasicBlock *BB, StoreInst *AtomicStore, Value *CaptureOpnd, // In
    CastInst *&CaptureOpndCast,                                 // Out
    SmallVectorImpl<Instruction *> &InstsToDelete) {            // In, Out

  assert(BB != nullptr && "BB is null.");
  assert(AtomicStore != nullptr && "AtomicStore is null.");
  assert(CaptureOpnd != nullptr && "CaptureOpnd is null.");
  assert(AtomicStore->getParent() == BB && "AtomicStore is not in BB.");

  Value *AtomicOpnd = AtomicStore->getPointerOperand();
  StoreInst *CaptureStore = getStoreToOpndIfUnique(*BB, *CaptureOpnd);
  if (CaptureStore == nullptr) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": Multiple stores to capture operand within BB.\n");
    return CaptureUnknown;
  }

  // Now, check if the value being stored to `v` is a load of `x`.
  // i.e. `v = x`. Strip any cast on the way.
  // val = load x
  // [val_cast = <cast> val]
  // store val[v_cast], v
  Value *ValV = CaptureStore->getValueOperand();
  auto *ValVCast = dyn_cast<CastInst>(ValV);
  CaptureOpndCast = ValVCast;

  if (ValVCast != nullptr)
    ValV = ValVCast->getOperand(0);
  auto *LoadX = dyn_cast<LoadInst>(ValV);

  if (LoadX != nullptr && LoadX->getPointerOperand() != AtomicOpnd) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Value stored to capture operand ("
                      << *(LoadX->getPointerOperand())
                      << ") is not atomic operand (" << *AtomicOpnd << ") \n");
    return CaptureUnknown;
  } else if (LoadX != nullptr) {
    // Match found for `v = x`. We mark the instructions which assign `x` to `v`
    // for deletion, because `v` will be assigned the return value of the KMPC
    // call.
    InstsToDelete.push_back(CaptureStore); // (c)
    InstsToDelete.push_back(LoadX);        // (a)
    if (ValVCast != nullptr)
      InstsToDelete.push_back(ValVCast);   // (b)

    assert(LoadX->getParent() == BB && "ValVLoad is not in BB.");
    assert(AtomicStore->getParent() == BB && "AtomicStore is not in BB.");

    // This load of `x` could have happened either before, or after the
    // update to `x`. Find out which.
    BasicBlock::iterator It, EndIt;
    BasicBlock::iterator StIt = AtomicStore->getIterator();
    for (It = LoadX->getIterator(), EndIt = BB->end(); It != EndIt; ++It) {
      if (It == StIt)
        return CaptureBeforeOp;
    }
    return CaptureAfterOp;
  }

  // Reaching here means that LoadX was null. i.e. the value being stored to
  // `v` is not a load of `x`. So it might be the result of the update
  // operation being stored to `x`. Compare it wih that value. Expected pattern:
  //    store valx, x
  //    [valv_cast = <cast> valx]
  //    store val[v_cast], v
  // The cast for the value stored to `v` has already been stripped.
  Value* ValX = AtomicStore->getValueOperand();

  if (ValV != ValX) {
    LLVM_DEBUG(
        dbgs() << __FUNCTION__
               << ": Value stored to capture operand is not atomic operand: v: "
               << *ValV << "x: " << *ValX << " \n");
    return CaptureUnknown;
  }

  // Reaching here means value stored to `v' is the result of update op.
  // ValX should already have been marked for deletion in extractAtomicUpdateOp.
  // No need to do it again.
  InstsToDelete.push_back(CaptureStore); // (c)
  if (ValVCast != nullptr)
    InstsToDelete.push_back(ValVCast); // (b)

  return CaptureAfterOp;
}

// Using a given AtomicOpnd and CaptureOpnd, try to find ValueOpnd assuming the
// capture operation is swap.
// Example:
//     v = x; x = expr;
//     Here AtomicOpnd `x` is float, CaptureOpnd `v` is uint64_t and
//     ValueOpnd `expr` is double.
// IR:
//     %2 = load float, float* %x, align 4       ; <- (1)
//     %conv = fptoui float %2 to i64            ; <- (2)
//     store i64 %conv, i64* %v, align 8         ; <- (3)
//     %3 = load double, double* %expr, align 8
//     %conv1 = fptrunc double %3 to float
//     store float %conv1, float* %x, align 4    ; <- (4)
//
//  Details on InstsToDelete are in the header file.
bool VPOParoptAtomics::extractSwapOp(
    BasicBlock *BB, Value *AtomicOpnd, Value *CaptureOpnd, // In
    Value *&ValueOpnd, StoreInst *&AtomicOpndStore,        // Out
    CastInst *&CaptureOpndCast,                            // Out
    SmallVectorImpl<Instruction *> &InstsToDelete) {       // In, Out

  // Consider the swap operation: v = x; x = expr;
  // Here x is AtomicOpnd, v is CaptureOpnd and expr is Value Operand.
  assert(BB != nullptr && "BB is null.");
  assert(AtomicOpnd != nullptr && "AtomicOpnd is null.");
  assert(CaptureOpnd != nullptr && "CaptureOpnd is null.");

#ifndef NDEBUG
  // The function should be called only after making sure that the store to `x`
  // is not of atomic update type.
  Instruction* OpInst;
  StoreInst* AtStore;
  bool Reversed;
  AtomicUpdateOp OpKind =
      extractAtomicUpdateOp(BB, AtomicOpnd,                       // In
                            OpInst, ValueOpnd, Reversed, AtStore, // Out
                            InstsToDelete);                       // In, Out
  assert(OpKind == AtomicUpdateNone && "Atomic capture is not of swap kind.");
  (void)OpKind;
#endif

  StoreInst* AtomicStore = getStoreToOpndIfUnique(*BB, *AtomicOpnd);
  StoreInst* CaptureStore = getStoreToOpndIfUnique(*BB, *CaptureOpnd);

  if(AtomicStore == nullptr || CaptureStore == nullptr) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": There should be only one store to "
                         "atomic and capture operands.\n");
    return false;
  }

  // The value being stored to `v` should be a load of `x`.
  //    val = load x
  //    [val_cast = <cast> val]
  //    store val[_cast], v
  Value* ValV = CaptureStore->getValueOperand();
  auto *ValVCast = dyn_cast<CastInst>(ValV);
  CaptureOpndCast = ValVCast; // Set out-parameter CaptureOpndCast.

  if (ValVCast != nullptr)
    ValV = ValVCast->getOperand(0);
  auto  *LoadX = dyn_cast<LoadInst>(ValV);
  if (LoadX == nullptr ||
      LoadX->getPointerOperand() != AtomicOpnd)
    return false;

  // At this point, it should have been checked that `x` is not an atomic
  // update type operand. So, the value being stored to `x` is the ValueOpnd
  // `expr` for the swap operation.
  ValueOpnd = AtomicStore->getValueOperand();

  // Also the KMPC call will be of form: v = __kmpc_atomic<...>(...x, expr). So
  // we don't need the existing store to v and x.
  InstsToDelete.push_back(AtomicStore);  // (4)
  InstsToDelete.push_back(CaptureStore); // (3)
  InstsToDelete.push_back(LoadX);        // (1)
  if (ValVCast != nullptr)
    InstsToDelete.push_back(ValVCast); // (2)
  AtomicOpndStore = AtomicStore;
  return true;
}

// Functions for generating a cast for the value operand for update/capture.

// Generates a CastInst for ValueOpnd depending upon the Op and AtomicOpnd, if
// needed.
template <WRNAtomicKind AtomicKind>
CastInst *VPOParoptAtomics::genCastForValueOpnd(const Instruction *Op,
                                                bool Reversed,
                                                Type *AtomicOpndElemTy,
                                                Value *ValueOpnd) {
  assert((AtomicKind == WRNAtomicUpdate || AtomicKind == WRNAtomicCapture) &&
         "Invalid atomic kind.");
  assert((Op != nullptr || AtomicKind == WRNAtomicCapture) &&
         "Op is needed for atomic update.");
  assert((AtomicKind == WRNAtomicCapture || isa<BinaryOperator>(Op) ||
          isa<SelectInst>(Op)) && "Unsupported Op type.");
  assert(AtomicOpndElemTy != nullptr && "AtomicOpndElemTy is null.");
  assert(ValueOpnd != nullptr && "ValueOpnd is null.");

  Type* ValueOpndTy = ValueOpnd->getType();

  if (AtomicOpndElemTy->isIntegerTy() && ValueOpndTy->isIntegerTy())
    return genTruncForValueOpnd(AtomicOpndElemTy, *ValueOpnd);

  if (AtomicKind == WRNAtomicUpdate && AtomicOpndElemTy->isIntegerTy() &&
      ValueOpndTy->isFloatingPointTy())
    return genFPExtForValueOpnd(*Op, Reversed, AtomicOpndElemTy, *ValueOpnd);

  if (AtomicOpndElemTy->isFloatingPointTy() && ValueOpndTy->isFloatingPointTy())
    return genFPTruncForValueOpnd<AtomicKind>(AtomicOpndElemTy, *ValueOpnd);

  // No need for a Cast.
  return nullptr;
}

// Generates an integer Trunc Cast for ValueOpnd for Atomic Update, if needed.
CastInst *VPOParoptAtomics::genTruncForValueOpnd(Type *AtomicOpndElemTy,
                                                 Value &ValueOpnd) {
  IntegerType *ValueOpndTy = dyn_cast<IntegerType>(ValueOpnd.getType());
  IntegerType *AtomicOpndTy = dyn_cast<IntegerType>(AtomicOpndElemTy);

  if (AtomicOpndTy == nullptr || ValueOpndTy == nullptr ||
      AtomicOpndTy->getBitWidth() >=
          ValueOpndTy->getBitWidth()) // Cast not needed when AtomicOpnd has
                                      // more bits than ValueOpnd.
    return nullptr;

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Generating Trunc for ValueOpnd: "
                    << ValueOpnd << "\n");

  return new TruncInst(&ValueOpnd, AtomicOpndTy, "val.opnd.trunc");
}


// Generates an FPExt cast for ValueOpnd for Atomic Update, if needed.
CastInst *VPOParoptAtomics::genFPExtForValueOpnd(const Instruction &Op,
                                                 bool Reversed,
                                                 Type *AtomicOpndElemTy,
                                                 Value &ValueOpnd) {
  assert(isa<BinaryOperator>(Op) && "Unsupported Op type.");

  Type *ValueOpndTy = ValueOpnd.getType();

  if (!AtomicOpndElemTy->isIntegerTy() ||                         // Ptr to Int
      !(ValueOpndTy->isFloatTy() || ValueOpndTy->isX86_FP80Ty())) // F32 or F80
    return nullptr;

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Generating FPExt for ValueOpnd: "
                    << ValueOpnd << "\n");

  const BasicBlock *B = Op.getParent();
  const Function *F = B->getParent();
  LLVMContext &C = F->getContext();

  if (ValueOpndTy->isFloatTy() &&
      // We generate a cast to double (64-bit FP) for mul and div of signed
      // integer with float(32-bit)
      ((Op.getOpcode() == Instruction::FMul ||  // <signed int> mul <fp>
        (Op.getOpcode() == Instruction::FDiv && // <signed int> div <fp>
         !isUIToFPCast(*(Op.getOperand(Reversed ? 1 : 0)))))))
    return CastInst::CreateFPCast(&ValueOpnd, Type::getDoubleTy(C),
                                  "val.opnd.fpext");

  // For unsigned div and other operations for 32-bit float, and for 80-bit
  // X86_FP80Ty, we generate a cast to 128-bit float.
  return CastInst::CreateFPCast(&ValueOpnd, Type::getFP128Ty(C),
                                "val.opnd.fpext");
}

// Generates an FPTrunc cast for ValueOpnd for Atomic Update, if needed.
template <WRNAtomicKind AtomicKind>
CastInst *VPOParoptAtomics::genFPTruncForValueOpnd(Type *AtomicOpndElemTy,
                                                   Value &ValueOpnd) {
  assert((AtomicKind == WRNAtomicUpdate || AtomicKind == WRNAtomicCapture) &&
         "Invalid atomic kind.");

  Type *ValueOpndTy = ValueOpnd.getType();

  if (!AtomicOpndElemTy->isFloatingPointTy() ||
      !ValueOpndTy->isFloatingPointTy() ||
      AtomicOpndElemTy->getScalarSizeInBits() >=
          ValueOpndTy->getScalarSizeInBits()) // Cast not needed when AtomicOpnd
                                              // has more bits than ValueOpnd.
    return nullptr;

  if (AtomicKind == WRNAtomicUpdate && AtomicOpndElemTy->isFloatTy() &&
      ValueOpndTy->isDoubleTy()) // float4-float8 update intrinsics exist.
    return nullptr;

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Generating FPTrunc for ValueOpnd: "
                    << ValueOpnd << "\n");

  return CastInst::CreateFPCast(&ValueOpnd, AtomicOpndElemTy,
                                "val.opnd.fptrunc");
}

// Misc helper methods.

// Populates CandidateList with all StoreInsts from BB which could be storing to
// the atomic operand.
SmallVector<StoreInst *, 3>
VPOParoptAtomics::gatherFirstSecondToLastAndLastStores(BasicBlock &BB) {

  SmallVector<StoreInst*, 3> StoreList;

  for (auto It = BB.begin(), End = BB.end(); It != End; ++It) {
    if (StoreInst *SI = dyn_cast<StoreInst>(It)) {
      StoreList.push_back(SI); // First store -> StoreList[0]
      break;
    }
  }

  if (StoreList.size() != 1) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << " : No StoreInst found in BB\n.");
    return StoreList;
  }

  for (auto It = BB.rbegin(), REnd = BB.rend();
       It != REnd && StoreList.size() <= 3; ++It) {

    if (auto *SI = dyn_cast<StoreInst>(&*It)) {
      StoreList.insert(StoreList.begin() + 1,
                       SI); // Last store ->  StoreList[2], and
                            // Second-last store -> StoreList[1]
    }
  }

  return StoreList;
}

// Find the last StoreInst in the BasicBlock BB.
StoreInst *VPOParoptAtomics::getLastStoreInBB(BasicBlock &BB) {
  StoreInst *LastStore = nullptr;
  for (Instruction &I : BB) {
    if (StoreInst *SI = dyn_cast<StoreInst>(&I))
      LastStore = SI;
  }
  return LastStore;
}

// If the BB has only one Instruction of type InstTy, return it.
template <typename InstTy>
InstTy *VPOParoptAtomics::getLoneInstructionOfTypeInBB(BasicBlock &BB) {
  InstTy *SeenInst = nullptr;
  for (Instruction &I : BB) {
    if (InstTy *TI = dyn_cast<InstTy>(&I)) {
      if (SeenInst) // There is another Instruction of InstTy in BB.
        return nullptr;
      SeenInst = TI;
    }
  }
  return SeenInst;
}

// If there is a single StoreInst to Opnd inside BasicBlock BB, then return it.
// Otherwise, return nullptr.
StoreInst *VPOParoptAtomics::getStoreToOpndIfUnique(BasicBlock &BB,
                                                    const Value &Opnd) {

  StoreInst* OpndStore = nullptr;
  for (Instruction &I : BB) {

    // Look for a store to Opnd within BB.
    StoreInst *SI = dyn_cast<StoreInst>(&I);
    if (SI == nullptr || SI->getPointerOperand() != &Opnd)
      continue;

    if(OpndStore != nullptr) {
      LLVM_DEBUG(dbgs() << __FUNCTION__
                        << " : More than one StoreInsts to operand: " << Opnd
                        << " \n");
      return nullptr; // Only one StoreInst should be storing to Opnd.
    }

    OpndStore = SI;
  }

  return OpndStore;
}

// Tells whether Val is a CastInst of type UIToFP.
bool VPOParoptAtomics::isUIToFPCast(const Value &Val) {
  auto *Cast = dyn_cast<CastInst>(&Val);
  if (Cast != nullptr && Cast->getOpcode() == Instruction::UIToFP)
    return true;

  return false;
}

// Remove duplicate Instructions from the input SmallVector.
void VPOParoptAtomics::removeDuplicateInstsFromList(
    SmallVectorImpl<Instruction *> &Insts) {

  if (Insts.empty())
    return;

  std::sort(Insts.begin(), Insts.end());
  auto last = std::unique(Insts.begin(), Insts.end());
  Insts.erase(last, Insts.end());
}

// Check if any instruction in the list is used outside the given BB.
bool VPOParoptAtomics::instructionsAreUsedOutsideBB(
    SmallVectorImpl<Instruction *> &Insts, BasicBlock* &BB) {
  for (auto *Inst : Insts) {
    for (Use &Use : Inst->uses()) {
      User *User = Use.getUser();
      Instruction *UserInst = dyn_cast<Instruction>(User);
      if (UserInst != nullptr && UserInst->getParent() != BB) {

#ifndef NDEBUG
        LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Instruction '" << *Inst
                          << "' is used in '" << *UserInst
                          << "', which is outside the given BB.\n");
#endif
        return true;
      }
    }
  }
  return false;
}

// Delete the Instructions in the vecor InstsToDelete.
void VPOParoptAtomics::deleteInstructionsInList(
    SmallVectorImpl<Instruction *> &InstsToDelete) {

  if (InstsToDelete.empty())
    return;

#ifndef NDEBUG
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Instructions to be deleted:\n");
  for (auto *Inst: InstsToDelete) {
    LLVM_DEBUG(dbgs() << *Inst << "\n");
  }
#endif

  // Now delete all Instructions in the list.
  for (auto *Inst : InstsToDelete) {
    Inst->replaceAllUsesWith(UndefValue::get(Inst->getType()));
    Inst->eraseFromParent();
  }
}

// Functions for intrinsic name lookup.

// Does map lookup to find the atomic read/write and capture(swap) intrinsic
// name.
template <WRNAtomicKind AtomicKind,
          VPOParoptAtomics::AtomicCaptureKind CaptureKind>
const std::string
VPOParoptAtomics::getAtomicRWSIntrinsicName(const BasicBlock &BB,
                                            const Type &OpndTy) {
  assert((AtomicKind == WRNAtomicRead || AtomicKind == WRNAtomicWrite ||
          (AtomicKind == WRNAtomicCapture && CaptureKind == CaptureSwap)) &&
         "Unsupported AtomicKind for genAtomicRWSIntrinsicName");

  auto &MapToUse = (AtomicKind == WRNAtomicRead)
                       ? TypeToReadIntrinsicMap
                       : (AtomicKind == WRNAtomicCapture)
                             ? TypeToSwapIntrinsicMap
                             : TypeToWriteIntrinsicMap;

  const IntrinsicOperandTy Ty = {OpndTy.getTypeID(),
                                 OpndTy.getPrimitiveSizeInBits()};

  auto MapEntry = MapToUse.find(Ty);

  if (MapEntry == MapToUse.end()) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic not found.\n");
    return std::string();
  }

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic found: " << MapEntry->second
                    << "\n");
  return adjustIntrinsicNameForArchitecture(*BB.getParent(), MapEntry->second);
}

// Does map lookup to find the atomic update/non-swap capture intrinsic name.
template <WRNAtomicKind AtomicKind,
          VPOParoptAtomics::AtomicCaptureKind CaptureKind>
const std::string VPOParoptAtomics::getAtomicUCIntrinsicName(
    const Instruction &Operation, AtomicUpdateOp OpKind,
    bool Reversed, Type *AtomicOpndElemTy,
    const Value &ValueOpnd, bool IsTargetSPIRV) {

  assert((AtomicKind == WRNAtomicUpdate || ((AtomicKind == WRNAtomicCapture) &&
                                            (CaptureKind == CaptureBeforeOp ||
                                             CaptureKind == CaptureAfterOp))) &&
         "Unsupported AtomicKind for genAtomicUCIntrinsicName");
  assert((!Reversed || !Operation.isCommutative()) &&
         "Unexpected Reversed flag for commutative operation.");

  Type *ValueOpndType = ValueOpnd.getType();
  assert(AtomicOpndElemTy != nullptr && "AtomicOpndElemTy is null");
  assert(ValueOpndType != nullptr && "ValueOpndType is null");

  const Function *F = Operation.getFunction();

  if (IsTargetSPIRV &&
      ((!Enable64BitOpenCLAtomics ||
        // Do not use 64-bit atomics, if optimizations are not enabled.
        !F || F->hasFnAttribute(Attribute::OptimizeNone)) &&
       (AtomicOpndElemTy->getScalarSizeInBits() > 32 ||
        ValueOpndType->getScalarSizeInBits() > 32))) {
    // If 64-bit OpenCL atomics are not supported,
    // then we have to use critical section.
    //
    // TODO: stringify AtomicUpdateOp enum for debug output.
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic not found for "
               "atomic update "<< OpKind << ".\n");

    return std::string();
  }

  const bool AtomicUpdate = (AtomicKind == WRNAtomicUpdate);
  // No reversed atomic intrinsics for spir64 target.
  if (IsTargetSPIRV && Reversed)
    return std::string();
  // Select map table for atomic intrinsics
  auto &MapToUse =
      (IsTargetSPIRV
           ? (AtomicUpdate ? OpToUpdateIntrinsicForTgtMap
                           : OpToCaptureIntrinsicForTgtMap)
           : (AtomicUpdate ? (Reversed ? ReversedOpToUpdateIntrinsicMap
                                       : OpToUpdateIntrinsicMap)
                           : (Reversed ? ReversedOpToCaptureIntrinsicMap
                                       : OpToCaptureIntrinsicMap)));

  // We need the operand types and the operation's op code to do a map lookup.
  const IntrinsicOperandTy AtomicOpndTy = {
      AtomicOpndElemTy->getTypeID(),
      AtomicOpndElemTy->getPrimitiveSizeInBits()};
  const IntrinsicOperandTy ValueOpndTy = {
      ValueOpndType->getTypeID(), ValueOpndType->getPrimitiveSizeInBits()};

  // Create the key from op code and opnd types, and do the map lookup.
  const AtomicOperationTy OpTy = {OpKind, {AtomicOpndTy, ValueOpndTy}};
  auto MapEntry = MapToUse.find(OpTy);

  if (MapEntry == MapToUse.end()) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic not found for "
               "atomic update "<< OpKind << ".\n");
    return std::string();
  }

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic found: " << MapEntry->second
                    << "\n");
  return adjustIntrinsicNameForArchitecture(*F, MapEntry->second);
}

// Invokes appropriate functions to find the atomic capture intrinsic name.
const std::string VPOParoptAtomics::getAtomicCaptureIntrinsicName(
    AtomicCaptureKind CaptureKind, const BasicBlock *BB,
    const Instruction *Operation, AtomicUpdateOp OpKind,
    bool Reversed, Type *AtomicOpndElemTy,
    const Value *ValueOpnd, bool IsTargetSPIRV) {

  assert(CaptureKind != CaptureUnknown && "Unknown capture operation.");
  assert(BB != nullptr && "BB is null.");
  assert((CaptureKind == CaptureSwap || Operation != nullptr) &&
         "Need Operation for name lookup for CaptureBefore and CaptureAfter.");
  assert((CaptureKind == CaptureSwap || !Reversed ||
          !Operation->isCommutative()) &&
         "Reversed flag found true for commutative operation.");
  assert(AtomicOpndElemTy != nullptr && "AtomicOpndElemTy is null.");
  assert(ValueOpnd != nullptr && "ValueOpnd is null.");

  switch (CaptureKind) {
  case CaptureSwap:
    return getAtomicRWSIntrinsicName<WRNAtomicCapture, CaptureSwap>(
        *BB, *AtomicOpndElemTy);

  case CaptureBeforeOp:
    return getAtomicUCIntrinsicName<WRNAtomicCapture, CaptureBeforeOp>(
        *Operation, OpKind, Reversed, AtomicOpndElemTy, *ValueOpnd,
        IsTargetSPIRV);

  case CaptureAfterOp:
    return getAtomicUCIntrinsicName<WRNAtomicCapture, CaptureAfterOp>(
        *Operation, OpKind, Reversed, AtomicOpndElemTy, *ValueOpnd,
        IsTargetSPIRV);
  default:
    llvm_unreachable("Unknown capture kind in getAtomicCaptureIntrinsicName.");
  }
}

// Remove '_a16' from an intrinsic's name.
const std::string VPOParoptAtomics::adjustIntrinsicNameForArchitecture(
    const Function &F, const std::string &IntrinsicName) {

  assert(!IntrinsicName.empty() && "Intrinsic name is empty");

  // If IntrinsicName is not an '_a16' version, return as is.
  size_t A16StartingPosition = IntrinsicName.find("_a16");
  if (A16StartingPosition == std::string::npos)
    return IntrinsicName;

  // Otherwise, find the architecture.
  const Module *M = F.getParent();
  assert(M != nullptr && "Unable to find module.");

  // And if architecture is X86, return IntrinsicName as is.
  Triple TargetTriple(M->getTargetTriple());
  if (TargetTriple.getArch() == Triple::x86)
    return IntrinsicName;

  // If not, strip '_a16' from the intrinsic name.
  const std::string ArchAdjustedIntrinsicName =
      IntrinsicName.substr(0, A16StartingPosition) +
      IntrinsicName.substr(A16StartingPosition + 4);
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Adjusted intrinsic name: "
                    << ArchAdjustedIntrinsicName << "\n");
  return ArchAdjustedIntrinsicName;
}

// Initialize intrinsic maps

// TODO: Add calls for complexes when supported.
// NOTE: Read Intrinsic for a complex takes 3 args instead of two.
// TODO: Use different non '_a16` versions for float128 on x86.
// TODO: Use TableGen for maps.
const std::map<IntrinsicOperandTy, const std::string>
    VPOParoptAtomics::TypeToReadIntrinsicMap = {
        {I8, "__kmpc_atomic_fixed1_rd"},
        {I16, "__kmpc_atomic_fixed2_rd"},
        {I32, "__kmpc_atomic_fixed4_rd"},
        {P32, "__kmpc_atomic_fixed4_rd"},
        {I64, "__kmpc_atomic_fixed8_rd"},
        {P64, "__kmpc_atomic_fixed8_rd"},
        {F32, "__kmpc_atomic_float4_rd"},
        {F64, "__kmpc_atomic_float8_rd"},
        {F80, "__kmpc_atomic_float10_rd"},
        {F128, "__kmpc_atomic_float16_a16_rd"}};

const std::map<IntrinsicOperandTy, const std::string>
    VPOParoptAtomics::TypeToWriteIntrinsicMap = {
        {I8, "__kmpc_atomic_fixed1_wr"},
        {I16, "__kmpc_atomic_fixed2_wr"},
        {I32, "__kmpc_atomic_fixed4_wr"},
        {P32, "__kmpc_atomic_fixed4_wr"},
        {I64, "__kmpc_atomic_fixed8_wr"},
        {P64, "__kmpc_atomic_fixed8_wr"},
        {F32, "__kmpc_atomic_float4_wr"},
        {F64, "__kmpc_atomic_float8_wr"},
        {F80, "__kmpc_atomic_float10_wr"},
        {F128, "__kmpc_atomic_float16_a16_wr"}};

const std::map<IntrinsicOperandTy, const std::string>
    VPOParoptAtomics::TypeToSwapIntrinsicMap = {
        {I8, "__kmpc_atomic_fixed1_swp"},
        {I16, "__kmpc_atomic_fixed2_swp"},
        {I32, "__kmpc_atomic_fixed4_swp"},
        {I64, "__kmpc_atomic_fixed8_swp"},
        {F32, "__kmpc_atomic_float4_swp"},
        {F64, "__kmpc_atomic_float8_swp"},
        {F80, "__kmpc_atomic_float10_swp"},
        {F128, "__kmpc_atomic_float16_a16_swp"}};

// Note: Fortran supports intrinsic calls: max, min, ior, ieor as well as
// logical .and., .or., .neqv. and .eqv. operations.
// Note: The entries for si = si/float and ui = ui/float are distinguished by
// using AtomicUpdateFDiv for the first case, and AtomicUpdateUDiv for the
// second.
const std::map<VPOParoptAtomics::AtomicOperationTy, const std::string>
    VPOParoptAtomics::OpToUpdateIntrinsicMap = {
        // I8 = I8 op I8
        {{AtomicUpdateAdd, {I8, I8}}, "__kmpc_atomic_fixed1_add"},
        {{AtomicUpdateSub, {I8, I8}}, "__kmpc_atomic_fixed1_sub"},
        {{AtomicUpdateMul, {I8, I8}}, "__kmpc_atomic_fixed1_mul"},
        {{AtomicUpdateSDiv, {I8, I8}}, "__kmpc_atomic_fixed1_div"},
        {{AtomicUpdateUDiv, {I8, I8}}, "__kmpc_atomic_fixed1u_div"},
        {{AtomicUpdateAnd, {I8, I8}}, "__kmpc_atomic_fixed1_andb"},
        {{AtomicUpdateOr, {I8, I8}}, "__kmpc_atomic_fixed1_orb"},
        {{AtomicUpdateXor, {I8, I8}}, "__kmpc_atomic_fixed1_xor"},
        {{AtomicUpdateShl, {I8, I8}}, "__kmpc_atomic_fixed1_shl"},
        {{AtomicUpdateAShr, {I8, I8}}, "__kmpc_atomic_fixed1_shr"},
        {{AtomicUpdateLShr, {I8, I8}}, "__kmpc_atomic_fixed1u_shr"},
        // There are no unsigned versions for min/max
        // in the host OpenMP library.
        {{AtomicUpdateSMax, {I8, I8}}, "__kmpc_atomic_fixed1_max"},
        {{AtomicUpdateSMin, {I8, I8}}, "__kmpc_atomic_fixed1_min"},
        // I16 = I16 op I16
        {{AtomicUpdateAdd, {I16, I16}}, "__kmpc_atomic_fixed2_add"},
        {{AtomicUpdateSub, {I16, I16}}, "__kmpc_atomic_fixed2_sub"},
        {{AtomicUpdateMul, {I16, I16}}, "__kmpc_atomic_fixed2_mul"},
        {{AtomicUpdateSDiv, {I16, I16}}, "__kmpc_atomic_fixed2_div"},
        {{AtomicUpdateUDiv, {I16, I16}}, "__kmpc_atomic_fixed2u_div"},
        {{AtomicUpdateAnd, {I16, I16}}, "__kmpc_atomic_fixed2_andb"},
        {{AtomicUpdateOr, {I16, I16}}, "__kmpc_atomic_fixed2_orb"},
        {{AtomicUpdateXor, {I16, I16}}, "__kmpc_atomic_fixed2_xor"},
        {{AtomicUpdateShl, {I16, I16}}, "__kmpc_atomic_fixed2_shl"},
        {{AtomicUpdateAShr, {I16, I16}}, "__kmpc_atomic_fixed2_shr"},
        {{AtomicUpdateLShr, {I16, I16}}, "__kmpc_atomic_fixed2u_shr"},
        {{AtomicUpdateSMax, {I16, I16}}, "__kmpc_atomic_fixed2_max"},
        {{AtomicUpdateSMin, {I16, I16}}, "__kmpc_atomic_fixed2_min"},
        // I32 = I32 op I32
        {{AtomicUpdateAdd, {I32, I32}}, "__kmpc_atomic_fixed4_add"},
        {{AtomicUpdateSub, {I32, I32}}, "__kmpc_atomic_fixed4_sub"},
        {{AtomicUpdateMul, {I32, I32}}, "__kmpc_atomic_fixed4_mul"},
        {{AtomicUpdateSDiv, {I32, I32}}, "__kmpc_atomic_fixed4_div"},
        {{AtomicUpdateUDiv, {I32, I32}}, "__kmpc_atomic_fixed4u_div"},
        {{AtomicUpdateAnd, {I32, I32}}, "__kmpc_atomic_fixed4_andb"},
        {{AtomicUpdateOr, {I32, I32}}, "__kmpc_atomic_fixed4_orb"},
        {{AtomicUpdateXor, {I32, I32}}, "__kmpc_atomic_fixed4_xor"},
        {{AtomicUpdateShl, {I32, I32}}, "__kmpc_atomic_fixed4_shl"},
        {{AtomicUpdateAShr, {I32, I32}}, "__kmpc_atomic_fixed4_shr"},
        {{AtomicUpdateLShr, {I32, I32}}, "__kmpc_atomic_fixed4u_shr"},
        {{AtomicUpdateSMax, {I32, I32}}, "__kmpc_atomic_fixed4_max"},
        {{AtomicUpdateSMin, {I32, I32}}, "__kmpc_atomic_fixed4_min"},
        // I64 = I64 op I64
        {{AtomicUpdateAdd, {I64, I64}}, "__kmpc_atomic_fixed8_add"},
        {{AtomicUpdateSub, {I64, I64}}, "__kmpc_atomic_fixed8_sub"},
        {{AtomicUpdateMul, {I64, I64}}, "__kmpc_atomic_fixed8_mul"},
        {{AtomicUpdateSDiv, {I64, I64}}, "__kmpc_atomic_fixed8_div"},
        {{AtomicUpdateUDiv, {I64, I64}}, "__kmpc_atomic_fixed8u_div"},
        {{AtomicUpdateAnd, {I64, I64}}, "__kmpc_atomic_fixed8_andb"},
        {{AtomicUpdateOr, {I64, I64}}, "__kmpc_atomic_fixed8_orb"},
        {{AtomicUpdateXor, {I64, I64}}, "__kmpc_atomic_fixed8_xor"},
        {{AtomicUpdateShl, {I64, I64}}, "__kmpc_atomic_fixed8_shl"},
        {{AtomicUpdateAShr, {I64, I64}}, "__kmpc_atomic_fixed8_shr"},
        {{AtomicUpdateLShr, {I64, I64}}, "__kmpc_atomic_fixed8u_shr"},
        {{AtomicUpdateSMax, {I64, I64}}, "__kmpc_atomic_fixed8_max"},
        {{AtomicUpdateSMin, {I64, I64}}, "__kmpc_atomic_fixed8_min"},
        // I8 = I8 op F64
        {{AtomicUpdateFMul, {I8, F64}}, "__kmpc_atomic_fixed1_mul_float8"},
        {{AtomicUpdateFDiv, {I8, F64}}, "__kmpc_atomic_fixed1_div_float8"},
        // I16 = I16 op F64
        {{AtomicUpdateFMul, {I16, F64}}, "__kmpc_atomic_fixed2_mul_float8"},
        {{AtomicUpdateFDiv, {I16, F64}}, "__kmpc_atomic_fixed2_div_float8"},
        // I32 = I32 op F64
        {{AtomicUpdateFMul, {I32, F64}}, "__kmpc_atomic_fixed4_mul_float8"},
        {{AtomicUpdateFDiv, {I32, F64}}, "__kmpc_atomic_fixed4_div_float8"},
        // I64 = I64 op F64
        {{AtomicUpdateFMul, {I64, F64}}, "__kmpc_atomic_fixed8_mul_float8"},
        {{AtomicUpdateFDiv, {I64, F64}}, "__kmpc_atomic_fixed8_div_float8"},
        // I8 = I8 op F128
        {{AtomicUpdateFAdd, {I8, F128}}, "__kmpc_atomic_fixed1_add_fp"},
        {{AtomicUpdateFSub, {I8, F128}}, "__kmpc_atomic_fixed1_sub_fp"},
        {{AtomicUpdateFMul, {I8, F128}}, "__kmpc_atomic_fixed1_mul_fp"},
        {{AtomicUpdateMul,  {I8, F128}}, "__kmpc_atomic_fixed1u_mul_fp"},
        {{AtomicUpdateFDiv, {I8, F128}}, "__kmpc_atomic_fixed1_div_fp"},
        {{AtomicUpdateUDiv, {I8, F128}}, "__kmpc_atomic_fixed1u_div_fp"},
        // I16 = I16 op F128
        {{AtomicUpdateFAdd, {I16, F128}}, "__kmpc_atomic_fixed2_add_fp"},
        {{AtomicUpdateFSub, {I16, F128}}, "__kmpc_atomic_fixed2_sub_fp"},
        {{AtomicUpdateFMul, {I16, F128}}, "__kmpc_atomic_fixed2_mul_fp"},
        {{AtomicUpdateMul,  {I16, F128}}, "__kmpc_atomic_fixed2u_mul_fp"},
        {{AtomicUpdateFDiv, {I16, F128}}, "__kmpc_atomic_fixed2_div_fp"},
        {{AtomicUpdateUDiv, {I16, F128}}, "__kmpc_atomic_fixed2u_div_fp"},
        // I32 = I32 op F128
        {{AtomicUpdateFAdd, {I32, F128}}, "__kmpc_atomic_fixed4_add_fp"},
        {{AtomicUpdateFSub, {I32, F128}}, "__kmpc_atomic_fixed4_sub_fp"},
        {{AtomicUpdateFMul, {I32, F128}}, "__kmpc_atomic_fixed4_mul_fp"},
        {{AtomicUpdateMul,  {I32, F128}}, "__kmpc_atomic_fixed4u_mul_fp"},
        {{AtomicUpdateFDiv, {I32, F128}}, "__kmpc_atomic_fixed4_div_fp"},
        {{AtomicUpdateUDiv, {I32, F128}}, "__kmpc_atomic_fixed4u_div_fp"},
        // I64 = I64 op F128
        {{AtomicUpdateFAdd, {I64, F128}}, "__kmpc_atomic_fixed8_add_fp"},
        {{AtomicUpdateFSub, {I64, F128}}, "__kmpc_atomic_fixed8_sub_fp"},
        {{AtomicUpdateFMul, {I64, F128}}, "__kmpc_atomic_fixed8_mul_fp"},
        {{AtomicUpdateMul,  {I64, F128}}, "__kmpc_atomic_fixed8u_mul_fp"},
        {{AtomicUpdateFDiv, {I64, F128}}, "__kmpc_atomic_fixed8_div_fp"},
        {{AtomicUpdateUDiv, {I64, F128}}, "__kmpc_atomic_fixed8u_div_fp"},
        // F32 = F32 op F32
        {{AtomicUpdateFAdd, {F32, F32}}, "__kmpc_atomic_float4_add"},
        {{AtomicUpdateFSub, {F32, F32}}, "__kmpc_atomic_float4_sub"},
        {{AtomicUpdateFMul, {F32, F32}}, "__kmpc_atomic_float4_mul"},
        {{AtomicUpdateFDiv, {F32, F32}}, "__kmpc_atomic_float4_div"},
        {{AtomicUpdateFMax, {F32, F32}}, "__kmpc_atomic_float4_max"},
        {{AtomicUpdateFMin, {F32, F32}}, "__kmpc_atomic_float4_min"},
        // F64 = F64 op F64
        {{AtomicUpdateFAdd, {F64, F64}}, "__kmpc_atomic_float8_add"},
        {{AtomicUpdateFSub, {F64, F64}}, "__kmpc_atomic_float8_sub"},
        {{AtomicUpdateFMul, {F64, F64}}, "__kmpc_atomic_float8_mul"},
        {{AtomicUpdateFDiv, {F64, F64}}, "__kmpc_atomic_float8_div"},
        {{AtomicUpdateFMax, {F64, F64}}, "__kmpc_atomic_float8_max"},
        {{AtomicUpdateFMin, {F64, F64}}, "__kmpc_atomic_float8_min"},
        // F80 = F80 op F80
        {{AtomicUpdateFAdd, {F80, F80}}, "__kmpc_atomic_float10_add"},
        {{AtomicUpdateFSub, {F80, F80}}, "__kmpc_atomic_float10_sub"},
        {{AtomicUpdateFMul, {F80, F80}}, "__kmpc_atomic_float10_mul"},
        {{AtomicUpdateFDiv, {F80, F80}}, "__kmpc_atomic_float10_div"},
        // F128 = F128 op F128
        // TODO: Make sure using _a16 version is correct
        {{AtomicUpdateFAdd, {F128, F128}}, "__kmpc_atomic_float16_add_a16"},
        {{AtomicUpdateFSub, {F128, F128}}, "__kmpc_atomic_float16_sub_a16"},
        {{AtomicUpdateFMul, {F128, F128}}, "__kmpc_atomic_float16_mul_a16"},
        {{AtomicUpdateFDiv, {F128, F128}}, "__kmpc_atomic_float16_div_a16"},
        // F32 = F32 op F64
        {{AtomicUpdateFAdd, {F32, F64}}, "__kmpc_atomic_float4_add_float8"},
        {{AtomicUpdateFSub, {F32, F64}}, "__kmpc_atomic_float4_sub_float8"},
        {{AtomicUpdateFMul, {F32, F64}}, "__kmpc_atomic_float4_mul_float8"},
        {{AtomicUpdateFDiv, {F32, F64}}, "__kmpc_atomic_float4_div_float8"}
#if 0 // NOT Emitted in ICC
        // F32 = F32 op F128
        ,{{AtomicUpdateFAdd, {F32, F128}}, "__kmpc_atomic_float4_add_fp"},
        {{AtomicUpdateFSub, {F32, F128}}, "__kmpc_atomic_float4_sub_fp"},
        {{AtomicUpdateFMul, {F32, F128}}, "__kmpc_atomic_float4_mul_fp"},
        {{AtomicUpdateFDiv, {F32, F128}}, "__kmpc_atomic_float4_div_fp"},
        // F64 = F64 op F128
        {{AtomicUpdateFAdd, {F64, F128}}, "__kmpc_atomic_float8_add_fp"},
        {{AtomicUpdateFSub, {F64, F128}}, "__kmpc_atomic_float8_sub_fp"},
        {{AtomicUpdateFMul, {F64, F128}}, "__kmpc_atomic_float8_mul_fp"},
        {{AtomicUpdateFDiv, {F64, F128}}, "__kmpc_atomic_float8_div_fp"},
        // F80 = F80 op F128
        {{AtomicUpdateFAdd, {F80, F128}}, "__kmpc_atomic_float10_add_fp"},
        {{AtomicUpdateFSub, {F80, F128}}, "__kmpc_atomic_float10_sub_fp"},
        {{AtomicUpdateFMul, {F80, F128}}, "__kmpc_atomic_float10_mul_fp"},
        {{AtomicUpdateFDiv, {F80, F128}}, "__kmpc_atomic_float10_div_fp"}
#endif
};

const std::map<VPOParoptAtomics::AtomicOperationTy, const std::string>
    VPOParoptAtomics::OpToUpdateIntrinsicForTgtMap = {
        // I32 = I32 op I32
        {{AtomicUpdateAdd, {I32, I32}}, "__kmpc_atomic_fixed4_add"},
        {{AtomicUpdateSub, {I32, I32}}, "__kmpc_atomic_fixed4_sub"},
        {{AtomicUpdateMul, {I32, I32}}, "__kmpc_atomic_fixed4_mul"},
        {{AtomicUpdateSDiv, {I32, I32}}, "__kmpc_atomic_fixed4_div"},
        {{AtomicUpdateUDiv, {I32, I32}}, "__kmpc_atomic_fixed4u_div"},
        {{AtomicUpdateAnd, {I32, I32}}, "__kmpc_atomic_fixed4_andb"},
        {{AtomicUpdateOr, {I32, I32}}, "__kmpc_atomic_fixed4_orb"},
        {{AtomicUpdateXor, {I32, I32}}, "__kmpc_atomic_fixed4_xor"},
        {{AtomicUpdateShl, {I32, I32}}, "__kmpc_atomic_fixed4_shl"},
        {{AtomicUpdateAShr, {I32, I32}}, "__kmpc_atomic_fixed4_shr"},
        {{AtomicUpdateLShr, {I32, I32}}, "__kmpc_atomic_fixed4u_shr"},
        {{AtomicUpdateSMax, {I32, I32}}, "__kmpc_atomic_fixed4_max"},
        {{AtomicUpdateUMax, {I32, I32}}, "__kmpc_atomic_fixed4u_max"},
        {{AtomicUpdateSMin, {I32, I32}}, "__kmpc_atomic_fixed4_min"},
        {{AtomicUpdateUMin, {I32, I32}}, "__kmpc_atomic_fixed4u_min"},
        // I64 = I64 op I64
        {{AtomicUpdateAdd, {I64, I64}}, "__kmpc_atomic_fixed8_add"},
        {{AtomicUpdateSub, {I64, I64}}, "__kmpc_atomic_fixed8_sub"},
        {{AtomicUpdateMul, {I64, I64}}, "__kmpc_atomic_fixed8_mul"},
        {{AtomicUpdateSDiv, {I64, I64}}, "__kmpc_atomic_fixed8_div"},
        {{AtomicUpdateUDiv, {I64, I64}}, "__kmpc_atomic_fixed8u_div"},
        {{AtomicUpdateAnd, {I64, I64}}, "__kmpc_atomic_fixed8_andb"},
        {{AtomicUpdateOr, {I64, I64}}, "__kmpc_atomic_fixed8_orb"},
        {{AtomicUpdateXor, {I64, I64}}, "__kmpc_atomic_fixed8_xor"},
        {{AtomicUpdateShl, {I64, I64}}, "__kmpc_atomic_fixed8_shl"},
        {{AtomicUpdateAShr, {I64, I64}}, "__kmpc_atomic_fixed8_shr"},
        {{AtomicUpdateLShr, {I64, I64}}, "__kmpc_atomic_fixed8u_shr"},
        {{AtomicUpdateSMax, {I64, I64}}, "__kmpc_atomic_fixed8_max"},
        {{AtomicUpdateUMax, {I64, I64}}, "__kmpc_atomic_fixed8u_max"},
        {{AtomicUpdateSMin, {I64, I64}}, "__kmpc_atomic_fixed8_min"},
        {{AtomicUpdateUMin, {I64, I64}}, "__kmpc_atomic_fixed8u_min"},
        // F32 = F32 op F32
        {{AtomicUpdateFAdd, {F32, F32}}, "__kmpc_atomic_float4_add"},
        {{AtomicUpdateFSub, {F32, F32}}, "__kmpc_atomic_float4_sub"},
        {{AtomicUpdateFMul, {F32, F32}}, "__kmpc_atomic_float4_mul"},
        {{AtomicUpdateFDiv, {F32, F32}}, "__kmpc_atomic_float4_div"},
        {{AtomicUpdateFMax, {F32, F32}}, "__kmpc_atomic_float4_max"},
        {{AtomicUpdateFMin, {F32, F32}}, "__kmpc_atomic_float4_min"},
        // F64 = F64 op F64
        {{AtomicUpdateFAdd, {F64, F64}}, "__kmpc_atomic_float8_add"},
        {{AtomicUpdateFSub, {F64, F64}}, "__kmpc_atomic_float8_sub"},
        {{AtomicUpdateFMul, {F64, F64}}, "__kmpc_atomic_float8_mul"},
        {{AtomicUpdateFDiv, {F64, F64}}, "__kmpc_atomic_float8_div"},
        {{AtomicUpdateFMax, {F64, F64}}, "__kmpc_atomic_float8_max"},
        {{AtomicUpdateFMin, {F64, F64}}, "__kmpc_atomic_float8_min"},
};

const std::map<VPOParoptAtomics::AtomicOperationTy, const std::string>
    VPOParoptAtomics::ReversedOpToUpdateIntrinsicMap = {
        // I8 = I8 op I8
        {{AtomicUpdateSub, {I8, I8}}, "__kmpc_atomic_fixed1_sub_rev"},
        {{AtomicUpdateSDiv, {I8, I8}}, "__kmpc_atomic_fixed1_div_rev"},
        {{AtomicUpdateUDiv, {I8, I8}}, "__kmpc_atomic_fixed1u_div_rev"},
        {{AtomicUpdateShl, {I8, I8}}, "__kmpc_atomic_fixed1_shl_rev"},
        {{AtomicUpdateAShr, {I8, I8}}, "__kmpc_atomic_fixed1_shr_rev"},
        {{AtomicUpdateLShr, {I8, I8}}, "__kmpc_atomic_fixed1u_shr_rev"},
        // I16 = I16 op I16
        {{AtomicUpdateSub, {I16, I16}}, "__kmpc_atomic_fixed2_sub_rev"},
        {{AtomicUpdateSDiv, {I16, I16}}, "__kmpc_atomic_fixed2_div_rev"},
        {{AtomicUpdateUDiv, {I16, I16}}, "__kmpc_atomic_fixed2u_div_rev"},
        {{AtomicUpdateShl, {I16, I16}}, "__kmpc_atomic_fixed2_shl_rev"},
        {{AtomicUpdateAShr, {I16, I16}}, "__kmpc_atomic_fixed2_shr_rev"},
        {{AtomicUpdateLShr, {I16, I16}}, "__kmpc_atomic_fixed2u_shr_rev"},
        // I32 = I32 op I32
        {{AtomicUpdateSub, {I32, I32}}, "__kmpc_atomic_fixed4_sub_rev"},
        {{AtomicUpdateSDiv, {I32, I32}}, "__kmpc_atomic_fixed4_div_rev"},
        {{AtomicUpdateUDiv, {I32, I32}}, "__kmpc_atomic_fixed4u_div_rev"},
        {{AtomicUpdateShl, {I32, I32}}, "__kmpc_atomic_fixed4_shl_rev"},
        {{AtomicUpdateAShr, {I32, I32}}, "__kmpc_atomic_fixed4_shr_rev"},
        {{AtomicUpdateLShr, {I32, I32}}, "__kmpc_atomic_fixed4u_shr_rev"},
        // I64 = I64 op I64
        {{AtomicUpdateSub, {I64, I64}}, "__kmpc_atomic_fixed8_sub_rev"},
        {{AtomicUpdateSDiv, {I64, I64}}, "__kmpc_atomic_fixed8_div_rev"},
        {{AtomicUpdateUDiv, {I64, I64}}, "__kmpc_atomic_fixed8u_div_rev"},
        {{AtomicUpdateShl, {I64, I64}}, "__kmpc_atomic_fixed8_shl_rev"},
        {{AtomicUpdateAShr, {I64, I64}}, "__kmpc_atomic_fixed8_shr_rev"},
        {{AtomicUpdateLShr, {I64, I64}}, "__kmpc_atomic_fixed8u_shr_rev"},
        // I8 = F128 op I8
        {{AtomicUpdateFSub, {I8, F128}}, "__kmpc_atomic_fixed1_sub_rev_fp"},
        {{AtomicUpdateFDiv, {I8, F128}}, "__kmpc_atomic_fixed1_div_rev_fp"},
        {{AtomicUpdateUDiv, {I8, F128}}, "__kmpc_atomic_fixed1u_div_rev_fp"},
        // I16 = F128 op I16
        {{AtomicUpdateFSub, {I16, F128}}, "__kmpc_atomic_fixed2_sub_rev_fp"},
        {{AtomicUpdateFDiv, {I16, F128}}, "__kmpc_atomic_fixed2_div_rev_fp"},
        {{AtomicUpdateUDiv, {I16, F128}}, "__kmpc_atomic_fixed2u_div_rev_fp"},
        // I32 = F128 op I32
        {{AtomicUpdateFSub, {I32, F128}}, "__kmpc_atomic_fixed4_sub_rev_fp"},
        {{AtomicUpdateFDiv, {I32, F128}}, "__kmpc_atomic_fixed4_div_rev_fp"},
        {{AtomicUpdateUDiv, {I32, F128}}, "__kmpc_atomic_fixed4u_div_rev_fp"},
        // I64 = F128 op I64
        {{AtomicUpdateFSub, {I64, F128}}, "__kmpc_atomic_fixed8_sub_rev_fp"},
        {{AtomicUpdateFDiv, {I64, F128}}, "__kmpc_atomic_fixed8_div_rev_fp"},
        {{AtomicUpdateUDiv, {I64, F128}}, "__kmpc_atomic_fixed8u_div_rev_fp"},
        // F32 = F32 op F32
        {{AtomicUpdateFSub, {F32, F32}}, "__kmpc_atomic_float4_sub_rev"},
        {{AtomicUpdateFDiv, {F32, F32}}, "__kmpc_atomic_float4_div_rev"},
        // F64 = F64 op F64
        {{AtomicUpdateFSub, {F64, F64}}, "__kmpc_atomic_float8_sub_rev"},
        {{AtomicUpdateFDiv, {F64, F64}}, "__kmpc_atomic_float8_div_rev"},
        // F80 = F80 op F80
        {{AtomicUpdateFSub, {F80, F80}}, "__kmpc_atomic_float10_sub_rev"},
        {{AtomicUpdateFDiv, {F80, F80}}, "__kmpc_atomic_float10_div_rev"},
        // F128 = F128 op F128
        {{AtomicUpdateFSub, {F128, F128}},
         "__kmpc_atomic_float16_sub_a16_rev"},
        {{AtomicUpdateFDiv, {F128, F128}},
         "__kmpc_atomic_float16_div_a16_rev"}};

// Note: The entries for si = si/float and ui = ui/float are distinguished by
// using AtomicUpdateFDiv for the first case, and AtomicUpdateUDiv for the
// second.
const std::map<VPOParoptAtomics::AtomicOperationTy, const std::string>
    VPOParoptAtomics::OpToCaptureIntrinsicMap = {
        // I8 = I8 op I8
        {{AtomicUpdateAdd, {I8, I8}}, "__kmpc_atomic_fixed1_add_cpt"},
        {{AtomicUpdateSub, {I8, I8}}, "__kmpc_atomic_fixed1_sub_cpt"},
        {{AtomicUpdateMul, {I8, I8}}, "__kmpc_atomic_fixed1_mul_cpt"},
        {{AtomicUpdateSDiv, {I8, I8}}, "__kmpc_atomic_fixed1_div_cpt"},
        {{AtomicUpdateUDiv, {I8, I8}}, "__kmpc_atomic_fixed1u_div_cpt"},
        {{AtomicUpdateAnd, {I8, I8}}, "__kmpc_atomic_fixed1_andb_cpt"},
        {{AtomicUpdateOr, {I8, I8}}, "__kmpc_atomic_fixed1_orb_cpt"},
        {{AtomicUpdateXor, {I8, I8}}, "__kmpc_atomic_fixed1_xor_cpt"},
        {{AtomicUpdateShl, {I8, I8}}, "__kmpc_atomic_fixed1_shl_cpt"},
        {{AtomicUpdateAShr, {I8, I8}}, "__kmpc_atomic_fixed1_shr_cpt"},
        {{AtomicUpdateLShr, {I8, I8}}, "__kmpc_atomic_fixed1u_shr_cpt"},
        {{AtomicUpdateSMax, {I8, I8}}, "__kmpc_atomic_fixed1_max_cpt"},
        {{AtomicUpdateSMin, {I8, I8}}, "__kmpc_atomic_fixed1_min_cpt"},
        // I16 = I16 op I16
        {{AtomicUpdateAdd, {I16, I16}}, "__kmpc_atomic_fixed2_add_cpt"},
        {{AtomicUpdateSub, {I16, I16}}, "__kmpc_atomic_fixed2_sub_cpt"},
        {{AtomicUpdateMul, {I16, I16}}, "__kmpc_atomic_fixed2_mul_cpt"},
        {{AtomicUpdateSDiv, {I16, I16}}, "__kmpc_atomic_fixed2_div_cpt"},
        {{AtomicUpdateUDiv, {I16, I16}}, "__kmpc_atomic_fixed2u_div_cpt"},
        {{AtomicUpdateAnd, {I16, I16}}, "__kmpc_atomic_fixed2_andb_cpt"},
        {{AtomicUpdateOr, {I16, I16}}, "__kmpc_atomic_fixed2_orb_cpt"},
        {{AtomicUpdateXor, {I16, I16}}, "__kmpc_atomic_fixed2_xor_cpt"},
        {{AtomicUpdateShl, {I16, I16}}, "__kmpc_atomic_fixed2_shl_cpt"},
        {{AtomicUpdateAShr, {I16, I16}}, "__kmpc_atomic_fixed2_shr_cpt"},
        {{AtomicUpdateLShr, {I16, I16}}, "__kmpc_atomic_fixed2u_shr_cpt"},
        {{AtomicUpdateSMax, {I16, I16}}, "__kmpc_atomic_fixed2_max_cpt"},
        {{AtomicUpdateSMin, {I16, I16}}, "__kmpc_atomic_fixed2_min_cpt"},
        // I32 = I32 op I32
        {{AtomicUpdateAdd, {I32, I32}}, "__kmpc_atomic_fixed4_add_cpt"},
        {{AtomicUpdateSub, {I32, I32}}, "__kmpc_atomic_fixed4_sub_cpt"},
        {{AtomicUpdateMul, {I32, I32}}, "__kmpc_atomic_fixed4_mul_cpt"},
        {{AtomicUpdateSDiv, {I32, I32}}, "__kmpc_atomic_fixed4_div_cpt"},
        {{AtomicUpdateUDiv, {I32, I32}}, "__kmpc_atomic_fixed4u_div_cpt"},
        {{AtomicUpdateAnd, {I32, I32}}, "__kmpc_atomic_fixed4_andb_cpt"},
        {{AtomicUpdateOr, {I32, I32}}, "__kmpc_atomic_fixed4_orb_cpt"},
        {{AtomicUpdateXor, {I32, I32}}, "__kmpc_atomic_fixed4_xor_cpt"},
        {{AtomicUpdateShl, {I32, I32}}, "__kmpc_atomic_fixed4_shl_cpt"},
        {{AtomicUpdateAShr, {I32, I32}}, "__kmpc_atomic_fixed4_shr_cpt"},
        {{AtomicUpdateLShr, {I32, I32}}, "__kmpc_atomic_fixed4u_shr_cpt"},
        {{AtomicUpdateSMax, {I32, I32}}, "__kmpc_atomic_fixed4_max_cpt"},
        {{AtomicUpdateSMin, {I32, I32}}, "__kmpc_atomic_fixed4_min_cpt"},
        // I64 = I64 op I64
        {{AtomicUpdateAdd, {I64, I64}}, "__kmpc_atomic_fixed8_add_cpt"},
        {{AtomicUpdateSub, {I64, I64}}, "__kmpc_atomic_fixed8_sub_cpt"},
        {{AtomicUpdateMul, {I64, I64}}, "__kmpc_atomic_fixed8_mul_cpt"},
        {{AtomicUpdateSDiv, {I64, I64}}, "__kmpc_atomic_fixed8_div_cpt"},
        {{AtomicUpdateUDiv, {I64, I64}}, "__kmpc_atomic_fixed8u_div_cpt"},
        {{AtomicUpdateAnd, {I64, I64}}, "__kmpc_atomic_fixed8_andb_cpt"},
        {{AtomicUpdateOr, {I64, I64}}, "__kmpc_atomic_fixed8_orb_cpt"},
        {{AtomicUpdateXor, {I64, I64}}, "__kmpc_atomic_fixed8_xor_cpt"},
        {{AtomicUpdateShl, {I64, I64}}, "__kmpc_atomic_fixed8_shl_cpt"},
        {{AtomicUpdateAShr, {I64, I64}}, "__kmpc_atomic_fixed8_shr_cpt"},
        {{AtomicUpdateLShr, {I64, I64}}, "__kmpc_atomic_fixed8u_shr_cpt"},
        {{AtomicUpdateSMax, {I64, I64}}, "__kmpc_atomic_fixed8_max_cpt"},
        {{AtomicUpdateSMin, {I64, I64}}, "__kmpc_atomic_fixed8_min_cpt"},
        // I8 = I8 op F128
        {{AtomicUpdateFAdd, {I8, F128}}, "__kmpc_atomic_fixed1_add_cpt_fp"},
        {{AtomicUpdateFSub, {I8, F128}}, "__kmpc_atomic_fixed1_sub_cpt_fp"},
        {{AtomicUpdateFMul, {I8, F128}}, "__kmpc_atomic_fixed1_mul_cpt_fp"},
        {{AtomicUpdateMul,  {I8, F128}}, "__kmpc_atomic_fixed1u_mul_cpt_fp"},
        {{AtomicUpdateFDiv, {I8, F128}}, "__kmpc_atomic_fixed1_div_cpt_fp"},
        {{AtomicUpdateUDiv, {I8, F128}}, "__kmpc_atomic_fixed1u_div_cpt_fp"},
        // I16 = I16 op F128
        {{AtomicUpdateFAdd, {I16, F128}}, "__kmpc_atomic_fixed2_add_cpt_fp"},
        {{AtomicUpdateFSub, {I16, F128}}, "__kmpc_atomic_fixed2_sub_cpt_fp"},
        {{AtomicUpdateFMul, {I16, F128}}, "__kmpc_atomic_fixed2_mul_cpt_fp"},
        {{AtomicUpdateMul,  {I16, F128}}, "__kmpc_atomic_fixed2u_mul_cpt_fp"},
        {{AtomicUpdateFDiv, {I16, F128}}, "__kmpc_atomic_fixed2_div_cpt_fp"},
        {{AtomicUpdateUDiv, {I16, F128}}, "__kmpc_atomic_fixed2u_div_cpt_fp"},
        // I32 = I32 op F128
        {{AtomicUpdateFAdd, {I32, F128}}, "__kmpc_atomic_fixed4_add_cpt_fp"},
        {{AtomicUpdateFSub, {I32, F128}}, "__kmpc_atomic_fixed4_sub_cpt_fp"},
        {{AtomicUpdateFMul, {I32, F128}}, "__kmpc_atomic_fixed4_mul_cpt_fp"},
        {{AtomicUpdateMul,  {I32, F128}}, "__kmpc_atomic_fixed4u_mul_cpt_fp"},
        {{AtomicUpdateFDiv, {I32, F128}}, "__kmpc_atomic_fixed4_div_cpt_fp"},
        {{AtomicUpdateUDiv, {I32, F128}}, "__kmpc_atomic_fixed4u_div_cpt_fp"},
        // I64 = I64 op F128
        {{AtomicUpdateFAdd, {I64, F128}}, "__kmpc_atomic_fixed8_add_cpt_fp"},
        {{AtomicUpdateFSub, {I64, F128}}, "__kmpc_atomic_fixed8_sub_cpt_fp"},
        {{AtomicUpdateFMul, {I64, F128}}, "__kmpc_atomic_fixed8_mul_cpt_fp"},
        {{AtomicUpdateMul,  {I64, F128}}, "__kmpc_atomic_fixed8u_mul_cpt_fp"},
        {{AtomicUpdateFDiv, {I64, F128}}, "__kmpc_atomic_fixed8_div_cpt_fp"},
        {{AtomicUpdateUDiv, {I64, F128}}, "__kmpc_atomic_fixed8u_div_cpt_fp"},
        // F32 = F32 op F32
        {{AtomicUpdateFAdd, {F32, F32}}, "__kmpc_atomic_float4_add_cpt"},
        {{AtomicUpdateFSub, {F32, F32}}, "__kmpc_atomic_float4_sub_cpt"},
        {{AtomicUpdateFMul, {F32, F32}}, "__kmpc_atomic_float4_mul_cpt"},
        {{AtomicUpdateFDiv, {F32, F32}}, "__kmpc_atomic_float4_div_cpt"},
        {{AtomicUpdateFMax, {F32, F32}}, "__kmpc_atomic_float4_max_cpt"},
        {{AtomicUpdateFMin, {F32, F32}}, "__kmpc_atomic_float4_min_cpt"},
        // F64 = F64 op F64
        {{AtomicUpdateFAdd, {F64, F64}}, "__kmpc_atomic_float8_add_cpt"},
        {{AtomicUpdateFSub, {F64, F64}}, "__kmpc_atomic_float8_sub_cpt"},
        {{AtomicUpdateFMul, {F64, F64}}, "__kmpc_atomic_float8_mul_cpt"},
        {{AtomicUpdateFDiv, {F64, F64}}, "__kmpc_atomic_float8_div_cpt"},
        {{AtomicUpdateFMax, {F64, F64}}, "__kmpc_atomic_float8_max_cpt"},
        {{AtomicUpdateFMin, {F64, F64}}, "__kmpc_atomic_float8_min_cpt"},
        // F80 = F80 op F80
        {{AtomicUpdateFAdd, {F80, F80}}, "__kmpc_atomic_float10_add_cpt"},
        {{AtomicUpdateFSub, {F80, F80}}, "__kmpc_atomic_float10_sub_cpt"},
        {{AtomicUpdateFMul, {F80, F80}}, "__kmpc_atomic_float10_mul_cpt"},
        {{AtomicUpdateFDiv, {F80, F80}}, "__kmpc_atomic_float10_div_cpt"},
        // F128 = F128 op F128
        {{AtomicUpdateFAdd, {F128, F128}},
         "__kmpc_atomic_float16_add_a16_cpt"},
        {{AtomicUpdateFSub, {F128, F128}},
         "__kmpc_atomic_float16_sub_a16_cpt"},
        {{AtomicUpdateFMul, {F128, F128}},
         "__kmpc_atomic_float16_mul_a16_cpt"},
        {{AtomicUpdateFDiv, {F128, F128}},
         "__kmpc_atomic_float16_div_a16_cpt"}};

const std::map<VPOParoptAtomics::AtomicOperationTy, const std::string>
    VPOParoptAtomics::OpToCaptureIntrinsicForTgtMap = {
        // I32 = I32 op I32
        {{AtomicUpdateAdd, {I32, I32}}, "__kmpc_atomic_fixed4_add_cpt"},
        {{AtomicUpdateSub, {I32, I32}}, "__kmpc_atomic_fixed4_sub_cpt"},
        {{AtomicUpdateMul, {I32, I32}}, "__kmpc_atomic_fixed4_mul_cpt"},
        {{AtomicUpdateSDiv, {I32, I32}}, "__kmpc_atomic_fixed4_div_cpt"},
        {{AtomicUpdateUDiv, {I32, I32}}, "__kmpc_atomic_fixed4u_div_cpt"},
        {{AtomicUpdateAnd, {I32, I32}}, "__kmpc_atomic_fixed4_andb_cpt"},
        {{AtomicUpdateOr, {I32, I32}}, "__kmpc_atomic_fixed4_orb_cpt"},
        {{AtomicUpdateXor, {I32, I32}}, "__kmpc_atomic_fixed4_xor_cpt"},
        {{AtomicUpdateShl, {I32, I32}}, "__kmpc_atomic_fixed4_shl_cpt"},
        {{AtomicUpdateAShr, {I32, I32}}, "__kmpc_atomic_fixed4_shr_cpt"},
        {{AtomicUpdateLShr, {I32, I32}}, "__kmpc_atomic_fixed4u_shr_cpt"},
        {{AtomicUpdateSMax, {I32, I32}}, "__kmpc_atomic_fixed4_max_cpt"},
        {{AtomicUpdateUMax, {I32, I32}}, "__kmpc_atomic_fixed4u_max_cpt"},
        {{AtomicUpdateSMin, {I32, I32}}, "__kmpc_atomic_fixed4_min_cpt"},
        {{AtomicUpdateUMin, {I32, I32}}, "__kmpc_atomic_fixed4u_min_cpt"},
        // I64 = I64 op I64
        {{AtomicUpdateAdd, {I64, I64}}, "__kmpc_atomic_fixed8_add_cpt"},
        {{AtomicUpdateSub, {I64, I64}}, "__kmpc_atomic_fixed8_sub_cpt"},
        {{AtomicUpdateMul, {I64, I64}}, "__kmpc_atomic_fixed8_mul_cpt"},
        {{AtomicUpdateSDiv, {I64, I64}}, "__kmpc_atomic_fixed8_div_cpt"},
        {{AtomicUpdateUDiv, {I64, I64}}, "__kmpc_atomic_fixed8u_div_cpt"},
        {{AtomicUpdateAnd, {I64, I64}}, "__kmpc_atomic_fixed8_andb_cpt"},
        {{AtomicUpdateOr, {I64, I64}}, "__kmpc_atomic_fixed8_orb_cpt"},
        {{AtomicUpdateXor, {I64, I64}}, "__kmpc_atomic_fixed8_xor_cpt"},
        {{AtomicUpdateShl, {I64, I64}}, "__kmpc_atomic_fixed8_shl_cpt"},
        {{AtomicUpdateAShr, {I64, I64}}, "__kmpc_atomic_fixed8_shr_cpt"},
        {{AtomicUpdateLShr, {I64, I64}}, "__kmpc_atomic_fixed8u_shr_cpt"},
        {{AtomicUpdateSMax, {I64, I64}}, "__kmpc_atomic_fixed8_max_cpt"},
        {{AtomicUpdateUMax, {I64, I64}}, "__kmpc_atomic_fixed8u_max_cpt"},
        {{AtomicUpdateSMin, {I64, I64}}, "__kmpc_atomic_fixed8_min_cpt"},
        {{AtomicUpdateUMin, {I64, I64}}, "__kmpc_atomic_fixed8u_min_cpt"},
        // F32 = F32 op F32
        {{AtomicUpdateFAdd, {F32, F32}}, "__kmpc_atomic_float4_add_cpt"},
        {{AtomicUpdateFSub, {F32, F32}}, "__kmpc_atomic_float4_sub_cpt"},
        {{AtomicUpdateFMul, {F32, F32}}, "__kmpc_atomic_float4_mul_cpt"},
        {{AtomicUpdateFDiv, {F32, F32}}, "__kmpc_atomic_float4_div_cpt"},
        {{AtomicUpdateFMax, {F32, F32}}, "__kmpc_atomic_float4_max_cpt"},
        {{AtomicUpdateFMin, {F32, F32}}, "__kmpc_atomic_float4_min_cpt"},
        // F64 = F64 op F64
        {{AtomicUpdateFAdd, {F64, F64}}, "__kmpc_atomic_float8_add_cpt"},
        {{AtomicUpdateFSub, {F64, F64}}, "__kmpc_atomic_float8_sub_cpt"},
        {{AtomicUpdateFMul, {F64, F64}}, "__kmpc_atomic_float8_mul_cpt"},
        {{AtomicUpdateFDiv, {F64, F64}}, "__kmpc_atomic_float8_div_cpt"},
        {{AtomicUpdateFMax, {F64, F64}}, "__kmpc_atomic_float8_max_cpt"},
        {{AtomicUpdateFMin, {F64, F64}}, "__kmpc_atomic_float8_min_cpt"},
};

const std::map<VPOParoptAtomics::AtomicOperationTy, const std::string>
    VPOParoptAtomics::ReversedOpToCaptureIntrinsicMap = {
        // I8 = I8 op I8
        {{AtomicUpdateSub, {I8, I8}}, "__kmpc_atomic_fixed1_sub_cpt_rev"},
        {{AtomicUpdateSDiv, {I8, I8}}, "__kmpc_atomic_fixed1_div_cpt_rev"},
        {{AtomicUpdateUDiv, {I8, I8}}, "__kmpc_atomic_fixed1u_div_cpt_rev"},
        {{AtomicUpdateShl, {I8, I8}}, "__kmpc_atomic_fixed1_shl_cpt_rev"},
        {{AtomicUpdateAShr, {I8, I8}}, "__kmpc_atomic_fixed1_shr_cpt_rev"},
        {{AtomicUpdateLShr, {I8, I8}}, "__kmpc_atomic_fixed1u_shr_cpt_rev"},
        // I16 = I16 op I16
        {{AtomicUpdateSub, {I16, I16}}, "__kmpc_atomic_fixed2_sub_cpt_rev"},
        {{AtomicUpdateSDiv, {I16, I16}}, "__kmpc_atomic_fixed2_div_cpt_rev"},
        {{AtomicUpdateUDiv, {I16, I16}}, "__kmpc_atomic_fixed2u_div_cpt_rev"},
        {{AtomicUpdateShl, {I16, I16}}, "__kmpc_atomic_fixed2_shl_cpt_rev"},
        {{AtomicUpdateAShr, {I16, I16}}, "__kmpc_atomic_fixed2_shr_cpt_rev"},
        {{AtomicUpdateLShr, {I16, I16}}, "__kmpc_atomic_fixed2u_shr_cpt_rev"},
        // I32 = I32 op I32
        {{AtomicUpdateSub, {I32, I32}}, "__kmpc_atomic_fixed4_sub_cpt_rev"},
        {{AtomicUpdateSDiv, {I32, I32}}, "__kmpc_atomic_fixed4_div_cpt_rev"},
        {{AtomicUpdateUDiv, {I32, I32}}, "__kmpc_atomic_fixed4u_div_cpt_rev"},
        {{AtomicUpdateShl, {I32, I32}}, "__kmpc_atomic_fixed4_shl_cpt_rev"},
        {{AtomicUpdateAShr, {I32, I32}}, "__kmpc_atomic_fixed4_shr_cpt_rev"},
        {{AtomicUpdateLShr, {I32, I32}}, "__kmpc_atomic_fixed4u_shr_cpt_rev"},
        // I64 = I64 op I64
        {{AtomicUpdateSub, {I64, I64}}, "__kmpc_atomic_fixed8_sub_cpt_rev"},
        {{AtomicUpdateSDiv, {I64, I64}}, "__kmpc_atomic_fixed8_div_cpt_rev"},
        {{AtomicUpdateUDiv, {I64, I64}}, "__kmpc_atomic_fixed8u_div_cpt_rev"},
        {{AtomicUpdateShl, {I64, I64}}, "__kmpc_atomic_fixed8_shl_cpt_rev"},
        {{AtomicUpdateAShr, {I64, I64}}, "__kmpc_atomic_fixed8_shr_cpt_rev"},
        {{AtomicUpdateLShr, {I64, I64}}, "__kmpc_atomic_fixed8u_shr_cpt_rev"},
        // I8 = F128 op I8
        {{AtomicUpdateFSub, {I8, F128}}, "__kmpc_atomic_fixed1_sub_cpt_rev_fp"},
        {{AtomicUpdateFDiv, {I8, F128}}, "__kmpc_atomic_fixed1_div_cpt_rev_fp"},
        {{AtomicUpdateUDiv, {I8, F128}},
         "__kmpc_atomic_fixed1u_div_cpt_rev_fp"},
        // I16 = F128 op I16
        {{AtomicUpdateFSub, {I16, F128}},
         "__kmpc_atomic_fixed2_sub_cpt_rev_fp"},
        {{AtomicUpdateFDiv, {I16, F128}},
         "__kmpc_atomic_fixed2_div_cpt_rev_fp"},
        {{AtomicUpdateUDiv, {I16, F128}},
         "__kmpc_atomic_fixed2u_div_cpt_rev_fp"},
        // I32 = F128 op I32
        {{AtomicUpdateFSub, {I32, F128}},
         "__kmpc_atomic_fixed4_sub_cpt_rev_fp"},
        {{AtomicUpdateFDiv, {I32, F128}},
         "__kmpc_atomic_fixed4_div_cpt_rev_fp"},
        {{AtomicUpdateUDiv, {I32, F128}},
         "__kmpc_atomic_fixed4u_div_cpt_rev_fp"},
        // I64 = F128 op I64
        {{AtomicUpdateFSub, {I64, F128}},
         "__kmpc_atomic_fixed8_sub_cpt_rev_fp"},
        {{AtomicUpdateFDiv, {I64, F128}},
         "__kmpc_atomic_fixed8_div_cpt_rev_fp"},
        {{AtomicUpdateUDiv, {I64, F128}},
         "__kmpc_atomic_fixed8u_div_cpt_rev_fp"},
        // F32 = F32 op F32
        {{AtomicUpdateFSub, {F32, F32}}, "__kmpc_atomic_float4_sub_cpt_rev"},
        {{AtomicUpdateFDiv, {F32, F32}}, "__kmpc_atomic_float4_div_cpt_rev"},
        // F64 = F64 op F64
        {{AtomicUpdateFSub, {F64, F64}}, "__kmpc_atomic_float8_sub_cpt_rev"},
        {{AtomicUpdateFDiv, {F64, F64}}, "__kmpc_atomic_float8_div_cpt_rev"},
        // F80 = F80 op F80
        {{AtomicUpdateFSub, {F80, F80}}, "__kmpc_atomic_float10_sub_cpt_rev"},
        {{AtomicUpdateFDiv, {F80, F80}}, "__kmpc_atomic_float10_div_cpt_rev"},
        // F128 = F128 op F128
        {{AtomicUpdateFSub, {F128, F128}},
         "__kmpc_atomic_float16_sub_a16_cpt_rev"},
        {{AtomicUpdateFDiv, {F128, F128}},
         "__kmpc_atomic_float16_div_a16_cpt_rev"}};
#endif // INTEL_COLLAB

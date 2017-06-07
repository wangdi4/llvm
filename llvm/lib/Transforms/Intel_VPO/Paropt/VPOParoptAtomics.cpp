//===- VPOParoptAtomics.cpp - Transformation of W-Region for threading --===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation. and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
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

#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptAtomics.h"

#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptUtils.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-atomics"

// Main driver for handling a WRNAtomicNode.
bool VPOParoptAtomics::handleAtomic(WRNAtomicNode *AtomicNode,
                                    StructType *IdentTy, AllocaInst *TidPtr) {
  bool handled;
  assert(AtomicNode != nullptr && "AtomicNode is null.");

  switch (AtomicNode->getAtomicKind()) {
  case WRNAtomicRead:
    handled = handleAtomicRW<WRNAtomicRead>(AtomicNode, IdentTy, TidPtr);
    break;
  case WRNAtomicWrite:
    handled = handleAtomicRW<WRNAtomicWrite>(AtomicNode, IdentTy, TidPtr);
    break;
  case WRNAtomicUpdate:
    handled = handleAtomicUpdate(AtomicNode, IdentTy, TidPtr);
    break;
  case WRNAtomicCapture:
    handled = handleAtomicCapture(AtomicNode, IdentTy, TidPtr);
    break;
  default:
    llvm_unreachable("Unexpected Atomic Kind");
  }

  if (!handled)
    handled =
        VPOParoptUtils::genKmpcCriticalSection(AtomicNode, IdentTy, TidPtr);

  assert(handled == true && "Handling of AtomicNode failed.\n");

  if (handled) {
    bool directivesCleared = VPOUtils::stripDirectives(AtomicNode);
    (void)directivesCleared;
    assert(directivesCleared &&
           "Unable to strip directives from WRNAtomicNode.");

    DEBUG(dbgs() << __FUNCTION__ << ": Handling of AtomicNode successful.\n");
  }
  else
    DEBUG(dbgs() << __FUNCTION__ << ": Handling of AtomicNode failed.\n");

  return handled;
}

// Functions for handling different WRNAtomicNodes.

// Handles generation of KMPC intrinsics for Atomic Read and Write.
template <WRNAtomicKind AtomicKind>
bool VPOParoptAtomics::handleAtomicRW(WRNAtomicNode *AtomicNode,
                                             StructType *IdentTy,
                                             AllocaInst *TidPtr) {
  assert((AtomicKind == WRNAtomicRead || AtomicKind == WRNAtomicWrite) &&
         "Unsupported AtomicKind for handleAtomicReadAndWrite.");
  assert(AtomicNode != nullptr && "AtomicNode is null.");
  assert(IdentTy != nullptr && "IdentTy is null.");
  assert(TidPtr != nullptr && "TidPtr is null.");
  assert(AtomicNode->getBBSetSize() == 3 &&
         "AtomicNode for Read/Write is expected to have 3 BBlocks.");

  bool AtomicRead = (AtomicKind == WRNAtomicRead);

  // The first and last BasicBlocks contain directive intrinsic calls.
  // We're interested in only the middle one here.
  BasicBlock *BB = *(AtomicNode->bbset_begin() + 1);

  // We expect there to be one load/store inside this BBlock, followed by a
  // branch to the next BBlock. We're concerned with the first one.
  assert(BB->size() == 2 && "Unexpected number of instructions in BBlock.");
  Instruction *Inst = &(*(BB->begin()));

  assert(Inst != nullptr && "Inst is null.");
  assert(((AtomicKind == WRNAtomicRead) ==  isa<LoadInst>(Inst)) &&
         "Unexpected Instruction Type for AtomicRead");
  assert(((AtomicKind == WRNAtomicWrite) == isa<StoreInst>(Inst)) &&
         "Unexpected Instruction Type for AtomicWrite");
  DEBUG(dbgs() << __FUNCTION__ << ": Source Instruction: " << *Inst << "\n");

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
  SmallVector<Value*, 2> FnArgs;
  AtomicRead ? FnArgs.assign({Ptr})                       // {ptr}
             : FnArgs.assign({Ptr, Inst->getOperand(0)}); // {ptr, val}

  // We have the val and ptr operands now. So we can use them to get the
  // operand type, and find the KMPC intrinsic name, if it exists for the type.
  assert(isa<PointerType>(Ptr->getType()) && "Unexpected type for operand.");
  Type* OpndTy = Ptr->getType()->getContainedType(0);
  assert(OpndTy != nullptr && "Operand Type is null.");
  const std::string Name =
      getAtomicRWSIntrinsicName<AtomicKind>(*(Inst->getParent()), *OpndTy);
  if (Name.empty())
    return false; // No intrinsic found. Handle using critical sections.

  // The return type is the Type of the operand for load, and void for store.
  Type *ReturnTy = AtomicRead ? OpndTy : nullptr;

  // Now try to generate the call for kmpc_atomic_rd/kmpc_atomic_wr of type:
  //     __kmpc_atomic_<type>_rd(loc, tid, ptr)
  //     __kmpc_atomic_<type>_wr(loc, tid, ptr, val)
  CallInst *AtomicCall = VPOParoptUtils::genKmpcCallWithTid(
      AtomicNode, IdentTy, TidPtr, Inst, Name, ReturnTy, FnArgs);
  assert(AtomicCall != nullptr && "Generated KMPC call is null.");

  ReplaceInstWithInst(Inst, AtomicCall);
  DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic call inserted.\n");
  return true;
}

// Handles generation of KMPC intrinsics for Atomic Update.
bool VPOParoptAtomics::handleAtomicUpdate(WRNAtomicNode *AtomicNode,
                                          StructType *IdentTy,
                                          AllocaInst *TidPtr) {
  assert(AtomicNode != nullptr && "AtomicNode is null.");
  assert(IdentTy != nullptr && "IdentTy is null.");
  assert(TidPtr != nullptr && "TidPtr is null.");
  assert(AtomicNode->getBBSetSize() == 3 &&
         "AtomicNode for Atomic Update is expected to have 3 BBlocks.");

  // The first and last BasicBlocks contain directive intrinsic calls.
  // We're interested in only the middle one here.
  BasicBlock *BB = *(AtomicNode->bbset_begin() + 1);

  // We maintain a list of Instructions that will be deleted from BB, when the
  // KMPC call is added.
  SmallVector<Instruction*, ApproxNumInstsToDeleteForUpdate> InstsToDelete;

  // Now, the last instruction of the BBlock will be an unconditional branch to
  // the next BBlock, and its predecessor is the a store to the Atomic Operand.
  // We start off with this Instruction to get the atomic operand.
  assert(BB->size() >= 4 && "Unexpected number of instructions in BBlock.");
  StoreInst *OpndStore = dyn_cast<StoreInst>(&*(++(BB->rbegin())));
  assert(OpndStore != nullptr && "Unable to find store to atomic operand");

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
  bool UpdateOpFound = extractAtomicUpdateOp(
      BB, AtomicOpnd,                                             // In
      OpInst, ValueOpnd, Reversed, AtomicOpndStore, InstsToDelete // Out
      );

  if (!UpdateOpFound)
    return false; // Handle using critical sections.

  // At this point, we may need to generate a CastInst for ValueOpnd, in case
  // the types of AtomicOpnd and ValueOpnd are not the same.
  CastInst *ValueOpndCast = genCastForValueOpnd<WRNAtomicUpdate>(
      OpInst, Reversed, AtomicOpnd, ValueOpnd);

  ValueOpnd = ValueOpndCast != nullptr ? ValueOpndCast : ValueOpnd;
  // Note that we have not yet inserted this Cast into the IR. We do that only
  // after we find a matching intrinsic.

  // Now we know the atomic operand, the value operand, and the operation. We
  // now check whether an intrinsic exists which supports the given combination
  // of operation and operands.
  DEBUG(dbgs() << __FUNCTION__ << ": Atomic Opnd: " << *AtomicOpnd << "\n");
  DEBUG(dbgs() << __FUNCTION__ << ": Value Opnd: " << *ValueOpnd << "\n");
  const std::string Name = getAtomicUCIntrinsicName<WRNAtomicUpdate>(
      *OpInst, Reversed, *AtomicOpnd, *ValueOpnd);
  if (Name.empty()) {
    if (ValueOpndCast != nullptr)
      delete ValueOpndCast;
    // No intrinsic found. Handle using critical sections.
    return false;
  }

  // We found the matching intrinsic. So, it's safe to insert ValueOpndCast
  // into the IR.
  if (ValueOpndCast != nullptr)
    ValueOpndCast->insertBefore(AtomicOpndStore);

  // Next, generate and insert the KMPC call for atomic update. It looks like:
  //     call void __kmpc_atomic_<...>(loc, tid, atomic_opnd, value_opnd)
  CallInst *AtomicCall = VPOParoptUtils::genKmpcCallWithTid(
      AtomicNode, IdentTy, TidPtr, OpndStore, Name, nullptr,
      {AtomicOpnd, ValueOpnd});
  assert(AtomicCall != nullptr && "Generated KMPC call is null.");

  AtomicCall->insertBefore(OpndStore);
  DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic call inserted.\n");

  // And finally, delete the instructions that are no longer needed.
  for (auto *Inst : InstsToDelete) {
    Inst->replaceAllUsesWith(UndefValue::get(Inst->getType()));
    Inst->eraseFromParent();
  }

  return true;
}

// Handles generation of KMPC intrinsics for Atomic Capture.
bool VPOParoptAtomics::handleAtomicCapture(WRNAtomicNode *AtomicNode,
                                           StructType *IdentTy,
                                           AllocaInst *TidPtr) {
  assert(AtomicNode != nullptr && "AtomicNode is null.");
  assert(IdentTy != nullptr && "IdentTy is null.");
  assert(TidPtr != nullptr && "TidPtr is null.");
  assert(AtomicNode->getBBSetSize() == 3 &&
         "AtomicNode for Atomic Capture is expected to have 3 BBlocks.");

  // The first and last BasicBlocks contain directive intrinsic calls.
  // We're interested in only the middle one here.
  BasicBlock *BB = *(AtomicNode->bbset_begin() + 1);
  assert(BB->size() >= 4 && "Unexpected number of instructions in BBlock.");
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
                             InstsToDelete); // In, Out

  if (CaptureKind == CaptureUnknown)
    return false; // Handle using critical section.

  // At this point, we may need to generate a CastInst for ValueOpnd, in case
  // the types of AtomicOpnd and ValueOpnd are not the same.
  CastInst *ValueOpndCast = genCastForValueOpnd<WRNAtomicCapture>(
      OpInst, Reversed, AtomicOpnd, ValueOpnd);

  ValueOpnd = ValueOpndCast != nullptr ? ValueOpndCast : ValueOpnd;
  // Note that we have not yet inserted this Cast into the IR. We do that only
  // after we find a matching intrinsic.

  // We know the atomic operand, the value operand, and the operation. We can
  // find a compatible KMPC intrinsic now.
  DEBUG(dbgs() << __FUNCTION__ << ": Atomic Opnd: " << *AtomicOpnd << "\n");
  DEBUG(dbgs() << __FUNCTION__ << ": Capture Opnd: " << *CaptureOpnd << "\n");
  DEBUG(dbgs() << __FUNCTION__ << ": Value Opnd: " << *ValueOpnd << "\n");
  assert(AtomicOpnd->getType()->isPointerTy() && "Unexpected AtomicOpnd.");
  assert(CaptureOpnd->getType()->isPointerTy() && "Unexpected CaptureOpnd.");

  const std::string Name = getAtomicCaptureIntrinsicName(
      CaptureKind, BB, OpInst, Reversed, AtomicOpnd, ValueOpnd);
  if (Name.empty()) {
    if (ValueOpndCast != nullptr)
      delete ValueOpndCast;
    // No intrinsic found. Handle using critical sections.
    return false;
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
  SmallVector<Value*, 3> FnArgs = {AtomicOpnd, ValueOpnd};
  if (CaptureKind != CaptureSwap) {

    Function *F = BB->getParent();
    LLVMContext &C = F->getContext();

    Value *CaptureFlag = ConstantInt::get(
        Type::getInt32Ty(C), CaptureKind == CaptureBeforeOp ? 0 : 1);
    FnArgs.push_back(CaptureFlag);
  }

  // Second, the return type.
  Type* ReturnTy = AtomicOpnd->getType()->getContainedType(0);
  assert(ReturnTy != nullptr && "Invalid return type for KMPC call.");

  // Now we can generate the call.
  CallInst *AtomicCall = VPOParoptUtils::genKmpcCallWithTid(
      AtomicNode, IdentTy, TidPtr, Anchor, Name, ReturnTy,
      FnArgs);
  assert(AtomicCall != nullptr && "Generated KMPC call is null.");

  AtomicCall->insertBefore(Anchor);
  DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic call inserted.\n");

  // Now we need to store the result of this call to the capture operand,
  // which may have a different type than the value returned by the call.
  Value* CaptureVal = AtomicCall;
  if (CaptureOpndCast != nullptr)
    CaptureVal = CastInst::Create(CaptureOpndCast->getOpcode(), AtomicCall,
                                  CaptureOpnd->getType()->getContainedType(0),
                                  "cpt.opnd.cast", Anchor);

  // Now generate the store to CaptureOpnd.
  StoreInst *CaptureStore = new StoreInst(CaptureVal, CaptureOpnd);
  CaptureStore->insertBefore(Anchor);

  // And finally, delete the instructions that are no longer needed.
  for (auto *Inst : InstsToDelete) {
    Inst->replaceAllUsesWith(UndefValue::get(Inst->getType()));
    Inst->eraseFromParent();
  }

  return true;
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
bool VPOParoptAtomics::extractAtomicUpdateOp(
    BasicBlock *BB, Value *AtomicOpnd,                       // In
    Instruction *&OpInst, Value *&ValueOpnd, bool &Reversed, // Out
    StoreInst *&AtomicOpndStore,                             // Out
    SmallVectorImpl<Instruction *> &InstsToDelete) {         // In, Out

  assert(BB != nullptr && "BasicBlock is null.");
  assert(AtomicOpnd != nullptr && "AtomicOpnd is null.");

  // We have AtomicOpnd. We look for a store to it within the BasicBlock BB.
  // There should only be one.
  StoreInst *AtomicStore = getStoreToOpndIfUnique(*BB, *AtomicOpnd);
  if (AtomicStore == nullptr) {
    DEBUG(dbgs() << __FUNCTION__
                 << ": More than one stores in BB for AtomicOpnd:"
                 << *AtomicOpnd << "\n");
    return false; // Handle using critical sections.
  }

  InstsToDelete.push_back(AtomicStore); // (v)

  Instruction* Op; // atomic update operation e.g. `sub` for `x = x - expr`.
  // Now, the value being stored in AtomicStore will give us the atomic
  // operation (Op). But to get that, we need to strip off any casts, if
  // present.
  Value *OpResult = AtomicStore->getValueOperand();
  auto *OpResultCast = dyn_cast<CastInst>(OpResult);
  if (OpResultCast != nullptr) {
    OpResult = OpResultCast->getOperand(0);
    InstsToDelete.push_back(OpResultCast); // (iv)
  }

  Op = dyn_cast<Instruction>(OpResult);

  if (!isa<BinaryOperator>(Op)) {
    DEBUG(dbgs() << __FUNCTION__ << ": Unexpected update Op:" << *Op << "\n");
    InstsToDelete.clear();
    return false;
  }
  InstsToDelete.push_back(Op); // (iii)

  // Now, we have the operation. One of its two operands should be a load from
  // AtomicOpnd, (or a cast of it). Then, the other operand will be ValueOpnd.
  unsigned Idx;
  for (Idx = 0; Idx <= 1; ++Idx) {
    Value *Opnd = Op->getOperand(Idx);
    auto *OpndLoadCast = dyn_cast<CastInst>(Opnd);
    if (OpndLoadCast != nullptr)
      Opnd = OpndLoadCast->getOperand(0);

    auto *OpndLoad = dyn_cast<LoadInst>(Opnd);
    if (OpndLoad == nullptr)
      continue;

    if (OpndLoad->getPointerOperand() == AtomicOpnd) {
      if (OpndLoadCast != nullptr)
        InstsToDelete.push_back(OpndLoadCast); // (ii)
      InstsToDelete.push_back(OpndLoad);       // (i)
      break;
    }
  }

  if (Idx > 1) {
    DEBUG(dbgs() << __FUNCTION__
                 << ": Load of AtomicOpnd not used in Op:" << *Op << "\n");
    InstsToDelete.clear();
    return false;
  }
  // A load for AtomicOpnd was found at index `Idx`. So, the operand at position
  // `1-Idx` is ValueOpnd.
  ValueOpnd = Op->getOperand(1 - Idx);
  assert(ValueOpnd != nullptr && "ValueOpnd is null.");
  // Also, if Op is a non-commutative operation, like div and sub, and
  // AtomicOpnd is the second operand, then the update operation is reversed.
  Reversed = Idx == 1 && !Op->isCommutative();
  OpInst = Op;
  AtomicOpndStore = AtomicStore;
  return true;
}

// For a given BB, try to find the capture op and operands.
VPOParoptAtomics::AtomicCaptureKind VPOParoptAtomics::extractAtomicCaptureOp(
    BasicBlock *BB,                                                   // In
    Instruction *&OpInst,  Value *&AtomicOpnd, Value *&ValueOpnd,      // Out
    Value *&CaptureOpnd, bool &Reversed, StoreInst *&AtomicOpndStore, // Out
    CastInst *&CaptureOpndCast,                                       // Out
    SmallVectorImpl<Instruction *> &InstsToDelete) {                  // In, Out

  assert(BB != nullptr && "BB is null.");

  // A store to AtomicOpnd/CaptureOpnd could only be the first, second-last or
  // the last StoreInst within BB.
  SmallVector<StoreInst *, 3> StoreCandidates =
      gatherFirstSecondToLastAndLastStores(*BB);

  if (StoreCandidates.size() < 3) {
    DEBUG(dbgs()
          << __FUNCTION__
          << ": Unexpected: Less than two stores found in BB.\n");
    return CaptureUnknown;
  }

  AtomicCaptureKind CaptureKind = CaptureUnknown;

  // (A) CaptureBeforeOp:
  // First, try to process the capture-before-op case. Here, the last store of
  // BB should be the store to x, and the first store should be a store to v.
  DEBUG(dbgs() << __FUNCTION__ << ": Processing as CaptureBeforeOp...\n");
  AtomicOpnd = (*StoreCandidates.rbegin())->getPointerOperand();
  CaptureOpnd = (*StoreCandidates.begin())->getPointerOperand();

  bool UpdateOpFound =
      extractAtomicUpdateOp(BB, AtomicOpnd,                               // In
                            OpInst, ValueOpnd, Reversed, AtomicOpndStore, // Out
                            InstsToDelete); // In, Out

  if (UpdateOpFound)
    CaptureKind =
        identifyNonSwapCaptureKind(BB, AtomicOpndStore, CaptureOpnd, // In
                                   CaptureOpndCast,                  // Out
                                   InstsToDelete);                   // In, Out

  if (CaptureKind != CaptureUnknown) {
    DEBUG(dbgs() << __FUNCTION__
                 << ": CaptureBeforeOp identified. Op: " << *OpInst << "\n");
    return CaptureKind;
  }

  // (B) CaptureSwap
  // Second, try to process CaptureSwap. Here again, first store of BB will be to v, and
  // the last store to x.
  DEBUG(dbgs() << __FUNCTION__ << ": Processing as CaptureSwap...\n");
  AtomicOpnd = (*StoreCandidates.rbegin())->getPointerOperand();
  CaptureOpnd = (*StoreCandidates.begin())->getPointerOperand();

  bool SwapOpFound =
      extractSwapOp(BB, AtomicOpnd, CaptureOpnd,                 // In
                    ValueOpnd, AtomicOpndStore, CaptureOpndCast, // Out
                    InstsToDelete);                              // In, Out

  if (SwapOpFound) {
    DEBUG(dbgs() << __FUNCTION__ << ": CaptureSwap identified.\n");
    return CaptureSwap;
  }

  // (C) CaptureAfterOp
  // Last, try to process CaptureAfterOp. Here Last store of BB will be to v,
  // and second last to x.
  DEBUG(dbgs() << __FUNCTION__ << ": Processing as CaptureAfterOp...\n");
  AtomicOpnd =(*(StoreCandidates.rbegin() + 1))->getPointerOperand();
  CaptureOpnd =(*StoreCandidates.rbegin())->getPointerOperand();

  UpdateOpFound =
      extractAtomicUpdateOp(BB, AtomicOpnd,                               // In
                            OpInst, ValueOpnd, Reversed, AtomicOpndStore, // Out
                            InstsToDelete); // In, Out

  if (UpdateOpFound)
    CaptureKind =
        identifyNonSwapCaptureKind(BB, AtomicOpndStore, CaptureOpnd, // In
                                   CaptureOpndCast,                  // Out
                                   InstsToDelete);                   // In, Out

  if (CaptureKind != CaptureUnknown) {
    DEBUG(dbgs() << __FUNCTION__
                 << ": CaptureAfterOp identified. Op: " << *OpInst << "\n");
    return CaptureKind;
  }

  // (D) No Capture operation found.
  DEBUG(dbgs() << __FUNCTION__ << ": Capture Op not found.\n");
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
    DEBUG(dbgs() << __FUNCTION__
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
    DEBUG(dbgs() << __FUNCTION__
                 << ": Value stored to capture operand is not atomic operand: "
                 << LoadX << " \n");
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
    DEBUG(
        dbgs() << __FUNCTION__
               << ": Value stored to capture operand is not atomic operand: v: "
               << ValV << "x: " << ValX << " \n");
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

// Usig a given AtomicOpnd and CaptureOpnd, try to find ValueOpnd assuming the
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
//     store float %conv1, float* %x, align 4
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
  assert(!extractAtomicUpdateOp(BB, AtomicOpnd,                       // In
                                OpInst, ValueOpnd, Reversed, AtStore, // Out
                                InstsToDelete) &&                     // In, Out
         "Atomic capture is not of swap kind.");
#endif

  StoreInst* AtomicStore = getStoreToOpndIfUnique(*BB, *AtomicOpnd);
  StoreInst* CaptureStore = getStoreToOpndIfUnique(*BB, *CaptureOpnd);

  if(AtomicStore == nullptr || CaptureStore == nullptr) {
    DEBUG(dbgs() << __FUNCTION__ << ": There should be only one store to "
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
  // we don't need the existing store to v.
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
                                                const Value *AtomicOpnd,
                                                Value *ValueOpnd) {
  assert((AtomicKind == WRNAtomicUpdate || AtomicKind == WRNAtomicCapture) &&
         "Invalid atomic kind.");
  assert((Op != nullptr || AtomicKind == WRNAtomicCapture) &&
         "Op is needed for atomic update.");
  assert((AtomicKind == WRNAtomicCapture || isa<BinaryOperator>(Op)) &&
         "Unsupported Op type.");
  assert(AtomicOpnd != nullptr && "AtomicOpnd is null.");
  assert(ValueOpnd != nullptr && "ValueOpnd is null.");

  Type* ValueOpndTy = ValueOpnd->getType();
  Type* AtomicOpndTy = AtomicOpnd->getType()->getContainedType(0);

  if (AtomicOpndTy->isIntegerTy() && ValueOpndTy->isIntegerTy())
    return genTruncForValueOpnd(*AtomicOpnd, *ValueOpnd);

  if (AtomicKind == WRNAtomicUpdate && AtomicOpndTy->isIntegerTy() &&
      ValueOpndTy->isFloatingPointTy())
    return genFPExtForValueOpnd(*Op, Reversed, *AtomicOpnd, *ValueOpnd);

  if (AtomicOpndTy->isFloatingPointTy() && ValueOpndTy->isFloatingPointTy())
    return genFPTruncForValueOpnd<AtomicKind>(*AtomicOpnd, *ValueOpnd);

  // No need for a Cast.
  return nullptr;
}

// Generates an integer Trunc Cast for ValueOpnd for Atomic Update, if needed.
CastInst *VPOParoptAtomics::genTruncForValueOpnd(const Value &AtomicOpnd,
                                                 Value &ValueOpnd) {
  assert(AtomicOpnd.getType()->isPointerTy() &&
         "Unexpected type for Atomic operand.");

  IntegerType *ValueOpndTy = dyn_cast<IntegerType>(ValueOpnd.getType());
  IntegerType *AtomicOpndTy =
      dyn_cast<IntegerType>(AtomicOpnd.getType()->getContainedType(0));

  if (AtomicOpndTy == nullptr || ValueOpndTy == nullptr ||
      AtomicOpndTy->getBitWidth() >=
          ValueOpndTy->getBitWidth()) // Cast not needed when AtomicOpnd has
                                      // more bits than ValueOpnd.
    return nullptr;

  DEBUG(dbgs() << __FUNCTION__
               << ": Generating Trunc for ValueOpnd: " << ValueOpnd << "\n");

  return new TruncInst(&ValueOpnd, AtomicOpndTy, "val.opnd.trunc");
}


// Generates an FPExt cast for ValueOpnd for Atomic Update, if needed.
CastInst *VPOParoptAtomics::genFPExtForValueOpnd(const Instruction &Op,
                                                 bool Reversed,
                                                 const Value &AtomicOpnd,
                                                 Value &ValueOpnd) {
  assert(isa<BinaryOperator>(Op) && "Unsupported Op type.");

  Type *ValueOpndTy = ValueOpnd.getType();
  Type *AtomicOpndTy = AtomicOpnd.getType();
  assert(AtomicOpndTy->isPointerTy() && "Unexpected type for Atomic operand.");

  if (!AtomicOpndTy->getContainedType(0)->isIntegerTy() ||        // Ptr to Int
      !(ValueOpndTy->isFloatTy() || ValueOpndTy->isX86_FP80Ty())) // F32 or F80
    return nullptr;

  DEBUG(dbgs() << __FUNCTION__
               << ": Generating FPExt for ValueOpnd: " << ValueOpnd << "\n");

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
CastInst *VPOParoptAtomics::genFPTruncForValueOpnd(const Value &AtomicOpnd,
                                                   Value &ValueOpnd) {
  assert((AtomicKind == WRNAtomicUpdate || AtomicKind == WRNAtomicCapture) &&
         "Invalid atomic kind.");
  assert(AtomicOpnd.getType()->isPointerTy() &&
         "Unexpected type for Atomic operand.");

  Type *ValueOpndTy = ValueOpnd.getType();
  Type *AtomicOpndTy = AtomicOpnd.getType()->getContainedType(0);

  if (!AtomicOpndTy->isFloatingPointTy() || !ValueOpndTy->isFloatingPointTy() ||
      AtomicOpndTy->getScalarSizeInBits() >=
          ValueOpndTy->getScalarSizeInBits()) // Cast not needed when AtomicOpnd
                                              // has more bits than ValueOpnd.
    return nullptr;

  if (AtomicKind == WRNAtomicUpdate && AtomicOpndTy->isFloatTy() &&
      ValueOpndTy->isDoubleTy()) // float4-float8 update intrinsics exist.
    return nullptr;

  DEBUG(dbgs() << __FUNCTION__
               << ": Generating FPTrunc for ValueOpnd: " << ValueOpnd << "\n");

  return CastInst::CreateFPCast(&ValueOpnd, AtomicOpndTy, "val.opnd.fptrunc");
}

// Misc helper methods.

// Populates CandidateList with all StoreInsts from BB which could be storing to
// the atomic operand.
SmallVector<StoreInst *, 3>
VPOParoptAtomics::gatherFirstSecondToLastAndLastStores(BasicBlock &BB) {

  SmallVector<StoreInst*, 3> StoreList;

  for (auto It = BB.begin(), End = BB.end(); It != End; ++It) {
    if (StoreInst *SI = dyn_cast<StoreInst>(&*It)) {
      StoreList.push_back(SI); // First store -> StoreList[0]
      break;
    }
  }

  if (StoreList.size() != 1) {
      DEBUG(dbgs() << __FUNCTION__ << " : No StoreInst found in BB\n.");
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
      DEBUG(dbgs() << __FUNCTION__ << " : More than one StoreInsts to operand: "
                   << Opnd << " \n");
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

  const AtomicOperandTy Ty = {OpndTy.getTypeID(),
                              OpndTy.getPrimitiveSizeInBits()};

  auto MapEntry = MapToUse.find(Ty);

  if (MapEntry == MapToUse.end()) {
    DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic not found.\n");
    return std::string();
  }

  DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic found: " << MapEntry->second
               << "\n");
  return adjustIntrinsicNameForArchitecture(BB, MapEntry->second);
}

// Does map lookup to find the atomic update/non-swap capture intrinsic name.
template <WRNAtomicKind AtomicKind,
          VPOParoptAtomics::AtomicCaptureKind CaptureKind>
const std::string VPOParoptAtomics::getAtomicUCIntrinsicName(
    const Instruction &Operation, bool Reversed, const Value &AtomicOpnd,
    const Value &ValueOpnd) {

  assert((AtomicKind == WRNAtomicUpdate || ((AtomicKind == WRNAtomicCapture) &&
                                            (CaptureKind == CaptureBeforeOp ||
                                             CaptureKind == CaptureAfterOp))) &&
         "Unsupported AtomicKind for genAtomicUCIntrinsicName");
  assert((!Reversed || !Operation.isCommutative()) &&
         "Unexpected Reversed flag for commutative operation.");
  assert(isa<PointerType>(AtomicOpnd.getType()) && "Invalid AtomicOpnd.");

  Type *AtomicOpndType = AtomicOpnd.getType()->getContainedType(0);
  Type *ValueOpndType = ValueOpnd.getType();
  assert(AtomicOpndType != nullptr && "AtomicOpndType is null");
  assert(ValueOpndType != nullptr && "ValueOpndType is null");

  const bool AtomicUpdate = (AtomicKind == WRNAtomicUpdate);
  auto &MapToUse = AtomicUpdate ? (Reversed ? ReversedOpToUpdateIntrinsicMap
                                            : OpToUpdateIntrinsicMap)
                                : (Reversed ? ReversedOpToCaptureIntrinsicMap
                                            : OpToCaptureIntrinsicMap);

  // We need the operand types and the operation's op code to do a map lookup.
  const AtomicOperandTy AtomicOpndTy = {
      AtomicOpndType->getTypeID(), AtomicOpndType->getPrimitiveSizeInBits()};
  const AtomicOperandTy ValueOpndTy = {ValueOpndType->getTypeID(),
                                       ValueOpndType->getPrimitiveSizeInBits()};

  unsigned OpCode = Operation.getOpcode();
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
  if (AtomicUpdate && OpCode == Instruction::FDiv && ValueOpndTy == F128) {
    if (isUIToFPCast(*(Operation.getOperand(Reversed ? 1 : 0))))
      OpCode = Instruction::UDiv;
  }

  // Create the key from op code and opnd types, and do the map lookup.
  const AtomicOperationTy OpTy = {OpCode, {AtomicOpndTy, ValueOpndTy}};
  auto MapEntry = MapToUse.find(OpTy);

  if (MapEntry == MapToUse.end()) {
    DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic not found.\n");
    return std::string();
  }

  DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic found: " << MapEntry->second
               << "\n");
  return adjustIntrinsicNameForArchitecture(*(Operation.getParent()),
                                            MapEntry->second);
}

// Invokes appropriate functions to find the atomic capture intrinsic name.
const std::string VPOParoptAtomics::getAtomicCaptureIntrinsicName(
    AtomicCaptureKind CaptureKind, const BasicBlock *BB,
    const Instruction *Operation, bool Reversed, const Value *AtomicOpnd,
    const Value *ValueOpnd) {

  assert(CaptureKind != CaptureUnknown && "Unknown capture operation.");
  assert(BB != nullptr && "BB is null.");
  assert((CaptureKind == CaptureSwap || Operation != nullptr) &&
         "Need Operation for name lookup for CaptureBefore and CaptureAfter.");
  assert((CaptureKind == CaptureSwap || !Reversed ||
          !Operation->isCommutative()) &&
         "Reversed flag found true for commutative operation.");
  assert(AtomicOpnd != nullptr && "AtomicOpnd is null.");
  assert(AtomicOpnd->getType()->isPointerTy() &&
         "Atomic operand should be pointer type.");
  assert(ValueOpnd != nullptr && "ValueOpnd is null.");

  switch (CaptureKind) {
  case CaptureSwap:
    return getAtomicRWSIntrinsicName<WRNAtomicCapture, CaptureSwap>(
        *BB, *(AtomicOpnd->getType()->getContainedType(0)));

  case CaptureBeforeOp:
    return getAtomicUCIntrinsicName<WRNAtomicCapture, CaptureBeforeOp>(
        *Operation, Reversed, *AtomicOpnd, *ValueOpnd);

  case CaptureAfterOp:
    return getAtomicUCIntrinsicName<WRNAtomicCapture, CaptureAfterOp>(
        *Operation, Reversed, *AtomicOpnd, *ValueOpnd);
  default:
    llvm_unreachable("Unknown capture kind in getAtomicCaptureIntrinsicName.");
  }
}

// Remove '_a16' from an intrinsic's name.
const std::string VPOParoptAtomics::adjustIntrinsicNameForArchitecture(
    const BasicBlock &BB, const std::string &IntrinsicName) {

  assert(!IntrinsicName.empty() && "Intrinsic name is empty");

  // If IntrinsicName is not an '_a16' version, return as is.
  size_t A16StartingPosition = IntrinsicName.find("_a16");
  if (A16StartingPosition == std::string::npos)
    return IntrinsicName;

  // Otherwise, find the architecture.
  const Function *F = BB.getParent();
  const Module *M = F->getParent();
  assert(M != nullptr && "Unable to find module.");

  // And if architecture is X86, return IntrinsicName as is.
  Triple TargetTriple(M->getTargetTriple());
  if (TargetTriple.getArch() == Triple::x86)
    return IntrinsicName;

  // If not, strip '_a16' from the intrinsic name.
  const std::string ArchAdjustedIntrinsicName =
      IntrinsicName.substr(0, A16StartingPosition) +
      IntrinsicName.substr(A16StartingPosition + 4);
  DEBUG(dbgs() << __FUNCTION__ << ": Adjusted intrinsic name: "
               << ArchAdjustedIntrinsicName << "\n");
  return ArchAdjustedIntrinsicName;
}

// Static member initializations

// Initialize AtomicOperandTy object instances for different types.
const VPOParoptAtomics::AtomicOperandTy
    VPOParoptAtomics::I8 = {Type::IntegerTyID, 8},
    VPOParoptAtomics::I16 = {Type::IntegerTyID, 16},
    VPOParoptAtomics::I32 = {Type::IntegerTyID, 32},
    VPOParoptAtomics::I64 = {Type::IntegerTyID, 64},
    VPOParoptAtomics::P32 = {Type::PointerTyID, 32},
    VPOParoptAtomics::P64 = {Type::PointerTyID, 64},
    VPOParoptAtomics::F32 = {Type::FloatTyID, 32},
    VPOParoptAtomics::F64 = {Type::DoubleTyID, 64},
    VPOParoptAtomics::F80 = {Type::X86_FP80TyID, 80},
    VPOParoptAtomics::F128 = {Type::FP128TyID, 128};

// Initialize intrinsic maps

// TODO: Add calls for complexes when supported.
// NOTE: Read Intrinsic for a complex takes 3 args instead of two.
// TODO: Use different non '_a16` versions for float128 on x86.
// TODO: Use TableGen for maps.
const std::map<VPOParoptAtomics::AtomicOperandTy, const std::string>
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

const std::map<VPOParoptAtomics::AtomicOperandTy, const std::string>
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

const std::map<VPOParoptAtomics::AtomicOperandTy, const std::string>
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
// using Instruction::FDiv for the first case, and Instruction::UDiv for the
// second.
const std::map<VPOParoptAtomics::AtomicOperationTy, const std::string>
    VPOParoptAtomics::OpToUpdateIntrinsicMap = {
        // I8 = I8 op I8
        {{Instruction::Add, {I8, I8}}, "__kmpc_atomic_fixed1_add"},
        {{Instruction::Sub, {I8, I8}}, "__kmpc_atomic_fixed1_sub"},
        {{Instruction::Mul, {I8, I8}}, "__kmpc_atomic_fixed1_mul"},
        {{Instruction::SDiv, {I8, I8}}, "__kmpc_atomic_fixed1_div"},
        {{Instruction::UDiv, {I8, I8}}, "__kmpc_atomic_fixed1u_div"},
        {{Instruction::And, {I8, I8}}, "__kmpc_atomic_fixed1_andb"},
        {{Instruction::Or, {I8, I8}}, "__kmpc_atomic_fixed1_orb"},
        {{Instruction::Xor, {I8, I8}}, "__kmpc_atomic_fixed1_xor"},
        {{Instruction::Shl, {I8, I8}}, "__kmpc_atomic_fixed1_shl"},
        {{Instruction::AShr, {I8, I8}}, "__kmpc_atomic_fixed1_shr"},
        {{Instruction::LShr, {I8, I8}}, "__kmpc_atomic_fixed1u_shr"},
        // I16 = I16 op I16
        {{Instruction::Add, {I16, I16}}, "__kmpc_atomic_fixed2_add"},
        {{Instruction::Sub, {I16, I16}}, "__kmpc_atomic_fixed2_sub"},
        {{Instruction::Mul, {I16, I16}}, "__kmpc_atomic_fixed2_mul"},
        {{Instruction::SDiv, {I16, I16}}, "__kmpc_atomic_fixed2_div"},
        {{Instruction::UDiv, {I16, I16}}, "__kmpc_atomic_fixed2u_div"},
        {{Instruction::And, {I16, I16}}, "__kmpc_atomic_fixed2_andb"},
        {{Instruction::Or, {I16, I16}}, "__kmpc_atomic_fixed2_orb"},
        {{Instruction::Xor, {I16, I16}}, "__kmpc_atomic_fixed2_xor"},
        {{Instruction::Shl, {I16, I16}}, "__kmpc_atomic_fixed2_shl"},
        {{Instruction::AShr, {I16, I16}}, "__kmpc_atomic_fixed2_shr"},
        {{Instruction::LShr, {I16, I16}}, "__kmpc_atomic_fixed2u_shr"},
        // I32 = I32 op I32
        {{Instruction::Add, {I32, I32}}, "__kmpc_atomic_fixed4_add"},
        {{Instruction::Sub, {I32, I32}}, "__kmpc_atomic_fixed4_sub"},
        {{Instruction::Mul, {I32, I32}}, "__kmpc_atomic_fixed4_mul"},
        {{Instruction::SDiv, {I32, I32}}, "__kmpc_atomic_fixed4_div"},
        {{Instruction::UDiv, {I32, I32}}, "__kmpc_atomic_fixed4u_div"},
        {{Instruction::And, {I32, I32}}, "__kmpc_atomic_fixed4_andb"},
        {{Instruction::Or, {I32, I32}}, "__kmpc_atomic_fixed4_orb"},
        {{Instruction::Xor, {I32, I32}}, "__kmpc_atomic_fixed4_xor"},
        {{Instruction::Shl, {I32, I32}}, "__kmpc_atomic_fixed4_shl"},
        {{Instruction::AShr, {I32, I32}}, "__kmpc_atomic_fixed4_shr"},
        {{Instruction::LShr, {I32, I32}}, "__kmpc_atomic_fixed4u_shr"},
        // I64 = I64 op I64
        {{Instruction::Add, {I64, I64}}, "__kmpc_atomic_fixed8_add"},
        {{Instruction::Sub, {I64, I64}}, "__kmpc_atomic_fixed8_sub"},
        {{Instruction::Mul, {I64, I64}}, "__kmpc_atomic_fixed8_mul"},
        {{Instruction::SDiv, {I64, I64}}, "__kmpc_atomic_fixed8_div"},
        {{Instruction::UDiv, {I64, I64}}, "__kmpc_atomic_fixed8u_div"},
        {{Instruction::And, {I64, I64}}, "__kmpc_atomic_fixed8_andb"},
        {{Instruction::Or, {I64, I64}}, "__kmpc_atomic_fixed8_orb"},
        {{Instruction::Xor, {I64, I64}}, "__kmpc_atomic_fixed8_xor"},
        {{Instruction::Shl, {I64, I64}}, "__kmpc_atomic_fixed8_shl"},
        {{Instruction::AShr, {I64, I64}}, "__kmpc_atomic_fixed8_shr"},
        {{Instruction::LShr, {I64, I64}}, "__kmpc_atomic_fixed8u_shr"},
        // I8 = I8 op F64
        {{Instruction::FMul, {I8, F64}}, "__kmpc_atomic_fixed1_mul_float8"},
        {{Instruction::FDiv, {I8, F64}}, "__kmpc_atomic_fixed1_div_float8"},
        // I16 = I16 op F64
        {{Instruction::FMul, {I16, F64}}, "__kmpc_atomic_fixed2_mul_float8"},
        {{Instruction::FDiv, {I16, F64}}, "__kmpc_atomic_fixed2_div_float8"},
        // I32 = I32 op F64
        {{Instruction::FMul, {I32, F64}}, "__kmpc_atomic_fixed4_mul_float8"},
        {{Instruction::FDiv, {I32, F64}}, "__kmpc_atomic_fixed4_div_float8"},
        // I64 = I64 op F64
        {{Instruction::FMul, {I64, F64}}, "__kmpc_atomic_fixed8_mul_float8"},
        {{Instruction::FDiv, {I64, F64}}, "__kmpc_atomic_fixed8_div_float8"},
        // I8 = I8 op F128
        {{Instruction::FAdd, {I8, F128}}, "__kmpc_atomic_fixed1_add_fp"},
        {{Instruction::FSub, {I8, F128}}, "__kmpc_atomic_fixed1_sub_fp"},
        {{Instruction::FMul, {I8, F128}}, "__kmpc_atomic_fixed1_mul_fp"},
        {{Instruction::FDiv, {I8, F128}}, "__kmpc_atomic_fixed1_div_fp"},
        {{Instruction::UDiv, {I8, F128}}, "__kmpc_atomic_fixed1u_div_fp"},
        // I16 = I16 op F128
        {{Instruction::FAdd, {I16, F128}}, "__kmpc_atomic_fixed2_add_fp"},
        {{Instruction::FSub, {I16, F128}}, "__kmpc_atomic_fixed2_sub_fp"},
        {{Instruction::FMul, {I16, F128}}, "__kmpc_atomic_fixed2_mul_fp"},
        {{Instruction::FDiv, {I16, F128}}, "__kmpc_atomic_fixed2_div_fp"},
        {{Instruction::UDiv, {I16, F128}}, "__kmpc_atomic_fixed2u_div_fp"},
        // I32 = I32 op F128
        {{Instruction::FAdd, {I32, F128}}, "__kmpc_atomic_fixed4_add_fp"},
        {{Instruction::FSub, {I32, F128}}, "__kmpc_atomic_fixed4_sub_fp"},
        {{Instruction::FMul, {I32, F128}}, "__kmpc_atomic_fixed4_mul_fp"},
        {{Instruction::FDiv, {I32, F128}}, "__kmpc_atomic_fixed4_div_fp"},
        {{Instruction::UDiv, {I32, F128}}, "__kmpc_atomic_fixed4u_div_fp"},
        // I64 = I64 op F128
        {{Instruction::FAdd, {I64, F128}}, "__kmpc_atomic_fixed8_add_fp"},
        {{Instruction::FSub, {I64, F128}}, "__kmpc_atomic_fixed8_sub_fp"},
        {{Instruction::FMul, {I64, F128}}, "__kmpc_atomic_fixed8_mul_fp"},
        {{Instruction::FDiv, {I64, F128}}, "__kmpc_atomic_fixed8_div_fp"},
        {{Instruction::UDiv, {I64, F128}}, "__kmpc_atomic_fixed8u_div_fp"},
        // F32 = F32 op F32
        {{Instruction::FAdd, {F32, F32}}, "__kmpc_atomic_float4_add"},
        {{Instruction::FSub, {F32, F32}}, "__kmpc_atomic_float4_sub"},
        {{Instruction::FMul, {F32, F32}}, "__kmpc_atomic_float4_mul"},
        {{Instruction::FDiv, {F32, F32}}, "__kmpc_atomic_float4_div"},
        // F64 = F64 op F64
        {{Instruction::FAdd, {F64, F64}}, "__kmpc_atomic_float8_add"},
        {{Instruction::FSub, {F64, F64}}, "__kmpc_atomic_float8_sub"},
        {{Instruction::FMul, {F64, F64}}, "__kmpc_atomic_float8_mul"},
        {{Instruction::FDiv, {F64, F64}}, "__kmpc_atomic_float8_div"},
        // F80 = F80 op F80
        {{Instruction::FAdd, {F80, F80}}, "__kmpc_atomic_float10_add"},
        {{Instruction::FSub, {F80, F80}}, "__kmpc_atomic_float10_sub"},
        {{Instruction::FMul, {F80, F80}}, "__kmpc_atomic_float10_mul"},
        {{Instruction::FDiv, {F80, F80}}, "__kmpc_atomic_float10_div"},
        // F128 = F128 op F128
        // TODO: Make sure using _a16 version is correct
        {{Instruction::FAdd, {F128, F128}}, "__kmpc_atomic_float16_add_a16"},
        {{Instruction::FSub, {F128, F128}}, "__kmpc_atomic_float16_sub_a16"},
        {{Instruction::FMul, {F128, F128}}, "__kmpc_atomic_float16_mul_a16"},
        {{Instruction::FDiv, {F128, F128}}, "__kmpc_atomic_float16_div_a16"},
        // F32 = F32 op F64
        {{Instruction::FAdd, {F32, F64}}, "__kmpc_atomic_float4_add_float8"},
        {{Instruction::FSub, {F32, F64}}, "__kmpc_atomic_float4_sub_float8"},
        {{Instruction::FMul, {F32, F64}}, "__kmpc_atomic_float4_mul_float8"},
        {{Instruction::FDiv, {F32, F64}}, "__kmpc_atomic_float4_div_float8"}
#if 0 // NOT Emitted in ICC
        // F32 = F32 op F128
        ,{{Instruction::FAdd, {F32, F128}}, "__kmpc_atomic_float4_add_fp"},
        {{Instruction::FSub, {F32, F128}}, "__kmpc_atomic_float4_sub_fp"},
        {{Instruction::FMul, {F32, F128}}, "__kmpc_atomic_float4_mul_fp"},
        {{Instruction::FDiv, {F32, F128}}, "__kmpc_atomic_float4_div_fp"},
        // F64 = F64 op F128
        {{Instruction::FAdd, {F64, F128}}, "__kmpc_atomic_float8_add_fp"},
        {{Instruction::FSub, {F64, F128}}, "__kmpc_atomic_float8_sub_fp"},
        {{Instruction::FMul, {F64, F128}}, "__kmpc_atomic_float8_mul_fp"},
        {{Instruction::FDiv, {F64, F128}}, "__kmpc_atomic_float8_div_fp"},
        // F80 = F80 op F128
        {{Instruction::FAdd, {F80, F128}}, "__kmpc_atomic_float10_add_fp"},
        {{Instruction::FSub, {F80, F128}}, "__kmpc_atomic_float10_sub_fp"},
        {{Instruction::FMul, {F80, F128}}, "__kmpc_atomic_float10_mul_fp"},
        {{Instruction::FDiv, {F80, F128}}, "__kmpc_atomic_float10_div_fp"}
#endif
};

const std::map<VPOParoptAtomics::AtomicOperationTy, const std::string>
    VPOParoptAtomics::ReversedOpToUpdateIntrinsicMap = {
        // I8 = I8 op I8
        {{Instruction::Sub, {I8, I8}}, "__kmpc_atomic_fixed1_sub_rev"},
        {{Instruction::SDiv, {I8, I8}}, "__kmpc_atomic_fixed1_div_rev"},
        {{Instruction::UDiv, {I8, I8}}, "__kmpc_atomic_fixed1u_div_rev"},
        {{Instruction::Shl, {I8, I8}}, "__kmpc_atomic_fixed1_shl_rev"},
        {{Instruction::AShr, {I8, I8}}, "__kmpc_atomic_fixed1_shr_rev"},
        {{Instruction::LShr, {I8, I8}}, "__kmpc_atomic_fixed1u_shr_rev"},
        // I16 = I16 op I16
        {{Instruction::Sub, {I16, I16}}, "__kmpc_atomic_fixed2_sub_rev"},
        {{Instruction::SDiv, {I16, I16}}, "__kmpc_atomic_fixed2_div_rev"},
        {{Instruction::UDiv, {I16, I16}}, "__kmpc_atomic_fixed2u_div_rev"},
        {{Instruction::Shl, {I16, I16}}, "__kmpc_atomic_fixed2_shl_rev"},
        {{Instruction::AShr, {I16, I16}}, "__kmpc_atomic_fixed2_shr_rev"},
        {{Instruction::LShr, {I16, I16}}, "__kmpc_atomic_fixed2u_shr_rev"},
        // I32 = I32 op I32
        {{Instruction::Sub, {I32, I32}}, "__kmpc_atomic_fixed4_sub_rev"},
        {{Instruction::SDiv, {I32, I32}}, "__kmpc_atomic_fixed4_div_rev"},
        {{Instruction::UDiv, {I32, I32}}, "__kmpc_atomic_fixed4u_div_rev"},
        {{Instruction::Shl, {I32, I32}}, "__kmpc_atomic_fixed4_shl_rev"},
        {{Instruction::AShr, {I32, I32}}, "__kmpc_atomic_fixed4_shr_rev"},
        {{Instruction::LShr, {I32, I32}}, "__kmpc_atomic_fixed4u_shr_rev"},
        // I64 = I64 op I64
        {{Instruction::Sub, {I64, I64}}, "__kmpc_atomic_fixed8_sub_rev"},
        {{Instruction::SDiv, {I64, I64}}, "__kmpc_atomic_fixed8_div_rev"},
        {{Instruction::UDiv, {I64, I64}}, "__kmpc_atomic_fixed8u_div_rev"},
        {{Instruction::Shl, {I64, I64}}, "__kmpc_atomic_fixed8_shl_rev"},
        {{Instruction::AShr, {I64, I64}}, "__kmpc_atomic_fixed8_shr_rev"},
        {{Instruction::LShr, {I64, I64}}, "__kmpc_atomic_fixed8u_shr_rev"},
        // F32 = F32 op F32
        {{Instruction::FSub, {F32, F32}}, "__kmpc_atomic_float4_sub_rev"},
        {{Instruction::FDiv, {F32, F32}}, "__kmpc_atomic_float4_div_rev"},
        // F64 = F64 op F64
        {{Instruction::FSub, {F64, F64}}, "__kmpc_atomic_float8_sub_rev"},
        {{Instruction::FDiv, {F64, F64}}, "__kmpc_atomic_float8_div_rev"},
        // F80 = F80 op F80
        {{Instruction::FSub, {F80, F80}}, "__kmpc_atomic_float10_sub_rev"},
        {{Instruction::FDiv, {F80, F80}}, "__kmpc_atomic_float10_div_rev"},
        // F128 = F128 op F128
        {{Instruction::FSub, {F128, F128}},
         "__kmpc_atomic_float16_sub_a16_rev"},
        {{Instruction::FDiv, {F128, F128}},
         "__kmpc_atomic_float16_div_a16_rev"}};

// Note: The entries for si = si/float and ui = ui/float are distinguished by
// using Instruction::FDiv for the first case, and Instruction::UDiv for the
// second.
const std::map<VPOParoptAtomics::AtomicOperationTy, const std::string>
    VPOParoptAtomics::OpToCaptureIntrinsicMap = {
        // I8 = I8 op I8
        {{Instruction::Add, {I8, I8}}, "__kmpc_atomic_fixed1_add_cpt"},
        {{Instruction::Sub, {I8, I8}}, "__kmpc_atomic_fixed1_sub_cpt"},
        {{Instruction::Mul, {I8, I8}}, "__kmpc_atomic_fixed1_mul_cpt"},
        {{Instruction::SDiv, {I8, I8}}, "__kmpc_atomic_fixed1_div_cpt"},
        {{Instruction::UDiv, {I8, I8}}, "__kmpc_atomic_fixed1u_div_cpt"},
        {{Instruction::And, {I8, I8}}, "__kmpc_atomic_fixed1_andb_cpt"},
        {{Instruction::Or, {I8, I8}}, "__kmpc_atomic_fixed1_orb_cpt"},
        {{Instruction::Xor, {I8, I8}}, "__kmpc_atomic_fixed1_xor_cpt"},
        {{Instruction::Shl, {I8, I8}}, "__kmpc_atomic_fixed1_shl_cpt"},
        {{Instruction::AShr, {I8, I8}}, "__kmpc_atomic_fixed1_shr_cpt"},
        {{Instruction::LShr, {I8, I8}}, "__kmpc_atomic_fixed1u_shr_cpt"},
        // I16 = I16 op I16
        {{Instruction::Add, {I16, I16}}, "__kmpc_atomic_fixed2_add_cpt"},
        {{Instruction::Sub, {I16, I16}}, "__kmpc_atomic_fixed2_sub_cpt"},
        {{Instruction::Mul, {I16, I16}}, "__kmpc_atomic_fixed2_mul_cpt"},
        {{Instruction::SDiv, {I16, I16}}, "__kmpc_atomic_fixed2_div_cpt"},
        {{Instruction::UDiv, {I16, I16}}, "__kmpc_atomic_fixed2u_div_cpt"},
        {{Instruction::And, {I16, I16}}, "__kmpc_atomic_fixed2_andb_cpt"},
        {{Instruction::Or, {I16, I16}}, "__kmpc_atomic_fixed2_orb_cpt"},
        {{Instruction::Xor, {I16, I16}}, "__kmpc_atomic_fixed2_xor_cpt"},
        {{Instruction::Shl, {I16, I16}}, "__kmpc_atomic_fixed2_shl_cpt"},
        {{Instruction::AShr, {I16, I16}}, "__kmpc_atomic_fixed2_shr_cpt"},
        {{Instruction::LShr, {I16, I16}}, "__kmpc_atomic_fixed2u_shr_cpt"},
        // I32 = I32 op I32
        {{Instruction::Add, {I32, I32}}, "__kmpc_atomic_fixed4_add_cpt"},
        {{Instruction::Sub, {I32, I32}}, "__kmpc_atomic_fixed4_sub_cpt"},
        {{Instruction::Mul, {I32, I32}}, "__kmpc_atomic_fixed4_mul_cpt"},
        {{Instruction::SDiv, {I32, I32}}, "__kmpc_atomic_fixed4_div_cpt"},
        {{Instruction::UDiv, {I32, I32}}, "__kmpc_atomic_fixed4u_div_cpt"},
        {{Instruction::And, {I32, I32}}, "__kmpc_atomic_fixed4_andb_cpt"},
        {{Instruction::Or, {I32, I32}}, "__kmpc_atomic_fixed4_orb_cpt"},
        {{Instruction::Xor, {I32, I32}}, "__kmpc_atomic_fixed4_xor_cpt"},
        {{Instruction::Shl, {I32, I32}}, "__kmpc_atomic_fixed4_shl_cpt"},
        {{Instruction::AShr, {I32, I32}}, "__kmpc_atomic_fixed4_shr_cpt"},
        {{Instruction::LShr, {I32, I32}}, "__kmpc_atomic_fixed4u_shr_cpt"},
        // I64 = I64 op I64
        {{Instruction::Add, {I64, I64}}, "__kmpc_atomic_fixed8_add_cpt"},
        {{Instruction::Sub, {I64, I64}}, "__kmpc_atomic_fixed8_sub_cpt"},
        {{Instruction::Mul, {I64, I64}}, "__kmpc_atomic_fixed8_mul_cpt"},
        {{Instruction::SDiv, {I64, I64}}, "__kmpc_atomic_fixed8_div_cpt"},
        {{Instruction::UDiv, {I64, I64}}, "__kmpc_atomic_fixed8u_div_cpt"},
        {{Instruction::And, {I64, I64}}, "__kmpc_atomic_fixed8_andb_cpt"},
        {{Instruction::Or, {I64, I64}}, "__kmpc_atomic_fixed8_orb_cpt"},
        {{Instruction::Xor, {I64, I64}}, "__kmpc_atomic_fixed8_xor_cpt"},
        {{Instruction::Shl, {I64, I64}}, "__kmpc_atomic_fixed8_shl_cpt"},
        {{Instruction::AShr, {I64, I64}}, "__kmpc_atomic_fixed8_shr_cpt"},
        {{Instruction::LShr, {I64, I64}}, "__kmpc_atomic_fixed8u_shr_cpt"},
        // F32 = F32 op F32
        {{Instruction::FAdd, {F32, F32}}, "__kmpc_atomic_float4_add_cpt"},
        {{Instruction::FSub, {F32, F32}}, "__kmpc_atomic_float4_sub_cpt"},
        {{Instruction::FMul, {F32, F32}}, "__kmpc_atomic_float4_mul_cpt"},
        {{Instruction::FDiv, {F32, F32}}, "__kmpc_atomic_float4_div_cpt"},
        // F64 = F64 op F64
        {{Instruction::FAdd, {F64, F64}}, "__kmpc_atomic_float8_add_cpt"},
        {{Instruction::FSub, {F64, F64}}, "__kmpc_atomic_float8_sub_cpt"},
        {{Instruction::FMul, {F64, F64}}, "__kmpc_atomic_float8_mul_cpt"},
        {{Instruction::FDiv, {F64, F64}}, "__kmpc_atomic_float8_div_cpt"},
        // F80 = F80 op F80
        {{Instruction::FAdd, {F80, F80}}, "__kmpc_atomic_gloat10_add_cpt"},
        {{Instruction::FSub, {F80, F80}}, "__kmpc_atomic_gloat10_sub_cpt"},
        {{Instruction::FMul, {F80, F80}}, "__kmpc_atomic_gloat10_mul_cpt"},
        {{Instruction::FDiv, {F80, F80}}, "__kmpc_atomic_gloat10_div_cpt"},
        // F128 = F128 op F128
        {{Instruction::FAdd, {F128, F128}},
         "__kmpc_atomic_float16_add_a16_cpt"},
        {{Instruction::FSub, {F128, F128}},
         "__kmpc_atomic_float16_sub_a16_cpt"},
        {{Instruction::FMul, {F128, F128}},
         "__kmpc_atomic_float16_mul_a16_cpt"},
        {{Instruction::FDiv, {F128, F128}},
         "__kmpc_atomic_float16_div_a16_cpt"}};

const std::map<VPOParoptAtomics::AtomicOperationTy, const std::string>
    VPOParoptAtomics::ReversedOpToCaptureIntrinsicMap = {
        // I8 = I8 op I8
        {{Instruction::Sub, {I8, I8}}, "__kmpc_atomic_fixed1_sub_cpt_rev"},
        {{Instruction::SDiv, {I8, I8}}, "__kmpc_atomic_fixed1_div_cpt_rev"},
        {{Instruction::UDiv, {I8, I8}}, "__kmpc_atomic_fixed1u_div_cpt_rev"},
        {{Instruction::Shl, {I8, I8}}, "__kmpc_atomic_fixed1_shl_cpt_rev"},
        {{Instruction::AShr, {I8, I8}}, "__kmpc_atomic_fixed1_shr_cpt_rev"},
        {{Instruction::LShr, {I8, I8}}, "__kmpc_atomic_fixed1u_shr_cpt_rev"},
        // I16 = I16 op I16
        {{Instruction::Sub, {I16, I16}}, "__kmpc_atomic_fixed2_sub_cpt_rev"},
        {{Instruction::SDiv, {I16, I16}}, "__kmpc_atomic_fixed2_div_cpt_rev"},
        {{Instruction::UDiv, {I16, I16}}, "__kmpc_atomic_fixed2u_div_cpt_rev"},
        {{Instruction::Shl, {I16, I16}}, "__kmpc_atomic_fixed2_shl_cpt_rev"},
        {{Instruction::AShr, {I16, I16}}, "__kmpc_atomic_fixed2_shr_cpt_rev"},
        {{Instruction::LShr, {I16, I16}}, "__kmpc_atomic_fixed2u_shr_cpt_rev"},
        // I32 = I32 op I32
        {{Instruction::Sub, {I32, I32}}, "__kmpc_atomic_fixed4_sub_cpt_rev"},
        {{Instruction::SDiv, {I32, I32}}, "__kmpc_atomic_fixed4_div_cpt_rev"},
        {{Instruction::UDiv, {I32, I32}}, "__kmpc_atomic_fixed4u_div_cpt_rev"},
        {{Instruction::Shl, {I32, I32}}, "__kmpc_atomic_fixed4_shl_cpt_rev"},
        {{Instruction::AShr, {I32, I32}}, "__kmpc_atomic_fixed4_shr_cpt_rev"},
        {{Instruction::LShr, {I32, I32}}, "__kmpc_atomic_fixed4u_shr_cpt_rev"},
        // I64 = I64 op I64
        {{Instruction::Sub, {I64, I64}}, "__kmpc_atomic_fixed8_sub_cpt_rev"},
        {{Instruction::SDiv, {I64, I64}}, "__kmpc_atomic_fixed8_div_cpt_rev"},
        {{Instruction::UDiv, {I64, I64}}, "__kmpc_atomic_fixed8u_div_cpt_rev"},
        {{Instruction::Shl, {I64, I64}}, "__kmpc_atomic_fixed8_shl_cpt_rev"},
        {{Instruction::AShr, {I64, I64}}, "__kmpc_atomic_fixed8_shr_cpt_rev"},
        {{Instruction::LShr, {I64, I64}}, "__kmpc_atomic_fixed8u_shr_cpt_rev"},
        // F32 = F32 op F32
        {{Instruction::FSub, {F32, F32}}, "__kmpc_atomic_float4_sub_cpt_rev"},
        {{Instruction::FDiv, {F32, F32}}, "__kmpc_atomic_float4_div_cpt_rev"},
        // F64 = F64 op F64
        {{Instruction::FSub, {F64, F64}}, "__kmpc_atomic_float8_sub_cpt_rev"},
        {{Instruction::FDiv, {F64, F64}}, "__kmpc_atomic_float8_div_cpt_rev"},
        // F80 = F80 op F80
        {{Instruction::FSub, {F80, F80}}, "__kmpc_atomic_float10_sub_cpt_rev"},
        {{Instruction::FDiv, {F80, F80}}, "__kmpc_atomic_float10_div_cpt_rev"},
        // F128 = F128 op F128
        {{Instruction::FSub, {F128, F128}},
         "__kmpc_atomic_float16_sub_a16_cpt_rev"},
        {{Instruction::FDiv, {F128, F128}},
         "__kmpc_atomic_float16_div_a16_cpt_rev"}};

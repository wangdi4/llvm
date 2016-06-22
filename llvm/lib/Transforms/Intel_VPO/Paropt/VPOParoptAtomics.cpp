//===- VPOParoptTransform.cpp - Transformation of W-Region for threading --===//
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

#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionUtils.h"
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
    break;
  case WRNAtomicCapture:
    break;
  default:
    llvm_unreachable("Unexpected Atomic Kind");
  }

  // TODO: If not handled, then generate critical section here.

  if (handled) {
    bool directivesCleared = WRegionUtils::stripDirectives(AtomicNode);
    assert(directivesCleared &&
           "Unable to strip directives from WRNAtomicNode.");
  }

  return handled;
}

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
  Type* OpndTy = Ptr->getType()->getContainedType(0);
  assert(OpndTy != nullptr && "Operand Type is null.");
  const std::string *Name = getAtomicRWIntrinsicName<AtomicKind>(OpndTy);
  if (Name == nullptr)
    return false; // No intrinsic found. Handle using critical sections.

  // The return type is the Type of the operand for load, and void for store.
  Type *ReturnTy = AtomicRead ? OpndTy : nullptr;

  // Now we have the Intrinsic name, its return type and ptr, val operands. The
  // loc function argument is obtained using the IdentTy struct inside genKmpcCall.
  // But we need a valid Tid, which we can load into memory using TidPtr.
  LoadInst *LoadTid = new LoadInst(TidPtr, "my.tid", Inst);
  LoadTid->setAlignment(4);
  FnArgs.insert(FnArgs.begin(), LoadTid);

  // Now try to generate the call for kmpc_atomic_rd/kmpc_atomic_wr.
  CallInst *AtomicCall = VPOParoptUtils::genKmpcCall(AtomicNode, IdentTy, Inst,
                                                     *Name, ReturnTy, FnArgs);
  assert(AtomicCall != nullptr && "Generated KMPC call is null.");

  ReplaceInstWithInst(Inst, AtomicCall);
  DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic call inserted.\n");
  return true;
}

// Private Helpers

// Does map lookup to find the atomic read/write intrinsic name for operand type
// OpndTy.
template <WRNAtomicKind AtomicKind>
const std::string *VPOParoptAtomics::getAtomicRWIntrinsicName(Type *OpndTy) {
  assert((AtomicKind == WRNAtomicRead || AtomicKind == WRNAtomicWrite) &&
         "Unsupported AtomicKind for genAtomicRWIntrinsicName");
  auto &MapToUse = (AtomicKind == WRNAtomicRead) ? TypeToReadIntrinsicMap
                                                 : TypeToWriteIntrinsicMap;

  const AtomicOperandTy Ty = {OpndTy->getTypeID(),
                              OpndTy->getPrimitiveSizeInBits()};

  auto MapEntry = MapToUse.find(Ty);

  if (MapEntry != MapToUse.end()) {
    DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic found: " << MapEntry->second
                 << "\n");
    return &(MapEntry->second);
  } else {
    DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic not found.\n");
    return nullptr;
  }
}

// Initialize intrinsic maps

// TODO: Add calls for complexes when supported.
// NOTE: Read Intrinsic for a complex takes 3 args instead of two.
// TODO: Ensure that x86 version does not need _a16 calls for F16.
const std::map<VPOParoptAtomics::AtomicOperandTy, const std::string>
    VPOParoptAtomics::TypeToReadIntrinsicMap = {
        {{Type::IntegerTyID, 8}, "__kmpc_atomic_fixed1_rd"},
        {{Type::IntegerTyID, 16}, "__kmpc_atomic_fixed2_rd"},
        {{Type::IntegerTyID, 32}, "__kmpc_atomic_fixed4_rd"},
        {{Type::PointerTyID, 32}, "__kmpc_atomic_fixed4_rd"},
        {{Type::IntegerTyID, 64}, "__kmpc_atomic_fixed8_rd"},
        {{Type::PointerTyID, 64}, "__kmpc_atomic_fixed8_rd"},
        {{Type::FloatTyID, 32}, "__kmpc_atomic_float4_rd"},
        {{Type::DoubleTyID, 64}, "__kmpc_atomic_float8_rd"},
        {{Type::X86_FP80TyID, 80}, "__kmpc_atomic_float10_rd"},
        {{Type::FP128TyID, 128}, "__kmpc_atomic_float16_a16_rd"}};

const std::map<VPOParoptAtomics::AtomicOperandTy, const std::string>
    VPOParoptAtomics::TypeToWriteIntrinsicMap = {
        {{Type::IntegerTyID, 8}, "__kmpc_atomic_fixed1_wr"},
        {{Type::IntegerTyID, 16}, "__kmpc_atomic_fixed2_wr"},
        {{Type::IntegerTyID, 32}, "__kmpc_atomic_fixed4_wr"},
        {{Type::PointerTyID, 32}, "__kmpc_atomic_fixed4_wr"},
        {{Type::IntegerTyID, 64}, "__kmpc_atomic_fixed8_wr"},
        {{Type::PointerTyID, 64}, "__kmpc_atomic_fixed8_wr"},
        {{Type::FloatTyID, 32}, "__kmpc_atomic_float4_wr"},
        {{Type::DoubleTyID, 64}, "__kmpc_atomic_float8_wr"},
        {{Type::X86_FP80TyID, 80}, "__kmpc_atomic_float10_wr"},
        {{Type::FP128TyID, 128}, "__kmpc_atomic_float16_a16_wr"}};

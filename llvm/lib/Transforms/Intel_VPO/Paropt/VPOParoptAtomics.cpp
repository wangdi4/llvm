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
// May 2016: Added support for OMP Atomic Update. (Abhinav Gaba)
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
    handled = handleAtomicUpdate(AtomicNode, IdentTy, TidPtr);
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
  assert(isa<PointerType>(Ptr->getType()) && "Unexpected type for operand.");
  Type* OpndTy = Ptr->getType()->getContainedType(0);
  assert(OpndTy != nullptr && "Operand Type is null.");
  const std::string Name = getAtomicRWIntrinsicName<AtomicKind>(Inst, OpndTy);
  if (Name.empty())
    return false; // No intrinsic found. Handle using critical sections.

  // The return type is the Type of the operand for load, and void for store.
  Type *ReturnTy = AtomicRead ? OpndTy : nullptr;

  // Now try to generate the call for kmpc_atomic_rd/kmpc_atomic_wr of type:
  //     __kmpc_atomic_fixed4_rd(loc, tid, ptr)
  //     __kmpc_atomic_fixed4_wr(loc, tid, ptr, val)
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

  // We maintain a set of Instructions that will be deleted from BB, when the
  // KMPC call is added. These will be:
  // (i)   load of `atomic_opnd` from memory.
  // (ii)  cast of loaded `atomic_opnd` to another type (if present).
  // (iii) the binary operation which calculates the result.
  // (iv)  cast of the result of the binary operation back to the type of
  //       `atomic_opnd`, before its store to memory (if present).
  Instruction *InstsToDelete[4];
  unsigned DeleteCount = 0;

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
  BinaryOperator *BinOp;
  Value *AtomicOpnd, *ValueOpnd;

  AtomicOpnd = OpndStore->getPointerOperand();

  // Now, we have AtomicOpnd. The value being stored at AtomicOpnd is the result
  // of the BinOp instruction. But to get to the BinOp, we need to strip
  // off any casts, if present.
  Value *OpResult = OpndStore->getValueOperand();
  auto OpResultToAtomicOpndTyCast = dyn_cast<CastInst>(OpResult);
  if (OpResultToAtomicOpndTyCast != nullptr) {
    OpResult = OpResultToAtomicOpndTyCast->getOperand(0);
    InstsToDelete[DeleteCount++] = OpResultToAtomicOpndTyCast; // (iv)
  }

  BinOp = dyn_cast<BinaryOperator>(OpResult);
  assert(BinOp != nullptr && "Cannot find BinOp for Atomic Update.");
  DEBUG(dbgs() << __FUNCTION__ << ": Op Instruction: " << *BinOp << "\n");
  InstsToDelete[DeleteCount++] = BinOp; // (iii)

  // Now, we have the operation. One of its two operands should be a load from
  // AtomicOpnd, or a cast of that loaded value to BinOp's operand type.
  // Considering that, the other operand should be ValueOpnd.
  unsigned Idx;
  for (Idx = 0; Idx <= 1; ++Idx) {
    Value *OpOpnd = BinOp->getOperand(Idx);
    auto AtomicOpndLoadToOpTyCast = dyn_cast<CastInst>(OpOpnd);
    if (AtomicOpndLoadToOpTyCast != nullptr)
      OpOpnd = AtomicOpndLoadToOpTyCast->getOperand(0);

    auto *OpndLoad = dyn_cast<LoadInst>(OpOpnd);
    if (OpndLoad == nullptr)
      continue;

    if (OpndLoad->getPointerOperand() == AtomicOpnd) {
      InstsToDelete[DeleteCount++] = OpndLoad; // (i)
      if (AtomicOpndLoadToOpTyCast != nullptr)
        InstsToDelete[DeleteCount++] = AtomicOpndLoadToOpTyCast; // (ii)
      break;
    }
  }
  assert(Idx <= 1 && "Atomic operand not found in BinOp Inst.");
  // The operand at position `Idx` is the loaded AtomicOpnd (or its cast to
  // another data type). This means, the operand at position `1-Idx` is
  // ValueOpnd.
  ValueOpnd = BinOp->getOperand(1 - Idx);
  assert(ValueOpnd != nullptr && "ValueOpnd is null.");
  // Also, if BinOp is a non-commutative operation, like div and sub, and
  // AtomicOpnd is the second operand, then different intrinsic calls are needed
  // to handle them.
  const bool Reversed = Idx == 1 && !BinOp->isCommutative();

  // At this point, there is a special case we need to handle. When AtomicOpnd
  // is an integer, and ValueOpnd is a float, we might need to FPExtend the
  // ValueOpnd.
  CastInst *FPExtValueOpnd =
      genFPExtForNonAtomicOpnd(BinOp, AtomicOpnd, ValueOpnd);
  ValueOpnd = FPExtValueOpnd != nullptr ? FPExtValueOpnd : ValueOpnd;
  // Note that we have not yet inserted this Cast into the IR. We do that only
  // after we find a matching intrinsic.

  // Now we know the atomic operand, the value operand, and the operation. We
  // now check whether an intrinsic exists which supports the given combination
  // of operation and operands.
  DEBUG(dbgs() << __FUNCTION__ << ": Atomic Opnd: " << *AtomicOpnd << "\n");
  DEBUG(dbgs() << __FUNCTION__ << ": Value Opnd: " << *ValueOpnd << "\n");
  std::string Name =
      getAtomicUpdateIntrinsicName(Reversed, BinOp, AtomicOpnd, ValueOpnd);
  if (Name.empty()) {
    if (FPExtValueOpnd != nullptr)
      delete FPExtValueOpnd;
    // No intrinsic found. Handle using critical sections.
    return false;
  }

  // We found the matching intrinsic. So, it's safe to insert FPExtValueOpnd
  // into the IR.
  if (FPExtValueOpnd != nullptr)
    FPExtValueOpnd->insertBefore(OpndStore);

  // Next, generate and insert the KMPC call for atomic update. It looks like:
  //     call void __kmpc_atomic_<...>(loc, tid, atomic_opnd, value_opnd)
  CallInst *AtomicCall = VPOParoptUtils::genKmpcCallWithTid(
      AtomicNode, IdentTy, TidPtr, OpndStore, Name, nullptr,
      {AtomicOpnd, ValueOpnd});
  assert(AtomicCall != nullptr && "Generated KMPC call is null.");

  ReplaceInstWithInst(OpndStore, AtomicCall);
  DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic call inserted.\n");

  // And finally, delete the instructions that are no longer needed.
  for (unsigned I = 0; I < DeleteCount; ++I) {
    Instruction *Inst = InstsToDelete[I];
    Inst->replaceAllUsesWith(UndefValue::get(Inst->getType()));
    Inst->eraseFromParent();
  }

  return true;
}

// Private Helper Methods.

// Generates an FPExt cast for ValueOpnd for Atomic Update, if needed.
CastInst *VPOParoptAtomics::genFPExtForNonAtomicOpnd(Instruction *Op,
                                                     Value *AtomicOpnd,
                                                     Value *ValueOpnd) {
  assert(Op != nullptr && "Op is null.");
  assert(isa<BinaryOperator>(Op) && "Unsupported Op type.");
  assert(AtomicOpnd != nullptr && "AtomicOpnd is null.");
  assert(!AtomicOpnd->use_empty() && "No uses of AtomicOpnd.");
  assert(ValueOpnd != nullptr && "ValueOpnd is null.");

  Type *ValueOpndTy = ValueOpnd->getType();
  Type *AtomicOpndTy = AtomicOpnd->getType();
  assert(AtomicOpndTy->isPointerTy() && "Unexpected type for Atomic operand.");

  if (!AtomicOpndTy->getContainedType(0)->isIntegerTy() ||        // Ptr to Int
      !(ValueOpndTy->isFloatTy() || ValueOpndTy->isX86_FP80Ty())) // F32 or F80
    return nullptr;

  DEBUG(dbgs() << __FUNCTION__
               << ": Generating FPExt for ValueOpnd: " << *ValueOpnd << "\n");

  BasicBlock *B = Op->getParent();
  Function *F = B->getParent();
  LLVMContext &C = F->getContext();

  if (ValueOpndTy->isFloatTy() &&
      // We generate a cast to double (64-bit FP) for mul and div of signed
      // integer with float(32-bit)
      ((Op->getOpcode() == Instruction::FMul ||  // <signed int> mul <fp>
        (Op->getOpcode() == Instruction::FDiv && // <signed int> div <fp>
         !isUIToFPCast(*(AtomicOpnd->use_begin()))))))
    return CastInst::CreateFPCast(ValueOpnd, Type::getDoubleTy(C),
                                  "fpext.value.opnd");

  // For unsigned div and other operations for 32-bit float, and for 80-bit
  // X86_FP80Ty, we generate a cast to 128-bit float.
  return CastInst::CreateFPCast(ValueOpnd, Type::getFP128Ty(C),
                                "fpext.value.opnd");
}

// Tells whether Val is a CastInst of type UIToFP.
bool VPOParoptAtomics::isUIToFPCast(Value *Val) {
  assert(Val != nullptr && "Val is null");

  CastInst *Cast = dyn_cast<CastInst>(Val);
  if (Cast != nullptr && Cast->getOpcode() == Instruction::UIToFP)
    return true;

  return false;
}

// Functions for intrinsic name lookup.

// Does map lookup to find the atomic read/write intrinsic name.
template <WRNAtomicKind AtomicKind>
const std::string
VPOParoptAtomics::getAtomicRWIntrinsicName(Instruction *OpInst, Type *OpndTy) {
  assert((AtomicKind == WRNAtomicRead || AtomicKind == WRNAtomicWrite) &&
         "Unsupported AtomicKind for genAtomicRWIntrinsicName");
  assert(OpInst != nullptr && "Op Inst is null");
  assert(OpndTy != nullptr && "OpndTy is null.");
  auto &MapToUse = (AtomicKind == WRNAtomicRead) ? TypeToReadIntrinsicMap
                                                 : TypeToWriteIntrinsicMap;

  const AtomicOperandTy Ty = {OpndTy->getTypeID(),
                              OpndTy->getPrimitiveSizeInBits()};

  auto MapEntry = MapToUse.find(Ty);

  if (MapEntry == MapToUse.end()) {
    DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic not found.\n");
    return std::string();
  }

  DEBUG(dbgs() << __FUNCTION__ << ": Intrinsic found: " << MapEntry->second
               << "\n");
  return adjustIntrinsicNameForArchitecture(OpInst, MapEntry->second);
}

// Does map lookup to find the atomic update intrinsic name.
const std::string VPOParoptAtomics::getAtomicUpdateIntrinsicName(
    bool Reversed, Instruction *Operation, Value *AtomicOpnd,
    Value *ValueOpnd) {
  assert(Operation != nullptr && "Operation is null.");
  assert(AtomicOpnd != nullptr && "AtomicOpnd is null.");
  assert(ValueOpnd != nullptr && "ValueOpnd is null.");
  assert((!Reversed || !Operation->isCommutative()) &&
         "Unexpected Reversed flag for commutative operation.");

  assert(isa<PointerType>(AtomicOpnd->getType()) && "Invalid AtomicOpnd.");
  Type *AtomicOpndType = AtomicOpnd->getType()->getContainedType(0);
  Type *ValueOpndType = ValueOpnd->getType();
  assert(AtomicOpndType != nullptr && "AtomicOpndTy is null");
  assert(ValueOpndType != nullptr && "ValueOpndTy is null");

  auto &MapToUse =
      Reversed ? ReversedOpToUpdateIntrinsicMap : OpToUpdateIntrinsicMap;

  // We need the operand types and the operation's op code to do a map lookup.
  const AtomicOperandTy AtomicOpndTy = {
      AtomicOpndType->getTypeID(), AtomicOpndType->getPrimitiveSizeInBits()};
  const AtomicOperandTy ValueOpndTy = {ValueOpndType->getTypeID(),
                                       ValueOpndType->getPrimitiveSizeInBits()};

  unsigned OpCode = Operation->getOpcode();
  // Special case:
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
  if (OpCode == Instruction::FDiv && ValueOpndTy == F128) {
    assert(!AtomicOpnd->use_empty() && "AtomicOpnd has no uses.");
    if (isUIToFPCast(*(AtomicOpnd->use_begin())))
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
  return adjustIntrinsicNameForArchitecture(Operation, MapEntry->second);
}

const std::string VPOParoptAtomics::adjustIntrinsicNameForArchitecture(
    Instruction *Inst, const std::string &IntrinsicName) {

  assert(Inst != nullptr && "Inst is null.");
  assert(!IntrinsicName.empty() && "Intrinsic name is empty");

  // If IntrinsicName is not an '_a16' version, return as is.
  size_t A16StartingPosition = IntrinsicName.find("_a16");
  if (A16StartingPosition == std::string::npos)
    return IntrinsicName;

  // Otherwise, find the architecture.
  BasicBlock *B = Inst->getParent();
  Function *F = B->getParent();
  Module *M = F->getParent();
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

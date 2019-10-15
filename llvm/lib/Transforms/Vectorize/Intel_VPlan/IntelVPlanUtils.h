//===---IntelVPlanUtils.h - General Utilities for VPlan and LoadCoalescing-==//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANUTILS_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANUTILS_H

#include "IntelVPlan.h"
#include "llvm/Analysis/MemoryLocation.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"

namespace llvm {
namespace vpo {

/// \returns true if \p I is a memory instruction.
// NOTE: This is copied from GVNSink.cpp
inline bool isMemoryInst(const Instruction *I) {
  return isa<LoadInst>(I) || isa<StoreInst>(I) ||
         (isa<InvokeInst>(I) && !cast<InvokeInst>(I)->doesNotAccessMemory()) ||
         (isa<CallInst>(I) && !cast<CallInst>(I)->doesNotAccessMemory());
}

/// \returns true if \p I is any instruction that can create a belong's-to, or
/// simple 'aliasing' relationship, with it's input operand.
template <typename InstTy = Instruction>
inline bool isTrivialPointerAliasingInst(const InstTy *Inst) {
  assert(Inst && "Expect a non-null input for isTrivialPointerAliasingInst");
  return (Inst->getOpcode() == Instruction::BitCast ||
          Inst->getOpcode() == Instruction::AddrSpaceCast ||
          Inst->getOpcode() == Instruction::GetElementPtr ||
          Inst->getOpcode() == Instruction::PHI);
}

/// \returns the vector bit-size of \p LI .
inline size_t getVecBits(Instruction *LI, const DataLayout &DL,
                         bool AllowScalars = false) {
  assert((AllowScalars || isa<VectorType>(LI->getType())) &&
         "Expect this to be a Vector Instruction");
  return DL.getTypeSizeInBits(LI->getType());
}

/// \returns the scalar bit-size of \p LI .
inline size_t getScalarBits(Instruction *LI, const DataLayout &DL) {
  auto InstTy = getLoadStoreType(LI);
  Type *ScalarTy = isa<VectorType>(InstTy)
                       ? cast<VectorType>(InstTy)->getVectorElementType()
                       : InstTy;

  size_t Bits = DL.getTypeSizeInBits(ScalarTy);
  return Bits;
}

/// \returns the number of vector elements of \p Ty. It returns 1 if \Ty is
/// scalar.
inline size_t getNumElementsSafe(Type *Ty) {
  return (isa<VectorType>(Ty) ? cast<VectorType>(Ty)->getNumElements() : 1);
}

/// \returns the memory location that is being access by the instruction.
inline MemoryLocation getLocation(Instruction *I) {
  if (StoreInst *SI = dyn_cast<StoreInst>(I))
    return MemoryLocation::get(SI);
  if (LoadInst *LI = dyn_cast<LoadInst>(I))
    return MemoryLocation::get(LI);
  return MemoryLocation();
}

/// \returns true if the instruction is not a volatile or atomic load/store.
inline bool isVolatileOrAtomic(Instruction *I) {
  if (LoadInst *LI = dyn_cast<LoadInst>(I))
    return !LI->isSimple();
  if (StoreInst *SI = dyn_cast<StoreInst>(I))
    return !SI->isSimple();
  if (MemIntrinsic *MI = dyn_cast<MemIntrinsic>(I))
    return MI->isVolatile();
  return true;
}

/// \returns true if it is a memory RAW or WAR dependence. This is handy for
/// LoadCoalescing. In case of coalescing stores, we might want to add a check
/// for WAW dependencies.
inline bool isMemoryDependency(const Instruction *SrcI,
                               const Instruction *DstI) {
  bool RAW = SrcI->mayWriteToMemory() && DstI->mayReadFromMemory();
  bool WAR = SrcI->mayReadFromMemory() && DstI->mayWriteToMemory();
  return RAW || WAR;
}

/////////// VPValue version of common LLVM load/store utilities ///////////

/// A helper function that returns the pointer operand of a load or store
/// VPInstruction. Returns nullptr if not load or store.
inline VPValue *getLoadStorePointerOperand(const VPValue *V) {
  if (auto *VPInst = dyn_cast<VPInstruction>(V)) {
    if (VPInst->getOpcode() == Instruction::Load)
      return VPInst->getOperand(0);
    if (VPInst->getOpcode() == Instruction::Store)
      return VPInst->getOperand(1);
  }
  return nullptr;
}

/// Helper function to return type of operand based on whether it is a
/// load/store VPInstruction.
inline Type *getLoadStoreType(const VPInstruction *VPI) {
  if (VPI->getOpcode() == Instruction::Load)
    return VPI->getType();
  else if (VPI->getOpcode() == Instruction::Store)
    return VPI->getOperand(0)->getType();
  llvm_unreachable("Expected Load or Store VPI.");
}

/// Helper function to return pointer operand for a VPInstruction representing
/// load, store or GEP.
inline VPValue *getPointerOperand(VPInstruction *VPI) {
  if (auto *Ptr = getLoadStorePointerOperand(VPI))
    return Ptr;
  if (auto *Gep = dyn_cast<VPGEPInstruction>(VPI))
    return Gep->getOperand(0);
  return nullptr;
}

/// Helper function that returns the address space of the pointer operand of
/// load or store VPInstruction.
inline unsigned getLoadStoreAddressSpace(VPInstruction *VPI) {
  assert((VPI->getOpcode() == Instruction::Load ||
          VPI->getOpcode() == Instruction::Store) &&
         "Expect 'VPI' to be either a LoadInst or a StoreInst");
  return getPointerOperand(VPI)->getType()->getPointerAddressSpace();
}

/// Helper function to get preferred alignment for a VPInstruction representing
/// load/store.
inline unsigned getLoadStoreAlignment(VPInstruction *VPInst, Loop *L) {
  assert((VPInst->getOpcode() == Instruction::Load ||
          VPInst->getOpcode() == Instruction::Store) &&
         "Expect 'VPInst' to be either a LoadInst or a StoreInst");

  VPValue *Ptr = getLoadStorePointerOperand(VPInst);

  Type *PtrType = nullptr;
  // Ptr can be a GEP or the alloca directly
  if (isa<VPGEPInstruction>(Ptr)) {
    // First operand of GEP will be alloca
    PtrType = cast<VPGEPInstruction>(Ptr)->getOperand(0)->getType();
  } else {
    PtrType = Ptr->getType();
  }

  const DataLayout &DL = L->getHeader()->getModule()->getDataLayout();
  return DL.getPrefTypeAlignment(PtrType);
}

/////////// VPValue version of common LLVM CallInst utilities ///////////

/// Get the called Function for given VPInstruction representing a call.
inline Function *getCalledFunction(const VPInstruction *Call) {
  assert(Call->getOpcode() == Instruction::Call &&
         "getCalledFunction called on non-call VPInstruction,");
  // The called function will always be the last operand.
  VPValue *FuncOp = Call->getOperand(Call->getNumOperands() - 1);
  auto *Func = dyn_cast<VPConstant>(FuncOp);
  if (!Func)
    // Indirect function call (function pointers).
    return nullptr;

  assert(isa<Function>(Func->getConstant()) &&
         "Underlying value for function operand is not Function.");
  return cast<Function>(Func->getConstant());
}

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANUTILS_H

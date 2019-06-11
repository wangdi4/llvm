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
bool isVolatileOrAtomic(Instruction *I) {
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
bool isMemoryDependency(const Instruction *SrcI, const Instruction *DstI) {
  bool RAW = SrcI->mayWriteToMemory() && DstI->mayReadFromMemory();
  bool WAR = SrcI->mayReadFromMemory() && DstI->mayWriteToMemory();
  return RAW || WAR;
}

} // namespace vpo
} // namespace llvm

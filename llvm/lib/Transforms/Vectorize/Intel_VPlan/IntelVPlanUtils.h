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
#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLInst.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"
#endif // INTEL_CUSTOMIZATION
#include <iterator>

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

  // In case of VPInstructions, we can have aliasing on account of InductionInit
  // instruction as well.
  if (std::is_same<InstTy, VPInstruction>::value)
    if (Inst->getOpcode() == VPInstruction::InductionInit)
      return true;

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
  auto InstVecTy = dyn_cast<VectorType>(InstTy);
  Type *ScalarTy = InstVecTy ? InstVecTy->getElementType() : InstTy;

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

/// \returns true if the instruction is a volatile or atomic load/store.
inline bool isVolatileOrAtomic(Instruction *I) {
  if (LoadInst *LI = dyn_cast<LoadInst>(I))
    return !LI->isSimple();
  if (StoreInst *SI = dyn_cast<StoreInst>(I))
    return !SI->isSimple();
  if (MemIntrinsic *MI = dyn_cast<MemIntrinsic>(I))
    return MI->isVolatile();
  return false;
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

/// Helper function to identify Types that are valid for vectorization.
/// AggregateType like structs and arrays cannot be represented in vectors, such
/// operations are expected to be serialized in vector loops.
inline bool isVectorizableTy(Type *Ty) {
  if (auto *VecTy = dyn_cast<VectorType>(Ty))
    return VecTy->getElementType()->isSingleValueType();

  if (!Ty->isVoidTy())
    return VectorType::isValidElementType(Ty);

  // VoidTy is trivial case.
  return true;
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
/// load, store, GEP or subscript.
inline VPValue *getPointerOperand(const VPInstruction *VPI) {
  if (auto *Ptr = getLoadStorePointerOperand(VPI))
    return Ptr;
  if (auto *Gep = dyn_cast<VPGEPInstruction>(VPI))
    return Gep->getPointerOperand();
  if (auto *Subscript = dyn_cast<VPSubscriptInst>(VPI))
    return Subscript->getPointerOperand();
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

#if INTEL_CUSTOMIZATION
// Obtain stride information using loopopt interfaces. HNode is expected to
// specify the underlying node for a load/store VPInstruction. We return false
// if this is not the case. Function returns true if the memory reference
// corresponding to pointer operand of load/store VPInst has known stride. The
// stride value is returned in Stride. Function returns false for unknown
// stride.
inline bool getStrideUsingHIR(const loopopt::HLNode &HNode, int64_t &Stride) {
  const loopopt::HLInst *HInst = dyn_cast<loopopt::HLInst>(&HNode);
  if (!HInst)
    return false;

  const loopopt::RegDDRef *MemRef =
      HInst->getLLVMInstruction()->getOpcode() == Instruction::Load
          ? HInst->getRvalDDRef()
          : HInst->getLvalDDRef();

  // Memref is expected to be non-null and a memory reference. If this is not
  // the case, return false. We assert in debug mode.
  if (!MemRef || !MemRef->isMemRef()) {
    assert(false && "Unexpected null or non-memory ref");
    return false;
  }

  const loopopt::HLLoop *HLoop = HInst->getParentLoop();
  // The code here assumes inner loop vectorization. TODO: Once we start
  // supporting outer loop vectorization, this interface will need to be changed
  // to also take the HLLoop being vectorized.
  assert(HLoop && HLoop->isInnermost() &&
         "Outerloop vectorization is not supported.");

  return MemRef->getConstStrideAtLevel(HLoop->getNestingLevel(), &Stride);
}
#endif // INTEL_CUSTOMIZATION

// Add a new depth-first iterator (sese_df_iterator) for traversing the blocks
// of SESE region.
template <typename BlockTy>
class sese_df_iterator
    : public std::iterator<std::forward_iterator_tag, BlockTy> {

public:
  sese_df_iterator(df_iterator<BlockTy> Iter, BlockTy End)
      : Iter(Iter), End(End) {}

  sese_df_iterator &operator++() {
    if (*Iter == End)
      // Don't go outside of SESE region. It does move the iterator, so avoid
      // usual increment.
      Iter.skipChildren();
    else
      // Go to the next block in ordinary way.
      ++Iter;
    return *this;
  }

  BlockTy const &operator*() const { return *Iter; }

  bool operator!=(const sese_df_iterator &It2) const {
    return Iter != It2.Iter;
  }

  bool operator==(const sese_df_iterator &It2) const {
    return Iter == It2.Iter;
  }

private:
  df_iterator<BlockTy> Iter;
  BlockTy End;
};

template <typename BlockTy>
sese_df_iterator<BlockTy> sese_df_begin(BlockTy Begin, BlockTy End) {
  return sese_df_iterator<BlockTy>(df_begin(Begin), End);
}

template <typename BlockTy>
sese_df_iterator<BlockTy> sese_df_end(BlockTy Begin, BlockTy End) {
  return sese_df_iterator<BlockTy>(df_end(Begin), End);
}

// Provide an accessor that can be used in ranged patterns.
template <typename BlockTy>
iterator_range<sese_df_iterator<BlockTy>> sese_depth_first(BlockTy Begin,
                                                           BlockTy End) {
  return make_range(sese_df_begin(Begin, End), sese_df_end(Begin, End));
}
} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANUTILS_H

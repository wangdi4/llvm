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
    if (Inst->getOpcode() == VPInstruction::InductionInit ||
        Inst->getOpcode() == VPInstruction::Subscript)
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
  size_t Bits = DL.getTypeSizeInBits(InstTy->getScalarType());
  return Bits;
}

/// \returns the number of vector elements of \p Ty. It returns 1 if \Ty is
/// scalar.
inline size_t getNumElementsSafe(Type *Ty) {
  return (isa<FixedVectorType>(Ty) ?
            cast<FixedVectorType>(Ty)->getNumElements() : 1);
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

/// \returns true if it is a memory RAW, WAR or WAW dependence.
inline bool isMemoryDependency(const Instruction *SrcI,
                               const Instruction *DstI) {
  bool RAW = SrcI->mayWriteToMemory() && DstI->mayReadFromMemory();
  bool WAR = SrcI->mayReadFromMemory() && DstI->mayWriteToMemory();
  bool WAW = SrcI->mayWriteToMemory() && DstI->mayWriteToMemory();
  return RAW || WAR || WAW;
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

/// \returns true if \p Ty's pointee-type is a scalar.
inline bool isScalarTy(Type *Ty) {
  assert(Ty && "Expect a non-null argument to isScalarTy function.");
  return (!(Ty->isAggregateType() || Ty->isVectorTy()));
}

// Return true if the divisor is safe for integer div/rem.
// Non-constants are considered as unsafe. The known unsafe constant values are:
// 0 for all and -1 for signed div/rem, (INT_MIN / -1) raises an exception.
inline bool isDivisorSpeculationSafeForDivRem(unsigned Opcode, VPValue *Div) {
  bool IsSigned;
  switch (Opcode) {
  case Instruction::UDiv:
  case Instruction::URem:
    IsSigned = false;
    break;
  case Instruction::SDiv:
  case Instruction::SRem:
    IsSigned = true;
    break;
  default:
    llvm_unreachable("Unexpected opcode");
    return false;
  }

  auto *Const = dyn_cast<VPConstantInt>(Div);
  if (!Const)
    return false;

  int64_t Val = Const->getSExtValue();
  return Val != 0 && (!IsSigned || Val != -1);
}

/////////// VPValue version of common LLVM load/store utilities ///////////

/// Helper function to return pointer operand for a VPInstruction representing
/// load, store, GEP or subscript.
inline VPValue *getPointerOperand(const VPInstruction *VPI) {
  if (auto *LoadStore = dyn_cast<VPLoadStoreInst>(VPI))
    return LoadStore->getPointerOperand();
  if (auto *Gep = dyn_cast<VPGEPInstruction>(VPI))
    return Gep->getPointerOperand();
  if (auto *Subscript = dyn_cast<VPSubscriptInst>(VPI))
    return Subscript->getPointerOperand();
  return nullptr;
}

/// Helper function to determine if given VPValue \p V is a vectorizable
/// load/store. A load/store is not vectorizable if it's not simple or if it
/// operates on non-vectorizable types.
inline bool isVectorizableLoadStore(const VPValue *V) {
  auto *VPLoadStore = dyn_cast<VPLoadStoreInst>(V);
  if (!VPLoadStore)
    return false;

  // TODO: Load/store to struct types can be potentially vectorized by doing a
  // wide load/store followed by shuffle + bitcast.
  if (!isVectorizableTy(VPLoadStore->getValueType()))
    return false;

  return VPLoadStore->isSimple();
}

/// Helper function that returns true if the given type is irregular for unit
/// stride accesses. The type is irregular if vector <VF x Ty> is not bitcast
/// compatible with an array of Ty containing VF elements. This is typically
/// the case when a type's allocated size and type's size do not match.
/// Note: Community LV code has a similar function(hasIrregularType). However,
/// it does not work with vector types, the check is VF specific and it also
/// returns incorrect results for types such as i7
inline bool hasIrregularTypeForUnitStride(Type *Ty, const DataLayout *DL) {
  // We can't do unit-stride access optimization if Ty's allocated size(in
  // bits) does not match its size since it implies implicit padding. Check
  // https://godbolt.org/z/7e1r1j. For example -
  // Type         SizeInBits  StoreSizeInBits  AllocSizeInBits
  // ----         ----------  ---------------  ---------------
  // <3 x i32>        96           96               128
  //
  // TODO: This bailout is too conservative since vectorizer codegen can
  // generate optimal unit-stride accesses for types such as above where we
  // have full padding bytes by masking out lanes that access such padding
  // bytes. Check JIRA - CMPLRLLVM-22929.
  //
  // NOTE: We also have types such as i1/i2/i7 whose alloc size and store size
  // are the same (1-byte) but whose type size is 1/2/7 bits respectively with
  // padding bits 7/6/1. However, vectors of such types say <2 x i1> also
  // have alloc/store size of 1-byte i.e. the vectors are written to memory
  // without any padding bits in between the vector elements. We also need to
  // treat such types as irregular. Type irregularity thus compares type size
  // and type alloc size in bits.
  //
  // Type         SizeInBits  StoreSizeInBits  AllocSizeInBits
  // ----         ----------  ---------------  ---------------
  // i1               1            8                8

  return DL->getTypeAllocSizeInBits(Ty) != DL->getTypeSizeInBits(Ty);
}

/// Given a type \p GroupTy of a wide VLS-optimized operation, and a type \p
/// ValueTy corresponding to an element, calculate how many group's base
/// elements are fit into the value.
inline unsigned getNumGroupEltsPerValue(const DataLayout &DL, Type *GroupTy,
                                        Type *ValueTy) {
  auto ValueTypeSize = DL.getTypeSizeInBits(ValueTy);
  Type *GroupEltType = cast<VectorType>(GroupTy)->getElementType();
  auto GroupElementTypeSize = DL.getTypeSizeInBits(GroupEltType);

  assert(ValueTypeSize % GroupElementTypeSize == 0 &&
         "Group element type is invalid!");
  return ValueTypeSize / GroupElementTypeSize;
}

/// Helper function to check if VPValue is a private memory pointer that was
/// allocated by VPlan. The implementation also checks for any aliases obtained
/// via casts, gep and PHI-instructions.
// TODO: Check if this utility is still relevant after data layout
// representation is finalized in VPlan.
inline const VPValue *getVPValuePrivateMemoryPtr(const VPValue *V) {

  // Early quick-check to seen if this is a VPAllocatePrivte.
  if (isa<VPAllocatePrivate>(V))
    return V;

  SmallVector<const VPValue *, 20> WL;
  SmallPtrSet<const VPValue *, 20> Visited;
  WL.push_back(V);

  while (!WL.empty()) {
    const VPValue *CurrentI = WL.pop_back_val();

    // If we encounter VPAllocatePrivate, we have reached the end and return the
    // instruction.
    if (isa<VPAllocatePrivate>(CurrentI))
      return CurrentI;

    // If this instruction/value has been incountered before, continue.
    if (!Visited.insert(CurrentI).second)
      continue;

    // Check that it is a valid transform of private memory's address, by
    // recurring into operand.
    if (auto *VPI = dyn_cast<VPInstruction>(CurrentI))
      if (VPI->isCast() || isa<VPGEPInstruction>(VPI) ||
          isa<VPSubscriptInst>(VPI))
        WL.push_back(VPI->getOperand(0));

    // This can be a PHI instruction.
    if (auto *PHI = dyn_cast<VPPHINode>(CurrentI)) {
      for (auto *InVal : PHI->incoming_values())
        WL.push_back(InVal);
    }
  }
  // All checks failed.
  return nullptr;
}

inline Type *getInt8OrPointerElementTy(Type *ValTy) {
  assert(ValTy->isPointerTy() && "Expected Pointer type");
  if (ValTy->isOpaquePointerTy())
    return Type::getInt8Ty(ValTy->getContext());
  return ValTy->getPointerElementType();
}

#if INTEL_CUSTOMIZATION
// Obtain stride information using loopopt interfaces for the given memory
// reference MemRef. DDNode specifies the underlying HLDDNode for the
// load/store VPInstruction. Function returns true if the given memory
// reference has known stride. The stride value is returned in Stride.
// Function returns false for unknown stride.
inline bool getStrideUsingHIR(const loopopt::RegDDRef *MemRef,
                              const loopopt::HLDDNode &DDNode,
                              int64_t &Stride) {

  // Memref is expected to be non-null and a memory reference. If this is not
  // the case, return false. We assert in debug mode.
  if (!MemRef || !MemRef->isMemRef()) {
    assert(false && "Unexpected null or non-memory ref");
    return false;
  }

  assert(MemRef->getHLDDNode() == &DDNode &&
         "MemRef expected to be a ref in DDNode");
  const loopopt::HLLoop *HLoop = DDNode.getParentLoop();
  // The code here assumes inner loop vectorization. TODO: Once we start
  // supporting outer loop vectorization, this interface will need to be changed
  // to also take the HLLoop being vectorized.
  assert(HLoop && HLoop->isInnermost() &&
         "Outerloop vectorization is not supported.");

  return MemRef->getConstStrideAtLevel(HLoop->getNestingLevel(), &Stride);
}

// This function adds new SzAddMD metadata string to the loop. We use it to set
// llvm.loop.vectorize.enable and llvm.loop.isvectorized metadata attributes:
//   llvm.loop.vectorize.enable - is added by the front-end (called without
//   -fiopenmp option) or VPlan Vectorizer if pragma simd is specified on
//   the loop.
//   llvm.loop.isvectorized - is added by vectorizer for vectorized loops.
inline void setLoopMD(const Loop *const Lp, const char *const SzAddMD) {
  if (!Lp)
    return;
  LLVMContext &Context = Lp->getHeader()->getContext();
  MDNode *AddMD = MDNode::get(
      Context,
      {MDString::get(Context, SzAddMD),
       ConstantAsMetadata::get(ConstantInt::get(Context, APInt(32, 1)))});
  MDNode *LoopID = Lp->getLoopID();
  MDNode *NewLoopID =
      makePostTransformationMetadata(Context, LoopID, {SzAddMD}, {AddMD});
  Lp->setLoopID(NewLoopID);
}

// Same functionality as above method - specialized for HIR loops.
inline void setHLLoopMD(loopopt::HLLoop *Lp, const char *SzAddMD) {
  if (!Lp)
    return;
  LLVMContext &Context = Lp->getHLNodeUtils().getContext();
  MDNode *AddMD = MDNode::get(
      Context,
      {MDString::get(Context, SzAddMD),
       ConstantAsMetadata::get(ConstantInt::get(Context, APInt(32, 1)))});
  Lp->addLoopMetadata({AddMD});
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

// Return true if VPInst is equivalent to &ptrOp[0]. Currently, this is
// the case if we are dealing with a VPSubscriptInst with no dimensions.
// TODO - the issue can also arise for GEPs and we need to account for
// the same.
static bool isSelfAddressOfInst(const VPValue *VPVal) {
  if (auto *VPSubscrInst = dyn_cast<VPSubscriptInst>(VPVal))
    return VPSubscrInst->isSelfAddressOfInst();

  return false;
}

static const VPValue *getPtrThroughCast(const VPValue *Op) {
  // Look at the pointers only.
  assert(Op->getType()->isPointerTy() && "expected pointer");
  while (isa<VPInstruction>(Op)) {
    auto Inst = cast<VPInstruction>(Op);

    // We also treat SelfAddressOfInst as a no-op cast.
    if (Inst->getOpcode() != Instruction::BitCast &&
        Inst->getOpcode() != Instruction::AddrSpaceCast &&
        !isSelfAddressOfInst(Inst))
      break;
    Op = Inst->getOperand(0);
  }

  return Op;
}

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANUTILS_H

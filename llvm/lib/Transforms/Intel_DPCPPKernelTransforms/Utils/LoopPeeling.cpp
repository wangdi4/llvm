//===- LoopPeeling.h - Loop peeling utilities ----------------------*- C++-===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/LoopPeeling.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

#include <numeric>

#define DEBUG_TYPE "dpcpp-kernel-loop-peeling"

namespace llvm {

/// This function is copied from IntelVPlanAlignmentAnalysis.cpp.
/// Modular multiplicative inverse.
static int modularMultiplicativeInverse(int Val, int Mod) {
  assert(Val > 0 && Mod > Val && "Invalid Arguments");
  assert(std::gcd(Val, Mod) == 1 && "Arguments are not coprime");

  int Prev = 1;
  int Curr = Val;

  // Given coprime Val and Mod (Mod > Val > 0), there exists n such that
  // Van^n ≡ 1 (mod Mod).
  while (Curr != 1) {
    Prev = Curr;
    Curr = (Curr * Val) % Mod;
  }

  // Return Val^{n-1}.
  return Prev;
}

/// This function is copied from IntelVplanAlignmentAnalysis.cpp
/// Compute -a^{-1} (mod b),
/// where a = InductionStep / RequiredAlignment
///       b = TargetAlignment / RequiredAlignment
static int computeMultiplierForPeeling(int Step, Align RequiredAlignment,
                                       Align TargetAlignment) {
  assert(TargetAlignment > RequiredAlignment &&
         "Peeling makes sense only if TargetAlignment > RequiredAlignment");
  assert(Step % RequiredAlignment.value() == 0 && "Bad RequiredAligment");

  int ReqLog = Log2(RequiredAlignment);
  int TgtLog = Log2(TargetAlignment);
  int A = Step >> ReqLog;
  int B = 1 << (TgtLog - ReqLog);
  int InvA = modularMultiplicativeInverse(A % B, B);
  return B - InvA;
}

namespace LoopDynamicPeeling {

Optional<Value *> computePeelCount(BasicBlock &EntryBB, BasicBlock &VectorEntry,
                                   ArrayRef<Value *> InitGIDs,
                                   Value *MaxPeelSize) {
  // Search for load or store instruction with intel.preferred_alignment
  // metadata inside vector entry basic block.
  Instruction *PeelTarget = nullptr;
  for (Instruction &I : VectorEntry) {
    if (I.getMetadata("intel.preferred_alignment") &&
        (dyn_cast<StoreInst>(&I) || dyn_cast<LoadInst>(&I))) {
      PeelTarget = &I;
      break;
    }
  }
  if (!PeelTarget)
    return None;

  // Traverse use-def chain in post order to find all dependent instructions of
  // peeling target's pointer operand and clone them into EntryBB.
  // Post-order traversal guarantees that defs are visited before their uses.
  Module *M = EntryBB.getModule();
  auto *ConstZero = ConstantInt::get(LoopUtils::getIndTy(M), 0);
  Use *PtrUse = PeelTarget->getOperandList() + PeelTarget->getNumOperands() - 1;
  Value *Ptr = PtrUse->get();
  assert(Ptr == getLoadStorePointerOperand(PeelTarget) &&
         "The last operand of a load/store instruction must be the pointer "
         "operand!");
  Value *PeelPtr = nullptr;
  if (auto *PtrI = dyn_cast<Instruction>(Ptr)) {
    std::string GID = CompilationUtils::mangledGetGID();
    std::string LID = CompilationUtils::mangledGetLID();
    ValueToValueMapTy VMap;
    Instruction *PreviousBBEnd = &EntryBB.back();
    SmallSet<Instruction *, 8> Visited;

    LLVM_DEBUG(dbgs() << "Collecting all dependent instructions of " << *PtrI
                      << '\n');
    for (Use *DepUse : make_range(po_begin(PtrUse), po_end(PtrUse))) {
      if (auto *I = dyn_cast<Instruction>(DepUse->get())) {
        LLVM_DEBUG(dbgs().indent(2) << "Instruction use: " << *I << '\n');
        if (Visited.count(I))
          continue;

        Visited.insert(I);
        if (auto *CI = dyn_cast<CallInst>(I)) {
          if (Function *CalledF = CI->getCalledFunction()) {
            StringRef FName = CalledF->getName();
            if (FName == GID) {
              // Replace get_global_id call with InitGID.
              uint64_t Dim =
                  cast<ConstantInt>(CI->getArgOperand(0))->getZExtValue();
              assert(Dim < InitGIDs.size() && "Dim is out of range");
              VMap[I] = InitGIDs[Dim];
              continue;
            }
            if (FName == LID) {
              // Replace get_local_id call with zero.
              VMap[I] = ConstZero;
              continue;
            }
          }
        }

        Instruction *NewI = I->clone();
        VMap[I] = NewI;
        NewI->insertAfter(&EntryBB.back());
        NewI->takeName(I);
      }
    }

    // Update operand references of cloned instructions.
    for (BasicBlock::iterator It(&EntryBB.back()), E(PreviousBBEnd); It != E;
         --It)
      RemapInstruction(&*It, VMap,
                       RF_NoModuleLevelChanges | RF_IgnoreMissingLocals);
    PeelPtr = &EntryBB.back();
  } else {
    PeelPtr = Ptr;
  }

  // Get required alignment, i.e. the number of bytes (step) by which peeling
  // target's access address get incremented each iteration.
  // SCEV is not able to give this information here, so we use type alloc size.
  assert(PeelPtr && PeelPtr->getType()->isPointerTy() &&
         "Peeling target is expected to have pointer type");
  Type *VTy = cast<PointerType>(PeelPtr->getType())->getElementType();
  assert(isa<VectorType>(VTy) &&
         "Peeling target is expected to be pointer to vector");
  Type *ETy = cast<VectorType>(VTy)->getElementType();
  const DataLayout &DL = M->getDataLayout();
  uint64_t ElementSize = DL.getTypeAllocSize(ETy).getFixedSize();
  Align RequiredAlign(ElementSize);

  // Get target (preferred) alignment from metadata.
  MDNode *MD = PeelTarget->getMetadata("intel.preferred_alignment");
  auto *MDV = cast<ConstantAsMetadata>(MD->getOperand(0));
  uint64_t PreferredAlign = cast<ConstantInt>(MDV->getValue())->getZExtValue();
  Align TargetAlign(PreferredAlign);

  // Set target alignment for peeling target.
  if (auto *LI = dyn_cast<LoadInst>(PeelTarget))
    LI->setAlignment(TargetAlign);
  else
    cast<StoreInst>(PeelTarget)->setAlignment(TargetAlign);

  // Compute peel count using formula from VPlanDynamicPeeling:
  //   Quotient = InvariantBase / RequiredAlignment;
  //   Divisor = TargetAlignment / RequiredAlignment;
  //   PeelCount = (Quotient * Multiplier) % Divisor;
  Twine NamePrefix = "peel.";
  IRBuilder<> Builder(&EntryBB);
  Type *IntPtrTy = Builder.getIntPtrTy(DL);
  Value *Ptr2Int =
      Builder.CreatePtrToInt(PeelPtr, IntPtrTy, NamePrefix + "ptr2int");
  int Step = static_cast<int>(ElementSize);
  int Multiplier =
      computeMultiplierForPeeling(Step, RequiredAlign, TargetAlign);
  unsigned LogRequiredAlign = Log2(RequiredAlign);
  Value *Quotient = Builder.CreateBinOp(
      Instruction::AShr, Ptr2Int, ConstantInt::get(IntPtrTy, LogRequiredAlign),
      NamePrefix + "quotient");
  Value *QuoMul = Builder.CreateBinOp(Instruction::Mul, Quotient,
                                      ConstantInt::get(IntPtrTy, Multiplier),
                                      NamePrefix + "quotient.multiplier");
  uint64_t Divisor = PreferredAlign / ElementSize;
  assert(Divisor > 1 && ((Divisor & (Divisor - 1)) == 0) &&
         "Divisor is expected to be power of 2");
  Value *DynamicPeelCount = Builder.CreateBinOp(
      Instruction::And, QuoMul, ConstantInt::get(IntPtrTy, Divisor - 1),
      NamePrefix + "count.dynamic");

  // Peel size is 0 if PeelPtr is not aligned by required alignment, and is
  // DynamicPeelCount otherwise.
  Value *AddrReqAlign = Builder.CreateBinOp(
      Instruction::And, Ptr2Int, ConstantInt::get(IntPtrTy, ElementSize - 1),
      NamePrefix + "ptr2int.and.req.align");
  Value *AddrReqAlignCmp = Builder.CreateICmp(
      CmpInst::ICMP_EQ, AddrReqAlign, ConstantInt::get(IntPtrTy, 0),
      NamePrefix + "ptr2int.req.aligned");
  Value *PeelSize =
      Builder.CreateSelect(AddrReqAlignCmp, DynamicPeelCount,
                           ConstantInt::get(IntPtrTy, 0), NamePrefix + "size");

  // Peel size should be not exceed the limit.
  PeelSize =
      Builder.CreateBinaryIntrinsic(Intrinsic::umin, MaxPeelSize, PeelSize,
                                    nullptr, NamePrefix + "actual.size");
  return PeelSize;
}

} // namespace LoopDynamicPeeling
} // namespace llvm

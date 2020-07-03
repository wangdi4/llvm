// INTEL CONFIDENTIAL
//
// Copyright 2020 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "LoopPeeling.h"
#include "CompilationUtils.h"
#include "LoopUtils/LoopUtils.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

/// This function is copied from IntelVPlanAlignmentAnalysis.cpp.
/// Modular multiplicative inverse.
static int modularMultiplicativeInverse(int Val, int Mod) {
  assert(Val > 0 && Mod > Val && "Invalid Arguments");
  assert(greatestCommonDivisor(Val, Mod) == 1 && "Arguments are not coprime");

  int Prev = 1;
  int Curr = Val;

  // Given coprime Val and Mod (Mod > Val > 0), there exists n such that
  // Val^n ≡ 1 (mod Mod).
  while (Curr != 1) {
    Prev = Curr;
    Curr = (Curr * Val) % Mod;
  }

  // Return Val^{n - 1}.
  return Prev;
}

/// This function is copied from IntelVPlanAlignmentAnalysis.cpp.
/// Compute -a^{-1} (mod b),
/// where a = InductionStep / RequiredAlignment
///       b = TargetAlignment / RequiredAlignment
static int computeMultiplierForPeeling(int Step, Align RequiredAlignment,
                                       Align TargetAlignment) {
  assert(TargetAlignment > RequiredAlignment &&
         "Peeling makes sense only if TargetAlignment > RequiredAlignment");
  assert(Step % RequiredAlignment.value() == 0 && "Bad RequiredAlignment");

  int ReqLog = Log2(RequiredAlignment);
  int TgtLog = Log2(TargetAlignment);
  int A = Step >> ReqLog;
  int B = 1 << (TgtLog - ReqLog);
  int InvA = modularMultiplicativeInverse(A % B, B);
  return B - InvA;
}

Value *LoopDynamicPeeling::computePeelCount(BasicBlock &EntryBB,
                                            BasicBlock &VectorEntry,
                                            const VVec &InitGIDs) {
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

  // Don't peel if intel.preferred_alignment is not found.
  if (!PeelTarget)
    return nullptr;

  // Run DFS to find all dependent instructions of peeling target's pointer
  // operand and clone them into EntryBB.
  Module *M = EntryBB.getParent()->getParent();
  auto *ConstZero = ConstantInt::get(LoopUtils::getIndTy(M), 0);
  Value *Ptr = getLoadStorePointerOperand(PeelTarget);
  Value *PeelPtr = nullptr;
  if (auto PtrI = dyn_cast<Instruction>(Ptr)) {
    std::string GID = CompilationUtils::mangledGetGID();
    std::string LID = CompilationUtils::mangledGetLID();
    ValueToValueMapTy VMap;
    Instruction *InsertPt = &EntryBB.back();
    SmallSet<Instruction *, 8> Visited;
    SmallVector<Instruction *, 8> WorkList;
    WorkList.push_back(PtrI);
    while (!WorkList.empty()) {
      Instruction *I = WorkList.pop_back_val();
      if (CallInst *CI = dyn_cast<CallInst>(I)) {
        if (Function *CF = CI->getCalledFunction()) {
          StringRef CFName = CF->getName();
          if (CFName == GID) {
            // Replace get_global_id call with initGID.
            ConstantInt *C = cast<ConstantInt>(CI->getArgOperand(0));
            uint64_t Dim = C->getZExtValue();
            VMap[I] = InitGIDs[Dim];
            continue;
          } else if (CFName == LID) {
            // Replace get_local_id call with zero.
            VMap[I] = ConstZero;
            continue;
          }
        }
      }
      Instruction *NewI = I->clone();
      VMap[I] = NewI;
      NewI->insertAfter(InsertPt);
      Visited.insert(I);
      for (Value *Op : I->operands()) {
        auto *OpInst = dyn_cast<Instruction>(Op);
        if (!OpInst)
          continue;
        if (Visited.count(OpInst)) {
          // Move dependent instruction to the beginning so that it donimates
          // its users.
          cast<Instruction>(VMap[OpInst])->moveAfter(InsertPt);
          continue;
        }
        WorkList.push_back(OpInst);
      }
    }
    // Update operand references of cloned instructions.
    for (BasicBlock::iterator It(&EntryBB.back()), E(InsertPt); It != E; --It)
      RemapInstruction(&*It, VMap,
                       RF_NoModuleLevelChanges | RF_IgnoreMissingLocals);
    PeelPtr = &EntryBB.back();
  } else
    PeelPtr = Ptr;

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
  assert(TargetAlign >= RequiredAlign &&
         "Peeling makes sense only if TargetAlign >= RequiredAlign");

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

  return PeelSize;
}

} // namespace intel

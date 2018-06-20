#if INTEL_COLLAB
//==-- IntrinsicUtils.cpp - Utilities for VPO related intrinsics -*- C++ -*-==//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file provides a set of utilities for VPO-based intrinsic function
/// calls. E.g., directives that mark the beginning and end of SIMD and
/// parallel regions.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"

#define DEBUG_TYPE "VPOIntrinsicUtils"

using namespace llvm;
using namespace llvm::vpo;

bool VPOUtils::stripDirectives(WRegionNode *WRN) {
  bool success = true;
  BasicBlock *ExitBB = WRN->getExitBBlock();

  // Under the old representation, we still need to remove dirs from EntryBB
  BasicBlock *EntryBB = WRN->getEntryBBlock();
  bool SeenRegionDirective = false;
  for (Instruction &I : *ExitBB) // use ExitBB until EntryBB issue is fixed
    if (VPOAnalysisUtils::isIntelDirective(&I)) {
      if (VPOAnalysisUtils::isRegionDirective(&I))
        SeenRegionDirective = true;
      break;
    }
  if (!SeenRegionDirective)
    success = VPOUtils::stripDirectives(*EntryBB);

  // Under the new region representation:
  //   %1 = call token @llvm.directive.region.entry() [...]
  //     ...
  //   call void @llvm.directive.region.exit(token %1) [...]
  // We have to remove the END dir before the BEGIN dir. If not, when removing
  // the BEGIN it will first remove the END (which is a use of the token
  // defined by the BEGIN intrinsic) and then later stripDirectives(*ExitBB)
  // would return false because there's nothing left in the ExitBB to remove.
  success = VPOUtils::stripDirectives(*ExitBB) && success;

  return success;
}

bool VPOUtils::stripDirectives(BasicBlock &BB) {
  SmallVector<Instruction *, 4> IntrinsicsToRemove;

  for (Instruction &I : BB) {
    if (VPOAnalysisUtils::isIntelDirectiveOrClause(&I)) {
      // Should not add I.Users() to IntrinsicsToRemove Vector,
      // otherwise, I.users() will be deleted twice,
      bool IsUser = false;
      for (unsigned int Idx = 0; Idx < IntrinsicsToRemove.size(); ++Idx) {
        Instruction *II = IntrinsicsToRemove[Idx];
        for (User *U : II->users())
          if (Instruction *UI = dyn_cast<Instruction>(U))
            if (&I == UI) {
              IsUser = true;
              break;
            }
        if (IsUser) break;
      }

      if (!IsUser)
        IntrinsicsToRemove.push_back(&I);
    }
  }

  // Remove the directive intrinsics.
  // SimplifyCFG will remove any blocks that become empty.
  unsigned Idx = 0;

  unsigned Sz = IntrinsicsToRemove.size();

  SmallVector<Instruction *, 4> IntrinsicsRegionBeginContainer;
  for (Idx = 0; Idx < Sz; ++Idx) {
    Instruction *I = IntrinsicsToRemove[Idx];
    IntrinsicInst *Call = dyn_cast<IntrinsicInst>(I);
    if (Call) {
      Intrinsic::ID IntrinId = Call->getIntrinsicID();
      if (IntrinId == Intrinsic::directive_region_exit) {
        Value *Arg = Call->getArgOperand(0);
        if (Instruction *I = dyn_cast<Instruction>(Arg))
          IntrinsicsRegionBeginContainer.push_back(I);
      }
    }
  }

  for (Idx = 0; Idx < IntrinsicsToRemove.size(); ++Idx) {
    Instruction *I = IntrinsicsToRemove[Idx];
    // Under the region representation, the BEGIN directive writes to a token
    // that is used by the matching END directive. Therefore, before removing
    // I, we must first remove all its uses, if any. Failing to do that
    // will result in this assertion: "Uses remain when a value is destroyed!"
    assert(I->getNumUses() <= 1 && "Expected not more than one use!");
    if (I->hasOneUse())
      if (auto *UI = dyn_cast<Instruction>(*I->user_begin()))
        UI->eraseFromParent();

    I->eraseFromParent();
  }

  while (!IntrinsicsRegionBeginContainer.empty()) {
    auto I = IntrinsicsRegionBeginContainer.pop_back_val();
    I->eraseFromParent();
  }

  // Returns true if any elimination happens.
  return Idx > 0;
}

bool VPOUtils::stripDirectives(Function &F) {
  bool changed = false;

  for (BasicBlock &BB: F) {
    changed |= stripDirectives(BB);
  }

  return changed;
}

bool VPOUtils::stripPrivateClauses(WRegionNode *WRN) {
  BasicBlock *EntryBB = WRN->getEntryBBlock();
  return VPOUtils::stripPrivateClauses(*EntryBB);
}

bool VPOUtils::stripPrivateClauses(BasicBlock &BB) {
  SmallVector<Instruction *, 4> IntrinsicsToRemove;

  for (Instruction &I : BB) {
    IntrinsicInst *Call = dyn_cast<IntrinsicInst>(&I);
    if (Call) {
      Intrinsic::ID Id = Call->getIntrinsicID();
      if (Id == Intrinsic::directive_region_entry) {
        // TODO: add support for this representation
        LLVM_DEBUG(
            dbgs() << "** WARNING: stripPrivateClauses() support for the "
                   << "OperandBundle representation will be done later.\n");
      }
      else if (Id == Intrinsic::intel_directive_qual_opndlist) {
        StringRef ClauseString = VPOAnalysisUtils::getDirOrClauseString(Call);
        ClauseSpecifier ClauseInfo(ClauseString);
        int ClauseID = ClauseInfo.getId();
        if (ClauseID == QUAL_OMP_PRIVATE)
          IntrinsicsToRemove.push_back(&I);
      }
    }
  }

  unsigned Idx = 0;
  for (Idx = 0; Idx < IntrinsicsToRemove.size(); ++Idx) {
    Instruction *I = IntrinsicsToRemove[Idx];
    I->eraseFromParent();
  }
  return Idx > 0;
}

CallInst *VPOUtils::createMaskedGatherCall(Value *VecPtr,
                                           IRBuilder<> &Builder,
                                           unsigned Alignment,
                                           Value *Mask,
                                           Value *PassThru) {
  auto NewCallInst = Builder.CreateMaskedGather(VecPtr, Alignment, Mask,
                                                PassThru);
  return NewCallInst;
}

CallInst *VPOUtils::createMaskedScatterCall(Value *VecPtr,
                                            Value *VecData,
                                            IRBuilder<> &Builder,
                                            unsigned Alignment,
                                            Value *Mask) {
  auto NewCallInst = Builder.CreateMaskedScatter(VecData, VecPtr, Alignment,
                                                 Mask);
  return NewCallInst;
}

CallInst *VPOUtils::createMaskedLoadCall(Value *VecPtr,
                                         IRBuilder<> &Builder,
                                         unsigned Alignment,
                                         Value *Mask,
                                         Value *PassThru) {
  auto NewCallInst = Builder.CreateMaskedLoad(VecPtr, Alignment, Mask,
                                               PassThru);
  return NewCallInst;
}

CallInst *VPOUtils::createMaskedStoreCall(Value *VecPtr,
                                          Value *VecData,
                                          IRBuilder<> &Builder,
                                          unsigned Alignment,
                                          Value *Mask) {
  auto NewCallInst = Builder.CreateMaskedStore(VecData, VecPtr, Alignment,
                                                Mask);
  return NewCallInst;
}

// Removes '@llvm.dbg.declare', '@llvm.dbg.value' calls from the Function F.
// This is a workaround for now till CodeExtractor learns to handle these.
void VPOUtils::stripDebugInfoInstrinsics(Function &F)
{
  for (auto &BB : F) {
    for (BasicBlock::iterator BI = BB.begin(), BE = BB.end(); BI != BE;) {
      Instruction *Insn = &*BI++;
      if (DbgValueInst *DVI = dyn_cast<DbgValueInst>(Insn)) {
        DVI->eraseFromParent();
      } else if (DbgDeclareInst *DDI = dyn_cast<DbgDeclareInst>(Insn)) {
        DDI->eraseFromParent();
      }
    }
  }
}

// Generates a memcpy call at the end of the given basic block BB.
// The value D represents the destination while the value S represents
// the source. The size of the memcpy is the size of destination.
// The compiler will insert the typecast if the type of source or destination
// does not match with the type i8.
// One example of the output is as follows.
//   call void @llvm.memcpy.p0i8.p0i8.i32(i8* bitcast (i32* @a to i8*), i8* %2, i32 4, i32 4, i1 false)
CallInst *VPOUtils::genMemcpy(Value *D, Value *S, const DataLayout &DL,
                              unsigned Align, BasicBlock *BB) {
  IRBuilder<> MemcpyBuilder(BB);
  MemcpyBuilder.SetInsertPoint(BB->getTerminator());

  Value *Dest, *Src, *Size;

  // The first two arguments of the memcpy expects the i8 operands.
  // The instruction bitcast is introduced if the incoming src or dest
  // operand in not in i8 type.
  if (D->getType() !=
      Type::getInt8PtrTy(BB->getParent()->getContext())) {
    Dest = MemcpyBuilder.CreatePointerCast(D, MemcpyBuilder.getInt8PtrTy());
    Src = MemcpyBuilder.CreatePointerCast(S, MemcpyBuilder.getInt8PtrTy());
  }
  else {
    Dest = D;
    Src = S;
  }
  // For 32/64 bit architecture, the size and alignment should be
  // set accordingly.
  if (DL.getIntPtrType(MemcpyBuilder.getInt8PtrTy())->getIntegerBitWidth() ==
      64)
    Size = MemcpyBuilder.getInt64(
        DL.getTypeAllocSize(D->getType()->getPointerElementType()));
  else
    Size = MemcpyBuilder.getInt32(
        DL.getTypeAllocSize(D->getType()->getPointerElementType()));

  AllocaInst *AI = dyn_cast<AllocaInst>(D);
  if (AI && AI->isArrayAllocation())
    Size = MemcpyBuilder.CreateMul(Size, AI->getArraySize());

  return MemcpyBuilder.CreateMemCpy(Dest, Align, Src, Align, Size);
}

// Utility to copy the data from the source to the destination.
void VPOUtils::genCopyFromSrcToDst(Type *AllocaTy, const DataLayout &DL,
                                   IRBuilder<> &Builder,
                                   AllocaInst *NewPrivInst,
                                   Value *Source, Value *Destination,
                                   BasicBlock *InsertBB) {
  Type *ScalarTy = AllocaTy->getScalarType();

  if (!AllocaTy->isSingleValueType() ||
      !DL.isLegalInteger(DL.getTypeSizeInBits(ScalarTy)) ||
      DL.getTypeSizeInBits(ScalarTy) % 8 != 0 ||
      NewPrivInst->isArrayAllocation())
    genMemcpy(Destination, Source, DL,
              NewPrivInst->getAlignment(), InsertBB);
  else
    Builder.CreateStore(Builder.CreateLoad(Source), Destination);
}
#endif // INTEL_COLLAB

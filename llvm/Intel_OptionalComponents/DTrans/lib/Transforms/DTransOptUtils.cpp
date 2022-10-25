//===- DTransOptUtils.cpp - Common utility functions for DTrans transforms-===//
//
// Copyright (C) 2018-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements common utility functions that may be useful to one or
// more of the DTrans transformation passes.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/DTransOptUtils.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Support/Debug.h"

using namespace llvm;
using namespace dtrans;

#define DEBUG_TYPE "dtrans-optutils"

// Given a stack of value-operand pairs representing the use-def chain from
// a place where a size-multiple value is used, back to the instruction that
// defines a multiplication by a constant multiple of the size, replace
// the size constant and clone any intermediate values as needed based on
// other uses of values in the chain.
static void
replaceSizeValue(Instruction *BaseI,
                 SmallVectorImpl<std::pair<User *, unsigned>> &SizeUseStack,
                 uint64_t OrigSize, uint64_t ReplSize) {
  // If we need to replace a constant in some instruction other than our
  // call, we need to check all the values in our use stack before the call
  // to see if they have other users. If they do, we'll need to clone all
  // values before and including the first one with multiple uses.
  // Note that we are walking from the bottom of the stack here -- that is
  // from the call instruction back to the use where the constant was found.
  bool NeedToClone = false;
  std::pair<User *, unsigned> PrevPair;
  for (auto &UsePair : SizeUseStack) {
    // Skip over the base instruction, its number of users doesn't matter.
    if (UsePair.first == BaseI) {
      PrevPair = UsePair;
      continue;
    }

    // If we haven't seen a value with multiple uses yet, and this value
    // doesn't have multiple uses, don't clone it.
    if (!NeedToClone && (UsePair.first->getNumUses() == 1)) {
      PrevPair = UsePair;
      continue;
    }

    // Otherwise, we need to clone.
    NeedToClone = true;
    auto *OrigUse = cast<Instruction>(UsePair.first);
    auto *Clone = OrigUse->clone();
    if (OrigUse->hasName())
      Clone->setName(OrigUse->getName() + ".dt");
    Clone->insertBefore(OrigUse);
    UsePair.first = Clone;

    // Also replace the use of this value in the previous instruction
    // on the stack.
    User *PrevUser = PrevPair.first;
    assert(PrevUser && "Null user in size use stack!");
    unsigned PrevIdx = PrevPair.second;
    assert((PrevUser->getOperand(PrevIdx) == OrigUse) &&
           "Size use stack is broken!");
    PrevUser->setOperand(PrevIdx, Clone);

    // Get ready for the next iteration.
    PrevPair = UsePair;
  }

  // Figure out the multiplier, if any, needed for the size constant.
  std::pair<User *, unsigned> SizePair = SizeUseStack.back();
  User *SizeUser = SizePair.first;
  unsigned SizeOpIdx = SizePair.second;
  auto *BinOp = dyn_cast<BinaryOperator>(SizeUser);
  if (BinOp && (BinOp->getOpcode() == Instruction::Shl)) {
    assert(SizeOpIdx == 1 && "Unexpected size operand for shl");
    auto *ConstVal = cast<ConstantInt>(SizeUser->getOperand(SizeOpIdx));
    uint64_t ConstShift = ConstVal->getLimitedValue();
    assert((!ConstVal->isNegative() && ConstShift < 64 &&
            ((1ull << ConstShift) % OrigSize) == 0ull) &&
           "Size shift left handling in multiplier search is broken");
    uint64_t Multiplier = (1ull << ConstShift) / OrigSize;

    // Rather than trying to update the shift value, which won't always work,
    // we just replace the shift with a multiplication.
    Value *NewSizeVal =
        ConstantInt::get(BinOp->getType(), ReplSize * Multiplier);
    Instruction *Mul =
        BinaryOperator::CreateMul(BinOp->getOperand(0), NewSizeVal);
    Mul->insertBefore(BinOp);
    LLVM_DEBUG(dbgs() << "Delete field: Replacing size-based shift:\n    "
                      << *BinOp << ")\n  with:\n    " << *Mul << "\n");
    Mul->takeName(BinOp);
    BinOp->replaceAllUsesWith(Mul);
    BinOp->eraseFromParent();
  } else {
    auto *ConstVal = cast<ConstantInt>(SizeUser->getOperand(SizeOpIdx));
    uint64_t ConstSize = ConstVal->getLimitedValue();
    assert((ConstSize % OrigSize) == 0 && "Size multiplier search is broken");
    uint64_t Multiplier = ConstSize / OrigSize;

    LLVM_DEBUG(dbgs() << "Delete field: Updating size operand (" << SizeOpIdx
                      << ") of " << *SizeUser << "\n");
    llvm::Type *SizeOpTy = SizeUser->getOperand(SizeOpIdx)->getType();
    SizeUser->setOperand(SizeOpIdx,
                         ConstantInt::get(SizeOpTy, ReplSize * Multiplier));
    LLVM_DEBUG(dbgs() << "  New value: " << *SizeUser << "\n");
  }
}

void llvm::dtrans::updateCallSizeOperand(llvm::Instruction *I,
                                         llvm::dtrans::CallInfo *CInfo,
                                         llvm::Type *OrigTy, llvm::Type *ReplTy,
                                         const llvm::TargetLibraryInfo &TLI) {
  const DataLayout &DL = I->getModule()->getDataLayout();
  uint64_t OrigSize = DL.getTypeAllocSize(OrigTy);
  uint64_t ReplSize = DL.getTypeAllocSize(ReplTy);

  updateCallSizeOperand(I, CInfo, OrigSize, ReplSize, TLI);
}

// This function performs the actual replacement for the size parameter
// of a function call, by finding the original constant that is a
// multiple of \p OrigSize, and replacing that value with a multiple
// of \p ReplSize.
void llvm::dtrans::updateCallSizeOperand(llvm::Instruction *I,
                                         llvm::dtrans::CallInfo *CInfo,
                                         uint64_t OrigSize, uint64_t ReplSize,
                                         const llvm::TargetLibraryInfo &TLI) {
  // Find the User value that has a constant integer multiple of the original
  // structure size as an operand.
  bool Found = false;
  SmallVector<std::pair<User *, unsigned>, 4> SizeUseStack;
  if (auto *AInfo = dyn_cast<dtrans::AllocCallInfo>(CInfo)) {
    dtrans::AllocKind AK = AInfo->getAllocKind();
    switch (AK) {
    case dtrans::AK_NotAlloc:
      llvm_unreachable("No AllocCallInfo for AK_NotAlloc!");
    case dtrans::AK_New:
    case dtrans::AK_Malloc:
    case dtrans::AK_Realloc:
    case dtrans::AK_Calloc:
    case dtrans::AK_UserMalloc:
    case dtrans::AK_UserMalloc0:
    case dtrans::AK_UserMallocThis: {
      unsigned SizeArgPos = 0;
      unsigned CountArgPos = 0;
      getAllocSizeArgs(AK, cast<CallBase>(I), SizeArgPos, CountArgPos, TLI);
      if (AK == dtrans::AK_Calloc) {
        Found =
            findValueMultipleOfSizeInst(I, CountArgPos, OrigSize, SizeUseStack);
        assert((Found || SizeUseStack.empty()) &&
               "SizeUseStack not empty after failed value search!");
        if (!Found)
          Found = findValueMultipleOfSizeInst(I, SizeArgPos, OrigSize,
                                              SizeUseStack);
      } else {
        Found =
            findValueMultipleOfSizeInst(I, SizeArgPos, OrigSize, SizeUseStack);
      }
      break;
    }
    }
  } else {
    // This asserts because we only expect alloc info or memfunc info.
    assert(isa<dtrans::MemfuncCallInfo>(CInfo) &&
           "Expected either alloc or memfunc!");
    // All memfunc calls have the size as operand 2.
    Found = findValueMultipleOfSizeInst(I, 2, OrigSize, SizeUseStack);
  }

  // The safety conditions should guarantee that we can find this constant.
  assert(Found && "Constant multiple of size not found!");

  replaceSizeValue(I, SizeUseStack, OrigSize, ReplSize);
}

void llvm::dtrans::updatePtrSubDivUserSizeOperand(llvm::BinaryOperator *Sub,
                                                  llvm::Type *OrigTy,
                                                  llvm::Type *ReplTy,
                                                  const DataLayout &DL) {
  uint64_t OrigSize = DL.getTypeAllocSize(OrigTy);
  uint64_t ReplSize = DL.getTypeAllocSize(ReplTy);

  updatePtrSubDivUserSizeOperand(Sub, OrigSize, ReplSize);
}

void llvm::dtrans::updatePtrSubDivUserSizeOperand(llvm::BinaryOperator *Sub,
                                                  uint64_t OrigSize,
                                                  uint64_t ReplSize) {
  for (auto *U : Sub->users()) {
    auto *BinOp = cast<BinaryOperator>(U);
    assert((BinOp->getOpcode() == Instruction::SDiv ||
            BinOp->getOpcode() == Instruction::UDiv) &&
           "Unexpected user in updatePtrSubDivUserSizeOperand!");
    // The sub instruction must be operand zero.
    assert(BinOp->getOperand(0) == Sub &&
           "Unexpected operand use for ptr sub!");
    // Look for the size in operand 1.
    SmallVector<std::pair<User *, unsigned>, 4> SizeUseStack;
    bool Found = findValueMultipleOfSizeInst(U, 1, OrigSize, SizeUseStack);
    assert(Found && "Couldn't find size div for ptr sub!");
    if (Found)
      replaceSizeValue(BinOp, SizeUseStack, OrigSize, ReplSize);
  }
}

// This helper function searches, starting with \p U operand \p Idx and
// following only multiply operations, for a User value with an operand that
// is a constant integer and is an exact multiple of the specified size. If a
// match is found, the \p UseStack vector will be populated with <User, Index>
// pairs of the use chain between \p U and the value where the constant was
// found.
//
// The return value indicates whether or not a match was found.
bool llvm::dtrans::findValueMultipleOfSizeInst(
    User *U, unsigned Idx, uint64_t Size,
    SmallVectorImpl<std::pair<User *, unsigned>> &UseStack) {
  if (!U)
    return false;

  // Get the specified operand value.
  Value *Val = U->getOperand(Idx);

  // Is it a constant?
  if (auto *ConstVal = dyn_cast<ConstantInt>(Val)) {
    // If so, is it a multiple of the size?
    uint64_t ConstSize = ConstVal->getLimitedValue();
    if (ConstSize == ~0ULL || ((ConstSize % Size) != 0))
      return false;
    // If it is, this is what we were looking for.
    UseStack.push_back(std::make_pair(U, Idx));
    return true;
  }

  // Is it a binary operator?
  if (auto *BinOp = dyn_cast<BinaryOperator>(Val)) {
    Value *LHS;
    Value *RHS;
    if (PatternMatch::match(Val,
                            PatternMatch::m_Shl(PatternMatch::m_Value(LHS),
                                                PatternMatch::m_Value(RHS)))) {
      uint64_t Shift = 0;
      if (dtrans::isValueConstant(RHS, &Shift) &&
          ((uint64_t(1) << Shift) % Size == 0)) {
        // In this case, the incoming size is shifted by some multiple of
        // the structure size. That makes the shift instruction the final
        // operation in our search.
        UseStack.push_back(std::make_pair(U, Idx));
        UseStack.push_back(std::make_pair(BinOp, 1));
        return true;
      }
      return false;
    }
    // Not a mul? Then it's not what we're looking for.
    if (BinOp->getOpcode() != Instruction::Mul)
      return false;
    // If it is a mul, speculatively push the current value operand pair
    // on the use stack and then check both operands for a constant multiple.
    UseStack.push_back(std::make_pair(U, Idx));
    if (findValueMultipleOfSizeInst(BinOp, 0, Size, UseStack))
      return true;
    if (findValueMultipleOfSizeInst(BinOp, 1, Size, UseStack))
      return true;
    // If neither matched, get our pair off of the stack and return false.
    UseStack.pop_back();
    return false;
  }

  // Is it sext or zext?
  if (isa<SExtInst>(Val) || isa<ZExtInst>(Val)) {
    // If so, speculatively push the current value operand pair on the stack
    // and check the operand for a constant multiple.
    UseStack.push_back(std::make_pair(U, Idx));
    if (findValueMultipleOfSizeInst(cast<User>(Val), 0, Size, UseStack))
      return true;
    // If it didn't match, get our pair off of the stack and return false.
    UseStack.pop_back();
    return false;
  }

  // Otherwise, it's definitely not what we were looking for.
  return false;
}

void llvm::dtrans::resetLoadStoreAlignment(GEPOperator *GEP,
                                           const DataLayout &DL,
                                           bool IsPacked) {
  Align DefaultAlign;
  for (auto *U : GEP->users()) {
    if (auto *LI = dyn_cast<LoadInst>(U)) {
      Align PrefAlign =
          IsPacked ? DefaultAlign : DL.getPrefTypeAlign(LI->getType());
      LI->setAlignment(PrefAlign);
    } else if (auto *SI = dyn_cast<StoreInst>(U)) {
      Align PrefAlign =
          IsPacked ? DefaultAlign
                   : DL.getPrefTypeAlign(SI->getValueOperand()->getType());
      SI->setAlignment(PrefAlign);
    } else if (auto *GEP = dyn_cast<GEPOperator>(U)) {
      resetLoadStoreAlignment(GEP, DL, IsPacked);
    }
  }
}

bool dtrans::isMainFunction(Function &F) { return F.getName() == "main"; }

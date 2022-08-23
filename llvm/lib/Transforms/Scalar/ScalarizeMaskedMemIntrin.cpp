//===- ScalarizeMaskedMemIntrin.cpp - Scalarize unsupported masked mem ----===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//                                    intrinsics
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass replaces masked memory intrinsics - when unsupported by the target
// - with a chain of basic blocks, that deal with the elements one-by-one if the
// appropriate mask bit is set.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/ScalarizeMaskedMemIntrin.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/DomTreeUpdater.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/VectorUtils.h" // INTEL
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/PatternMatch.h" //INTEL
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Local.h" // INTEL
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <cassert>

using namespace llvm;

#define DEBUG_TYPE "scalarize-masked-mem-intrin"

#if INTEL_CUSTOMIZATION
static cl::opt<unsigned> MaxDepth(
    "scalarize-masked-mem-intrin-max-depth", cl::Hidden, cl::init(4),
    cl::desc("Maximum depth of checking isSplatAndConst. (default = 4)"));

static cl::opt<unsigned>
    MaxLoads("scalarize-masked-mem-intrin-max-loads", cl::Hidden, cl::init(1),
             cl::desc("Maximum load count in GEP's elements when it can do "
                      "tryScalarizeGEP. (default = 1)"));

static cl::opt<unsigned>
MaxConst("scalarize-masked-mem-intrin-max-const", cl::Hidden, cl::init(3),
  cl::desc("Maximum constant count in GEP's elements when it can do "
    "tryScalarizeGEP. (default = 3)"));

static cl::opt<unsigned>
MaxScalar("scalarize-masked-mem-intrin-max-scalar", cl::Hidden, cl::init(10),
  cl::desc("Maximum scalar count in GEP's elements when it can do "
    "tryScalarizeGEP. (default = 10)"));
#endif // INTEL_CUSTOMIZATION

namespace {

class ScalarizeMaskedMemIntrinLegacyPass : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid

  explicit ScalarizeMaskedMemIntrinLegacyPass() : FunctionPass(ID) {
    initializeScalarizeMaskedMemIntrinLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;

  StringRef getPassName() const override {
    return "Scalarize Masked Memory Intrinsics";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.addPreserved<DominatorTreeWrapperPass>();
  }
};

} // end anonymous namespace

static bool optimizeBlock(BasicBlock &BB, bool &ModifiedDT,
                          const TargetTransformInfo &TTI, const DataLayout &DL,
                          DomTreeUpdater *DTU);
static bool optimizeCallInst(CallInst *CI, bool &ModifiedDT,
                             const TargetTransformInfo &TTI,
                             const DataLayout &DL, DomTreeUpdater *DTU);

char ScalarizeMaskedMemIntrinLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(ScalarizeMaskedMemIntrinLegacyPass, DEBUG_TYPE,
                      "Scalarize unsupported masked memory intrinsics", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(ScalarizeMaskedMemIntrinLegacyPass, DEBUG_TYPE,
                    "Scalarize unsupported masked memory intrinsics", false,
                    false)

FunctionPass *llvm::createScalarizeMaskedMemIntrinLegacyPass() {
  return new ScalarizeMaskedMemIntrinLegacyPass();
}

static bool isConstantIntVector(Value *Mask) {
  Constant *C = dyn_cast<Constant>(Mask);
  if (!C)
    return false;

  unsigned NumElts = cast<FixedVectorType>(Mask->getType())->getNumElements();
  for (unsigned i = 0; i != NumElts; ++i) {
    Constant *CElt = C->getAggregateElement(i);
    if (!CElt || !isa<ConstantInt>(CElt))
      return false;
  }

  return true;
}

static unsigned adjustForEndian(const DataLayout &DL, unsigned VectorWidth,
                                unsigned Idx) {
  return DL.isBigEndian() ? VectorWidth - 1 - Idx : Idx;
}

#if INTEL_CUSTOMIZATION

static unsigned getTruePrefixMaskNum(Value *Mask) {
  Constant *C = cast<Constant>(Mask);
  unsigned NumElts = cast<FixedVectorType>(Mask->getType())->getNumElements();
  unsigned i = 0;
  unsigned TruePrefixMaskNum = 0;
  for (; i != NumElts; ++i) {
    Constant *CElt = C->getAggregateElement(i);
    if (CElt->isNullValue())
      break;
  }

  TruePrefixMaskNum = i;
  // If there is still 'true' mask, return 0 directly.
  for (; i != NumElts; ++i) {
    Constant *CElt = C->getAggregateElement(i);
    if (CElt->isOneValue())
      return 0;
  }

  return TruePrefixMaskNum;
}

// Transform
// masked.store(<4 x i8> %data, <4 x i8>* %ptr, i32 1, <4 x i1> <0xe>)
// to
// %new_data = shufflevector <4 x i8> %data, <4 x i8> poison,
//                                           <3 x i32> <i32 0, i32 1, i32 2>
// %new_ptr = bitcast <4 x i8>* %ptr to <3 x i8>*
// store <3 x i8> %new_data, <3 x i8>* %new_ptr, 1
static bool scalarizeTruePrefixMaskStore(const DataLayout &DL, CallInst *CI) {

  Value *Data = CI->getArgOperand(0);
  Value *Ptr = CI->getArgOperand(1);
  Value *Alignment = CI->getArgOperand(2);
  Value *Mask = CI->getArgOperand(3);

  unsigned TruePrefixMaskNum = getTruePrefixMaskNum(Mask);
  if (TruePrefixMaskNum == 0)
    return false;

  const Align AlignVal = cast<ConstantInt>(Alignment)->getAlignValue();

  IRBuilder<> Builder(CI);
  VectorType *VecType = cast<FixedVectorType>(Data->getType());

  Type *EltTy = VecType->getElementType();
  VectorType *NewVecType = FixedVectorType::get(EltTy, TruePrefixMaskNum);

  SmallVector<int, 8> ShufMask;
  for (unsigned i = 0; i < TruePrefixMaskNum; ++i)
    ShufMask.push_back(i);
  Value *NewData = Builder.CreateShuffleVector(Data, ShufMask);

  auto NewVecPtrType =
      PointerType::get(NewVecType, Ptr->getType()->getPointerAddressSpace());
  auto NewPtr = Builder.CreateBitCast(Ptr, NewVecPtrType);

  Builder.CreateAlignedStore(NewData, NewPtr, AlignVal);

  CI->eraseFromParent();

  return true;
}

// Transform
// %data = call <4 x i8> masked.load(<4 x i8>* %ptr, i32 1, <4 x i1> <0xe>,
//                                   <4 x i8> undef)
// to
// %new_ptr = bitcast <4 x i8>* %ptr to <3 x i8>*
// %data = load <3 x i8>, <3 x i8>* %new_ptr, align 1
// %new_data = shufflevector <3 x i8> %data, <3 x i8> undef,
//                           <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
static bool scalarizeTruePrefixMaskLoad(const DataLayout &DL, CallInst *CI) {

  Value *Ptr = CI->getArgOperand(0);
  Value *Alignment = CI->getArgOperand(1);
  Value *Mask = CI->getArgOperand(2);
  Value *Src0 = CI->getArgOperand(3);

  // Only support undef value current.
  if (!isa<UndefValue>(Src0))
    return false;

  unsigned NumElts = cast<FixedVectorType>(Mask->getType())->getNumElements();

  unsigned TruePrefixMaskNum = getTruePrefixMaskNum(Mask);
  if (TruePrefixMaskNum == 0)
    return false;

  IRBuilder<> Builder(CI);
  const Align AlignVal = cast<ConstantInt>(Alignment)->getAlignValue();
  VectorType *VecType = cast<FixedVectorType>(CI->getType());

  Type *EltTy = VecType->getElementType();
  VectorType *NewVecType = FixedVectorType::get(EltTy, TruePrefixMaskNum);
  auto NewVecPtrType =
      PointerType::get(NewVecType, Ptr->getType()->getPointerAddressSpace());
  auto NewPtr = Builder.CreateBitCast(Ptr, NewVecPtrType);
  Value *Load = Builder.CreateAlignedLoad(NewVecType, NewPtr, AlignVal);

  SmallVector<int, 8> WidenMask;
  for (unsigned i = 0; i < NumElts; ++i) {
    if (i < TruePrefixMaskNum)
      WidenMask.push_back(i);
    else
      WidenMask.push_back(-1);
  }

  Value *WidenLoad = Builder.CreateShuffleVector(
      Load, UndefValue::get(Load->getType()), WidenMask);

  CI->replaceAllUsesWith(WidenLoad);
  CI->eraseFromParent();

  return true;
}

#endif // INTEL_CUSTOMIZATION

// Translate a masked load intrinsic like
// <16 x i32 > @llvm.masked.load( <16 x i32>* %addr, i32 align,
//                               <16 x i1> %mask, <16 x i32> %passthru)
// to a chain of basic blocks, with loading element one-by-one if
// the appropriate mask bit is set
//
//  %1 = bitcast i8* %addr to i32*
//  %2 = extractelement <16 x i1> %mask, i32 0
//  br i1 %2, label %cond.load, label %else
//
// cond.load:                                        ; preds = %0
//  %3 = getelementptr i32* %1, i32 0
//  %4 = load i32* %3
//  %5 = insertelement <16 x i32> %passthru, i32 %4, i32 0
//  br label %else
//
// else:                                             ; preds = %0, %cond.load
//  %res.phi.else = phi <16 x i32> [ %5, %cond.load ], [ undef, %0 ]
//  %6 = extractelement <16 x i1> %mask, i32 1
//  br i1 %6, label %cond.load1, label %else2
//
// cond.load1:                                       ; preds = %else
//  %7 = getelementptr i32* %1, i32 1
//  %8 = load i32* %7
//  %9 = insertelement <16 x i32> %res.phi.else, i32 %8, i32 1
//  br label %else2
//
// else2:                                          ; preds = %else, %cond.load1
//  %res.phi.else3 = phi <16 x i32> [ %9, %cond.load1 ], [ %res.phi.else, %else ]
//  %10 = extractelement <16 x i1> %mask, i32 2
//  br i1 %10, label %cond.load4, label %else5
//
static void scalarizeMaskedLoad(const DataLayout &DL, CallInst *CI,
                                DomTreeUpdater *DTU, bool &ModifiedDT) {
  Value *Ptr = CI->getArgOperand(0);
  Value *Alignment = CI->getArgOperand(1);
  Value *Mask = CI->getArgOperand(2);
  Value *Src0 = CI->getArgOperand(3);

  const Align AlignVal = cast<ConstantInt>(Alignment)->getAlignValue();
  VectorType *VecType = cast<FixedVectorType>(CI->getType());

  Type *EltTy = VecType->getElementType();

  IRBuilder<> Builder(CI->getContext());
  Instruction *InsertPt = CI;
  BasicBlock *IfBlock = CI->getParent();

  Builder.SetInsertPoint(InsertPt);
  Builder.SetCurrentDebugLocation(CI->getDebugLoc());

  // Short-cut if the mask is all-true.
  if (isa<Constant>(Mask) && cast<Constant>(Mask)->isAllOnesValue()) {
    Value *NewI = Builder.CreateAlignedLoad(VecType, Ptr, AlignVal);
    CI->replaceAllUsesWith(NewI);
    CI->eraseFromParent();
    return;
  }

  // Adjust alignment for the scalar instruction.
  const Align AdjustedAlignVal =
      commonAlignment(AlignVal, EltTy->getPrimitiveSizeInBits() / 8);
  // Bitcast %addr from i8* to EltTy*
  Type *NewPtrType =
      EltTy->getPointerTo(Ptr->getType()->getPointerAddressSpace());
  Value *FirstEltPtr = Builder.CreateBitCast(Ptr, NewPtrType);
  unsigned VectorWidth = cast<FixedVectorType>(VecType)->getNumElements();

  // The result vector
  Value *VResult = Src0;

  if (isConstantIntVector(Mask)) {
#if INTEL_CUSTOMIZATION
    if (scalarizeTruePrefixMaskLoad(DL, CI))
      return;
#endif // INTEL_CUSTOMIZATION
    for (unsigned Idx = 0; Idx < VectorWidth; ++Idx) {
      if (cast<Constant>(Mask)->getAggregateElement(Idx)->isNullValue())
        continue;
      Value *Gep = Builder.CreateConstInBoundsGEP1_32(EltTy, FirstEltPtr, Idx);
      LoadInst *Load = Builder.CreateAlignedLoad(EltTy, Gep, AdjustedAlignVal);
      VResult = Builder.CreateInsertElement(VResult, Load, Idx);
    }
    CI->replaceAllUsesWith(VResult);
    CI->eraseFromParent();
    return;
  }

  // If the mask is not v1i1, use scalar bit test operations. This generates
  // better results on X86 at least.
  Value *SclrMask;
  if (VectorWidth != 1) {
    Type *SclrMaskTy = Builder.getIntNTy(VectorWidth);
    SclrMask = Builder.CreateBitCast(Mask, SclrMaskTy, "scalar_mask");
  }

  for (unsigned Idx = 0; Idx < VectorWidth; ++Idx) {
    // Fill the "else" block, created in the previous iteration
    //
    //  %res.phi.else3 = phi <16 x i32> [ %11, %cond.load1 ], [ %res.phi.else, %else ]
    //  %mask_1 = and i16 %scalar_mask, i32 1 << Idx
    //  %cond = icmp ne i16 %mask_1, 0
    //  br i1 %mask_1, label %cond.load, label %else
    //
    Value *Predicate;
    if (VectorWidth != 1) {
      Value *Mask = Builder.getInt(APInt::getOneBitSet(
          VectorWidth, adjustForEndian(DL, VectorWidth, Idx)));
      Predicate = Builder.CreateICmpNE(Builder.CreateAnd(SclrMask, Mask),
                                       Builder.getIntN(VectorWidth, 0));
    } else {
      Predicate = Builder.CreateExtractElement(Mask, Idx);
    }

    // Create "cond" block
    //
    //  %EltAddr = getelementptr i32* %1, i32 0
    //  %Elt = load i32* %EltAddr
    //  VResult = insertelement <16 x i32> VResult, i32 %Elt, i32 Idx
    //
    Instruction *ThenTerm =
        SplitBlockAndInsertIfThen(Predicate, InsertPt, /*Unreachable=*/false,
                                  /*BranchWeights=*/nullptr, DTU);

    BasicBlock *CondBlock = ThenTerm->getParent();
    CondBlock->setName("cond.load");

    Builder.SetInsertPoint(CondBlock->getTerminator());
    Value *Gep = Builder.CreateConstInBoundsGEP1_32(EltTy, FirstEltPtr, Idx);
    LoadInst *Load = Builder.CreateAlignedLoad(EltTy, Gep, AdjustedAlignVal);
    Value *NewVResult = Builder.CreateInsertElement(VResult, Load, Idx);

    // Create "else" block, fill it in the next iteration
    BasicBlock *NewIfBlock = ThenTerm->getSuccessor(0);
    NewIfBlock->setName("else");
    BasicBlock *PrevIfBlock = IfBlock;
    IfBlock = NewIfBlock;

    // Create the phi to join the new and previous value.
    Builder.SetInsertPoint(NewIfBlock, NewIfBlock->begin());
    PHINode *Phi = Builder.CreatePHI(VecType, 2, "res.phi.else");
    Phi->addIncoming(NewVResult, CondBlock);
    Phi->addIncoming(VResult, PrevIfBlock);
    VResult = Phi;
  }

  CI->replaceAllUsesWith(VResult);
  CI->eraseFromParent();

  ModifiedDT = true;
}

// Translate a masked store intrinsic, like
// void @llvm.masked.store(<16 x i32> %src, <16 x i32>* %addr, i32 align,
//                               <16 x i1> %mask)
// to a chain of basic blocks, that stores element one-by-one if
// the appropriate mask bit is set
//
//   %1 = bitcast i8* %addr to i32*
//   %2 = extractelement <16 x i1> %mask, i32 0
//   br i1 %2, label %cond.store, label %else
//
// cond.store:                                       ; preds = %0
//   %3 = extractelement <16 x i32> %val, i32 0
//   %4 = getelementptr i32* %1, i32 0
//   store i32 %3, i32* %4
//   br label %else
//
// else:                                             ; preds = %0, %cond.store
//   %5 = extractelement <16 x i1> %mask, i32 1
//   br i1 %5, label %cond.store1, label %else2
//
// cond.store1:                                      ; preds = %else
//   %6 = extractelement <16 x i32> %val, i32 1
//   %7 = getelementptr i32* %1, i32 1
//   store i32 %6, i32* %7
//   br label %else2
//   . . .
static void scalarizeMaskedStore(const DataLayout &DL, CallInst *CI,
                                 DomTreeUpdater *DTU, bool &ModifiedDT) {
  Value *Src = CI->getArgOperand(0);
  Value *Ptr = CI->getArgOperand(1);
  Value *Alignment = CI->getArgOperand(2);
  Value *Mask = CI->getArgOperand(3);

  const Align AlignVal = cast<ConstantInt>(Alignment)->getAlignValue();
  auto *VecType = cast<VectorType>(Src->getType());

  Type *EltTy = VecType->getElementType();

  IRBuilder<> Builder(CI->getContext());
  Instruction *InsertPt = CI;
  Builder.SetInsertPoint(InsertPt);
  Builder.SetCurrentDebugLocation(CI->getDebugLoc());

  // Short-cut if the mask is all-true.
  if (isa<Constant>(Mask) && cast<Constant>(Mask)->isAllOnesValue()) {
    Builder.CreateAlignedStore(Src, Ptr, AlignVal);
    CI->eraseFromParent();
    return;
  }

  // Adjust alignment for the scalar instruction.
  const Align AdjustedAlignVal =
      commonAlignment(AlignVal, EltTy->getPrimitiveSizeInBits() / 8);
  // Bitcast %addr from i8* to EltTy*
  Type *NewPtrType =
      EltTy->getPointerTo(Ptr->getType()->getPointerAddressSpace());
  Value *FirstEltPtr = Builder.CreateBitCast(Ptr, NewPtrType);
  unsigned VectorWidth = cast<FixedVectorType>(VecType)->getNumElements();

  if (isConstantIntVector(Mask)) {
#if INTEL_CUSTOMIZATION
    if (scalarizeTruePrefixMaskStore(DL, CI))
      return;
#endif // INTEL_CUSTOMIZATION
    for (unsigned Idx = 0; Idx < VectorWidth; ++Idx) {
      if (cast<Constant>(Mask)->getAggregateElement(Idx)->isNullValue())
        continue;
      Value *OneElt = Builder.CreateExtractElement(Src, Idx);
      Value *Gep = Builder.CreateConstInBoundsGEP1_32(EltTy, FirstEltPtr, Idx);
      Builder.CreateAlignedStore(OneElt, Gep, AdjustedAlignVal);
    }
    CI->eraseFromParent();
    return;
  }

  // If the mask is not v1i1, use scalar bit test operations. This generates
  // better results on X86 at least.
  Value *SclrMask;
  if (VectorWidth != 1) {
    Type *SclrMaskTy = Builder.getIntNTy(VectorWidth);
    SclrMask = Builder.CreateBitCast(Mask, SclrMaskTy, "scalar_mask");
  }

  for (unsigned Idx = 0; Idx < VectorWidth; ++Idx) {
    // Fill the "else" block, created in the previous iteration
    //
    //  %mask_1 = and i16 %scalar_mask, i32 1 << Idx
    //  %cond = icmp ne i16 %mask_1, 0
    //  br i1 %mask_1, label %cond.store, label %else
    //
    Value *Predicate;
    if (VectorWidth != 1) {
      Value *Mask = Builder.getInt(APInt::getOneBitSet(
          VectorWidth, adjustForEndian(DL, VectorWidth, Idx)));
      Predicate = Builder.CreateICmpNE(Builder.CreateAnd(SclrMask, Mask),
                                       Builder.getIntN(VectorWidth, 0));
    } else {
      Predicate = Builder.CreateExtractElement(Mask, Idx);
    }

    // Create "cond" block
    //
    //  %OneElt = extractelement <16 x i32> %Src, i32 Idx
    //  %EltAddr = getelementptr i32* %1, i32 0
    //  %store i32 %OneElt, i32* %EltAddr
    //
    Instruction *ThenTerm =
        SplitBlockAndInsertIfThen(Predicate, InsertPt, /*Unreachable=*/false,
                                  /*BranchWeights=*/nullptr, DTU);

    BasicBlock *CondBlock = ThenTerm->getParent();
    CondBlock->setName("cond.store");

    Builder.SetInsertPoint(CondBlock->getTerminator());
    Value *OneElt = Builder.CreateExtractElement(Src, Idx);
    Value *Gep = Builder.CreateConstInBoundsGEP1_32(EltTy, FirstEltPtr, Idx);
    Builder.CreateAlignedStore(OneElt, Gep, AdjustedAlignVal);

    // Create "else" block, fill it in the next iteration
    BasicBlock *NewIfBlock = ThenTerm->getSuccessor(0);
    NewIfBlock->setName("else");

    Builder.SetInsertPoint(NewIfBlock, NewIfBlock->begin());
  }
  CI->eraseFromParent();

  ModifiedDT = true;
}

#if INTEL_CUSTOMIZATION

// Return true if all elements are ConstantInts,
// use the Constant for this element.
static Constant *legalConst(Constant *C, unsigned &ConstCount) {
  auto VT = cast<FixedVectorType>(C->getType());
  if (!VT)
    return nullptr;

  unsigned NumElts = VT->getNumElements();
  ++ConstCount;
  if (ConstCount > MaxConst)
    return nullptr;

  for (unsigned j = 0; j != NumElts; ++j) {
    Constant *Elt = C->getAggregateElement(j);
    if (!Elt || !isa<ConstantInt>(Elt))
      return nullptr;
  }
  return C;
}

// Return true iff all the leaf variables are splat vectors
// and only constants are real vectors.
static bool isSplatAndConst(Value *V, unsigned Depth, unsigned &LoadCount,
                            unsigned &ConstCount, unsigned &ScalarCount) {
  if (Depth > MaxDepth || LoadCount > MaxLoads || ConstCount > MaxConst ||
      ScalarCount > MaxScalar)
    return false;

  if (auto Bin = dyn_cast<BinaryOperator>(V)) {
    auto LHS = Bin->getOperand(0);
    auto RHS = Bin->getOperand(1);

    switch (Bin->getOpcode()) {
    case Instruction::Add:
    case Instruction::Sub:
    case Instruction::Mul:
    case Instruction::Shl:
      break;
    default:
      return false;
    }

    if (getSplatValue(LHS)) {
      ScalarCount++;
      if (auto *C = dyn_cast<Constant>(RHS))
        return legalConst(C, ConstCount);
      return isSplatAndConst(RHS, Depth + 1, LoadCount, ConstCount,
                             ScalarCount);
    }
    if (auto *C = dyn_cast<Constant>(LHS)) {
      if (!legalConst(C, ConstCount))
        return false;
      if (getSplatValue(RHS)) {
        ScalarCount++;
        return true;
      }
      return isSplatAndConst(RHS, Depth + 1, LoadCount, ConstCount,
                            ScalarCount);
    }
    if (getSplatValue(RHS)) {
      ScalarCount++;
      if (auto *C = dyn_cast<Constant>(LHS))
        return legalConst(C, ConstCount);
      return isSplatAndConst(LHS, Depth + 1, LoadCount, ConstCount,
                             ScalarCount);
    }
    if (auto *C = dyn_cast<Constant>(RHS)) {
      if (!legalConst(C, ConstCount))
        return false;
      if (getSplatValue(LHS)) {
        ScalarCount++;
        return true;
      }
      return isSplatAndConst(LHS, Depth + 1, LoadCount, ConstCount,
                             ScalarCount);
    }
  } else if (isa<LoadInst>(V)) {
    ++LoadCount;
    if (LoadCount <= MaxLoads)
      return true;
  } else if (isa<SExtInst>(V) || isa<ZExtInst>(V)) {
    auto *Ext = cast<CastInst>(V);
    return isSplatAndConst(Ext->getOperand(0), Depth + 1, LoadCount, ConstCount,
                           ScalarCount);
  } else if (auto Shuf = dyn_cast<ShuffleVectorInst>(V)) {
    auto *SrcTy = dyn_cast<FixedVectorType>(Shuf->getOperand(0)->getType());
    if (!SrcTy)
      return false;

    int NumSrcElts = SrcTy->getNumElements();
    auto ShufMask = Shuf->getShuffleMask();
    for (int I = 0, NumElts = ShufMask.size(); I < NumElts; ++I) {
      if (ShufMask[I] == -1)
        continue;
      if (NumSrcElts < ShufMask[I])
        return false;
    }
    return isSplatAndConst(Shuf->getOperand(0), Depth + 1, LoadCount,
                           ConstCount, ScalarCount);
  }

  return false;
}

static Value *createSplatAndConstExpr(Value *V, unsigned Element,
                                      IRBuilder<> &Builder) {
  auto createBinOpExpr = [](Value *&Op0, Value *&Op1, unsigned Element,
                            IRBuilder<> &Builder) {
    if (auto Splat = getSplatValue(Op0)) {
      Op0 = Splat;
      if (auto *C = dyn_cast<Constant>(Op1))
        Op1 = C->getAggregateElement(Element);
      else
        Op1 = createSplatAndConstExpr(Op1, Element, Builder);
      return true;
    }
    if (auto *C = dyn_cast<Constant>(Op0)) {
      Op0 = C->getAggregateElement(Element);
      if (auto Splat = getSplatValue(Op1))
        Op1 = Splat;
      else
        Op1 = createSplatAndConstExpr(Op1, Element, Builder);
      return true;
    }
    return false;
  };
  if (auto Bin = dyn_cast<BinaryOperator>(V)) {
    auto LHS = Bin->getOperand(0);
    auto RHS = Bin->getOperand(1);

    if (createBinOpExpr(LHS, RHS, Element, Builder) ||
        createBinOpExpr(RHS, LHS, Element, Builder))
      return Builder.CreateBinOp(Bin->getOpcode(), LHS, RHS);
  } else if (isa<SExtInst>(V) || isa<ZExtInst>(V)) {
    auto *Ext = cast<CastInst>(V);
    Value *Src = createSplatAndConstExpr(Ext->getOperand(0), Element, Builder);
    auto *DstTy = dyn_cast<VectorType>(Ext->getType());
    assert(DstTy && "DstTy must be ");
    Type *ScalarDstTy = DstTy->getElementType();
    return Builder.CreateCast(Ext->getOpcode(), Src, ScalarDstTy);
  } else if (auto Shuf = dyn_cast<ShuffleVectorInst>(V)) {
    auto ShufMask = Shuf->getShuffleMask();
    auto ShufTy = cast<FixedVectorType>(Shuf->getOperand(0)->getType());
    if (ShufMask[Element] == -1)
      return UndefValue::get(ShufTy->getElementType());
    return createSplatAndConstExpr(Shuf->getOperand(0), ShufMask[Element],
                                   Builder);
  } else if (auto Load = dyn_cast<LoadInst>(V)) {
    Value *Ptrs = Load->getPointerOperand();
    auto *PtrsTy = cast<PointerType>(Ptrs->getType());
    auto *DataTy = cast<FixedVectorType>(Load->getType());
    Type *ElemTy = DataTy->getElementType();
    auto *NewPtrTy = PointerType::get(ElemTy, PtrsTy->getAddressSpace());
    Value *NewPtr = Builder.CreateBitCast(Ptrs, NewPtrTy);
    Value *NewGEP = Builder.CreateConstGEP1_32(ElemTy, NewPtr, Element);
    return Builder.CreateLoad(ElemTy, NewGEP);
  }
  llvm_unreachable("Not reachable.");
  return nullptr;
}

// Return true if V matches following pattern:
// %ld = load <16 x i16>, <16 x i16>* %0, align 2
// %2 = sext <16 x i16> %ld to <16 x i64>
// %3 = add nsw <16 x i64> %2, <i64 32768, ..., i64 32768>
static bool isSplatAndConstRestrictI16(Value *V, unsigned Depth,
                                       unsigned &LoadCount,
                                       unsigned &ConstCount,
                                       unsigned &ScalarCount) {
  auto Bin = dyn_cast<BinaryOperator>(V);
  if (!Bin)
    return false;

  auto LHS = Bin->getOperand(0);
  auto RHS = Bin->getOperand(1);

  if (Bin->getOpcode() != Instruction::Add)
    return false;

  auto* C = dyn_cast_or_null<ConstantInt>(getSplatValue(RHS));
  if (!C)
    return false;

  if (!isa<SExtInst>(LHS))
    return false;

  auto *Ext = cast<CastInst>(LHS);
  auto *Ty = dyn_cast<FixedVectorType>(Ext->getSrcTy());
  if (!Ty || !Ty->getElementType()->isIntegerTy(16))
    return false;

  if (!isa<LoadInst>(Ext->getOperand(0)))
    return false;

  ++ScalarCount;
  ++LoadCount;

  if (LoadCount > 1 || ConstCount || ScalarCount > 1)
    return false;

  return true;
}

// Get splat value if the input is a splat vector or return nullptr.
// The value may be extracted from a splat constants vector or from
// a sequence of instructions that broadcast a single value into a vector.
// If there is BitCast for the splat value return the type after BitCast.
static Value *getSplatValueBitCast(const Value *V, Type *&CastType) {
  using namespace llvm;
  using namespace llvm::PatternMatch;
  CastType = nullptr;
  Value *Splat = getSplatValue(V);
  if (Splat)
    return Splat;

  auto VecType = cast<FixedVectorType>(V->getType());
  if (VecType) {
    // shuf (bitcast (inselt ?, Splat, 0)), ?, <0, undef, 0, ...>
    if (match(V,
              m_Shuffle(m_BitCast(m_InsertElt(m_Value(),
                  m_Value(Splat), m_ZeroInt())),
                        m_Value(), m_ZeroMask()))) {
      CastType = VecType->getElementType();
      return Splat;
    }
  }
  return nullptr;
}

// Look for GEP where the base pointer and all indices are made of scalars,
// splats of scalars, or constant ints. These GEPs are trivially scalarizable.
// This creates opportunities for the arithmetic to be folded into the address
// of the load/stores we're going to create. Otherwise we end up broadcasting
// the scalars into vector registers, doing the address arithmetic there, and
// then extracting the address for each element.
static Value *tryScalarizeGEP(GetElementPtrInst *GEP, unsigned Element,
                              IRBuilder<> &Builder, unsigned &LoadCount,
                              unsigned &ConstCount, unsigned &ScalarCount,
                              const TargetTransformInfo &TTI) {
  Value *Base = GEP->getPointerOperand();
  Type *BasePtrTy = GEP->getSourceElementType();
  BitCastInst *BC = nullptr;

  // Base should be a scalar, or a splatted scalar.
  if (Base->getType()->isVectorTy()) {
    Type *CastType;
    Value *Splat = getSplatValueBitCast(Base, CastType);
    if (Splat) {
      if (CastType) {
        // handle following example:
        //   %t1 = insertelement <2 x float*> poison, float* %Splat, i64 0
        //   %t2 = bitcast <2 x float*> %t1 to <2 x i8*>
        //   %Base = shufflevector <2 x i8*> %t2, <2 x i8*> poison, <2 x i32> zeroinitializer
        Splat = Builder.CreateBitCast(Splat, CastType);
      }
      Base = Splat;
    } else {
      auto NGEP = dyn_cast<GetElementPtrInst>(Base);
      if (!NGEP) {
        BC = dyn_cast<BitCastInst>(Base);
        if (BC) {
          // handle following example:
          //   %NGEP = getelementptr inbounds i8, <2 x i8*> %4, <2 x i64> %12
          //   %BC = bitcast <2 x i8*> %NGEP to <2 x float*>
          //   %GP = getelementptr inbounds float, <2 x float*> %14, <2 x i64> %BC
          NGEP = dyn_cast<GetElementPtrInst>(BC->getOperand(0));
          if (!NGEP)
            return nullptr;
        } else {
          return nullptr;
        }
      }
      Base = tryScalarizeGEP(NGEP, Element, Builder, LoadCount,
                             ConstCount, ScalarCount, TTI);
      if (!Base)
        return nullptr;
    }
  }

  // Found a scalar to use for base. Now try to find scalars for the indices.
  SmallVector<Value *, 8> Indices;

  for (unsigned i = 1, e = GEP->getNumOperands(); i != e; ++i) {
    Value *GEPIdx = GEP->getOperand(i);
    if (!GEPIdx->getType()->isVectorTy()) {
      // Just copy it.
      Indices.push_back(GEPIdx);
    } else if (Value *V = getSplatValue(GEPIdx)) {
      // Splatted scalar, copy the original scalar value.
      Indices.push_back(V);
    } else {
      // We have some kind of non-splat vector.
      if (auto *C = dyn_cast<Constant>(GEPIdx)) {
        // If all elements are ConstantInts, use the Constant for this element.
        auto VT = dyn_cast<FixedVectorType>(C->getType());
        if (!VT)
          return nullptr;
        unsigned NumElts = VT->getNumElements();
        for (unsigned j = 0; j != NumElts; ++j) {
          Constant *Elt = C->getAggregateElement(j);
          if (!Elt || !isa<ConstantInt>(Elt))
            return nullptr;
        }
        Indices.push_back(C->getAggregateElement(Element));
        continue;
      } else if ((TTI.isAdvancedOptEnabled(
              TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelSSE42) &&
              isSplatAndConst(GEPIdx, 1, LoadCount, ConstCount, ScalarCount)) ||
              isSplatAndConstRestrictI16(GEPIdx, 1, LoadCount, ConstCount,
                                         ScalarCount)) {
        Indices.push_back(nullptr);
        continue;
      }

      // Don't know how to scalarize this yet.
      return nullptr;
    }
  }

  for (unsigned i = 1, e = GEP->getNumOperands(); i != e; ++i) {
    Value *GEPIdx = GEP->getOperand(i);
    if (Indices[i - 1] != nullptr)
      continue;
    Indices[i - 1] = createSplatAndConstExpr(GEPIdx, Element, Builder);
  }

  // Create a GEP from the scalar components.
  if (BC) {
    auto BCTy = cast<FixedVectorType>(BC->getType());
    Base = Builder.CreateBitCast(Base, BCTy->getElementType());
  }
  return Builder.CreateGEP(BasePtrTy, Base, Indices, "Ptr" + Twine(Element));
}

static Value *getScalarAddress(Value *Ptrs, unsigned Element,
                               IRBuilder<> &Builder,
                               const TargetTransformInfo &TTI) {
  unsigned LoadCount = 0;
  unsigned ConstCount = 0;
  unsigned ScalarCount = 0;

  auto BC = dyn_cast<BitCastInst>(Ptrs);
  if (auto *GEP = dyn_cast<GetElementPtrInst>(BC ? BC->getOperand(0) : Ptrs))
    if (Value *V = tryScalarizeGEP(GEP, Element, Builder, LoadCount,
                                   ConstCount, ScalarCount, TTI)) {
      if (BC) {
        auto BCTy = cast<FixedVectorType>(BC->getType());
        return Builder.CreateBitCast(V, BCTy->getElementType());
      }
      return V;
    }

  return Builder.CreateExtractElement(Ptrs, Element, "Ptr" + Twine(Element));
}
#endif // INTEL_CUSTOMIZATION

// Translate a masked gather intrinsic like
// <16 x i32 > @llvm.masked.gather.v16i32( <16 x i32*> %Ptrs, i32 4,
//                               <16 x i1> %Mask, <16 x i32> %Src)
// to a chain of basic blocks, with loading element one-by-one if
// the appropriate mask bit is set
//
// %Ptrs = getelementptr i32, i32* %base, <16 x i64> %ind
// %Mask0 = extractelement <16 x i1> %Mask, i32 0
// br i1 %Mask0, label %cond.load, label %else
//
// cond.load:
// %Ptr0 = extractelement <16 x i32*> %Ptrs, i32 0
// %Load0 = load i32, i32* %Ptr0, align 4
// %Res0 = insertelement <16 x i32> undef, i32 %Load0, i32 0
// br label %else
//
// else:
// %res.phi.else = phi <16 x i32>[%Res0, %cond.load], [undef, %0]
// %Mask1 = extractelement <16 x i1> %Mask, i32 1
// br i1 %Mask1, label %cond.load1, label %else2
//
// cond.load1:
// %Ptr1 = extractelement <16 x i32*> %Ptrs, i32 1
// %Load1 = load i32, i32* %Ptr1, align 4
// %Res1 = insertelement <16 x i32> %res.phi.else, i32 %Load1, i32 1
// br label %else2
// . . .
// %Result = select <16 x i1> %Mask, <16 x i32> %res.phi.select, <16 x i32> %Src
// ret <16 x i32> %Result
static void scalarizeMaskedGather(const DataLayout &DL, CallInst *CI,
                                  DomTreeUpdater *DTU, bool &ModifiedDT, // INTEL
                                  const TargetTransformInfo &TTI) {      // INTEL
  Value *Ptrs = CI->getArgOperand(0);
  Value *Alignment = CI->getArgOperand(1);
  Value *Mask = CI->getArgOperand(2);
  Value *Src0 = CI->getArgOperand(3);

  auto *VecType = cast<FixedVectorType>(CI->getType());
  Type *EltTy = VecType->getElementType();

  IRBuilder<> Builder(CI->getContext());
  Instruction *InsertPt = CI;
  BasicBlock *IfBlock = CI->getParent();
  Builder.SetInsertPoint(InsertPt);
  MaybeAlign AlignVal = cast<ConstantInt>(Alignment)->getMaybeAlignValue();

  Builder.SetCurrentDebugLocation(CI->getDebugLoc());

  // The result vector
  Value *VResult = Src0;
  unsigned VectorWidth = VecType->getNumElements();

  // Shorten the way if the mask is a vector of constants.
  if (isConstantIntVector(Mask)) {
    for (unsigned Idx = 0; Idx < VectorWidth; ++Idx) {
      if (cast<Constant>(Mask)->getAggregateElement(Idx)->isNullValue())
        continue;
      Value *Ptr = getScalarAddress(Ptrs, Idx, Builder, TTI); // INTEL
      LoadInst *Load =
          Builder.CreateAlignedLoad(EltTy, Ptr, AlignVal, "Load" + Twine(Idx));
      VResult =
          Builder.CreateInsertElement(VResult, Load, Idx, "Res" + Twine(Idx));
    }
    CI->replaceAllUsesWith(VResult);
    CI->eraseFromParent();
#if INTEL_CUSTOMIZATION
    // If we scalarized the address, remove it too.
    if (Ptrs->use_empty())
      RecursivelyDeleteTriviallyDeadInstructions(Ptrs);
#endif
    return;
  }

  // If the mask is not v1i1, use scalar bit test operations. This generates
  // better results on X86 at least.
  Value *SclrMask;
  if (VectorWidth != 1) {
    Type *SclrMaskTy = Builder.getIntNTy(VectorWidth);
    SclrMask = Builder.CreateBitCast(Mask, SclrMaskTy, "scalar_mask");
  }

  for (unsigned Idx = 0; Idx < VectorWidth; ++Idx) {
    // Fill the "else" block, created in the previous iteration
    //
    //  %Mask1 = and i16 %scalar_mask, i32 1 << Idx
    //  %cond = icmp ne i16 %mask_1, 0
    //  br i1 %Mask1, label %cond.load, label %else
    //

    Value *Predicate;
    if (VectorWidth != 1) {
      Value *Mask = Builder.getInt(APInt::getOneBitSet(
          VectorWidth, adjustForEndian(DL, VectorWidth, Idx)));
      Predicate = Builder.CreateICmpNE(Builder.CreateAnd(SclrMask, Mask),
                                       Builder.getIntN(VectorWidth, 0));
    } else {
      Predicate = Builder.CreateExtractElement(Mask, Idx, "Mask" + Twine(Idx));
    }

    // Create "cond" block
    //
    //  %EltAddr = getelementptr i32* %1, i32 0
    //  %Elt = load i32* %EltAddr
    //  VResult = insertelement <16 x i32> VResult, i32 %Elt, i32 Idx
    //
    Instruction *ThenTerm =
        SplitBlockAndInsertIfThen(Predicate, InsertPt, /*Unreachable=*/false,
                                  /*BranchWeights=*/nullptr, DTU);

    BasicBlock *CondBlock = ThenTerm->getParent();
    CondBlock->setName("cond.load");

    Builder.SetInsertPoint(CondBlock->getTerminator());
    Value *Ptr = getScalarAddress(Ptrs, Idx, Builder, TTI); // INTEL
    LoadInst *Load =
        Builder.CreateAlignedLoad(EltTy, Ptr, AlignVal, "Load" + Twine(Idx));
    Value *NewVResult =
        Builder.CreateInsertElement(VResult, Load, Idx, "Res" + Twine(Idx));

    // Create "else" block, fill it in the next iteration
    BasicBlock *NewIfBlock = ThenTerm->getSuccessor(0);
    NewIfBlock->setName("else");
    BasicBlock *PrevIfBlock = IfBlock;
    IfBlock = NewIfBlock;

    // Create the phi to join the new and previous value.
    Builder.SetInsertPoint(NewIfBlock, NewIfBlock->begin());
    PHINode *Phi = Builder.CreatePHI(VecType, 2, "res.phi.else");
    Phi->addIncoming(NewVResult, CondBlock);
    Phi->addIncoming(VResult, PrevIfBlock);
    VResult = Phi;
  }

  CI->replaceAllUsesWith(VResult);
  CI->eraseFromParent();

  ModifiedDT = true;
}

// Translate a masked scatter intrinsic, like
// void @llvm.masked.scatter.v16i32(<16 x i32> %Src, <16 x i32*>* %Ptrs, i32 4,
//                                  <16 x i1> %Mask)
// to a chain of basic blocks, that stores element one-by-one if
// the appropriate mask bit is set.
//
// %Ptrs = getelementptr i32, i32* %ptr, <16 x i64> %ind
// %Mask0 = extractelement <16 x i1> %Mask, i32 0
// br i1 %Mask0, label %cond.store, label %else
//
// cond.store:
// %Elt0 = extractelement <16 x i32> %Src, i32 0
// %Ptr0 = extractelement <16 x i32*> %Ptrs, i32 0
// store i32 %Elt0, i32* %Ptr0, align 4
// br label %else
//
// else:
// %Mask1 = extractelement <16 x i1> %Mask, i32 1
// br i1 %Mask1, label %cond.store1, label %else2
//
// cond.store1:
// %Elt1 = extractelement <16 x i32> %Src, i32 1
// %Ptr1 = extractelement <16 x i32*> %Ptrs, i32 1
// store i32 %Elt1, i32* %Ptr1, align 4
// br label %else2
//   . . .
static void scalarizeMaskedScatter(const DataLayout &DL, CallInst *CI,
                                   DomTreeUpdater *DTU, bool &ModifiedDT, // INTEL
                                   const TargetTransformInfo &TTI) {      // INTEL
  Value *Src = CI->getArgOperand(0);
  Value *Ptrs = CI->getArgOperand(1);
  Value *Alignment = CI->getArgOperand(2);
  Value *Mask = CI->getArgOperand(3);

  auto *SrcFVTy = cast<FixedVectorType>(Src->getType());

  assert(
      isa<VectorType>(Ptrs->getType()) &&
      isa<PointerType>(cast<VectorType>(Ptrs->getType())->getElementType()) &&
      "Vector of pointers is expected in masked scatter intrinsic");

  IRBuilder<> Builder(CI->getContext());
  Instruction *InsertPt = CI;
  Builder.SetInsertPoint(InsertPt);
  Builder.SetCurrentDebugLocation(CI->getDebugLoc());

  MaybeAlign AlignVal = cast<ConstantInt>(Alignment)->getMaybeAlignValue();
  unsigned VectorWidth = SrcFVTy->getNumElements();

  // Shorten the way if the mask is a vector of constants.
  if (isConstantIntVector(Mask)) {
    for (unsigned Idx = 0; Idx < VectorWidth; ++Idx) {
      if (cast<Constant>(Mask)->getAggregateElement(Idx)->isNullValue())
        continue;
      Value *OneElt =
          Builder.CreateExtractElement(Src, Idx, "Elt" + Twine(Idx));
      Value *Ptr = getScalarAddress(Ptrs, Idx, Builder, TTI); // INTEL
      Builder.CreateAlignedStore(OneElt, Ptr, AlignVal);
    }
    CI->eraseFromParent();
#if INTEL_CUSTOMIZATION
    // If we scalarized the address, remove it too.
    if (Ptrs->use_empty())
      RecursivelyDeleteTriviallyDeadInstructions(Ptrs);
#endif
    return;
  }

  // If the mask is not v1i1, use scalar bit test operations. This generates
  // better results on X86 at least.
  Value *SclrMask;
  if (VectorWidth != 1) {
    Type *SclrMaskTy = Builder.getIntNTy(VectorWidth);
    SclrMask = Builder.CreateBitCast(Mask, SclrMaskTy, "scalar_mask");
  }

  for (unsigned Idx = 0; Idx < VectorWidth; ++Idx) {
    // Fill the "else" block, created in the previous iteration
    //
    //  %Mask1 = and i16 %scalar_mask, i32 1 << Idx
    //  %cond = icmp ne i16 %mask_1, 0
    //  br i1 %Mask1, label %cond.store, label %else
    //
    Value *Predicate;
    if (VectorWidth != 1) {
      Value *Mask = Builder.getInt(APInt::getOneBitSet(
          VectorWidth, adjustForEndian(DL, VectorWidth, Idx)));
      Predicate = Builder.CreateICmpNE(Builder.CreateAnd(SclrMask, Mask),
                                       Builder.getIntN(VectorWidth, 0));
    } else {
      Predicate = Builder.CreateExtractElement(Mask, Idx, "Mask" + Twine(Idx));
    }

    // Create "cond" block
    //
    //  %Elt1 = extractelement <16 x i32> %Src, i32 1
    //  %Ptr1 = extractelement <16 x i32*> %Ptrs, i32 1
    //  %store i32 %Elt1, i32* %Ptr1
    //
    Instruction *ThenTerm =
        SplitBlockAndInsertIfThen(Predicate, InsertPt, /*Unreachable=*/false,
                                  /*BranchWeights=*/nullptr, DTU);

    BasicBlock *CondBlock = ThenTerm->getParent();
    CondBlock->setName("cond.store");

    Builder.SetInsertPoint(CondBlock->getTerminator());
    Value *OneElt = Builder.CreateExtractElement(Src, Idx, "Elt" + Twine(Idx));
    Value *Ptr = getScalarAddress(Ptrs, Idx, Builder, TTI); // INTEL
    Builder.CreateAlignedStore(OneElt, Ptr, AlignVal);

    // Create "else" block, fill it in the next iteration
    BasicBlock *NewIfBlock = ThenTerm->getSuccessor(0);
    NewIfBlock->setName("else");

    Builder.SetInsertPoint(NewIfBlock, NewIfBlock->begin());
  }
  CI->eraseFromParent();

  ModifiedDT = true;
}

static void scalarizeMaskedExpandLoad(const DataLayout &DL, CallInst *CI,
                                      DomTreeUpdater *DTU, bool &ModifiedDT) {
  Value *Ptr = CI->getArgOperand(0);
  Value *Mask = CI->getArgOperand(1);
  Value *PassThru = CI->getArgOperand(2);

  auto *VecType = cast<FixedVectorType>(CI->getType());

  Type *EltTy = VecType->getElementType();

  IRBuilder<> Builder(CI->getContext());
  Instruction *InsertPt = CI;
  BasicBlock *IfBlock = CI->getParent();

  Builder.SetInsertPoint(InsertPt);
  Builder.SetCurrentDebugLocation(CI->getDebugLoc());

  unsigned VectorWidth = VecType->getNumElements();

  // The result vector
  Value *VResult = PassThru;

  // Shorten the way if the mask is a vector of constants.
  // Create a build_vector pattern, with loads/undefs as necessary and then
  // shuffle blend with the pass through value.
  if (isConstantIntVector(Mask)) {
    unsigned MemIndex = 0;
    VResult = UndefValue::get(VecType);
    SmallVector<int, 16> ShuffleMask(VectorWidth, UndefMaskElem);
    for (unsigned Idx = 0; Idx < VectorWidth; ++Idx) {
      Value *InsertElt;
      if (cast<Constant>(Mask)->getAggregateElement(Idx)->isNullValue()) {
        InsertElt = UndefValue::get(EltTy);
        ShuffleMask[Idx] = Idx + VectorWidth;
      } else {
        Value *NewPtr =
            Builder.CreateConstInBoundsGEP1_32(EltTy, Ptr, MemIndex);
        InsertElt = Builder.CreateAlignedLoad(EltTy, NewPtr, Align(1),
                                              "Load" + Twine(Idx));
        ShuffleMask[Idx] = Idx;
        ++MemIndex;
      }
      VResult = Builder.CreateInsertElement(VResult, InsertElt, Idx,
                                            "Res" + Twine(Idx));
    }
    VResult = Builder.CreateShuffleVector(VResult, PassThru, ShuffleMask);
    CI->replaceAllUsesWith(VResult);
    CI->eraseFromParent();
    return;
  }

  // If the mask is not v1i1, use scalar bit test operations. This generates
  // better results on X86 at least.
  Value *SclrMask;
  if (VectorWidth != 1) {
    Type *SclrMaskTy = Builder.getIntNTy(VectorWidth);
    SclrMask = Builder.CreateBitCast(Mask, SclrMaskTy, "scalar_mask");
  }

  for (unsigned Idx = 0; Idx < VectorWidth; ++Idx) {
    // Fill the "else" block, created in the previous iteration
    //
    //  %res.phi.else3 = phi <16 x i32> [ %11, %cond.load1 ], [ %res.phi.else, %else ]
    //  %mask_1 = extractelement <16 x i1> %mask, i32 Idx
    //  br i1 %mask_1, label %cond.load, label %else
    //

    Value *Predicate;
    if (VectorWidth != 1) {
      Value *Mask = Builder.getInt(APInt::getOneBitSet(
          VectorWidth, adjustForEndian(DL, VectorWidth, Idx)));
      Predicate = Builder.CreateICmpNE(Builder.CreateAnd(SclrMask, Mask),
                                       Builder.getIntN(VectorWidth, 0));
    } else {
      Predicate = Builder.CreateExtractElement(Mask, Idx, "Mask" + Twine(Idx));
    }

    // Create "cond" block
    //
    //  %EltAddr = getelementptr i32* %1, i32 0
    //  %Elt = load i32* %EltAddr
    //  VResult = insertelement <16 x i32> VResult, i32 %Elt, i32 Idx
    //
    Instruction *ThenTerm =
        SplitBlockAndInsertIfThen(Predicate, InsertPt, /*Unreachable=*/false,
                                  /*BranchWeights=*/nullptr, DTU);

    BasicBlock *CondBlock = ThenTerm->getParent();
    CondBlock->setName("cond.load");

    Builder.SetInsertPoint(CondBlock->getTerminator());
    LoadInst *Load = Builder.CreateAlignedLoad(EltTy, Ptr, Align(1));
    Value *NewVResult = Builder.CreateInsertElement(VResult, Load, Idx);

    // Move the pointer if there are more blocks to come.
    Value *NewPtr;
    if ((Idx + 1) != VectorWidth)
      NewPtr = Builder.CreateConstInBoundsGEP1_32(EltTy, Ptr, 1);

    // Create "else" block, fill it in the next iteration
    BasicBlock *NewIfBlock = ThenTerm->getSuccessor(0);
    NewIfBlock->setName("else");
    BasicBlock *PrevIfBlock = IfBlock;
    IfBlock = NewIfBlock;

    // Create the phi to join the new and previous value.
    Builder.SetInsertPoint(NewIfBlock, NewIfBlock->begin());
    PHINode *ResultPhi = Builder.CreatePHI(VecType, 2, "res.phi.else");
    ResultPhi->addIncoming(NewVResult, CondBlock);
    ResultPhi->addIncoming(VResult, PrevIfBlock);
    VResult = ResultPhi;

    // Add a PHI for the pointer if this isn't the last iteration.
    if ((Idx + 1) != VectorWidth) {
      PHINode *PtrPhi = Builder.CreatePHI(Ptr->getType(), 2, "ptr.phi.else");
      PtrPhi->addIncoming(NewPtr, CondBlock);
      PtrPhi->addIncoming(Ptr, PrevIfBlock);
      Ptr = PtrPhi;
    }
  }

  CI->replaceAllUsesWith(VResult);
  CI->eraseFromParent();

  ModifiedDT = true;
}

static void scalarizeMaskedCompressStore(const DataLayout &DL, CallInst *CI,
                                         DomTreeUpdater *DTU,
                                         bool &ModifiedDT) {
  Value *Src = CI->getArgOperand(0);
  Value *Ptr = CI->getArgOperand(1);
  Value *Mask = CI->getArgOperand(2);

  auto *VecType = cast<FixedVectorType>(Src->getType());

  IRBuilder<> Builder(CI->getContext());
  Instruction *InsertPt = CI;
  BasicBlock *IfBlock = CI->getParent();

  Builder.SetInsertPoint(InsertPt);
  Builder.SetCurrentDebugLocation(CI->getDebugLoc());

  Type *EltTy = VecType->getElementType();

  unsigned VectorWidth = VecType->getNumElements();

  // Shorten the way if the mask is a vector of constants.
  if (isConstantIntVector(Mask)) {
    unsigned MemIndex = 0;
    for (unsigned Idx = 0; Idx < VectorWidth; ++Idx) {
      if (cast<Constant>(Mask)->getAggregateElement(Idx)->isNullValue())
        continue;
      Value *OneElt =
          Builder.CreateExtractElement(Src, Idx, "Elt" + Twine(Idx));
      Value *NewPtr = Builder.CreateConstInBoundsGEP1_32(EltTy, Ptr, MemIndex);
      Builder.CreateAlignedStore(OneElt, NewPtr, Align(1));
      ++MemIndex;
    }
    CI->eraseFromParent();
    return;
  }

  // If the mask is not v1i1, use scalar bit test operations. This generates
  // better results on X86 at least.
  Value *SclrMask;
  if (VectorWidth != 1) {
    Type *SclrMaskTy = Builder.getIntNTy(VectorWidth);
    SclrMask = Builder.CreateBitCast(Mask, SclrMaskTy, "scalar_mask");
  }

  for (unsigned Idx = 0; Idx < VectorWidth; ++Idx) {
    // Fill the "else" block, created in the previous iteration
    //
    //  %mask_1 = extractelement <16 x i1> %mask, i32 Idx
    //  br i1 %mask_1, label %cond.store, label %else
    //
    Value *Predicate;
    if (VectorWidth != 1) {
      Value *Mask = Builder.getInt(APInt::getOneBitSet(
          VectorWidth, adjustForEndian(DL, VectorWidth, Idx)));
      Predicate = Builder.CreateICmpNE(Builder.CreateAnd(SclrMask, Mask),
                                       Builder.getIntN(VectorWidth, 0));
    } else {
      Predicate = Builder.CreateExtractElement(Mask, Idx, "Mask" + Twine(Idx));
    }

    // Create "cond" block
    //
    //  %OneElt = extractelement <16 x i32> %Src, i32 Idx
    //  %EltAddr = getelementptr i32* %1, i32 0
    //  %store i32 %OneElt, i32* %EltAddr
    //
    Instruction *ThenTerm =
        SplitBlockAndInsertIfThen(Predicate, InsertPt, /*Unreachable=*/false,
                                  /*BranchWeights=*/nullptr, DTU);

    BasicBlock *CondBlock = ThenTerm->getParent();
    CondBlock->setName("cond.store");

    Builder.SetInsertPoint(CondBlock->getTerminator());
    Value *OneElt = Builder.CreateExtractElement(Src, Idx);
    Builder.CreateAlignedStore(OneElt, Ptr, Align(1));

    // Move the pointer if there are more blocks to come.
    Value *NewPtr;
    if ((Idx + 1) != VectorWidth)
      NewPtr = Builder.CreateConstInBoundsGEP1_32(EltTy, Ptr, 1);

    // Create "else" block, fill it in the next iteration
    BasicBlock *NewIfBlock = ThenTerm->getSuccessor(0);
    NewIfBlock->setName("else");
    BasicBlock *PrevIfBlock = IfBlock;
    IfBlock = NewIfBlock;

    Builder.SetInsertPoint(NewIfBlock, NewIfBlock->begin());

    // Add a PHI for the pointer if this isn't the last iteration.
    if ((Idx + 1) != VectorWidth) {
      PHINode *PtrPhi = Builder.CreatePHI(Ptr->getType(), 2, "ptr.phi.else");
      PtrPhi->addIncoming(NewPtr, CondBlock);
      PtrPhi->addIncoming(Ptr, PrevIfBlock);
      Ptr = PtrPhi;
    }
  }
  CI->eraseFromParent();

  ModifiedDT = true;
}

static bool runImpl(Function &F, const TargetTransformInfo &TTI,
                    DominatorTree *DT) {
  Optional<DomTreeUpdater> DTU;
  if (DT)
    DTU.emplace(DT, DomTreeUpdater::UpdateStrategy::Lazy);

  bool EverMadeChange = false;
  bool MadeChange = true;
  auto &DL = F.getParent()->getDataLayout();
  while (MadeChange) {
    MadeChange = false;
    for (BasicBlock &BB : llvm::make_early_inc_range(F)) {
      bool ModifiedDTOnIteration = false;
      MadeChange |= optimizeBlock(BB, ModifiedDTOnIteration, TTI, DL,
                                  DTU ? DTU.getPointer() : nullptr);

      // Restart BB iteration if the dominator tree of the Function was changed
      if (ModifiedDTOnIteration)
        break;
    }

    EverMadeChange |= MadeChange;
  }
  return EverMadeChange;
}

bool ScalarizeMaskedMemIntrinLegacyPass::runOnFunction(Function &F) {
  auto &TTI = getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
  DominatorTree *DT = nullptr;
  if (auto *DTWP = getAnalysisIfAvailable<DominatorTreeWrapperPass>())
    DT = &DTWP->getDomTree();
  return runImpl(F, TTI, DT);
}

PreservedAnalyses
ScalarizeMaskedMemIntrinPass::run(Function &F, FunctionAnalysisManager &AM) {
  auto &TTI = AM.getResult<TargetIRAnalysis>(F);
  auto *DT = AM.getCachedResult<DominatorTreeAnalysis>(F);
  if (!runImpl(F, TTI, DT))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<TargetIRAnalysis>();
  PA.preserve<DominatorTreeAnalysis>();
  return PA;
}

static bool optimizeBlock(BasicBlock &BB, bool &ModifiedDT,
                          const TargetTransformInfo &TTI, const DataLayout &DL,
                          DomTreeUpdater *DTU) {
  bool MadeChange = false;

  BasicBlock::iterator CurInstIterator = BB.begin();
  while (CurInstIterator != BB.end()) {
    if (CallInst *CI = dyn_cast<CallInst>(&*CurInstIterator++))
      MadeChange |= optimizeCallInst(CI, ModifiedDT, TTI, DL, DTU);
    if (ModifiedDT)
      return true;
  }

  return MadeChange;
}

static bool optimizeCallInst(CallInst *CI, bool &ModifiedDT,
                             const TargetTransformInfo &TTI,
                             const DataLayout &DL, DomTreeUpdater *DTU) {
  IntrinsicInst *II = dyn_cast<IntrinsicInst>(CI);
  if (II) {
    // The scalarization code below does not work for scalable vectors.
    if (isa<ScalableVectorType>(II->getType()) ||
        any_of(II->args(),
               [](Value *V) { return isa<ScalableVectorType>(V->getType()); }))
      return false;

    switch (II->getIntrinsicID()) {
    default:
      break;
    case Intrinsic::masked_load:
      // Scalarize unsupported vector masked load
      if (TTI.isLegalMaskedLoad(
              CI->getType(),
              cast<ConstantInt>(CI->getArgOperand(1))->getAlignValue()))
        return false;
      scalarizeMaskedLoad(DL, CI, DTU, ModifiedDT);
      return true;
    case Intrinsic::masked_store:
      if (TTI.isLegalMaskedStore(
              CI->getArgOperand(0)->getType(),
              cast<ConstantInt>(CI->getArgOperand(2))->getAlignValue()))
        return false;
      scalarizeMaskedStore(DL, CI, DTU, ModifiedDT);
      return true;
    case Intrinsic::masked_gather: {
      MaybeAlign MA =
          cast<ConstantInt>(CI->getArgOperand(1))->getMaybeAlignValue();
      Type *LoadTy = CI->getType();
      Align Alignment = DL.getValueOrABITypeAlignment(MA,
                                                      LoadTy->getScalarType());
#if INTEL_CUSTOMIZATION
      if (!TTI.shouldScalarizeMaskedGather(CI) &&
          !TTI.forceScalarizeMaskedGather(cast<VectorType>(LoadTy), Alignment))
#endif // INTEL_CUSTOMIZATION
        return false;
      scalarizeMaskedGather(DL, CI, DTU, ModifiedDT, TTI);
      return true;
    }
    case Intrinsic::masked_scatter: {
      MaybeAlign MA =
          cast<ConstantInt>(CI->getArgOperand(2))->getMaybeAlignValue();
      Type *StoreTy = CI->getArgOperand(0)->getType();
      Align Alignment = DL.getValueOrABITypeAlignment(MA,
                                                      StoreTy->getScalarType());
      if (TTI.isLegalMaskedScatter(StoreTy, Alignment) &&
          !TTI.forceScalarizeMaskedScatter(cast<VectorType>(StoreTy),
                                           Alignment))
        return false;
      scalarizeMaskedScatter(DL, CI, DTU, ModifiedDT, TTI);
      return true;
    }
    case Intrinsic::masked_expandload:
      if (TTI.isLegalMaskedExpandLoad(CI->getType()))
        return false;
      scalarizeMaskedExpandLoad(DL, CI, DTU, ModifiedDT);
      return true;
    case Intrinsic::masked_compressstore:
      if (TTI.isLegalMaskedCompressStore(CI->getArgOperand(0)->getType()))
        return false;
      scalarizeMaskedCompressStore(DL, CI, DTU, ModifiedDT);
      return true;
    }
  }

  return false;
}

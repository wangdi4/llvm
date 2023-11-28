//====-- Intel_X86Gather2LoadPermute.cpp ----------------====
//
//      Copyright (c) 2019 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//
// This file defines the pass which will transform gather
// instruction to load + permute.
//
// Example:
// %struct.2 = type {[2 x float], %struct.2*, %struct.2* }
// %gep = getelementptr inbounds %struct.2, %struct.2* %node, i64 0, i32 0,
//        <6 x i64> %index_i64
// %res = call <6 x float> @llvm.masked.gather.v6f32.v6p0f32(<6 x float*> %gep,
//        i32 4, <6 x i1> <i1 false, i1 true, i1 true, i1 true, i1 true,
//        i1 true>, <6 x float> undef)
// ==>
// %0 = getelementptr %struct.2, %struct.2* %node, i64 0, i32 0, i32 0
// %1 = bitcast float* %0 to <2 x float>*
// %2 = load <2 x float>, <2 x float>* %1, align 4
// %3 = shufflevector <2 x float> %2, <2 x float> undef, <8 x i32> <i32 0,
//      i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef,
//      i32 undef>
// %4 = trunc <6 x i64> %index_i64 to <6 x i32>
// %5 = shufflevector <6 x i32> %4, <6 x i32> undef, <8 x i32> <i32 0, i32 1,
//      i32 2, i32 3, i32 4, i32 5, i32 undef, i32 undef>
// %6 = call <8 x float> @llvm.x86.avx2.permps(<8 x float> %3, <8 x i32> %5)
// %res = shufflevector <8 x float> %6, <8 x float> undef, <6 x i32> <i32 0,
//        i32 1, i32 2, i32 3, i32 4, i32 5>
//

#include "X86.h"
#include "X86Subtarget.h"
#include "X86TargetMachine.h"
#include "llvm/ADT/FloatingPointMode.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/ConstantRange.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicsX86.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

#define DEBUG_TYPE "x86-gather-to-load-permute"

namespace {

class X86Gather2LoadPermutePass : public FunctionPass {
  const TargetTransformInfo *TTI = nullptr;

  void genMask(Constant *&PreShuffleMask, Constant *&PostShuffleMask,
               Constant *&LoadShuffleMask, unsigned GatherNum, uint64_t LoadNum,
               unsigned WidenNum, IRBuilder<> &Builder) {
    SmallVector<Constant *, 8> PreShuffleMaskVec;
    SmallVector<Constant *, 8> PostShuffleMaskVec;
    SmallVector<Constant *, 8> LoadShuffleMaskVec;

    for (unsigned I = 0; I < GatherNum; ++I) {
      PreShuffleMaskVec.push_back(Builder.getInt32(I));
      PostShuffleMaskVec.push_back(Builder.getInt32(I));
    }
    for (unsigned I = GatherNum; I < WidenNum; ++I) {
      UndefValue *Undef = UndefValue::get(PreShuffleMaskVec[0]->getType());
      PreShuffleMaskVec.push_back(Undef);
    }
    for (unsigned I = 0; I < LoadNum; ++I) {
      LoadShuffleMaskVec.push_back(Builder.getInt32(I));
    }
    for (unsigned I = LoadNum; I < WidenNum; ++I) {
      UndefValue *Undef = UndefValue::get(PreShuffleMaskVec[0]->getType());
      LoadShuffleMaskVec.push_back(Undef);
    }
    PreShuffleMask = ConstantVector::get(PreShuffleMaskVec);
    PostShuffleMask = ConstantVector::get(PostShuffleMaskVec);
    LoadShuffleMask = ConstantVector::get(LoadShuffleMaskVec);
  }

  Intrinsic::ID getPermuteIntrinsicID(Type *ArrayElemTy,
                                      uint32_t WidenNum) const {
    if (ArrayElemTy->isIntegerTy(32)) {
      if (WidenNum == 8) {
        return Intrinsic::x86_avx2_permd;
      }
    } else if (ArrayElemTy->isFloatTy()) {
      if (WidenNum == 8) {
        return Intrinsic::x86_avx2_permps;
      }
    }

    llvm_unreachable(
        "shouldOptGatherToLoadPermute does not match this function!");
    return Intrinsic::num_intrinsics;
  }

  bool optimizeGather2LoadPermute(IntrinsicInst *II) {
    Value *Ptrs = II->getArgOperand(0);
    Value *Alignment = II->getArgOperand(1);

    uint64_t ArrayNum = 0;
    unsigned GatherNum = 0;
    unsigned WidenNum = 0;
    Type *ArrayEleTy = nullptr;

    if (!TTI->isLegalToTransformGather2PermuteLoad(II, ArrayEleTy, ArrayNum,
                                                   GatherNum, WidenNum))
      return false;

    auto *GEP = cast<GetElementPtrInst>(Ptrs);

    // Check if last index is vector type.
    unsigned GEPNumOper = GEP->getNumOperands();
    Value *LastIndex = GEP->getOperand(GEPNumOper - 1);

    Intrinsic::ID PermuteID =
        getPermuteIntrinsicID(ArrayEleTy, WidenNum);
    if (PermuteID == Intrinsic::num_intrinsics)
      return false;

    LLVM_DEBUG({
      dbgs() << "Found:\n";
      II->dump();
      dbgs() << "======>\n";
    });

    IRBuilder<> Builder(II);

    Constant *PreShuffleMask = nullptr, *PostShuffleMask = nullptr,
             *LoadShuffleMask = nullptr;
    // Generate the shuffle masks.
    genMask(PreShuffleMask, PostShuffleMask, LoadShuffleMask, GatherNum,
            ArrayNum, WidenNum, Builder);

    // Build a new indices whose's last index is zero.
    SmallVector<Value*, 8> Indices(GEP->indices());
    Indices[Indices.size() - 1] =
        ConstantInt::getNullValue(Builder.getInt32Ty());

    // Create GEP with new indices.
    Value *ZeroLastIdxGEP = Builder.CreateGEP(
        GEP->getSourceElementType(), GEP->getPointerOperand(), Indices);

    // Bitcast the new GEP to the array size's vector pointer.
    PointerType *ZeroLastIdxGEPTy =
        cast<PointerType>(ZeroLastIdxGEP->getType());
    FixedVectorType *BitCastTy = FixedVectorType::get(ArrayEleTy, ArrayNum);
    PointerType *ArrayPtrTy =
        PointerType::get(BitCastTy, ZeroLastIdxGEPTy->getAddressSpace());
    Value *BitCast = Builder.CreateBitCast(ZeroLastIdxGEP, ArrayPtrTy);

    // Load the whole array.
    unsigned AlignVal = cast<ConstantInt>(Alignment)->getZExtValue();
    LoadInst *Load =
        Builder.CreateAlignedLoad(BitCastTy, BitCast, MaybeAlign(AlignVal));
    // Widen load to WidenNum.
    Value *WidenLoad = Builder.CreateShuffleVector(
        Load, UndefValue::get(Load->getType()), LoadShuffleMask);

    // Make sure LastIndexTy's scalar type is euqal to ArrayEleTy's type.
    // This is permute intrinsic's definition.
    auto *LastIndexTy = cast<FixedVectorType>(LastIndex->getType());
    if (LastIndexTy->getScalarType()->getScalarSizeInBits() !=
        ArrayEleTy->getScalarSizeInBits()) {
      Type *NewLastIndexType =
          Builder.getIntNTy(ArrayEleTy->getScalarSizeInBits());
      LastIndex = Builder.CreateSExtOrTrunc(
          LastIndex, FixedVectorType::get(NewLastIndexType, GatherNum));
    }

    // Widen LastIndex to WidenNum.
    Value *IndirectIndex = Builder.CreateShuffleVector(
        LastIndex, UndefValue::get(LastIndex->getType()), PreShuffleMask);

    // Get the indriected value.
    SmallVector<Type *, 1> NullTy;
    CallInst *Permute =
        Builder.CreateIntrinsic(PermuteID, NullTy, {WidenLoad, IndirectIndex});

    // Shuffle permuted value to match gather's type.
    Value *Result = Builder.CreateShuffleVector(
        Permute, UndefValue::get(Permute->getType()), PostShuffleMask);

    II->replaceAllUsesWith(Result);
    II->eraseFromParent();

    LLVM_DEBUG({
      ZeroLastIdxGEP->dump();
      BitCast->dump();
      Load->dump();
      WidenLoad->dump();
      IndirectIndex->dump();
      Permute->dump();
      Result->dump();
      dbgs() << "=====================\n";
    });

    return true;
  }

  bool optimizeCallInst(CallInst *CI) {
    IntrinsicInst *II = dyn_cast<IntrinsicInst>(CI);
    if (!II)
      return false;
    // The optimization code below does not work for scalable vectors.
    if (isa<ScalableVectorType>(II->getType()) ||
        any_of(II->args(),
               [](Value *V) { return isa<ScalableVectorType>(V->getType()); }))
      return false;

    switch (II->getIntrinsicID()) {
    default:
      break;
    case Intrinsic::masked_gather:
      return optimizeGather2LoadPermute(II);
    }
    return false;
  }

  bool optimizeBlock(BasicBlock &BB) {
    bool MadeChange = false;

    BasicBlock::iterator CurInstIterator = BB.begin();
    while (CurInstIterator != BB.end()) {
      if (CallInst *CI = dyn_cast<CallInst>(&*CurInstIterator++))
        MadeChange |= optimizeCallInst(CI);
    }

    return MadeChange;
  }

public:
  static char ID; // Pass identification, replacement for typeid..

  X86Gather2LoadPermutePass() : FunctionPass(ID) {
    initializeX86Gather2LoadPermutePassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetPassConfig>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
  }

  bool runOnFunction(Function &F) override {
    TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);

    if (skipFunction(F))
      return false;

    bool MadeChange = false;
    for (Function::iterator I = F.begin(); I != F.end();) {
      BasicBlock *BB = &*I++;
      MadeChange |= optimizeBlock(*BB);
    }
    return MadeChange;
  }
};

} // end anonymous namespace

char X86Gather2LoadPermutePass::ID = 0;

INITIALIZE_PASS_BEGIN(X86Gather2LoadPermutePass, DEBUG_TYPE,
                      "X86 transform gather to load + permute pass", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(X86Gather2LoadPermutePass, DEBUG_TYPE,
                    "X86 transform gather to load + permute pass", false, false)

FunctionPass *llvm::createX86Gather2LoadPermutePass() {
  return new X86Gather2LoadPermutePass();
}

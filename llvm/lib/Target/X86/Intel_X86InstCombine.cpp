//===--------- Intel_X86InstCombine.cpp - X86 Instruction Combine ---------===//
//
// Copyright (C) 2019-2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass is mainly anti middle end optimization which is not really good
// for X86 target, at the same time, it is not a good choice to disable it
// at middle end. Just put these optimizations here.
//
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86Subtarget.h"
#include "X86TargetMachine.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicsX86.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/Intel_IMLUtils.h"
#include "llvm/Transforms/Utils/Local.h"

using namespace llvm;
using namespace llvm::PatternMatch;

#define DEBUG_TYPE "x86-inst-combine"

static cl::opt<bool> LinkMSVCCPPRuntimeLib(
    "link-msvc-cpp-runtime-lib",
    llvm::cl::desc("Indicate if you'll link with MSVC's C++ runtime library."),
    cl::init(false), cl::Hidden);

static cl::opt<bool> ScalarizeAVX2GatherIntrinsic(
    "scalarize-avx2-gather-intrinsic",
    llvm::cl::desc("Indicate if scalarize avx2 gather intrinsic."),
    cl::init(true), cl::Hidden);

static cl::opt<bool> ScalarizeAVX512GatherIntrinsic(
    "scalarize-avx512-gather-intrinsic",
    llvm::cl::desc("Indicate if scalarize avx512 gather intrinsic."),
    cl::init(true), cl::Hidden);

namespace {

class X86InstCombine : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid

  X86InstCombine() : FunctionPass(ID) {
    initializeX86InstCombinePass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.setPreservesCFG();
    AU.addRequired<AssumptionCacheTracker>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<TargetPassConfig>();
    FunctionPass::getAnalysisUsage(AU);
  }

  bool doInitialization(Module &M) override {
    TPC = getAnalysisIfAvailable<TargetPassConfig>();
    if (!TPC)
      return false;

    TM = &TPC->getTM<X86TargetMachine>();
    return false;
  }
  bool runOnFunction(Function &F) override;

private:
  bool replaceOrToAdd(Instruction &I);
  bool replaceFRem(Instruction &I);
  bool replaceX86IntrinsicToIR(Instruction &I);
  bool replaceLibmToSVML(Instruction &I);
  bool replaceFDTest(Instruction &I);

  X86TargetMachine *TM = nullptr;
  const X86Subtarget *ST = nullptr;
  DominatorTree *DT = nullptr;
  TargetTransformInfo *TTI = nullptr;
  AssumptionCache *AC = nullptr;
  TargetPassConfig *TPC = nullptr;
};

} // end anonymous namespace

char X86InstCombine::ID = 0;
char &llvm::X86InstCombineID = X86InstCombine::ID;

INITIALIZE_PASS_BEGIN(X86InstCombine, DEBUG_TYPE,
                    "Instruction Combine for X86 Target", false, false)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(X86InstCombine, DEBUG_TYPE,
                    "Instruction Combine for X86 Target", false, false)

FunctionPass *llvm::createX86InstCombinePass() {
  return new X86InstCombine();
}

static void replaceValue(Instruction &Old, Value &New) {
  Old.replaceAllUsesWith(&New);
  New.takeName(&Old);
}

static bool isBitwiseInst(Value *V) {
  if (auto BinOper = dyn_cast<BinaryOperator>(V))
    if (BinOper->getOpcode() == Instruction::Or ||
        BinOper->getOpcode() == Instruction::And ||
        BinOper->getOpcode() == Instruction::Xor)
      return true;

  return false;
}

bool X86InstCombine::replaceOrToAdd(Instruction &I) {
  Value *LHS = I.getOperand(0), *RHS = I.getOperand(1);
  const DataLayout &DL = I.getModule()->getDataLayout();

  // Check if it is profitable to replace:
  // For avx512, 'Or' with other bitwise instructions may can be
  // transformed to VPTERNLOG.
  if ((ST && ST->hasAVX512()) && I.getType()->isVectorTy()) {
    for (auto User : I.users())
      if (isBitwiseInst(User))
        return false;

    for (Value *V : I.operands())
      if (isBitwiseInst(V))
          return false;
  }

  // Check if 'or' can be transformed to 'add'.
  if (!haveNoCommonBitsSet(LHS, RHS, SimplifyQuery(DL, DT, AC, &I)))
    return false;

  IRBuilder<> Builder(&I);
  auto Add = Builder.CreateAdd(LHS, RHS);
  replaceValue(I, *Add);

  return true;
}

// Transform frem to "a - trunc(a / b) * b",
// This transformation may have different results when a or b is INF,
// but it's OK when we use fast math flags with O3 optimization level.
bool X86InstCombine::replaceFRem(Instruction &I) {
  if (!I.isFast())
    return false;

  Value *LHS = I.getOperand(0);
  Value *RHS = I.getOperand(1);

  bool IsVectorTy = LHS->getType()->isVectorTy();
  Type *Ty = LHS->getType();
  Type *ScalarTy = LHS->getType()->getScalarType();

  if (!ScalarTy->isFloatTy() && !ScalarTy->isDoubleTy())
    return false;

  // Extend to double type?
  bool NeedExtTy = ScalarTy->isFloatTy() ? true : false;

  IRBuilder<> Builder(&I);
  if (NeedExtTy) {
    Type *ExtTy = Type::getDoubleTy(I.getContext());

    if (IsVectorTy)
      ExtTy = VectorType::get(
          ExtTy, cast<VectorType>(LHS->getType())->getElementCount());

    LHS = Builder.CreateFPExt(LHS, ExtTy);
    RHS = Builder.CreateFPExt(RHS, ExtTy);
  }

  Value *ADivB = Builder.CreateFDiv(LHS, RHS);
  Value *Trunc = Builder.CreateUnaryIntrinsic(Intrinsic::trunc, ADivB, &I);
  Value *FMul = Builder.CreateFMul(Trunc, RHS);
  Value *Result = Builder.CreateFSub(LHS, FMul);

  // Back to float type.
  if (NeedExtTy)
    Result = Builder.CreateFPCast(Result, Ty);

  replaceValue(I, *Result);
  return true;
}

static Value *replaceX86GatherToGather(IntrinsicInst *II) {
  bool isAVX512 = false;
  switch (II->getIntrinsicID()) {
  case Intrinsic::x86_avx2_gather_d_d:
  case Intrinsic::x86_avx2_gather_d_d_256:
  case Intrinsic::x86_avx2_gather_d_pd:
  case Intrinsic::x86_avx2_gather_d_pd_256:
  case Intrinsic::x86_avx2_gather_d_ps:
  case Intrinsic::x86_avx2_gather_d_ps_256:
  case Intrinsic::x86_avx2_gather_d_q:
  case Intrinsic::x86_avx2_gather_d_q_256:
  case Intrinsic::x86_avx2_gather_q_d:
  case Intrinsic::x86_avx2_gather_q_d_256:
  case Intrinsic::x86_avx2_gather_q_pd:
  case Intrinsic::x86_avx2_gather_q_pd_256:
  case Intrinsic::x86_avx2_gather_q_ps:
  case Intrinsic::x86_avx2_gather_q_ps_256:
  case Intrinsic::x86_avx2_gather_q_q:
  case Intrinsic::x86_avx2_gather_q_q_256:
    if (!ScalarizeAVX2GatherIntrinsic)
      return nullptr;
    break;
  case Intrinsic::x86_avx512_mask_gather_dpd_512:
  case Intrinsic::x86_avx512_mask_gather_dpi_512:
  case Intrinsic::x86_avx512_mask_gather_dpq_512:
  case Intrinsic::x86_avx512_mask_gather_dps_512:
  case Intrinsic::x86_avx512_mask_gather_qpd_512:
  case Intrinsic::x86_avx512_mask_gather_qpi_512:
  case Intrinsic::x86_avx512_mask_gather_qpq_512:
  case Intrinsic::x86_avx512_mask_gather_qps_512:
  case Intrinsic::x86_avx512_mask_gather3div2_df:
  case Intrinsic::x86_avx512_mask_gather3div2_di:
  case Intrinsic::x86_avx512_mask_gather3div4_df:
  case Intrinsic::x86_avx512_mask_gather3div4_di:
  case Intrinsic::x86_avx512_mask_gather3div4_sf:
  case Intrinsic::x86_avx512_mask_gather3div4_si:
  case Intrinsic::x86_avx512_mask_gather3div8_sf:
  case Intrinsic::x86_avx512_mask_gather3div8_si:
  case Intrinsic::x86_avx512_mask_gather3siv2_df:
  case Intrinsic::x86_avx512_mask_gather3siv2_di:
  case Intrinsic::x86_avx512_mask_gather3siv4_df:
  case Intrinsic::x86_avx512_mask_gather3siv4_di:
  case Intrinsic::x86_avx512_mask_gather3siv4_sf:
  case Intrinsic::x86_avx512_mask_gather3siv4_si:
  case Intrinsic::x86_avx512_mask_gather3siv8_sf:
  case Intrinsic::x86_avx512_mask_gather3siv8_si:
    if (!ScalarizeAVX512GatherIntrinsic)
      return nullptr;
    isAVX512 = true;
    break;
  default:
    return nullptr;
  }

  LLVMContext &C = II->getContext();
  Align Align;

  auto *RetTy = cast<FixedVectorType>(II->getType());

  Value *Mask = II->getOperand(3);
  auto *MaskTy = cast<FixedVectorType>(Mask->getType());

  auto *ScaleInt = cast<ConstantInt>(II->getOperand(4));
  uint64_t Scale = ScaleInt->getLimitedValue();

  Value *PassThru = II->getOperand(0);

  Value *Index = II->getOperand(2);
  auto *IndexTy = cast<FixedVectorType>(Index->getType());

  unsigned MaskNumElement = MaskTy->getNumElements();

  IRBuilder<> Builder(II);
  Value *NewMask = nullptr;

  NewMask = Mask;

  if (!isAVX512) {
    // X86 gather check the MSB of mask, should right shift MSB to LSB for
    // matching llvm.masked.gather's definition.
    if (auto *CV = dyn_cast<ConstantVector>(Mask)) {
      SmallVector<Constant *> Masks;
      for (unsigned I = 0; I < MaskNumElement; ++I) {
          const APInt &APC = CV->getAggregateElement(I)->getUniqueInteger();
          auto *MaskElem = ConstantInt::getIntegerValue(Type::getInt1Ty(C),
                                                        APC.getHiBits(1));
          Masks.push_back(MaskElem);
      }
      NewMask = ConstantVector::get(Masks);
    } else {
      // X86 gather's mask type is same with data type, need extra handling:
      // <8 x float> %mask
      // ==>
      // %IntMask = bitcast %mask to <8 x i32>
      // %ElemHiBit = 31
      // %ShiftV = <8 x i32> <i32 31, i32 31, i32 31, i32 31, i32 31, i32 31,
      // ...> %ShiftMask = lshr <8 x i32> %IntMask, %ShiftV %NewMask = trunc <8
      // x i32> %ShiftMask to <8 x i1>
      unsigned ElemBits = MaskTy->getElementType()->getScalarSizeInBits();
      Value *IntMask = Builder.CreateBitCast(
          Mask,
          FixedVectorType::get(Type::getIntNTy(C, ElemBits), MaskNumElement));

      auto *ElemHiBit =
          ConstantInt::get(Type::getIntNTy(C, ElemBits), ElemBits - 1);
      Value *ShiftV = Builder.CreateVectorSplat(MaskNumElement, ElemHiBit);
      Value *ShiftMask = Builder.CreateLShr(IntMask, ShiftV);
      NewMask = Builder.CreateTrunc(
          ShiftMask, FixedVectorType::get(Type::getInt1Ty(C), MaskNumElement));
    }
  }

  Value *Base = II->getOperand(1);
  auto *BaseTy = Base->getType();
  auto *NewBaseTy = Type::getIntNTy(C, Scale * 8)
                       ->getPointerTo(BaseTy->getPointerAddressSpace());
  Value *NewBase = Builder.CreateBitCast(Base, NewBaseTy);

  unsigned IndexElemCount = IndexTy->getNumElements();
  unsigned RetElemCount = RetTy->getNumElements();

  // For X86 gather, IndexTy and RetTy's minimal size is 128-bit,
  // If any of them is smaller than 128-bit, it should be extended to 128-bit by
  // increase element count, the result is IndexTy's element count is different
  // with RetTy's element count. Need shrink or extend it.
  if (IndexElemCount > RetElemCount) {
    // Example:
    // <2 x i64> @x86.gather.d.q(<2 x i64> %src, i8* @base, <4 x i32> %index)
    // Shrink %index from <4 x i32> to <2 x i32>.
    SmallVector<int> ShufMask;
    for (unsigned I = 0, E = RetElemCount; I < E; ++I)
      ShufMask.push_back(I);
    Index = Builder.CreateShuffleVector(Index, ShufMask);
    IndexTy = cast<FixedVectorType>(Index->getType());
  } else if (IndexElemCount < RetElemCount) {
    // Example:
    // <4 x i32> @x86.gather.q.d(<4 x i32> %src, i8* @base, <2 x i64> %index, <4
    // x i32> %mask)
    // Shrink mask/passthru from <4 x i32> to <2 x i32>.
    RetTy = FixedVectorType::get(RetTy->getElementType(), IndexElemCount);
    SmallVector<int> ShufMask;
    for (unsigned I = 0, E = IndexElemCount; I < E; ++I)
      ShufMask.push_back(I);
    NewMask = Builder.CreateShuffleVector(NewMask, ShufMask);
    PassThru = Builder.CreateShuffleVector(PassThru, ShufMask);
  }

  Value *GEP =
      Builder.CreateInBoundsGEP(Type::getIntNTy(C, Scale * 8), NewBase, Index);

  auto *PtrsTy = FixedVectorType::get(
      RetTy->getElementType()->getPointerTo(BaseTy->getPointerAddressSpace()),
      IndexTy->getNumElements());
  Value *Ptrs = Builder.CreateBitCast(GEP, PtrsTy);

  Value *NewGather =
      Builder.CreateMaskedGather(RetTy, Ptrs, Align, NewMask, PassThru);

  // When IndexElemCount < RetElemCount, it means we have shrink the result type,
  // need to extend it to original type.
  if (IndexElemCount < RetElemCount) {
    SmallVector<int> ShufMask;
    for (unsigned I = 0, E = IndexElemCount; I < E; ++I)
      ShufMask.push_back(I);
    for (unsigned I = IndexElemCount, E = RetElemCount; I < E; ++I)
      ShufMask.push_back(IndexElemCount);

    auto *ZeroInitializer = ConstantDataVector::getSplat(
        IndexElemCount, Constant::getNullValue(RetTy->getElementType()));
    NewGather = Builder.CreateShuffleVector(NewGather, ZeroInitializer, ShufMask);
  }

  return NewGather;
}

// SVML's performance is better than libm for some math functions.
// Try to use SVML version if possible.
bool X86InstCombine::replaceLibmToSVML(Instruction &I) {
  auto *II = dyn_cast<IntrinsicInst>(&I);
  if (!II)
    return false;

  switch (II->getIntrinsicID()) {
  case Intrinsic::log2:
  case Intrinsic::log10:
    break;
  default:
    return false;
  }

  if (!II->isFast())
    return false;

  II->addFnAttr(Attribute::get(I.getContext(), "imf-use-svml", "true"));

  return true;
}

bool X86InstCombine::replaceX86IntrinsicToIR(Instruction &I) {
  auto II = dyn_cast<IntrinsicInst>(&I);
  if (!II)
    return false;

  if (ST && ST->preferGather())
    return false;

  if (auto V = replaceX86GatherToGather(II)) {
    replaceValue(I, *V);
    return true;
  }

  return false;
}

static bool isFDTest(const CallInst *C) {
  auto M = C->getModule();
  Triple T(M->getTargetTriple());

  // Make sure we are using MSVC's C++ runtime library.
  // Otherwise the fdtest may not be the MSVC internal API.
  if (!LinkMSVCCPPRuntimeLib)
    return false;

  if (!T.isWindowsMSVCEnvironment())
    return false;

  Value *CalledOp = C->getCalledOperand();
  if (!CalledOp)
    return false;

  if (CalledOp->getName() != "_fdtest")
    return false;

  if (C->getNumOperands() != 2)
    return false;

  if (!C->getArgOperand(0)->getType()->isPointerTy())
    return false;

  if (C->getType() != Type::getInt16Ty(C->getContext()))
    return false;

  return true;
}

// _fdtest is a MSVC internal API which returns a float-point number's type,
// such as FP_NAN, FP_INFINITE, FP_NORMAL, FP_SUBNORMAL, FP_ZERO. When we want
// to check if a number is NAN, ZERO or some other type, we don't need to know
// what type it is but just check if it is that type. So we can optimize such
// call with bitwise operation. Reference:
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/floating-point-primitives?view=msvc-170#_dtest-_ldtest-_fdtest
bool X86InstCombine::replaceFDTest(Instruction &I) {
  Instruction *C = nullptr;
  uint64_t CmpInt;
  ICmpInst::Predicate Pred;

  // Try to match such pattern:
  // %fdtest = call i16 @_fdtest(ptr %X)
  // %res = icmp P i16 %fdtest, CmpInt
  if (!match(&I,
             m_ICmp(Pred, m_OneUse(m_Instruction(C)), m_ConstantInt(CmpInt))))
    return false;

  if (!isa<CallInst>(C))
    return false;

  if (!isFDTest(cast<CallInst>(C)))
    return false;

  Value *FloatPtr = C->getOperand(0);
  Value *Res = nullptr;

  IRBuilder<> Builder(&I);
  if (Pred == CmpInst::ICMP_EQ || Pred == CmpInst::ICMP_NE) {

    if (CmpInt == 1) {
      // isinf:
      // (i & 0x7fffffff) == 0x7f800000
      auto *IntF =
          Builder.CreateLoad(Type::getInt32Ty(I.getContext()), FloatPtr);
      auto *And = Builder.CreateAnd(IntF, 0x7fffffff);
      Res = Builder.CreateICmp(CmpInst::ICMP_EQ, And,
                               ConstantInt::get(IntF->getType(), 0x7f800000));
    } else if (CmpInt == 2) {
      // isnan:
      // ((i & 0x7f800000) == 0x7f800000) && ((i & 0x7fffff) != 0)
      auto *IntF =
          Builder.CreateLoad(Type::getInt32Ty(I.getContext()), FloatPtr);
      auto *And0 = Builder.CreateAnd(IntF, 0x7f800000);
      auto *Cmp0 =
          Builder.CreateICmp(CmpInst::ICMP_EQ, And0,
                             ConstantInt::get(IntF->getType(), 0x7f800000));
      auto *And1 = Builder.CreateAnd(IntF, 0x7fffff);
      auto *Cmp1 = Builder.CreateICmp(CmpInst::ICMP_NE, And1,
                                      ConstantInt::get(IntF->getType(), 0));
      Res = Builder.CreateAnd(Cmp0, Cmp1);
    } else if (CmpInt == (uint16_t)-1) {
      // isnormal:
      // ((i & 0x7f800000) != 0) && ((i & 0x7f800000) != 0x7f800000)
      auto *IntF =
          Builder.CreateLoad(Type::getInt32Ty(I.getContext()), FloatPtr);
      auto *And = Builder.CreateAnd(IntF, 0x7f800000);
      auto *Cmp0 = Builder.CreateICmp(
          CmpInst::ICMP_NE, And, ConstantInt::get(IntF->getType(), 0x7f800000));
      auto *Cmp1 = Builder.CreateICmp(CmpInst::ICMP_NE, And,
                                      ConstantInt::get(IntF->getType(), 0));
      Res = Builder.CreateAnd(Cmp0, Cmp1);
    }

    if (Pred == CmpInst::ICMP_NE && Res)
      Res = Builder.CreateNot(Res);

  } else if (Pred == CmpInst::ICMP_SLT || Pred == CmpInst::ICMP_SGE) {
    if (CmpInt == 1) {
      // isfinite:
      // (i & 0x7f800000) != 0x7f800000
      auto *IntF =
          Builder.CreateLoad(Type::getInt32Ty(I.getContext()), FloatPtr);
      auto *And = Builder.CreateAnd(IntF, 0x7f800000);
      Res = Builder.CreateICmp(CmpInst::ICMP_NE, And,
                               ConstantInt::get(IntF->getType(), 0x7f800000));
    }

    if (Pred == CmpInst::ICMP_SGE && Res)
      Res = Builder.CreateNot(Res);
  }

  if (!Res)
    return false;

  replaceValue(I, *Res);
  replaceValue(*C, *UndefValue::get(C->getType()));
  C->eraseFromParent();
  return true;
}

bool X86InstCombine::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;

  TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  if (TM)
    ST = &TM->getSubtarget<X86Subtarget>(F);
  AC = &getAnalysis<AssumptionCacheTracker>().getAssumptionCache(F);

  bool MadeChange = false;

  for (Instruction &I : make_early_inc_range(instructions(F))) {
    switch (I.getOpcode()) {
    case Instruction::Call:
      MadeChange |= replaceX86IntrinsicToIR(I);
      break;
    }
  }

  if (TPC && TPC->getOptLevel() >= CodeGenOptLevel::Aggressive) {
    for (Instruction &I : make_early_inc_range(instructions(F))) {
      switch (I.getOpcode()) {
      case Instruction::Or:
        MadeChange |= replaceOrToAdd(I);
        break;
      case Instruction::FRem:
        MadeChange |= replaceFRem(I);
        break;
      case Instruction::Call:
        MadeChange |= replaceLibmToSVML(I);
        break;
      case Instruction::ICmp:
        MadeChange |= replaceFDTest(I);
        break;
      }
    }
  }

  // We're done with transforms, so remove dead instructions.
  if (MadeChange)
    for (BasicBlock &BB : F)
      SimplifyInstructionsInBlock(&BB);

  return MadeChange;
}

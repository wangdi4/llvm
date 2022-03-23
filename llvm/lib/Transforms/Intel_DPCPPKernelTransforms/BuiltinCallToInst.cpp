//===- BuiltinCallToInst.cpp - Resolve supported builtin calls --*- C++ -*-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinCallToInst.h"
#include "NameMangleAPI.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ImplicitArgsAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/VectorizerUtils.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-builtin-call-to-inst"

/// Shuffle and Shuffle2 arguments positions.
static const unsigned SHUFFLE_VEC1_POS = 0;
static const unsigned SHUFFLE_MASK_POS = 1;
static const unsigned SHUFFLE2_VEC1_POS = 0;
static const unsigned SHUFFLE2_VEC2_POS = 1;
static const unsigned SHUFFLE2_MASK_POS = 2;

namespace {
/// Legacy BuiltinCallToInst pass.
class BuiltinCallToInstLegacy : public FunctionPass {
public:
  static char ID;

  BuiltinCallToInstLegacy() : FunctionPass(ID) {
    initializeBuiltinCallToInstLegacyPass(*PassRegistry::getPassRegistry());
  }

  llvm::StringRef getPassName() const override {
    return "BuiltinCallToInstLegacy";
  }

  bool runOnFunction(Function &F) override { return Impl.runImpl(F); }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<ImplicitArgsAnalysisLegacy>();
  }

private:
  BuiltinCallToInstPass Impl;
};

} // namespace

char BuiltinCallToInstLegacy::ID = 0;

INITIALIZE_PASS(BuiltinCallToInstLegacy, DEBUG_TYPE,
                "Resolve supported builtin calls", false, false)

FunctionPass *llvm::createBuiltinCallToInstLegacyPass() {
  return new BuiltinCallToInstLegacy();
}

PreservedAnalyses BuiltinCallToInstPass::run(Function &F,
                                             FunctionAnalysisManager &AM) {

  PreservedAnalyses PA;
  PA.preserve<ImplicitArgsAnalysis>();
  return runImpl(F) ? PA : PreservedAnalyses::all();
}

bool BuiltinCallToInstPass::runImpl(Function &F) {
  findBuiltinCallsToHandle(F);
  return handleSupportedBuiltinCalls();
}

void BuiltinCallToInstPass::findBuiltinCallsToHandle(Function &F) {
  BuiltinCalls.clear();

  for (Instruction &I : instructions(&F)) {
    auto *CI = dyn_cast<CallInst>(&I);
    if (!CI)
      continue;

    // Check if called function is a supported built-in.
    BuiltinType BuiltinTy = isSupportedBuiltin(CI);
    if (BI_NOT_SUPPORTED != BuiltinTy) {
      // Add this instruction for handling.
      std::pair<CallInst *, BuiltinType> BuiltinCallPair =
          std::pair<CallInst *, BuiltinType>(CI, BuiltinTy);
      BuiltinCalls.push_back(BuiltinCallPair);
      LLVM_DEBUG(dbgs() << "Add builtin call to be handled: " << *CI << ", "
                        << BuiltinTy << "\n");
    }
  }
}

bool BuiltinCallToInstPass::handleSupportedBuiltinCalls() {
  // Check if there are shuffle calls.
  if (BuiltinCalls.empty())
    return false;

  // Run over all supported built-in calls in function.
  for (auto &It : BuiltinCalls) {
    CallInst *BuiltinCall = It.first;
    BuiltinType BuiltinTy = It.second;
    switch (BuiltinTy) {
    case BI_SHUFFLE1:
    case BI_SHUFFLE2:
      handleShuffleCalls(BuiltinCall, BuiltinTy);
      break;
    case BI_REL_IS_LESS:
    case BI_REL_IS_LESS_EQUAL:
    case BI_REL_IS_GREATER:
    case BI_REL_IS_GREATER_EQUAL:
    case BI_REL_IS_EQUAL:
    case BI_REL_IS_NOT_EQUAL:
      handleRelationalCalls(BuiltinCall, BuiltinTy);
      break;
    default:
      llvm_unreachable("Need to handle new supported built-in");
    }
  }
  return true;
}

/// In OpenCL:
///   - gentypen shuffle (gentypem x, ugentypen mask)
///   - gentypen shuffle2 (gentypem x, gentypem y, ugentypen mask)
/// Clang translates shuffle and shuffle2 functions calls to LLVM function calls
/// with specific types.
/// Example: %call = call <8 x i32> @_Z7shuffleDv4_iDv8_j( ... )
/// In case the mask argument is a vector of constants, BuiltinCallToInst
/// translates the shuffle call to shufflevector instruction in LLVM, to gain
/// some performance boost.
/// Example:
/// From this:
///   %tmp = call <8 x i32> @_Z7shuffleDv4_iDv4_j(<4 x i32> %x, <4 x i32>
///     <i32 3, i32 2, i32 1, i32 0>)
/// To this:
///   %tmp = shufflevector <4 x i32> %x, <4 x i32> undef, <4 x i32>
///     <i32 3, i32 2, i32 1, i32 0>
void BuiltinCallToInstPass::handleShuffleCalls(CallInst *ShuffleCall,
                                               BuiltinType ShuffleTy) {
  // If the function returns by pointer, shift all arguments by one.
  // Note that in the mangled name, the arguments are not shifted.
  const unsigned ArgStart = (ShuffleCall->getType()->isVoidTy()) ? 1 : 0;
  Value *RetVal = ShuffleCall;
  if (ShuffleCall->getType()->isVoidTy()) {
    Value *RetPtr = ShuffleCall->getArgOperand(0);
    // If it's not a pointer, don't try to handle it.
    if (!isa<PointerType>(RetPtr->getType()))
      return;

    Type *DesiredTy = (cast<PointerType>(RetPtr->getType()))->getElementType();

    RetVal = VectorizerUtils::rootReturnValue(RetPtr, DesiredTy, ShuffleCall);
    if (!RetVal)
      return;
  }

  // Get function operands, depending on shuffle type.
  Value *FirstVec = nullptr;
  Value *SecondVec = nullptr;
  Value *Mask = nullptr;
  switch (ShuffleTy) {
  case BI_SHUFFLE2:
    // First vector operand.
    FirstVec = VectorizerUtils::rootInputArgumentBySignature(
        ShuffleCall->getArgOperand(ArgStart + SHUFFLE2_VEC1_POS),
        SHUFFLE2_VEC1_POS, ShuffleCall);
    // Second vector operand.
    SecondVec = VectorizerUtils::rootInputArgumentBySignature(
        ShuffleCall->getArgOperand(ArgStart + SHUFFLE2_VEC2_POS),
        SHUFFLE2_VEC2_POS, ShuffleCall);
    // Mask operand.
    // Sometimes rooting the mask results in taking an int argument and creating
    // a bitcast, which is useless since ShuffleVector requires an explicit
    // vector constant.
    // TODO: If the mask is not a vector, do the conversion manually. VERY
    // manually. (get the actual constant as an APInt, and create the
    // appropriate vector by hand).
    Mask = VectorizerUtils::rootInputArgumentBySignature(
        ShuffleCall->getArgOperand(ArgStart + SHUFFLE2_MASK_POS),
        SHUFFLE2_MASK_POS, ShuffleCall);
    break;

  case BI_SHUFFLE1:
    // First vector operand.
    FirstVec = VectorizerUtils::rootInputArgumentBySignature(
        ShuffleCall->getArgOperand(ArgStart + SHUFFLE_VEC1_POS),
        SHUFFLE_VEC1_POS, ShuffleCall);
    if (FirstVec) {
      // Second vector operand: undef vector with type of FirstVec.
      SecondVec = UndefValue::get(FirstVec->getType());
    }
    // Mask operand.
    // Don't root, same as above.
    Mask = VectorizerUtils::rootInputArgumentBySignature(
        ShuffleCall->getArgOperand(ArgStart + SHUFFLE_MASK_POS),
        SHUFFLE_MASK_POS, ShuffleCall);
    break;

  default:
    llvm_unreachable("Shuffle function of unknown type");
  }

  if (!FirstVec || !SecondVec || !Mask || !isa<Constant>(Mask)) {
    // Failed to generate valid params to shuffle, do not optimize this shufle
    // call! Or mask is not of type const.
    return;
  }

  assert(isa<VectorType>(Mask->getType()) && "mask is not vector type");

  // Convert mask type to vector of i32, shuffflevector mask scalar size is
  // always 32.
  Constant *NewMask = nullptr;
  unsigned MaskVecSize =
      cast<FixedVectorType>(Mask->getType())->getNumElements();
  Type *MaskTy = FixedVectorType::get(
      Type::getInt32Ty(ShuffleCall->getContext()), MaskVecSize);

  // We previously searched for shuffle calls with constant mask only.
  // So we can assume mask is constant here.
  // If mask scalar size is not 32 then Zext or Trunc the mask to get to 32.
  if (Mask->getType()->getScalarSizeInBits() < MaskTy->getScalarSizeInBits())
    NewMask = ConstantExpr::getZExt(cast<Constant>(Mask), MaskTy);
  else if (Mask->getType()->getScalarSizeInBits() >
           MaskTy->getScalarSizeInBits())
    NewMask = ConstantExpr::getTrunc(cast<Constant>(Mask), MaskTy);
  else
    NewMask = cast<Constant>(Mask);

  // Create the new shufflevector instruction.
  // Unfortunately, due to the rooting issue discussed above, NewMask may be a
  // ConstantExpr, which is not supported by shuffles. Make a more generic check
  // here for safety, but assert...
  if (!ShuffleVectorInst::isValidOperands(FirstVec, SecondVec, NewMask)) {
    assert(isa<ConstantExpr>(NewMask) &&
           "Got invalid shuffle paramters not due to mask being a constant "
           "expressions");
    return;
  }

  Instruction *NewShuffleInst = new ShuffleVectorInst(
      FirstVec, SecondVec, NewMask, "newShuffle", ShuffleCall);
  NewShuffleInst->setDebugLoc(ShuffleCall->getDebugLoc());

  LLVM_DEBUG(dbgs() << "Replace shufflevetor instruction: " << *ShuffleCall
                    << "\n  with\n"
                    << *NewShuffleInst << "\n");
  // Due to an optimization in clang, the return type of the original call may
  // be a longer vector than what the shuffle produces.
  if (NewShuffleInst->getType() != RetVal->getType())
    NewShuffleInst = VectorizerUtils::extendValToType(
        NewShuffleInst, RetVal->getType(), ShuffleCall);

  RetVal->replaceAllUsesWith(NewShuffleInst);
  // Remove origin shuffle call.
  ShuffleCall->eraseFromParent();
}

void BuiltinCallToInstPass::handleRelationalCalls(CallInst *RelationalCall,
                                                  BuiltinType RelationalTy) {
  Value *Op1 = RelationalCall->getOperand(0);
  Value *Op2 = RelationalCall->getOperand(1);
  assert((Op1->getType() == Op2->getType()) &&
         Op1->getType()->isFPOrFPVectorTy() &&
         "Relational built-ins assumed to take primitive types");

  CmpInst::Predicate CmpOpcode;
  switch (RelationalTy) {
  case BI_REL_IS_LESS:
    CmpOpcode = CmpInst::FCMP_OLT;
    break;
  case BI_REL_IS_LESS_EQUAL:
    CmpOpcode = CmpInst::FCMP_OLE;
    break;
  case BI_REL_IS_GREATER:
    CmpOpcode = CmpInst::FCMP_OGT;
    break;
  case BI_REL_IS_GREATER_EQUAL:
    CmpOpcode = CmpInst::FCMP_OGE;
    break;
  case BI_REL_IS_EQUAL:
    CmpOpcode = CmpInst::FCMP_OEQ;
    break;
  case BI_REL_IS_NOT_EQUAL:
    CmpOpcode = CmpInst::FCMP_UNE;
    break;
  default:
    llvm_unreachable("Unhandled relational built-in type");
  }

  IRBuilder<> Builder(RelationalCall);
  LLVM_DEBUG(dbgs() << "Replace relational call: " << *RelationalCall
                    << "\n  with:\n");
  Value *NewRelationalInst = Builder.CreateFCmp(CmpOpcode, Op1, Op2);
  LLVM_DEBUG(dbgs() << "  " << *NewRelationalInst << "\n");
  Type *RetTy = RelationalCall->getType();
  NewRelationalInst = RetTy->isVectorTy()
                          ? Builder.CreateSExt(NewRelationalInst, RetTy)
                          : Builder.CreateZExt(NewRelationalInst, RetTy);
  LLVM_DEBUG(dbgs() << "  " << *NewRelationalInst << "\n");

  RelationalCall->replaceAllUsesWith(NewRelationalInst);
  // Remove origin relational call.
  RelationalCall->eraseFromParent();
}

BuiltinCallToInstPass::BuiltinType
BuiltinCallToInstPass::isSupportedBuiltin(CallInst *CI) {
  Value *CalledOp = CI->getCalledOperand();
  if (!CalledOp)
    return BI_NOT_SUPPORTED;
  Function *CalledFunc = dyn_cast<Function>(CalledOp->stripPointerCasts());
  // Indirect function call is not supported now.
  if (!CalledFunc)
    return BI_NOT_SUPPORTED;

  StringRef CalledFuncName = CalledFunc->getName();
  if (!NameMangleAPI::isMangledName(CalledFuncName))
    return BI_NOT_SUPPORTED;
  StringRef StrippedName = NameMangleAPI::stripName(CalledFuncName);

  // Check if it is a supported built-in function.
  return StringSwitch<BuiltinType>(StrippedName)
      .Case("shuffle", BI_SHUFFLE1)
      .Case("__ocl_helper_shuffle", BI_SHUFFLE1)
      .Case("shuffle2", BI_SHUFFLE2)
      .Case("__ocl_helper_shuffle2", BI_SHUFFLE2)
      .Case("isless", BI_REL_IS_LESS)
      .Case("islessequal", BI_REL_IS_LESS_EQUAL)
      .Case("isgreater", BI_REL_IS_GREATER)
      .Case("isgreaterequal", BI_REL_IS_GREATER_EQUAL)
      .Case("isequal", BI_REL_IS_EQUAL)
      .Case("isnotequal", BI_REL_IS_NOT_EQUAL)
      .Default(BI_NOT_SUPPORTED);
}

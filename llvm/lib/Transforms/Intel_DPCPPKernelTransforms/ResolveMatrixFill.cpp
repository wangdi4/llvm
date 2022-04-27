//==---- ResolveMatrixFill.cpp - Resolve matrix fill intrinsics -- C++ -*---==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ResolveMatrixFill.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicsX86.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/PassRegistry.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-resolve-matrix-fill"

namespace {

/// Legacy ResolveMatrixFill pass.
class ResolveMatrixFillLegacy : public ModulePass {
  ResolveMatrixFillPass Impl;

public:
  static char ID;

  ResolveMatrixFillLegacy() : ModulePass(ID) {
    initializeResolveMatrixFillLegacyPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return "ResolveMatrixFillLegacy"; }

  bool runOnModule(Module &M) override { return Impl.runImpl(M); }
};

} // namespace

char ResolveMatrixFillLegacy::ID = 0;
INITIALIZE_PASS(ResolveMatrixFillLegacy, DEBUG_TYPE,
                "Resolve matrix fill intrinsic", false, false)

ModulePass *llvm::createResolveMatrixFillLegacyPass() {
  return new ResolveMatrixFillLegacy();
}

static Value *createFillZeroCall(unsigned Rows, unsigned Cols,
                                 FixedVectorType *MatrixType,
                                 MetadataAsValue *Layout,
                                 MetadataAsValue *Scope,
                                 Instruction *InsertPoint) {
  IRBuilder<> B(InsertPoint);
  assert(MatrixType->getNumElements() == (Rows * Cols));
  Type *DataType = MatrixType->getElementType();
  Value *Zero = nullptr;
  assert(DPCPPKernelCompilationUtils::isValidMatrixType(MatrixType) &&
         "Unsupported matrix type");
  if (DataType->isIntegerTy())
    Zero = ConstantInt::get(DataType, 0);
  else
    Zero = ConstantFP::get(DataType, 0);
  Value *ZeroMat = B.CreateIntrinsic(
      Intrinsic::experimental_matrix_fill, {MatrixType, DataType},
      {Zero, B.getInt32(Rows), B.getInt32(Cols), Layout, Scope}, nullptr,
      "mat.init");
  return ZeroMat;
}

static Value *createFillSliceLoop(Value *InitMatrix, unsigned Rows,
                                  unsigned Cols, Value *Data,
                                  MetadataAsValue *Layout,
                                  MetadataAsValue *Scope,
                                  Instruction *InsertPoint) {
  IRBuilder<> B(InsertPoint);
  BasicBlock *OriginalBB = InsertPoint->getParent();
  auto *MatrixType = InitMatrix->getType();
  auto *RowVal = B.getInt32(Rows);
  auto *ColVal = B.getInt32(Cols);
  Value *SliceLength = B.CreateIntrinsic(
      Intrinsic::experimental_matrix_wi_slice_length, {MatrixType},
      {InitMatrix, RowVal, ColVal, Layout, Scope}, nullptr, "slice.length");
  BasicBlock *LoopHeader =
      OriginalBB->splitBasicBlock(InsertPoint, "matrix.fill.slice.loop.header");
  BasicBlock *LoopBody =
      LoopHeader->splitBasicBlock(InsertPoint, "matrix.fill.slice.loop");
  BasicBlock *LoopEnd =
      LoopBody->splitBasicBlock(InsertPoint, "matrix.fill.slice.loop.end");

  // Build header
  B.SetInsertPoint(LoopHeader->getTerminator());
  auto *I = B.CreatePHI(SliceLength->getType(), 2, "element.index");
  auto *Matrix = B.CreatePHI(MatrixType, 2, "mat");
  I->addIncoming(ConstantInt::get(SliceLength->getType(), 0), OriginalBB);
  Matrix->addIncoming(InitMatrix, OriginalBB);
  auto *Condition = B.CreateICmpSLT(I, SliceLength);
  B.CreateCondBr(Condition, LoopBody, LoopEnd);
  LoopHeader->getTerminator()->eraseFromParent();

  // Build loop body
  B.SetInsertPoint(LoopBody->getTerminator());
  auto *UpdatedMatrix = B.CreateIntrinsic(
      Intrinsic::experimental_matrix_wi_slice_insertelement,
      {MatrixType, I->getType()},
      {Matrix, RowVal, ColVal, Data, I, Layout, Scope}, nullptr, "mat.update");
  auto *Inc = B.CreateAdd(I, ConstantInt::get(I->getType(), 1),
                          "element.index.inc", true);
  I->addIncoming(Inc, LoopBody);
  Matrix->addIncoming(UpdatedMatrix, LoopBody);
  LoopBody->getTerminator()->setOperand(0, LoopHeader);

  return Matrix;
}

static std::pair<bool, Value *> resolveMatrixFillCall(CallInst *CI) {
  bool Changed = false;
  Value *Data = CI->getArgOperand(0);
  // If the data arg is a constant zero, we don't need to resolve it --
  // CodeGen will lower it to tilezero intrinsics later.
  Constant *C;
  if ((C = dyn_cast<Constant>(Data)) && C->isZeroValue())
    return std::make_pair(Changed, CI);

  // Handle the first arg, if it's a pointer.
  if (Data->getType()->isPointerTy()) {
    auto *LI = new LoadInst(Data->getType()->getPointerElementType(), Data,
                            "loaded.fill.data", CI);
    LI->setDebugLoc(CI->getDebugLoc());
    Data = LI;
  }

  // Extract row, col size of the matrix from the second, third args.
  unsigned Rows = cast<ConstantInt>(CI->getArgOperand(1))->getZExtValue();
  unsigned Cols = cast<ConstantInt>(CI->getArgOperand(2))->getZExtValue();
  // Extract layout metadata from the fourth arg.
  auto *Layout = cast<MetadataAsValue>(CI->getArgOperand(3));
  // Extract scope metadata from the fifth arg.
  auto *Scope = cast<MetadataAsValue>(CI->getArgOperand(4));
  assert(cast<MDString>(Scope->getMetadata())
             ->getString()
             .equals("scope.subgroup") &&
         "Only supports subgroup scope for now");

  Changed = true;
  // Initialize the matrix with zeros.
  auto *Matrix = createFillZeroCall(
      Rows, Cols, cast<FixedVectorType>(CI->getType()), Layout, Scope, CI);
  // Fill the matrix with a loop of @llvm.experimental.wi.slice.insertelement
  Matrix = createFillSliceLoop(Matrix, Rows, Cols, Data, Layout, Scope, CI);
  return std::make_pair(Changed, Matrix);
}

bool ResolveMatrixFillPass::runImpl(Module &M) {
  bool Changed = false;
  for (auto &F : M) {
    if (F.getIntrinsicID() != Intrinsic::experimental_matrix_fill)
      continue;
    SmallVector<User *> WorkList(F.users());
    for (auto *U : WorkList) {
      CallInst *CI = cast<CallInst>(U);
      bool Resolved;
      Value *Replacement;
      std::tie(Resolved, Replacement) = resolveMatrixFillCall(CI);
      if (Resolved) {
        CI->replaceAllUsesWith(Replacement);
        CI->eraseFromParent();
        Changed = true;
      }
    }
  }
  return Changed;
}

PreservedAnalyses ResolveMatrixFillPass::run(Module &M,
                                             ModuleAnalysisManager &AM) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

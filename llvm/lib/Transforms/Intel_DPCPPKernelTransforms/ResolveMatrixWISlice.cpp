//==------------------- ResolveMatrixWISlice.cpp -  C++ -*------------------==//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ResolveMatrixWISlice.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-resolve-matrix-wi-slice"

static Value *resolveSliceLengthCall(CallInst *CI) {
  // Extract row, col size of the matrix from the second, third args.
  unsigned Rows = cast<ConstantInt>(CI->getArgOperand(1))->getZExtValue();
  unsigned Cols = cast<ConstantInt>(CI->getArgOperand(2))->getZExtValue();
  // Create a @get_sub_group_slice_length.(i32 %total.element.count) call.
  return CompilationUtils::createGetSubGroupSliceLengthCall(Rows * Cols, CI,
                                                            "sg.slice.length");
}

// Create @get_sub_group_rowslice_id call for either
// - call i32 @llvm.experimental.matrix.wi.slice.extractelement.v144i32.i64(<144
//   x i32> %mat, i32 12, i32 12, i64 %element.index, metadata
//   !"matrix.rowmajor", metadata !"scope.subgroup", metadata
//   !"matrix.use.unnecessary")
// - call <144 x i32>
//   @llvm.experimental.matrix.wi.slice.insertelement.v144i32.i64(<144 x i32>
//   %mat, i32 12, i32 12, i32 %val, i64 %element.index, metadata
//   !"matrix.rowmajor", metadata !"scope.subgroup", metadata
//   !"matrix.use.unnecessary")
static Value *createGetSubGroupRowSliceIdFromExtractOrInsert(CallInst *CI) {
  // Matrix is always the first arg.
  auto *Matrix = CI->getArgOperand(0);
  assert(CompilationUtils::isValidMatrixType(
             cast<FixedVectorType>(Matrix->getType())) &&
         "Unsupported matrix type");
  // Matrix size are always the second, third args.
  unsigned Rows = cast<ConstantInt>(CI->getArgOperand(1))->getZExtValue();
  unsigned Cols = cast<ConstantInt>(CI->getArgOperand(2))->getZExtValue();
  // Element index is always the third last arg.
  auto *Index = CI->getArgOperand(CI->arg_size() - 4);
  return CompilationUtils::createGetSubGroupRowSliceIdCall(
      Matrix, Rows, Cols, Index, CI, "rowslice.id");
}

static Value *resolveSliceExtractElement(CallInst *CI) {
  auto *RowSliceId = createGetSubGroupRowSliceIdFromExtractOrInsert(CI);
  // Element data type of the matrix.
  auto *DataType = CI->getArgOperand(0)->getType()->getScalarType();
  return CompilationUtils::createSubGroupRowSliceExtractElementCall(
      RowSliceId, DataType, CI, "extract.elem");
}

static Value *resolveSliceInsertElement(CallInst *CI) {
  auto *RowSliceId = createGetSubGroupRowSliceIdFromExtractOrInsert(CI);
  // %val is the fourth arg.
  auto *Val = CI->getArgOperand(3);
  (void)CompilationUtils::createSubGroupRowSliceInsertElementCall(RowSliceId,
                                                                  Val, CI);
  auto *MatrixType = CI->getArgOperand(0)->getType();
  return CompilationUtils::createSubGroupInsertRowSliceToMatrixCall(
      RowSliceId, MatrixType, CI, "mat.update");
}

bool ResolveMatrixWISlicePass::runImpl(Module &M) {
  bool Changed = false;
  function_ref<Value *(CallInst *)> GetReplacementOf;
  for (auto &F : M) {
    switch (F.getIntrinsicID()) {
    case Intrinsic::experimental_matrix_wi_slice_length:
      GetReplacementOf = resolveSliceLengthCall;
      break;
    case Intrinsic::experimental_matrix_wi_slice_extractelement:
      GetReplacementOf = resolveSliceExtractElement;
      break;
    case Intrinsic::experimental_matrix_wi_slice_insertelement:
      GetReplacementOf = resolveSliceInsertElement;
      break;
    default:
      continue;
    }
    SmallVector<User *> WorkList(F.users());
    for (auto *U : WorkList) {
      CallInst *CI = cast<CallInst>(U);
      CI->replaceAllUsesWith(GetReplacementOf(CI));
      CI->eraseFromParent();
      Changed = true;
    }
  }
  return Changed;
}

PreservedAnalyses ResolveMatrixWISlicePass::run(Module &M,
                                                ModuleAnalysisManager &AM) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

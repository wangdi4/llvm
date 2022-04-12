//==------------------- ResolveMatrixLayout.cpp -  C++ -*------------------==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ResolveMatrixLayout.h"
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
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-resolve-matrix-layout"

namespace {

/// Legacy ResolveMatrixLayout pass.
class ResolveMatrixLayoutLegacy : public ModulePass {
  ResolveMatrixLayoutPass Impl;

public:
  static char ID;

  ResolveMatrixLayoutLegacy() : ModulePass(ID) {
    initializeResolveMatrixLayoutLegacyPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return "ResolveMatrixLayoutLegacy"; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
  }

  bool runOnModule(Module &M) override { return Impl.runImpl(M); }
};
} // namespace

char ResolveMatrixLayoutLegacy::ID = 0;
INITIALIZE_PASS(ResolveMatrixLayoutLegacy, DEBUG_TYPE,
                "Resolve layout for matrix intrinsic", false, false)

ModulePass *llvm::createResolveMatrixLayoutLegacyPass() {
  return new ResolveMatrixLayoutLegacy();
}

static CallInst *generateCall(Module *M, StringRef FnName, Type *ReturnType,
                              ArrayRef<Value *> Args, IRBuilder<> &Builder,
                              const Twine &Name = "",
                              AttributeList AttrList = AttributeList()) {
  SmallVector<Type *> ArgTypes;
  for (auto *Arg : Args)
    ArgTypes.push_back(Arg->getType());
  auto *FuncType = FunctionType::get(ReturnType, ArgTypes, false);
  auto Func = M->getOrInsertFunction(FnName, FuncType, AttrList);
  return Builder.CreateCall(Func, Args, Name);
}

static AllocaInst *createMatrixAllocaInst(Function &F, FixedVectorType *FVT) {
  Module *M = F.getParent();
  const DataLayout &DL = M->getDataLayout();

  auto AllocaAlignment = DL.getPrefTypeAlign(FVT);
  unsigned AllocaAS = DL.getAllocaAddrSpace();
  AllocaInst *AllocaRes =
      new AllocaInst(FVT, AllocaAS, "", &F.getEntryBlock().front());
  AllocaRes->setAlignment(AllocaAlignment);
  return AllocaRes;
}

static std::pair<bool, Value *> resolveMatrixLayoutLoadHelper(
    IRBuilder<> &Builder, CallInst *CI, StringRef BuiltinName,
    PointerType *PtrType, Value *NewMemL, Value *OldStride, Value *NewStride,
    bool ShouldAddWorkList, SmallVector<User *> &WorkList) {
  FixedVectorType *MatrixType = cast<FixedVectorType>(CI->getType());
  Value *MatAlloca = createMatrixAllocaInst(*CI->getFunction(), MatrixType);
  Value *Src =
      Builder.CreatePointerBitCastOrAddrSpaceCast(CI->getOperand(0), PtrType);
  Value *Dst = Builder.CreateBitCast(MatAlloca, PtrType);
  SmallVector<Value *, 5> Args = {Src, Dst, CI->getOperand(3),
                                  CI->getOperand(4), OldStride};
  generateCall(CI->getModule(), BuiltinName, Builder.getVoidTy(), Args,
               Builder);
  // stride should be calculated according to the size of the temporary matrix
  SmallVector<Value *, 8> ArgsForNewLoad = {Dst,
                                            NewStride,
                                            CI->getOperand(2),
                                            CI->getOperand(3),
                                            CI->getOperand(4),
                                            CI->getOperand(5),
                                            NewMemL,
                                            CI->getOperand(7)};
  SmallVector<Type *, 2> TypesForNewLoad = {MatrixType, Dst->getType()};
  CallInst *NewMatrixLoad = Builder.CreateIntrinsic(
      Intrinsic::experimental_matrix_load, TypesForNewLoad, ArgsForNewLoad);
  if (ShouldAddWorkList)
    WorkList.push_back(NewMatrixLoad);
  return std::make_pair(true, NewMatrixLoad);
}

static std::pair<bool, Value *>
resolveMatrixLayoutLoad(CallInst *CI, SmallVector<User *> &WorkList) {
  int64_t MRows = cast<ConstantInt>(CI->getOperand(3))->getSExtValue();
  int64_t MCols = cast<ConstantInt>(CI->getOperand(4))->getSExtValue();
  FixedVectorType *MatrixType = cast<FixedVectorType>(CI->getType());
  Metadata *MatL = cast<MetadataAsValue>(CI->getOperand(5))->getMetadata();
  Metadata *MemL = cast<MetadataAsValue>(CI->getOperand(6))->getMetadata();
  if (cast<MDString>(MatL)->getString() == cast<MDString>(MemL)->getString())
    return std::make_pair(false, CI);
  IRBuilder<> Builder(CI);
  LLVMContext &Ctx = Builder.getContext();
  if (cast<MDString>(MatL)->getString().equals("matrix.packed.b") &&
      cast<MDString>(MemL)->getString().equals("matrix.rowmajor") &&
      MatrixType->getElementType()->isIntegerTy(8)) {
    // Transform
    // %res = call <8 x i8> @llvm.experimental.matrix.load.v8i8.p4i8(
    //   i32* addressspace(4) %ptr, i64 stride, i1 false, i32 4, i32 2,
    //   metadata !"matrix.packed_b", metadata !"matrix.rowmajor", metadata
    //   !"scope.subgroup")
    // =>
    // %alloc = alloca [i8 x 8]
    // %ptr2 = bitcast %alloc to i8*
    // matrix_layout_transform_rowmajor_to_vnni(ptr, %ptr2, rows, cols, stride)
    // %res = call <8 x i8> @llvm.experimental.matrix.load.v8i8.p4i8(
    //   i8% ptr2, i64 stride, i1 false, i32 4, i32 2,
    //   metadata !"matrix.packed_b", metadata !"matrix.packed_b", metadata
    //   !"scope.subgroup")
    return resolveMatrixLayoutLoadHelper(
        Builder, CI,
        "_Z40matrix_layout_transform_rowmajor_to_vnniPU3AS4cS0_iii",
        Builder.getInt8PtrTy(), CI->getOperand(5), CI->getOperand(1),
        Builder.getInt64(MCols * 4), false, WorkList);
  } else if (cast<MDString>(MatL)->getString().equals("matrix.packed.b") &&
             cast<MDString>(MemL)->getString().equals("matrix.rowmajor") &&
             MatrixType->getElementType()->isIntegerTy(16)) {
    return resolveMatrixLayoutLoadHelper(
        Builder, CI,
        "_Z40matrix_layout_transform_rowmajor_to_vnniPU3AS4sS0_iii",
        Type::getInt16PtrTy(Ctx), CI->getOperand(5),
        Builder.CreateMul(CI->getOperand(1), Builder.getInt64(2)),
        Builder.getInt64(MCols * 2), false, WorkList);
  } else if (cast<MDString>(MatL)->getString().equals("matrix.rowmajor") &&
             cast<MDString>(MemL)->getString().equals("matrix.colmajor") &&
             MatrixType->getElementType()->isIntegerTy(8)) {
    // Transform
    // %res = call <8 x i8> @llvm.experimental.matrix.load.v8i8.p4i8(
    //   i32* addressspace(4) %ptr, i64 stride, i1 false, i32 4, i32 2,
    //   metadata !"matrix.rowmajor", metadata !"matrix.colmajor", metadata
    //   !"scope.subgroup")
    // =>
    // %alloc = alloca [i8 x 8]
    // %ptr2 = bitcast %alloc to i8*
    // matrix_layout_transform_colmajor_to_rowmajor(ptr, %ptr2, rows, cols,
    // stride) %res = call <8 x i8> @llvm.experimental.matrix.load.v8i8.p4i8(
    //   i8% ptr2, i64 stride, i1 false, i32 4, i32 2,
    //   metadata !"matrix.rowmajor", metadata !"matrix.rowmajor", metadata
    //   !"scope.subgroup")
    return resolveMatrixLayoutLoadHelper(
        Builder, CI,
        "_Z44matrix_layout_transform_colmajor_to_rowmajorPU3AS4cS0_iii",
        Builder.getInt8PtrTy(), CI->getOperand(5), CI->getOperand(1),
        Builder.getInt64(MRows), false, WorkList);
  } else if (cast<MDString>(MatL)->getString().equals("matrix.rowmajor") &&
             cast<MDString>(MemL)->getString().equals("matrix.colmajor") &&
             MatrixType->getElementType()->isIntegerTy(16)) {
    return resolveMatrixLayoutLoadHelper(
        Builder, CI,
        "_Z44matrix_layout_transform_colmajor_to_rowmajorPU3AS4sS0_iii",
        Type::getInt16PtrTy(Ctx), CI->getOperand(5),
        Builder.CreateMul(CI->getOperand(1), Builder.getInt64(2)),
        Builder.getInt64(MRows), false, WorkList);
  } else if (cast<MDString>(MatL)->getString().equals("matrix.packed.b") &&
             cast<MDString>(MemL)->getString().equals("matrix.colmajor") &&
             MatrixType->getElementType()->isIntegerTy(8)) {
    return resolveMatrixLayoutLoadHelper(
        Builder, CI,
        "_Z44matrix_layout_transform_colmajor_to_rowmajorPU3AS4cS0_iii",
        Builder.getInt8PtrTy(),
        MetadataAsValue::get(Ctx, MDString::get(Ctx, "matrix.rowmajor")),
        CI->getOperand(1), Builder.getInt64(MRows), true, WorkList);
  } else if (cast<MDString>(MatL)->getString().equals("matrix.packed.b") &&
             cast<MDString>(MemL)->getString().equals("matrix.colmajor") &&
             MatrixType->getElementType()->isIntegerTy(16)) {
    return resolveMatrixLayoutLoadHelper(
        Builder, CI,
        "_Z44matrix_layout_transform_colmajor_to_rowmajorPU3AS4sS0_iii",
        Type::getInt16PtrTy(Ctx),
        MetadataAsValue::get(Ctx, MDString::get(Ctx, "matrix.rowmajor")),
        Builder.CreateMul(CI->getOperand(1), Builder.getInt64(2)),
        Builder.getInt64(MRows), true, WorkList);
  } else {
    assert(false && "invalid matrix layout or memory layout!");
  }

  return std::make_pair(false, CI);
}

bool ResolveMatrixLayoutPass::runImpl(Module &M) {
  bool Changed = false;
  for (auto &F : M) {
    if (F.getIntrinsicID() != Intrinsic::experimental_matrix_load)
      continue;
    SmallVector<User *> WorkList(F.users());
    for (auto *U : WorkList) {
      CallInst *CI = cast<CallInst>(U);
      bool Resolved;
      Value *Replacement;
      std::tie(Resolved, Replacement) = resolveMatrixLayoutLoad(CI, WorkList);
      if (Resolved) {
        CI->replaceAllUsesWith(Replacement);
        CI->eraseFromParent();
        Changed = true;
      }
    }
  }
  return Changed;
}

PreservedAnalyses ResolveMatrixLayoutPass::run(Module &M,
                                               ModuleAnalysisManager &AM) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

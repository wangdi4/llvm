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

  StringRef getPassName() const override {
    return "ResolveMatrixLayoutLegacy";
  }

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

static std::pair<bool, Value *> resolveMatrixLayoutLoad(CallInst *CI) {
  // int64_t MRows = cast<ConstantInt>(CI->getOperand(3))->getSExtValue();
  int64_t MCols = cast<ConstantInt>(CI->getOperand(4))->getSExtValue();
  FixedVectorType *MatrixType = cast<FixedVectorType>(CI->getType());
  Metadata *MatL = cast<MetadataAsValue>(CI->getOperand(5))->getMetadata();
  Metadata *MemL = cast<MetadataAsValue>(CI->getOperand(6))->getMetadata();
  if (cast<MDString>(MatL)->getString() == cast<MDString>(MemL)->getString())
    return std::make_pair(false, CI);
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
    IRBuilder<> Builder(CI);
    Value *MatAlloca = createMatrixAllocaInst(*CI->getFunction(), MatrixType);
    Value *Src = CI->getOperand(0)->getType()->getPointerAddressSpace() == 0
                     ? Builder.CreateBitCast(
                           CI->getOperand(0),
                           llvm::Type::getInt8PtrTy(Builder.getContext()))
                     : Builder.CreateAddrSpaceCast(
                           CI->getOperand(0),
                           llvm::Type::getInt8PtrTy(Builder.getContext()));
    Value *Dst = Builder.CreateBitCast(
        MatAlloca, Type::getInt8PtrTy(Builder.getContext()));
    SmallVector<Value *> Args = {Src, Dst, CI->getOperand(3), CI->getOperand(4),
                                 CI->getOperand(1)};
    generateCall(CI->getModule(), "_Z40matrix_layout_transform_rowmajor_to_vnniPU3AS4cS0_iii",
                 Type::getVoidTy(Builder.getContext()), Args, Builder);
    // stride should be the calculated according to the temp matrix's size, aka,
    Value *NewStride = Builder.getInt64(MCols * 4);
    SmallVector<Value *> ArgsForNewLoad = {Dst,
                                           NewStride,
                                           CI->getOperand(2),
                                           CI->getOperand(3),
                                           CI->getOperand(4),
                                           CI->getOperand(5),
                                           CI->getOperand(5),
                                           CI->getOperand(7)};
    SmallVector<Type *> TypesForNewLoad = {MatrixType, Dst->getType()};
    return std::make_pair(
        true, Builder.CreateIntrinsic(Intrinsic::experimental_matrix_load,
                                      TypesForNewLoad, ArgsForNewLoad));
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
      std::tie(Resolved, Replacement) = resolveMatrixLayoutLoad(CI);
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

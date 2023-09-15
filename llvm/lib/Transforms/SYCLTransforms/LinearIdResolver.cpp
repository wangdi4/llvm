//===- LinearIdResolver.cpp - DPC++ linear id resolver --------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/LinearIdResolver.h"

#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-linear-id-resolver"

PreservedAnalyses LinearIdResolverPass::run(Module &M,
                                            ModuleAnalysisManager &AM) {
  CallGraph *CG = &AM.getResult<CallGraphAnalysis>(M);
  return runImpl(M, CG) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool LinearIdResolverPass::runImpl(Module &M, CallGraph *CG) {
  bool Changed = false;

  auto &Ctx = M.getContext();
  auto Int32Ty = Type::getInt32Ty(Ctx);
  Zero = ConstantInt::get(Int32Ty, 0);
  One = ConstantInt::get(Int32Ty, 1);
  Two = ConstantInt::get(Int32Ty, 2);
  RetTy = IntegerType::get(Ctx, M.getDataLayout().getPointerSizeInBits(0));

  enum LinearIdKind { LK_GLOBAL, LK_LOCAL };

  for (Function &F : M) {
    if (F.isDeclaration())
      continue;

    SmallVector<std::pair<CallInst *, LinearIdKind>, 2> WorkList;
    // looking for get_{global,local}_linear_id()
    for (auto &N : *(*CG)[&F]) {
      auto *CI = cast<CallInst>(*N.first);
      Function *Callee = CI->getCalledFunction();
      if (!Callee)
        continue;

      StringRef CalleeName = Callee->getName();
      if (CompilationUtils::isGetGlobalLinearId(CalleeName))
        WorkList.push_back(std::make_pair(CI, LK_GLOBAL));
      else if (CompilationUtils::isGetLocalLinearId(CalleeName))
        WorkList.push_back(std::make_pair(CI, LK_LOCAL));
    }

    for (auto &It : WorkList) {
      CallInst *CI = It.first;
      if (It.second == LK_GLOBAL)
        replaceGetGlobalLinearId(&M, CI);
      else
        replaceGetLocalLinearId(&M, CI);
    }

    Changed |= !WorkList.empty();
  }
  return Changed;
}

void LinearIdResolverPass::replaceGetGlobalLinearId(Module *M, CallInst *CI) {
  // Replace get_global_linear_id() with the following sequence
  // (get_global_id(2) – get_global_offset(2)) * get_global_size(1) *
  // get_global_size(0) + (get_global_id(1) – get_global_offset(1)) *
  // get_global_size(0) + (get_global_id(0) – get_global_offset(0))
  //    ==
  // ((get_global_id(2) – get_global_offset(2))
  //  * get_global_size(1)
  //  + (get_global_id(1) – get_global_offset(1)))
  // * get_global_size(0)
  // + (get_global_id(0) – get_global_offset(0))
  IRBuilder<> Builder(CI);

  // call get_global_id(0), get_global_id(1), get_global_id(2)
  static std::string IdName = CompilationUtils::mangledGetGID();
  CallInst *GId2 = createWIFunctionCall(M, Builder, IdName, Two, "gid2");
  CallInst *GId1 = createWIFunctionCall(M, Builder, IdName, One, "gid1");
  CallInst *GId0 = createWIFunctionCall(M, Builder, IdName, Zero, "gid0");
  assert(GId0 && GId1 && GId2 && "Can't create get_global_id calls");

  // call get_global_offset(0), get_global_offset(1), get_global_offset(2)
  static std::string OffName = CompilationUtils::mangledGetGlobalOffset();
  CallInst *GOff2 = createWIFunctionCall(M, Builder, OffName, Two, "goff2");
  CallInst *GOff1 = createWIFunctionCall(M, Builder, OffName, One, "goff1");
  CallInst *GOff0 = createWIFunctionCall(M, Builder, OffName, Zero, "goff0");
  assert(GOff0 && GOff1 && GOff2 && "Can't create get_global_offset calls");

  // call get_global_size(0), get_global_size(1)
  static std::string SizeName = CompilationUtils::mangledGetGlobalSize();
  CallInst *GSize1 = createWIFunctionCall(M, Builder, SizeName, One, "gsz1");
  CallInst *GSize0 = createWIFunctionCall(M, Builder, SizeName, Zero, "gsz0");
  assert(GSize0 && GSize1 && "Can't create get_global_size calls");

  // ((get_global_id(2) – get_global_offset(2))
  Value *Op0 = Builder.CreateSub(GId2, GOff2, "lgid.op0");
  //  * get_global_size(1)
  Value *Op1 = Builder.CreateMul(Op0, GSize1, "lgid.op1");
  //  + (get_global_id(1) – get_global_offset(1)))
  Value *Op2 = Builder.CreateSub(GId1, GOff1, "lgid.op2");
  Value *Op3 = Builder.CreateAdd(Op1, Op2, "lgid.op3");
  // * get_global_size(0)
  Value *Op4 = Builder.CreateMul(Op3, GSize0, "lgid.op4");
  // + (get_global_id(0) – get_global_offset(0))
  Value *Op5 = Builder.CreateSub(GId0, GOff0, "lgid.op5");
  Value *Res = Builder.CreateAdd(Op4, Op5, "lgid.res");

  CI->replaceAllUsesWith(Res);
  CI->eraseFromParent();
}

void LinearIdResolverPass::replaceGetLocalLinearId(Module *M, CallInst *CI) {
  // Replace get_local_linear_id() with the following sequence.
  // get_local_id(2) * get_local_size(1) * get_local_size(0) +
  // get_local_id(1) * get_local_size(0) +
  // get_local_id(0)
  //    ==
  // (get_local_id(2) * get_local_size(1)
  //  + get_local_id(1))
  // * get_local_size(0)
  // + get_local_id(0)
  IRBuilder<> Builder(CI);

  // call get_local_id(2), get_local_id(1), get_local_id(0)
  static std::string IdName = CompilationUtils::mangledGetLID();
  CallInst *LId2 = createWIFunctionCall(M, Builder, IdName, Two, "lid2");
  CallInst *LId1 = createWIFunctionCall(M, Builder, IdName, One, "lid1");
  CallInst *LId0 = createWIFunctionCall(M, Builder, IdName, Zero, "lid0");
  assert(LId0 && LId1 && LId2 && "Can't create get_local_id calls");

  // call get_local_size(1), get_local_size(0)
  static std::string SizeName = CompilationUtils::mangledGetLocalSize();
  CallInst *LSize1 = createWIFunctionCall(M, Builder, SizeName, One, "lsz1");
  CallInst *LSize0 = createWIFunctionCall(M, Builder, SizeName, Zero, "lsz0");
  assert(LSize0 && LSize1 && "Can't create get_local_size calls");

  // (get_local_id(2) * get_local_size(1)
  Value *Op0 = Builder.CreateMul(LId2, LSize1, "llid.p0");
  //  + get_local_id(1))
  Value *Op1 = Builder.CreateAdd(Op0, LId1, "llid.p1");
  // * get_local_size(0)
  Value *Op2 = Builder.CreateMul(Op1, LSize0, "llid.p2");
  // + get_local_id(0)
  Value *Res = Builder.CreateAdd(Op2, LId0, "llid.res");

  CI->replaceAllUsesWith(Res);
  CI->eraseFromParent();
}

CallInst *LinearIdResolverPass::createWIFunctionCall(Module *M,
                                                     IRBuilderBase &Builder,
                                                     StringRef FuncName,
                                                     Value *Args,
                                                     StringRef NameStr) {
  Function *F = M->getFunction(FuncName);

  // If the function does not exist then we need to create it.
  if (!F) {
    std::vector<Type *> Params(1, Args->getType());
    auto *FTy = FunctionType::get(RetTy, Params, /*isVarArg*/ false);
    F = cast<Function>(M->getOrInsertFunction(FuncName, FTy).getCallee());
  }

  CallInst *NewInst = Builder.CreateCall(F, Args, NameStr);
  return NewInst;
}

//===-------- Intel_X86EmitMultiVersionResolver.cpp -----------------------===//
//
// Copyright (C) 2021-2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/Intel_X86EmitMultiVersionResolver.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Triple.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Intel_CPU_utils.h"
#include "llvm/Support/X86TargetParser.h"
#include "llvm/Transforms/Utils/Intel_IMLUtils.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

using namespace llvm;
using namespace llvm::X86;

Value *llvm::formResolverCondition(IRBuilderBase &Builder,
                                   const MultiVersionResolverOption &RO,
                                   bool UseLibIRC) {
  Value *Condition = nullptr;

  if (!RO.Conditions.Architecture.empty())
    Condition = X86::emitCpuIs(Builder, RO.Conditions.Architecture);

  if (!RO.Conditions.Features.empty()) {
    Value *FeatureCond = nullptr;
    if (UseLibIRC) {
      std::array<uint64_t, 2> Bitmaps =
          X86::getCpuFeatureBitmap(RO.Conditions.Features, /*OnlyAutoGen=*/true);
      FeatureCond = X86::mayIUseCpuFeatureHelper(
          Builder, {APSInt{APInt(64, Bitmaps[0]), true},
                    APSInt{APInt(64, Bitmaps[1]), true}});
    } else
      FeatureCond = X86::emitCpuSupports(Builder, RO.Conditions.Features);

    Condition =
        Condition ? Builder.CreateAnd(Condition, FeatureCond) : FeatureCond;
  }

  return Condition;
}

static void CreateMultiVersionResolverReturn(Function *Resolver,
                                             IRBuilderBase &Builder,
                                             Function *FuncToReturn,
                                             bool UseIFunc) {
  if (UseIFunc) {
    Builder.CreateRet(FuncToReturn);
    return;
  }

  Module *M = Resolver->getParent();
  std::string ResolverPtrName = Resolver->getName().str() + ".ptr";
  GlobalValue *ResolverPtr = M->getNamedValue(ResolverPtrName);
  Builder.CreateAlignedStore(FuncToReturn, ResolverPtr, MaybeAlign(8));

  SmallVector<Value *, 10> Args;
  for_each(Resolver->args(), [&](Argument &Arg) { Args.push_back(&Arg); });

  CallInst *Result = Builder.CreateCall(FuncToReturn, Args);
  Result->setTailCall();
  Result->setCallingConv(FuncToReturn->getCallingConv());

  if (Resolver->getReturnType()->isVoidTy())
    Builder.CreateRetVoid();
  else
    Builder.CreateRet(Result);
}

static void emitResolverPtrTest(Function *Resolver, IRBuilderBase &Builder) {

  Module *M = Resolver->getParent();
  std::string ResolverPtrName = Resolver->getName().str() + ".ptr";

  Type *ResolverPtrType = Resolver->getFunctionType()->getPointerTo();
  GlobalVariable *ResolverPtr =
      new GlobalVariable(*M, ResolverPtrType, false, GlobalValue::InternalLinkage,
                         Constant::getNullValue(ResolverPtrType), ResolverPtrName);
  ResolverPtr->setDSOLocal(true);

  auto ResolverPtrVal = Builder.CreateAlignedLoad(ResolverPtrType, ResolverPtr, MaybeAlign(8));

  Value *CmpResult = Builder.CreateICmpNE(ResolverPtrVal, Constant::getNullValue(ResolverPtrType),
                                          "ptr_compare");

  auto &Ctx = Resolver->getContext();
  BasicBlock *ThenBlock = BasicBlock::Create(Ctx, "resolver_then", Resolver);
  BasicBlock *ElseBlock = BasicBlock::Create(Ctx, "resolver_else", Resolver);

  Builder.CreateCondBr(CmpResult, ThenBlock, ElseBlock);

  Builder.SetInsertPoint(ThenBlock);

  SmallVector<Value *, 10> Args;
  for_each(Resolver->args(), [&](Argument &Arg) { Args.push_back(&Arg); });

  CallInst *Result =
      Builder.CreateCall(FunctionCallee(Resolver->getFunctionType(), ResolverPtrVal), Args);
  Result->setTailCall();
  Result->setCallingConv(Resolver->getCallingConv());

  if (Resolver->getReturnType()->isVoidTy())
    Builder.CreateRetVoid();
  else
    Builder.CreateRet(Result);

  Builder.SetInsertPoint(ElseBlock);
}

void llvm::emitMultiVersionResolver(Function *Resolver,
                                    ArrayRef<MultiVersionResolverOption> Options,
                                    bool UseIFunc, bool UseLibIRC) {

  assert(Triple(Resolver->getParent()->getTargetTriple()).isX86() &&
         "Only implemented for x86 targets");

  auto &Ctx = Resolver->getContext();
  // Main function's basic block.
  BasicBlock *CurBlock = BasicBlock::Create(Ctx, "resolver_entry", Resolver);

  IRBuilder<> Builder(CurBlock, CurBlock->begin());
  if (!UseIFunc) {
    emitResolverPtrTest(Resolver, Builder);
    CurBlock = Builder.GetInsertBlock();
  }

  if (UseLibIRC)
    X86::emitCpuFeaturesInit(Builder, UseIFunc);
  else
    X86::emitCPUInit(Builder, UseIFunc);

  for (const MultiVersionResolverOption &RO : Options) {
    Builder.SetInsertPoint(CurBlock);
    Value *Condition = formResolverCondition(Builder, RO, UseLibIRC);

    // The 'default' or 'generic' case.
    if (!Condition) {
      assert(&RO == Options.end() - 1 &&
             "Default or Generic case must be last");
      CreateMultiVersionResolverReturn(Resolver, Builder, RO.Fn, UseIFunc);
      return;
    }

    BasicBlock *RetBlock = BasicBlock::Create(Ctx, "resolver_return", Resolver);
    {
      IRBuilderBase::InsertPointGuard Guard(Builder);
      Builder.SetInsertPoint(RetBlock);
      CreateMultiVersionResolverReturn(Resolver, Builder, RO.Fn, UseIFunc);
    }
    CurBlock = BasicBlock::Create(Ctx, "resolver_else", Resolver);
    Builder.CreateCondBr(Condition, RetBlock, CurBlock);
  }

  // If no generic/default, emit an unreachable.
  Builder.SetInsertPoint(CurBlock);
  CallInst *TrapCall = Builder.CreateIntrinsic(Intrinsic::trap, {}, {});
  TrapCall->setDoesNotReturn();
  TrapCall->setDoesNotThrow();
  Builder.CreateUnreachable();
}

static Type *getCpuModelType(IRBuilderBase &Builder) {
  Type *Int32Ty = Builder.getInt32Ty();

  // Matching the struct layout from the compiler-rt/libgcc structure that is
  // filled in:
  // unsigned int __cpu_vendor;
  // unsigned int __cpu_type;
  // unsigned int __cpu_subtype;
  // unsigned int __cpu_features[1];
  Type *STy =
      StructType::get(Int32Ty, Int32Ty, Int32Ty, ArrayType::get(Int32Ty, 1));
  return STy;
}

static Value *getOrCreateGlobal(IRBuilderBase &Builder, StringRef Name,
                                Type *Ty, bool SetDSOLocal = true) {
  Module *M = Builder.GetInsertBlock()->getParent()->getParent();
  if (GlobalValue *Val = M->getNamedValue(Name)) {
    return Val;
  }

  auto *New = new GlobalVariable(
      *M, Ty, false, GlobalValue::ExternalLinkage, nullptr, Name, nullptr,
      GlobalVariable::NotThreadLocal, 0 /* AddressSpace */);
  if (SetDSOLocal)
    New->setDSOLocal(true);
  return New;
}

static void emitInit(IRBuilderBase &Builder, StringRef FuncName, bool UseIFunc) {

  Module *M = Builder.GetInsertBlock()->getParent()->getParent();
  Function *F = M->getFunction(FuncName);
  if (F) {
    if (UseIFunc)
      Builder.CreateCall(F);
    return;
  }

  FunctionType *FTy = FunctionType::get(Builder.getVoidTy(), /*Variadic*/ false);
  F = Function::Create(FTy, Function::ExternalLinkage, FuncName, M);

  if (shouldUseIntelFeaturesInitCallConv(FuncName))
    F->setCallingConv(CallingConv::Intel_Features_Init);

  if (UseIFunc)
    Builder.CreateCall(F);
  else
    appendToGlobalCtors(*M, F, 0);
}

void X86::emitCPUInit(IRBuilderBase &Builder, bool UseIFunc) {
  emitInit(Builder, "__cpu_indicator_init", UseIFunc);
}

void X86::emitCpuFeaturesInit(IRBuilderBase &Builder, bool UseIFunc) {
  emitInit(Builder, "__intel_cpu_features_init", UseIFunc);
}

Value *X86::emitCpuIs(IRBuilderBase &Builder, StringRef CPUStr) {
  // Calculate the index needed to access the correct field based on the
  // range. Also adjust the expected value.
  unsigned Index;
  unsigned Val;
  std::tie(Index, Val) = StringSwitch<std::pair<unsigned, unsigned>>(CPUStr)
#define X86_VENDOR(ENUM, STRING)                                               \
  .Case(STRING, {0u, static_cast<unsigned>(X86::ENUM)})
#define X86_CPU_TYPE_ALIAS(ENUM, ALIAS)                                        \
  .Case(ALIAS, {1u, static_cast<unsigned>(X86::ENUM)})
#define X86_CPU_TYPE(ENUM, STR)                                                \
  .Case(STR, {1u, static_cast<unsigned>(X86::ENUM)})
#define X86_CPU_SUBTYPE(ENUM, STR)                                             \
  .Case(STR, {2u, static_cast<unsigned>(X86::ENUM)})
#include "llvm/Support/X86TargetParser.def"
                             .Default({0, 0});
  assert(Val != 0 && "Invalid CPUStr passed to CpuIs");

  // Grab the appropriate field from __cpu_model.
  Value *Idxs[] = {Builder.getInt32(0), Builder.getInt32(Index)};
  Type *CpuModelType = getCpuModelType(Builder);
  Value *CpuValue = Builder.CreateGEP(
      CpuModelType, getOrCreateGlobal(Builder, "__cpu_model", CpuModelType),
      Idxs);
  CpuValue =
      Builder.CreateAlignedLoad(Builder.getInt32Ty(), CpuValue, MaybeAlign(4));

  // Check the value of the field against the requested value.
  return Builder.CreateICmpEQ(CpuValue, Builder.getInt32(Val));
}

Value *X86::emitCpuSupports(IRBuilderBase &Builder, uint64_t FeaturesMask) {

  uint32_t Features1 = Lo_32(FeaturesMask);
  uint32_t Features2 = Hi_32(FeaturesMask);

  Value *Result = Builder.getTrue();
  Type *Int32Ty = Builder.getInt32Ty();
  Type *CpuModelType = getCpuModelType(Builder);

  if (Features1 != 0) {
    Value *CpuModel = getOrCreateGlobal(Builder, "__cpu_model", CpuModelType);

    // Grab the first (0th) element from the field __cpu_features off of the
    // global in the struct STy.
    Value *Idxs[] = {Builder.getInt32(0), Builder.getInt32(3),
                     Builder.getInt32(0)};
    Value *CpuFeatures = Builder.CreateGEP(CpuModelType, CpuModel, Idxs);
    Value *Features =
        Builder.CreateAlignedLoad(Int32Ty, CpuFeatures, MaybeAlign(4));

    // Check the value of the bit corresponding to the feature requested.
    Value *Mask = Builder.getInt32(Features1);
    Value *Bitset = Builder.CreateAnd(Features, Mask);
    Value *Cmp = Builder.CreateICmpEQ(Bitset, Mask);
    Result = Builder.CreateAnd(Result, Cmp);
  }

  if (Features2 != 0) {
    Value *CpuFeatures2 =
        getOrCreateGlobal(Builder, "__cpu_features2", Int32Ty);

    Value *Features =
        Builder.CreateAlignedLoad(Int32Ty, CpuFeatures2, Align(4));

    // Check the value of the bit corresponding to the feature requested.
    Value *Mask = Builder.getInt32(Features2);
    Value *Bitset = Builder.CreateAnd(Features, Mask);
    Value *Cmp = Builder.CreateICmpEQ(Bitset, Mask);
    Result = Builder.CreateAnd(Result, Cmp);
  }

  return Result;
}

Value *X86::emitCpuSupports(IRBuilderBase &Builder,
                            ArrayRef<StringRef> FeatureStrs) {
  return emitCpuSupports(Builder, getCpuSupportsMask(FeatureStrs));
}

Value *X86::mayIUseCpuFeatureHelper(IRBuilderBase &Builder,
                                    ArrayRef<APSInt> Pages) {

  Type *Ty = ArrayType::get(Builder.getInt64Ty() , 2);
  Value *IndicatorPtr = getOrCreateGlobal(
      Builder, "__intel_cpu_feature_indicator", Ty, false /*SetDSOLocal*/);

  Value *RollingResult = nullptr;
  for (unsigned CurPage = 0; CurPage < Pages.size(); ++CurPage) {
    APSInt Feats = Pages[CurPage];
    if (Feats == 0)
      continue;

    Value *OffsetIndicator = Builder.CreateConstGEP2_64(
        Ty, IndicatorPtr, 0, CurPage, "cpu_feature_offset");
    Value *Indicator = Builder.CreateAlignedLoad(
        Builder.getInt64Ty(), OffsetIndicator,
        MaybeAlign(8), // TODO: Verify this.
        "cpu_feature_indicator");
    Value *Join =
        Builder.CreateAnd(Indicator, Feats, "cpu_feature_join");
    Value *Result = Builder.CreateICmpEQ(
        Join, Builder.getInt(Feats), "cpu_feature_check");

    if (RollingResult)
      RollingResult = Builder.CreateAnd(RollingResult, Result, "page_join");
    else
      RollingResult = Result;
  }

  // If we never found anything to check because we are attempting to validate
  // if you can use the 'zero feature', make sure the result is 'true'.
  if (!RollingResult)
    RollingResult = Builder.getTrue();

  return RollingResult;
}

//===-------- Intel_X86EmitMultiVersionResolver.cpp -----------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/Intel_X86EmitMultiVersionResolver.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Intel_CPU_utils.h"
#include "llvm/TargetParser/X86TargetParser.h"
#include "llvm/Transforms/Utils/Intel_IMLUtils.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

using namespace llvm;
using namespace llvm::X86;

Value *llvm::formResolverCondition(IRBuilderBase &Builder,
                                   const MultiVersionResolverOption &RO,
                                   bool UseLibIRC, bool PerformCPUBrandCheck) {
  Value *Condition = nullptr;

  if (!RO.Conditions.Architecture.empty())
    Condition = X86::emitCpuIs(Builder, RO.Conditions.Architecture);

  if (!RO.Conditions.Features.empty()) {
    Value *FeatureCond = nullptr;
    if (UseLibIRC) {
      std::array<uint64_t, 2> Bitmaps =
          X86::getCpuFeatureBitmap(RO.Conditions.Features, /*OnlyAutoGen=*/true);
      FeatureCond =
          X86::mayIUseCpuFeatureHelper(Builder,
                                       {APSInt{APInt(64, Bitmaps[0]), true},
                                        APSInt{APInt(64, Bitmaps[1]), true}},
                                       PerformCPUBrandCheck);
    } else
      FeatureCond = X86::emitCpuSupports(Builder, RO.Conditions.Features);

    Condition =
        Condition ? Builder.CreateAnd(Condition, FeatureCond) : FeatureCond;
  }

  return Condition;
}

static void CreateMultiVersionResolverBranch(IRBuilderBase &Builder,
                                             Function *FuncToReturn,
                                             GlobalVariable *DispatchPtr,
                                             bool UseIFunc,
                                             BasicBlock *ExitBlock) {
  if (UseIFunc) {
    Builder.CreateRet(FuncToReturn);
    return;
  }

  Builder.CreateAlignedStore(FuncToReturn, DispatchPtr, MaybeAlign(8));
  Builder.CreateBr(ExitBlock);
}

void llvm::emitMultiVersionResolver(Function *Resolver,
                                    GlobalVariable *DispatchPtr,
                                    ArrayRef<MultiVersionResolverOption> Options,
                                    bool UseIFunc, bool UseLibIRC,
                                    bool PerformCPUBrandCheck) {

  assert(Triple(Resolver->getParent()->getTargetTriple()).isX86() &&
         "Only implemented for x86 targets");

  auto &Ctx = Resolver->getContext();

  BasicBlock *CurBlock = nullptr;
  if (Resolver->empty())
    CurBlock = BasicBlock::Create(Ctx, "resolver_entry", Resolver);
  else {
    CurBlock = &Resolver->back();
    CurBlock->erase(CurBlock->begin(), CurBlock->end());
  }

  BasicBlock *ExitBlock = nullptr;
  if (!UseIFunc)
    ExitBlock = BasicBlock::Create(Ctx, "resolver_exit", Resolver);

  IRBuilder<> Builder(CurBlock, CurBlock->begin());

  if (UseLibIRC)
    X86::emitCpuFeaturesInit(Builder, UseIFunc, PerformCPUBrandCheck);
  else
    X86::emitCPUInit(Builder, UseIFunc);

  for (const MultiVersionResolverOption &RO : Options) {

    Builder.SetInsertPoint(CurBlock);

    Value *Condition =
        formResolverCondition(Builder, RO, UseLibIRC, PerformCPUBrandCheck);

    // The 'default' or 'generic' case.
    if (!Condition) {
      assert(&RO == Options.end() - 1 &&
             "Default or Generic case must be last");
      CreateMultiVersionResolverBranch(Builder, RO.Fn, DispatchPtr, UseIFunc,
                                       ExitBlock);
      break;
    }

    BasicBlock *RetBlock =
        BasicBlock::Create(Ctx, "resolver_return", Resolver, ExitBlock);
    {
      IRBuilderBase::InsertPointGuard Guard(Builder);
      Builder.SetInsertPoint(RetBlock);
      CreateMultiVersionResolverBranch(Builder, RO.Fn, DispatchPtr, UseIFunc,
                                       ExitBlock);
    }
    CurBlock = BasicBlock::Create(Ctx, "resolver_else", Resolver, ExitBlock);
    Builder.CreateCondBr(Condition, RetBlock, CurBlock);
  }

  if (!UseIFunc) {
    Builder.SetInsertPoint(ExitBlock);
    Builder.CreateRetVoid();
  }
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

void X86::emitCpuFeaturesInit(IRBuilderBase &Builder, bool UseIFunc,
                              bool PerformCPUBrandCheck) {
  if (PerformCPUBrandCheck)
    emitInit(Builder, "__intel_cpu_features_init", UseIFunc);
  else
    emitInit(Builder, "__intel_cpu_features_init_x", UseIFunc);
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
#include "llvm/TargetParser/X86TargetParser.def"
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

Value *X86::emitCpuSupports(IRBuilderBase &Builder,
                            std::array<uint32_t, 4> FeaturesMask) {

  Value *Result = Builder.getTrue();
  Type *Int32Ty = Builder.getInt32Ty();
  Type *CpuModelType = getCpuModelType(Builder);

  if (FeaturesMask[0] != 0) {
    Value *CpuModel = getOrCreateGlobal(Builder, "__cpu_model", CpuModelType);

    // Grab the first (0th) element from the field __cpu_features off of the
    // global in the struct STy.
    Value *Idxs[] = {Builder.getInt32(0), Builder.getInt32(3),
                     Builder.getInt32(0)};
    Value *CpuFeatures = Builder.CreateGEP(CpuModelType, CpuModel, Idxs);
    Value *Features =
        Builder.CreateAlignedLoad(Int32Ty, CpuFeatures, MaybeAlign(4));

    // Check the value of the bit corresponding to the feature requested.
    Value *Mask = Builder.getInt32(FeaturesMask[0]);
    Value *Bitset = Builder.CreateAnd(Features, Mask);
    Value *Cmp = Builder.CreateICmpEQ(Bitset, Mask);
    Result = Builder.CreateAnd(Result, Cmp);
  }
  llvm::Type *ATy = llvm::ArrayType::get(Int32Ty, 3);
  Value *CpuFeatures2 = getOrCreateGlobal(Builder, "__cpu_features2", Int32Ty);
  cast<llvm::GlobalValue>(CpuFeatures2)->setDSOLocal(true);
  for (int i = 1; i != 4; ++i) {
    const uint32_t M = FeaturesMask[i];
    if (!M)
      continue;
    Value *Idxs[] = {Builder.getInt32(0), Builder.getInt32(i - 1)};
    Value *Features = Builder.CreateAlignedLoad(
        Int32Ty, Builder.CreateGEP(ATy, CpuFeatures2, Idxs), Align(4));
    // Check the value of the bit corresponding to the feature requested.
    Value *Mask = Builder.getInt32(M);
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
                                    ArrayRef<APSInt> Pages,
                                    bool PerformCPUBrandCheck) {

  Type *Ty = ArrayType::get(Builder.getInt64Ty(), 2);
  StringRef IndicatorName =
      PerformCPUBrandCheck ? "__intel_cpu_feature_indicator" :
                             "__intel_cpu_feature_indicator_x";
  Value *IndicatorPtr =
      getOrCreateGlobal(Builder, IndicatorName, Ty, false /*SetDSOLocal*/);

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

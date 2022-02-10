//===-- X86EmitMultiVersionResolver -----------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements utitlities to generate code used for CPU dispatch code.
// INTEL: Upstreaming attempt is at https://reviews.llvm.org/D108424.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/Intel_X86EmitMultiVersionResolver.h" // INTEL
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Triple.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Intel_CPU_utils.h"
#include "llvm/Support/X86TargetParser.h"

using namespace llvm;
using namespace llvm::X86;

Value *llvm::formResolverCondition(IRBuilderBase &Builder,
#if INTEL_CUSTOMIZATION
                                   const MultiVersionResolverOption &RO,
                                   bool UseLibIRC) {
#endif // INTEL_CUSTOMIZATION
  llvm::Value *Condition = nullptr;

  if (!RO.Conditions.Architecture.empty())
    Condition = llvm::X86::emitCpuIs(Builder, RO.Conditions.Architecture);
  if (!RO.Conditions.Features.empty()) {
#if INTEL_CUSTOMIZATION
    llvm::Value *FeatureCond = nullptr;
    if (UseLibIRC) {
      std::array<uint64_t, 2> Bitmaps =
          llvm::X86::getCpuFeatureBitmap(RO.Conditions.Features,
                                         /*OnlyAutoGen=*/true);
      FeatureCond = llvm::X86::mayIUseCpuFeatureHelper(
          Builder,
          {APSInt{APInt(64, Bitmaps[0]), true},
           APSInt{APInt(64, Bitmaps[1]), true}},
          /*CreateInitCall=*/false);
    } else
      FeatureCond = llvm::X86::emitCpuSupports(Builder, RO.Conditions.Features);
#endif // INTEL_CUSTOMIZATION
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

void llvm::emitMultiVersionResolver(
#if INTEL_CUSTOMIZATION
    Function *Resolver, ArrayRef<MultiVersionResolverOption> Options,
    bool UseIFunc, bool UseLibIRC) {
#endif // INTEL_CUSTOMIZATION
  assert(Triple(Resolver->getParent()->getTargetTriple()).isX86() &&
         "Only implemented for x86 targets");

  auto &Ctx = Resolver->getContext();
  // Main function's basic block.
  BasicBlock *CurBlock = BasicBlock::Create(Ctx, "resolver_entry", Resolver);

  IRBuilder<> Builder(CurBlock, CurBlock->begin());
#if INTEL_CUSTOMIZATION
  if (UseLibIRC)
    llvm::X86::emitCpuFeaturesInit(Builder);
  else
    llvm::X86::emitCPUInit(Builder);
#endif // INTEL_CUSTOMIZATION

  for (const MultiVersionResolverOption &RO : Options) {
    Builder.SetInsertPoint(CurBlock);
#if INTEL_CUSTOMIZATION
    llvm::Value *Condition = formResolverCondition(Builder, RO, UseLibIRC);
#endif // INTEL_CUSTOMIZATION

    // The 'default' or 'generic' case.
    if (!Condition) {
      assert(&RO == Options.end() - 1 &&
             "Default or Generic case must be last");
      CreateMultiVersionResolverReturn(Resolver, Builder, RO.Fn, UseIFunc);
      return;
    }

    llvm::BasicBlock *RetBlock =
        BasicBlock::Create(Ctx, "resolver_return", Resolver);
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

#if INTEL_CUSTOMIZATION
static Value *emitInit(IRBuilderBase &Builder, StringRef FuncName) {
  FunctionType *FTy = FunctionType::get(Builder.getVoidTy(),
                                        /*Variadic*/ false);
  Module *M = Builder.GetInsertBlock()->getParent()->getParent();
  GlobalValue *Entry = M->getNamedValue(FuncName);
  if (Entry) {
    // TODO: asserts and possibly return nullptr if something bad.
    return Builder.CreateCall(cast<Function>(Entry));
  }

  Function *F = Function::Create(FTy, Function::ExternalLinkage,
                                 FuncName, M);

  F->setDSOLocal(true);
  F->setDLLStorageClass(GlobalValue::DefaultStorageClass);
  return Builder.CreateCall(F);
}
Value *llvm::X86::emitCPUInit(IRBuilderBase &Builder) {
  return emitInit(Builder, "__cpu_indicator_init");
}
void llvm::X86::emitCpuFeaturesInit(IRBuilderBase &Builder) {
  emitInit(Builder, "__intel_cpu_features_init_x");
}
#endif // INTEL_CUSTOMIZATION

Value *llvm::X86::emitCpuIs(IRBuilderBase &Builder, StringRef CPUStr) {
  // Calculate the index needed to access the correct field based on the
  // range. Also adjust the expected value.
  unsigned Index;
  unsigned Val;
  std::tie(Index, Val) = StringSwitch<std::pair<unsigned, unsigned>>(CPUStr)
#define X86_VENDOR(ENUM, STRING)                                               \
  .Case(STRING, {0u, static_cast<unsigned>(llvm::X86::ENUM)})
#define X86_CPU_TYPE_ALIAS(ENUM, ALIAS)                                        \
  .Case(ALIAS, {1u, static_cast<unsigned>(llvm::X86::ENUM)})
#define X86_CPU_TYPE(ENUM, STR)                                                \
  .Case(STR, {1u, static_cast<unsigned>(llvm::X86::ENUM)})
#define X86_CPU_SUBTYPE(ENUM, STR)                                             \
  .Case(STR, {2u, static_cast<unsigned>(llvm::X86::ENUM)})
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

Value *llvm::X86::emitCpuSupports(IRBuilderBase &Builder,
                                  uint64_t FeaturesMask) {
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

Value *llvm::X86::emitCpuSupports(IRBuilderBase &Builder,
                                  ArrayRef<StringRef> FeatureStrs) {
  return emitCpuSupports(Builder, getCpuSupportsMask(FeatureStrs));
}

Value *llvm::X86::mayIUseCpuFeatureHelper(IRBuilderBase &Builder,
                                          ArrayRef<llvm::APSInt> Pages,
                                          bool CreateInitCall) {
  if (CreateInitCall)
    emitCpuFeaturesInit(Builder);

  Type *Ty = ArrayType::get(Builder.getInt64Ty() , 2);
  Value *IndicatorPtr = getOrCreateGlobal(
      Builder, "__intel_cpu_feature_indicator_x", Ty, false /*SetDSOLocal*/);

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

//=----- SGHelper.cpp - Helper functions for subgroup emulation - C++ -*-----=//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SubgroupEmulation/SGHelper.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Twine.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/KernelBarrierUtils.h"

#include <string>

#define CLK_LOCAL_MEM_FENCE 0x01

using namespace llvm;

#define CHECK_IF_INITIALIZED                                                   \
  assert(Initialized && "SGHelper is called before initialized");

namespace llvm {

const char *SGHelper::DummyBarrierName = "dummy_sg_barrier";
const char *SGHelper::BarrierNameNoScope = "_Z17sub_group_barrierj";
const char *SGHelper::BarrierNameWithScope =
    "_Z17sub_group_barrierj12memory_scope";

SGHelper::SGHelper()
    : M(nullptr), Context(nullptr), SGBarrierF(nullptr),
      SGDummyBarrierF(nullptr), VoidType(nullptr), Int32Type(nullptr),
      LocalMemFence(nullptr), GetSGSizeF(nullptr), GetMaxSGSizeF(nullptr),
      GetSGLIdF(nullptr), ConstZero(nullptr), ConstOne(nullptr),
      Initialized(false) {}

void SGHelper::initialize(Module &M) {
  invalidate();
  this->M = &M;
  Context = &M.getContext();
  VoidType = Type::getVoidTy(*Context);
  Int32Type = IntegerType::get(*Context, 32);
  ConstZero = ConstantInt::get(Int32Type, 0);
  ConstOne = ConstantInt::get(Int32Type, 1);
  LocalMemFence = ConstantInt::get(Int32Type, CLK_LOCAL_MEM_FENCE);
  findBarriers();
  findDummyBarriers();
  Initialized = true;
}

void SGHelper::removeBarriers(ArrayRef<Instruction *> Insts) {
  CHECK_IF_INITIALIZED
  for (auto *I : Insts) {
    assert(isBarrier(I) && "Not a barrier call");
    FuncToBarriers[I->getFunction()].remove(I);
    // No need to remove sth from SGSyncFunctions, since we won't remove all
    // sub_group_barrier calls from a function.
    I->eraseFromParent();
  }
}

void SGHelper::removeDummyBarriers(ArrayRef<Instruction *> Insts) {
  CHECK_IF_INITIALIZED
  for (auto *I : Insts) {
    assert(isDummyBarrier(I) && "Not a dummy barrier call");
    FuncToDummyBarriers[I->getFunction()].remove(I);
    I->eraseFromParent();
  }
}

bool SGHelper::isDummyBarrier(Instruction *I) {
  CallInst *CI = dyn_cast<CallInst>(I);
  if (!CI)
    return false;
  auto *CF = CI->getCalledFunction();
  if (!CF)
    return false;
  return CF->getName() == DummyBarrierName;
}

bool SGHelper::isBarrier(Instruction *I) {
  CallInst *CI = dyn_cast<CallInst>(I);
  if (!CI)
    return false;
  auto *CF = CI->getCalledFunction();
  if (!CF)
    return false;
  auto Name = CF->getName();
  return Name == BarrierNameWithScope || Name == BarrierNameNoScope;
}

bool SGHelper::isSyncCall(Instruction *I) {
  return isDummyBarrier(I) || isBarrier(I);
}

void SGHelper::findBarriers() {
  static SmallVector<std::string, 2> BarrierNames = {BarrierNameNoScope,
                                                     BarrierNameWithScope};
  for (auto &BarrierName : BarrierNames) {
    auto *SGBarrierFunc = M->getFunction(BarrierName);
    if (SGBarrierFunc == nullptr)
      continue;
    for (User *U : SGBarrierFunc->users()) {
      CallInst *CI = dyn_cast<CallInst>(U);
      assert(CI && "sub_group_barrier is used not as a call");
      SGSyncFunctions.insert(CI->getFunction());
      FuncToBarriers[CI->getFunction()].insert(CI);
    }
  }
}

void SGHelper::findDummyBarriers() {
  auto *SGDummyBarrierFunc = M->getFunction(DummyBarrierName);
  if (SGDummyBarrierFunc == nullptr)
    return;
  for (User *U : SGDummyBarrierFunc->users()) {
    CallInst *CI = dyn_cast<CallInst>(U);
    assert(CI && "sub_group_barrier is used not as a call");
    FuncToDummyBarriers[CI->getFunction()].insert(CI);
  }
}

const InstSet &SGHelper::getBarriersForFunction(Function *F) {
  CHECK_IF_INITIALIZED
  return FuncToBarriers[F];
}

const InstSet &SGHelper::getDummyBarriersForFunction(Function *F) {
  CHECK_IF_INITIALIZED
  return FuncToDummyBarriers[F];
}

FuncSet SGHelper::getAllFunctionsNeedEmulation() {
  CHECK_IF_INITIALIZED
  FuncSet FuncsNeedEmuation;
  // Only functions to be emulated have dummy_sg_barrier calls.
  for (auto &Item : FuncToDummyBarriers)
    FuncsNeedEmuation.insert(Item.first);
  return FuncsNeedEmuation;
}

InstSet SGHelper::getSyncInstsForFunction(Function *F) {
  CHECK_IF_INITIALIZED
  InstSet SyncInsts = FuncToBarriers[F];
  for (auto *I : FuncToDummyBarriers[F])
    SyncInsts.insert(I);
  return SyncInsts;
}

const FuncSet &SGHelper::getAllSyncFunctions() const {
  CHECK_IF_INITIALIZED
  return SGSyncFunctions;
}

Instruction *SGHelper::getFirstDummyBarrier(Function *F) {
  for (auto &I : instructions(F))
    if (isDummyBarrier(&I))
      return &I;
  return nullptr;
}

Instruction *SGHelper::insertBarrierBefore(Instruction *IP) {
  CHECK_IF_INITIALIZED
  assert(IP != nullptr && IP->getModule() == M && "illegal insert point");
  auto *CI = createBarrierCall();
  CI->setDebugLoc(IP->getDebugLoc());
  CI->insertBefore(IP);
  SGSyncFunctions.insert(CI->getFunction());
  FuncToBarriers[CI->getFunction()].insert(CI);
  return CI;
}

Instruction *SGHelper::insertBarrierAfter(Instruction *IP) {
  CHECK_IF_INITIALIZED
  assert(IP != nullptr && IP->getModule() == M && "illegal insert point");
  auto *CI = createBarrierCall();
  CI->setDebugLoc(IP->getDebugLoc());
  CI->insertAfter(IP);
  SGSyncFunctions.insert(CI->getFunction());
  FuncToBarriers[CI->getFunction()].insert(CI);
  return CI;
}

Instruction *SGHelper::insertDummyBarrierBefore(Instruction *IP) {
  CHECK_IF_INITIALIZED
  assert(IP != nullptr && IP->getModule() == M && "illegal insert point");
  auto *CI = createDummyBarrierCall();
  CI->insertBefore(IP);
  FuncToDummyBarriers[CI->getFunction()].insert(CI);
  return CI;
}

Instruction *SGHelper::insertDummyBarrierAfter(Instruction *IP) {
  CHECK_IF_INITIALIZED
  assert(IP != nullptr && IP->getModule() == M && "illegal insert point");
  auto *CI = createDummyBarrierCall();
  CI->insertAfter(IP);
  FuncToDummyBarriers[CI->getFunction()].insert(CI);
  return CI;
}

Value *SGHelper::createGetMaxSubGroupSize(Instruction *IP) {
  static const std::string GetMaxSGSizeName =
      DPCPPKernelCompilationUtils::mangledGetMaxSubGroupSize();
  if (GetMaxSGSizeF == nullptr) {
    GetMaxSGSizeF = M->getFunction(GetMaxSGSizeName);
    if (GetMaxSGSizeF == nullptr) {
      SmallVector<Type *, 2> ArgTy;
      FunctionType *GetMaxSGSizeFTy =
          FunctionType::get(Int32Type, ArgTy, false);
      assert(GetMaxSGSizeFTy && "Failed to create FunctionType");
      GetMaxSGSizeF = Function::Create(
          GetMaxSGSizeFTy, GlobalValue::ExternalLinkage, GetMaxSGSizeName, M);
      assert(GetMaxSGSizeF && "Failed to create Function");
    }
  }
  return CallInst::Create(GetMaxSGSizeF, "sg.max.size.", IP);
}

Value *SGHelper::createGetSubGroupSize(Instruction *IP) {
  static const std::string GetSGSizeName =
      DPCPPKernelCompilationUtils::mangledGetSubGroupSize();
  if (GetSGSizeF == nullptr) {
    GetSGSizeF = M->getFunction(GetSGSizeName);
    if (GetSGSizeF == nullptr) {
      SmallVector<Type *, 2> ArgTy;
      FunctionType *GetSGSizeFTy = FunctionType::get(Int32Type, ArgTy, false);
      assert(GetSGSizeFTy && "Failed to create FunctionType");
      GetSGSizeF = Function::Create(GetSGSizeFTy, GlobalValue::ExternalLinkage,
                                    GetSGSizeName, M);
      assert(GetSGSizeF && "Failed to create Function");
    }
  }
  CallInst *CI = CallInst::Create(GetSGSizeF, "sg.size.", IP);
  CI->setDebugLoc(IP->getDebugLoc());
  return CI;
}

Value *SGHelper::createGetSubGroupLId(Instruction *IP) {
  static const std::string GetSGLIdName =
      DPCPPKernelCompilationUtils::mangledGetSubGroupLocalId();
  if (GetSGLIdF == nullptr) {
    GetSGLIdF = M->getFunction(GetSGLIdName);
    if (GetSGLIdF == nullptr) {
      SmallVector<Type *, 2> ArgTy;
      FunctionType *GetSGSizeFTy = FunctionType::get(Int32Type, ArgTy, false);
      assert(GetSGSizeFTy && "Failed to create FunctionType");
      GetSGLIdF = Function::Create(GetSGSizeFTy, GlobalValue::ExternalLinkage,
                                   GetSGLIdName, M);
      assert(GetSGLIdF && "Failed to create Function");
    }
  }
  CallInst *CI = CallInst::Create(GetSGLIdF, "sg.lid.", IP);
  CI->setDebugLoc(IP->getDebugLoc());
  return CI;
}

Instruction *SGHelper::createBarrierCall() {
  CHECK_IF_INITIALIZED
  if (SGBarrierF == nullptr) {
    SGBarrierF = M->getFunction(BarrierNameNoScope);
    if (SGBarrierF == nullptr) {
      SmallVector<Type *, 2> ArgTy{Int32Type};
      FunctionType *SGBarrierFTy = FunctionType::get(VoidType, ArgTy, false);
      assert(SGBarrierFTy && "Failed to create FunctionType");
      SGBarrierF = Function::Create(SGBarrierFTy, GlobalValue::ExternalLinkage,
                                    BarrierNameNoScope, M);
      assert(SGBarrierF && "Failed to create Function");
    }
  }
  return CallInst::Create(SGBarrierF, LocalMemFence);
}

Instruction *SGHelper::createDummyBarrierCall() {
  CHECK_IF_INITIALIZED
  if (SGDummyBarrierF == nullptr) {
    SGDummyBarrierF = M->getFunction(DummyBarrierName);
    if (SGDummyBarrierF == nullptr) {
      SmallVector<Type *, 2> ArgTy;
      FunctionType *SGDummyBarrierFTy =
          FunctionType::get(VoidType, ArgTy, false);
      assert(SGDummyBarrierFTy && "Failed to create FunctionType");
      SGDummyBarrierF = Function::Create(
          SGDummyBarrierFTy, GlobalValue::ExternalLinkage, DummyBarrierName, M);
      assert(SGDummyBarrierF && "Failed to create Function");
    }
  }
  return CallInst::Create(SGDummyBarrierF);
}

Type *SGHelper::getPromotedIntVecType(Type *IntVecType) {
  auto *VecType = dyn_cast<FixedVectorType>(IntVecType);
  // Only cares about integer vector promotion.
  if (!VecType || !VecType->getScalarType()->isIntegerTy())
    return IntVecType;

  auto *IntElemType = cast<IntegerType>(VecType->getScalarType());
  auto ElemBitWidth = IntElemType->getBitWidth();
  // If bit width is multiple of 8, no need to promote.
  if (ElemBitWidth % 8 == 0)
    return IntVecType;

  // Ceil to multiple of 8.
  auto PromotedBitWidth = (ElemBitWidth + 7) / 8 * 8;
  return FixedVectorType::get(
      IntegerType::get(IntVecType->getContext(), PromotedBitWidth),
      VecType->getNumElements());
}

Type *SGHelper::getVectorType(Type *T, unsigned Size) {
  Type *WidenType = nullptr;
  if (auto *VecType = dyn_cast<FixedVectorType>(T)) {
    WidenType = FixedVectorType::get(VecType->getScalarType(),
                                     Size * VecType->getNumElements());
  } else {
    // Only allow non-agg Type.
    WidenType = FixedVectorType::get(T, Size);
  }

  return getPromotedIntVecType(WidenType);
}

Value *SGHelper::createZExtOrTruncProxy(Value *From, Type *ToType, IRBuilder<> &Builder) {
  if (!From->getType()->isIntOrIntVectorTy() || !ToType->isIntOrIntVectorTy())
    return From;
  return Builder.CreateZExtOrTrunc(From, ToType);
}

} // namespace llvm

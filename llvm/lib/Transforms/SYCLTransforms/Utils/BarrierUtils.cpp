//===- BarrierUtils.cpp - Barrier Utils -----------------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/Utils/BarrierUtils.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ModRef.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/SYCLTransforms/Utils/ParameterType.h"

using namespace llvm;
using namespace SYCLKernelMetadataAPI;

BarrierUtils::BarrierUtils() : M(nullptr), UISizeT(0) { clean(); }

void BarrierUtils::init(Module *M) {
  assert(M && "Trying to initialize BarrierUtils with NULL module");
  this->M = M;

  // Get size of size_t in bits from the module
  clean();
  UISizeT = M->getDataLayout().getPointerSizeInBits(0);
  assert(UISizeT == 32 || UISizeT == 64);
  I32Ty = Type::getInt32Ty(M->getContext());
  SizetTy = IntegerType::get(M->getContext(), UISizeT);
  SpecialBufferValueTy = IntegerType::get(M->getContext(), 8);
}

void BarrierUtils::clean() {
  LocalMemFenceValue = 0;
  BarrierFunc = nullptr;
  DummyBarrierFunc = nullptr;
  GetSpecialBufferFunc = nullptr;
  GetGIDFunc = nullptr;
  GetLIDFunc = nullptr;
  GetSGSizeFunc = nullptr;
  GetBaseGIDFunc = nullptr;
  GetLocalSizeFunc = nullptr;
  SyncDataInitialized = false;
  LIDInitialized = false;
  GIDInitialized = false;
  NonInlinedCallsInitialized = false;
}

SmallVector<BasicBlock *>
BarrierUtils::findBasicBlocksOfPhiNode(Value *Val, PHINode *PhiNode) {
  // Usage is a PHINode, find previous basic block according to Val
  SmallVector<BasicBlock *, 1> PrevBBs;
  for (BasicBlock *BB : predecessors(PhiNode->getParent())) {
    Value *PHINodeVal = PhiNode->getIncomingValueForBlock(BB);
    if (PHINodeVal == Val) {
      PrevBBs.push_back(BB);
    }
  }
  assert(PrevBBs.size() && "Failed to find previous basic block!");
  return PrevBBs;
}

SyncType BarrierUtils::getSyncType(Instruction *Inst) {
  initializeSyncData();

  if (!isa<CallInst>(Inst))
    return SyncType::None;
  if (Barriers.count(Inst))
    return SyncType::Barrier;
  if (DummyBarriers.count(Inst))
    return SyncType::DummyBarrier;
  return SyncType::None;
}

SyncType BarrierUtils::getSyncType(BasicBlock *BB) {
  return getSyncType(&*BB->begin());
}

CompilationUtils::InstVec BarrierUtils::getAllSynchronizeInstructions() {
  // Initialize sync data if it is not done yet
  initializeSyncData();

  // Clear old collected data!
  CompilationUtils::InstVec SyncInstructions;

  SyncInstructions.insert(SyncInstructions.end(), Barriers.begin(),
                          Barriers.end());
  SyncInstructions.insert(SyncInstructions.end(), DummyBarriers.begin(),
                          DummyBarriers.end());

  return SyncInstructions;
}

CompilationUtils::InstVec BarrierUtils::getDeviceBarrierCallInsts() {
  CompilationUtils::InstVec RGcallInstructions;
  std::string Names[] = {CompilationUtils::mangledDeviceBarrier(),
                         CompilationUtils::mangledDeviceBarrierWithScope()};
  for (const auto &Name : Names) {
    Function *Func = M->getFunction(Name);
    if (Func) {
      for (User *U : Func->users()) {
        CallInst *CI = cast<CallInst>(U);
        RGcallInstructions.push_back(CI);
      }
    }
  }
  return RGcallInstructions;
}

CompilationUtils::InstVec BarrierUtils::getWGCallInstructions(CALL_BI_TYPE Ty) {
  CompilationUtils::InstVec WGcallInstructions;

  // Scan external function definitions in the module
  for (auto &F : *M) {
    if (!F.isDeclaration()) {
      // Built-in functions are assumed to be declarations.
      continue;
    }
    StringRef FName = F.getName();
    if ((CALL_BI_TYPE_WG == Ty && CompilationUtils::isWorkGroupBuiltin(FName) &&
         !CompilationUtils::isWorkGroupSort(FName)) ||
        (CALL_BI_TYPE_WG_ASYNC_OR_PIPE == Ty &&
         CompilationUtils::isWorkGroupAsyncOrPipeBuiltin(FName, *M)) ||
        (CALL_BI_TYPE_WG_SORT == Ty &&
         CompilationUtils::isWorkGroupSort(FName))) {
      // Module contains declaration of a WG function built-in, FIx its
      // usages.
      for (User *U : F.users()) {
        CallInst *CI = cast<CallInst>(U);
        WGcallInstructions.push_back(CI);
      }
    }
  }

  return WGcallInstructions;
}

CompilationUtils::FuncSet BarrierUtils::getAllFunctionsWithSynchronization() {
  auto SyncInstructions = getAllSynchronizeInstructions();

  CompilationUtils::FuncSet SyncFunctions;
  for (auto *Inst : SyncInstructions)
    SyncFunctions.insert(Inst->getFunction());
  return SyncFunctions;
}

CompilationUtils::FuncSet BarrierUtils::getRecursiveFunctionsWithSync() {
  CompilationUtils::FuncSet SyncFunctions =
      getAllFunctionsWithSynchronization();
  CompilationUtils::FuncSet RecursiveFunctions;
  for (Function *F : SyncFunctions) {
    auto FMD = FunctionMetadataAPI(F);
    if (FMD.RecursiveCall.hasValue() && FMD.RecursiveCall.get())
      RecursiveFunctions.insert(F);
  }
  return RecursiveFunctions;
}

CompilationUtils::FuncVec BarrierUtils::getAllKernelsAndVectorizedCounterparts(
    const SmallVectorImpl<Function *> &KernelList) {
  CompilationUtils::FuncVec Result;

  for (auto *F : KernelList) {
    Result.push_back(F);
    auto VectorizedKernelMetadata =
        KernelInternalMetadataAPI(F).VectorizedKernel;
    if (VectorizedKernelMetadata.hasValue() && VectorizedKernelMetadata.get())
      Result.push_back(VectorizedKernelMetadata.get());
  }

  return Result;
}

CompilationUtils::FuncVec BarrierUtils::getAllKernelsWithBarrier() {
  auto Kernels = KernelList(M);

  CompilationUtils::FuncVec KernelFunctions;
  if (Kernels.empty())
    return KernelFunctions;

  // Get the kernels using the barrier for work group loops.
  SmallVector<Function *, 4> KernelsWithBarrier;
  for (auto *Func : Kernels) {
    auto kimd = KernelInternalMetadataAPI(Func);
    if (kimd.NoBarrierPath.get())
      continue;

    // Currently no check if kernel already added to the list!
    KernelsWithBarrier.push_back(Func);
  }
  KernelFunctions = getAllKernelsAndVectorizedCounterparts(KernelsWithBarrier);

  // collect functions to process
  const auto &TodoList =
      getAllKernelsAndVectorizedCounterparts(Kernels.getList());

  for (auto *Func : TodoList) {
    auto kimd = KernelInternalMetadataAPI(Func);
    KernelVectorizationWidths[Func] =
        kimd.VectorizedWidth.hasValue() ? kimd.VectorizedWidth.get() : 1;
  }

  return KernelFunctions;
}

unsigned BarrierUtils::getFunctionVectorizationWidth(const Function *F) const {
  FunctionToUnsigned::const_iterator I = KernelVectorizationWidths.find(F);
  if (I == KernelVectorizationWidths.end()) {
    // For subgroup emulation.
    if (F->hasFnAttribute("widened-size")) {
      unsigned WidenedSize = 0;
      bool Failed = F->getFnAttribute("widened-size")
                        .getValueAsString()
                        .getAsInteger(10, WidenedSize);
      (void)Failed;
      assert(!Failed && "Unexpected widened-size attribute");
      return WidenedSize;
    }
    auto FIMD = KernelInternalMetadataAPI(const_cast<Function *>(F));
    return FIMD.VectorizedWidth.hasValue() ? FIMD.VectorizedWidth.get() : 1;
  }
  return I->second;
}

Instruction *BarrierUtils::createBarrier(Instruction *InsertBefore) {
  if (!BarrierFunc)
    BarrierFunc = M->getFunction(CompilationUtils::mangledWGBarrier(
        CompilationUtils::BarrierType::NoScope));
  if (!BarrierFunc) {
    // Module has no barrier declaration. Create one
    Type *Result = Type::getVoidTy(M->getContext());
    Type *FuncArgTys[] = {IntegerType::get(M->getContext(), 32)};
    BarrierFunc = CompilationUtils::createFunctionDeclaration(
        CompilationUtils::mangledWGBarrier(
            CompilationUtils::BarrierType::NoScope),
        Result, FuncArgTys, M);
    BarrierFunc->setAttributes(BarrierFunc->getAttributes().addFnAttribute(
        BarrierFunc->getContext(),
        Attribute::Convergent));
  }
  if (!LocalMemFenceValue) {
    // LocalMemFenceValue is not initialized yet. Create one
    Type *memFenceType = BarrierFunc->getFunctionType()->getParamType(0);
    LocalMemFenceValue = ConstantInt::get(memFenceType, CLK_LOCAL_MEM_FENCE);
  }
  IRBuilder<> Builder(InsertBefore);
  return Builder.CreateCall(BarrierFunc, LocalMemFenceValue, "");
}

Instruction *BarrierUtils::createDummyBarrier(Instruction *InsertBefore) {
  if (!DummyBarrierFunc)
    DummyBarrierFunc = M->getFunction(DUMMY_BARRIER_FUNC_NAME);
  if (!DummyBarrierFunc) {
    // Module has no Dummy barrier declaration
    // Create one
    Type *Result = Type::getVoidTy(M->getContext());
    DummyBarrierFunc = CompilationUtils::createFunctionDeclaration(
        DUMMY_BARRIER_FUNC_NAME, Result, {}, M);
  }
  return CallInst::Create(DummyBarrierFunc, "", InsertBefore);
}

bool BarrierUtils::isDummyBarrierCall(Instruction *CallInstr) {
  assert(CallInstr && "Instruction should not BE NULL!");
  initializeSyncData();
  return DummyBarriers.count(CallInstr);
}

bool BarrierUtils::isBarrierCall(Instruction *CallInstr) {
  assert(CallInstr && "Instruction should not BE NULL!");
  initializeSyncData();
  return Barriers.count(CallInstr);
}

Instruction *BarrierUtils::createGetSpecialBuffer(Instruction *InsertBefore) {
  if (!GetSpecialBufferFunc) {
    // get_special_buffer() function is not initialized yet
    // There should not BE get_special_buffer function declaration in the
    // module
    assert(!M->getFunction(CompilationUtils::nameSpecialBuffer()) &&
           "get_special_buffer() instruction is origanlity declared by the "
           "module!!!");

    // Create one
    Type *Result =
        PointerType::get(SpecialBufferValueTy, SPECIAL_BUFFER_ADDR_SPACE);
    GetSpecialBufferFunc = CompilationUtils::createFunctionDeclaration(
        CompilationUtils::nameSpecialBuffer(), Result, {}, M);
    CompilationUtils::SetFunctionAttributeReadNone(GetSpecialBufferFunc);
  }
  return CallInst::Create(GetSpecialBufferFunc, "pSB", InsertBefore);
}

CompilationUtils::InstVec &BarrierUtils::getAllGetLocalId() {
  if (!LIDInitialized) {
    GetLIDInstructions.clear();
    auto CIs = CompilationUtils::getCallInstUsersOfFunc(
        *M, CompilationUtils::mangledGetLID());
    GetLIDInstructions.assign(CIs.begin(), CIs.end());
    LIDInitialized = true;
  }
  return GetLIDInstructions;
}

CompilationUtils::InstVec &BarrierUtils::getAllGetGlobalId() {
  if (!GIDInitialized) {
    GetGIDInstructions.clear();
    auto CIs = CompilationUtils::getCallInstUsersOfFunc(
        *M, CompilationUtils::mangledGetGID());
    GetGIDInstructions.assign(CIs.begin(), CIs.end());
    GIDInitialized = true;
  }
  return GetGIDInstructions;
}

Instruction *BarrierUtils::createGetBaseGlobalId(Value *Dim,
                                                 Instruction *InsertBefore) {
  StringRef FuncName = CompilationUtils::nameGetBaseGID();
  if (!GetBaseGIDFunc) {
    // Get existing get_global_id function
    GetBaseGIDFunc = M->getFunction(FuncName);
  }
  if (!GetBaseGIDFunc) {
    // Create one
    Type *Result = IntegerType::get(M->getContext(), UISizeT);
    Type *FuncArgTys[] = {IntegerType::get(M->getContext(), 32)};
    GetBaseGIDFunc = CompilationUtils::createFunctionDeclaration(
        FuncName, Result, FuncArgTys, M);
    CompilationUtils::SetFunctionAttributeReadNone(GetBaseGIDFunc);
  }
  return CallInst::Create(
      GetBaseGIDFunc, Dim,
      CompilationUtils::AppendWithDimension("BaseGlobalId_", Dim),
      InsertBefore);
}

Instruction *BarrierUtils::createGetLocalId(unsigned Dim, IRBuilderBase &B) {
  const std::string strLID = CompilationUtils::mangledGetLID();
  if (!GetLIDFunc) {
    // Get existing get_local_id function
    GetLIDFunc = M->getFunction(strLID);
  }
  if (!GetLIDFunc) {
    // Create one
    Type *Result = IntegerType::get(M->getContext(), UISizeT);
    Type *FuncArgTys[] = {IntegerType::get(M->getContext(), 32)};
    GetLIDFunc = CompilationUtils::createFunctionDeclaration(strLID, Result,
                                                             FuncArgTys, M);
    CompilationUtils::SetFunctionAttributeReadNone(GetLIDFunc);
  }
  Type *uintType = IntegerType::get(M->getContext(), 32);
  Value *constDim = ConstantInt::get(uintType, Dim, false);
  return B.CreateCall(GetLIDFunc, constDim,
                      CompilationUtils::AppendWithDimension("LocalID_", Dim));
}

Instruction *BarrierUtils::createGetGlobalId(unsigned Dim, IRBuilderBase &B) {
  const std::string strGID = CompilationUtils::mangledGetGID();
  if (!GetGIDFunc) {
    // Get existing get_global_id function
    GetGIDFunc = M->getFunction(strGID);
  }
  if (!GetGIDFunc) {
    // Create one
    Type *Result = IntegerType::get(M->getContext(), UISizeT);
    Type *FuncArgTys[] = {IntegerType::get(M->getContext(), 32)};
    GetGIDFunc = CompilationUtils::createFunctionDeclaration(strGID, Result,
                                                             FuncArgTys, M);
    CompilationUtils::SetFunctionAttributeReadNone(GetGIDFunc);
  }
  Type *uintType = IntegerType::get(M->getContext(), 32);
  Value *constDim = ConstantInt::get(uintType, Dim, false);
  return B.CreateCall(GetGIDFunc, constDim,
                      CompilationUtils::AppendWithDimension("GlobalID_", Dim));
}

Value *BarrierUtils::createGetLocalIdLinearResult(IRBuilderBase &B) {
  // result = (get_local_id(2) * get_local_size(1) + get_local_id(1))
  // * get_local_size(0)
  // + get_local_id(0)
  Value *LocalId0 = createGetLocalId(0, B);
  Value *LocalId1 = createGetLocalId(1, B);
  Value *LocalId2 = createGetLocalId(2, B);

  Value *LocalSize1 = createGetLocalSize(1, &*(B.GetInsertPoint()));
  Value *LocalSize0 = createGetLocalSize(0, &*(B.GetInsertPoint()));

  // (get_local_id(2) * get_local_size(1)
  Value *Op0 = B.CreateMul(LocalId2, LocalSize1, "llid.p0", /*HasNUW=*/true,
                           /*HasNSW=*/true);

  //  + get_local_id(1))
  Value *Op1 =
      B.CreateAdd(Op0, LocalId1, "llid.p1", /*HasNUW=*/true, /*HasNSW=*/true);
  // * get_local_size(0)
  Value *Op2 =
      B.CreateMul(Op1, LocalSize0, "llid.p2", /*HasNUW=*/true, /*HasNSW=*/true);
  // + get_local_id(0)
  return B.CreateAdd(Op2, LocalId0, "llid.res", /*HasNUW=*/true,
                     /*HasNSW=*/true);
}

Value *BarrierUtils::createGetLocalSizeLinearResult(IRBuilderBase &B) {
  // result = get_local_size(2) * get_local_size(1) * get_local_size(1)
  Value *LocalSize2 = createGetLocalSize(2, &*(B.GetInsertPoint()));
  Value *LocalSize1 = createGetLocalSize(1, &*(B.GetInsertPoint()));
  Value *LocalSize0 = createGetLocalSize(0, &*(B.GetInsertPoint()));

  // get_local_size(2) * get_local_size(1)
  Value *Op0 = B.CreateMul(LocalSize2, LocalSize1, "llsize.p0", /*HasNUW=*/true,
                           /*HasNSW=*/true);
  // * get_local_size(0)
  return B.CreateMul(Op0, LocalSize0, "llsize.p1", /*HasNUW=*/true,
                     /*HasNSW=*/true);
}

CallInst *BarrierUtils::createWorkGroupSortCopyBuiltin(
    Module &M, IRBuilderBase &B, CallInst *WGCallInst, bool ToScratch,
    Value *LLID, Value *LLSize) {
  // Get mangled copy function name
  Function *Callee = WGCallInst->getCalledFunction();
  assert(Callee && "Indirect function call");

  StringRef FuncName = Callee->getName();
  const std::string CopyFuncName =
      CompilationUtils::getWorkGroupSortCopyName(FuncName, ToScratch);

  // Copy builtin params list
  // key-only :  src_ptr, per_item_size(uint), dst_ptr(i8*),
  //             local_id(int), local_size(int), direction(bool)
  // key-value : key_ptr, value_ptr, per_item_size(uint), dst_ptr(i8*),
  //             local_id(int), local_size(int), direction(bool)
  // Sort builtin params list
  // key-only :  src_ptr, per_item_size(uint), dst_ptr(i8*)
  // key-value : key_ptr, value_ptr, per_item_size(uint), dst_ptr(i8*)
  SmallVector<Type *> FuncArgTys;
  SmallVector<Value *> FuncArgValues;
  for (auto &Arg : WGCallInst->args()) {
    // Put sort builtin params to copy call
    if (Arg->getType()->isVectorTy() &&
        Arg->getType()->getScalarType()->isIntegerTy()) {
      // Arg is mask, now assume sort builtin uniform
      continue;
    }
    FuncArgTys.push_back(Arg->getType());
    FuncArgValues.push_back(Arg);
  }
  Type *DirectionType = IntegerType::get(M.getContext(), 1);

  // Get or create copy builtin function
  Function *CopyFunc = M.getFunction(CopyFuncName);
  if (!CopyFunc) {
    // Create new copy function
    Type *VoidResult = B.getVoidTy(); // Gen return value
    // Gen localId param type
    Type *LocalIdType = IntegerType::get(M.getContext(), 64);
    FuncArgTys.push_back(LocalIdType);
    // Gen localSize param type
    Type *LocalSizeType = IntegerType::get(M.getContext(), 64);
    FuncArgTys.push_back(LocalSizeType);
    // Gen direction param type
    FuncArgTys.push_back(DirectionType);
    CopyFunc = CompilationUtils::createFunctionDeclaration(
        CopyFuncName, VoidResult, FuncArgTys, &M);
  }
  // Put other needed params to copy call
  FuncArgValues.push_back(LLID);
  FuncArgValues.push_back(LLSize);
  Value *Direction = ConstantInt::get(DirectionType, ToScratch, false);
  FuncArgValues.push_back(Direction);
  // Create call to copy function
  CallInst *Result = B.CreateCall(CopyFunc, FuncArgValues, "");

  return Result;
}

void BarrierUtils::replaceSortSizeWithTotalSize(IRBuilderBase &B,
                                                CallInst *WGCallInst,
                                                unsigned ArgIdx,
                                                Value *LLSize) {
  Value *OldSize = WGCallInst->getArgOperand(ArgIdx);
  Value *Size32 = B.CreateTrunc(LLSize, OldSize->getType(), "llsize32");
  Value *NewSize = B.CreateMul(OldSize, Size32, "sort.size", /*HasNUW=*/true,
                               /*HasNSW=*/true);
  WGCallInst->setArgOperand(ArgIdx, NewSize);
}

CallInst *BarrierUtils::createDeviceBarrierWithWGCount(Value *NumsGroup,
                                                       IRBuilderBase &B) {
  const std::string StrRGBarrier = CompilationUtils::mangledDeviceBarrier();
  if (!DeviceBarrierFunc) {
    // Get existing intel_device_barrier function
    DeviceBarrierFunc = M->getFunction(StrRGBarrier);
  }
  if (!DeviceBarrierFunc) {
    // Create one
    Type *VoidResult = B.getVoidTy();
    Type *FuncArgTys[] = {IntegerType::get(M->getContext(), 32)};
    DeviceBarrierFunc = CompilationUtils::createFunctionDeclaration(
        StrRGBarrier, VoidResult, FuncArgTys, M);
    CompilationUtils::SetFunctionAttributeReadNone(DeviceBarrierFunc);
  }
  Type *NewType = IntegerType::get(M->getContext(), 32);
  Value *NumsGroup32 = B.CreateTrunc(NumsGroup, NewType, "nums32");

  CallInst *NewRgCallInst = B.CreateCall(DeviceBarrierFunc, NumsGroup32);
  return NewRgCallInst;
}

bool BarrierUtils::doesCallModuleFunction(Function *Func) {
  if (!NonInlinedCallsInitialized) {
    FunctionsWithNonInlinedCalls.clear();
    // Collect all functions with non inlined calls, i.e.
    // functions that calls other functions from this module
    for (auto &CalledFunc : *M) {
      if (CalledFunc.isDeclaration())
        continue;
      for (auto *U : CalledFunc.users()) {
        CallInst *CI = dyn_cast<CallInst>(U);
        // The user may be a global variable.
        if (!CI)
          continue;
        Function *CallingFunc = CI->getCaller();
        FunctionsWithNonInlinedCalls.insert(CallingFunc);
      }
    }
    NonInlinedCallsInitialized = true;
  }
  return FunctionsWithNonInlinedCalls.count(Func);
}

void BarrierUtils::initializeSyncData() {
  if (SyncDataInitialized)
    return;

  Barriers.clear();
  DummyBarriers.clear();

  // Find all calls to barrier()
  findAllUsesOfFunc(CompilationUtils::mangledBarrier(), Barriers);
  // Find all calls to work_group_barrier()
  findAllUsesOfFunc(CompilationUtils::mangledWGBarrier(
                        CompilationUtils::BarrierType::NoScope),
                    Barriers);
  findAllUsesOfFunc(CompilationUtils::mangledWGBarrier(
                        CompilationUtils::BarrierType::WithScope),
                    Barriers);
  // Find all calls to dummyBarrier()
  findAllUsesOfFunc(DUMMY_BARRIER_FUNC_NAME, DummyBarriers);

  SyncDataInitialized = true;
}

void BarrierUtils::findAllUsesOfFunc(const StringRef Name, InstSet &UsesSet) {
  Function *Func = M->getFunction(Name);
  if (!Func)
    return;
  for (User *U : Func->users()) {
    CallInst *Call = cast<CallInst>(U);
    UsesSet.insert(Call);
  }
}

Instruction *BarrierUtils::createGetLocalSize(unsigned Dim,
                                              Instruction *InsertBefore) {
  // Callee's declaration: size_t get_local_size(uint Dimindx);
  const std::string strGID = CompilationUtils::mangledGetLocalSize();
  if (!GetLocalSizeFunc) {
    // Get existing get_local_size function
    GetLocalSizeFunc = M->getFunction(strGID);
  }
  if (!GetLocalSizeFunc) {
    // Create one
    Type *Result = SizetTy;
    Type *FuncArgTys[] = {I32Ty};
    GetLocalSizeFunc = CompilationUtils::createFunctionDeclaration(
        strGID, Result, FuncArgTys, M);
    CompilationUtils::SetFunctionAttributeReadNone(GetLocalSizeFunc);
  }
  Value *ConstDim = ConstantInt::get(I32Ty, Dim, false);
  return CallInst::Create(
      GetLocalSizeFunc, ConstDim,
      CompilationUtils::AppendWithDimension("LocalSize_", Dim), InsertBefore);
}

bool BarrierUtils::isCrossedByBarrier(const InstSet &SyncInstructions,
                                      BasicBlock *ValUsageBB,
                                      BasicBlock *ValBB) {
  if (ValUsageBB == ValBB) {
    // This can happen when ValUsage is a PHINode
    return false;
  }

  CompilationUtils::BBSet Predecessors;
  SmallVector<BasicBlock *, 8> BasicBlocksToHandle;
  BasicBlocksToHandle.push_back(ValUsageBB);

  do {
    BasicBlock *BBToHandle = BasicBlocksToHandle.pop_back_val();
    Instruction *FirstInst = &*(BBToHandle->begin());
    if (SyncInstructions.count(FirstInst)) {
      // Found a barrier
      return true;
    }
    for (BasicBlock *Pred : predecessors(BBToHandle)) {
      if (Pred == ValBB) {
        // Reached ValBB stop recursive at this direction!
        continue;
      }

      if (!Predecessors.insert(Pred)) {
        // Pred was already added to Predecessors
        continue;
      }
      // Also add it to the BasicBlocksToHandle to calculate its Predecessors
      BasicBlocksToHandle.push_back(Pred);
    }
  } while (!BasicBlocksToHandle.empty());
  return false;
}

bool BarrierUtils::isBarrierOrDummyBarrierCall(Value *Val) {
  static std::string Barriers[] = {
      CompilationUtils::mangledBarrier(),
      CompilationUtils::mangledWGBarrier(
          CompilationUtils::BarrierType::NoScope),
      CompilationUtils::mangledWGBarrier(
          CompilationUtils::BarrierType::WithScope),
      DUMMY_BARRIER_FUNC_NAME,
  };
  CallInst *CI;
  Function *F;
  if (!(CI = dyn_cast<CallInst>(Val)) || !(F = CI->getCalledFunction()))
    return false;
  StringRef FName = F->getName();
  return llvm::any_of(Barriers, [&](std::string &B) { return FName == B; });
}

inst_range BarrierUtils::findDummyRegion(Function &F) {
  // Dummy region would only exist at the beginning of a function,
  // so we do greedy search from the First instruction.
  inst_iterator FirstIt;
  for (inst_iterator It = inst_begin(F), End = inst_end(F); It != End; ++It) {
    auto *Inst = &*It;
    if (isBarrierCall(Inst))
      return make_range(inst_iterator(), inst_iterator());
    if (isDummyBarrierCall(Inst)) {
      FirstIt = It;
      for (inst_iterator It2 = ++It; It2 != End; ++It2) {
        Inst = &*It2;
        if (isBarrierCall(Inst))
          return make_range(inst_iterator(), inst_iterator());
        if (isDummyBarrierCall(Inst))
          return make_range(FirstIt, ++It2);
      }
    }
  }
  return make_range(inst_iterator(), inst_iterator());
}

//===- BarrierUtils.cpp - Barrier Utils -----------------------------------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/BarrierUtils.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ModRef.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

using namespace llvm;
using namespace DPCPPKernelMetadataAPI;

BarrierUtils::BarrierUtils()
    : M(nullptr), UISizeT(0), SizetTy(nullptr), I32Ty(nullptr) {
  clean();
}

void BarrierUtils::init(Module *M) {
  assert(M && "Trying to initialize BarrierUtils with NULL module");
  this->M = M;

  // Get size of size_t in bits from the module
  clean();
  UISizeT = M->getDataLayout().getPointerSizeInBits(0);
  assert(UISizeT == 32 || UISizeT == 64);
  I32Ty = Type::getInt32Ty(M->getContext());
  SizetTy = IntegerType::get(M->getContext(), UISizeT);
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

BasicBlock *BarrierUtils::findBasicBlockOfUsageInst(Value *Val,
                                                    Instruction *UserInst) {
  if (!isa<PHINode>(UserInst))
    return UserInst->getParent();

  // Usage is a PHINode, find previous basic block according to Val
  PHINode *PhiNode = cast<PHINode>(UserInst);
  BasicBlock *PrevBB = nullptr;
  for (BasicBlock *BB : predecessors(PhiNode->getParent())) {
    Value *PHINodeVal = PhiNode->getIncomingValueForBlock(BB);
    if (PHINodeVal == Val) {
      // BB is the previous basic block
      assert(!PrevBB && "PHINode is using Val twice!");
      PrevBB = BB;
    }
  }
  assert(PrevBB && "Failed to find previous basic block!");
  return PrevBB;
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

CompilationUtils::InstVec BarrierUtils::getWGCallInstructions(CALL_BI_TYPE Ty) {
  CompilationUtils::InstVec WGcallInstructions;

  // Scan external function definitions in the module
  for (auto &F : *M) {
    if (!F.isDeclaration()) {
      // Built-in functions are assumed to be declarations.
      continue;
    }
    StringRef FName = F.getName();
    if ((CALL_BI_TYPE_WG == Ty &&
         CompilationUtils::isWorkGroupBuiltin(FName)) ||
        (CALL_BI_TYPE_WG_ASYNC_OR_PIPE == Ty &&
         CompilationUtils::isWorkGroupAsyncOrPipeBuiltin(FName, *M))) {
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
  for (auto Func : Kernels) {
    auto kimd = KernelInternalMetadataAPI(Func);
    if (kimd.NoBarrierPath.hasValue() && kimd.NoBarrierPath.get())
      continue;

    // Currently no check if kernel already added to the list!
    KernelsWithBarrier.push_back(Func);
  }
  KernelFunctions = getAllKernelsAndVectorizedCounterparts(KernelsWithBarrier);

  // collect functions to process
  auto TodoList = getAllKernelsAndVectorizedCounterparts(Kernels.getList());

  for (auto Func : TodoList) {
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
    BarrierFunc =
        createFunctionDeclaration(CompilationUtils::mangledWGBarrier(
                                      CompilationUtils::BarrierType::NoScope),
                                  Result, FuncArgTys);
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
    DummyBarrierFunc =
        createFunctionDeclaration(DUMMY_BARRIER_FUNC_NAME, Result, {});
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
    assert(!M->getFunction(GET_SPECIAL_BUFFER) &&
           "get_special_buffer() instruction is origanlity declared by the "
           "module!!!");

    // Create one
    Type *Result = PointerType::get(IntegerType::get(M->getContext(), 8),
                                    SPECIAL_BUFFER_ADDR_SPACE);
    GetSpecialBufferFunc =
        createFunctionDeclaration(GET_SPECIAL_BUFFER, Result, {});
    SetFunctionAttributeReadNone(GetSpecialBufferFunc);
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
  const std::string FuncName = GET_BASE_GID;
  if (!GetBaseGIDFunc) {
    // Get existing get_global_id function
    GetBaseGIDFunc = M->getFunction(FuncName);
  }
  if (!GetBaseGIDFunc) {
    // Create one
    Type *Result = IntegerType::get(M->getContext(), UISizeT);
    Type *FuncArgTys[] = {IntegerType::get(M->getContext(), 32)};
    GetBaseGIDFunc = createFunctionDeclaration(FuncName, Result, FuncArgTys);
    SetFunctionAttributeReadNone(GetBaseGIDFunc);
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
    GetLIDFunc = createFunctionDeclaration(strLID, Result, FuncArgTys);
    SetFunctionAttributeReadNone(GetLIDFunc);
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
    GetGIDFunc = createFunctionDeclaration(strGID, Result, FuncArgTys);
    SetFunctionAttributeReadNone(GetGIDFunc);
  }
  Type *uintType = IntegerType::get(M->getContext(), 32);
  Value *constDim = ConstantInt::get(uintType, Dim, false);
  return B.CreateCall(GetGIDFunc, constDim,
                      CompilationUtils::AppendWithDimension("GlobalID_", Dim));
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
    GetLocalSizeFunc = createFunctionDeclaration(strGID, Result, FuncArgTys);
    SetFunctionAttributeReadNone(GetLocalSizeFunc);
  }
  Value *ConstDim = ConstantInt::get(I32Ty, Dim, false);
  return CallInst::Create(
      GetLocalSizeFunc, ConstDim,
      CompilationUtils::AppendWithDimension("LocalSize_", Dim), InsertBefore);
}

Function *BarrierUtils::createFunctionDeclaration(const llvm::Twine &Name,
                                                  Type *Result,
                                                  ArrayRef<Type *> FuncArgTys) {
  FunctionType *FuncTy = FunctionType::get(
      /*Result=*/Result,
      /*Params=*/FuncArgTys,
      /*isVarArg=*/false);

  assert(FuncTy && "Failed to create new function type");

  Function *NewFunc = Function::Create(
      /*Type=*/FuncTy,
      /*Linkage=*/GlobalValue::ExternalLinkage,
      /*Name=*/Name, M); //(external, no body)
  assert(NewFunc && "Failed to create new function declaration");
  NewFunc->setCallingConv(CallingConv::C);
  return NewFunc;
}

void BarrierUtils::SetFunctionAttributeReadNone(Function *Func) {
  AttrBuilder attBuilder(Func->getContext());
  attBuilder.addAttribute(Attribute::NoUnwind); /* .addAttribute(Attribute::UWTable) */
  attBuilder.addMemoryAttr(llvm::MemoryEffects::none());
  auto FuncFactorialPAL = AttributeList::get(
      Func->getContext(), AttributeList::FunctionIndex, attBuilder);
  Func->setAttributes(FuncFactorialPAL);
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

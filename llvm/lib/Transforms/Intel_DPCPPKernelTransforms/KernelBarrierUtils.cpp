//===- KernelBarrierUtils.cpp - Barrier Utils -----------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/KernelBarrierUtils.h"

#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

#include <vector>

namespace llvm {

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
  if (!isa<PHINode>(UserInst)) {
    // Not PHINode, return usage instruction basic block
    return UserInst->getParent();
  }
  // Usage is a PHINode, find previous basic block according to Val
  PHINode *PhiNode = cast<PHINode>(UserInst);
  BasicBlock *PrevBB = nullptr;
  for (pred_iterator BI = pred_begin(PhiNode->getParent()),
                     BE = pred_end(PhiNode->getParent());
       BI != BE; ++BI) {
    BasicBlock *BB = *BI;
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
  // Initialize sync data if it is not done yet
  initializeSyncData();

  if (!isa<CallInst>(Inst)) {
    // Not a call instruction, cannot BE a synchronize instruction
    return SyncType::None;
  }
  if (Barriers.count(Inst)) {
    // It is a barrier instruction
    return SyncType::Barrier;
  }
  if (DummyBarriers.count(Inst)) {
    // It is a dummyBarrier instruction
    return SyncType::DummyBarrier;
  }
  return SyncType::None;
}

SyncType BarrierUtils::getSyncType(BasicBlock *BB) {
  return getSyncType(&*BB->begin());
}

InstVector &BarrierUtils::getAllSynchronizeInstructions() {
  // Initialize sync data if it is not done yet
  initializeSyncData();

  // Clear old collected data!
  SyncInstructions.clear();

  // Insert all barrier instructions
  for (InstSet::iterator II = Barriers.begin(), IE = Barriers.end(); II != IE;
       ++II) {
    SyncInstructions.push_back(*II);
  }
  // Insert all dummyBarrier instructions
  for (InstSet::iterator II = DummyBarriers.begin(), IE = DummyBarriers.end();
       II != IE; ++II) {
    SyncInstructions.push_back(*II);
  }

  return SyncInstructions;
}

InstVector &BarrierUtils::getWGCallInstructions(CALL_BI_TYPE Ty) {

  // Clear old collected data
  WGcallInstructions.clear();

  // Scan external function definitions in the module
  for (Module::iterator FI = M->begin(), FE = M->end(); FI != FE; ++FI) {
    Function *Func = &*FI;
    if (!Func->isDeclaration()) {
      // Built-in functions assumed to BE declarations at this point.
      continue;
    }
    std::string FuncName = Func->getName().str();
    if ((CALL_BI_TYPE_WG == Ty &&
         DPCPPKernelCompilationUtils::isWorkGroupBuiltin(FuncName)) ||
        (CALL_BI_TYPE_WG_ASYNC_OR_PIPE == Ty &&
         DPCPPKernelCompilationUtils::isWorkGroupAsyncOrPipeBuiltin(FuncName,
                                                                    *M))) {
      // Module contains declaration of a WG function built-in, FIx its
      // usages.
      Function::user_iterator ui = Func->user_begin();
      Function::user_iterator ue = Func->user_end();
      for (; ui != ue; ++ui) {
        CallInst *CI = dyn_cast<CallInst>(*ui);
        if (!CI) {
          assert(false &&
                 "usage of work-group built-in is not a call instruction!");
          continue;
        }
        // Found a call instruction to work-group built-in, collect it.
        WGcallInstructions.push_back(CI);
      }
    }
  }

  return WGcallInstructions;
}

FuncSet &BarrierUtils::getAllFunctionsWithSynchronization() {
  // Initialize SyncInstructions
  getAllSynchronizeInstructions();

  // Clear old collected data!
  SyncFunctions.clear();

  for (InstVector::iterator II = SyncInstructions.begin(),
                            IE = SyncInstructions.end();
       II != IE; ++II) {
    SyncFunctions.insert((*II)->getFunction());
  }
  return SyncFunctions;
}

FuncSet BarrierUtils::getRecursiveFunctionsWithSync() {
  FuncSet &SyncFunctions = getAllFunctionsWithSynchronization();
  FuncSet RecursiveFunctions;
  for (Function *F : SyncFunctions) {
    auto FMD = FunctionMetadataAPI(F);
    if (FMD.RecursiveCall.hasValue() && FMD.RecursiveCall.get())
      RecursiveFunctions.insert(F);
  }
  return RecursiveFunctions;
}

FuncVector BarrierUtils::getAllKernelsAndVectorizedCounterparts(
    const SmallVectorImpl<Function *> &KernelList) {
  FuncVector Result;

  for (auto *F : KernelList) {
    Result.push_back(F);
    auto VectorizedKernelMetadata =
        KernelInternalMetadataAPI(F).VectorizedKernel;
    if (VectorizedKernelMetadata.hasValue() && VectorizedKernelMetadata.get())
      Result.push_back(VectorizedKernelMetadata.get());
  }

  // rely on move ctor.
  return Result;
}

FuncVector &BarrierUtils::getAllKernelsWithBarrier() {
  auto Kernels = KernelList(M);

  // Clear old collected data!
  KernelFunctions.clear();
  if (Kernels.empty()) {
    return KernelFunctions;
  }

  // Get the kernels using the barrier for work group loops.
  SmallVector<Function *, 4> KernelsWithBarrier;
  for (auto Func : Kernels) {
    auto kimd = KernelInternalMetadataAPI(Func);
    // Need to check if NoBarrierPath Value exists, it is not guaranteed that
    // KernelAnalysisPass is running in all scenarios.
    if (kimd.NoBarrierPath.hasValue() && kimd.NoBarrierPath.get()) {
      // Kernel that should not be handled in Barrier path, skip it.
      continue;
    }
    // Add kernel to the list
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
    // TODO: Maybe we need to rename KernelInternalMetadataAPI.
    auto FIMD = KernelInternalMetadataAPI(const_cast<Function *>(F));
    return FIMD.VectorizedWidth.hasValue() ? FIMD.VectorizedWidth.get() : 1;
  }
  return I->second;
}

Instruction *BarrierUtils::createBarrier(Instruction *InsertBefore) {
  if (!BarrierFunc) {
    // Barrier function is not initialized yet
    // Check if there is a declaration in the module
    BarrierFunc = M->getFunction(DPCPPKernelCompilationUtils::mangledWGBarrier(
        DPCPPKernelCompilationUtils::BarrierType::NoScope));
  }
  if (!BarrierFunc) {
    // Module has no barrier declaration. Create one
    Type *Result = Type::getVoidTy(M->getContext());
    std::vector<Type *> FuncTyArgs;
    FuncTyArgs.push_back(IntegerType::get(M->getContext(), 32));
    BarrierFunc = createFunctionDeclaration(
        DPCPPKernelCompilationUtils::mangledWGBarrier(
            DPCPPKernelCompilationUtils::BarrierType::NoScope),
        Result, FuncTyArgs);
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
  if (!DummyBarrierFunc) {
    // Dummy Barrier function is not initialized yet
    // Check if there is a declaration in the module
    DummyBarrierFunc = M->getFunction(DUMMY_BARRIER_FUNC_NAME);
  }
  if (!DummyBarrierFunc) {
    // Module has no Dummy barrier declaration
    // Create one
    Type *Result = Type::getVoidTy(M->getContext());
    std::vector<Type *> FuncTyArgs;
    DummyBarrierFunc =
        createFunctionDeclaration(DUMMY_BARRIER_FUNC_NAME, Result, FuncTyArgs);
  }
  return CallInst::Create(DummyBarrierFunc, "", InsertBefore);
}

bool BarrierUtils::isDummyBarrierCall(Instruction *CallInstr) {
  assert(CallInstr && "Instruction should not BE NULL!");
  // Initialize sync data if it is not done yet
  initializeSyncData();
  return DummyBarriers.count(CallInstr);
}

bool BarrierUtils::isBarrierCall(Instruction *CallInstr) {
  assert(CallInstr && "Instruction should not BE NULL!");
  // Initialize sync data if it is not done yet
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
    std::vector<Type *> FuncTyArgs;
    GetSpecialBufferFunc =
        createFunctionDeclaration(GET_SPECIAL_BUFFER, Result, FuncTyArgs);
    SetFunctionAttributeReadNone(GetSpecialBufferFunc);
  }
  return CallInst::Create(GetSpecialBufferFunc, "pSB", InsertBefore);
}

InstVector &BarrierUtils::getAllGetLocalId() {
  if (!LIDInitialized) {
    GetLIDInstructions.clear();
    Function *Func =
        M->getFunction(DPCPPKernelCompilationUtils::mangledGetLID());
    if (Func) {
      for (Value::user_iterator ui = Func->user_begin(), ue = Func->user_end();
           ui != ue; ++ui) {
        CallInst *InstCall = dyn_cast<CallInst>(*ui);
        assert(InstCall &&
               "Something other than CallInst is using get_local_id function!");
        GetLIDInstructions.push_back(InstCall);
      }
    }
    LIDInitialized = true;
  }
  return GetLIDInstructions;
}

InstVector &BarrierUtils::getAllGetGlobalId() {
  if (!GIDInitialized) {
    GetGIDInstructions.clear();
    Function *Func =
        M->getFunction(DPCPPKernelCompilationUtils::mangledGetGID());
    if (Func) {
      for (Value::user_iterator ui = Func->user_begin(), ue = Func->user_end();
           ui != ue; ++ui) {
        CallInst *InstCall = dyn_cast<CallInst>(*ui);
        assert(InstCall && "Something other than CallInst is using "
                           "get_globalal_id function!");
        GetGIDInstructions.push_back(InstCall);
      }
    }
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
    std::vector<Type *> FuncTyArgs;
    FuncTyArgs.push_back(IntegerType::get(M->getContext(), 32));
    GetBaseGIDFunc = createFunctionDeclaration(FuncName, Result, FuncTyArgs);
    SetFunctionAttributeReadNone(GetBaseGIDFunc);
  }
  return CallInst::Create(
      GetBaseGIDFunc, Dim,
      DPCPPKernelCompilationUtils::AppendWithDimension("BaseGlobalId_", Dim),
      InsertBefore);
}

Instruction *BarrierUtils::createGetLocalId(unsigned Dim, IRBuilderBase &B) {
  const std::string strLID = DPCPPKernelCompilationUtils::mangledGetLID();
  if (!GetLIDFunc) {
    // Get existing get_local_id function
    GetLIDFunc = M->getFunction(strLID);
  }
  if (!GetLIDFunc) {
    // Create one
    Type *Result = IntegerType::get(M->getContext(), UISizeT);
    std::vector<Type *> FuncTyArgs;
    FuncTyArgs.push_back(IntegerType::get(M->getContext(), 32));
    GetLIDFunc = createFunctionDeclaration(strLID, Result, FuncTyArgs);
    SetFunctionAttributeReadNone(GetLIDFunc);
  }
  Type *uintType = IntegerType::get(M->getContext(), 32);
  Value *constDim = ConstantInt::get(uintType, Dim, false);
  return B.CreateCall(
      GetLIDFunc, constDim,
      DPCPPKernelCompilationUtils::AppendWithDimension("LocalID_", Dim));
}

Instruction *BarrierUtils::createGetGlobalId(unsigned Dim, IRBuilderBase &B) {
  const std::string strGID = DPCPPKernelCompilationUtils::mangledGetGID();
  if (!GetGIDFunc) {
    // Get existing get_global_id function
    GetGIDFunc = M->getFunction(strGID);
  }
  if (!GetGIDFunc) {
    // Create one
    Type *Result = IntegerType::get(M->getContext(), UISizeT);
    std::vector<Type *> FuncTyArgs;
    FuncTyArgs.push_back(IntegerType::get(M->getContext(), 32));
    GetGIDFunc = createFunctionDeclaration(strGID, Result, FuncTyArgs);
    SetFunctionAttributeReadNone(GetGIDFunc);
  }
  Type *uintType = IntegerType::get(M->getContext(), 32);
  Value *constDim = ConstantInt::get(uintType, Dim, false);
  return B.CreateCall(
      GetGIDFunc, constDim,
      DPCPPKernelCompilationUtils::AppendWithDimension("GlobalID_", Dim));
}

bool BarrierUtils::doesCallModuleFunction(Function *Func) {
  if (!NonInlinedCallsInitialized) {
    FunctionsWithNonInlinedCalls.clear();
    // Collect all functions with non inlined calls, i.e.
    // functions that calls other functions from this module
    for (Module::iterator FI = M->begin(), FE = M->end(); FI != FE; ++FI) {
      Function *CalledFunc = &*FI;
      if (CalledFunc->isDeclaration()) {
        // It is not an internal function, only delaration
        continue;
      }
      for (Value::user_iterator UI = CalledFunc->user_begin(),
                                UE = CalledFunc->user_end();
           UI != UE; ++UI) {
        CallInst *CI = dyn_cast<CallInst>(*UI);
        // usage of Func can BE a global variable!
        if (!CI) {
          // usage of Func is not a CallInst
          continue;
        }
        Function *CallingFunc = CI->getCaller();
        FunctionsWithNonInlinedCalls.insert(CallingFunc);
      }
    }
    NonInlinedCallsInitialized = true;
  }
  return FunctionsWithNonInlinedCalls.count(Func);
}

void BarrierUtils::initializeSyncData() {
  if (SyncDataInitialized) {
    // Sync data already initialized
    return;
  }

  // Clear old collected data!
  Barriers.clear();
  DummyBarriers.clear();

  // Find all calls to barrier()
  findAllUsesOfFunc(DPCPPKernelCompilationUtils::mangledBarrier(), Barriers);
  // Find all calls to work_group_barrier()
  findAllUsesOfFunc(DPCPPKernelCompilationUtils::mangledWGBarrier(
                        DPCPPKernelCompilationUtils::BarrierType::NoScope),
                    Barriers);
  findAllUsesOfFunc(DPCPPKernelCompilationUtils::mangledWGBarrier(
                        DPCPPKernelCompilationUtils::BarrierType::WithScope),
                    Barriers);
  // Find all calls to dummyBarrier()
  findAllUsesOfFunc(DUMMY_BARRIER_FUNC_NAME, DummyBarriers);

  SyncDataInitialized = true;
}

void BarrierUtils::findAllUsesOfFunc(const llvm::StringRef &Name,
                                     InstSet &UsesSet) {
  // Check if given function name is declared in the module
  Function *Func = M->getFunction(Name);
  if (!Func) {
    // Function is not declared
    return;
  }
  // Find all calls to given function name
  for (Value::user_iterator ui = Func->user_begin(), ue = Func->user_end();
       ui != ue; ++ui) {
    CallInst *Call = dyn_cast<CallInst>(*ui);
    assert(Call && "Something other than CallInst is using function!");
    // Add the call instruction into uses set
    UsesSet.insert(Call);
  }
}

Instruction *BarrierUtils::createGetLocalSize(unsigned Dim,
                                              Instruction *InsertBefore) {
  // Callee's declaration: size_t get_local_size(uint Dimindx);
  const std::string strGID = DPCPPKernelCompilationUtils::mangledGetLocalSize();
  if (!GetLocalSizeFunc) {
    // Get existing get_local_size function
    GetLocalSizeFunc = M->getFunction(strGID);
  }
  if (!GetLocalSizeFunc) {
    // Create one
    Type *Result = SizetTy;
    std::vector<Type *> FuncTyArgs;
    FuncTyArgs.push_back(I32Ty);
    GetLocalSizeFunc = createFunctionDeclaration(strGID, Result, FuncTyArgs);
    SetFunctionAttributeReadNone(GetLocalSizeFunc);
  }
  Value *ConstDim = ConstantInt::get(I32Ty, Dim, false);
  return CallInst::Create(
      GetLocalSizeFunc, ConstDim,
      DPCPPKernelCompilationUtils::AppendWithDimension("LocalSize_", Dim),
      InsertBefore);
}

Function *
BarrierUtils::createFunctionDeclaration(const llvm::Twine &Name, Type *Result,
                                        std::vector<Type *> &FuncTyArgs) {
  FunctionType *FuncTy = FunctionType::get(
      /*Result=*/Result,
      /*Params=*/FuncTyArgs,
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
  AttrBuilder attBuilder;
  attBuilder.addAttribute(Attribute::NoUnwind)
      .addAttribute(
          Attribute::ReadNone) /* .addAttribute(Attribute::UWTable) */;
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

  BasicBlockSet Predecessors;
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
  // so we do greedy search from the FIrst instruction.
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
} // namespace llvm

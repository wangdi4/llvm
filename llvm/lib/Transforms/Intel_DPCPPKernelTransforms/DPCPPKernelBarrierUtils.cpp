//==--- DPCPPKernelBarrierUtils.cpp - Barrier helper functions - C++ -*-----==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelBarrierUtils.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/StringExtras.h"

namespace llvm {

DPCPPKernelBarrierUtils::DPCPPKernelBarrierUtils()
    : M(nullptr), SizeTSize(0), IsSyncDataInitialized(false), SizeTTy(nullptr),
      I32Ty(nullptr) {
  clean();
}

void DPCPPKernelBarrierUtils::init(Module *M) {
  assert(M && "Trying to initialize BarrierUtils with NULL module");
  this->M = M;

  clean();
  SizeTSize = M->getDataLayout().getPointerSizeInBits(0);
  I32Ty = Type::getInt32Ty(M->getContext());
  SizeTTy = IntegerType::get(M->getContext(), SizeTSize);
}

void DPCPPKernelBarrierUtils::clean() {
  LocalMemFenceValue = nullptr;
  BarrierFunc = nullptr;
  DummyBarrierFunc = nullptr;

  GetSpecialBufferFunc = nullptr;
  GetLocalSizeFunc = nullptr;

  IsSyncDataInitialized = false;

  HasNonInlinedCallsInitialized = false;

  HasLIDInstsInitialized = false;
}

void DPCPPKernelBarrierUtils::initializeSyncData() {
  if (IsSyncDataInitialized) {
    // Sync data already initialized.
    return;
  }

  // Clear old collected data!
  Barriers.clear();
  DummyBarriers.clear();

  // Find all calls to barrier().
  findAllUsesOfFunc(StringRef(BarrierName), Barriers);
  // Find all calls to dummyBarrier().
  findAllUsesOfFunc(StringRef(DummyBarrierName), DummyBarriers);

  IsSyncDataInitialized = true;
  HasLIDInstsInitialized = false;
}

void DPCPPKernelBarrierUtils::findAllUsesOfFunc(StringRef Name,
                                                InstSet &UsesSet) {
  // Check if given function name is declared in the module.
  Function *F = M->getFunction(Name);
  if (!F) {
    // Function is not declared.
    return;
  }
  // Find all calls to given function name.
  for (auto *U : F->users()) {
    CallInst *Call = dyn_cast<CallInst>(U);
    assert(Call && "Something other than CallInst is using function!");
    // Add the call instruction into uses set.
    UsesSet.insert(Call);
  }
}

InstVector &DPCPPKernelBarrierUtils::getAllSyncInstructions() {
  // Initialize sync data if it is not done yet.
  initializeSyncData();

  // Clear old collected data!
  SyncInstructions.clear();

  SyncInstructions.append(Barriers.begin(), Barriers.end());
  SyncInstructions.append(DummyBarriers.begin(), DummyBarriers.end());

  return SyncInstructions;
}

/// Find all functions directly calling sync instructions.
FuncSet &DPCPPKernelBarrierUtils::getAllFunctionsWithSynchronization() {
  // Initialize SyncInstructions.
  getAllSyncInstructions();

  // Clear old collected data!
  SyncFunctions.clear();

  for (auto *I : SyncInstructions) {
    SyncFunctions.insert(I->getFunction());
  }

  return SyncFunctions;
}

SyncType DPCPPKernelBarrierUtils::getSynchronizeType(const Instruction *I) {
  // Initialize sync data if it is not done yet.
  initializeSyncData();

  if (!isa<CallInst>(I)) {
    // Not a call instruction, cannot be a synchronize instruction.
    return SyncTypeNone;
  }
  if (Barriers.count(const_cast<Instruction *>(I))) {
    // It is a barrier instruction.
    return SyncTypeBarrier;
  }
  if (DummyBarriers.count(const_cast<Instruction *>(I))) {
    // It is a dummyBarrier instruction.
    return SyncTypeDummyBarrier;
  }
  return SyncTypeNone;
}

bool DPCPPKernelBarrierUtils::isDummyBarrierCall(const CallInst *CI) {
  assert(CI && "Instruction should not be nullptr!");
  // Initialize sync data if it is not done yet.
  initializeSyncData();
  return DummyBarriers.count(const_cast<CallInst *>(CI));
}

bool DPCPPKernelBarrierUtils::isBarrierCall(const CallInst *CI) {
  assert(CI && "Instruction should not be nullptr!");
  // Initialize sync data if it is not done yet.
  initializeSyncData();
  return Barriers.count(const_cast<CallInst *>(CI));
}

BasicBlock *
DPCPPKernelBarrierUtils::findBasicBlockOfUsageInst(Value *V,
                                                   Instruction *UserInst) {
  if (!isa<PHINode>(UserInst)) {
    // Not PHINode, return usage instruction basic block.
    return UserInst->getParent();
  }
  // Usage is a PHINode, find previous basic block according to V.
  PHINode *PhiNode = cast<PHINode>(UserInst);
  BasicBlock *PrevBB = nullptr;
  for (auto *BB : predecessors(PhiNode->getParent())) {
    Value *PHINodeVal = PhiNode->getIncomingValueForBlock(BB);
    if (PHINodeVal == V) {
      // BB is the previous basic block.
      assert(!PrevBB && "PHINode is using V twice!");
      PrevBB = BB;
    }
  }
  assert(PrevBB && "Failed to find previous basic block!");
  return PrevBB;
}

FuncVector &DPCPPKernelBarrierUtils::getAllKernelsWithBarrier() {
  FuncVector Kernels;

  for (auto &F : *M) {
    if (F.hasFnAttribute("sycl_kernel"))
      Kernels.push_back(&F);
  }

  // Clear old collected data!
  KernelFunctions.clear();
  if (Kernels.empty()) {
    return KernelFunctions;
  }

  // Get the kernels using the barrier for work group loops.
  SmallVector<Function *, 4> KernelsWithBarrier;
  for (auto Kernel : Kernels) {
    // OCL pipeline treats absense of the attribute as if there's a barrier.
    // In DPCPP path we require having this attrbiute on kernels.
    // Need to check if NoBarrierPath Value exists, it is not guaranteed that
    // KernelAnalysisPass is running in all scenarios.
    assert(Kernel->hasFnAttribute(NO_BARRIER_PATH_ATTRNAME) &&
           "DPCPPKernelBarrierUtils: " NO_BARRIER_PATH_ATTRNAME
           " has to be set!");
    StringRef Value =
        Kernel->getFnAttribute(NO_BARRIER_PATH_ATTRNAME).getValueAsString();
    assert((Value == "true" || Value == "false") &&
           "DPCPPKernelBarrierUtils: unexpected " NO_BARRIER_PATH_ATTRNAME
           " value!");
    bool NoBarrierPath = (Value == "true");
    if (NoBarrierPath) {
      // Kernel that should not be handled in Barrier path, skip it.
      continue;
    }
    // Add kernel to the list.
    // Currently no check if kernel already added to the list!
    KernelsWithBarrier.push_back(Kernel);
  }
  KernelFunctions =
      getAllKernelsAndVectorizedCounterparts(KernelsWithBarrier, M);

  // Collect functions to process.
  auto TodoList = getAllKernelsAndVectorizedCounterparts(Kernels, M);

  for (auto *F : TodoList) {
    unsigned int VectWidth = 1;
    if (F->hasFnAttribute("vectorized_width")) {
      bool Res = to_integer(
          F->getFnAttribute("vectorized_width").getValueAsString(), VectWidth);
      // Silence warning to avoid an extra call.
      (void)Res;
      assert(Res && "DPCPPKernelBarrierUtils: vectorized_width has to have a "
                    "numeric value");
    }
    KernelVectorizationWidths[F] = VectWidth;
  }

  return KernelFunctions;
}

Instruction *DPCPPKernelBarrierUtils::createBarrier(Instruction *InsertBefore) {
  if (!BarrierFunc) {
    // Barrier function is not initialized yet,
    // Check if there is a declaration in the module.
    BarrierFunc = M->getFunction(BarrierName);
  }
  if (!BarrierFunc) {
    // Module has no barrier declaration.
    // Create one.
    Type *Result = Type::getVoidTy(M->getContext());
    std::vector<Type *> FuncTyArgs;
    FuncTyArgs.push_back(IntegerType::get(M->getContext(), 32));
    BarrierFunc = createFunctionDeclaration(BarrierName, Result, FuncTyArgs);
    BarrierFunc->setAttributes(BarrierFunc->getAttributes().addAttribute(
        BarrierFunc->getContext(), AttributeList::FunctionIndex,
        Attribute::Convergent));
  }
  if (!LocalMemFenceValue) {
    // LocalMemFenceValue is not initialized yet, create one.
    Type *MemFenceType = BarrierFunc->getFunctionType()->getParamType(0);
    LocalMemFenceValue = ConstantInt::get(MemFenceType, CLK_LOCAL_MEM_FENCE);
  }
  return CallInst::Create(BarrierFunc, LocalMemFenceValue, "", InsertBefore);
}

Instruction *
DPCPPKernelBarrierUtils::createDummyBarrier(Instruction *InsertBefore) {
  if (!DummyBarrierFunc) {
    // Dummy Barrier function is not initialized yet,
    // Check if there is a declaration in the module.
    DummyBarrierFunc = M->getFunction(DummyBarrierName);
  }
  if (!DummyBarrierFunc) {
    // Module has no Dummy barrier declaration, create one.
    Type *Result = Type::getVoidTy(M->getContext());
    std::vector<Type *> FuncTyArgs;
    DummyBarrierFunc =
        createFunctionDeclaration(DummyBarrierName, Result, FuncTyArgs);
  }
  return CallInst::Create(DummyBarrierFunc, "", InsertBefore);
}

FuncVector DPCPPKernelBarrierUtils::getAllKernelsAndVectorizedCounterparts(
    const SmallVectorImpl<Function *> &KernelList, Module *M) {
  FuncVector Result;

  for (auto *F : KernelList) {
    Result.push_back(F);

    // Set the vectorized function if present.
    if (F->hasFnAttribute("vectorized_kernel")) {
      Function *VectKernel = M->getFunction(
          F->getFnAttribute("vectorized_kernel").getValueAsString());
      if (VectKernel)
        Result.push_back(VectKernel);
    }
  }

  // Rely on move ctor.
  return Result;
}

bool DPCPPKernelBarrierUtils::doesCallModuleFunction(Function *F) {
  if (!HasNonInlinedCallsInitialized) {
    FunctionsWithNonInlinedCalls.clear();
    // Collect all functions with non inlined calls, i.e.
    // functions that calls other functions from this module.
    for (auto &CalledFunc : *M) {
      if (CalledFunc.isDeclaration()) {
        // It is not an internal function, only delaration.
        continue;
      }
      for (auto *U : CalledFunc.users()) {
        CallInst *CI = dyn_cast<CallInst>(U);
        // Usage of F can be a global variable!
        if (!CI) {
          // Usage of F is not a CallInst.
          continue;
        }
        Function *CallingFunc = CI->getFunction();
        FunctionsWithNonInlinedCalls.insert(CallingFunc);
      }
    }
    HasNonInlinedCallsInitialized = true;
  }
  return FunctionsWithNonInlinedCalls.count(F);
}

Instruction *
DPCPPKernelBarrierUtils::createGetSpecialBuffer(Instruction *InsertBefore) {
  if (!GetSpecialBufferFunc) {
    // get_special_buffer() function is not initialized yet.
    // There should not be get_special_buffer function declaration in the module
    assert(!M->getFunction(GET_SPECIAL_BUFFER) &&
           "get_special_buffer() instruction is origanlity declared by the "
           "module!!!");

    // Create one.
    Type *Result = PointerType::get(IntegerType::get(M->getContext(), 8),
                                    SPECIAL_BUFFER_ADDR_SPACE);
    std::vector<Type *> FuncTyArgs;
    GetSpecialBufferFunc =
        createFunctionDeclaration(GET_SPECIAL_BUFFER, Result, FuncTyArgs);
    SetFunctionAttributeReadNone(GetSpecialBufferFunc);
  }
  return CallInst::Create(GetSpecialBufferFunc, "pSB", InsertBefore);
}

Function *DPCPPKernelBarrierUtils::createFunctionDeclaration(
    const llvm::Twine &Name, Type *Result, std::vector<Type *> &FuncTyArgs) {
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

Instruction *
DPCPPKernelBarrierUtils::createGetLocalSize(unsigned Dim,
                                            Instruction *InsertBefore) {
  // Callee's declaration: size_t get_local_size(uint dimindx);
  const std::string StrGID = "__builtin_get_local_size";
  if (!GetLocalSizeFunc) {
    // Get existing get_local_size function.
    GetLocalSizeFunc = M->getFunction(StrGID);
  }
  if (!GetLocalSizeFunc) {
    // Create one.
    Type *Result = SizeTTy;
    std::vector<Type *> FuncTyArgs;
    FuncTyArgs.push_back(I32Ty);
    GetLocalSizeFunc = createFunctionDeclaration(StrGID, Result, FuncTyArgs);
    SetFunctionAttributeReadNone(GetLocalSizeFunc);
  }
  Value *ConstDim = ConstantInt::get(I32Ty, Dim, false);
  return CallInst::Create(GetLocalSizeFunc, ConstDim,
                          AppendWithDimension("LocalSize_", Dim), InsertBefore);
}

void DPCPPKernelBarrierUtils::SetFunctionAttributeReadNone(Function *Func) {
  AttrBuilder Builder;
  Builder.addAttribute(Attribute::NoUnwind)
      .addAttribute(
          Attribute::ReadNone) /* .addAttribute(Attribute::UWTable) */;
  auto AttrList = AttributeList::get(Func->getContext(),
                                     AttributeList::FunctionIndex, Builder);
  Func->setAttributes(AttrList);
}

unsigned
DPCPPKernelBarrierUtils::getKernelVectorizationWidth(const Function *F) const {
  FunctionToUnsigned::const_iterator I = KernelVectorizationWidths.find(F);
  if (I == KernelVectorizationWidths.end())
    return 1;
  return I->second;
}

InstVector &DPCPPKernelBarrierUtils::getAllGetLocalId() {
  if (!HasLIDInstsInitialized) {
    GetLIDInstructions.clear();
    Function *Func = M->getFunction("__builtin_get_local_id");
    if (Func) {
      for (auto *U : Func->users()) {
        CallInst *CI = dyn_cast<CallInst>(U);
        assert(CI &&
               "Something other than CallInst is using get_local_id function!");
        GetLIDInstructions.push_back(CI);
      }
    }
    HasLIDInstsInitialized = true;
  }
  return GetLIDInstructions;
}

bool DPCPPKernelBarrierUtils::isCrossedByBarrier(InstSet &SyncInstructions,
                                                 BasicBlock *ValUsageBB,
                                                 BasicBlock *ValBB) {
  if (ValUsageBB == ValBB) {
    // This can happen when pValUsage is a PHINode.
    return false;
  }

  BasicBlockSet Predecessors;
  SmallVector<BasicBlock *, 8> BasicBlocksToHandle;
  BasicBlocksToHandle.push_back(ValUsageBB);

  while (!BasicBlocksToHandle.empty()) {
    BasicBlock *BBToHandle = BasicBlocksToHandle.pop_back_val();
    Instruction *FirstInst = &*(BBToHandle->begin());
    if (SyncInstructions.count(FirstInst)) {
      // Found a barrier.
      return true;
    }
    for (BasicBlock *Pred : predecessors(BBToHandle)) {
      if (Pred == ValBB) {
        // Reached ValBB stop recursive at this direction!
        continue;
      }
      if (Predecessors.count(Pred)) {
        // Pred was already added to Predecessors.
        continue;
      }
      // This is a new predecessor add it to the Predecessors container.
      Predecessors.insert(Pred);
      // Also add it to the BasicBlocksToHandle to calculate its Predecessors.
      BasicBlocksToHandle.push_back(Pred);
    }
  }
  return false;
}

bool DPCPPKernelBarrierUtils::isImplicitGID(AllocaInst *AI) {
  // No implicit GIDs in DPCPP.
  return false;
}

} // namespace llvm

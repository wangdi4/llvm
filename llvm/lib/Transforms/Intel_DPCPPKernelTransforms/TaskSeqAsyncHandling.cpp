// ===--- TaskSeqAsyncHandling.cpp ------------------------------ C++ -*--=== //
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/TaskSeqAsyncHandling.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

#define DEBUG_TYPE "dpcpp-kernel-handle-taskseq-async"

using namespace llvm;
using namespace llvm::DPCPPKernelCompilationUtils;

using FunctionVector = SmallVector<Function *>;

namespace {

// Bulit-in function patterns coming from frontend
constexpr const char *BuiltinCreateTaskSeqPattern = "_Z31__spirv_TaskSequenceCreateINTEL";
constexpr const char *BuiltinReleaseTaskSeqPattern = "_Z32__spirv_TaskSequenceReleaseINTEL";
constexpr const char *BuiltinGetPattern = "_Z28__spirv_TaskSequenceGetINTEL";
constexpr const char *BuiltinAsyncPattern = "_Z30__spirv_TaskSequenceAsyncINTEL";

// Bulit-in function names provided by backend
constexpr const char *BackendCreateTaskSeqName = "__create_task_sequence";
constexpr const char *BackendReleaseTaskSeqName = "__release_task_sequence";
constexpr const char *BackendGetName = "__get";
constexpr const char *BackendAsyncName = "__async";

/// Terms used in variable names:
///
/// - Async Function (AsyncFunc*)
///   Function passed to the task_sequence class and to be called
///   asynchronously.
///
/// - Literal/Block Literal
///   A struct contains parameters and result address of an async
///   function. See its definition in createBlockLiteralTypes.
///
/// - Async Invoke (AsyncInvoke*)
///   A fuction wrapper to extract parameters from block literal, and call the
///   async function, and save the result into the result address.
///
/// - Backend Bulit-in functions (Backend*)
///   Bulit-in functions provided by OpenCL bulit-in libraries.
///
/// - Built-in functions (Bulitin*)
///   Built-in declarations coming from frontend, and handled in this pass.
///
class Impl {
public:
  Impl(Module &M) : M(M), Ctx(M.getContext()), IRB(Ctx), Changed(false) {}

  bool run() {
    findAllAsyncBuiltins();
    createBlockLiteralTypes();
    createAsyncFunctionInvokes();
    generateBuiltinBodies();
    return Changed;
  }

private:
  void findAllAsyncBuiltins();
  void createBlockLiteralTypes();
  void createAsyncFunctionInvokes();
  FunctionCallee getBackendCreateTaskSeq();
  FunctionCallee getBackendReleaseTaskSeq();
  FunctionCallee getBackendGet();
  FunctionCallee getBackendAsync();
  void generateCreateTaskSeqBodies();
  void generateReleaseTaskSeqBodies();
  void generateGetBodies();
  void generateAsyncBodies();

  void generateBuiltinBodies() {
    generateCreateTaskSeqBodies();
    generateReleaseTaskSeqBodies();
    generateGetBodies();
    generateAsyncBodies();
  }

  Function *getCalledAsyncFunc(Function *F) {
    // All users of a F are expected to be same, and all of them are expected
    // to call a same async function. Thus, get AsyncFunc from any of them.
    CallInst *CI = cast<CallInst>(*F->user_begin());
    return cast<Function>(CI->getArgOperand(1)->stripPointerCasts());
  }

  StructType *getLiteralType(Function *F) {
    auto It = LiteralMap.find(F);
    assert(It != LiteralMap.end() && "Cannot find corresponding literal type. "
                                     "Is F a __spirv_TaskSequenceAsync?");
    return It->second;
  }

  static std::string getInovkeName(Function *F) {
    // The suffix is used to identify a block invoke, so don't change it.
    return (F->getName() + "._block_invoke_kernel").str();
  }

  DenseMap<Function *, StructType *> LiteralMap;
  FunctionVector Creates;
  FunctionVector Releases;
  FunctionVector Asyncs;
  FunctionVector Gets;
  Module &M;
  LLVMContext &Ctx;
  IRBuilder<> IRB;
  bool Changed;
};

void Impl::findAllAsyncBuiltins() {
  for (Function &F : M) {
    if (!F.isDeclaration())
      continue;

    StringRef FName = F.getName();
    if (FName.startswith(BuiltinGetPattern))
      Gets.push_back(&F);
    else if (FName.startswith(BuiltinAsyncPattern))
      Asyncs.push_back(&F);
    else if (FName.startswith(BuiltinCreateTaskSeqPattern))
      Creates.push_back(&F);
    else if (FName.startswith(BuiltinReleaseTaskSeqPattern))
      Releases.push_back(&F);
  }
}

void Impl::createBlockLiteralTypes() {
  // struct Literal {
  //   unsigned Size;
  //   unsigned Align;
  //   void *BlockInvoke;
  //   Arguments...;
  //   void *ResultAddr;
  // };
  for (Function *F : Asyncs) {
    auto *AsyncFuncType = cast<FunctionType>(
        F->getFunctionType()->getParamType(1)->getPointerElementType());
    auto *UnsignedTy = Type::getIntNTy(Ctx, sizeof(unsigned) * 8);
    auto *PtrTy = Type::getInt8PtrTy(Ctx);
    SmallVector<Type *> EltTypes;
    EltTypes.reserve(AsyncFuncType->getNumParams() + 4);
    EltTypes.append({UnsignedTy, UnsignedTy, PtrTy});
    // TODO: Reorder parameters to reduce struct size
    EltTypes.append(AsyncFuncType->param_begin(), AsyncFuncType->param_end());
    EltTypes.push_back(PtrTy);
    LiteralMap.insert({F, StructType::get(Ctx, EltTypes)});
  }
}

size_t getRetTypeSizeOfAsyncFunction(Function *F) {
  // The parameter F is task_sequence::__create_task_sequence, but not the
  // async function to call. The async function is the 1st argument of F.
  auto AsyncFuncType = cast<FunctionType>(
      F->getFunctionType()->getParamType(1)->getPointerElementType());
  auto &DL = F->getParent()->getDataLayout();
  auto RetType = AsyncFuncType->getReturnType();
  if (RetType->isVoidTy())
    return 0;
  TypeSize RetTypeSize = DL.getTypeAllocSize(RetType);
  return RetTypeSize.getFixedSize();
}

FunctionCallee Impl::getBackendCreateTaskSeq() {
  // void *__create_task_sequence(size_t return_type_size)
  auto *SizeTTy = Type::getIntNTy(Ctx, sizeof(size_t) * 8);
  auto *RetTy = Type::getInt8PtrTy(Ctx);
  auto *FTy = FunctionType::get(RetTy, {SizeTTy}, false);
  return M.getOrInsertFunction(BackendCreateTaskSeqName, FTy);
}

FunctionCallee Impl::getBackendReleaseTaskSeq() {
  // void __release_task_sequence(void *task_seq)
  auto *PtrTy = Type::getInt8PtrTy(Ctx, ADDRESS_SPACE_GENERIC);
  auto *RetTy = Type::getVoidTy(Ctx);
  auto *FTy = FunctionType::get(RetTy, {PtrTy}, false);
  return M.getOrInsertFunction(BackendReleaseTaskSeqName, FTy);
}

FunctionCallee Impl::getBackendGet() {
  // void *__get(void *task_seq, unsigned capacity)
  auto *PtrTy = Type::getInt8PtrTy(Ctx, ADDRESS_SPACE_GENERIC);
  auto *I32Ty = Type::getInt32Ty(Ctx);
  auto *RetTy = PtrTy;
  auto *FTy = FunctionType::get(RetTy, {PtrTy, I32Ty}, false);
  return M.getOrInsertFunction(BackendGetName, FTy);
}

FunctionCallee Impl::getBackendAsync() {
  // void __async(void *task_seq, unsigned capacity,
  //              void *block_invoke, void *block_literal)
  auto *PtrTy = Type::getInt8PtrTy(Ctx, ADDRESS_SPACE_GENERIC);
  auto *I32Ty = Type::getInt32Ty(Ctx);
  auto *RetTy = Type::getVoidTy(Ctx);
  auto *FTy = FunctionType::get(RetTy, {PtrTy, I32Ty, PtrTy, PtrTy}, false);
  return M.getOrInsertFunction(BackendAsyncName, FTy);
}

void Impl::generateCreateTaskSeqBodies() {
  if (Creates.empty())
    return;

  // size_t __spirv_TaskSequenceCreateINTEL(task_sequence *obj, f_t *f) {
  //   // Parse return_type_size from f_t
  //   void *impl = __create_task_sequence(return_type_size);
  //   return (size_t)impl;
  // }
  FunctionCallee BackendCreateTaskSeq = getBackendCreateTaskSeq();
  auto *RetTypeSizeTy = BackendCreateTaskSeq.getFunctionType()->getParamType(0);
  for (Function *F : Creates) {
    auto Entry = BasicBlock::Create(Ctx);
    Entry->insertInto(F);
    IRB.SetInsertPoint(Entry);
    size_t RetTypeSize = getRetTypeSizeOfAsyncFunction(F);
    auto *RetTypeSizeVal = ConstantInt::get(RetTypeSizeTy, RetTypeSize);
    auto *CI = IRB.CreateCall(BackendCreateTaskSeq, {RetTypeSizeVal});
    auto *RetBC = IRB.CreatePointerCast(CI, F->getReturnType());
    IRB.CreateRet(RetBC);
    F->setLinkage(GlobalValue::InternalLinkage);
  }
  Changed = true;
}

void Impl::generateReleaseTaskSeqBodies() {
  if (Releases.empty())
    return;

  // void __spirv_TaskSequenceReleaseINTEL(task_sequence *obj) {
  //   __release_task_sequence((void *)obj);
  // }
  FunctionCallee BackendReleaseTaskSeq = getBackendReleaseTaskSeq();
  auto *ArgTy = BackendReleaseTaskSeq.getFunctionType()->getParamType(0);
  for (Function *F : Releases) {
    auto Entry = BasicBlock::Create(Ctx);
    Entry->insertInto(F);
    IRB.SetInsertPoint(Entry);
    auto *Cast = IRB.CreatePointerCast(F->getArg(0), ArgTy);
    IRB.CreateCall(BackendReleaseTaskSeq, {Cast});
    IRB.CreateRetVoid();
    F->setLinkage(GlobalValue::InternalLinkage);
  }
  Changed = true;
}

void Impl::generateGetBodies() {
  if (Gets.empty())
    return;

  // ReturnT __spirv_TaskSequenceGetINTEL(task_sequence *obj, f_t *f,
  //                                      size_t id, unsigned capacity) {
  //   void *ret = __get((void *)obj, capacity);
  //   return *(ReturnT *)ret;
  // }
  FunctionCallee BackendGet = getBackendGet();
  auto *Arg0Ty = BackendGet.getFunctionType()->getParamType(0);
  for (Function *F : Gets) {
    auto Entry = BasicBlock::Create(Ctx);
    Entry->insertInto(F);
    IRB.SetInsertPoint(Entry);
    auto *Cast = IRB.CreatePointerCast(F->getArg(0), Arg0Ty);
    auto *Capacity = F->getArg(3);
    auto *CI = IRB.CreateCall(BackendGet, {Cast, Capacity});
    auto *RetTy = F->getReturnType();
    Value *RetVal;
    if (RetTy->isVoidTy())
      RetVal = nullptr;
    else {
      auto *RetValPtr = IRB.CreatePointerCast(CI, RetTy->getPointerTo());
      RetVal = IRB.CreateLoad(RetTy, RetValPtr);
    }
    IRB.CreateRet(RetVal);
    F->setLinkage(GlobalValue::InternalLinkage);
  }
  Changed = true;
}

void Impl::generateAsyncBodies() {
  if (Asyncs.empty())
    return;

  // void __spirv_TaskSequenceAsyncINTEL(
  //     task_sequence *obj, f_t *f, size_t id, unsigned capacity, ...) {
  //   // Get invoke;
  //   // Create literal;
  //   __async((void *)obj, capacity, (void *)invoke, (void *)literal);
  // }
  auto *Int32Ty = Type::getInt32Ty(Ctx);
  auto *Zero = ConstantInt::get(Int32Ty, 0);
  auto *VoidPtrTy = Type::getInt8PtrTy(Ctx, ADDRESS_SPACE_GENERIC);
  FunctionCallee BackendAsync = getBackendAsync();
  for (Function *F : Asyncs) {
    StructType *LiteralType = getLiteralType(F);
    Function *AsyncFunc = getCalledAsyncFunc(F);
    Function *AsyncInvoke = M.getFunction(getInovkeName(AsyncFunc));
    assert(AsyncInvoke &&
           "No invoke function found. Is createAsyncFunctionInvokes called?");

    auto *Entry = BasicBlock::Create(Ctx);
    Entry->insertInto(F);
    IRB.SetInsertPoint(Entry);

    auto *Literal =
        IRB.CreateAlloca(LiteralType, /*ArraySize*/ nullptr, "literal");

    unsigned LiteralIdx = 0;

    auto &DL = M.getDataLayout();
    auto LiteralSize = DL.getTypeStoreSize(LiteralType).getFixedSize();
    auto *SizePtr = IRB.CreateInBoundsGEP(
        LiteralType, Literal, {Zero, ConstantInt::get(Int32Ty, LiteralIdx)},
        "literal.size");
    IRB.CreateStore(
        ConstantInt::get(LiteralType->getElementType(LiteralIdx), LiteralSize),
        SizePtr);
    ++LiteralIdx;

    auto LiteralAlign = DL.getABITypeAlign(LiteralType).value();
    auto *AlignPtr = IRB.CreateInBoundsGEP(
        LiteralType, Literal, {Zero, ConstantInt::get(Int32Ty, LiteralIdx)},
        "literal.align");
    IRB.CreateStore(
        ConstantInt::get(LiteralType->getElementType(LiteralIdx), LiteralAlign),
        AlignPtr);
    ++LiteralIdx;

    auto *InvokePtr = IRB.CreateInBoundsGEP(
        LiteralType, Literal, {Zero, ConstantInt::get(Int32Ty, LiteralIdx)},
        "literal.invoke");
    auto *InvokeType = LiteralType->getElementType(LiteralIdx);
    auto *AsyncInvokeBC = IRB.CreatePointerCast(AsyncInvoke, InvokeType);
    IRB.CreateStore(AsyncInvokeBC, InvokePtr);
    ++LiteralIdx;

    constexpr unsigned FArgStart = 4; // Exclude the first n args of F.
    auto ArgNum = F->arg_size() - FArgStart;
    auto LiteralParamNum = LiteralType->getNumElements() - 4;
    (void)ArgNum, (void)LiteralParamNum;
    assert(ArgNum == LiteralParamNum && "Argument numbers mismatch.");

    // Fill literal.arguments
    for (unsigned FArgIdx = FArgStart, ArgSize = F->arg_size();
         FArgIdx < ArgSize; ++FArgIdx, ++LiteralIdx) {
      auto *ArgPtr = IRB.CreateGEP(
          LiteralType, Literal, {Zero, ConstantInt::get(Int32Ty, LiteralIdx)});
      IRB.CreateStore(F->getArg(FArgIdx), ArgPtr);
    }

    auto *TaskSeqObjBC = IRB.CreatePointerCast(F->getArg(0), VoidPtrTy);
    auto *AsyncInvBC = IRB.CreatePointerCast(AsyncInvoke, VoidPtrTy);
    auto *LiteralBC = IRB.CreatePointerCast(Literal, VoidPtrTy);
    auto *Capacity = F->getArg(3);

    IRB.CreateCall(BackendAsync,
                   {TaskSeqObjBC, Capacity, AsyncInvBC, LiteralBC});
    IRB.CreateRetVoid();
    F->setLinkage(GlobalValue::InternalLinkage);
  }
  Changed = true;
}

void Impl::createAsyncFunctionInvokes() {
  if (Asyncs.empty())
    return;

  // void func.invoke(void *raw_literal) {
  //   LiteralType *literal = raw_literal;
  //   args... = load from literal.Arguments...;
  //   ReturnT *result_addr = literal.ResultAddr;
  //   *result_addr = func(args...);
  // }
  auto KernelMD = DPCPPKernelMetadataAPI::KernelList(M);
  auto Kernels = KernelMD.getList();
  auto *Int32Ty = Type::getInt32Ty(Ctx);
  auto *Zero = ConstantInt::get(Int32Ty, 0);
  for (Function *F : Asyncs) {
    Function *AsyncFunc = getCalledAsyncFunc(F);
    StructType *LiteralType = getLiteralType(F);

    auto *RetTy = Type::getVoidTy(Ctx);
    auto *PtrTy = Type::getInt8PtrTy(Ctx);
    auto *FTy = FunctionType::get(RetTy, {PtrTy}, false);
    FunctionCallee FuncInvokeCallee =
        M.getOrInsertFunction(getInovkeName(AsyncFunc), FTy);
    Function *FuncInvoke = cast<Function>(FuncInvokeCallee.getCallee());

    auto *Entry = BasicBlock::Create(Ctx);
    Entry->insertInto(FuncInvoke);
    IRB.SetInsertPoint(Entry);
    auto *Literal = IRB.CreatePointerCast(
        FuncInvoke->getArg(0), LiteralType->getPointerTo(), "literal");

    SmallVector<Value *> Args;
    Args.reserve(AsyncFunc->getFunctionType()->getNumParams());

    // Load arguments
    unsigned ResPtrIdx = LiteralType->getNumElements() - 1;
    constexpr unsigned LiteralParamStartIdx = 3;
    for (unsigned i = LiteralParamStartIdx; i < ResPtrIdx; ++i) {
      auto *ArgPtr = IRB.CreateGEP(LiteralType, Literal,
                                   {Zero, ConstantInt::get(Int32Ty, i)});
      auto *Arg = IRB.CreateLoad(LiteralType->getElementType(i), ArgPtr);
      Args.push_back(Arg);
    }

    auto *Invoke =
        IRB.CreateCall(AsyncFunc->getFunctionType(), AsyncFunc, Args);

    auto *AsyncFuncRetTy = Invoke->getType();

    // Save result
    if (!AsyncFuncRetTy->isVoidTy()) {
      auto *ResPtrPtr = IRB.CreateGEP(
          LiteralType, Literal, {Zero, ConstantInt::get(Int32Ty, ResPtrIdx)});
      auto *ResPtr =
          IRB.CreateLoad(LiteralType->getElementType(ResPtrIdx), ResPtrPtr);
      auto *ResPtrBC =
          IRB.CreatePointerCast(ResPtr, AsyncFuncRetTy->getPointerTo());
      IRB.CreateStore(Invoke, ResPtrBC);
    }
    IRB.CreateRetVoid();

    auto &DL = M.getDataLayout();
    auto LiteralSize = DL.getTypeStoreSize(LiteralType).getFixedSize();
    DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(FuncInvoke);
    KIMD.BlockLiteralSize.set(LiteralSize);
    Kernels.push_back(FuncInvoke);
  }
  KernelMD.set(Kernels);
}

} // namespace

PreservedAnalyses TaskSeqAsyncHandling::run(Module &M,
                                            ModuleAnalysisManager &AM) {
  if (Impl(M).run())
    return PreservedAnalyses::none();
  return PreservedAnalyses::all();
}

// For legacy PM
namespace {

class TaskSeqAsyncHandlingLegacy : public ModulePass {
public:
  static char ID;

  TaskSeqAsyncHandlingLegacy() : ModulePass(ID) {
    initializeTaskSeqAsyncHandlingLegacyPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "TaskSeqAsyncHandlingLegacy";
  }

  bool runOnModule(Module &M) override { return Impl(M).run(); }
};

} // namespace

char TaskSeqAsyncHandlingLegacy::ID = 0;

INITIALIZE_PASS(TaskSeqAsyncHandlingLegacy, DEBUG_TYPE,
                "Handle async APIs in task_sequence", false, false)

namespace llvm {
ModulePass *createTaskSeqAsyncHandlingLegacyPass() {
  return new TaskSeqAsyncHandlingLegacy();
}
} // namespace llvm

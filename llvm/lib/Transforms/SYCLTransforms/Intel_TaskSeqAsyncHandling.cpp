// ===--- Intel_TaskSeqAsyncHandling.cpp ------------------------ C++ -*--=== //
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/SYCLTransforms/Intel_TaskSeqAsyncHandling.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

#define DEBUG_TYPE "sycl-kernel-handle-taskseq-async"

using namespace llvm;
using namespace llvm::CompilationUtils;

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
/// - Task Function (TaskFunc*)
///   Function passed to the task_sequence class and to be called
///   asynchronously.
///
/// - Literal/Block Literal
///   A struct contains parameters and result address of a task function. See
///   its definition in createBlockLiteralTypes.
///
/// - Task Invoke (TaskInvoke*)
///   A fuction wrapper to extract parameters from block literal, and call the
///   task function, and save the result into the result address.
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
    collectTaskFuncs();
    lowerAllFunctionsSRetArgToReturnType();
    createBlockLiteralTypes();
    createTaskFunctionInvokes();
    generateInvokeMappers();
    generateBuiltinBodies();
    return Changed;
  }

private:
  void findAllAsyncBuiltins();
  void createBlockLiteralTypes();
  void createTaskFunctionInvokes();
  FunctionCallee getBackendCreateTaskSeq();
  FunctionCallee getBackendReleaseTaskSeq();
  FunctionCallee getBackendGet();
  FunctionCallee getBackendAsync();
  void generateCreateTaskSeqBodies();
  void generateReleaseTaskSeqBodies();
  void generateGetBodies();
  void generateAsyncBodies();

  void updateBuiltinInAsyncMap(Function *Builtin, Function *NewBuiltin);
  void updateTaskFuncInAsyncMap(Function *Builtin, Function *TaskF,
                                Function *NewTaskF);
  void replaceBuiltinInVector(Function *Builtin, Function *NewBuiltin);

  void
  collectTaskFunctionsWithSRetArg(SmallVector<Function *> &TaskFuncsWithSRet);
  Function *lowerTaskFunctionSRetArgToReturnType(Function *F);
  void fixupTaskFuncUsersAfterLoweringSRetArg(Function *TaskF,
                                              Function *NewTaskF);
  void lowerTaskFuncsSRetArgToReturnTypeAndFixupUsers(
      SmallVector<Function *> &TaskFuncsWithSRet);
  void lowerAllTaskFunctionsSRetArgToReturnType();

  void collectBuiltinsWithSRetArg(SmallVector<Function *> &Builtins,
                                  SmallVector<Function *> &BuiltinsWithSRet);
  Function *lowerBuiltinSRetArgToReturnType(Function *Builtin);
  void fixupBuiltinUsersAfterLoweringSRetArg(Function *Builtin,
                                             Function *NewBuiltin);
  void lowerBuiltinsSRetArgToReturnTypeAndFixupUsers(
      SmallVector<Function *> &BuiltinsWithSRet);
  void lowerAllBuiltinsSRetArgToReturnType();
  void lowerAllFunctionsSRetArgToReturnType();

  /// For each __spirv_TaskSequenceAsync template instance, there may be several
  /// task functions passed to it. Collect all of them. E.g.,
  ///   int foo(int, int);
  //    int bar(int, int);
  //    __spirv_TaskSequenceAsync(..., foo, ...)
  //    __spirv_TaskSequenceAsync(..., bar, ...)
  void collectTaskFuncs();

  /// For each __spirv_TaskSequenceAsync template instance, generate a function.
  /// It accept a task function passed to __spirv_TaskSequenceAsync, and
  /// return the corresponding block invoke function.
  void generateInvokeMappers();

  void generateBuiltinBodies() {
    generateCreateTaskSeqBodies();
    generateReleaseTaskSeqBodies();
    generateGetBodies();
    generateAsyncBodies();
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

  FunctionType *getBlockInvokeType() {
    auto *RetTy = Type::getVoidTy(Ctx);
    auto *PtrTy = PointerType::getUnqual(Ctx);
    auto *FTy = FunctionType::get(RetTy, {PtrTy}, false);
    return FTy;
  }

  static std::string getBlockInvokeMapperName(Function *F) {
    return (F->getName() + ".block_invoke_mapper").str();
  }

  /// A map between __spirv_TaskSequenceAsync template instances and task
  /// functions (the 2nd arg) passed to them.
  DenseMap<Function *, SmallVector<Function *>> AsyncBuiltinToTaskFuncMap;

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

void Impl::collectTaskFunctionsWithSRetArg(
    SmallVector<Function *> &TaskFuncsWithSRet) {
  for (Function *F : Asyncs) {
    for (Function *TaskF : AsyncBuiltinToTaskFuncMap[F]) {
      if (TaskF->arg_empty())
        continue;
      auto *Arg0 = TaskF->getArg(0);
      if (Arg0->getType()->isPointerTy() && Arg0->getParamStructRetType()) {
        LLVM_DEBUG(dbgs() << "Task function " << TaskF->getName()
                          << " with sret in argument 0 " << *Arg0 << "\n");
        TaskFuncsWithSRet.push_back(TaskF);
      }
    }
  }
}

Function *Impl::lowerTaskFunctionSRetArgToReturnType(Function *F) {
  SmallVector<Type *> ArgTys;
  for (auto I = F->arg_begin() + 1, E = F->arg_end(); I != E; I++)
    ArgTys.push_back(&*I->getType());

  auto *Arg0 = F->getArg(0);
  auto *SRetTy = Arg0->getParamStructRetType();
  FunctionType *NewFTy = FunctionType::get(SRetTy, ArgTys, false);
  std::string Name = F->getName().str();
  F->setName(F->getName() + "_before.TaskSeqAsyncHandling");
  Function *NewF =
      Function::Create(NewFTy, F->getLinkage(), Name, F->getParent());
  ValueToValueMapTy VMap;

  // Map Argument 0
  auto I = F->arg_begin();
  unsigned AddrSpace = cast<PointerType>(Arg0->getType())->getAddressSpace();
  Align AL = Arg0->getParamAlign().valueOrOne();
  AllocaInst *AI = new AllocaInst(SRetTy, AddrSpace, 0, AL, I->getName());
  VMap[&*I] = AI;

  // Map left arguments
  auto NewI = NewF->arg_begin();
  for (auto I = F->arg_begin() + 1, E = F->arg_end(); I != E; ++I) {
    NewI->setName(I->getName());
    VMap[&*I] = &*NewI;
    ++NewI;
  }
  SmallVector<ReturnInst *, 8> Rets;
  CloneFunctionInto(NewF, F, VMap, CloneFunctionChangeType::LocalChangesOnly,
                    Rets);

  for (ReturnInst *RI : make_early_inc_range(Rets)) {
    IRBuilder<> Builder(RI);
    LoadInst *LI = Builder.CreateLoad(SRetTy, AI);
    Builder.CreateRet(LI);
    RI->eraseFromParent();
  }

  // Insert alloca instruction at beginning
  Instruction *At = &*(NewF->getEntryBlock().begin());
  AI->insertBefore(At);

  LLVM_DEBUG(dbgs() << "New task function created: " << *NewF << "\n");
  return NewF;
}

void Impl::fixupTaskFuncUsersAfterLoweringSRetArg(Function *TaskF,
                                                  Function *NewTaskF) {
  for (auto *U : make_early_inc_range(TaskF->users())) {
    CallInst *CI = cast<CallInst>(U);
    LLVM_DEBUG(dbgs() << "Fix up user of task function: " << *CI << "\n");
    Function *Builtin = CI->getCalledFunction();
    assert(Builtin->isDeclaration() && "Expect function declaration");

    size_t Index = 0;
    for (; Index < CI->arg_size(); Index++)
      if (dyn_cast<Function>(CI->getArgOperand(Index)) == TaskF)
        break;
    // Update builtin function args. We don't need to update builtin function
    // type since the arg is function pointer type.
    CI->setArgOperand(Index, NewTaskF);
    if (Builtin->getName().startswith(BuiltinAsyncPattern))
      updateTaskFuncInAsyncMap(Builtin, TaskF, NewTaskF);
  }
}

static void replaceFunctionInVector(SmallVector<Function *> &FuncVec,
                                    Function *OldF, Function *NewF) {
  for (auto I = FuncVec.begin(), E = FuncVec.end(); I != E; I++)
    if (*I == OldF)
      *I = NewF;
}

void Impl::replaceBuiltinInVector(Function *Builtin, Function *NewBuiltin) {
  StringRef FName = Builtin->getName();
  if (FName.startswith(BuiltinGetPattern))
    replaceFunctionInVector(Gets, Builtin, NewBuiltin);
  else if (FName.startswith(BuiltinAsyncPattern))
    replaceFunctionInVector(Asyncs, Builtin, NewBuiltin);
  else if (FName.startswith(BuiltinCreateTaskSeqPattern))
    replaceFunctionInVector(Creates, Builtin, NewBuiltin);
  else if (FName.startswith(BuiltinReleaseTaskSeqPattern))
    replaceFunctionInVector(Releases, Builtin, NewBuiltin);
}

void Impl::updateBuiltinInAsyncMap(Function *Builtin, Function *NewBuiltin) {
  AsyncBuiltinToTaskFuncMap[NewBuiltin] = AsyncBuiltinToTaskFuncMap[Builtin];
  AsyncBuiltinToTaskFuncMap.erase(Builtin);
  LLVM_DEBUG(dbgs() << "Update builtin " << NewBuiltin->getName()
                    << " in mapping of builtin to task function\n");
}

void Impl::updateTaskFuncInAsyncMap(Function *Builtin, Function *TaskF,
                                    Function *NewTaskF) {
  for (auto I = AsyncBuiltinToTaskFuncMap[Builtin].begin(),
            E = AsyncBuiltinToTaskFuncMap[Builtin].end();
       I != E; I++) {
    if (*I == TaskF) {
      LLVM_DEBUG(dbgs() << "Erase mapping builtin " << Builtin->getName()
                        << " to " << TaskF->getName()
                        << " in task func map!\n");
      AsyncBuiltinToTaskFuncMap[Builtin].erase(I);
    }
  }
  AsyncBuiltinToTaskFuncMap[Builtin].push_back(NewTaskF);
  LLVM_DEBUG(dbgs() << "Map " << Builtin->getName() << " with "
                    << NewTaskF->getName() << " in task function map\n");
}

void Impl::lowerTaskFuncsSRetArgToReturnTypeAndFixupUsers(
    SmallVector<Function *> &TaskFuncsWithSRet) {
  for (Function *TaskF : make_early_inc_range(TaskFuncsWithSRet)) {
    LLVM_DEBUG(dbgs() << "Fix up task function: " << TaskF->getName() << "\n");
    Function *NewTaskF = lowerTaskFunctionSRetArgToReturnType(TaskF);
    fixupTaskFuncUsersAfterLoweringSRetArg(TaskF, NewTaskF);
    TaskF->eraseFromParent();
  }
}

void Impl::lowerAllTaskFunctionsSRetArgToReturnType() {
  SmallVector<Function *> TaskFuncsWithSRet;
  collectTaskFunctionsWithSRetArg(TaskFuncsWithSRet);
  lowerTaskFuncsSRetArgToReturnTypeAndFixupUsers(TaskFuncsWithSRet);
}

void Impl::collectBuiltinsWithSRetArg(
    SmallVector<Function *> &Builtins,
    SmallVector<Function *> &BuiltinsWithSRet) {
  for (Function *F : Builtins) {
    auto *Arg0 = F->getArg(0);
    if (Arg0->getType()->isPointerTy() && Arg0->getParamStructRetType()) {
      LLVM_DEBUG(dbgs() << "Builtin " << F->getName()
                        << " with sret in argument 0 " << *Arg0 << "\n");
      BuiltinsWithSRet.push_back(F);
    }
  }
}

Function *Impl::lowerBuiltinSRetArgToReturnType(Function *Builtin) {
  assert(Builtin->isDeclaration() && "Expect function declaration");

  auto *SRetTy = Builtin->getArg(0)->getParamStructRetType();
  assert(SRetTy && "No StructRet in argument 0");

  assert(Builtin->arg_size() >= 1 &&
         "Expect at least 1 argument with struct ret type");
  SmallVector<Type *> NewTys;
  for (auto I = Builtin->arg_begin() + 1, E = Builtin->arg_end(); I != E; I++)
    NewTys.push_back(I->getType());

  FunctionType *NewFTy = FunctionType::get(SRetTy, NewTys, false);
  std::string Name = Builtin->getName().str();
  Builtin->setName(Builtin->getName() + "_before.TaskSeqAsyncHandling");
  Function *NewBuiltin = Function::Create(NewFTy, Builtin->getLinkage(), Name,
                                          Builtin->getParent());

  LLVM_DEBUG(dbgs() << "Create new builtin: " << *NewBuiltin << "\n");

  return NewBuiltin;
}

void Impl::fixupBuiltinUsersAfterLoweringSRetArg(Function *Builtin,
                                                 Function *NewBuiltin) {
  for (auto *U : make_early_inc_range(Builtin->users())) {
    CallInst *CI = cast<CallInst>(U);

    assert(CI->arg_size() >= 1 &&
           "Expect at least 1 argument with struct ret type");
    SmallVector<Value *> NewArgs;
    for (auto I = CI->arg_begin() + 1, E = CI->arg_end(); I != E; I++)
      NewArgs.push_back(*I);

    IRBuilder<> Builder(CI);
    CallInst *NewCI =
        Builder.CreateCall(NewBuiltin->getFunctionType(), NewBuiltin, NewArgs);
    auto SRetArgOp = CI->getArgOperand(0);
    Builder.CreateStore(NewCI, SRetArgOp);
    CI->eraseFromParent();
  }
}

void Impl::lowerBuiltinsSRetArgToReturnTypeAndFixupUsers(
    SmallVector<Function *> &BuiltinsWithSRet) {
  for (Function *Builtin : make_early_inc_range(BuiltinsWithSRet)) {
    LLVM_DEBUG(dbgs() << "Fix up builtin: " << *Builtin << "\n");
    Function *NewBuiltin = lowerBuiltinSRetArgToReturnType(Builtin);
    fixupBuiltinUsersAfterLoweringSRetArg(Builtin, NewBuiltin);
    replaceBuiltinInVector(Builtin, NewBuiltin);
    StringRef FName = Builtin->getName();
    if (FName.startswith(BuiltinAsyncPattern))
      updateBuiltinInAsyncMap(Builtin, NewBuiltin);
    Builtin->eraseFromParent();
  }
}

void Impl::lowerAllBuiltinsSRetArgToReturnType() {
  SmallVector<Function *> BuiltinsWithSRet;
  collectBuiltinsWithSRetArg(Gets, BuiltinsWithSRet);
  collectBuiltinsWithSRetArg(Asyncs, BuiltinsWithSRet);
  collectBuiltinsWithSRetArg(Creates, BuiltinsWithSRet);
  collectBuiltinsWithSRetArg(Releases, BuiltinsWithSRet);
  lowerBuiltinsSRetArgToReturnTypeAndFixupUsers(BuiltinsWithSRet);
}

// Fix up all functions (including builtins and task functions) with arguments
// having sret attribute. For example, task function:
//   define void @foo(%struct.ty * sret(%struct.ty) %sret_ptr, i1 %func_arg)
// will be translated to:
//   define %struct.ty @foo(i1 %func_arg)
// And the related async intrinics will be translated from:
//   declare void @_Z30__spirv_TaskSequenceAsyncINTEL...(
//     %"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)*
//     % objptr, void(% struct.ty addrspace(4) *, i1) * % funcptr, i64 % id,
//     i32 % async_capacity, i1 % func_arg)
// to:
//   declare void @_Z30__spirv_TaskSequenceAsyncINTEL...(
//     %"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)*
//     % objptr, % struct.ty(i1) * % funcptr, i64 % id, i32 % async_capacity,
//     i1 % func_arg)
void Impl::lowerAllFunctionsSRetArgToReturnType() {
  // Fix up builtins before fixing up task functions so that it's not necessary
  // to handle sret attribute in the 1st argument of builtins.
  lowerAllBuiltinsSRetArgToReturnType();
  lowerAllTaskFunctionsSRetArgToReturnType();
}

void Impl::collectTaskFuncs() {
  for (Function *F : Asyncs) {
    SmallSetVector<Function *, 8> TaskFuncs;
    for (auto *U : F->users()) {
      auto *CI = cast<CallInst>(U);
      assert(CI->getCalledFunction() == F &&
             "__spirv_TaskSequenceAsync isn't directly called?");
      auto *TaskFunc =
          cast<Function>(CI->getArgOperand(1)->stripPointerCasts());
      TaskFuncs.insert(TaskFunc);
    }
    AsyncBuiltinToTaskFuncMap[F] = TaskFuncs.takeVector();
  }
}

void Impl::generateInvokeMappers() {
  for (Function *F : Asyncs) {
    auto *VoidPtrTy = PointerType::get(Ctx, ADDRESS_SPACE_GENERIC);
    auto *MapperType = FunctionType::get(VoidPtrTy, {VoidPtrTy}, false);
    auto MapperName = getBlockInvokeMapperName(F);
    auto MapperCallee = M.getOrInsertFunction(MapperName, MapperType);
    auto *Mapper = cast<Function>(MapperCallee.getCallee());

    Mapper->addFnAttr(Attribute::AlwaysInline);
    Mapper->setLinkage(GlobalValue::InternalLinkage);

    auto Entry = BasicBlock::Create(Ctx, "entry");
    Entry->insertInto(Mapper);

    auto &TaskFuncs = AsyncBuiltinToTaskFuncMap[F];
    assert(!TaskFuncs.empty() && "__spirv_TaskSequenceAsync wasn't called?");

    IRB.SetInsertPoint(Entry);

    auto *IntPtrTy = IntegerType::getIntNTy(Ctx, sizeof(intptr_t) * 8);
    auto *TaskFuncVar = IRB.CreatePtrToInt(Mapper->getArg(0), IntPtrTy);

    // Set the first block invoke as the default value.
    auto TaskIt = TaskFuncs.begin();
    auto *BlockInvoke = M.getFunction(getInovkeName(*TaskIt));
    assert(BlockInvoke && "Invoker function missed.");
    Value *LastVal = BlockInvoke;

    // Select the proper invoke based on the given task function using 'icmp'
    // and 'select'. Switch instruction won't work here because a ConstantExpr
    // 'ptrtoint (%block.invoke)' isn't a ConstantInt, while 'switch' only
    // accepts ConstantInt's as its value.
    auto E = TaskFuncs.end();
    for (++TaskIt; TaskIt != E; ++TaskIt) {
      auto *TaskFuncVal = ConstantExpr::getPtrToInt(*TaskIt, IntPtrTy);
      auto *Cmp = IRB.CreateICmp(CmpInst::ICMP_EQ, TaskFuncVar, TaskFuncVal);
      BlockInvoke = M.getFunction(getInovkeName(*TaskIt));
      assert(BlockInvoke && "Invoker function missed.");
      auto *Select = IRB.CreateSelect(Cmp, BlockInvoke, LastVal);
      LastVal = Select;
    }

    auto *RetBC = IRB.CreatePointerCast(LastVal, VoidPtrTy);
    IRB.CreateRet(RetBC);
  }
}

static FunctionType *findTaskFuncType(Function *F) {
  for (User *U : F->users())
    if (auto *CI = dyn_cast<CallInst>(U))
      return cast<Function>(CI->getArgOperand(1))->getFunctionType();
  return nullptr;
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
    LLVM_DEBUG(dbgs() << "createBlockLiteralTypes: F=" << F->getName() << "\n");
    // Find task func type from CallInst users of F.
    FunctionType *TaskFuncType = findTaskFuncType(F);
    assert(TaskFuncType && "task func type not found");
    auto *UnsignedTy = Type::getIntNTy(Ctx, sizeof(unsigned) * 8);
    auto *PtrTy = PointerType::getUnqual(Ctx);
    SmallVector<Type *> EltTypes;
    EltTypes.reserve(TaskFuncType->getNumParams() + 4);
    EltTypes.append({UnsignedTy, UnsignedTy, PtrTy});
    // TODO: Reorder parameters to reduce struct size
    EltTypes.append(TaskFuncType->param_begin(), TaskFuncType->param_end());
    EltTypes.push_back(PtrTy);
    LiteralMap.insert({F, StructType::get(Ctx, EltTypes)});
  }
}

size_t getRetTypeSizeOfTaskFunction(Function *F) {
  // The parameter F is task_sequence::__create_task_sequence, but not the
  // task function to call. The task function is the 1st argument of F.
  FunctionType *TaskFuncType = findTaskFuncType(F);
  assert(TaskFuncType && "task func type not found");
  auto &DL = F->getParent()->getDataLayout();
  auto RetType = TaskFuncType->getReturnType();
  if (RetType->isVoidTy())
    return 0;
  TypeSize RetTypeSize = DL.getTypeAllocSize(RetType);
  return RetTypeSize.getFixedValue();
}

FunctionCallee Impl::getBackendCreateTaskSeq() {
  // void *__create_task_sequence(size_t return_type_size)
  auto *SizeTTy = Type::getIntNTy(Ctx, sizeof(size_t) * 8);
  auto *RetTy = PointerType::getUnqual(Ctx);
  auto *FTy = FunctionType::get(RetTy, {SizeTTy}, false);
  return M.getOrInsertFunction(BackendCreateTaskSeqName, FTy);
}

FunctionCallee Impl::getBackendReleaseTaskSeq() {
  // void __release_task_sequence(void *task_seq)
  auto *PtrTy = PointerType::get(Ctx, ADDRESS_SPACE_GENERIC);
  auto *RetTy = Type::getVoidTy(Ctx);
  auto *FTy = FunctionType::get(RetTy, {PtrTy}, false);
  return M.getOrInsertFunction(BackendReleaseTaskSeqName, FTy);
}

FunctionCallee Impl::getBackendGet() {
  // void *__get(void *task_seq, unsigned capacity)
  auto *PtrTy = PointerType::get(Ctx, ADDRESS_SPACE_GENERIC);
  auto *I32Ty = Type::getInt32Ty(Ctx);
  auto *RetTy = PtrTy;
  auto *FTy = FunctionType::get(RetTy, {PtrTy, I32Ty}, false);
  return M.getOrInsertFunction(BackendGetName, FTy);
}

FunctionCallee Impl::getBackendAsync() {
  // void __async(void *task_seq, unsigned capacity,
  //              void *block_invoke, void *block_literal)
  auto *PtrTy = PointerType::get(Ctx, ADDRESS_SPACE_GENERIC);
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
  // We need to obtain return type size from task function. Convert task
  // function pointer to integer, compare the value with every task function and
  // then get results. Generated IR is like following:
  //   define internal i64 @"__spirv_TaskSequenceCreateINTEL"(ptr %obj, ptr %f)
  //   {
  //       %4 = ptrtoint ptr %f to i64
  //       %5 = icmp eq i64 %4, ptrtoint (ptr @userTaskFunc1 to i64)
  //       %6 = select i1 %5, i64 0, i64 8
  //       %7 = icmp eq i64 %4, ptrtoint (ptr @userTaskFunc2 to i64)
  //       %8 = select i1 %7, i64 0, i64 %6
  //       %9 = icmp eq i64 %4, ptrtoint (ptr @userTaskFunc3 to i64)
  //       %10 = select i1 %9, i64 8, i64 %8
  //       %11 = call ptr @__create_task_sequence(i64 %10)
  //       %12 = ptrtoint ptr %11 to i64
  //       ret i64 %12
  //     }
  FunctionCallee BackendCreateTaskSeq = getBackendCreateTaskSeq();
  auto *RetTypeSizeTy = BackendCreateTaskSeq.getFunctionType()->getParamType(0);
  for (Function *F : Creates) {
    auto Entry = BasicBlock::Create(Ctx);
    Entry->insertInto(F);
    IRB.SetInsertPoint(Entry);
    std::set<Function *> TaskFuncs;
    for (auto *U : F->users()) {
      auto *CI = cast<CallInst>(U);
      assert(CI->getCalledFunction() == F &&
             "__spirv_TaskSequenceCreate isn't directly called?");
      TaskFuncs.insert(
          cast<Function>(CI->getArgOperand(1)->stripPointerCasts()));
    }
    assert(!TaskFuncs.empty() &&
           "__spirv_TaskSequenceCreate builtin doesn't have users?");
    auto TaskIt = TaskFuncs.begin();
    auto &DL = F->getParent()->getDataLayout();
    auto GetTaskFuncRetTypeSize = [&](Function *TaskFunc) {
      auto RetType = TaskFunc->getReturnType();
      return RetType->isVoidTy() ? 0
                                 : DL.getTypeAllocSize(RetType).getFixedValue();
    };
    Value *LastVal =
        ConstantInt::get(RetTypeSizeTy, GetTaskFuncRetTypeSize(*TaskIt));
    if (TaskFuncs.size() > 1) {
      auto *IntPtrTy = IntegerType::getIntNTy(Ctx, sizeof(intptr_t) * 8);
      auto *TaskFuncVar = IRB.CreatePtrToInt(F->getArg(1), IntPtrTy);
      for (++TaskIt; TaskIt != TaskFuncs.end(); ++TaskIt) {
        auto *TaskFuncVal = ConstantExpr::getPtrToInt(*TaskIt, IntPtrTy);
        auto *Cmp = IRB.CreateICmp(CmpInst::ICMP_EQ, TaskFuncVar, TaskFuncVal);
        Value *RetVal =
            ConstantInt::get(RetTypeSizeTy, GetTaskFuncRetTypeSize(*TaskIt));
        auto *Select = IRB.CreateSelect(Cmp, RetVal, LastVal);
        LastVal = Select;
      }
    }
    auto *CI = IRB.CreateCall(BackendCreateTaskSeq, {LastVal});
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
    LLVM_DEBUG(dbgs() << "Generate get body: " << F->getName() << "\n");

    assert(F->arg_size() == 4 &&
           "Invalid number of arguments for get function");
    auto Entry = BasicBlock::Create(Ctx);
    Entry->insertInto(F);
    IRB.SetInsertPoint(Entry);
    auto *Cast = IRB.CreatePointerCast(F->getArg(0), Arg0Ty);
    auto *Capacity = F->getArg(F->arg_size() - 1);
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
  //   // Get invoke by calling the created invoke mapper;
  //   // Create literal;
  //   __async((void *)obj, capacity, (void *)invoke, (void *)literal);
  // }
  auto *Int32Ty = Type::getInt32Ty(Ctx);
  auto *Zero = ConstantInt::get(Int32Ty, 0);
  auto *VoidPtrTy = PointerType::get(Ctx, ADDRESS_SPACE_GENERIC);
  FunctionCallee BackendAsync = getBackendAsync();
  for (Function *F : Asyncs) {
    LLVM_DEBUG(dbgs() << "Generate async body: " << F->getName() << "\n");
    StructType *LiteralType = getLiteralType(F);

    auto *Entry = BasicBlock::Create(Ctx);
    Entry->insertInto(F);
    IRB.SetInsertPoint(Entry);

    auto *BlockInvokeMapper = M.getFunction(getBlockInvokeMapperName(F));
    assert(BlockInvokeMapper && "Block_invoke_mapper missed.");
    auto *TaskFunc = IRB.CreatePointerCast(F->getArg(1), VoidPtrTy);
    auto *TaskInvoke =
        IRB.CreateCall(BlockInvokeMapper->getFunctionType(), BlockInvokeMapper,
                       {TaskFunc}, "block.invoke");

    auto *Literal =
        IRB.CreateAlloca(LiteralType, /*ArraySize*/ nullptr, "literal");

    unsigned LiteralIdx = 0;

    auto &DL = M.getDataLayout();
    auto LiteralSize = DL.getTypeStoreSize(LiteralType).getFixedValue();
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
    auto *TaskInvokeBC = IRB.CreatePointerCast(TaskInvoke, InvokeType);
    IRB.CreateStore(TaskInvokeBC, InvokePtr);
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
          LiteralType, Literal, {Zero, ConstantInt::get(Int32Ty, LiteralIdx)},
          Twine("literal.argument.") + Twine(FArgIdx - FArgStart));
      IRB.CreateStore(F->getArg(FArgIdx), ArgPtr);
    }

    auto *TaskSeqObjBC = IRB.CreatePointerCast(F->getArg(0), VoidPtrTy);
    auto *TaskInvBC = IRB.CreatePointerCast(TaskInvoke, VoidPtrTy);
    auto *LiteralBC = IRB.CreatePointerCast(Literal, VoidPtrTy);
    auto *Capacity = F->getArg(3);

    IRB.CreateCall(BackendAsync,
                   {TaskSeqObjBC, Capacity, TaskInvBC, LiteralBC});
    IRB.CreateRetVoid();
    F->setLinkage(GlobalValue::InternalLinkage);
  }
  Changed = true;
}

void Impl::createTaskFunctionInvokes() {
  if (Asyncs.empty())
    return;

  // void func.invoke(void *raw_literal) {
  //   LiteralType *literal = raw_literal;
  //   args... = load from literal.Arguments...;
  //   ReturnT *result_addr = literal.ResultAddr;
  //   *result_addr = func(args...);
  // }
  auto KernelMD = SYCLKernelMetadataAPI::KernelList(M);
  auto Kernels = KernelMD.getList();
  auto *Int32Ty = Type::getInt32Ty(Ctx);
  auto *Zero = ConstantInt::get(Int32Ty, 0);

  for (Function *F : Asyncs) {
    LLVM_DEBUG(dbgs() << "Create task function invoke: " << F->getName()
                      << "\n");
    StructType *LiteralType = getLiteralType(F);

    auto *FTy = getBlockInvokeType();
    auto &DL = M.getDataLayout();
    auto LiteralSize = DL.getTypeStoreSize(LiteralType).getFixedValue();
    for (Function *TaskFunc : AsyncBuiltinToTaskFuncMap[F]) {
      FunctionCallee FuncInvokeCallee =
          M.getOrInsertFunction(getInovkeName(TaskFunc), FTy);
      Function *FuncInvoke = cast<Function>(FuncInvokeCallee.getCallee());

      auto *Entry = BasicBlock::Create(Ctx);
      Entry->insertInto(FuncInvoke);
      IRB.SetInsertPoint(Entry);
      auto *Literal = IRB.CreatePointerCast(
          FuncInvoke->getArg(0), LiteralType->getPointerTo(), "literal");

      SmallVector<Value *> Args;
      Args.reserve(TaskFunc->getFunctionType()->getNumParams());

      // Load arguments
      unsigned ResPtrIdx = LiteralType->getNumElements() - 1;
      constexpr unsigned LiteralParamStartIdx = 3;
      for (unsigned i = LiteralParamStartIdx; i < ResPtrIdx; ++i) {
        auto *ArgPtr = IRB.CreateGEP(
            LiteralType, Literal, {Zero, ConstantInt::get(Int32Ty, i)},
            Twine("literal.param.") + Twine(i - LiteralParamStartIdx));
        auto *Arg = IRB.CreateLoad(LiteralType->getElementType(i), ArgPtr,
                                   Twine("loaded.literal.param.") +
                                       Twine(i - LiteralParamStartIdx));
        Args.push_back(Arg);
      }
      auto *Invoke =
          IRB.CreateCall(TaskFunc->getFunctionType(), TaskFunc, Args);

      auto *TaskFuncRetTy = Invoke->getType();

      // Save result
      if (!TaskFuncRetTy->isVoidTy()) {
        auto *ResPtrPtr = IRB.CreateGEP(
            LiteralType, Literal, {Zero, ConstantInt::get(Int32Ty, ResPtrIdx)},
            "res.ptr.ptr");
        auto *ResPtr = IRB.CreateLoad(LiteralType->getElementType(ResPtrIdx),
                                      ResPtrPtr, "res.ptr");
        auto *ResPtrBC = IRB.CreatePointerCast(
            ResPtr, TaskFuncRetTy->getPointerTo(), "res.ptr.bcast");
        IRB.CreateStore(Invoke, ResPtrBC);
      }
      IRB.CreateRetVoid();

      SYCLKernelMetadataAPI::KernelInternalMetadataAPI KIMD(FuncInvoke);
      KIMD.BlockLiteralSize.set(LiteralSize);
      Kernels.push_back(FuncInvoke);
    }
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

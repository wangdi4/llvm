//===-- PipeSupport.cpp ---------------------------------------------------===//
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/PipeSupport.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Transforms/SYCLTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/SYCLChannelPipeUtils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <stack>

using namespace llvm;
using namespace SYCLChannelPipeUtils;
using namespace CompilationUtils;

#define DEBUG_TYPE "sycl-kernel-pipe-support"

struct PipeArrayView {
  Value *Ptr;
  Value *Size;
};

struct PipeCallInfo {
  CallInst *Call;
  PipeKind Kind;
};

static bool hasUsersInFunction(Value &V, Function &F) {
  for (auto *VU : V.users()) {
    auto *VUI = dyn_cast<Instruction>(VU);
    if (!VUI && hasUsersInFunction(*VU, F))
      return true;

    if (VUI && VUI->getFunction() == &F)
      return true;
  }
  return false;
}

static bool isPipeTy(Type *Ty) {
  auto *TETy = dyn_cast<TargetExtType>(Ty);
  return TETy && (TETy->getName() == "spirv.Pipe" ||
                  TETy->getName() == "spirv.Channel");
}

static int getNumUsedPipes(Function &F, const PipeTypesHelper &PipeTypes) {
  SmallPtrSet<Value *, 8> Pipes;

  // Count local pipes.
  SYCLKernelMetadataAPI::KernelInternalMetadataAPI KIMD(&F);
  if (KIMD.ArgTypeNullValList.hasValue()) {
    if (F.arg_size() == KIMD.ArgTypeNullValList.size()) {
      for (const auto &[Idx, C] : llvm::enumerate(KIMD.ArgTypeNullValList))
        if (isPipeTy(C->getType()))
          Pipes.insert(F.getArg(Idx));
    } else {
      // Some arguments are eliminated. In this case it is ok to over-estimate
      // the number of pipes, since this would only increase the capacity of
      // pipe arrays.
      auto MaxNumPipeArgs =
          (unsigned)llvm::count_if(KIMD.ArgTypeNullValList, [](auto *C) {
            return isPipeTy(C->getType());
          });
      for (auto &A : F.args()) {
        if (Pipes.size() == MaxNumPipeArgs)
          break;
        if (A.getType() ==
            PointerType::get(A.getContext(), ADDRESS_SPACE_GLOBAL))
          Pipes.insert(&A);
      }
    }
  }
  if (Pipes.empty()) {
    // Parse pipe from mangled non-kernel function name, e.g. the first
    // parameter of a sycl device function:
    // static int32_t
    // __latency_control_nb_read_wrapper(__ocl_RPipeTy<_T> Pipe, _T *Data,
    //                                   int32_t AnchorID, int32_t TargetAnchor,
    //                                   int32_t Type, int32_t Cycle)
    ItaniumPartialDemangler Demangler;
    std::string FName = F.getName().str();
    if (!Demangler.partialDemangle(FName.c_str())) {
      char *Params = Demangler.getFunctionParameters(nullptr, nullptr);
      SmallVector<StringRef, 8> ParamList;
      StringRef ParamsRef(Params);
      std::ignore = ParamsRef.consume_front("(");
      std::ignore = ParamsRef.consume_back(")");
      SplitString(ParamsRef, ParamList, ",");
      // Skip kernel name which doesn't follow Itanium C++ ABI mangling.
      if (ParamList.size() == F.getFunctionType()->getNumParams()) {
        for (const auto &[Idx, Arg] : llvm::enumerate(F.args()))
          if (ParamList[Idx].ltrim(" ") == "ocl_pipe")
            Pipes.insert(&Arg);
      }
      free(Params);
    }
  }

  // Count global pipes
  auto *M = F.getParent();
  for (auto &GV : M->globals())
    if (isGlobalPipe(&GV))
      Pipes.insert(&GV);

  unsigned PipesNum = 0;
  llvm::for_each(Pipes, [&](Value *V) {
    if (!hasUsersInFunction(*V, F) && !isa<Argument>(V))
      return;
    if (const auto *GV = dyn_cast<GlobalVariable>(V)) {
      if (auto *ArrTy = dyn_cast<ArrayType>(GV->getValueType())) {
        PipesNum += getNumElementsOfNestedArray(ArrTy);
        return;
      }
    }
    PipesNum += 1;
  });

  return PipesNum;
}

static void findPipeCalls(Function &F,
                          SmallVectorImpl<PipeCallInfo> &PipeCalls) {
  for (auto &I : instructions(F)) {
    auto *Call = dyn_cast<CallInst>(&I);
    if (!Call)
      continue;

    auto *Callee = Call->getCalledFunction();
    if (!Callee)
      continue;

    PipeKind Kind = getPipeKind(Callee->getName());
    if (Kind)
      PipeCalls.push_back({Call, Kind});
  }
}

static PipeArrayView createPipeArray(Function &F, int Size) {
  IRBuilder<> Builder(&F.getEntryBlock().front());

  auto *Array = Builder.CreateAlloca(
      Builder.getInt8PtrTy(AddressSpace::ADDRESS_SPACE_GLOBAL),
      Builder.getInt32(Size));
  auto *ArraySize = Builder.CreateAlloca(Builder.getInt32Ty());
  Builder.CreateStore(Builder.getInt32(0), ArraySize);

  return {Array, ArraySize};
}

static CallInst *createFlushCall(Module *M, StringRef Name,
                                 PipeArrayView PipeStorage,
                                 RuntimeService &RTS) {
  Function *FlushF =
      importFunctionDecl(M, RTS.findFunctionInBuiltinModules(Name));
  return CallInst::Create(FlushF, {PipeStorage.Ptr, PipeStorage.Size});
}

static void insertFlushAtExit(Function &F, Instruction *FlushReadCall,
                              Instruction *FlushWriteCall) {
  for (auto &BB : F) {
    auto *Term = BB.getTerminator();
    assert(Term && "Ill-formed BasicBlock.");

    if (!isa<ReturnInst>(Term))
      continue;

    IRBuilder<> Builder(Term);
    Builder.Insert(FlushReadCall);
    Builder.Insert(FlushWriteCall);

    return;
  }

  // No return instuction found in the function, remove redundant calls
  FlushReadCall->deleteValue();
  FlushWriteCall->deleteValue();
}

static void insertStorePipeCall(Module *M, StringRef Name, CallInst *Call,
                                PipeArrayView StoreA, RuntimeService &RTS) {
  IRBuilder<> Builder(Call);
  Function *StoreF =
      importFunctionDecl(M, RTS.findFunctionInBuiltinModules(Name));
  auto *Pipe = Call->getOperand(0);

  Value *StoreArgs[] = {StoreA.Ptr, StoreA.Size, Pipe};
  Builder.CreateCall(StoreF, StoreArgs);
}

static Value *getPipeCallRetcode(const PipeCallInfo &PC) {

  // SIMD read pipe built-in has a retcode as a second parameter
  if (PC.Kind.Access == PipeKind::AccessKind::Read &&
      !PC.Kind.SimdSuffix.empty()) {
    auto *RetcodePtr = PC.Call->getArgOperand(1);

    Type *Ty = RetcodePtr->getType();
    return new LoadInst(Ty, RetcodePtr, "", PC.Call->getNextNode());
  }

  // all others just return it directly
  return PC.Call;
}

static PipeCallInfo replaceBlockingCall(Module *M, const PipeCallInfo &PC,
                                        RuntimeService &RTS) {
  PipeKind NonBlockingKind = PC.Kind;
  NonBlockingKind.Blocking = false;

  Function *NonBlockingFun = importFunctionDecl(
      M, RTS.findFunctionInBuiltinModules(getPipeName(NonBlockingKind)));

  SmallVector<Value *, 4> NewArgs(PC.Call->args());

  // Blocking SIMD read does not have a retcode, allocate it.
  if (NonBlockingKind.Access == PipeKind::AccessKind::Read &&
      !NonBlockingKind.SimdSuffix.empty()) {
    IRBuilder<> Builder(PC.Call);

    auto *NonBlockingFTy = NonBlockingFun->getFunctionType();
    assert(NonBlockingFTy->getNumParams() == 2 &&
           "Unexpected number of function parameters");

    auto *RetcodePtrTy = cast<PointerType>(
        NonBlockingFTy->getParamType(NonBlockingFTy->getNumParams() - 1));

    NewArgs.push_back(Builder.CreatePointerBitCastOrAddrSpaceCast(
        Builder.CreateAlloca(RetcodePtrTy), RetcodePtrTy, "retcode"));
  }
  auto *NonBlockingCall = CallInst::Create(NonBlockingFun, NewArgs);
  ReplaceInstWithInst(PC.Call, NonBlockingCall);

  return {NonBlockingCall, NonBlockingKind};
}

static void restoreBlockingCall(CallInst *Call, BasicBlock *FlushBB) {

  BasicBlock *CallBB = SplitBlock(Call->getParent(), Call);
  auto *NewTerm = BranchInst::Create(CallBB);
  ReplaceInstWithInst(FlushBB->getTerminator(), NewTerm);
}

static BasicBlock *insertFlushAtNonBlockingCall(const PipeCallInfo &PC,
                                                Instruction *FlushReadCall,
                                                Instruction *FlushWriteCall) {
  assert(PC.Call->getNextNode() && "Call does not have a next node");
  IRBuilder<> Builder(PC.Call->getNextNode());

  // 'At call' flushes should be executed iff pipe BI call failed
  auto *Retcode = getPipeCallRetcode(PC);
  auto *CallSuccessRetcode = Builder.getInt32(0);
  auto *IsCallFailed = Builder.CreateICmpNE(Retcode, CallSuccessRetcode);

  auto *ThenTerm = SplitBlockAndInsertIfThen(
      IsCallFailed, cast<Instruction>(IsCallFailed)->getNextNode(),
      /*Unreachable=*/false);

  Builder.SetInsertPoint(ThenTerm);
  Builder.Insert(FlushReadCall);
  Builder.Insert(FlushWriteCall);

  return ThenTerm->getParent();
}

static BasicBlock *insertFlushAtBlockingCall(Module *M, const PipeCallInfo &PC,
                                             Instruction *FlushReadCall,
                                             Instruction *FlushWriteCall,
                                             RuntimeService &RTS) {
  // Blocking BIs are the same as non-blocking but with a spin-loop over
  // We can easily replace them here to get flushes into the spin-loop
  PipeCallInfo NonBlockingPC = replaceBlockingCall(M, PC, RTS);

  BasicBlock *FlushBB = insertFlushAtNonBlockingCall(
      NonBlockingPC, FlushReadCall, FlushWriteCall);

  // Restore blocking functionality by creating a loop over the call and flushes
  restoreBlockingCall(NonBlockingPC.Call, FlushBB);

  return FlushBB;
}

static bool addImplicitFlushCalls(Module *M, Function &F, RuntimeService &RTS,
                                  const PipeTypesHelper &PipeTypes) {

  SmallVector<PipeCallInfo, 16> PipeCalls;
  findPipeCalls(F, PipeCalls);

  if (PipeCalls.empty())
    return false; // no pipe BI calls in the function, nothing to do

  int NumPipes = getNumUsedPipes(F, PipeTypes);
  assert(NumPipes > 0 && "Expected pipe users in the function");

  // Implicit flush insertion requires to know which pipes are used in the
  // current function. In compile time we may not have such information
  // (in case of non-constant GEPs from arrays, for example).
  // Allocate 2 arrays in the function to store used pipes in run time.
  PipeArrayView ReadArray = createPipeArray(F, NumPipes);
  PipeArrayView WriteArray = createPipeArray(F, NumPipes);

  CallInst *FlushReadCall =
      createFlushCall(M, "__flush_pipe_read_array", ReadArray, RTS);
  CallInst *FlushWriteCall =
      createFlushCall(M, "__flush_pipe_write_array", WriteArray, RTS);

  // Insert store pipe and flushes BI calls at every pipe BI call
  for (const auto &PC : PipeCalls) {

    if (PC.Kind.Access == PipeKind::AccessKind::Write)
      insertStorePipeCall(M, "__store_write_pipe_use", PC.Call, WriteArray,
                          RTS);
    else
      insertStorePipeCall(M, "__store_read_pipe_use", PC.Call, ReadArray, RTS);

    if (PC.Kind.Blocking)
      insertFlushAtBlockingCall(M, PC, FlushReadCall->clone(),
                                FlushWriteCall->clone(), RTS);
    else
      insertFlushAtNonBlockingCall(PC, FlushReadCall->clone(),
                                   FlushWriteCall->clone());
  }

  // Ensure that nothing is cached upon exit from a function
  insertFlushAtExit(F, FlushReadCall, FlushWriteCall);
  return true;
}

PreservedAnalyses PipeSupportPass::run(Module &M, ModuleAnalysisManager &MAM) {
  BuiltinLibInfo *BLI = &MAM.getResult<BuiltinLibInfoAnalysis>(M);
  return runImpl(M, BLI) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool PipeSupportPass::runImpl(Module &M, BuiltinLibInfo *BLI) {
  assert(BLI && "Invalid builtin lib info!");
  RuntimeService &RTS = BLI->getRuntimeService();

  bool Changed = false;

  PipeTypesHelper PipeTypes(M);
  if (!PipeTypes.hasPipeTypes())
    return false; // no pipes in the module, nothing to do

  std::stack<Function *, std::vector<Function *>> WorkList;
  for (auto &F : M) {
    if (F.isDeclaration())
      continue;
    bool FuncUseFPGAPipes = addImplicitFlushCalls(&M, F, RTS, PipeTypes);
    Changed |= FuncUseFPGAPipes;

    SYCLKernelMetadataAPI::KernelInternalMetadataAPI(&F).UseFPGAPipes.set(
        FuncUseFPGAPipes);
    if (!FuncUseFPGAPipes)
      continue;

    // OpenCL NDRange vectorizer is not able to handle non-blocking pipe
    // operations. Disable vectorization of the functions with
    // these operations by setting vector width to 1.

    SYCLKernelMetadataAPI::KernelMetadataAPI(&F).VecLenHint.set(
        1 /* TRANSPOSE_SIZE_1 */);

    WorkList.push(&F);
  }

  while (!WorkList.empty()) {
    auto *F = WorkList.top();
    WorkList.pop();
    for (const auto &U : F->users()) {
      if (CallInst *Call = dyn_cast<CallInst>(U)) {
        auto *ParentFunction = Call->getFunction();
        auto UseFPGAPipesMD =
            SYCLKernelMetadataAPI::KernelInternalMetadataAPI(ParentFunction)
                .UseFPGAPipes;
        // In WorkList all functions have pipes, so all parent functions
        // should have pipes.
        if (!UseFPGAPipesMD.hasValue() ||
            (UseFPGAPipesMD.hasValue() && !UseFPGAPipesMD.get()))
          WorkList.push(ParentFunction);
        UseFPGAPipesMD.set(true);
      }
    }
  }
  return Changed;
}

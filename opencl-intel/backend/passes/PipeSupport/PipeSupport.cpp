// INTEL CONFIDENTIAL
//
// Copyright 2017-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "PipeSupport.h"

#include <BuiltinLibInfo.h>
#include <CompilationUtils.h>
#include <InitializePasses.h>
#include <MetadataAPI.h>
#include <OCLAddressSpace.h>
#include <OCLPassSupport.h>
#include "ICLDevBackendOptions.h"

#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

#include <algorithm>
#include <stack>

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

char PipeSupport::ID = 0;
OCL_INITIALIZE_PASS_BEGIN(
    PipeSupport, "pipe-support",
    "Apply transformation required by pipe built-ins implementation",
    false, true)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_DEPENDENCY(ChannelPipeTransformation)
OCL_INITIALIZE_PASS_END(
    PipeSupport, "pipe-support",
    "Apply transformation required by pipe built-ins implementation",
    false, true)

PipeSupport::PipeSupport() : ModulePass(ID) {}

} // namespace intel

namespace {

struct PipeArrayView {
  Value *Ptr;
  Value *Size;
};

struct PipeCallInfo {
  CallInst *Call;
  PipeKind Kind;
};

} // namespace

namespace intel {

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

static int getNumUsedPipes(Function &F, const PipeTypesHelper &PipeTypes) {
  int PipesNum = 0;

  // Count local pipes
  for (auto &Arg : F.args()) {
    if (PipeTypes.isPipeType(Arg.getType()))
      ++PipesNum;
  }

  // Count global pipes
  auto *M = F.getParent();
  for (auto &GV : M->globals()) {
    auto *Ty = GV.getType()->getElementType();

    auto *ArrTy = dyn_cast<ArrayType>(Ty);
    if (ArrTy)
      Ty = CompilationUtils::getArrayElementType(ArrTy);

    if (!PipeTypes.isGlobalPipeType(Ty))
      continue;

    if (hasUsersInFunction(GV, F))
      PipesNum += (ArrTy) ? CompilationUtils::getArrayNumElements(ArrTy) : 1;
  }
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

    PipeKind Kind = CompilationUtils::getPipeKind(Callee->getName());
    if (Kind)
      PipeCalls.push_back({Call, Kind});
  }
}

static PipeArrayView createPipeArray(Function &F, int Size) {
  IRBuilder<> Builder(&F.getEntryBlock().front());

  auto *Array =
      Builder.CreateAlloca(Builder.getInt8PtrTy(Utils::OCLAddressSpace::Global),
                           Builder.getInt32(Size));
  auto *ArraySize = Builder.CreateAlloca(Builder.getInt32Ty());
  Builder.CreateStore(Builder.getInt32(0), ArraySize);

  return {Array, ArraySize};
}

static CallInst *createFlushCall(StringRef Name, PipeArrayView PipeStorage,
                                 OCLBuiltins &Builtins) {
  auto *FlushF = Builtins.get(Name);
  return CallInst::Create(FlushF, {PipeStorage.Ptr, PipeStorage.Size});
}

static void insertFlushAtExit(Function &F, Instruction *FlushReadCall,
                              Instruction *FlushWriteCall) {
  for (auto &BB : F) {
    auto *Term = BB.getTerminator();
    assert(Term && "Ill-formed BasicBlock.");

    if (!isa<ReturnInst>(Term))
      continue;

    FlushReadCall->insertBefore(Term);
    FlushWriteCall->insertBefore(Term);

    return;
  }

  // No return instuction found in the function, remove redundant calls
  FlushReadCall->deleteValue();
  FlushWriteCall->deleteValue();
}

static void insertStorePipeCall(StringRef Name, CallInst *Call,
                                PipeArrayView StoreA, OCLBuiltins &Builtins) {
  IRBuilder<> Builder(Call);

  auto *StoreF = Builtins.get(Name);
  auto *Pipe = Call->getOperand(0);

  Value *StoreArgs[] = {StoreA.Ptr, StoreA.Size, Pipe};
  Builder.CreateCall(StoreF, StoreArgs);
}

static Value *getPipeCallRetcode(const PipeCallInfo &PC) {

  // SIMD read pipe built-in has a retcode as a second parameter
  if (PC.Kind.Access == PipeKind::READ && !PC.Kind.SimdSuffix.empty()) {
    auto *RetcodePtr = PC.Call->getArgOperand(1);

    return new LoadInst(RetcodePtr, "", PC.Call->getNextNode());
  }

  // all others just return it directly
  return PC.Call;
}

static PipeCallInfo replaceBlockingCall(const PipeCallInfo &PC,
                                        OCLBuiltins &Builtins) {
  PipeKind NonBlockingKind = PC.Kind;
  NonBlockingKind.Blocking = false;

  auto *NonBlockingFun =
    Builtins.get(CompilationUtils::getPipeName(NonBlockingKind));

  SmallVector<Value *, 4> NewArgs(PC.Call->arg_operands());

  // Blocking SIMD read does not have a retcode, allocate it.
  if (NonBlockingKind.Access == PipeKind::READ &&
      !NonBlockingKind.SimdSuffix.empty()) {
    IRBuilder<> Builder(PC.Call);

    auto *NonBlockingFTy = NonBlockingFun->getFunctionType();
    assert(NonBlockingFTy->getNumParams() == 2
           && "Unexpected number of function parameters");

    auto *RetcodePtrTy = cast<PointerType>(
      NonBlockingFTy->getParamType(NonBlockingFTy->getNumParams() - 1));

    NewArgs.push_back(
      Builder.CreatePointerBitCastOrAddrSpaceCast(
        Builder.CreateAlloca(RetcodePtrTy->getElementType()),
        RetcodePtrTy,
        "retcode"));
  }
  auto *NonBlockingCall = CallInst::Create(NonBlockingFun, NewArgs);
  llvm::ReplaceInstWithInst(PC.Call, NonBlockingCall);

  return {NonBlockingCall, NonBlockingKind};
}

static void restoreBlockingCall(CallInst *Call, BasicBlock *FlushBB) {

  BasicBlock *CallBB = llvm::SplitBlock(Call->getParent(), Call);
  auto *NewTerm = BranchInst::Create(CallBB);
  llvm::ReplaceInstWithInst(FlushBB->getTerminator(), NewTerm);
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

  auto *ThenTerm = llvm::SplitBlockAndInsertIfThen(
    IsCallFailed, cast<Instruction>(IsCallFailed)->getNextNode(),
    /*Unreachable=*/false);

  FlushReadCall->insertBefore(ThenTerm);
  FlushWriteCall->insertBefore(ThenTerm);

  return ThenTerm->getParent();
}

static BasicBlock *insertFlushAtBlockingCall(const PipeCallInfo &PC,
                                             Instruction *FlushReadCall,
                                             Instruction *FlushWriteCall,
                                             OCLBuiltins &Builtins) {
  // Blocking BIs are the same as non-blocking but with a spin-loop over
  // We can easily replace them here to get flushes into the spin-loop
  PipeCallInfo NonBlockingPC = replaceBlockingCall(PC, Builtins);

  BasicBlock* FlushBB =
    insertFlushAtNonBlockingCall(NonBlockingPC, FlushReadCall, FlushWriteCall);

  // Restore blocking functionality by creating a loop over the call and flushes
  restoreBlockingCall(NonBlockingPC.Call, FlushBB);

  return FlushBB;
}

static bool addImplicitFlushCalls(Function &F, OCLBuiltins &Builtins,
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
    createFlushCall("__flush_pipe_read_array", ReadArray, Builtins);
  CallInst *FlushWriteCall =
    createFlushCall("__flush_pipe_write_array", WriteArray, Builtins);

  // Insert store pipe and flushes BI calls at every pipe BI call
  for (const auto &PC : PipeCalls) {

    if (PC.Kind.Access == PipeKind::WRITE)
      insertStorePipeCall("__store_write_pipe_use", PC.Call, WriteArray,
                          Builtins);
    else
      insertStorePipeCall("__store_read_pipe_use", PC.Call, ReadArray,
                          Builtins);

    if (PC.Kind.Blocking)
      insertFlushAtBlockingCall(
        PC, FlushReadCall->clone(), FlushWriteCall->clone(), Builtins);
    else
      insertFlushAtNonBlockingCall(
        PC, FlushReadCall->clone(), FlushWriteCall->clone());
  }

  // Ensure that nothing is cached upon exit from a function
  insertFlushAtExit(F, FlushReadCall, FlushWriteCall);
  return true;
}

bool PipeSupport::runOnModule(Module &M) {
  bool Changed = false;

  PipeTypesHelper PipeTypes(M);
  if (!PipeTypes.hasPipeTypes())
    return false; // no pipes in the module, nothing to do

  BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfo>();
  OCLBuiltins Builtins(M, BLI.getBuiltinModules());

  std::stack<Function *, std::vector<Function *>> WorkList;
  for (auto &F : M) {
    if (F.isDeclaration())
      continue;
    bool FuncUseFPGAPipes = addImplicitFlushCalls(F, Builtins, PipeTypes);
    Changed |= FuncUseFPGAPipes;

    Intel::MetadataAPI::KernelInternalMetadataAPI(&F).UseFPGAPipes.set(
        FuncUseFPGAPipes);
    if (!FuncUseFPGAPipes)
      continue;

    // OpenCL NDRange vectorizer is not able to handle non-blocking pipe
    // operations. Disable vectorization of the functions with
    // these operations by setting vector width to 1.

    Intel::MetadataAPI::KernelMetadataAPI(&F).VecLenHint.set(TRANSPOSE_SIZE_1);

    WorkList.push(&F);
  }

  while (!WorkList.empty()) {
    auto *F = WorkList.top();
    WorkList.pop();
    for (const auto &U : F->users()) {
      if (CallInst *Call = dyn_cast<CallInst>(U)) {
        auto ParentFunction = Call->getFunction();
        auto UseFPGAPipesMD =
            Intel::MetadataAPI::KernelInternalMetadataAPI(ParentFunction)
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

void PipeSupport::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<BuiltinLibInfo>();
}

} // namespace intel

extern "C" {
ModulePass *createPipeSupportPass() { return new intel::PipeSupport(); }
}

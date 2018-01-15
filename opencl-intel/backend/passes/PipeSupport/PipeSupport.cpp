// Copyright (c) 2017 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

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

class OCLBuiltins {
private:
  Module &TargetModule;
  SmallVector<Module *, 2> RTLs;

public:
  OCLBuiltins(Module &TargetModule, intel::BuiltinLibInfo &BLI) :
      TargetModule(TargetModule), RTLs(BLI.getBuiltinModules()) {}

  Function *get(StringRef Name) {
    if (auto F = TargetModule.getFunction(Name))
      return F;

    for (auto *BIModule : RTLs) {
      if (auto *F = BIModule->getFunction(Name))
        return cast<Function>(
          CompilationUtils::importFunctionDecl(&TargetModule, F));
    }

    llvm_unreachable("Built-in not found.");
  }
};

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

static int getNumUsedPipes(Function &F, PointerType *PipeTy) {
  int PipesNum = 0;

  // Count local pipes
  for (auto &Arg : F.args()) {
    if (CompilationUtils::isSameStructPtrType(Arg.getType(), PipeTy))
      ++PipesNum;
  }

  // Count global pipes
  auto *M = F.getParent();
  for (auto &GV : M->globals()) {
    auto *Ty = GV.getType()->getElementType();

    auto *ArrTy = dyn_cast<ArrayType>(Ty);
    if (ArrTy)
      Ty = CompilationUtils::getArrayElementType(ArrTy);

    if (!CompilationUtils::isSameStructPtrType(Ty, PipeTy))
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

static PipeArrayView createPipeArray(Function &F,
                                     PointerType* PipeTy, int Size) {
  IRBuilder<> Builder(&F.getEntryBlock().front());

  auto *Array = Builder.CreateAlloca(PipeTy, Builder.getInt32(Size));
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

static void insertStorePipeCall(CallInst *Call, PipeArrayView StoreA,
                                OCLBuiltins &Builtins) {
  IRBuilder<> Builder(Call);

  auto *StoreF = Builtins.get("__store_pipe_use");
  auto *Pipe = Builder.CreateBitCast(
    Call->getOperand(0), StoreF->getFunctionType()->getParamType(2));

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

  SmallVector<Value *, 2> NewArgs(PC.Call->arg_operands());

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
                                  PointerType *PipeTy,
                                  PointerType *PipeImplTy) {

  SmallVector<PipeCallInfo, 16> PipeCalls;
  findPipeCalls(F, PipeCalls);

  if (PipeCalls.empty())
    return false; // no pipe BI calls in the function, nothing to do

  int NumPipes = getNumUsedPipes(F, PipeTy);
  assert(NumPipes > 0 && "Expected pipe users in the function");

  // Implicit flush insertion requires to know which pipes are used in the
  // current function. In compile time we may not have such information
  // (in case of non-constant GEPs from arrays, for example).
  // Allocate 2 arrays in the function to store used pipes in run time.
  PipeArrayView ReadArray = createPipeArray(F, PipeImplTy, NumPipes);
  PipeArrayView WriteArray = createPipeArray(F, PipeImplTy, NumPipes);

  CallInst *FlushReadCall =
    createFlushCall("__flush_pipe_read_array", ReadArray, Builtins);
  CallInst *FlushWriteCall =
    createFlushCall("__flush_pipe_write_array", WriteArray, Builtins);

  // Insert store pipe and flushes BI calls at every pipe BI call
  for (const auto &PC : PipeCalls) {

    PipeArrayView &Array =
      PC.Kind.Access == PipeKind::READ ? ReadArray : WriteArray;
    insertStorePipeCall(PC.Call, Array, Builtins);

    if (PC.Kind.Blocking)
      insertFlushAtBlockingCall(
        PC, FlushReadCall->clone(), FlushWriteCall->clone(), Builtins);
    else
      insertFlushAtNonBlockingCall(
        PC, FlushReadCall->clone(), FlushWriteCall->clone());
  }

  // Ensure that nothing is cached upon exit from a function
  insertFlushAtExit(F, FlushReadCall, FlushWriteCall);

  // OpenCL NDRange vectorizer is not able to handle non-blocking pipe
  // operations. Disable vectorization of the functions with
  // these operations by setting vector width to 1.
  Intel::MetadataAPI::KernelMetadataAPI(&F).VecLenHint.set(TRANSPOSE_SIZE_1);

  return true;
}

bool PipeSupport::runOnModule(Module &M) {
  bool Changed = false;

  auto *PipeTy = M.getTypeByName("opencl.pipe_t");
  if (!PipeTy) // no pipes in the module, nothing to do
    return false;
  auto *PipePtrTy = PointerType::get(PipeTy, Utils::OCLAddressSpace::Global);

  auto *PipeImplTy = M.getTypeByName("struct.__pipe_t");
  assert(PipeImplTy && "__pipe_t type should exists in the module");
  auto *PipeImplPtrTy =
    PointerType::get(PipeImplTy, Utils::OCLAddressSpace::Global);

  BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfo>();
  OCLBuiltins Builtins(M, BLI);

  for (auto &F : M) {
    if (F.isDeclaration())
      continue;

    Changed |= addImplicitFlushCalls(F, Builtins, PipePtrTy, PipeImplPtrTy);
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

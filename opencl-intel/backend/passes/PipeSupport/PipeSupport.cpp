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

#include <llvm/ADT/SmallString.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>

#include <BuiltinLibInfo.h>
#include <CompilationUtils.h>
#include <InitializePasses.h>
#include <OCLPassSupport.h>

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

char PipeSupport::ID = 0;
OCL_INITIALIZE_PASS_BEGIN(PipeSupport, "pipe-support",
                          "Apply transformation required by pipe built-ins implementation",
                          false, true)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_DEPENDENCY(ChannelPipeTransformation)
OCL_INITIALIZE_PASS_END(PipeSupport, "pipe-support",
                        "Apply transformation required by pipe built-ins implementation",
                        false, true)


PipeSupport::PipeSupport() : ModulePass(ID) {
}

static Function *importRTLFunctionDecl(Module &TargetModule,
                                       const SmallVectorImpl<Module *> &RTLs,
                                       StringRef Name) {
  for (auto *BIModule : RTLs) {
    if (auto *F = BIModule->getFunction(Name)) {
      return cast<Function>(
          CompilationUtils::importFunctionDecl(&TargetModule, F));
    }
  }

  return nullptr;
}

static void findPipeCalls(Function &F,
                          SmallVectorImpl<CallInst *> &Calls) {

  for (auto &BB : F) {
    for (auto &I : BB) {
      if (auto *Call = dyn_cast<CallInst>(&I)) {
        StringRef Name = Call->getCalledFunction()->getName();
        if (CompilationUtils::isReadPipeBuiltin(Name) ||
            CompilationUtils::isWritePipeBuiltin(Name)) {
          Calls.push_back(Call);
        }
      }
    }
  }
}

static bool insertFlushCall(Module &M, CallInst *PipeCall,
                            Function *FlushRead, Function *FlushWrite,
                            Function *FlushReadArray,
                            Function *FlushWriteArray) {
  Function *ReqFlush = CompilationUtils::isReadPipeBuiltin(
      PipeCall->getCalledFunction()->getName()) ? FlushRead
                                                : FlushWrite;

  Function *ReqFlushArray =
      CompilationUtils::isReadPipeBuiltin(
          PipeCall->getCalledFunction()->getName()) ? FlushReadArray
                                                    : FlushWriteArray;

  bool Changed = false;

  auto *F = PipeCall->getParent()->getParent();
  for (auto &BB: *F) {
    auto *Term = BB.getTerminator();
    assert(Term && "Ill-formed BasicBlock.");
    if (!isa<ReturnInst>(Term)) {
      continue;
    }

    assert(PipeCall->getNumArgOperands() > 0
           && "Unexpected number of arguments");

    Value *PipeArg = PipeCall->getArgOperand(0)->stripPointerCasts();

    IRBuilder<> Builder(Term);

    bool IsAnArrayOfPipes = false;
    size_t NumArrayElements = 0;

    // pipe can either be a function argument, or a global variable
    // for global we cannot use the PipeArg, because it can be from another block
    if (auto *Load = dyn_cast<LoadInst>(PipeArg)) {
      Value *PipeGlobalPtr = Load->getPointerOperand();
      // Pipe global variable can be either an array of pipes
      if (auto *GEP = dyn_cast<GEPOperator>(PipeGlobalPtr)) {
        PipeGlobalPtr = GEP->getPointerOperand();
        PipeArg = PipeGlobalPtr;

        IsAnArrayOfPipes = true;
        ArrayType *PipeGlobalArrayTy = cast<ArrayType>(
            cast<PointerType>(PipeGlobalPtr->getType())->getElementType());
        NumArrayElements = CompilationUtils::getArrayNumElements(
            PipeGlobalArrayTy);
      } else {
        PipeArg = Builder.CreateLoad(PipeGlobalPtr);
      }
    }

    if (IsAnArrayOfPipes) {
      Type *FlushArgTy = ReqFlushArray->getFunctionType()->getParamType(0);

      Value *FlushArgs[] = {
        Builder.CreateBitCast(PipeArg, FlushArgTy),
        ConstantInt::get(Type::getInt32Ty(M.getContext()), NumArrayElements)
      };

      Builder.CreateCall(ReqFlushArray, FlushArgs);
      Changed = true;
    } else {
      Type *FlushArgTy = ReqFlush->getFunctionType()->getParamType(0);

      Value *FlushArgs[] = {
        Builder.CreateBitCast(PipeArg, FlushArgTy)
      };

      Builder.CreateCall(ReqFlush, FlushArgs);
      Changed = true;
    }
  }

  assert(Changed && "PipeSupport have not inserted a flush call!");
  return Changed;
}

bool PipeSupport::runOnModule(Module &M) {
  BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfo>();
  SmallVector<Module *, 2> BuiltinModules = BLI.getBuiltinModules();

  SmallVector<CallInst *, 32> PipeCalls;
  for (auto &F : M) {
    findPipeCalls(F, PipeCalls);
  }

  if (PipeCalls.empty()) {
    return false;
  }

  Function *FlushRead = importRTLFunctionDecl(M, BuiltinModules,
                                              "__flush_read_pipe");

  Function *FlushWrite = importRTLFunctionDecl(M, BuiltinModules,
                                               "__flush_write_pipe");

  Function *FlushReadArray = importRTLFunctionDecl(M, BuiltinModules,
                                                   "__flush_read_pipe_array");

  Function *FlushWriteArray = importRTLFunctionDecl(M, BuiltinModules,
                                                    "__flush_write_pipe_array");

  assert(FlushRead && "no '__flush_read_pipe' built-in declared in RTL");
  assert(FlushWrite && "no '__flush_write_pipe' built-in declared in RTL");
  assert(FlushReadArray &&
         "no '__flush_read_pipe_array' built-in declared in RTL");
  assert(FlushWriteArray &&
         "no '__flush_write_pipe_array' built-in declared in RTL");

  bool Changed = false;
  for (auto *Call : PipeCalls) {
    Changed |= insertFlushCall(M, Call, FlushRead, FlushWrite,
                               FlushReadArray, FlushWriteArray);
  }

  return Changed;
}

void PipeSupport::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<BuiltinLibInfo>();
}


} // namespace intel

extern "C"{
  ModulePass* createPipeSupportPass() {
    return new intel::PipeSupport();
  }
}

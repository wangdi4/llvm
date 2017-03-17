/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "PipeSupport.h"

#include <llvm/ADT/SmallString.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>

#include <BuiltinLibInfo.h>
#include <CompilationUtils.h>
#include <InitializePasses.h>
#include <OCLPassSupport.h>

using namespace llvm;

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

static Function *importRTLFuntionDecl(Module &TargetModule,
                                  const SmallVectorImpl<Module *> &RTLs,
                                  StringRef Name) {
  for (auto *BIModule : RTLs) {
    if (auto *F = BIModule->getFunction(Name)) {
      using namespace Intel::OpenCL::DeviceBackend;
      return cast<Function>(
          CompilationUtils::importFunctionDecl(&TargetModule, F));
    }
  }

  return nullptr;
}

static bool isReadPipeBI(StringRef Name) {
  return Name.equals("__read_pipe_2_intel") ||
    Name.equals("__read_pipe_2_bl_intel");
}

static bool isWritePipeBI(StringRef Name) {
  return Name.equals("__write_pipe_2_intel") ||
    Name.equals("__write_pipe_2_bl_intel");
}

static void findPipeCalls(Function &F,
                          SmallVectorImpl<CallInst *> &Calls) {

  for (auto &BB : F) {
    for (auto &I : BB) {
      if (auto *Call = dyn_cast<CallInst>(&I)) {
        StringRef Name = Call->getCalledFunction()->getName();
        if (isReadPipeBI(Name) || isWritePipeBI(Name)) {
          Calls.push_back(Call);
        }
      }
    }
  }
}

static bool insertFlushCall(CallInst *PipeCall,
                            Function *FlushRead, Function *FlushWrite) {
  Function *ReqFlush = isReadPipeBI(PipeCall->getCalledFunction()->getName())
    ? FlushRead
    : FlushWrite;

  auto *F = PipeCall->getParent()->getParent();
  auto &LastBB = F->back();
  auto *Term = LastBB.getTerminator();
  assert(Term && "Ill-formed BasicBlock.");

  assert(PipeCall->getNumArgOperands() > 1
         && "Unexpected number of arguments");

  Value *PipeArg = PipeCall->getArgOperand(0)->stripPointerCasts();
  Type *FlushArgTy = ReqFlush->getFunctionType()->getParamType(0);

  IRBuilder<> Builder(Term);

  // pipe can either be a function argument, or a global variable
  // for global we cannot use the PipeArg, because it can be from another block
  if (auto *Load = dyn_cast<LoadInst>(PipeArg)) {
    Value *PipeGlobalPtr = Load->getPointerOperand();
    PipeArg = Builder.CreateLoad(PipeGlobalPtr);
  }

  Value *FlushArgs[] = {
    Builder.CreateBitCast(PipeArg, FlushArgTy)
  };

  Builder.CreateCall(ReqFlush, FlushArgs);
  return true;
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

  Function *FlushRead = importRTLFuntionDecl(M, BuiltinModules,
                                             "__flush_read_pipe");

  Function *FlushWrite = importRTLFuntionDecl(M, BuiltinModules,
                                              "__flush_write_pipe");

  assert(FlushRead && "no '__flush_read_pipe' built-in declared in RTL");
  assert(FlushWrite && "no '__flush_write_pipe' built-in declared in RTL");

  bool Changed = false;
  for (auto *Call : PipeCalls) {
    Changed |= insertFlushCall(Call, FlushRead, FlushWrite);
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

//===----  Intel_AdvancedFastCall.cpp - Intel Advanced Fast Call   --------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass extends the cases that can be converted to use the 'fast call'
// calling convention. The GlobalOpt pass handles the setting of the 'fastcc'
// attribute on functions, but it will not mark the attribute on a function
// that is address taken. However, some of these functions may also be called
// directly. When using PGO the hottest indirect calls have been converted
// to direct calls, and performance can be improved if these use the 'fastcc'
// convention. This pass creates a separate version of the function for the
// direct call sites, so that GlobalOpt can optimize them. The direct calls
// also may become eligible for argument promotion by the converting pointer
// parameters into value parameters.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_AdvancedFastCall.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/ProfileSummaryInfo.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

// The pass normally is only enabled when profile information is present. This
// flag can force the pass to run without profiling information.
static cl::opt<bool>
    RequireProfile("intel-advancedfastcall-require-profile", cl::init(true),
                   cl::ReallyHidden,
                   cl::desc("When set, only perform the advanced fast call "
                            "transformation when there is profile data"));

#define DEBUG_TYPE "advancedfastcall"

namespace {

// Helper function to get the execution count for an instruction based on the
// intel-profx metadata, if it is present. Otherwise, returns 0.
uint64_t getCallCount(Instruction *I) {
  auto *MD = I->getMetadata(LLVMContext::MD_intel_profx);
  if (!MD)
    return 0;

  assert(MD->getNumOperands() == 2);
  ConstantInt *CI = mdconst::extract<ConstantInt>(MD->getOperand(1));
  assert(CI);
  return CI->getValue().getZExtValue();
}

//
// This class tries to replace direct calls to address taken functions with a
// function that is not address taken. It does this by creating a wrapper
// function that can be used for the cases where the function is address taken,
// which will call the original function with the same arguments. This call
// will be marked as a tail call to allow the code generator to emit a jump
// instruction in place of the call. Doing this enables the following
// optimization opportunities on direct calls:
//   1. The ability pass arguments in registers.
//   2. The ability to promote pointer arguments to by-value arguments.
//
class FastCallEnabler {
public:
  bool run(Module &M, ProfileSummaryInfo *PSI);

private:
  bool hasChangableCC(Function *F);
  bool worthChanging(Function *F);
  void convert(Function *F);
};

// This function looks for functions that we want to enable to be converted to
// use the 'fastcc' calling convention by 'GlobalOpt'. These are functions that
// are currently address taken, but meet some simple criteria that would allow
// GlobalOpt to otherwise convert them.
bool FastCallEnabler::run(Module &M, ProfileSummaryInfo *PSI) {
  // Currently, restrict this to PGO feedback compilations because in that case
  // the hottest indirect calls would have been converted to be direct calls.
  if (RequireProfile && (!PSI || !PSI->hasInstrumentationProfile()))
    return false;

  // Only do this for x86, because its default calling convention uses the
  // stack for arguments.
  Triple T(M.getTargetTriple());
  if (T.getArch() != Triple::x86)
    return false;

  // Collection of functions to convert.
  SmallPtrSet<Function *, 16> FunctionsToConvert;
  for (Module::iterator FI = M.begin(), E = M.end(); FI != E;) {
    Function *F = &*FI++;
    if (F->isDeclaration())
      continue;

    // For now only consider local functions, although because the wrapper
    // function created will have the original function name this restriction
    // could be removed in the future.
    if (!F->hasLocalLinkage())
      continue;

    // We only care about address taken functions because we are going
    // to try to create a version of the function that can be used for the
    // direct call sites, and will become 'fastcc' after GlobalOpt is run.
    if (!F->hasAddressTaken())
      continue;

    if (!hasChangableCC(F)) {
      LLVM_DEBUG(dbgs() << "AdvancedFastCall - Function not eligible: "
                        << F->getName() << "\n");
      continue;
    }

    if (!worthChanging(F)) {
      LLVM_DEBUG(dbgs() << "AdvancedFastCall - Function not worthy: "
                        << F->getName() << "\n");
      continue;
    }

    FunctionsToConvert.insert(F);
  }

  if (FunctionsToConvert.empty())
    return false;

  for (auto *F : FunctionsToConvert)
    convert(F);

  return true;
}

// This checks whether the global opt pass would allow changing the calling
// convention for a function if the function was not address taken.
bool FastCallEnabler::hasChangableCC(Function *F) {
  // Naked functions will not get 'fastcc'
  if (F->hasFnAttribute(Attribute::Naked))
    return false;

  if (F->isVarArg())
    return false;

  CallingConv::ID CC = F->getCallingConv();
  if (CC != CallingConv::C && CC != CallingConv::X86_ThisCall)
    return false;

  // GlobalOpt can't change CC of the function that either has musttail calls,
  // or is a musttail callee itself. First, check the caller of the function.
  for (User *U : F->users()) {
    CallInst *CI = dyn_cast<CallInst>(U);
    if (!CI)
      continue;

    if (CI->isMustTailCall())
      return false;
  }

  // Next check that this function does not have musttail calls.
  for (BasicBlock &BB : *F)
    if (BB.getTerminatingMustTailCall())
      return false;

  return true;
}

// Check whether it is worth creating a form of the function that can be made
// into a 'fastcc' version.
//   1. We want there to be arguments because the 'fastcc' will allow the
//      arguments to be passed in registers
//   2. and we want there to be direct function calls made to the function that
//      would benefit from the conversion.
bool FastCallEnabler::worthChanging(Function *F) {
  if (F->arg_size() == 0)
    return false;

  bool HasDirectCall = false;
  for (Use &U : F->uses()) {
    auto *Call = dyn_cast<CallBase>(U.getUser());
    if (Call && Call->isCallee(&U)) {
      HasDirectCall = true;
      break;
    }
  }

  return HasDirectCall;
}

// Create a new wrapper function and update the call sites. Note, the original
// function name will be placed on the wrapper function, and the function being
// processed will be renamed.
//
// For example:
//   define internal i32 @foo(i32 %a, i32 %b) {
//   ...
//   }
//
// Will result in the following:
//   define internal i32 @foo(i32 a, i32 b) {
//     %res = call foo.1(i32 %a, i32 %b)
//     ret i32 %res
//   }
// where the original function '@foo' has been renamed to be "@foo.1".
//
// Direct calls can be made to @foo.1, and indirect calls will still refer to
// @foo.
//
void FastCallEnabler::convert(Function *F) {
  assert(!F->isVarArg() && "Function should not be vararg");

  LLVM_DEBUG(dbgs() << "AdvancedFastCall - Processing: " << F->getName()
                    << "\n");

  // List of the parameter types for the new function.
  SmallVector<Type *, 8> Params;
  for (auto I = F->arg_begin(), E = F->arg_end(); I != E; ++I)
    Params.push_back(I->getType());

  FunctionType *FTy = F->getFunctionType();
  Type *RetTy = FTy->getReturnType();
  FunctionType *NewFTy = FunctionType::get(RetTy, Params, FTy->isVarArg());
  Function *NewF = Function::Create(NewFTy, F->getLinkage(),
                                    F->getAddressSpace(), F->getName());

  // Add the function into the module list.
  F->getParent()->getFunctionList().insert(F->getIterator(), NewF);

  // Set the function and parameter attribute lists.
  NewF->copyAttributesFrom(F);

  // Fill in the function body to just invoke the original function with all
  // the incoming arguments.
  SmallVector<Value *, 4> NewArgs;
  for (auto &Arg : NewF->args())
    NewArgs.push_back(&Arg);

  // Call to the original function.
  IRBuilder<> Builder(BasicBlock::Create(F->getContext(), "", NewF));
  CallInst *NewCall = Builder.CreateCall(F, NewArgs);
  if (!F->getFunctionType()->getReturnType()->isVoidTy())
    Builder.CreateRet(NewCall);
  else
    Builder.CreateRetVoid();

  // Mark it as a 'tailcall' so that the code generation can turn it into a
  // 'jmp' instruction.
  NewCall->setTailCallKind(CallInst::TCK_Tail);

  // Swap the names of the functions, so that the one we want to use the
  // 'fastcc' convention will have the clone name. This way any references to
  // the original function name will maintain the original function calling
  // convention.
  std::string CloneName = NewF->getName().str();
  NewF->takeName(F);
  F->setName(CloneName);
  LLVM_DEBUG(dbgs() << "AdvancedFastCall - Direct Call Function: "
                    << F->getName() << "\n");

  // Initially, set everything to use the wrapper function. Then replace the
  // direct call uses to call the function that can become a 'fastcc' version.
  F->replaceAllUsesWith(NewF);

  bool HasProfile = false;
  uint64_t DirectCallExecCount = 0;
  uint64_t OrigExecCount = 0;
  Function::ProfileCount ExecCount = F->getEntryCount();
  if (ExecCount.hasValue()) {
    HasProfile = true;
    OrigExecCount = ExecCount.getCount();
  }

  // Collect a list of users to update, and then replace them. This is to avoid
  // invalidating the iterator.
  SmallVector<CallBase *, 4> Calls;
  for (Use &U : NewF->uses()) {
    // Must be a direct call.
    auto *Call = dyn_cast<CallBase>(U.getUser());
    if (Call && Call->isCallee(&U)) {
      Calls.push_back(Call);
      DirectCallExecCount += getCallCount(Call);
    }
  }
  for (auto *Call : Calls)
    Call->setCalledFunction(F);

  if (HasProfile) {
    // Update the profile count for the wrapper function. The original function
    // maintains the existing count, because the wrapper will always call it.
    uint64_t RemainExecCount = OrigExecCount > DirectCallExecCount
                                   ? OrigExecCount - DirectCallExecCount
                                   : 0;
    NewF->setEntryCount(RemainExecCount);
  }
}

// Legacy pass manager implementation
class IntelAdvancedFastCallWrapperPass : public ModulePass {
public:
  static char ID;
  IntelAdvancedFastCallWrapperPass() : ModulePass(ID) {
    initializeIntelAdvancedFastCallWrapperPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<ProfileSummaryInfoWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    ProfileSummaryInfo *PSI =
        &getAnalysis<ProfileSummaryInfoWrapperPass>().getPSI();

    FastCallEnabler Impl;
    return Impl.run(M, PSI);
  }
};

} // End anonymous namespace

char IntelAdvancedFastCallWrapperPass::ID = 0;
INITIALIZE_PASS_BEGIN(IntelAdvancedFastCallWrapperPass,
                      "intel-advancedfastcall", "Intel advanced fastcall",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(ProfileSummaryInfoWrapperPass)
INITIALIZE_PASS_END(IntelAdvancedFastCallWrapperPass, "intel-advancedfastcall",
                    "Intel advanced fastcall", false, false)

namespace llvm {

ModulePass *createIntelAdvancedFastCallWrapperPass() {
  return new IntelAdvancedFastCallWrapperPass();
}

PreservedAnalyses IntelAdvancedFastCallPass::run(Module &M,
                                                 ModuleAnalysisManager &AM) {
  ProfileSummaryInfo *PSI = &AM.getResult<ProfileSummaryAnalysis>(M);

  FastCallEnabler Impl;
  bool Changed = Impl.run(M, PSI);
  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

} // End namespace llvm

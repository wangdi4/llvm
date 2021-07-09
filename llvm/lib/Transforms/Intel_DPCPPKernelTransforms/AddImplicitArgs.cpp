//===- AddImplicitArgs.cpp - Add implicit arguments to DPC++ kernel  ------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/AddImplicitArgs.h"
#include "ImplicitArgsUtils.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DevLimits.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-add-implicit-args"

namespace {

/// Legacy AddImplicitArgs pass.
class AddImplicitArgsLegacy : public ModulePass {
  AddImplicitArgsPass Impl;

public:
  /// Pass identification, replacement for typeid
  static char ID;

  AddImplicitArgsLegacy();

  StringRef getPassName() const override { return "AddImplicitArgsLegacy"; }

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<CallGraphWrapperPass>();
    AU.addRequired<ImplicitArgsAnalysisLegacy>();
    // Depends on LocalBufferAnalysis for finding all local buffers each
    // function uses directly.
    AU.addRequired<LocalBufferAnalysisLegacy>();
    AU.setPreservesCFG();
    AU.addPreserved<CallGraphWrapperPass>();
    AU.addPreserved<ImplicitArgsAnalysisLegacy>();
  }
};

} // namespace

INITIALIZE_PASS_BEGIN(AddImplicitArgsLegacy, DEBUG_TYPE,
                      "Add implicit arguments to functions", false, false)
INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ImplicitArgsAnalysisLegacy)
INITIALIZE_PASS_DEPENDENCY(LocalBufferAnalysisLegacy)
INITIALIZE_PASS_END(AddImplicitArgsLegacy, DEBUG_TYPE,
                    "Add implicit arguments to functions", false, false)

char AddImplicitArgsLegacy::ID = 0;

AddImplicitArgsLegacy::AddImplicitArgsLegacy() : ModulePass(ID) {
  initializeAddImplicitArgsLegacyPass(*PassRegistry::getPassRegistry());
}

ModulePass *llvm::createAddImplicitArgsLegacyPass() {
  return new AddImplicitArgsLegacy();
}

bool AddImplicitArgsLegacy::runOnModule(Module &M) {
  CallGraph *CG = &getAnalysis<CallGraphWrapperPass>().getCallGraph();
  LocalBufferInfo *LBInfo =
      &getAnalysis<LocalBufferAnalysisLegacy>().getResult();
  ImplicitArgsInfo *IAInfo =
      &getAnalysis<ImplicitArgsAnalysisLegacy>().getResult();
  return Impl.runImpl(M, LBInfo, IAInfo, CG);
}

PreservedAnalyses AddImplicitArgsPass::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  CallGraph *CG = &AM.getResult<CallGraphAnalysis>(M);
  LocalBufferInfo *LBInfo = &AM.getResult<LocalBufferAnalysis>(M);
  ImplicitArgsInfo *IAInfo = &AM.getResult<ImplicitArgsAnalysis>(M);
  if (!runImpl(M, LBInfo, IAInfo, CG))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<ImplicitArgsAnalysis>();
  return PA;
}

bool AddImplicitArgsPass::runImpl(Module &M, LocalBufferInfo *LBInfo,
                                  ImplicitArgsInfo *IAInfo, CallGraph *CG) {
  this->LBInfo = LBInfo;
  this->IAInfo = IAInfo;
  this->CG = CG;

  // Clear call instruction to fix container
  FixupCalls.clear();
  // Clear functions refs to fix container
  FixupFunctionsRefs.clear();

  // Collect all module functions that are not declarations into work list.
  SmallVector<Function *, 4> WorkList;
  for (auto &F : M) {
    if (F.isDeclaration()) {
      // Function is not defined inside module.
      continue;
    }

    // global ctor's and dtor's don't need implicit arguments, as it only
    // called once by runStaticConstructorsDestructors function.
    // C++ function does not need implicit arguments.
    if (DPCPPKernelCompilationUtils::isGlobalCtorDtorOrCPPFunc(&F))
      continue;

    WorkList.push_back(&F);
  }

  // Run on all collected functions.
  for (auto *F : WorkList)
    runOnFunction(F);

  // update Metadata now.
  DPCPPKernelCompilationUtils::updateFunctionMetadata(&M, FixupFunctionsRefs);

  // Indirect calls are not users of any functions. We need to collect them
  // and add stubs for implicit arguments here
  SmallPtrSet<CallInst *, 16> IndirectCalls;
  for (const auto &It : FixupCalls) {
    CallInst *CI = It.first;
    if (!CI->getCalledFunction())
      IndirectCalls.insert(CI);
  }

  for (auto *CI : IndirectCalls) {
    Value **Args = FixupCalls[CI];
    SmallVector<Type *, 16> ArgTys;
    for (unsigned I = 0; I < ImplicitArgsUtils::NUM_IMPLICIT_ARGS; ++I)
      ArgTys.push_back(Args[I]->getType());

    replaceCallInst(CI, ArgTys, nullptr);
  }

  // Go over all call instructions that need to be changed and add implicit
  // arguments to them.
  // Call instructions already contain extra agruments, but all of them are
  // Undef's - we need to replace Undef's with the correct values.
  for (auto &It : FixupCalls) {
    CallInst *CI = It.first;
    Value **CallArgs = It.second;

    // Create new call instruction with extended arguments.
    SmallVector<Value *, 16> NewArgs;

    // Go over explicit arguments, they are currently undef and need to be
    // assigned their actual value.
    for (unsigned I = 0, E = CI->getNumArgOperands() -
                             ImplicitArgsUtils::NUM_IMPLICIT_ARGS;
         I < E; ++I)
      NewArgs.push_back(CI->getArgOperand(I));

    // Add implicit parameters.
    for (unsigned I = 0; I < ImplicitArgsUtils::NUM_IMPLICIT_ARGS; ++I)
      NewArgs.push_back(CallArgs[I]);

    CallInst *NewCI = CallInst::Create(
        FunctionCallee(CI->getFunctionType(), CI->getCalledOperand()),
        ArrayRef<Value *>(NewArgs), "", CI);
    NewCI->setCallingConv(CI->getCallingConv());
    NewCI->setTailCall(CI->getTailCallKind());
    NewCI->setDebugLoc(CI->getDebugLoc());
    // Copy attributes from the callee which contains aligment for arguments.
    if (Function *Callee = CI->getCalledFunction())
      NewCI->setAttributes(Callee->getAttributes());

    // Copy debug metadata to new function if available.
    if (CI->hasMetadata())
      NewCI->setDebugLoc(CI->getDebugLoc());

    delete[] CallArgs;

    CI->replaceAllUsesWith(NewCI);
    CI->eraseFromParent();
  }

  return true;
}

Function *AddImplicitArgsPass::runOnFunction(Function *F) {
  SmallVector<Type *, 16> NewTypes;
  SmallVector<const char *, 16> NewNames;
  SmallVector<AttributeSet, 16> NewAttrs;
  unsigned NumExplicitArgs = F->arg_size();

  // Calculate pointer to the local memory buffer. Do this before F is
  // deleted.
  size_t DirectLocalSize = LBInfo->getDirectLocalsSize(F);
  AttrBuilder B;
  B.addAttribute(Attribute::NoAlias);
  AttributeSet NoAlias = AttributeSet::get(F->getContext(), B);
  AttributeSet NoAttr;
  // For each implicit arg, setup its type, name and attributes.
  for (unsigned I = 0; I < ImplicitArgsUtils::NUM_IMPLICIT_ARGS; ++I) {
    Type *ArgType = IAInfo->getArgType(I);
    NewTypes.push_back(ArgType);
    NewNames.push_back(ImplicitArgsUtils::getArgName(I));
    // We know that all implicit args that are pointers are non-aliasing, but
    // we must be careful are review this assumption every time we add a new
    // implicit arg.
    if (ArgType->isPointerTy())
      NewAttrs.push_back(NoAlias);
    else
      NewAttrs.push_back(NoAttr);
  }
  // Create the new function with appended implicit attributes.
  Function *NewF = DPCPPKernelCompilationUtils::AddMoreArgsToFunc(
      F, NewTypes, NewNames, NewAttrs, "AddImplicitArgs");

  // maintain this map to preserve original/modified relation for functions.
  FixupFunctionsRefs[F] = NewF;

  // Apple LLVM-IR workaround
  // 1.  Pass WI information structure as the next parameter after given
  // function parameters
  // 2.  We don't want to use TLS for local memory.
  //    Our solution to move all internal local memory blocks to be allocated
  //    by the execution engine and passed within additional parameters to the
  //    kernel, those parameters are not exposed to the user.

  // Go through new function instructions and search calls.
  CG->addToCallGraph(NewF);
  for (auto &N : *(*CG)[NewF]) {
    auto *CI = cast<CallInst>(*N.first);
    // Ignore calls of inline assembly code.
    if (CI->isInlineAsm())
      continue;

    // Check call for not inlined module function.
    Function *Callee = CI->getCalledFunction();
    bool IsDirectCall = (Callee != nullptr);
    // C++ Callee will be skipped as C++ func does not add implicit arguments.
    // TODO: handle C++ function as indirect callee.
    if (IsDirectCall &&
        (Callee->isDeclaration() ||
         DPCPPKernelCompilationUtils::isGlobalCtorDtorOrCPPFunc(Callee)))
      continue;
    StringRef Name =
        IsDirectCall ? Callee->getName() : CI->getCalledOperand()->getName();
    Value **CallArgs = new Value *[ImplicitArgsUtils::NUM_IMPLICIT_ARGS];
    Function::arg_iterator IA = NewF->arg_begin();
    // Skip over explicit args.
    for (unsigned I = 0; I < NumExplicitArgs; ++I, ++IA)
      ;
    // Copy over implicit args.
    for (unsigned I = 0; I < ImplicitArgsUtils::NUM_IMPLICIT_ARGS; ++I, ++IA)
      CallArgs[I] = static_cast<Value *>(&*IA);
    assert(IA == NewF->arg_end());
    // Calculate pointer to the local memory buffer for callee.
    Value *LocalMem = CallArgs[ImplicitArgsUtils::IA_SLM_BUFFER];
    Twine ValName = "LocalMem_" + Name;
    // [LLVM 3.8 UPGRADE] ToDo: Replace nullptr for pointer type with actual
    // type (not using type from pointer as this functionality is planned o
    // be removed.
    Type *Ty = LocalMem->getType()->getScalarType()->getPointerElementType();
    Value *NewLocalMem = GetElementPtrInst::Create(
        Ty, LocalMem,
        ConstantInt::get(IntegerType::get(F->getContext(), 32),
                         DirectLocalSize),
        ValName, CI);
    CallArgs[ImplicitArgsUtils::IA_SLM_BUFFER] = NewLocalMem;

    FixupCalls[CI] = CallArgs;
  }

  // All users which need to be replaced are handled here.
  SmallVector<User *, 16> Users(F->users());
  for (User *U : Users) {
    // Handle constant expression with bitcast of function pointer
    // it handles cases like block_literal global variable definitions
    // Example of case:
    // @__block_literal_global = internal constant { ..., i8*, ... }
    //    { ..., i8* bitcast (i32 (i8*, i32)* @globalBlock_block_invoke to i8*),
    //    ... }
    if (ConstantExpr *CE = dyn_cast<ConstantExpr>(U)) {
      if ((CE->getOpcode() == Instruction::BitCast ||
           CE->getOpcode() == Instruction::AddrSpaceCast) &&
          CE->getType()->isPointerTy()) {
        // this case happens when global block variable is used.
        Constant *newCE = ConstantExpr::getPointerCast(NewF, CE->getType());
        CE->replaceAllUsesWith(newCE);
      } else if (CE->getOpcode() == Instruction::PtrToInt) {
        // function pointer is converted into an int.
        Constant *NewCE = ConstantExpr::getPtrToInt(NewF, CE->getType());
        CE->replaceAllUsesWith(NewCE);
      }
    }
    // Handle call instruction.
    else if (CallInst *CI = dyn_cast<CallInst>(U)) {
      replaceCallInst(CI, NewTypes, NewF);
    } else if (auto *CI = dyn_cast<PtrToIntInst>(U)) {
      auto *NewCE = CastInst::CreatePointerCast(NewF, CI->getType(), "", CI);
      NewCE->setDebugLoc(CI->getDebugLoc());
      CI->replaceAllUsesWith(NewCE);
      CI->eraseFromParent();
    } else if (auto *SI = dyn_cast<StoreInst>(U)) {
      assert(isa<AllocaInst>(SI->getPointerOperand()) &&
             "Expected store of function pointer into an alloca");
      // This function was stored as a function pointer, but the type of
      // function was changed (implicit args were added) - to avoid changing
      // types let's just cast new function type into the old one before
      // storing.
      auto *OldAllocaTy = cast<PointerType>(SI->getPointerOperand()->getType());
      Type *OldFPtrTy = OldAllocaTy->getElementType();

      auto *Cast = CastInst::CreatePointerCast(NewF, OldFPtrTy, "", SI);
      auto *NewSI = new StoreInst(Cast, SI->getPointerOperand(), SI);
      NewSI->setDebugLoc(SI->getDebugLoc());
      SI->replaceAllUsesWith(NewSI);
      SI->eraseFromParent();
    } else {
      // We should not be here.
      // Unhandled case except for SelectInst - they will be handled later.
      assert(isa<SelectInst>(U) && "Unhandled function reference");
    }
  }

  // It seems that removing function use (by changing its operand to another
  // function) somehow breaks data structure used to hold uses and for example
  // for two uses, the loop stops after the first one.
  // Let's store info which need to be updated and perform updates outside
  // of the loop over function uses.
  DenseMap<User *, std::pair<unsigned, Value *>> UsersToReplace;

  // All users which are not to be replaced are handled here.
  for (Use &U : F->uses()) {
    User *Usr = U.getUser();
    if (auto *SI = dyn_cast<SelectInst>(Usr)) {
      unsigned OpNo = U.getOperandNo();
      // This function goes though a select instruction, but the type of the
      // function was changed (implicit args were added) - to avoid changing
      // types let's just cast new function type into the old one before
      // select.
      if (SI->getOperand(OpNo)->getType() != NewF->getType()) {
        auto *Cast = CastInst::CreatePointerCast(
            NewF, SI->getOperand(OpNo)->getType(), "", SI);
        UsersToReplace[SI] = {OpNo, Cast};
      }
    }
  }

  for (const auto &I : UsersToReplace) {
    const std::pair<unsigned, Value *> &R = I.second;
    I.first->setOperand(R.first, R.second);
  }

  for (User *U : NewF->users()) {
    if (CallInst *I = dyn_cast<CallInst>(U)) {
      // Change the calling convention of the call site to match the
      // calling convention of the called function.
      I->setCallingConv(NewF->getCallingConv());
    }
  }

  return NewF;
}

void AddImplicitArgsPass::replaceCallInst(CallInst *CI,
                                          ArrayRef<Type *> ImplicitArgsTypes,
                                          Function *NewF) {
  bool FixupCallsNeedsUpdate = FixupCalls.count(CI) > 0;
  Value **CallArgs = nullptr;
  if (FixupCallsNeedsUpdate) {
    // Remove the old call from the mapping.
    CallArgs = FixupCalls[CI];
    FixupCalls.erase(CI);
  }
  SmallVector<Value *, 16> NewArgs;
  // Push undefs for new arguments.
  assert(ImplicitArgsTypes.size() == ImplicitArgsUtils::NUM_IMPLICIT_ARGS);
  for (unsigned I = 0; I < ImplicitArgsUtils::NUM_IMPLICIT_ARGS; ++I) {
    NewArgs.push_back(UndefValue::get(ImplicitArgsTypes[I]));
  }
  CallInst *NewCI =
      NewF
          ? DPCPPKernelCompilationUtils::AddMoreArgsToCall(CI, NewArgs, NewF)
          : DPCPPKernelCompilationUtils::addMoreArgsToIndirectCall(CI, NewArgs);

  // Place the new call into the mapping.
  if (FixupCallsNeedsUpdate)
    FixupCalls[NewCI] = CallArgs;
}

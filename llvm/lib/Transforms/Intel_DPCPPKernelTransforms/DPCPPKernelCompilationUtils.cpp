//===-- DPCPPKernelCompilationUtils.cpp - Function definitions -*- C++ ----===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelBarrierUtils.h"

using namespace llvm;

namespace llvm {

void DPCPPKernelCompilationUtils::moveAllocaToEntry(BasicBlock *FromBB,
                                                    BasicBlock *EntryBB) {
  // This implementation is only correct when ToBB is an entry block.
  llvm::SmallVector<AllocaInst *, 8> Allocas;
  for (auto &I : *FromBB)
    if (auto *AI = dyn_cast<AllocaInst>(&I))
      Allocas.push_back(AI);

  if (EntryBB->empty()) {
    IRBuilder<> Builder(EntryBB);
    for (auto *AI : Allocas) {
      AI->removeFromParent();
      Builder.Insert(AI);
    }
    return;
  }

  Instruction *InsPt = EntryBB->getFirstNonPHI();
  assert(InsPt && "At least one non-PHI insruction is expected in ToBB");
  for (auto *AI : Allocas) {
    AI->moveBefore(InsPt);
  }
}

void DPCPPKernelCompilationUtils::getAllSyncBuiltinsDecls(FuncSet &FuncSet,
                                                          Module *M) {
  // Clear old collected data!
  FuncSet.clear();

  // TODO: port handling of WG collectives here as well
  auto *F = M->getFunction(BarrierName);

  if (F && F->isDeclaration())
    FuncSet.insert(F);
}

Function *DPCPPKernelCompilationUtils::AddMoreArgsToFunc(
    Function *F, ArrayRef<Type *> NewTypes, ArrayRef<const char *> NewNames,
    ArrayRef<AttributeSet> NewAttrs, StringRef Prefix) {
  assert(NewTypes.size() == NewNames.size());
  assert(NewTypes.size() == NewAttrs.size());
  // Initialize with all original arguments in the function sugnature.
  SmallVector<llvm::Type *, 16> Types;

  for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
       I != E; ++I) {
    Types.push_back(I->getType());
  }
  Types.append(NewTypes.begin(), NewTypes.end());
  FunctionType *NewFTy = FunctionType::get(F->getReturnType(), Types, false);
  // Change original function name.
  std::string Name = F->getName().str();
  F->setName("__" + F->getName() + "_before." + Prefix);
  // Create a new function with explicit and implict arguments types
  Function *NewF =
      Function::Create(NewFTy, F->getLinkage(), Name, F->getParent());
  // Copy old function attributes (including attributes on original arguments)
  // to new function.
  NewF->copyAttributesFrom(F);
  NewF->copyMetadata(F, 0);
  // Set original arguments' names.
  Function::arg_iterator NewI = NewF->arg_begin();
  for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
       I != E; ++I, ++NewI) {
    NewI->setName(I->getName());
  }
  // Set new arguments' names.
  for (unsigned I = 0, E = NewNames.size(); I < E; ++I, ++NewI) {
    Argument *A = &*NewI;
    A->setName(NewNames[I]);
    if (!NewAttrs.empty())
      for (auto Attr : NewAttrs[I])
        A->addAttr(Attr);
  }
  // Since we have now created the new function, splice the body of the old
  // function right into the new function, leaving the old body of the function
  // empty.
  NewF->getBasicBlockList().splice(NewF->begin(), F->getBasicBlockList());
  assert(F->isDeclaration() &&
         "splice did not work, original function body is not empty!");

  // Set DISubprogram as an original function has. Do it before delete body
  // since DISubprogram will be deleted too.
  NewF->setSubprogram(F->getSubprogram());

  // Delete original function body - this is needed to remove linkage (if
  // exists).
  F->deleteBody();
  // Loop over the argument list, transferring uses of the old arguments over to
  // the new arguments.
  for (Function::arg_iterator I = F->arg_begin(), E = F->arg_end(),
                              NI = NewF->arg_begin();
       I != E; ++I, ++NI) {
    // Replace the users to the new version.
    I->replaceAllUsesWith(&*NI);
  }

  // Make NewF a kernel instead of F.
  F->removeFnAttr("sycl_kernel");

  return NewF;
}

CallInst *DPCPPKernelCompilationUtils::AddMoreArgsToCall(
    CallInst *OldC, ArrayRef<Value *> NewArgs, Function *NewF) {
  assert(OldC && "CallInst is NULL");
  assert(NewF && "function is NULL");
  assert(OldC->getNumArgOperands() + NewArgs.size() == NewF->arg_size() &&
         "Function argument number mismatch");

  SmallVector<Value *, 16> Args;
  for (unsigned I = 0, E = OldC->getNumArgOperands(); I != E; ++I)
    Args.push_back(OldC->getArgOperand(I));
  Args.append(NewArgs.begin(), NewArgs.end());

  // Replace the original function with a call
  CallInst *NewC = CallInst::Create(NewF, Args, "", OldC);

  // Copy debug metadata to new function if available
  if (OldC->hasMetadata()) {
    NewC->setDebugLoc(OldC->getDebugLoc());
  }

  OldC->replaceAllUsesWith(NewC);
  // Need to erase from parent to make sure there are no uses for the called
  // function when we delete it
  OldC->eraseFromParent();
  return NewC;
}

} // end namespace llvm

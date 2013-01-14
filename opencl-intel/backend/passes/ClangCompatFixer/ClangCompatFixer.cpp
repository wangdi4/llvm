/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "ClangCompatFixer.h"

#include "llvm/Constants.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/LLVMContext.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/Support/InstIterator.h"

extern "C" {
  /// @brief Creates new PreventDivCrashes module pass
  /// @returns new PreventDivCrashes module pass  
  void* createClangCompatFixerPass() {
    return new Intel::OpenCL::DeviceBackend::ClangCompatFixer();
  }
}

/// Register pass to for opt
static llvm::RegisterPass<Intel::OpenCL::DeviceBackend::ClangCompatFixer> ClangCompatFixerPass("clang-compat-fixer", "Fix clang output incompatabilities");


namespace Intel { namespace OpenCL { namespace DeviceBackend {

char ClangCompatFixer::ID = 0;

bool ClangCompatFixer::runOnModule(llvm::Module &M) {
  bool bChanged = false;
  for (Module::iterator F = M.begin(), FE = M.end(); F != FE; ++F) {
    bChanged |= handleFMAIntrinsics(*F);
  }
  
  return bChanged;
}

bool ClangCompatFixer::handleFMAIntrinsics(Function &F) {
  
  std::vector<Instruction *> toDelete;
  for (Function::iterator bbit = F.begin(), bbe = F.end(); bbit != bbe; ++bbit) {
    for(BasicBlock::iterator i = bbit->begin(), ie = bbit->end(); ie != i; ++i) {
      IntrinsicInst *Intrin = dyn_cast<IntrinsicInst>(i);
      
      if (!Intrin)
        continue;
      
      if (Intrin->getIntrinsicID() ==  Intrinsic::fmuladd) {
        Value* Mul0 = Intrin->getArgOperand(0);
        Value* Mul1 = Intrin->getArgOperand(1);
        Value* Add0 = Intrin->getArgOperand(2);
        Instruction* MulResult = BinaryOperator::CreateFMul(Mul0, Mul1, "splitfma", Intrin);
        Instruction* AddResult = BinaryOperator::CreateFAdd(MulResult, Add0, "splitfma", Intrin);
        Intrin->replaceAllUsesWith(AddResult);
        toDelete.push_back(Intrin);
      }
    }
  }
  
  
  for (std::vector<Instruction*>::iterator i = toDelete.begin(), e = toDelete.end(); i != e; ++i) {
    (*i)->eraseFromParent();
  }
    
  return (toDelete.size() > 0);
}
  
}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {
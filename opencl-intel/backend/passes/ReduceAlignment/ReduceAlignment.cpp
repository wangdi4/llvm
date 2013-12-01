/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "ReduceAlignment.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include <string>


using namespace llvm;

extern "C" {
  /// @brief Creates new ReduceAlignment pass
    void* createReduceAlignmentPass() {
       return new intel::ReduceAlignment();
  }
}
namespace intel {

  char ReduceAlignment::ID = 0;

  /// Register pass to for opt
  OCL_INITIALIZE_PASS(ReduceAlignment, "reduce-alignment", "Reduce the alignment of all loads and stores to 1", false, false)

  bool ReduceAlignment::runOnFunction(Function &F) {
       //hasChange- true if the pass change anything, false otherwise
       bool hasChange = false;
       for (inst_iterator i = inst_begin(F), e = inst_end(F); i != e; ++i) {
           switch ((&*i)->getOpcode())
           {
             case (Instruction::Load):
                cast<llvm::LoadInst>(&*i)->setAlignment(1);
                hasChange = true;
                break;
             case (Instruction::Store):
                cast<llvm::StoreInst>(&*i)->setAlignment(1);
                hasChange = true;
                break;
           }
      }
      return hasChange;
  } //runOnFunction
}//namespace intel

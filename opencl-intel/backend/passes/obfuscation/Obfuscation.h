/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __OBFUSCATION_H__
#define __OBFUSCATION_H__

#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

namespace intel {

///////////////////////////////////////////////////////////////////////////////
// @name  Obfuscation
// @brief Obfuscate the names of variables, to prevent possible IP leaks.
///////////////////////////////////////////////////////////////////////////////
class Obfuscation : public llvm::ModulePass {
public:
  static char ID;

  Obfuscation();

  virtual llvm::StringRef getPassName() const {
    return "Module Obfuscator";
}

  bool runOnModule(llvm::Module&);
}; //End class Obfuscation

}

#endif

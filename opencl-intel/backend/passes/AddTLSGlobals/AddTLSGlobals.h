// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
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

#ifndef __ADD_TLS_GLOBALS_H__
#define __ADD_TLS_GLOBALS_H__
#include "ImplicitArgsAnalysis/ImplicitArgsAnalysis.h"
#include "LocalBuffAnalysis/LocalBuffAnalysis.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"

namespace intel {

using namespace llvm;

/// @brief  AddTLSGlobals class adds TLS global variables to the module
class AddTLSGlobals : public ModulePass {

public:
  /// Pass identification
  static char ID;

  /// @brief Constructor
  AddTLSGlobals();

  /// @brief Provides name of pass
  virtual llvm::StringRef getPassName() const override {
    return "AddTLSGlobals";
  }

  /// @brief LLVM Module pass entry
  /// @param M Module to transform
  /// @returns true if changed
  bool runOnModule(Module &M) override;

  /// @brief LLVM Interface
  /// @param AU Analysis
  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    // Depends on LocalBuffAnalysis for finding all local buffers each function
    // uses directly
    AU.addRequired<LocalBuffAnalysis>();
    AU.addRequired<ImplicitArgsAnalysis>();
  }

private:
  /// @brief Add updates to pLocalMemBase before and after each call instruction
  /// in the function
  /// @param pFunc The function to handle
  void runOnFunction(Function *pFunc);

  /// @brief The llvm module this pass needs to update
  Module *m_pModule;
  /// @brief The LocalBuffAnalysis pass, on which the current pass depends
  LocalBuffAnalysis *m_localBuffersAnalysis;
  /// @brief The ImplicitArgsAnalysis pass, on which the current pass depends
  ImplicitArgsAnalysis *m_IAA;
  /// @brief The llvm context
  LLVMContext *m_pLLVMContext;
  /// @brief Local memory pointer TLS global
  GlobalVariable *m_pLocalMemBase;
};

} // namespace intel

#endif // __ADD_TLS_GLOBALS_H__

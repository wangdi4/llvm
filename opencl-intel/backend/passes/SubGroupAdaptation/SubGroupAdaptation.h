// INTEL CONFIDENTIAL
//
// Copyright 2015-2018 Intel Corporation.
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

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include <string>

namespace intel {

/// @brief This pass replaces sub-group built-ins with appropriate IR sequences
/// It goes over all sub-group built-in declarations in a module in order
/// and substitutes with a call to work-group built-in or a constant.
class SubGroupAdaptation : public llvm::ModulePass {

public:
  static char ID;

  SubGroupAdaptation() : ModulePass(ID){};

  virtual llvm::StringRef getPassName() const { return "SubGroupAdaptation"; }

  virtual bool runOnModule(llvm::Module &M);

private:
  llvm::Module *m_pModule;

  llvm::LLVMContext *m_pLLVMContext;

  llvm::IntegerType *m_pSizeT;

  void replaceFunction(llvm::Function *oldFunc, std::string newFuncName);
  void replaceWithConst(llvm::Function *oldFunc, unsigned constInt,
                        bool isSigned);

  // Generate sub_group_broudcast body
  void defineSubGroupBroadcast(llvm::Function *pFunc);

  // Helper for WI function call generation.
  // Generates a call to WI function upon its name and dimension index
  llvm::CallInst *getWICall(llvm::BasicBlock *pAtEnd, char const *instName,
                            std::string funcName, unsigned dimIdx);
};
}

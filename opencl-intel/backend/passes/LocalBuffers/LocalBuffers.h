// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef __LOCAL_BUFFERS_H__
#define __LOCAL_BUFFERS_H__

#include "LocalBuffAnalysis.h"

#include "llvm/IR/DebugInfo.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>

#include <map>
#include <set>
#include <vector>

using namespace llvm;

namespace intel{

  /// LocalBuffers pass handles implicit local variables
  class LocalBuffers : public ModulePass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;

    /// @brief Constructor
    LocalBuffers(bool useTLSGlobals = false);

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const override {
      return "LocalBuffers";
    }

    /// @brief execute pass on given module
    /// @param M module to optimize
    /// @returns True if module was modified
    bool runOnModule(Module &M) override;

    /// @brief Inform about usage/mofication/dependency of this pass
    virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<LocalBuffAnalysis>();
    }

  protected:
    /// @brief Resolves the internal local variables and map them to local buffer
    /// @param pFunc The function which needs it handle its implicite local variables
    void runOnFunction(Function *pFunc);

    /// @brief Resolves the internal local variables and map them to local buffer
    /// @param pFunc The function which needs it handle its implicite local variables
    void parseLocalBuffers(Function *pFunc, Value *pLocalMem);

    /// @brief Replaces all uses of Constant `From` with `To`. We can't simply
    ///        call `From->replaceAllUsesWith` because the users of `From` may
    ///        be ConstantExpr, ConstantVector, ConstantStruct. We have to
    ///        convert those Constant uses to instructions first.
    /// @param From The constant value needs to be replaced
    /// @param To The replacement target value
    void ReplaceAllUsesOfConstantWith(Constant *From, Value *To);

    /// @brief Create instructions (GEP, insertvalue, etc.) to generate a Value
    ///        of same semantic with the original Constant `C`. Also replace
    ///        `From` with `To` during the instruction creation.
    /// @param C The constant to be converted as instructions
    /// @param From Should be one of the operands of `C`
    /// @return The generated Value which should be equivalent as `C`
    Value *CreateInstructionFromConstantWithReplacement(Constant *C,
                                                        Value *From, Value *To);

    /// @brief Copies DebugInfo of `GV` to Local Memory Buffer `pLocalMem`,
    ///        with corresponding `offset`.
    void AttachDebugInfoToLocalMem(GlobalVariable *GV, Value *pLocalMem,
                                   unsigned offset);

    /// @brief At the end of this pass, the GlobalVariables (__local) DebugInfo
    ///        should be removed from DICompileUnit's "globals" field, so that
    ///        the created Local Debug Variables are visible to the debugger.
    void UpdateDICompileUnitGlobals();

  protected:
    /// @brief The llvm current processed module
    Module                     *m_pModule;
    /// @brief The llvm context
    LLVMContext                *m_pLLVMContext;
    /// @brief instance of LocalBuffAnalysis pass
    LocalBuffAnalysis       *m_localBuffersAnalysis;

    /// @brief use TLS globals instead of implicit arguments
    bool m_useTLSGlobals;

    /// @brief save the first instruction as insert point for current function
    Instruction *m_pInsertPoint;

    /// @brief the DISubprogram of current function
    ///        when this equals to `nullptr`, then no need to handle debug info
    DISubprogram *m_pSubprogram;

    /// @brief help to find all compile units in the module
    DebugInfoFinder m_DIFinder;

    /// @brief stores all the DIGlobalVariableExpression's need to be removed
    ///        in DICompileUnit.globals
    SmallPtrSet<DIGlobalVariableExpression *, 4> m_GVEToRemove;

    /// @brief stores all the GlobalVariable's need to be removed
    SmallPtrSet<GlobalVariable *, 4> m_GVToRemove;
  };

} // namespace intel

#endif // __LOCAL_BUFFERS_H__

/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __LOCAL_BUFFERS_H__
#define __LOCAL_BUFFERS_H__

#include "LocalBuffAnalysis.h"

#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>

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

    /// @brief Constructor with debug parameter
    /// @param isNatveiDBG true if native debug set
    LocalBuffers(bool isNativeDBG);

    /// @brief Provides name of pass
    virtual StringRef getPassName() const {
      return "LocalBuffers";
    }

    /// @brief execute pass on given module
    /// @param M module to optimize
    /// @returns True if module was modified
    bool runOnModule(Module &M);

    /// @brief Inform about usage/mofication/dependency of this pass
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<LocalBuffAnalysis>();
    }

  protected:
    /// @brief Resolves the internal local variables and map them to local buffer
    /// @param pFunc The function which needs it handle its implicite local variables
    void runOnFunction(Function *pFunc);

    /// @brief Resolves the internal local variables and map them to local buffer
    /// @param pFunc The function which needs it handle its implicite local variables
    void parseLocalBuffers(Function *pFunc, Argument *pLocalMem);

    /// @brief TODO: add comments
    bool ChangeConstant(Value *pTheValue, Value *pUser, Instruction *pBC, Instruction *Where);

    /// @brief TODO: add comments
    Instruction* CreateInstrFromConstant(Constant *pCE, Value *From, Value *To, std::vector<Instruction*> *InstInsert);

    /// @brief Iterates over all basic blocks for a function looking for
    ///        DebugStack.() call. The calls are deleted and basic blocks
    ///        containing these calls are added to a set.
    /// @param pFunc The function to iterate over for its basic blocks
    void updateUsageBlocks(Function *pFunc);

  protected:
    /// @brief The llvm current processed module
    Module                     *m_pModule;
    /// @brief The llvm context
    LLVMContext                *m_pLLVMContext;
    /// @brief instance of LocalBuffAnalysis pass
    LocalBuffAnalysis       *m_localBuffersAnalysis;

    /// @brief vector of llvm instructions
    typedef std::vector<llvm::Instruction*> TInstVector;

    /// @brief set of basic blocks which need copying of globals into the
    ///        stack for debugging
    std::set<llvm::BasicBlock*> m_basicBlockSet;

    /// @brief true if and only if we are running in native (gdb) dbg mode
    bool m_isNativeDBG;
  };

} // namespace intel

#endif // __LOCAL_BUFFERS_H__

/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __ADD_IMPLICIT_ARGS_H__
#define __ADD_IMPLICIT_ARGS_H__

#include "LocalBuffAnalysis/LocalBuffAnalysis.h"
#include "ImplicitArgsAnalysis/ImplicitArgsAnalysis.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include <set>
#include <map>

namespace intel {

  using namespace llvm;

  /// @brief  AddImplicitArgs class adds the implicit arguments to signature
  ///         of all function of the module (that are defined inside the module)
  class AddImplicitArgs : public ModulePass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;

    /// @brief Constructor
    AddImplicitArgs();

    /// @brief Provides name of pass
    virtual StringRef getPassName() const {
      return "AddImplicitArgs";
    }

    /// @brief LLVM Module pass entry
    /// @param M Module to transform
    /// @returns true if changed
    bool runOnModule(Module &M);

    /// @brief LLVM Interface
    /// @param AU Analysis
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      // Depends on LocalBuffAnalysis for finding all local buffers each function uses directly
      AU.addRequired<LocalBuffAnalysis>();
      AU.addRequired<ImplicitArgsAnalysis>();
    }

  protected:
    /// @brief Replaces the given function with a copy function that receives
    ///        the implicit arguments, maps call instructions that appear in
    ///        the given funciton and need to add implicit arguments to its
    ///        original arguments, i.e. calls to functions define in module.
    /// @param pFunc The function to create a copy of
    /// @param isAKernel true is the gevin function is a kernel
    /// @returns The new function that receives the implicit arguemnts
    Function* runOnFunction(Function *pFunc, bool isAKernel);

    /// @brief Updates metadata nodes with new Function signature
    /// @param pMetadata The current metadata node
    /// @param visited set with metadata we alreay visit.
    void iterateMDTree(MDNode* pMetadata, std::set<MDNode *> &visited);

    /// @brief helper function. replaces call instruction with call instruction
    ///        that receives implicit arguments
    /// @param CI pointer to CallInst
    /// @param newArgsVec arguments of new function with implicit arguments added
    /// @param pNewF function with implicit arguments added
    void replaceCallInst(CallInst *CI, ArrayRef<Type *> newArgs, Function * pNewF);

  private:
    /// @brief The llvm module this pass needs to update
    Module                     *m_pModule;

    /// @brief The LocalBuffAnalysis pass, on which the current pass depends
    LocalBuffAnalysis       *m_localBuffersAnalysis;
    /// @brief The ImplicitArgsAnalysis pass, on which the current pass depends
    ImplicitArgsAnalysis    *m_IAA;
    /// @brief The llvm context
    LLVMContext                *m_pLLVMContext;

    /// @brief Maps call instructions to the implicit arguments needed to patch up the call
    std::map<llvm::CallInst *, llvm::Value **> m_fixupCalls;

    Type* m_struct_WorkDim;

    Function* m_pFunc;
    Function* m_pNewF;
  };

} // namespace intel

#endif // __ADD_IMPLICIT_ARGS_H__

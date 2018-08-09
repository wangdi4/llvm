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
    virtual llvm::StringRef getPassName() const {
      return "AddImplicitArgs";
    }

    /// @brief LLVM Module pass entry
    /// @param M Module to transform
    /// @returns true if changed
    bool runOnModule(Module &M) override;

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
    /// @returns The new function that receives the implicit arguemnts
    Function* runOnFunction(Function *pFunc);

    /// @brief helper function. replaces call instruction with call instruction
    ///        that receives implicit arguments
    /// @param CI pointer to CallInst
    /// @param newArgsVec arguments of new function with implicit arguments added
    /// @param pNewF function with implicit arguments added
    void replaceCallInst(CallInst *CI, ArrayRef<Type *> newArgs, Function * pNewF);

    /// @brief Updates metadata nodes with new Function signature
    /// @param pMetadata The current metadata node
    /// @param visited set with metadata we alreay visit.
    void iterateMDTree(MDNode* pMDNode, std::set<MDNode*> &visited);

    /// @brief Update Metadata after transformations were made.
    void updateMetadata();

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

    /// @brief Maps the original and modified Function with implicit args
    llvm::DenseMap<llvm::Function *, llvm::Function *> m_fixupFunctionsRefs;

    Type* m_struct_WorkDim;
  };

} // namespace intel

#endif // __ADD_IMPLICIT_ARGS_H__

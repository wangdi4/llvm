/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "BarrierUtils.h"
#include "DataPerBarrierPass.h"

#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/DIBuilder.h>

#include <vector>

using namespace llvm;

namespace intel {

  /// @brief Inserts debugging information for native (gdb) OpenCL debugging.
  class ImplicitGlobalIdPass : public ModulePass {

  public:
    static char ID;

    /// @brief C'tor
    ImplicitGlobalIdPass();

    /// @brief D'tor
    ~ImplicitGlobalIdPass() {}

    /// @brief Provides name of pass
    virtual StringRef getPassName() const {
      return "Intel OpenCL Implicit GID Debugging Pass";
    }

    /// @brief execute pass on given module
    /// @param M module to update
    /// @returns True if module was modified
    virtual bool runOnModule(Module& M);

    /// @brief Inform about usage/modification/dependency of this pass
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<DataPerBarrier>();
    }

  private:
    /// @brief execute pass on given function
    /// @param F Function to modify
    /// @returns True if function was modified
    virtual bool runOnFunction(Function &F);

    /// @brief execute pass on given basic block
    /// @param i global ID dimension
    /// @param pGIDAlloca pointer to allocation of GID variable
    /// @param firstInstr Instruction to insert other instructions before
    void runOnBasicBlock(unsigned i, Instruction *pGIDAlloca, Instruction* insertBefore/*, DIVariable GlobalIdVar*/);

    /// @brief Adds instructions to the beginning of the given function to compute the
    ///  global IDs for 3 dimensions. Fills in the FunctionContext.
    /// @param pFunc Function to modify
    void insertComputeGlobalIds(Function* pFunc);

    /// @brief Gets or create unsigned long debug info tyoe
    /// returns DIType unsigned long DIType
    DIType* getOrCreateUlongDIType() const;

    /// @brief Iterates instructions in a basic block and tries to find the first
    ///  instruction with scope and loc information and return these.
    /// @param BB Basic block to get the scope and loc from
    /// @param scope scope information
    /// @param loc debug location
    /// @returns bool True if a scope and loc were found
    bool getBBScope(const BasicBlock& BB, DIScope** scope, DebugLoc& loc);

  private:
    /// This is barrier utility class
    BarrierUtils m_util;

    /// This holds the data per barrier analysis pass
    DataPerBarrier* m_pDataPerBarrier;

    /// This holds the processed module
    Module* m_pModule;

    /// This holds the debug info builder
    std::unique_ptr<DIBuilder> m_pDIB;

    /// This holds the processed module context
    LLVMContext* m_pContext;

    /// This holds debug information for a module which can be queried
    DebugInfoFinder m_DbgInfoFinder;

    /// This holds the set of barrier sync instructions
    TInstructionSet* m_pSyncInstSet;
  };
}

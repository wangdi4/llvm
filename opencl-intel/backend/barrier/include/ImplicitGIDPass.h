// Copyright 2012-2021 Intel Corporation.
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

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DataPerBarrierPass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/KernelBarrierUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SubgroupEmulation/SGHelper.h"

#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/DIBuilder.h>

#include <memory>

using namespace llvm;

namespace intel {

  /// @brief Inserts debugging information for native (gdb) OpenCL debugging.
  class ImplicitGlobalIdPass : public ModulePass {

  public:
    static char ID;

    /// @brief C'tor
    ImplicitGlobalIdPass(bool HandleBarrier);

    /// @brief C'tor
    ImplicitGlobalIdPass() : ImplicitGlobalIdPass(true) {}

    /// @brief D'tor
    ~ImplicitGlobalIdPass() {}

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const override {
      return "Intel OpenCL Implicit GID Debugging Pass";
    }

    /// @brief execute pass on given module
    /// @param M module to update
    /// @returns True if module was modified
    virtual bool runOnModule(Module &M) override;

    /// @brief Inform about usage/modification/dependency of this pass
    virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<DataPerBarrierWrapper>();
      AU.addPreserved<DataPerBarrierWrapper>();
    }

  private:
    /// @brief execute pass on given function
    /// @param F Function to modify
    /// @returns True if function was modified
    virtual bool runOnFunction(Function &F);

    /// @brief Insert alloca and llvm.dbg.declare instructions of implicit GID
    /// variables
    /// @param F Function to modify
    /// @param HasSyncInst Function has dummy_barrier or barrier calls
    /// @param HasSyncInst Function has dummy_sg_barrier or sg_barrier calls
    void insertGIDAlloca(Function &F, bool HasSyncInst, bool HasSGSyncInst);

    /// @brief Insert store instructions of implicit GID variables for given
    /// function
    /// @param F Function to modify
    /// @param HasSyncInst Function has dummy_barrier or barrier calls
    /// @param HasSyncInst Function has dummy_sg_barrier or sg_barrier calls
    void insertGIDStore(Function &F, bool HasSyncInst, bool HasSGSyncInst);

    /// @brief Insert store instructions of implicit GID variables at given
    /// insert point
    /// @param B IRBuilder reference
    /// @param InsertPoint Instruction insert point
    void insertGIDStore(IRBuilder<> &B, Instruction *InsertPoint);

    /// @brief Gets or create Ind debug info tyoe
    /// returns Ind DIType
    DIType *getOrCreateIndDIType() const;

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

    /// This is subgroup emulation helper class
    SGHelper m_SGHelper;

    /// This holds the set of functions containing subgroup barrier
    FuncSet m_SGSyncFuncSet;

    /// This holds the insert point at entry block for the running function
    Instruction *m_pInsertPoint;

    /// This holds the GID allocas
    Instruction *m_pGIDAllocas[3];

    /// This holds the Ind DIType for GID variables
    DIType *m_IndDIType;

    /// Handle barrier if true, kernels without barrier otherwise.
    bool m_handleBarrier;

    /// Skip insertDeclare if true.
    bool m_skipInsertDbgDeclare;
  };
}

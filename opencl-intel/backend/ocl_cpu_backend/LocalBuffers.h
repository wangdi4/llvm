/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  LocalBuffers.h

\*****************************************************************************/

#ifndef __LOCAL_BUFFERS_H__
#define __LOCAL_BUFFERS_H__

#include "TLLVMKernelInfo.h"
#include "LocalBuffersAnalysis.h"

#include <llvm/Pass.h>
#include <llvm/Module.h>
#include <llvm/Instructions.h>

#include <map>

using namespace llvm;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  /// LocalBuffers pass handles implicit local variables
  class LocalBuffers : public ModulePass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;

    /// @brief Constructor
    LocalBuffers() : ModulePass(ID) {}

    /// @brief execute pass on given module
    /// @param M module to optimize
    /// @returns True if module was modified
    bool runOnModule(Module &M);

    /// @brief Inform about usage/mofication/dependency of this pass
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<LocalBuffersAnalysis>();
    }

    friend void getKernelInfoMap(ModulePass *pPass, std::map<const Function*, TLLVMKernelInfo>& infoMap);

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

  protected:
    /// @brief The llvm current processed module
    Module                     *m_pModule;
    /// @brief The llvm context
    LLVMContext                *m_pLLVMContext;
    /// @brief instance of LocalBuffersAnalysis pass
    LocalBuffersAnalysis       *m_localBuffersAnalysis;

    /// @brief map between kernel and its kernel info
    std::map<const Function*, TLLVMKernelInfo>  m_mapKernelInfo;
  };
  
}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __LOCAL_BUFFERS_H__
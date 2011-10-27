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

File Name:  ResolveWICall.h

\*****************************************************************************/

#ifndef __RESOLVE_WI_CALL_H__
#define __RESOLVE_WI_CALL_H__

#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Instructions.h"

using namespace llvm;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  typedef enum {
    ICT_NONE,
    ICT_GET_LOCAL_ID,
    ICT_GET_GLOBAL_ID,
    ICT_GET_ITER_COUNT,
    ICT_GET_SPECIAL_BUFFER,
    ICT_GET_CURR_WI,
    ICT_GET_WORK_DIM,
    ICT_GET_GLOBAL_SIZE,
    ICT_GET_LOCAL_SIZE,
    ICT_GET_NUM_GROUPS,
    ICT_GET_GROUP_ID,
    ICT_GET_GLOBAL_OFFSET,
    ICT_PRINTF,
    ICT_ASYNC_WORK_GROUP_COPY,
    ICT_ASYNC_WORK_GROUP_STRIDED,
    ICT_WAIT_GROUP_EVENTS,
    ICT_PREFETCH,
    ICT_NUMBER
  } TInternalCallType;

  /// @brief  ResolveWICall class used resolve work item function calls
  /// @Author Marina Yatsina
  class ResolveWICall : public ModulePass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;

    /// @brief Constructor
    ResolveWICall() : ModulePass(ID) {}

    /// @brief LLVM Module pass entry
    /// @param M Module to transform
    /// @returns true if changed
    bool runOnModule(Module &M);

  protected:
    /// @brief Resolves the work item function calls of the kernel
    /// @param pFunc The function which needs it work item function calls to be resolved
    /// @returns The new function that all its work item function calls were resolved
    Function* runOnFunction(Function *pFunc);

    /// @brief Substitues the a work item function calls with acesses
    ///        to implicit arguments and calculations based on them.
    /// @param pCall The call instruction that calls a work item function
    /// @param type  The call instruction type
    /// @returns The result value of the work item function call
    Value* updateGetFunction(CallInst *pCall, TInternalCallType type);

    /// @brief Calculates work-item information that use dimension
    ///        (this function is used where dimension is in bound)
    /// @param pCall         The call instruction that calls a work item function
    /// @param type          The call instruction type
    /// @param pInsterBefore The instruction to insert new instructions before
    /// @returns The result value of the work item function call
    Value* updateGetFunctionInBound(CallInst *pCall, TInternalCallType type, Instruction *pInsertBefore);

    /// @brief Calculates the global id
    /// @param pCall         The call instruction that calls a work item function
    /// @param pInsterBefore The instruction to insert new instructions before
    /// @returns The result value of the work item function call
    Value* calcGlobalId(CallInst *pCall, Instruction *pInsterBefore);

    /// @brief Returns Internal Call Type for given function name
    /// @param calledFuncName given function name
    /// @returns Internal Call Type for given function name
    TInternalCallType getCallFunctionType(std::string calledFuncName);

    // printf/AsyncCopy/Wait
    Value*  updatePrintf(llvm::CallInst *pCall);
    Value*  updateAsyncCopy(llvm::CallInst *pCall, bool strided);
    void  updateWaitGroup(llvm::CallInst *pCall);
    void  updatePrefetch(llvm::CallInst *pCall);

    void  addAsyncCopyDeclaration();
    void  addPrefetchDeclaration();
    void  addPrintfDeclaration();

  protected:
    /// @brief The llvm current processed module
    Module      *m_pModule;
    /// @brief The llvm context
    LLVMContext *m_pLLVMContext;

    /// This holds the pCtx implicit argument of current handled function
    Argument *m_pCtx;
    /// This holds the pWorkInfo implicit argument of current handled function
    Argument *m_pWorkInfo;
    /// This holds the pWGId implicit argument of current handled function
    Argument *m_pWGId;
    /// This holds the pBaseGlbId implicit argumnet of current handled function
    Argument *m_pBaseGlbId;
    /// This holds the pLocalId implicit argument of current handled function
    Argument *m_pLocalId;
    /// This holds the pIterCount implicit argument of current handled function
    Argument *m_pIterCount;
    /// This holds the pSpecialBuf implicit argument of current handled function
    Argument *m_pSpecialBuf;
    /// This holds the pCurrWI implicit argument of current handled function
    Argument *m_pCurrWI;

    /// This is flag indicates that AsyncCopy declarations already added to module
    bool m_bAsyncCopyDecl;
    /// This is flag indicates that Prefetch declarations already added to module
    bool m_bPrefetchDecl;
    /// This is flag indicates that Printf declarations already added to module
    bool m_bPrintfDecl;

  };
  
}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif //__RESOLVE_WI_CALL_H__
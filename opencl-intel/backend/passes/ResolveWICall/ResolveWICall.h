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

#ifndef __RESOLVE_WI_CALL_H__
#define __RESOLVE_WI_CALL_H__

#include "ImplicitArgsUtils.h"
#include "ImplicitArgsAnalysis/ImplicitArgsAnalysis.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"

#include <set>

using namespace llvm;

namespace intel {

  /// @brief  ResolveWICall class used resolve work item function calls
  class ResolveWICall : public ModulePass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;

    /// @brief Constructor
    ResolveWICall() : ModulePass(ID) {}

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
      return "ResolveWICall";
    }

    /// @brief LLVM Module pass entry
    /// @param M Module to transform
    /// @returns true if changed
    bool runOnModule(Module &M);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<ImplicitArgsAnalysis>();
    }

  protected:
    Value* updateEnqueueKernelFunction(SmallVectorImpl<Value*> &Params, const StringRef FunctionName, CallInst *pCall);
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

    /// @brief Returns Internal Call Type for given function name
    /// @param calledFuncName given function name
    /// @returns Internal Call Type for given function name
    TInternalCallType getCallFunctionType(std::string calledFuncName);

    // printf/prefetch
    Value*  updatePrintf(llvm::CallInst *pCall);
    void  updatePrefetch(llvm::CallInst *pCall);

    void  addPrefetchDeclaration();

    /// @brief add extended execution declarations
    void  addExtendedExecutionDeclarations();

    /// @brief add declaration of external function to Module
    /// @param type - callback type
    void addExternFunctionDeclaration(unsigned type, FunctionType* FT, StringRef Name);
    /// @brief obtain name for extexec callback
    const char* getExternCallbackName(unsigned type) const;

    /// Helper functions to construct OpenCL types
    /// @brief constructs type for queue_t
    Type * getQueueType() const;
    /// @brief constructs type for clk_event_t
    Type * getClkEventType() const;
    /// @brief constructs type for clk_profiling_info
    Type * getClkProfilingInfo() const;
    /// @brief constructs type for kernel_enqueue_flags_t
    Type * getKernelEnqueueFlagsType() const;
    /// @brief constructs type for ndrange_t
    Type * getNDRangeType() const;
    /// @brief constructs type for block without arguments
    Type * getBlockNoArgumentsType() const;
    /// @brief constructs type for block with local mem arguments
    Type * getBlockLocalMemType() const;
    /// @brief constructs type for return type of enqueue_kernel
    Type * getEnqueueKernelRetType() const;
    /// @brief return ConstantInt::int32_type with zero value
    ConstantInt * getConstZeroInt32Value() const;
    /// @brief get or add from/to  module declaration of type used for local
    /// memory buffers specified in enqueue_kernel
    Type* getLocalMemBufType() const;

    void appendWithCallBackContextAndRuntimeHandleTypes(
        unsigned FuncType, SmallVectorImpl<Type *> &ArgTypes);
    void appendWithCallBackContextAndRuntimeHandleValues(
        unsigned FuncType, SmallVectorImpl<Value *> &Args,
        Instruction *InsertBefore);

    ///@brief returns description of EnqueueMarker callback
    FunctionType* getEnqueueMarkerFunctionType();
    ///@brief returns types EnqueueKernel callbacks
    ///@param  type - type of callback {basic, localmem, event, ...}
    ///@return      - call back Function type
    FunctionType* getOrCreateEnqueueKernelFuncType(unsigned type);
    ///@brief returns description of GetKernelWGSize and GerKernelPreferredWGSizeMultiple callbacks
    FunctionType* getOrCreateGetKernelQueryFuncType(unsigned type);
    FunctionType* getOrCreatePrintfFuncType();

    /// @brief helper function.
    ///    maps ndrange_ built-ins argument index to index of array within ndrange_t struct
    uint32_t MapIndexToIndexOfArray(const uint32_t Index, const uint32_t argsNum);
    /// @brief get the pointer size for the current target, in bits (32 or 64)
    unsigned getPointerSize() const;
  protected:
    /// @brief The llvm current processed module
    Module      *m_pModule;
    /// @brief The llvm context
    LLVMContext *m_pLLVMContext;
    ImplicitArgsAnalysis *m_IAA;
    IntegerType *m_sizeTTy;

    /// This holds the Runtime Handle implicit argument of current handled function
    /// This argument is initialized passed thru to MIC's printf
    Argument *m_pRuntimeHandle;
    /// This holds the pWorkInfo implicit argument of current handled function
    Argument *m_pWorkInfo;
    /// This holds the pWGId implicit argument of current handled function
    Argument *m_pWGId;
    /// This holds the pBaseGlbId implicit argumnet of current handled function
    Argument *m_pBaseGlbId;
    /// This holds the pSpecialBuf implicit argument of current handled function
    Argument *m_pSpecialBuf;
    /// This holds the pExtExecutionContext implicit argument of current handled function
    //TODO-NDRANGE: Extended execution context not supported in branch
    //Argument *m_pExtendedExecutionCtx;

    /// This is flag indicates that Prefetch declarations already added to module
    bool m_bPrefetchDecl;
    /// flags indicates that extended execution built-in declarations already added to module
    std::set<unsigned> m_ExtExecDecls;

    /// type %struct.__ndrange_t type
    /// NULL means declaration were not added to module
    Type * m_pStructNDRangeType;
    /// number of bits in integer returned from enqueue_kernel BI
    /// constant introduced for readability of code
    enum { ENQUEUE_KERNEL_RETURN_BITS = 32 };

    // Per function cached values
    Function *m_F;
    Value* m_RuntimeInterface;
    Value* m_Block2KernelMapper;
    void clearPerFunctionCache();
    Value *getOrCreateRuntimeInterface();
    Value *getOrCreateBlock2KernelMapper();

    // Version of OpenCL C a processed module is compiled for.
    unsigned m_oclVersion;
    // true if a module is compiled with the support of the
    // non-uniform work-group size.
    bool     m_nonUniformLocalSize;
  };


} // namespace intel

#endif //__RESOLVE_WI_CALL_H__

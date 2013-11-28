/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __RESOLVE_WI_CALL_H__
#define __RESOLVE_WI_CALL_H__

#include "ImplicitArgsUtils.h"
#include "ImplicitArgsAnalysis/ImplicitArgsAnalysis.h"
#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Instructions.h"
#include "llvm/Constants.h"

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
    virtual const char *getPassName() const {
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
    /// @brief Resolves the work item function calls of the kernel
    /// @param pFunc The function which needs it work item function calls to be resolved
    /// @returns The new function that all its work item function calls were resolved
    Function* runOnFunction(Function *pFunc);

    /// @brief Substitues the a work item function calls with acesses
    ///        to implicit arguments and calculations based on them.
    /// @param pCall The call instruction that calls a work item function
    /// @param type  The call instruction type
    /// @returns The result value of the work item function call
    Value* updateGetFunction(CallInst *pCall, unsigned type);

    /// @brief Calculates work-item information that use dimension
    ///        (this function is used where dimension is in bound)
    /// @param pCall         The call instruction that calls a work item function
    /// @param type          The call instruction type
    /// @param pInsterBefore The instruction to insert new instructions before
    /// @returns The result value of the work item function call
    Value* updateGetFunctionInBound(CallInst *pCall, unsigned type, Instruction *pInsertBefore);

    /// @brief Returns Internal Call Type for given function name
    /// @param calledFuncName given function name
    /// @returns Internal Call Type for given function name
    unsigned getCallFunctionType(std::string calledFuncName);

    // printf/prefetch
    Value*  updatePrintf(llvm::CallInst *pCall);
    void  updatePrefetch(llvm::CallInst *pCall);

    void  addPrefetchDeclaration();
    void  addPrintfDeclaration();
    
    /// @brief add extended execution declarations
    void  addExtendedExecutionDeclarations();

    /// @brief calculates ndrange_1D(), ndrange_2D() or ndrange_3D()
    /// @param type          The call instruction type that represents one of the ndrange BIs
    /// @param pCall         The call instruction that calls a work item function
    /// @returns The result value of the ndrange_ND call, where N = [1,2,3]
    Value* updateNDRangeND(unsigned type, CallInst *pCall);

    /// @brief add declaration of Extended Execution callback to Module
    /// @param type - callback type
    void addExtExecFunctionDeclaration(unsigned type);
    /// @brief obtain name for extexec callback
    std::string getExtExecCallbackName(unsigned type) const;
    
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
    /// @brief constructs type for extended execution context
    Type * getExtendedExecContextType() const;
    /// @brief return ConstantInt::int32_type with zero value
    ConstantInt * getConstZeroInt32Value() const;
    /// @brief get or add from/to  module declaration of struct.__ndrange_t
    Type* getOrAddStructNDRangeType();
    /// @brief get or add from/to  module declaration of type used for local
    /// memory buffers specified in enqueue_kernel
    Type* getLocalMemBufType() const;
    /// @brief Add instructions handling variable number 
    ///        of local mem arguments in Enqueue_kernel
    /// @param args input/output vector with arguments to call ocl20_* callback
    ///        When done args will be added with 2 arguments for local_mem handling
    /// @param pCall - enqueue_kernel call instruction 
    /// @param LocalMemArgsOffs offset of 1st argument with local mem arguments
    void addLocalMemArgs(std::vector<Value*>& args, 
      CallInst *pCall,
      const unsigned LocalMemArgsOffs);

    ///@brief returns description of EnqueueMarker callback
    FunctionType* getEnqueueMarkerFunctionType();
    ///@brief returns types EnqueueKernel callbacks
    ///@param  type - type of callback {basic, localmem, event, ...}
    ///@return      - call back Function type
    FunctionType* getEnqueueKernelType(unsigned type);
    ///@brief returns description of DefaultQueue callback
    FunctionType* getDefaultQueueFunctionType();
    ///@brief returns description of GetKernelWGSize and GerKernelPreferredWGSizeMultiple callbacks
    FunctionType* getGetKernelQueryFunctionType(unsigned type);
    ///@brief return description of ReleaseEvent and RetainEvent callbacks
    FunctionType* getRetainAndReleaseEventFunctionType();
    ///@brief returns description of CreateUserEvent callback
    FunctionType* getCreateUserEventFunctionType();
    ///@brief returns description of SetUserEventStatus callback
    FunctionType* getSetUserEventStatusFunctionType();
    ///@brief returns description of CaptureEventProfilingInfo callback
    FunctionType* getCaptureEventProfilingInfoFunctionType();
    ///@brief returns type of extended execution callback
    FunctionType* getExtExecFunctionType(unsigned type);

    ///@brief returns params List taken from pCall call
    ///!!! NOTE implicitly copies all pCall params to output
    ///!!! callback function should have the same arguments list + ExtExecContext as last argument
    std::vector<Value*> getExtExecFunctionParams(CallInst *pCall);
    ///@brief returns params List taken from pCall call converted to
    /// proper representation
    ///!!! NOTE implicitly copies all pCall params to output
    ///!!! callback function should have the same arguments list + list of local vars sizes + ExtExecContext as last argument
    std::vector<Value*> getEnqueueKernelLocalMemFunctionParams(CallInst *pCall, const uint32_t FixedArgs);
    /// @brief Store Value to 'unsigned int workDimension' in ndrange_t struct
    StoreInst* StoreWorkDim(Value* Ptr, uint64_t V, LLVMContext* pContext, Instruction* InsertBefore);
    /// @brief store value to one of Arrays in ndrange_t struct
    StoreInst* StoreNDRangeArrayElement(Value* Ptr, Value* V, const uint64_t ArrayPosition,
     const uint64_t ElementIndex, const Twine &Name, LLVMContext* pContext, Instruction* InsertBefore);
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
    /// This holds the pCurrWI implicit argument of current handled function
    Argument *m_pCurrWI;
    /// This holds the pExtExecutionContext implicit argument of current handled function
    //TODO-NDRANGE: Extended execution context not supported in branch
    //Argument *m_pExtendedExecutionCtx;

    /// This is flag indicates that Prefetch declarations already added to module
    bool m_bPrefetchDecl;
    /// This is flag indicates that Printf declarations already added to module
    bool m_bPrintfDecl;
    /// flags indicates that extended execution built-in declarations already added to module
    std::set<unsigned> m_ExtExecDecls;

    /// type %struct.__ndrange_t type 
    /// NULL means declaration were not added to module
    Type * m_pStructNDRangeType;
    /// number of bits in integer returned from enqueue_kernel BI
    /// constant introduced for readability of code
    enum { ENQUEUE_KERNEL_RETURN_BITS = 32 };

    // since both get_new_local_id and get_new_global_id rely on the same CSE it
    // makes sense to cache this CSE it also makes the implementation avoid code
    // duplication.
    // maps a pair of <Dimension, CurrWI> -> get_new_local_id(Dimension, CurrWI)
    //typedef GetNewLocalIDCache
    //std::map<std::pair<Value *, Value *>, Instruction *>;
    //GetNewLocalIDCache m_GetNewLocalIDs;
  };
  
} // namespace intel 

#endif //__RESOLVE_WI_CALL_H__

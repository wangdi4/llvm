/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __RESOLVE_WI_CALL_H__
#define __RESOLVE_WI_CALL_H__

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"

#include <set>

using namespace llvm;

namespace intel {

  typedef enum {
    ICT_NONE,
    ICT_GET_LOCAL_ID,
    ICT_GET_GLOBAL_ID,
    ICT_GET_BASE_GLOBAL_ID,
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
    ICT_PREFETCH,
    // get_default_queue()
    ICT_GET_DEFAULT_QUEUE,
    // Basic Enqueue kernel
    // int enqueue_kernel (queue_t queue,kernel_enqueue_flags_t flags,const ndrange_t ndrange,void (^block)(void))
    ICT_ENQUEUE_KERNEL_BASIC,
    // int enqueue_kernel (queue_t queue,kernel_enqueue_flags_t flags, const ndrange_t ndrange, void (^block)(local void *, ?), uint size0, ?)
    ICT_ENQUEUE_KERNEL_LOCALMEM,
    // int enqueue_kernel (queue_t queue,kernel_enqueue_flags_t flags,const ndrange_t ndrange,uint num_events_in_wait_list, const clk_event_t *event_wait_list,clk_event_t *event_ret,void (^block)(void))
    ICT_ENQUEUE_KERNEL_EVENTS,
    // int enqueue_kernel (queue_t queue, kernel_enqueue_flags_t flags, const ndrange_t ndrange, uint num_events_in_wait_list, const clk_event_t *event_wait_list, clk_event_t *event_ret, void (^block)(local void *, ?), uint size0, ?)
    ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM,
    // uint get_kernel_work_group_size (void (^block)(void))
    ICT_GET_KERNEL_WORK_GROUP_SIZE,
    // uint get_kernel_work_group_size (void (^block)(local void *, ...))
    ICT_GET_KERNEL_WORK_GROUP_SIZE_LOCAL,
    // uint get_kernel_preferred_work_group_size_multiple(void (^block)(void))
    ICT_GET_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
    // uint get_kernel_preferred_work_group_size_multiple(void (^block)(local void *, ...));
    ICT_GET_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE_LOCAL,
    // int enqueue_marker ( queue_t queue, uint num_events_in_wait_list, const clk_event_t *event_wait_list, clk_event_t *event_ret)
    ICT_ENQUEUE_MARKER,
    // void retain_event (clk_event_t event)
    ICT_RETAIN_EVENT,
    // void release_event (clk_event_t event)
    ICT_RELEASE_EVENT,
    // clk_event_t create_user_event ()
    ICT_CREATE_USER_EVENT,
    // void set_user_event_status ( clk_event_t event, int status)
    ICT_SET_USER_EVENT_STATUS,
    // void capture_event_profiling_info ( clk_event_t event, clk_profiling_info name, global ulong *value)
    ICT_CAPTURE_EVENT_PROFILING_INFO,
    // ndrange_1D
    ICT_NDRANGE_1D,
    // ndrange_2D
    ICT_NDRANGE_2D,
    // ndrange_3D
    ICT_NDRANGE_3D,
    ICT_NUMBER
  } TInternalCallType;

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

    /// @brief Calculates the base global id (first global id in the work group)
    /// @param pCall         The call instruction that calls a work item function
    /// @param pInsterBefore The instruction to insert new instructions before
    /// @returns The result value of the work item function call
    Value* calcBaseGlobalId(CallInst *pCall, Instruction *pInsterBefore);

    /// @brief Returns Internal Call Type for given function name
    /// @param calledFuncName given function name
    /// @returns Internal Call Type for given function name
    TInternalCallType getCallFunctionType(std::string calledFuncName);

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
    Value* updateNDRangeND(const TInternalCallType type, CallInst *pCall);

    /// @brief add declaration of Extended Execution callback to Module
    /// @param type - callback type
    void addExtExecFunctionDeclaration(const TInternalCallType type);
    /// @brief obtain name for extexec callback
    std::string getExtExecCallbackName(const TInternalCallType type) const;
    
    /// @brief insert a call for Extended Execution callback
    /// @return CallInst
    Value* updateExtExecFunction(std::vector<Value*> Params, const StringRef FunctionName, CallInst *InsertBefore);

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
    /// @brief return size_t 
    Type * getSizeTType() const;
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
    FunctionType* getEnqueueKernelType(const TInternalCallType type);
    ///@brief returns description of DefaultQueue callback
    FunctionType* getDefaultQueueFunctionType();
    ///@brief returns description of GetKernelWGSize and GerKernelPreferredWGSizeMultiple callbacks
    FunctionType* getGetKernelQueryFunctionType(const TInternalCallType type);
    ///@brief return description of ReleaseEvent and RetainEvent callbacks
    FunctionType* getRetainAndReleaseEventFunctionType();
    ///@brief returns description of CreateUserEvent callback
    FunctionType* getCreateUserEventFunctionType();
    ///@brief returns description of SetUserEventStatus callback
    FunctionType* getSetUserEventStatusFunctionType();
    ///@brief returns description of CaptureEventProfilingInfo callback
    FunctionType* getCaptureEventProfilingInfoFunctionType();
    ///@brief returns type of extended execution callback
    FunctionType* getExtExecFunctionType(const TInternalCallType type);

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
    /// @brief helper fucntion. 
    ///    maps ndrange_ built-ins argument index to index of array within ndrange_t struct
    uint32_t MapIndexToIndexOfArray(const uint32_t Index, const uint32_t argsNum);
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
    /// This holds the pExtExecutionContext implicit argument of current handled function
    Argument *m_pExtendedExecutionCtx;

    /// This is flag indicates that Prefetch declarations already added to module
    bool m_bPrefetchDecl;
    /// This is flag indicates that Printf declarations already added to module
    bool m_bPrintfDecl;
    /// flags indicates that extended execution built-in declarations already added to module
    std::set<TInternalCallType> m_ExtExecDecls;

    /// type %struct.__ndrange_t type 
    /// NULL means declaration were not added to module
    Type * m_pStructNDRangeType;
    /// number of bits in integer returned from enqueue_kernel BI
    /// constant introduced for readability of code
    enum { ENQUEUE_KERNEL_RETURN_BITS = 32 };
  };
  
} // namespace intel 

#endif //__RESOLVE_WI_CALL_H__
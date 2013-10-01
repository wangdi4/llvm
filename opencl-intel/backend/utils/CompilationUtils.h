/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __COMPILATION_UTILS_H__
#define __COMPILATION_UTILS_H__

#include "cl_kernel_arg_type.h"
#ifndef __APPLE__
#include "exceptions.h"
#endif

#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Constants.h"
#include "llvm/ADT/SetVector.h"
#include <vector>
#include <map>

using namespace llvm;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

#ifndef __APPLE__
  DEFINE_EXCEPTION(CompilerException)
#endif

  /// @brief  CompilationUtils class used to provide helper utilies that are
  ///         used by several other classes.
  class CompilationUtils {

  public:
    /// We use a SetVector to ensure determinstic iterations
    typedef SetVector<Function*> FunctionSet;
    /// @brief Removes the from the given basic block the instruction pointed 
    ///        by the given iterator
    /// @param pBB         A Basic block from which the instruction needs to be removed
    /// @param it          An iterator pointing to the instruction that needs to be removed
    /// @returns An iterator to the next instruction after the instruction that was removed
    static BasicBlock::iterator removeInstruction(BasicBlock* pBB, BasicBlock::iterator it);
    
    /// @brief  Retrieves the pointer to the implicit arguments added to the given function
    /// @param  pFunc        The function for which implicit arguments need to be retrieved
    /// @param  ppLocalMem   The pLocalMem argument, NULL if this argument shouldn't be retrieved
    /// @param  ppWorkDim    The pWorkDim argument, NULL if this argument shouldn't be retrieved
    /// @param  ppWGId       The pWGId argument, NULL if this argument shouldn't be retrieved
    /// @param  ppBaseGlbId  The pBaseGlbId argument, NULL if this argument shouldn't be retrieved
    /// @param  ppLocalId    The LocalIds argument, NULL if this argument shouldn't be retrieved
    /// @param  ppSpecialBuf The SpecialBuf argument, NULL if this argument shouldn't be retrieved
    /// @param  ppIterCount  The IterCount argument, NULL if this argument shouldn't be retrieved
    /// @param  ppCurrWI     The CurrWI argument, NULL if this argument shouldn't be retrieved
    /// @param  ppCtx        The pCtx argument, NULL if this argument shouldn't be retrieved
    /// @param  ppExtExecCtx The ExtendedExecutionContext argument, NULL if this argument shouldn't be retrieved
    static void getImplicitArgs(Function *pFunc,
      Argument **ppLocalMem, Argument **ppWorkDim, Argument **ppWGId,
      Argument **ppBaseGlbId, Argument **ppLocalId, Argument **ppIterCount,
      Argument **ppSpecialBuf, Argument **ppCurrWI, Argument **ppCtx,
      Argument **ppExtExecCtx);

    /// @brief collect built-ins declared in the module and force synchronization.
    //         I.e. implemented using barrier built-in.
    /// @param functionSet container to insert all synchronized built-ins into
    /// @param pModule the module to search synchronize built-ins declarations in
    static void getAllSyncBuiltinsDcls(FunctionSet &functionSet, Module *pModule);

    /// @brief collect all kernel functions
    /// @param functionSet container to insert all kernel function into
    /// @param pModule the module to search kernel function inside
    static void getAllKernels(FunctionSet &functionSet, Module *pModule);

    /// @brief collect all kernel wrapper functions
    /// @param functionSet container to insert all kernel wrapper function into
    /// @param pModule the module to search kernel wrapper function inside
    static void getAllKernelWrappers(FunctionSet &functionSet, Module *pModule);

    /// @brief  fills a vector of cl_kernel_argument with arguments representing pFunc's
    ///         OpenCL level arguments
    /// @param pModule    The module
    /// @param pFunc      The kernel for which to create argument vector
    /// @param arguments  OUT param, the cl_kernel_argument which represent pFunc's
    ///                   OpenCL level argument
    static void parseKernelArguments(  Module* pModule, 
                                              Function* pFunc, 
                                              std::vector<cl_kernel_argument>& /* OUT */ arguments);
    
    /// @brief  maps between kernels (both scalar and vectorized) and their metdata
    /// @param pModule          The module
    /// @param pVectFunctions   The vectorized kernels, these kernel should be mapped
    ///                         to their scalar version metadata
    /// @param pVectFunctions   OUT param, maps between kernels (both scalar and
    ///                         vectorized) and their metdata
    static void getKernelsMetadata( Module* pModule, 
                                    const SmallVectorImpl<Function*>& pVectFunctions, 
                                    std::map<Function*, MDNode*>& /* OUT */ kernelMetadata);

    static bool isGetWorkDim(const std::string&);
    static bool isGetGlobalId(const std::string&);
    static bool isGetGlobalSize(const std::string&);
    static bool isGetLocalId(const std::string&);
    static bool isGetLocalSize(const std::string&);
    static bool isGetGlobalLinearId(const std::string&);
    static bool isGetLocalLinearId(const std::string&);
    static bool isGetNumGroups(const std::string&);
    static bool isGetGroupId(const std::string&);
    static bool isGlobalOffset(const std::string&);
    static bool isAsyncWorkGroupCopy(const std::string&);
    static bool isWaitGroupEvents(const std::string&);
    static bool isPrefetch(const std::string&);
    static bool isAsyncWorkGroupStridedCopy(const std::string&);
    static bool isMemFence(const std::string&);
    static bool isReadMemFence(const std::string&);
    static bool isWriteMemFence(const std::string&);
    static bool isNDRange_1D(const std::string&);
    static bool isNDRange_2D(const std::string&);
    static bool isNDRange_3D(const std::string&);
    static bool isEnqueueKernelBasic(const std::string&);
    static bool isEnqueueKernelLocalMem(const std::string&);
    static bool isEnqueueKernelEvents(const std::string&);
    static bool isEnqueueKernelEventsLocalMem(const std::string&);
    static bool isGetKernelWorkGroupSize(const std::string&);
    static bool isGetKernelWorkGroupSizeLocal(const std::string&);
    static bool isGetKernelPreferredWorkGroupSizeMultiple(const std::string&);
    static bool isGetKernelPreferredWorkGroupSizeMultipleLocal(const std::string&);
    static bool isEnqueueMarker(const std::string&);
    static bool isGetDefaultQueue(const std::string&);
    static bool isRetainEvent(const std::string&);
    static bool isReleaseEvent(const std::string&);
    static bool isCreateUserEvent(const std::string&);
    static bool isSetUserEventStatus(const std::string&);
    static bool isCaptureEventProfilingInfo(const std::string&);

    static const std::string NAME_GET_ORIG_GID;
    static const std::string NAME_GET_ORIG_LID;

    static const std::string NAME_GET_WORK_DIM;
    static const std::string NAME_GET_GLOBAL_SIZE;
    static const std::string NAME_GET_LOCAL_SIZE;
    static const std::string NAME_GET_NUM_GROUPS;
    static const std::string NAME_GET_GROUP_ID;
    static const std::string NAME_GET_GLOBAL_OFFSET;
    static const std::string NAME_PRINTF;

    static const std::string NAME_ASYNC_WORK_GROUP_COPY;
    static const std::string NAME_WAIT_GROUP_EVENTS;
    static const std::string NAME_PREFETCH;
    static const std::string NAME_ASYNC_WORK_GROUP_STRIDED_COPY;

    static const std::string NAME_MEM_FENCE;
    static const std::string NAME_READ_MEM_FENCE;
    static const std::string NAME_WRITE_MEM_FENCE;

    static const std::string NAME_GET_LINEAR_GID;
    static const std::string NAME_GET_LINEAR_LID;

    static const std::string BARRIER_FUNC_NAME;
    static const std::string WG_BARRIER_FUNC_NAME;
    //images
    static const std::string OCL_IMG_PREFIX;
    static const std::string IMG_2D;
    static const std::string IMG_2D_ARRAY;
    static const std::string IMG_3D;
    //kernel arg qualifiers
    static const std::string WRITE_ONLY;
    static const std::string READ_ONLY;
    static const std::string NONE;
    //kernel type qualifiers
    static const std::string SAMPLER;
  public:
    /// Holds the number of implicit arguments added to function
    static const unsigned int NUMBER_IMPLICIT_ARGS;

    /// '3' is a magic number for global variables
    /// that were in origin kernel local variable!
    static const unsigned int LOCL_VALUE_ADDRESS_SPACE;

    static const std::string NAME_GET_BASE_GID;
    static const std::string NAME_GET_GID;
    static const std::string NAME_GET_LID;
    static const std::string NAME_GET_ITERATION_COUNT;
    static const std::string NAME_GET_SPECIAL_BUFFER;
    static const std::string NAME_GET_CURR_WI;

    //////////////////////////////////////////////////////////////////
    // @brief returns the mangled name of the function get_global_id
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetGID();
    //////////////////////////////////////////////////////////////////
   // @brief returns the mangled name of the function get_global_size
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetGlobalSize();
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the function get_local_id
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetLID();
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the function get_local_size
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetLocalSize();
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the barrier function
    //////////////////////////////////////////////////////////////////
    static std::string mangledBarrier();
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the work_group_barrier function
    // @param wgBarrierType
    //                      WG_BARRIER_NO_SCOPE - for 
    // void work_group_barrier (cl_mem_fence_flags flags)
    //                      WG_BARRIER_WITH_SCOPE - for
    // void work_group_barrier (cl_mem_fence_flags flags, memory_scope scope)
    //////////////////////////////////////////////////////////////////
    typedef enum {
      WG_BARRIER_NO_SCOPE,
      WG_BARRIER_WITH_SCOPE
    } WG_BARRIER_TYPE;
    static std::string mangledWGBarrier(WG_BARRIER_TYPE wgBarrierType);
    //////////////////////////////////////////////////////////////////
    // @brief: returns the name of the argument metadata node for the
    //given module
    //////////////////////////////////////////////////////////////////
    static std::string argumentAttribute(const llvm::Module&);

    enum clVersion {
        CL_VER_NOT_DETECTED = -1,
        CL_VER_1_0 = 0,
        CL_VER_1_1 = 1,
        CL_VER_1_2 = 2,
        CL_VER_2_0 = 3
    };
    static const std::string NAME_GET_DEFAULT_QUEUE;
    
    /// Basic Enqueue kernel
    /// int enqueue_kernel (queue_t queue,kernel_enqueue_flags_t flags,
    ///                     const ndrange_t ndrange,void (^block)(void))
    static const std::string NAME_ENQUEUE_KERNEL_BASIC;
    /// ndrange_t ndrange_1D (). matches function with 1, 2, 3 arguments
    static const std::string NAME_NDRANGE_1D;
    /// ndrange_t ndrange_2D (). matches function with 1, 2, 3 arguments
    static const std::string NAME_NDRANGE_2D;
    /// ndrange_t ndrange_3D (). matches function with 1, 2, 3 arguments
    static const std::string NAME_NDRANGE_3D;
    /// Enqueue kernel with local memory
    /// int enqueue_kernel (queue_t queue,kernel_enqueue_flags_t flags, 
    ///     const ndrange_t ndrange, 
    ///     void (^block)(local void *, ?), uint size0, ?)
    static const std::string NAME_ENQUEUE_KERNEL_LOCALMEM;
    /// Enqueue kernel with events
    /// int enqueue_kernel (queue_t queue,kernel_enqueue_flags_t flags,
    ///    const ndrange_t ndrange,uint num_events_in_wait_list, 
    ///    const clk_event_t *event_wait_list,clk_event_t *event_ret,void (^block)(void))
    static const std::string NAME_ENQUEUE_KERNEL_EVENTS;
    /// Enqueue kernel with events and local memory
    /// int enqueue_kernel (queue_t queue, kernel_enqueue_flags_t flags, 
    ///    const ndrange_t ndrange, uint num_events_in_wait_list, 
    ///    const clk_event_t *event_wait_list, clk_event_t *event_ret, 
    ///    void (^block)(local void *, ?), uint size0, ?)
    static const std::string NAME_ENQUEUE_KERNEL_EVENTS_LOCALMEM;

    /// get maximum work-group size that can be used
    /// to execute a block on a specific device
    /// uint get_kernel_work_group_size
    static const std::string NAME_GET_KERNEL_WG_SIZE;
    static const std::string NAME_GET_KERNEL_WG_SIZE_LOCAL;

    /// Returns the preferred multiple of work-group
    /// size for launch
    /// uint get_kernel_preferred_work_group_size_multiple
    static const std::string NAME_GET_KERNEL_PREFERRED_WG_SIZE_MULTIPLE;
    static const std::string NAME_GET_KERNEL_PREFERRED_WG_SIZE_MULTIPLE_LOCAL;

    /// int enqueue_marker (
    ///     queue_t queue,
    ///     uint num_events_in_wait_list,
    ///     const clk_event_t *event_wait_list,
    ///     clk_event_t *event_ret)
    static const std::string NAME_ENQUEUE_MARKER;

    /// void retain_event (clk_event_t event)
    static const std::string NAME_RETAIN_EVENT;

    /// void release_event (clk_event_t event)
    static const std::string NAME_RELEASE_EVENT;

    /// clk_event_t create_user_event ()
    static const std::string NAME_CREATE_USER_EVENT;

    /// void set_user_event_status (
    ///     clk_event_t event,
    ///     int status)
    static const std::string NAME_SET_USER_EVENT_STATUS;

    /// void capture_event_profiling_info (
    ///     clk_event_t event,
    ///     clk_profiling_info name,
    ///     global ulong *value)
    static const std::string NAME_CAPTURE_EVENT_PROFILING_INFO;

    static clVersion getCLVersionFromModule(const Module &M);
    
  };

  //
  // Base class for all functors, which supports immutability query.
  //

  class AbstractFunctor{
  protected:
    bool m_isChanged;
  public:
    AbstractFunctor(): m_isChanged(false){}

    virtual ~AbstractFunctor() {}

    bool isChanged()const{
      return m_isChanged;
    }
  };

  class FunctionFunctor: public AbstractFunctor {
  public:
    virtual void operator ()(llvm::Function&) = 0;
  };

  class BlockFunctor: public AbstractFunctor {
  public:
    virtual void operator ()(llvm::BasicBlock&) = 0;
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __COMPILATION_UTILS_H__

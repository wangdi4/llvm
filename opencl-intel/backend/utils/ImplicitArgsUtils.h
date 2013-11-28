/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __IMPLICIT_ARGS_UTILS_H__
#define __IMPLICIT_ARGS_UTILS_H__

#include "ImplicitArgProperties.h"
#include "ImplicitArgument.h"
#include <cassert>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  class CallbackContext;
  class ExtendedExecutionContext;
  namespace NDInfo {
  // Keep these values in same order as structure so they can be used as indices
  // for GEP accesses
  enum _NDInfo {
    WORK_DIM,
    GLOBAL_OFFSET,
    GLOBAL_SIZE,
    LOCAL_SIZE,
    WG_NUMBER,
    LOOP_ITER_COUNT,
    RUNTIME_CALLBACKS,
    NEW_LOCAL_ID,
    LAST
  };
  static const char* getRecordName(unsigned RecordID) {
    const char* Names[LAST] = {
      "WorkDim",
      "GlobalOffset_",
      "GlobalSize_",
      "LocalSize_",
      "NumGroups_",
      "LoopIterCount",
      "RunTimeCallBacks",
      "NewLocalID_"
    };
    return Names[RecordID];
  }
  }
  enum TInternalCallType {
    ICT_NONE,
    ICT_GET_NEW_LOCAL_ID,
    ICT_GET_NEW_GLOBAL_ID,
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
 };
  static unsigned InternalCall2NDInfo(unsigned InternalCall) {
    assert(InternalCall < ICT_NUMBER);
    switch (InternalCall) {
    case ICT_GET_GLOBAL_OFFSET:
      return NDInfo::GLOBAL_OFFSET;
    case ICT_GET_GLOBAL_SIZE:
      return NDInfo::GLOBAL_SIZE;
    case ICT_GET_LOCAL_SIZE:
      return NDInfo::LOCAL_SIZE;
    case ICT_GET_NUM_GROUPS:
      return NDInfo::WG_NUMBER;
    }
    assert(false && "Unhandled case");
    return NDInfo::LAST;
  }

  struct sWorkInfo;

  /// @brief  ImplicitArgsUtils class used to provide helper utilies for handling
  ///         implicit arguments.
  class ImplicitArgsUtils {
  
  public:
    enum IMPLICIT_ARGS {
      IA_SLM_BUFFER,
      IA_WORK_GROUP_INFO,
      IA_WORK_GROUP_ID,
      IA_GLOBAL_BASE_ID,
      IA_BARRIER_BUFFER,
      IA_CURRENT_WORK_ITEM,
      IA_RUNTIME_CONTEXT,
      IA_NUMBER
    };
    static const unsigned int NUMBER_IMPLICIT_ARGS = IA_NUMBER;

    /// @brief Returns the implicit argument properties of given argument index
    /// @param arg     The implicit argument index
    /// @returns The implicit argument properties
    static const ImplicitArgProperties& getImplicitArgProps(unsigned int arg);

    /// @brief  Initialize properties on implicit arguments in run time
    /// @param  sizeOfPtr     Size of pointer, depends on target machine
    /// @returns none
    static void initImplicitArgProps(unsigned int sizeOfPtr);
    
    /// @brief Indicates that the properties were initialized
    static bool m_initialized;

    /// @brief Constructor
    ImplicitArgsUtils() {}
    /// @brief Destructor
    ~ImplicitArgsUtils() {}

#ifndef __APPLE__
    /// @brief Creates implicit arguments based on the implicit arguments properties
    /// @param pDest          A buffer that should hold the values of the implicit arguments
    void createImplicitArgs(char* pDest);
    
    /// @brief Sets values of implicit arguments for arguments that have same
    ///        values per executable
    /// @param pWorkInfo        The work group information parameter
    /// @param pGlobalBaseId    The global base id parameter
    /// @param pCallBackContext The callback context parameter
    /// @param bJitCreateWIids  The indiectaor for JIT creating WI ids parameter
    /// @param packetWidth      The packet width for vectorized JIT parameter
    /// @param pWIids           The work item ids buffer parameter
    /// @param iterCounter      The number of iterations parameter
    /// @param pExtendedExecutionContext
    ///                         The callback extended execution context parameter
    void setImplicitArgsPerExecutable(
                         const sWorkInfo* pWorkInfo,
                         const size_t* pGlobalBaseId,
                         const CallbackContext* pCallBackContext, 
                         bool bJitCreateWIids,
                         unsigned int packetWidth,
                         size_t* pWIids,
                         const size_t iterCounter,
                         const ExtendedExecutionContext* 
                                  pCallBackExtendedExecutionContext);
    
    /// @brief Sets values of implicit arguments for arguments that have same
    ///        values per work group
    /// @param pParams              The arguments values array
    void setImplicitArgsPerWG(const void* pParams);

    static const char* getArgName(unsigned Idx);
  
    /// @brief Initialized the work item local IDs
    static void initWILocalIds(size_t dim, const size_t* pLocalSizes, const unsigned int packetWidth, size_t* pWIids);
#endif //#ifndef __APPLE__

  private:
    /// static list of implicit argument properties 
    static ImplicitArgProperties m_implicitArgProps[NUMBER_IMPLICIT_ARGS];
  
    /// list of implicit arguments
    ImplicitArgument m_implicitArgs[NUMBER_IMPLICIT_ARGS];
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __IMPLICIT_ARGS_UTILS_H__

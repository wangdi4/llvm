/*****************************************************************************\

Copyright (c) Intel Corporation (2014).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  opencl20_ext_execution.cpp

\*****************************************************************************/

#define DEBUG_TYPE "opencl20-ext-execution"

#include <string.h>
#include "cl_dev_backend_api.h"
#include "cpu_dev_limits.h"
#include "ICLDevBackendServiceFactory.h"
#include "ExtendedExecutionContext.h"
#include "BlockLiteral.h"
#include "IBlockToKernelMapper.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SmallVector.h"

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

/////////////////////////////////////////////////////////////////////////////////////////////////////
/// Declaration section
/// !!!NOTE: Callbacks should have the same argument order and same return type as original BI
/// If it's not, then implement passing of arguments to callback for specified function
/////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief call back for get default queue
extern "C" LLVM_BACKEND_API 
  queue_t* ocl20_get_default_queue( ExtendedExecutionContext * pEEC);
   
/// @brief callback for 
///  int enqueue_kernel (queue_t queue, kernel_enqueue_flags_t flags,
///                     const cl_work_description_type ndrange,void (^block)(void))
extern "C" LLVM_BACKEND_API int ocl20_enqueue_kernel_basic(
        queue_t* queue,
        kernel_enqueue_flags_t flags,
        cl_work_description_type* ndrange,
        void *block,
        ExtendedExecutionContext * pEEC);

/// @brief callback for
///  int enqueue_kernel (
///     queue_t queue,
///     kernel_enqueue_flags_t flags,
///     const ndrange_t ndrange,
///     uint num_events_in_wait_list,
///     const clk_event_t *event_wait_list,
///     clk_event_t *event_ret,
///     void (^block)(void))
extern "C" LLVM_BACKEND_API int ocl20_enqueue_kernel_events(
        queue_t* queue,
        kernel_enqueue_flags_t flags,
        cl_work_description_type* ndrange,
        unsigned num_events_in_wait_list, clk_event_t *in_wait_list,
        clk_event_t *event_ret,
        void *block,
        ExtendedExecutionContext * pEEC);

/// @brief callback for
///  int enqueue_kernel (
///     queue_t queue,
///     kernel_enqueue_flags_t flags,
///     const ndrange_t ndrange,
///     void (^block)(local void *, ...),
///     uint size0, ...)
extern "C" LLVM_BACKEND_API int ocl20_enqueue_kernel_localmem(
        queue_t* queue,
        kernel_enqueue_flags_t flags,
        cl_work_description_type* ndrange,
        void *block,
        unsigned *localbuf_size, unsigned localbuf_size_len,
        ExtendedExecutionContext * pEEC);

/// @brief callback for
///  int enqueue_kernel (
///     queue_t queue,
///     kernel_enqueue_flags_t flags,
///     const ndrange_t ndrange,
///     uint num_events_in_wait_list,
///     const clk_event_t *event_wait_list,
///     clk_event_t *event_ret,
///     void (^block)(local void *, ...),
///     uint size0, ...)
extern "C" LLVM_BACKEND_API int ocl20_enqueue_kernel_events_localmem(
        queue_t* queue,
        kernel_enqueue_flags_t flags,
        cl_work_description_type* ndrange,
        unsigned num_events_in_wait_list, clk_event_t *in_wait_list,
        clk_event_t *event_ret,
        void *block,
        unsigned *localbuf_size, unsigned localbuf_size_len,
        ExtendedExecutionContext * pEEC);

/// @brief callback for
///  int enqueue_marker (
///     queue_t queue,
///     uint num_events_in_wait_list,
///     const clk_event_t *event_wait_list,
///     clk_event_t *event_ret)
extern "C" LLVM_BACKEND_API int ocl20_enqueue_marker(
        queue_t* queue,
        uint32_t num_events_in_wait_list,
        const clk_event_t* event_wait_list,
        clk_event_t* event_ret,
        ExtendedExecutionContext * pEEC);

/// @brief callback for
///  int retain_event (
///     clk_event_t event)
extern "C" LLVM_BACKEND_API void ocl20_retain_event(
        clk_event_t *event,
        ExtendedExecutionContext * pEEC);

/// @brief callback for
///  int release_event (
///     clk_event_t event)
extern "C" LLVM_BACKEND_API void ocl20_release_event(
        clk_event_t *event,
        ExtendedExecutionContext * pEEC);

/// @brief callback for
///  clk_event_t create_user_event ()
extern "C" LLVM_BACKEND_API clk_event_t* ocl20_create_user_event(ExtendedExecutionContext * pEEC);

/// @brief callback for
///  void set_user_event_status (
///     clk_event_t event,
///     int status)
extern "C" LLVM_BACKEND_API void ocl20_set_user_event_status(
        clk_event_t *event,
        uint32_t status,
        ExtendedExecutionContext * pEEC);

/// @brief callback for
///  void capture_event_profiling_info (
///     clk_event_t event,
///     clk_profiling_info name,
///     global ulong *value)
extern "C" LLVM_BACKEND_API void ocl20_capture_event_profiling_info(
        clk_event_t *event,
        clk_profiling_info name,
        uint64_t *value,
        ExtendedExecutionContext * pEEC);

/// @brief callback for
///  uint get_kernel_work_group_size (
///     void (^block)(void))
extern "C" LLVM_BACKEND_API uint32_t ocl20_get_kernel_wg_size(
        void *block,
        ExtendedExecutionContext * pEEC);

/// @brief callback for
///  uint get_kernel_work_group_size (
///     void (^block)(local void *, ...))
extern "C" LLVM_BACKEND_API uint32_t ocl20_get_kernel_wg_size_local(
        void *block,
        ExtendedExecutionContext * pEEC);

///@brief callback for
///  uint get_kernel_preferred_work_group_size_multiple (
///     void (^block)(void))
extern "C" LLVM_BACKEND_API uint32_t ocl20_get_kernel_preferred_wg_size_multiple(
        void *block,
        ExtendedExecutionContext * pEEC);

///@brief callback for
///  uint get_kernel_preferred_work_group_size_multiple (
///     void (^block)(local void *, ...))
extern "C" LLVM_BACKEND_API uint32_t ocl20_get_kernel_preferred_wg_size_multiple_local(
        void *block,
        ExtendedExecutionContext * pEEC);
/////////////////////////////////////////////////////////////////////////////////////////////////////
/// Implementation section
/////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" LLVM_BACKEND_API 
queue_t* ocl20_get_default_queue( ExtendedExecutionContext * pEEC )
{
  DEBUG(dbgs() << "ocl20_get_default_queue. Entry point \n");
  assert(pEEC && "ExtendedExecutionContext is NULL");
  
  IDeviceCommandManager *pDCM = pEEC->GetDeviceCommandManager();
  assert(pDCM && "IDeviceCommandManager is NULL");
  
  // todo: enable call to DeviceCommandManager
  queue_t res = pDCM->GetDefaultQueueForDevice();
  DEBUG(dbgs() << "ocl20_get_default_queue. Called GetDefaultQueueForDevice\n");
  return static_cast<queue_t*>(res);
}

/// @brief common enqueue_kernel code for ALL enqueue_kernel built-ins
/// @param queue - queue used
/// @param flags - enqueue flags
/// @param ndrange - pointer to cl_work_description_type which stores ndrange data
/// @param block - pointer to block literal structure
/// @param num_events_in_wait_list - 
///           number of events in wait list for the enqueued block
/// @param in_wait_list - pointer to array of events in waitlist
///           in case of no events num_events_in_wait_list=0, in_wait_list=NULL
/// @param event_ret - pointer to event variable which stores return event
///           if no return event it is NULL
/// @param localbuf_size - pointer to array with sizes of local buffers
///           if there is no local buffers it is NULL, localbuf_size_len is 0
/// @param localbuf_size_len - number of elements in localbuf_size array
/// @param pEEC - pointer to ExtendedExecutionContext object
static int enqueue_kernel_common(
   queue_t* queue, 
   kernel_enqueue_flags_t flags, 
   cl_work_description_type* ndrange,
   void *block,
   unsigned num_events_in_wait_list, clk_event_t *in_wait_list,
   clk_event_t *event_ret,
   unsigned *localbuf_size, unsigned localbuf_size_len, 
   ExtendedExecutionContext * pEEC)
{
  assert(pEEC && "ExtendedExecutionContext is NULL");
  assert(block && "block is NULL");

  const BlockLiteral * pBlockLiteral = static_cast<BlockLiteral*>(block);
  const IBlockToKernelMapper * pMapper = pEEC->GetBlockToKernelMapper();
  assert(pMapper && "IBlockToKernelMapper is NULL");

  IDeviceCommandManager *pDCM = pEEC->GetDeviceCommandManager();

  // obtain entry point as key
  void * key = pBlockLiteral->GetInvoke();
  // obtain Kernel object from mapper
  const ICLDevBackendKernel_ * pKernel = pMapper->Map(key);

  // allocate context on stack or on heap. SmallVector
  llvm::SmallVector<char, 256> pContext;

  ///////////////////////////////////////////////////////////////////////
  // calculate context size
  // context consists of 
  // block-literaloffset - offset from the beginning of Context to place where 
  //                       block literal is stored
  // local memory args - local memory buffer sizes
  // block_literal (stored in the end of Context)
  // 
  
  // get block_literal size
  const size_t block_literal_size = pBlockLiteral->GetBufferSizeForSerialization();
  // get size of block literal offset.
  const size_t block_literal_offs_size = sizeof(unsigned);
  
  // calculating ContextSize 
  const size_t ContextSize = 
    // size of field storing block_literal offset
    block_literal_offs_size + 
    // local mem arguments
    // each local mem arg size is size_t. See Binary::InitParams() method
    localbuf_size_len * sizeof(size_t) + 
    // block_literal structure size
    block_literal_size;
  
  // get Block_literal offset storage
  const size_t BlockLiteralOffs = ContextSize - block_literal_size;

  // resize pContext with the size of Context
  pContext.resize(ContextSize);

  ///////////////////////////////////////////////////////////////////////
  // fill in context
  char * p = &pContext[0];
  
  // store block literal offset
  *(unsigned*)p = BlockLiteralOffs;
  p += block_literal_offs_size;

  // store local memory args
  for(unsigned cnt=0;cnt<localbuf_size_len;++cnt){
    *(size_t*)p = localbuf_size[cnt];
    p += sizeof(size_t);
  }

  // store block literal
  pBlockLiteral->Serialize(p);
  p += block_literal_size;

  // sanity check context is filled up correctly
  assert((size_t)(p-&pContext[0]) == ContextSize && "ContextSize does not match filled arguments size");
  
  ///////////////////////////////////////////////////////////////////////
  // call enqueue
  int res = pDCM->EnqueueKernel(
    reinterpret_cast<queue_t>(queue), // queue_t 
    flags, // kernel_enqueue_flags_t
    num_events_in_wait_list, // uiNumEventsInWaitList
    in_wait_list, // pEventWaitList
    event_ret, // clk_event_t* pEventRet
    pKernel, // const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_* pKernel
    &pContext[0], // pContext
    ContextSize, // size_t szContextSize
    ndrange      // const cl_work_description_type* pNdrange
    );
  
  return res;
}
extern "C" LLVM_BACKEND_API int ocl20_enqueue_kernel_basic(
        queue_t* queue,
        kernel_enqueue_flags_t flags,
        cl_work_description_type* ndrange,
        void *block,
        ExtendedExecutionContext * pEEC)
{
  DEBUG(dbgs() << "ocl20_enqueue_kernel_basic. Entry point \n");
  DEBUG(dbgs() << "Enqueued ndrange_1d \n" << 
    " workDimension " << ndrange->workDimension <<
    " globalWorkSize0 " << ndrange->globalWorkSize[0] <<
    " localWorkSize0 " << ndrange->localWorkSize[0] <<
    " globalWorkOffset0 " << ndrange->globalWorkOffset[0] << "\n");
  
  int res = 
    enqueue_kernel_common(queue, flags, ndrange, block, 
                          0, NULL, NULL, // events
                          NULL, 0, // local buffers
                          pEEC);
  DEBUG(dbgs() << "ocl20_enqueue_kernel_basic. Return value " << res << "\n");
  return res;
}

extern "C" LLVM_BACKEND_API int ocl20_enqueue_kernel_localmem(
        queue_t* queue,
        kernel_enqueue_flags_t flags,
        cl_work_description_type* ndrange,
        void *block,
        unsigned *localbuf_size, unsigned localbuf_size_len,
        ExtendedExecutionContext * pEEC)
{
  DEBUG(dbgs() << "ocl20_enqueue_kernel_localmem. Entry point \n");
  int res = 
    enqueue_kernel_common(queue, flags, ndrange, block, 
                          0, NULL, NULL, // events
                          localbuf_size, localbuf_size_len, // local buffers
                          pEEC);
  return res;
}

extern "C" LLVM_BACKEND_API int ocl20_enqueue_kernel_events(
        queue_t* queue,
        kernel_enqueue_flags_t flags,
        cl_work_description_type* ndrange,
        unsigned num_events_in_wait_list, clk_event_t *in_wait_list,
        clk_event_t *event_ret,
        void *block,
        ExtendedExecutionContext * pEEC)
{
  DEBUG(dbgs() << "ocl20_enqueue_kernel_events. Entry point \n");
  int res = 
    enqueue_kernel_common(queue, flags, ndrange, block, 
                          num_events_in_wait_list, in_wait_list,  // events
                          event_ret, // event ret
                          NULL, 0, // local buffers
                          pEEC);
  return res;
}


extern "C" LLVM_BACKEND_API int ocl20_enqueue_kernel_events_localmem(
        queue_t* queue,
        kernel_enqueue_flags_t flags,
        cl_work_description_type* ndrange,
        unsigned num_events_in_wait_list, clk_event_t *in_wait_list,
        clk_event_t *event_ret,
        void *block,
        unsigned *localbuf_size, unsigned localbuf_size_len,
        ExtendedExecutionContext * pEEC)
{
  DEBUG(dbgs() << "ocl20_enqueue_kernel_events_localmem. Entry point \n");
  int res = 
    enqueue_kernel_common(queue, flags, ndrange, block, 
                          num_events_in_wait_list, in_wait_list,  // events
                          event_ret, // event ret
                          localbuf_size, localbuf_size_len, // local buffers
                          pEEC);
  return res;
}

extern "C" LLVM_BACKEND_API int ocl20_enqueue_marker(
        queue_t* queue,
        uint32_t num_events_in_wait_list,
        const clk_event_t* event_wait_list,
        clk_event_t* event_ret,
        ExtendedExecutionContext * pEEC)
{
  DEBUG(dbgs() << "ocl20_enqueue_marker. Entry point \n");
  assert(pEEC && "ExtendedExecutionContext is NULL");
  IDeviceCommandManager *pDCM = pEEC->GetDeviceCommandManager();
  assert(pDCM && "IDeviceCommandManager is NULL");

  int res = pDCM->EnqueueMarker(
      reinterpret_cast<queue_t>(queue), //queue_t queue
      num_events_in_wait_list, //EnqueueMarker
      event_wait_list, //pEventWaitList
      event_ret // pEventRet
      );
  DEBUG(dbgs() << "ocl20_enqueue_marker. Called EnqueueMarker\n");
  return res;
}

extern "C" LLVM_BACKEND_API void ocl20_retain_event(
        clk_event_t *event,
        ExtendedExecutionContext * pEEC)
{
  DEBUG(dbgs() << "ocl20_retain_event. Entry point \n");
  assert(pEEC && "ExtendedExecutionContext is NULL");
  IDeviceCommandManager *pDCM = pEEC->GetDeviceCommandManager();
  assert(pDCM && "IDeviceCommandManager is NULL");

  int err_code = pDCM->RetainEvent(reinterpret_cast<clk_event_t>(event));
  // spec neither provides interface to return error code nor
  // declares special behavior when error is occured.
  // Lets output error code in debug stream
  if(CL_SUCCESS != err_code)
      DEBUG(dbgs() << "ocl20_retain_event. Failed to execute RetainEvent with Error Code " << err_code << "\n");

  DEBUG(dbgs() << "ocl20_retain_event. Called RetainEvent\n");
  return;
}

extern "C" LLVM_BACKEND_API void ocl20_release_event(
        clk_event_t *event,
        ExtendedExecutionContext * pEEC)
{
  DEBUG(dbgs() << "ocl20_release_event. Entry point \n");
  assert(pEEC && "ExtendedExecutionContext is NULL");
  IDeviceCommandManager *pDCM = pEEC->GetDeviceCommandManager();
  assert(pDCM && "IDeviceCommandManager is NULL");

  int err_code = pDCM->ReleaseEvent(reinterpret_cast<clk_event_t>(event));
  // spec neither provides interface to return error code nor
  // declares special behavior when error is occured.
  // Lets output error code in debug stream
  if(CL_SUCCESS != err_code)
      DEBUG(dbgs() << "ocl20_release_event. Failed to execute ReleaseEvent with Error Code " << err_code << "\n");

  DEBUG(dbgs() << "ocl20_release_event. Called ReleaseEvent\n");
  return;
}

extern "C" LLVM_BACKEND_API clk_event_t* ocl20_create_user_event(ExtendedExecutionContext * pEEC)
{
  DEBUG(dbgs() << "ocl20_create_user_event. Entry point \n");
  assert(pEEC && "ExtendedExecutionContext is NULL");
  IDeviceCommandManager *pDCM = pEEC->GetDeviceCommandManager();
  assert(pDCM && "IDeviceCommandManager is NULL");

  int err_code = CL_SUCCESS;
  clk_event_t ret = pDCM->CreateUserEvent(&err_code);
  // CreateUserEvent returns error code through argument.
  // spec neither provides interface to return error code nor
  // declares special behavior when error is occured.
  // Lets output error code in debug stream
  if(CL_SUCCESS != err_code)
      DEBUG(dbgs() << "ocl20_create_user_event. Failed to execute CreateUserEvent with Error Code " << err_code << "\n");

  DEBUG(dbgs() << "ocl20_create_user_event. Called CreateUserEvent\n");
  return reinterpret_cast<clk_event_t*>(ret);
}

extern "C" LLVM_BACKEND_API void ocl20_set_user_event_status(
        clk_event_t *event,
        uint32_t status,
        ExtendedExecutionContext * pEEC)
{
  DEBUG(dbgs() << "ocl20_set_user_event_status. Entry point \n");
  assert(pEEC && "ExtendedExecutionContext is NULL");
  IDeviceCommandManager *pDCM = pEEC->GetDeviceCommandManager();
  assert(pDCM && "IDeviceCommandManager is NULL");

  int err_code = pDCM->SetEventStatus(reinterpret_cast<clk_event_t>(event), status);
  // spec neither provides interface to return error code nor
  // declares special behavior when error is occured.
  // Lets output error code in debug stream
  if(CL_SUCCESS != err_code)
      DEBUG(dbgs() << "ocl20_create_user_event. Failed to execute CreateUserEvent with Error Code " << err_code << "\n");

  DEBUG(dbgs() << "ocl20_set_user_event_status. Called SetEventStatus\n");
  return;
}

extern "C" LLVM_BACKEND_API void ocl20_capture_event_profiling_info(
        clk_event_t *event,
        clk_profiling_info name,
        uint64_t *value,
        ExtendedExecutionContext * pEEC)
{
  DEBUG(dbgs() << "ocl20_capture_event_profiling_info. Entry point \n");
  assert(pEEC && "ExtendedExecutionContext is NULL");
  IDeviceCommandManager *pDCM = pEEC->GetDeviceCommandManager();
  assert(pDCM && "IDeviceCommandManager is NULL");

  pDCM->CaptureEventProfilingInfo(reinterpret_cast<clk_event_t>(event), name, value);

  DEBUG(dbgs() << "ocl20_capture_event_profiling_info. Called CaptureEventProfilingInfo\n");
  return;
}

extern "C" LLVM_BACKEND_API uint32_t ocl20_get_kernel_wg_size(
        void *block,
        ExtendedExecutionContext * pEEC)
{
  DEBUG(dbgs() << "ocl20_get_kernel_wg_size. Entry point \n");
  uint32_t ret;
  assert(pEEC && "ExtendedExecutionContext is NULL");
  
  const BlockLiteral * pBlockLiteral = static_cast<BlockLiteral*>(block);
  const IBlockToKernelMapper * pMapper = pEEC->GetBlockToKernelMapper();
  assert(pMapper && "IBlockToKernelMapper is NULL");
  
  // obtain entry point as key
  void * key = pBlockLiteral->GetInvoke();
  const ICLDevBackendKernel_ * pKernel = pMapper->Map(key);
  const ICLDevBackendKernelProporties* pKernelProps = pKernel->GetKernelProporties();

  //logic taken from \cpu_device\program_service.cpp file ProgramService::GetKernelInfo function
  // SVN rev. 74469
  ret = std::min<uint64_t>(CPU_MAX_WORK_GROUP_SIZE, (CPU_DEV_MAX_WG_PRIVATE_SIZE / pKernelProps->GetPrivateMemorySize()) );
  ret = ((uint64_t)1) << ((uint64_t)(logf((float)ret)/logf(2.f)));

  DEBUG(dbgs() << "ocl20_get_kernel_wg_size. Called GetKernelWorkGroupSize\n");
  return ret;
}

extern "C" LLVM_BACKEND_API uint32_t ocl20_get_kernel_wg_size_local(
        void *block,
        ExtendedExecutionContext * pEEC)
{
    return ocl20_get_kernel_wg_size(block, pEEC);
}

extern "C" LLVM_BACKEND_API uint32_t ocl20_get_kernel_preferred_wg_size_multiple(
        void *block,
        ExtendedExecutionContext * pEEC)
{
  DEBUG(dbgs() << "ocl20_get_kernel_preferred_wg_size_multiple. Entry point \n");
  uint32_t ret;
  assert(pEEC && "ExtendedExecutionContext is NULL");
  
  const BlockLiteral * pBlockLiteral = static_cast<BlockLiteral*>(block);
  const IBlockToKernelMapper * pMapper = pEEC->GetBlockToKernelMapper();
  assert(pMapper && "IBlockToKernelMapper is NULL");
  
  // obtain entry point as key
  void * key = pBlockLiteral->GetInvoke();
  const ICLDevBackendKernel_ * pKernel = pMapper->Map(key);
  const ICLDevBackendKernelProporties* pKernelProps = pKernel->GetKernelProporties();

  //logic taken from \cpu_device\program_service.cpp file ProgramService::GetKernelInfo function
  // SVN rev. 74469
  ret = pKernelProps->GetKernelPackCount();

  DEBUG(dbgs() << "ocl20_get_kernel_preferred_wg_size_multiple. Called GetKernelPreferredWorkGroupSizeMultiple\n");
  return ret;
}

extern "C" LLVM_BACKEND_API uint32_t ocl20_get_kernel_preferred_wg_size_multiple_local(
        void *block,
        ExtendedExecutionContext * pEEC)
{
    return ocl20_get_kernel_preferred_wg_size_multiple(block, pEEC);
}
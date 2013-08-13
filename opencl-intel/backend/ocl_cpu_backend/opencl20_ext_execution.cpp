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

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

#if defined (__MIC__) || defined(__MIC2__)
  #include "mic_dev_limits.h"
  #define MAX_WORK_GROUP_SIZE      MIC_MAX_WORK_GROUP_SIZE
  #define MAX_WG_PRIVATE_SIZE      MIC_DEV_MAX_WG_PRIVATE_SIZE
#else
  #include "cpu_dev_limits.h"
  #define MAX_WORK_GROUP_SIZE      CPU_MAX_WORK_GROUP_SIZE
  #define MAX_WG_PRIVATE_SIZE      CPU_DEV_MAX_WG_PRIVATE_SIZE
#endif

#include "cl_dev_backend_api.h"
#include "ICLDevBackendServiceFactory.h"
#include "BlockLiteral.h"
#include "IBlockToKernelMapper.h"
#include "cl_device_api.h"
#include "IDeviceCommandManager.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SmallVector.h"

#define DEBUG_TYPE "opencl20-ext-execution"
#include "llvm/Support/Debug.h"

#include <string.h>

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

#define LLVM_BACKEND_NOINLINE_PRE
#include "opencl20_ext_execution.h"
#undef LLVM_BACKEND_NOINLINE_PRE

/////////////////////////////////////////////////////////////////////////////////////////////////////
/// Implementation section
/////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" LLVM_BACKEND_API queue_t
ocl20_get_default_queue(IDeviceCommandManager *DCM) {
  DEBUG(dbgs() << "ocl20_get_default_queue. Entry point \n");
  assert(DCM && "IDeviceCommandManager is NULL");
  
  // todo: enable call to DeviceCommandManager
  queue_t res = DCM->GetDefaultQueueForDevice();
  DEBUG(dbgs() << "ocl20_get_default_queue. Called GetDefaultQueueForDevice\n");
  return res;
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
/// @param DCM - pointer to IDeviceCommandManager object
static int enqueue_kernel_common(
    queue_t queue, kernel_enqueue_flags_t flags, _ndrange_t *ndrange,
    void *block, unsigned num_events_in_wait_list, clk_event_t*in_wait_list,
    clk_event_t*event_ret, unsigned *localbuf_size, unsigned localbuf_size_len,
    IDeviceCommandManager *DCM, const IBlockToKernelMapper *pMapper,
    void *RuntimeHandle) {
  assert(DCM && "IDeviceCommandManager is NULL");
  assert(block && "block is NULL");
  assert(pMapper && "const IBlockToKernelMapper is NULL");

  const BlockLiteral *pBlockLiteral = static_cast<BlockLiteral *>(block);

  // obtain entry point as key
  void * key = pBlockLiteral->GetInvoke();
  // obtain Kernel object from mapper
  const ICLDevBackendKernel_ * pKernel = pMapper->Map(key);
  // convert localbuf of type uint defined by 2.0 spec to type size_t accepted by BE
  SmallVector<size_t, 32> localbuf64_size(localbuf_size_len);
  for (unsigned I=0; I<localbuf_size_len; ++I)
    localbuf64_size[I] = localbuf_size[I];
  size_t *localbuf_size_arg = localbuf_size_len ? &(localbuf64_size[0]) : 0;

  // copy block_literal and set desc ptr field to NULL
  // reason: we dont copy structure pointed by desc pointer
  // we assume desc field is not used in enqueued kernels
  // setting the ptr to zero will simplify to rootcause issue 
  // in case desc field will happen to be used
  SmallVector<char, 256> bl(pBlockLiteral->GetSize());
  ::memcpy(&(bl[0]), block, pBlockLiteral->GetSize());

  BlockLiteral * pBl = reinterpret_cast<BlockLiteral *>(&bl[0]);
  pBl->SetDescPtr(0);

  ///////////////////////////////////////////////////////////////////////
  // call enqueue
  int res = DCM->EnqueueKernel(
      queue,                   // queue_t
      flags,                   // kernel_enqueue_flags_t
      num_events_in_wait_list, // uiNumEventsInWaitList
      in_wait_list,            // pEventWaitList
      event_ret,               // clk_event_tpEventRet
      pKernel, // const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_*
               // pKernel
      &bl[0],           // block literal structure as provided by clang
      bl.size(),               // size of block literal
      localbuf_size_arg,       // spec says its an array of uint but we convert it to size_t
      localbuf_size_len,       // !!!! size_t
      ndrange,                 // const cl_work_description_type* pNdrange
      RuntimeHandle            // pointer provided to RunWG
      );

  return res;
}
extern "C" LLVM_BACKEND_API int
ocl20_enqueue_kernel_basic(queue_t queue, kernel_enqueue_flags_t flags,
                           _ndrange_t *ndrange, void *block,
                           IDeviceCommandManager *DCM,
                           const IBlockToKernelMapper* Mapper,
                           void *RuntimeHandle) {
  DEBUG(dbgs() << "ocl20_enqueue_kernel_basic. Entry point \n");
  DEBUG(dbgs() << "Enqueued ndrange_1d \n"
               << " workDimension " << ndrange->workDimension
               << " globalWorkSize0 " << ndrange->globalWorkSize[0]
               << " localWorkSize0 " << ndrange->localWorkSize[0]
               << " globalWorkOffset0 " << ndrange->globalWorkOffset[0]
               << "\n");

  int res = enqueue_kernel_common(queue, flags, ndrange, block, 0, NULL,
                                  NULL,    // events
                                  NULL, 0, // local buffers
                                  DCM, Mapper, RuntimeHandle);
  DEBUG(dbgs() << "ocl20_enqueue_kernel_basic. Return value " << res << "\n");
  return res;
}

extern "C" LLVM_BACKEND_API int ocl20_enqueue_kernel_localmem(
    queue_t queue, kernel_enqueue_flags_t flags, _ndrange_t *ndrange,
    void *block, unsigned localbuf_size_len, unsigned *localbuf_size,
    IDeviceCommandManager *DCM, const IBlockToKernelMapper *Mapper,
    void *RuntimeHandle) {
  DEBUG(dbgs() << "ocl20_enqueue_kernel_localmem. Entry point \n");
  int res =
    enqueue_kernel_common(queue, flags, ndrange, block,
                          0, NULL, NULL, // events
                          localbuf_size, localbuf_size_len, // local buffers
                          DCM, Mapper, RuntimeHandle);
  return res;
}

extern "C" LLVM_BACKEND_API int ocl20_enqueue_kernel_events(
    queue_t queue, kernel_enqueue_flags_t flags, _ndrange_t *ndrange,
    unsigned num_events_in_wait_list, clk_event_t *in_wait_list,
    clk_event_t *event_ret, void *block, IDeviceCommandManager *DCM,
    const IBlockToKernelMapper *Mapper, void *RuntimeHandle) {
  DEBUG(dbgs() << "ocl20_enqueue_kernel_events. Entry point \n");
  int res =
      enqueue_kernel_common(queue, flags, ndrange, block,
                            num_events_in_wait_list, in_wait_list, // events
                            event_ret,                             // event ret
                            NULL, 0, // local buffers
                            DCM, Mapper, RuntimeHandle);
  return res;
}

extern "C" LLVM_BACKEND_API int ocl20_enqueue_kernel_events_localmem(
    queue_t queue, kernel_enqueue_flags_t flags, _ndrange_t *ndrange,
    unsigned num_events_in_wait_list, clk_event_t *in_wait_list,
    clk_event_t *event_ret, void *block, unsigned localbuf_size_len,
    unsigned *localbuf_size, IDeviceCommandManager *DCM,
    const IBlockToKernelMapper *Mapper, void *RuntimeHandle) {
  DEBUG(dbgs() << "ocl20_enqueue_kernel_events_localmem. Entry point \n");
  int res =
    enqueue_kernel_common(queue, flags, ndrange, block,
                          num_events_in_wait_list, in_wait_list,  // events
                          event_ret, // event ret
                          localbuf_size, localbuf_size_len, // local buffers
                          DCM, Mapper, RuntimeHandle);
  return res;
}

extern "C" LLVM_BACKEND_API int
ocl20_enqueue_marker(queue_t queue, uint32_t num_events_in_wait_list,
                     const clk_event_t *event_wait_list, clk_event_t *event_ret,
                     IDeviceCommandManager *DCM) {
  DEBUG(dbgs() << "ocl20_enqueue_marker. Entry point \n");
  assert(DCM && "IDeviceCommandManager is NULL");

  int res = DCM->EnqueueMarker(queue,                   // queue_t queue
                               num_events_in_wait_list, // EnqueueMarker
                               event_wait_list,         // pEventWaitList
                               event_ret                // pEventRet
                               );
  DEBUG(dbgs() << "ocl20_enqueue_marker. Called EnqueueMarker\n");
  return res;
}

extern "C" LLVM_BACKEND_API void
ocl20_retain_event(clk_event_t event, IDeviceCommandManager *DCM) {
  DEBUG(dbgs() << "ocl20_retain_event. Entry point \n");
  assert(DCM && "IDeviceCommandManager is NULL");

  int err_code = DCM->RetainEvent(event);
  // spec neither provides interface to return error code nor
  // declares special behavior when error is occured.
  // Lets output error code in debug stream
  if (CL_SUCCESS != err_code)
      DEBUG(dbgs() << "ocl20_retain_event. Failed to execute RetainEvent with Error Code " << err_code << "\n");

  DEBUG(dbgs() << "ocl20_retain_event. Called RetainEvent\n");
  return;
}

extern "C" LLVM_BACKEND_API void
ocl20_release_event(clk_event_t event, IDeviceCommandManager *DCM) {
  DEBUG(dbgs() << "ocl20_release_event. Entry point \n");
  assert(DCM && "IDeviceCommandManager is NULL");

  int err_code = DCM->ReleaseEvent(event);
  // spec neither provides interface to return error code nor
  // declares special behavior when error is occured.
  // Lets output error code in debug stream
  if(CL_SUCCESS != err_code)
      DEBUG(dbgs() << "ocl20_release_event. Failed to execute ReleaseEvent with Error Code " << err_code << "\n");

  DEBUG(dbgs() << "ocl20_release_event. Called ReleaseEvent\n");
  return;
}

extern "C" LLVM_BACKEND_API bool
ocl20_is_valid_event(clk_event_t event, IDeviceCommandManager *DCM) {
  DEBUG(dbgs() << "ocl20_is_valid_event. Entry point \n");
  assert(DCM && "IDeviceCommandManager is NULL");

  bool res = DCM->IsValidEvent(event);
 
  DEBUG(dbgs() << "ocl20_is_valid_event. Called IsValidEvent\n");
  return res;
}

extern "C" LLVM_BACKEND_API clk_event_t
ocl20_create_user_event(IDeviceCommandManager *DCM) {
  DEBUG(dbgs() << "ocl20_create_user_event. Entry point \n");
  assert(DCM && "IDeviceCommandManager is NULL");

  int err_code = CL_SUCCESS;
  clk_event_t ret = DCM->CreateUserEvent(&err_code);
  // CreateUserEvent returns error code through argument.
  // spec neither provides interface to return error code nor
  // declares special behavior when error is occured.
  // Lets output error code in debug stream
  if(CL_SUCCESS != err_code)
      DEBUG(dbgs() << "ocl20_create_user_event. Failed to execute CreateUserEvent with Error Code " << err_code << "\n");

  DEBUG(dbgs() << "ocl20_create_user_event. Called CreateUserEvent\n");
  return ret;
}

extern "C" LLVM_BACKEND_API void
ocl20_set_user_event_status(clk_event_t event, uint32_t status,
                            IDeviceCommandManager *DCM) {
  DEBUG(dbgs() << "ocl20_set_user_event_status. Entry point \n");
  assert(DCM && "IDeviceCommandManager is NULL");

  int err_code = DCM->SetEventStatus(event, status);
  // spec neither provides interface to return error code nor
  // declares special behavior when error is occured.
  // Lets output error code in debug stream
  if(CL_SUCCESS != err_code)
      DEBUG(dbgs() << "ocl20_create_user_event. Failed to execute CreateUserEvent with Error Code " << err_code << "\n");

  DEBUG(dbgs() << "ocl20_set_user_event_status. Called SetEventStatus\n");
  return;
}

extern "C" LLVM_BACKEND_API void
ocl20_capture_event_profiling_info(clk_event_t event, clk_profiling_info name,
                                   uint64_t *value,
                                   IDeviceCommandManager *DCM) {
  DEBUG(dbgs() << "ocl20_capture_event_profiling_info. Entry point \n");
  assert(DCM && "IDeviceCommandManager is NULL");

  DCM->CaptureEventProfilingInfo(event, name, value);

  DEBUG(dbgs() << "ocl20_capture_event_profiling_info. Called CaptureEventProfilingInfo\n");
  return;
}

extern "C" LLVM_BACKEND_API uint32_t
ocl20_get_kernel_wg_size(void *block, IDeviceCommandManager *DCM, const IBlockToKernelMapper* Mapper) {
  assert(DCM && "IDeviceCommandManager is NULL");
  assert(Mapper && "const IBlockToKernelMapper is NULL");
  DEBUG(dbgs() << "ocl20_get_kernel_wg_size. Entry point \n");

  const BlockLiteral * pBlockLiteral = static_cast<BlockLiteral*>(block);

  // obtain entry point as key
  void * key = pBlockLiteral->GetInvoke();
  const ICLDevBackendKernel_ * pKernel = Mapper->Map(key);
  const ICLDevBackendKernelProporties* pKernelProps = pKernel->GetKernelProporties();

  DEBUG(dbgs() << "ocl20_get_kernel_wg_size. Called GetKernelWorkGroupSize\n");
  DEBUG(dbgs() << "ocl20_get_kernel_wg_size. return value=" << pKernelProps->GetMaxWorkGroupSize(MAX_WORK_GROUP_SIZE, MAX_WG_PRIVATE_SIZE)<<"\n");
  return pKernelProps->GetMaxWorkGroupSize(MAX_WORK_GROUP_SIZE, MAX_WG_PRIVATE_SIZE);
}

extern "C" LLVM_BACKEND_API uint32_t
ocl20_get_kernel_preferred_wg_size_multiple(void *block,
                                            IDeviceCommandManager *DCM,
                                            const IBlockToKernelMapper *Mapper) {
  DEBUG(dbgs() << "ocl20_get_kernel_preferred_wg_size_multiple. Entry point \n");
  uint32_t ret;
  assert(DCM && "IDeviceCommandManager is NULL");
  assert(Mapper && "const IBlockToKernelMapper is NULL");

  const BlockLiteral * pBlockLiteral = static_cast<BlockLiteral*>(block);

  // obtain entry point as key
  void * key = pBlockLiteral->GetInvoke();
  const ICLDevBackendKernel_ * pKernel = Mapper->Map(key);
  const ICLDevBackendKernelProporties* pKernelProps = pKernel->GetKernelProporties();

  //logic taken from \cpu_device\program_service.cpp file ProgramService::GetKernelInfo function
  // SVN rev. 74469
  ret = pKernelProps->GetKernelPackCount();

  DEBUG(dbgs() << "ocl20_get_kernel_preferred_wg_size_multiple. Called GetKernelPreferredWorkGroupSizeMultiple\n");
  return ret;
}

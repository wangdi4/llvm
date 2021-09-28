// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END

#include "level_zero/include/ze_api.h"
#include "ze_api_extend.h"
#include "ze_buffer.hpp"
#include "ze_cmdlist.hpp"
#include "ze_cmdqueue.hpp"
#include "ze_common.hpp"
#include "ze_device.hpp"
#include "ze_driver.hpp"
#include "ze_event.hpp"
#include "ze_kernel.hpp"
#include "ze_module.hpp"
#include "ze_utils.hpp"
#include <array>

/* ZE simulation runtime implementation for GenISA interpreter */
/* ----------------------------------------------------------- */

namespace __zert__ {
extern "C" {

//=============================================================================
ZE_APIEXPORT ze_result_t ZE_APICALL zeInit(ze_init_flags_t flags) {
  // TODO: Currently only ZE_INIT_FLAG_GPU_ONLY and ZE_INIT_FLAG_VPU_ONLY are
  // defined in ze_api.h. Once the ZE_INIT_FLAG_CPU_ONLY is added, we need to
  // change the check to ZE_INIT_FLAG_CPU_ONLY.

  if (flags && !(flags & ZE_INIT_FLAG_GPU_ONLY)) {
    ZESIMERR << "Invalid flags " << flags;
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }
  ZeDriver::init();
  return ZE_RESULT_SUCCESS;
}

//=============================================================================

ZE_APIEXPORT ze_result_t ZE_APICALL zeDriverGet(uint32_t *pCount,
                                                ze_driver_handle_t *phDrivers) {
  if (nullptr == pCount) {
    ZESIMERR << "pCount is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  if (0 == *pCount) {
    *pCount = 1;
    return ZE_RESULT_SUCCESS;
  }
  if (*pCount > 1) {
    *pCount = 1;
  }

  if (nullptr == phDrivers) {
    ZESIMERR << "phDriver is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  for (uint32_t i = 0; i < *pCount; ++i) {
    phDrivers[i] = &ZeDriver::getInstance();
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeDriverGetProperties(
    ze_driver_handle_t hDriver, ze_driver_properties_t *pDriverProperties) {
  if (nullptr == hDriver || nullptr == pDriverProperties) {
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }
  return static_cast<ZeDriver *>(hDriver)->getProperties(pDriverProperties);
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeDriverGetApiVersion(ze_driver_handle_t hDriver, ze_api_version_t *version) {
  if (nullptr == hDriver || nullptr == version) {
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }
  return static_cast<ZeDriver *>(hDriver)->getApiVersion(version);
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeDriverGetExtensionProperties(
    ze_driver_handle_t hDriver, uint32_t *pCount,
    ze_driver_extension_properties_t *pExtensionProperties) {
  if (nullptr == hDriver || nullptr == pExtensionProperties) {
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }
  return static_cast<ZeDriver *>(hDriver)->getExtensionProperties(
      pCount, pExtensionProperties);
}

//=============================================================================

ZE_APIEXPORT ze_result_t ZE_APICALL zeDeviceGet(ze_driver_handle_t hDriver,
                                                uint32_t *pCount,
                                                ze_device_handle_t *phDevices) {
  if (nullptr == hDriver || nullptr == pCount) {
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }
  auto const dcount =
      static_cast<uint32_t>(static_cast<ZeDriver *>(hDriver)->devices().size());
  if (0 == *pCount) {
    *pCount = dcount;
    return ZE_RESULT_SUCCESS;
  }

  if (*pCount > dcount) {
    *pCount = dcount;
  }

  if (nullptr == phDevices) {
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }

  for (uint32_t i = 0; i < *pCount; ++i) {
    phDevices[i] = static_cast<ZeDriver *>(hDriver)->devices()[i];
  }

  return ZE_RESULT_SUCCESS;
}

//=============================================================================
ZE_APIEXPORT ze_result_t ZE_APICALL zeDeviceGetComputeProperties(
    ze_device_handle_t hDevice,
    ze_device_compute_properties_t *pComputeProperties) {
  if (nullptr == hDevice) {
    ZESIMERR << "zeDeviceGetSubDevices: hDevice is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  return static_cast<ZeDevice *>(hDevice)->getComputeProperties(
      pComputeProperties);
}

//=============================================================================
ZE_APIEXPORT ze_result_t ZE_APICALL
zeDeviceGetSubDevices(ze_device_handle_t hDevice, uint32_t *pCount,
                      ze_device_handle_t *phSubdevices) {
  if (nullptr == hDevice) {
    ZESIMERR << "zeDeviceGetSubDevices: hDevice is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (nullptr == pCount) {
    ZESIMERR << "zeDeviceGetSubDevices: pCount is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  uint32_t subdevice_count =
      uint32_t(static_cast<ZeDevice *>(hDevice)->subdeviceCount());
  if (nullptr == phSubdevices) {
    *pCount = subdevice_count;
  } else {
    if (*pCount != subdevice_count) {
      ZESIMERR << "zeDeviceGetSubDevices: pCount=" << *pCount
               << " but expecting " << subdevice_count;
      return ZE_RESULT_ERROR_INVALID_ARGUMENT;
    }
    for (uint32_t i = 0; i < subdevice_count; ++i) {
      phSubdevices[i] = static_cast<ZeDevice *>(hDevice)->subdevice(i);
    }
  }
  return ZE_RESULT_SUCCESS;
}

//=============================================================================

ZE_APIEXPORT ze_result_t ZE_APICALL zeDeviceGetProperties(
    ze_device_handle_t _hDevice, ze_device_properties_t *pDeviceProperties) {
  if (nullptr == _hDevice || nullptr == pDeviceProperties) {
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }
  auto hDevice = static_cast<ZeDevice *>(_hDevice);
  *pDeviceProperties = hDevice->properties();
  return ZE_RESULT_SUCCESS;
}

//=============================================================================

ZE_APIEXPORT ze_result_t ZE_APICALL zeContextCreate(
    ze_driver_handle_t hDriver,    ///< [in] handle of the driver object
    const ze_context_desc_t *desc, ///< [in] pointer to context descriptor
    ze_context_handle_t
        *phContext ///< [out] pointer to handle of context object created
    )              //
{
  if (nullptr == hDriver) {
    ZESIMERR << "hDriver is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (nullptr == desc) {
    ZESIMERR << "desc is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  return static_cast<ZeDriver *>(hDriver)->createContext(*desc, phContext);
}

//=============================================================================
ZE_APIEXPORT ze_result_t ZE_APICALL zeContextDestroy(
    ze_context_handle_t
        hContext) ///< [in][release] handle of context object to destroy
{
  if (nullptr == hContext) {
    ZESIMERR << "hContext is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  ZeContext *zeContext = static_cast<ZeContext *>(hContext);
  return zeContext->driver()->destroyContext(zeContext);
}

//=============================================================================

ZE_APIEXPORT ze_result_t ZE_APICALL
zeCommandQueueCreate(ze_context_handle_t hContext, ze_device_handle_t hDevice,
                     const ze_command_queue_desc_t *desc,
                     ze_command_queue_handle_t *phCommandQueue) {
  if (nullptr == hContext) {
    ZESIMERR << "hContext is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (nullptr == hDevice) {
    ZESIMERR << "hDevice is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (nullptr == desc) {
    ZESIMERR << "desc is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  if (nullptr == phCommandQueue) {
    ZESIMERR << "phCommandQueue is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  return static_cast<ZeDevice *>(hDevice)->createCmdQueue(
      static_cast<ZeContext *>(hContext), desc, phCommandQueue);
}

//=============================================================================

ZE_APIEXPORT ze_result_t ZE_APICALL
zeCommandQueueDestroy(ze_command_queue_handle_t hCommandQueue) {
  if (nullptr == hCommandQueue) {
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }
  return static_cast<ZeCmdQueue *>(hCommandQueue)
      ->device()
      ->destroyCmdQueue(hCommandQueue);
}

//=============================================================================
//
ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListCreate(
    ze_context_handle_t hContext, ///< [in] handle of the context object
    ze_device_handle_t hDevice,   ///< [in] handle of the device object
    const ze_command_list_desc_t
        *desc, ///< [in] pointer to command list descriptor
    ze_command_list_handle_t *phCommandList ///< [out] pointer to handle of
                                            ///< command list object created
) {
  if (nullptr == hContext) {
    ZESIMERR << "hContext is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (nullptr == hDevice) {
    ZESIMERR << "hDevice in null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (nullptr == desc) {
    ZESIMERR << "desc is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  if (nullptr == phCommandList) {
    ZESIMERR << "phCommandList is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  return static_cast<ZeDevice *>(hDevice)->createCmdList(
      static_cast<ZeContext *>(hContext), desc, phCommandList);
}

//=============================================================================
//
ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListCreateImmediate(
    ze_context_handle_t hContext, ///< [in] handle of the context object
    ze_device_handle_t hDevice,   ///< [in] handle of the device object
    const ze_command_queue_desc_t
        *desc, ///< [in] pointer to command queue descriptor
    ze_command_list_handle_t *phCommandList ///< [out] pointer to handle of
                                            ///< command list object created
) {
  if (nullptr == hContext) {
    ZESIMERR << "hContext is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (nullptr == hDevice) {
    ZESIMERR << "hDevice in null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (nullptr == desc) {
    ZESIMERR << "desc is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  if (nullptr == phCommandList) {
    ZESIMERR << "phCommandList is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  return static_cast<ZeDevice *>(hDevice)->createCmdList(
      static_cast<ZeContext *>(hContext), desc, phCommandList);
}

//=============================================================================

ZE_APIEXPORT ze_result_t ZE_APICALL
zeCommandListDestroy(ze_command_list_handle_t hCommandList) {
  if (nullptr == hCommandList) {
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }
  return static_cast<ZeCmdList *>(hCommandList)
      ->device()
      ->destroyCmdList(hCommandList);
}

//=============================================================================
//
ZE_APIEXPORT ze_result_t ZE_APICALL
zeCommandListReset(ze_command_list_handle_t hCommandList) {
  if (nullptr == hCommandList) {
    ZESIMERR << "hCommandList handle is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  return static_cast<ZeCmdList *>(hCommandList)->reset();
}

//=============================================================================
ZE_APIEXPORT ze_result_t ZE_APICALL
zeCommandListClose(ze_command_list_handle_t hCommandList) {
  if (nullptr == hCommandList)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  return static_cast<ZeCmdList *>(hCommandList)->close();
}

//=============================================================================

ZE_APIEXPORT ze_result_t ZE_APICALL zeMemAllocDevice(
    ze_context_handle_t hContext, ///< [in] handle of the context object
    const ze_device_mem_alloc_desc_t
        *device_desc, ///< [in] pointer to device memory allocation descriptor
    size_t size,      ///< [in] size in bytes to allocate; must be less-than
                      ///< ::ze_device_properties_t.maxMemAllocSize.
    size_t alignment, ///< [in] minimum alignment in bytes for the
                      ///< allocation; must be a power of two.
    ze_device_handle_t hDevice, ///< [in] handle of the device
    void **pptr                 ///< [out] pointer to device allocation
    )                           //
{
  if (nullptr == hContext) {
    ZESIMERR << "hContext is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (nullptr == hDevice)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  if (nullptr == pptr)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  if (nullptr == device_desc)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  ZeBuffer *buf = static_cast<ZeContext *>(hContext)->driver()->makeZeBuffer(
      static_cast<ZeDevice *>(hDevice));
  if (nullptr == buf) {
    return ZE_RESULT_ERROR_UNKNOWN;
  }
  if (ZE_RESULT_SUCCESS != buf->allocate(*device_desc, size, alignment)) {
    return ZE_RESULT_ERROR_UNKNOWN;
  }
  *pptr = buf->ptr_beg();
  return ZE_RESULT_SUCCESS;
}

//=============================================================================

ZE_APIEXPORT ze_result_t ZE_APICALL
zeMemFree(ze_context_handle_t hContext, ///< [in] handle of the context object
          void *ptr ///< [in][release] pointer to memory to free
          )         //
{
  if (nullptr == hContext) {
    ZESIMERR << "hContext is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (nullptr == ptr)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  auto buf = static_cast<ZeContext *>(hContext)->driver()->getZeBuffer(ptr);
  if (nullptr == buf || buf->ptr_beg() != ptr) {
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }
  return static_cast<ZeContext *>(hContext)->driver()->destroyZeBuffer(buf);
}

//=============================================================================

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListAppendMemoryCopy(
    ze_command_list_handle_t hCommandList, ///< [in] handle of command list
    void *dstptr,       ///< [in] pointer to destination memory to copy to
    const void *srcptr, ///< [in] pointer to source memory to copy from
    size_t size,        ///< [in] size in bytes to copy
    ze_event_handle_t hSignalEvent, ///< [in][optional] handle of the event
                                    ///< to signal on completion
    uint32_t
        numWaitEvents, ///< [in][optional] number of events to wait on before
                       ///< launching; must be 0 if `nullptr == phWaitEvents`
    ze_event_handle_t
        *phWaitEvents ///< [in][optional][range(0, numWaitEvents)] handle of
                      ///< the events to wait on before launching
) {
  if (nullptr == hCommandList) {
    ZESIMERR << "hCommandList is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (nullptr == dstptr) {
    ZESIMERR << "dstptr is null";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  if (nullptr == srcptr) {
    ZESIMERR << "srcpointer is null";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  if (numWaitEvents > 0 && nullptr == phWaitEvents) {
    ZESIMERR << "phWaitEvents is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  std::vector<ZeEvent *> wait_events;
  for (uint32_t i = 0; i < numWaitEvents; ++i) {
    wait_events.push_back(static_cast<ZeEvent *>(phWaitEvents[i]));
  }

  auto cmdlist = static_cast<ZeCmdList *>(hCommandList);
  auto srcbuf = cmdlist->device()->driver()->getZeBuffer(srcptr);
  auto dstbuf = cmdlist->device()->driver()->getZeBuffer(dstptr);

  // check if src is within bounds, if owned by runtime
  if (srcbuf != nullptr && (uint8_t *)srcptr + size > srcbuf->ptr_end()) {
    ZESIMERR << "src is not within bounds";
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }

  // check if dst is wihtin bounds, if owned by runtime
  if (dstbuf != nullptr && (uint8_t *)dstptr + size > dstbuf->ptr_end()) {
    ZESIMERR << "dst is not within bounds";
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }

  auto device = cmdlist->device();
  return cmdlist->append(
      static_cast<ZeEvent *>(hSignalEvent), wait_events,
      [device, srcbuf, dstbuf, dstptr, srcptr, size]() //
      { return ZeBuffer::copy(device, srcbuf, dstbuf, dstptr, srcptr, size); });
}

//=============================================================================

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandQueueExecuteCommandLists(
    ze_command_queue_handle_t hCommandQueue, uint32_t numCommandLists,
    ze_command_list_handle_t *phCommandLists, ze_fence_handle_t hFence) {
  if (nullptr == hCommandQueue) {
    ZESIMERR << "hCommandQueue is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (nullptr == phCommandLists) {
    ZESIMERR << "phCommandLists is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (nullptr != hFence) {
    ZESIMERR << "hFence is not supported yet";
    return ZE_RESULT_ERROR_UNKNOWN;
  }

  auto cmdqueue = static_cast<ZeCmdQueue *>(hCommandQueue);
  std::vector<ZeCmdList *> cmdlists(numCommandLists);
  bool is_all_closed = true;
  for (uint32_t i = 0; i < numCommandLists; ++i) {
    cmdlists[i] = static_cast<ZeCmdList *>(phCommandLists[i]);
    if (!cmdlists[i]->is_closed()) {
      ZESIMERR << "cmdList with index=" << i << " is not closed";
    }
    is_all_closed &= cmdlists[i]->is_closed();
  }
  if (!is_all_closed) {
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }
  return cmdqueue->queue_execute(cmdlists);
}

//=============================================================================

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandQueueSynchronize(
    ze_command_queue_handle_t hCommandQueue, uint64_t timeout) {
  if (nullptr == hCommandQueue)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  return static_cast<ZeCmdQueue *>(hCommandQueue)->sync(timeout);
}

//=============================================================================

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListAppendBarrier(
    ze_command_list_handle_t hCommandList, ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents) {
  if (nullptr == hCommandList) {
    ZESIMERR << "hCommandList is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (numWaitEvents > 0 && nullptr == phWaitEvents) {
    ZESIMERR << "phWaitEvents is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  std::vector<ZeEvent *> wait_events;
  for (uint32_t i = 0; i < numWaitEvents; ++i) {
    wait_events.push_back(static_cast<ZeEvent *>(phWaitEvents[i]));
  }
  return static_cast<ZeCmdList *>(hCommandList)
      ->barrier(static_cast<ZeEvent *>(hSignalEvent), wait_events);
}

//=============================================================================

ZE_APIEXPORT ze_result_t ZE_APICALL zeModuleCreate(
    ze_context_handle_t hContext, ///< [in] handle of the context object
    ze_device_handle_t hDevice,   ///< [in] handle of the device
    const ze_module_desc_t *desc, ///< [in] pointer to module descriptor
    ze_module_handle_t
        *phModule, ///< [out] pointer to handle of module object created
    ze_module_build_log_handle_t *phBuildLog ///< [out][optional] pointer to
                                             ///< handle of module's build log.
) {
  if (nullptr == hContext) {
    ZESIMERR << "hContext is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (nullptr == hDevice) {
    ZESIMERR << "hDevice is null";
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }
  if (nullptr == desc) {
    ZESIMERR << "descriptor is nullptr";
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }
  if (nullptr == phModule) {
    ZESIMERR << "phModule is nullptr";
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }
  // we don't support module build log
  (void)phBuildLog;
  ZESIMOUT << "phBuildLog not supported";

  return static_cast<ZeDevice *>(hDevice)->createModule(
      static_cast<ZeContext *>(hContext), desc, phModule);
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeModuleDestroy(ze_module_handle_t hModule) {
  if (nullptr == hModule)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  return static_cast<ZeModule *>(hModule)->device()->destroyModule(hModule);
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeKernelCreate(ze_module_handle_t hModule, const ze_kernel_desc_t *desc,
               ze_kernel_handle_t *phKernel) {
  if (nullptr == hModule) {
    ZESIMERR << "hModule is null";
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }
  if (nullptr == desc) {
    ZESIMERR << "desc is nullptr";
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }
  if (nullptr == phKernel) {
    ZESIMERR << "phKernel is nullptr";
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }
  return static_cast<ZeModule *>(hModule)->createKernel(desc, phKernel);
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeKernelDestroy(ze_kernel_handle_t hKernel) {
  return static_cast<ZeKernel *>(hKernel)->module()->destroyKernel(hKernel);
}

///////////////////////////////////////////////////////////////////////////////
//

ZE_APIEXPORT ze_result_t ZE_APICALL
zeKernelSetArgumentValue(ze_kernel_handle_t hKernel, uint32_t argIndex,
                         size_t argSize, const void *pArgValue) {
  if (nullptr == hKernel)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  return static_cast<ZeKernel *>(hKernel)->setArgumentValue(argIndex, argSize,
                                                            pArgValue);
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeKernelSetGroupSize(ze_kernel_handle_t hKernel, uint32_t groupSizeX,
                     uint32_t groupSizeY, uint32_t groupSizeZ) {
  if (nullptr == hKernel)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  return static_cast<ZeKernel *>(hKernel)->setGroupSize(groupSizeX, groupSizeY,
                                                        groupSizeZ);
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListAppendLaunchKernel(
    ze_command_list_handle_t hCommandList, ze_kernel_handle_t hKernel,
    const ze_group_count_t *pLaunchFuncArgs, ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents) {
  if (nullptr == hCommandList) {
    ZESIMERR << "hCommandList is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (nullptr == hKernel) {
    ZESIMERR << "hKernel is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (nullptr == pLaunchFuncArgs) {
    ZESIMERR << "phLaunchFuncArgs is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  if (numWaitEvents > 0 && nullptr == phWaitEvents) {
    ZESIMERR << "phEvents is nullptr, but numWaitEvents is " << numWaitEvents;
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  auto cmdlist = static_cast<ZeCmdList *>(hCommandList);
  auto kernel = static_cast<ZeKernel *>(hKernel);
  auto config = *pLaunchFuncArgs;
  if (ZE_RESULT_SUCCESS != kernel->setGroupCount(config.groupCountX,
                                                 config.groupCountY,
                                                 config.groupCountZ)) {
    ZESIMERR << "Failed to set group count";
    return ZE_RESULT_ERROR_UNKNOWN;
  }
  if (kernel->paramCount() != kernel->argCount()) {
    ZESIMERR << "Expecting " << kernel->paramCount()
             << " kernel arguments, but got " << kernel->argCount();
    return ZE_RESULT_ERROR_UNKNOWN;
  }
  std::vector<ZeEvent *> wait_events;
  for (uint32_t i = 0; i < numWaitEvents; ++i) {
    wait_events.push_back(static_cast<ZeEvent *>(phWaitEvents[i]));
  }
  return cmdlist->append(static_cast<ZeEvent *>(hSignalEvent), wait_events,
                         kernel,
                         [config, kernel]() //
                         { return kernel->launch(config); });
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeSimModuleSetLargeGRFMode(ze_module_handle_t hModule, bool isLargeGRFMode) {
  if (nullptr == hModule && isLargeGRFMode)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  return ZE_RESULT_SUCCESS;
  // cdai2
  //  return static_cast<ZeModule*>(hModule)->setLargeGRFMode(isLargeGRFMode);
}

///////////////////////////////////////////////////////////////////////////////
// Events API
///////////////////////////////////////////////////////////////////////////////

ZE_APIEXPORT ze_result_t ZE_APICALL zeEventPoolCreate(
    ze_context_handle_t hContext,     ///< [in] handle of the context object
    const ze_event_pool_desc_t *desc, ///< [in] pointer to event pool descriptor
    uint32_t numDevices, ///< [in][optional] number of device handles;
                         ///< must be 0 if `nullptr == phDevices`
    ze_device_handle_t
        *phDevices, ///< [in][optional][range(0, numDevices)] array of
                    ///< device handles which have visibility to the event
                    ///< pool. if nullptr, then event pool is visible to
                    ///< all devices supported by the driver instance.
    ze_event_pool_handle_t *phEventPool ///< [out] pointer handle of
                                        ///< event pool object created
) {
  if (nullptr == hContext) {
    ZESIMERR << "hDriver is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (nullptr == desc) {
    ZESIMERR << "desc is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  if (numDevices == 0) {
    ZESIMERR << "numDevices is 0";
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }
  if (nullptr == phDevices) {
    ZESIMERR << "phDevices is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  if (nullptr == phEventPool) {
    ZESIMERR << "phEventPool is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  std::vector<ZeDevice *> devices;
  for (uint32_t i = 0; i < numDevices; ++i) {
    devices.push_back(static_cast<ZeDevice *>(phDevices[i]));
  }
  return static_cast<ZeContext *>(hContext)->driver()->createEventPool(
      desc, devices, phEventPool);
}

//-----------------------------------------------------------------------------

ZE_APIEXPORT ze_result_t ZE_APICALL
zeEventPoolDestroy(ze_event_pool_handle_t hEventPool) {
  if (nullptr == hEventPool) {
    ZESIMERR << "hEventPool is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  return ZeEventPool::destroy(static_cast<ZeEventPool *>(hEventPool));
}

//-----------------------------------------------------------------------------

ZE_APIEXPORT ze_result_t ZE_APICALL
zeEventCreate(ze_event_pool_handle_t hEventPool, const ze_event_desc_t *desc,
              ze_event_handle_t *phEvent) {
  if (nullptr == hEventPool) {
    ZESIMERR << "hEventPull is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (nullptr == desc) {
    ZESIMERR << "desc is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  if (nullptr == phEvent) {
    ZESIMERR << "phEvent is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  return static_cast<ZeEventPool *>(hEventPool)->createEvent(*desc, phEvent);
}

//-----------------------------------------------------------------------------

ZE_APIEXPORT ze_result_t ZE_APICALL zeEventDestroy(ze_event_handle_t hEvent) {
  if (nullptr == hEvent) {
    ZESIMERR << "hEventis null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  return static_cast<ZeEvent *>(hEvent)->event_pool()->destroyEvent(
      static_cast<ZeEvent *>(hEvent));
}

//-----------------------------------------------------------------------------

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListAppendSignalEvent(
    ze_command_list_handle_t hCommandList, ze_event_handle_t hEvent) {
  if (nullptr == hCommandList) {
    ZESIMERR << "hCommandList is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (nullptr == hEvent) {
    ZESIMERR << "hEvent is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  return static_cast<ZeCmdList *>(hCommandList)
      ->append_signal_event(static_cast<ZeEvent *>(hEvent));
}

//-----------------------------------------------------------------------------

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListAppendWaitOnEvents(
    ze_command_list_handle_t hCommandList, uint32_t numEvents,
    ze_event_handle_t *phEvents) {
  if (nullptr == hCommandList) {
    ZESIMERR << "hCommandList is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (numEvents > 0 && nullptr == phEvents) {
    ZESIMERR << "phEvents is nullptr, but numEvents is " << numEvents;
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  std::vector<ZeEvent *> wait_events;
  for (uint32_t i = 0; i < numEvents; ++i) {
    wait_events.push_back(static_cast<ZeEvent *>(phEvents[i]));
  }
  return static_cast<ZeCmdList *>(hCommandList)
      ->append(nullptr, wait_events, []() { return ZE_RESULT_SUCCESS; });
}

//-----------------------------------------------------------------------------

ZE_APIEXPORT ze_result_t ZE_APICALL
zeEventHostSignal(ze_event_handle_t hEvent) {
  if (nullptr == hEvent) {
    ZESIMERR << "hEvent is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  return ZeEvent::signal(static_cast<ZeEvent *>(hEvent), ZE_RESULT_SUCCESS);
}

//-----------------------------------------------------------------------------

ZE_APIEXPORT ze_result_t ZE_APICALL
zeEventHostSynchronize(ze_event_handle_t hEvent, uint64_t timeout) {
  if (nullptr == hEvent) {
    ZESIMERR << "hEvent is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  return static_cast<ZeEvent *>(hEvent)->hostSync(timeout);
}

//-----------------------------------------------------------------------------

ZE_APIEXPORT ze_result_t ZE_APICALL
zeEventQueryStatus(ze_event_handle_t hEvent) {
  if (nullptr == hEvent) {
    ZESIMERR << "hEvent is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  return static_cast<ZeEvent *>(hEvent)->queryStatus();
}

//-----------------------------------------------------------------------------

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListAppendEventReset(
    ze_command_list_handle_t hCommandList, ze_event_handle_t hEvent) {
  if (nullptr == hCommandList) {
    ZESIMERR << "hCommandList is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (nullptr == hEvent) {
    ZESIMERR << "hEvent is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  return static_cast<ZeCmdList *>(hCommandList)
      ->append_reset_event(static_cast<ZeEvent *>(hEvent));
}

//-----------------------------------------------------------------------------

ZE_APIEXPORT ze_result_t ZE_APICALL zeEventHostReset(ze_event_handle_t hEvent) {
  if (nullptr == hEvent) {
    ZESIMERR << "hEvent is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  return ZeEvent::reset(static_cast<ZeEvent *>(hEvent));
}

//-----------------------------------------------------------------------------
ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListAppendMemoryCopyRegion(
    ze_command_list_handle_t hCommandList, void *dstptr,
    const ze_copy_region_t *dstRegion, uint32_t dstPitch,
    uint32_t dstSlicePitch, const void *srcptr,
    const ze_copy_region_t *srcRegion, uint32_t srcPitch,
    uint32_t srcSlicePitch, ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents) {
  (void)dstPitch;
  (void)srcPitch;

  if (nullptr == hCommandList) {
    ZESIMERR << "hCommandList is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }
  if (nullptr == dstptr) {
    ZESIMERR << "dstptr is null";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  if (nullptr == dstRegion) {
    ZESIMERR << "dstRegion is null";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  if (nullptr == srcptr) {
    ZESIMERR << "srcpointer is null";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  if (nullptr == srcRegion) {
    ZESIMERR << "srcRegion is null";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  if (dstSlicePitch != 0 || srcSlicePitch != 0 || dstRegion->height != 0 ||
      dstRegion->depth != 0 || dstRegion->originY != 0 ||
      dstRegion->originZ != 0 || srcRegion->height != 0 ||
      srcRegion->depth != 0 || srcRegion->originY != 0 ||
      srcRegion->originZ != 0) {
    ZESIMERR << "2D/3D copy region is not supported";
    return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
  }

  if (dstRegion->width != srcRegion->width ||
      dstRegion->height != srcRegion->height ||
      dstRegion->depth != srcRegion->depth) {
    ZESIMERR << "The region width, height, and depth for both src and dst"
                "must be same";
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }

  if (numWaitEvents > 0 && nullptr == phWaitEvents) {
    ZESIMERR << "phWaitEvents is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }
  std::vector<ZeEvent *> wait_events;
  for (uint32_t i = 0; i < numWaitEvents; ++i) {
    wait_events.push_back(static_cast<ZeEvent *>(phWaitEvents[i]));
  }

  auto cmdlist = static_cast<ZeCmdList *>(hCommandList);
  auto srcbuf = cmdlist->device()->driver()->getZeBuffer(srcptr);
  auto dstbuf = cmdlist->device()->driver()->getZeBuffer(dstptr);

  // check if src is within bounds, if owned by runtime
  if (srcbuf != nullptr &&
      (uint8_t *)srcptr + srcRegion->originX + srcRegion->width >
          srcbuf->ptr_end()) {
    ZESIMERR << "src is not within bounds";
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }

  // check if dst is wihtin bounds, if owned by runtime
  if (dstbuf != nullptr &&
      (uint8_t *)dstptr + dstRegion->originX + dstRegion->width >
          dstbuf->ptr_end()) {
    ZESIMERR << "dst is not within bounds";
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }

  void *dstCopyPtr = (uint8_t *)dstptr + dstRegion->originX;
  void *srcCopyPtr = (uint8_t *)srcptr + srcRegion->originX;
  size_t size = (size_t)srcRegion->width;
  auto device = cmdlist->device();
  return cmdlist->append(
      static_cast<ZeEvent *>(hSignalEvent), wait_events,
      [device, srcbuf, dstbuf, dstCopyPtr, srcCopyPtr, size]() //
      {
        return ZeBuffer::copy(device, srcbuf, dstbuf, dstCopyPtr, srcCopyPtr,
                              size);
      });
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeDeviceGetCommandQueueGroupProperties(
    ze_device_handle_t hDevice, ///< [in] handle of the device
    uint32_t
        *pCount, ///< [in,out] pointer to the number of command queue group
                 ///< properties. if count is zero, then the driver will
                 ///< update the value with the total number of command queue
                 ///< group properties available. if count is non-zero, then
                 ///< driver will only retrieve that number of command queue
                 ///< group properties. if count is larger than the number of
                 ///< command queue group properties available, then the
                 ///< driver will update the value with the correct number of
                 ///< command queue group properties available.
    ze_command_queue_group_properties_t
        *pCommandQueueGroupProperties ///< [in,out][optional][range(0,
                                      ///< *pCount)] array of query results
                                      ///< for command queue group properties
) {
  if (nullptr == hDevice) {
    ZESIMERR << "hDevice is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (nullptr == pCount) {
    ZESIMERR << "pCount is nullptr";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  uint32_t numQueueGroups = 2;
  static std::vector<ze_command_queue_group_properties_t> queueProperties(
      numQueueGroups);

  queueProperties[0].stype = ZE_STRUCTURE_TYPE_COMMAND_QUEUE_GROUP_PROPERTIES;
  queueProperties[0].numQueues = 2;
  queueProperties[0].flags = ZE_COMMAND_QUEUE_GROUP_PROPERTY_FLAG_COMPUTE;
  queueProperties[0].pNext = nullptr;

  queueProperties[1].stype = ZE_STRUCTURE_TYPE_COMMAND_QUEUE_GROUP_PROPERTIES;
  queueProperties[1].numQueues = 1;
  queueProperties[1].flags = ZE_COMMAND_QUEUE_GROUP_PROPERTY_FLAG_COPY;
  queueProperties[1].pNext = nullptr;

  if (0 == *pCount) {
    *pCount = numQueueGroups;
    return ZE_RESULT_SUCCESS;
  }

  if (*pCount > numQueueGroups) {
    *pCount = numQueueGroups;
  }

  for (uint32_t i = 0; i != numQueueGroups; i++) {
    pCommandQueueGroupProperties[i] = queueProperties[i];
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeKernelSuggestGroupSize(
    ze_kernel_handle_t hKernel, uint32_t globalSizeX, uint32_t globalSizeY,
    uint32_t globalSizeZ, uint32_t *groupSizeX, uint32_t *groupSizeY,
    uint32_t *groupSizeZ) {
  if (nullptr == hKernel) {
    ZESIMERR << "hKernel is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (nullptr == groupSizeX) {
    ZESIMERR << "groupSizeX is null";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (nullptr == groupSizeY) {
    ZESIMERR << "groupSizeY is null";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (nullptr == groupSizeZ) {
    ZESIMERR << "groupSizeZ is null";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  return static_cast<ZeKernel *>(hKernel)->suggestGroupSize(
      globalSizeX, globalSizeY, globalSizeZ, groupSizeX, groupSizeY,
      groupSizeZ);
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeEventQueryKernelTimestamp(
    ze_event_handle_t hEvent, ze_kernel_timestamp_result_t *dstptr) {
  if (nullptr == hEvent) {
    ZESIMERR << "hEvent is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (nullptr == dstptr) {
    ZESIMERR << "dstprt is null";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  return static_cast<ZeEvent *>(hEvent)->queryKernelTimestamp(dstptr);
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeMemGetAllocProperties(ze_context_handle_t hContext, const void *ptr,
                        ze_memory_allocation_properties_t *pMemAllocProperties,
                        ze_device_handle_t *phDevice) {
  if (nullptr == hContext) {
    ZESIMERR << "hContext is null";
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (nullptr == ptr) {
    ZESIMERR << "ptr(memory pointer to query) is null";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (nullptr == pMemAllocProperties) {
    ZESIMERR << "query result for memory allocation properties is null";
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  return static_cast<ZeContext *>(hContext)->getMemAllocProperties(
      ptr, pMemAllocProperties, phDevice);
}
} // extern "C"
} // namespace __zert__

#if INTEL_CUSTOMIZATION
//===--- Target RTLs Implementation ---------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Code for tracing RTL
//
//===----------------------------------------------------------------------===//
#ifndef RTL_TRACE_H
#define RTL_TRACE_H

#include <string>
#include "omptarget.h"

#define STR(x) #x
#define TO_STRING(x) STR(x)

extern int DebugLevel;

#define DPLEVEL(Level, ...)                                                    \
  do {                                                                         \
    if (DebugLevel > Level) {                                                  \
      fprintf(stderr, "%s --> ", "Target " TO_STRING(TARGET_NAME) " RTL");     \
      fprintf(stderr, __VA_ARGS__);                                            \
    }                                                                          \
  } while (0)

#define DP(...) DPLEVEL(0, __VA_ARGS__)
#define DP1(...) DPLEVEL(1, __VA_ARGS__)

#if INTEL_INTERNAL_BUILD
#define DPI(...) DP(__VA_ARGS__)
#else  // !INTEL_INTERNAL_BUILD
#define DPI(...)
#endif // !INTEL_INTERNAL_BUILD

#define FATAL_ERROR(Msg)                                                       \
  do {                                                                         \
    DPLEVEL(-1, "Error: %s failed (%s) -- exiting...\n", __func__, Msg);       \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

#define WARNING(...) DPLEVEL(-1, "Warning: " __VA_ARGS__)

///
/// Wrappers for tracing ze API calls.
///

#define TRACE_FN(Name) L0TR##Name
#define TRACE_FN_DEF(Name) ze_result_t TRACE_FN(Name)

#define TRACE_FN_ARG_BEGIN()                                                   \
  do {                                                                         \
    std::string fn(__func__);                                                  \
    DP1("ZE_CALLEE: %s (\n", fn.substr(4).c_str());                            \
  } while (0)

#define TRACE_FN_ARG_END() DP1(")\n")
#define TRACE_FN_ARG(Arg, Fmt) DP1("    %s = " Fmt "\n", TO_STRING(Arg), Arg)
#define TRACE_FN_ARG_PTR(Arg)                                                  \
  DP1("    %s = " DPxMOD "\n", TO_STRING(Arg), DPxPTR(Arg))

TRACE_FN_DEF(zeCommandListAppendBarrier)(
    ze_command_list_handle_t hCommandList,
    ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents,
    ze_event_handle_t *phWaitEvents) {
  auto rc = zeCommandListAppendBarrier(hCommandList, hSignalEvent,
                                       numWaitEvents, phWaitEvents);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hCommandList);
  TRACE_FN_ARG_PTR(hSignalEvent);
  TRACE_FN_ARG(numWaitEvents, "%" PRIu32);
  TRACE_FN_ARG_PTR(phWaitEvents);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeCommandListAppendLaunchKernel)(
    ze_command_list_handle_t hCommandList,
    ze_kernel_handle_t hKernel,
    const ze_group_count_t *pLaunchFuncArgs,
    ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents,
    ze_event_handle_t *phWaitEvents) {
  auto rc = zeCommandListAppendLaunchKernel(hCommandList, hKernel,
      pLaunchFuncArgs, hSignalEvent, numWaitEvents, phWaitEvents);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hCommandList);
  TRACE_FN_ARG_PTR(hKernel);
  TRACE_FN_ARG_PTR(pLaunchFuncArgs);
  TRACE_FN_ARG_PTR(hSignalEvent);
  TRACE_FN_ARG(numWaitEvents, "%" PRIu32);
  TRACE_FN_ARG_PTR(phWaitEvents);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeCommandListAppendMemoryCopy)(
    ze_command_list_handle_t hCommandList,
    void *dstptr,
    const void *srcptr,
    size_t size,
    ze_event_handle_t hEvent) {
  auto rc = zeCommandListAppendMemoryCopy(hCommandList, dstptr, srcptr, size,
                                          hEvent);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hCommandList);
  TRACE_FN_ARG_PTR(dstptr);
  TRACE_FN_ARG_PTR(srcptr);
  TRACE_FN_ARG(size, "%zu");
  TRACE_FN_ARG_PTR(hEvent);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeCommandListClose)(
    ze_command_list_handle_t hCommandList) {
  auto rc = zeCommandListClose(hCommandList);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hCommandList);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeCommandListCreate)(
    ze_device_handle_t hDevice,
    const ze_command_list_desc_t *desc,
    ze_command_list_handle_t *phCommandList) {
  auto rc = zeCommandListCreate(hDevice, desc, phCommandList);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDevice);
  TRACE_FN_ARG_PTR(desc);
  TRACE_FN_ARG_PTR(phCommandList);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeCommandListDestroy)(
    ze_command_list_handle_t hCommandList) {
  auto rc = zeCommandListDestroy(hCommandList);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hCommandList);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeCommandListReset)(
    ze_command_list_handle_t hCommandList) {
  auto rc = zeCommandListReset(hCommandList);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hCommandList);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeCommandQueueCreate)(
    ze_device_handle_t hDevice,
    const ze_command_queue_desc_t *desc,
    ze_command_queue_handle_t *phCommandQueue) {
  auto rc = zeCommandQueueCreate(hDevice, desc, phCommandQueue);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDevice);
  TRACE_FN_ARG_PTR(desc);
  TRACE_FN_ARG_PTR(phCommandQueue);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeCommandQueueDestroy)(
    ze_command_queue_handle_t hCommandQueue) {
  auto rc = zeCommandQueueDestroy(hCommandQueue);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hCommandQueue);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeCommandQueueExecuteCommandLists)(
    ze_command_queue_handle_t hCommandQueue,
    uint32_t numCommandLists,
    ze_command_list_handle_t *phCommandLists,
    ze_fence_handle_t hFence) {
  auto rc = zeCommandQueueExecuteCommandLists(hCommandQueue, numCommandLists,
                                              phCommandLists, hFence);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hCommandQueue);
  TRACE_FN_ARG(numCommandLists, "%" PRIu32);
  TRACE_FN_ARG_PTR(phCommandLists);
  TRACE_FN_ARG_PTR(hFence);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeCommandQueueSynchronize)(
    ze_command_queue_handle_t hCommandQueue,
    uint32_t timeout) {
  auto rc = zeCommandQueueSynchronize(hCommandQueue, timeout);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hCommandQueue);
  TRACE_FN_ARG(timeout, "%" PRIu32);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeDeviceGet)(
    ze_driver_handle_t hDriver,
    uint32_t *pCount,
    ze_device_handle_t *phDevices) {
  auto rc = zeDeviceGet(hDriver, pCount, phDevices);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDriver);
  TRACE_FN_ARG_PTR(pCount);
  TRACE_FN_ARG_PTR(phDevices);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeDeviceGetProperties)(
    ze_device_handle_t hDevice,
    ze_device_properties_t *pDeviceProperties) {
  auto rc = zeDeviceGetProperties(hDevice, pDeviceProperties);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDevice);
  TRACE_FN_ARG_PTR(pDeviceProperties);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeDeviceGetComputeProperties)(
    ze_device_handle_t hDevice,
    ze_device_compute_properties_t *pComputeProperties) {
  auto rc = zeDeviceGetComputeProperties(hDevice, pComputeProperties);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDevice);
  TRACE_FN_ARG_PTR(pComputeProperties);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeDriverAllocDeviceMem)(
    ze_driver_handle_t hDriver,
    const ze_device_mem_alloc_desc_t *device_desc,
    size_t size,
    size_t alignment,
    ze_device_handle_t hDevice,
    void **pptr) {
  auto rc = zeDriverAllocDeviceMem(hDriver, device_desc, size, alignment,
                                   hDevice, pptr);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDriver);
  TRACE_FN_ARG_PTR(device_desc);
  TRACE_FN_ARG(size, "%zu");
  TRACE_FN_ARG(alignment, "%zu");
  TRACE_FN_ARG_PTR(hDevice);
  TRACE_FN_ARG_PTR(pptr);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeDriverAllocHostMem)(
    ze_driver_handle_t hDriver,
    const ze_host_mem_alloc_desc_t *host_desc,
    size_t size,
    size_t alignment,
    void **pptr) {
  auto rc = zeDriverAllocHostMem(hDriver, host_desc, size, alignment, pptr);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDriver);
  TRACE_FN_ARG_PTR(host_desc);
  TRACE_FN_ARG(size, "%zu");
  TRACE_FN_ARG(alignment, "%zu");
  TRACE_FN_ARG_PTR(pptr);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeDriverAllocSharedMem)(
    ze_driver_handle_t hDriver,
    const ze_device_mem_alloc_desc_t *device_desc,
    const ze_host_mem_alloc_desc_t *host_desc,
    size_t size,
    size_t alignment,
    ze_device_handle_t hDevice,
    void **pptr) {
  auto rc = zeDriverAllocSharedMem(hDriver, device_desc, host_desc, size,
                                   alignment, hDevice, pptr);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDriver);
  TRACE_FN_ARG_PTR(device_desc);
  TRACE_FN_ARG_PTR(host_desc);
  TRACE_FN_ARG(size, "%zu");
  TRACE_FN_ARG(alignment, "%zu");
  TRACE_FN_ARG_PTR(hDevice);
  TRACE_FN_ARG_PTR(pptr);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeDriverFreeMem)(
    ze_driver_handle_t hDriver,
    void *ptr) {
  auto rc = zeDriverFreeMem(hDriver, ptr);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDriver);
  TRACE_FN_ARG_PTR(ptr);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeDriverGet)(
    uint32_t *pCount,
    ze_driver_handle_t *phDrivers) {
  auto rc = zeDriverGet(pCount, phDrivers);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(pCount);
  TRACE_FN_ARG_PTR(phDrivers);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeDriverGetMemAddressRange)(
    ze_driver_handle_t hDriver,
    const void *ptr,
    void **pBase,
    size_t *pSize) {
  auto rc = zeDriverGetMemAddressRange(hDriver, ptr, pBase, pSize);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDriver);
  TRACE_FN_ARG_PTR(ptr);
  TRACE_FN_ARG_PTR(pBase);
  TRACE_FN_ARG_PTR(pSize);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeDriverGetMemAllocProperties)(
    ze_driver_handle_t hDriver,
    const void *ptr,
    ze_memory_allocation_properties_t *pMemAllocProperties,
    ze_device_handle_t *phDevice) {
  auto rc = zeDriverGetMemAllocProperties(hDriver, ptr, pMemAllocProperties,
                                          phDevice);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDriver);
  TRACE_FN_ARG_PTR(ptr);
  TRACE_FN_ARG_PTR(pMemAllocProperties);
  TRACE_FN_ARG_PTR(phDevice);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeFenceCreate)(
    ze_command_queue_handle_t hCommandQueue,
    const ze_fence_desc_t *desc,
    ze_fence_handle_t *phFence) {
  auto rc = zeFenceCreate(hCommandQueue, desc, phFence);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hCommandQueue);
  TRACE_FN_ARG_PTR(desc);
  TRACE_FN_ARG_PTR(phFence);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeFenceDestroy)(
    ze_fence_handle_t hFence) {
  auto rc = zeFenceDestroy(hFence);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hFence);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeFenceHostSynchronize)(
    ze_fence_handle_t hFence,
    uint32_t timeout) {
  auto rc = zeFenceHostSynchronize(hFence, timeout);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hFence);
  TRACE_FN_ARG(timeout, "%" PRIu32);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeInit)(
    ze_init_flag_t flags) {
  auto rc = zeInit(flags);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG(flags, "%" PRId32);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeKernelCreate)(
    ze_module_handle_t hModule,
    const ze_kernel_desc_t *desc,
    ze_kernel_handle_t *phKernel) {
  auto rc = zeKernelCreate(hModule, desc, phKernel);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hModule);
  TRACE_FN_ARG_PTR(desc);
  TRACE_FN_ARG_PTR(phKernel);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeKernelDestroy)(
    ze_kernel_handle_t hKernel) {
  auto rc = zeKernelDestroy(hKernel);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hKernel);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeKernelGetProperties)(
    ze_kernel_handle_t hKernel,
    ze_kernel_properties_t *pKernelProperties) {
  auto rc = zeKernelGetProperties(hKernel, pKernelProperties);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hKernel);
  TRACE_FN_ARG_PTR(pKernelProperties);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeKernelSetArgumentValue)(
    ze_kernel_handle_t hKernel,
    uint32_t argIndex,
    size_t argSize,
    const void *pArgValue) {
  auto rc = zeKernelSetArgumentValue(hKernel, argIndex, argSize, pArgValue);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hKernel);
  TRACE_FN_ARG(argIndex, "%" PRIu32);
  TRACE_FN_ARG(argSize, "%zu");
  TRACE_FN_ARG_PTR(pArgValue);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeKernelSetAttribute)(
    ze_kernel_handle_t hKernel,
    ze_kernel_attribute_t attr,
    uint32_t size,
    const void *pValue) {
  auto rc = zeKernelSetAttribute(hKernel, attr, size, pValue);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hKernel);
  TRACE_FN_ARG(attr, "%" PRId32);
  TRACE_FN_ARG(size, "%" PRIu32);
  TRACE_FN_ARG_PTR(pValue);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeKernelSetGroupSize)(
    ze_kernel_handle_t hKernel,
    uint32_t groupSizeX,
    uint32_t groupSizeY,
    uint32_t groupSizeZ) {
  auto rc = zeKernelSetGroupSize(hKernel, groupSizeX, groupSizeY, groupSizeZ);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hKernel);
  TRACE_FN_ARG(groupSizeX, "%" PRIu32);
  TRACE_FN_ARG(groupSizeY, "%" PRIu32);
  TRACE_FN_ARG(groupSizeZ, "%" PRIu32);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeModuleBuildLogDestroy)(
    ze_module_build_log_handle_t hModuleBuildLog) {
  auto rc = zeModuleBuildLogDestroy(hModuleBuildLog);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hModuleBuildLog);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeModuleBuildLogGetString)(
    ze_module_build_log_handle_t hModuleBuildLog,
    size_t *pSize,
    char *pBuildLog) {
  auto rc = zeModuleBuildLogGetString(hModuleBuildLog, pSize, pBuildLog);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hModuleBuildLog);
  TRACE_FN_ARG_PTR(pSize);
  TRACE_FN_ARG_PTR(pBuildLog);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeModuleCreate)(
    ze_device_handle_t hDevice,
    const ze_module_desc_t *desc,
    ze_module_handle_t *phModule,
    ze_module_build_log_handle_t *phBuildLog) {
  auto rc = zeModuleCreate(hDevice, desc, phModule, phBuildLog);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDevice);
  TRACE_FN_ARG_PTR(desc);
  TRACE_FN_ARG_PTR(phModule);
  TRACE_FN_ARG_PTR(phBuildLog);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeModuleDestroy)(
    ze_module_handle_t hModule) {
  auto rc = zeModuleDestroy(hModule);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hModule);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeModuleGetGlobalPointer)(
    ze_module_handle_t hModule,
    const char *pGlobalName,
    void **pptr) {
  auto rc = zeModuleGetGlobalPointer(hModule, pGlobalName, pptr);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hModule);
  TRACE_FN_ARG_PTR(pGlobalName);
  TRACE_FN_ARG_PTR(pptr);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeKernelSuggestGroupSize)(
    ze_kernel_handle_t hKernel,
    uint32_t globalSizeX, uint32_t globalSizeY, uint32_t globalSizeZ,
    uint32_t *groupSizeX, uint32_t *groupSizeY, uint32_t *groupSizeZ) {
  auto rc = zeKernelSuggestGroupSize(hKernel,
                                     globalSizeX, globalSizeY, globalSizeZ,
                                     groupSizeX, groupSizeY, groupSizeZ);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hKernel);
  TRACE_FN_ARG(globalSizeX, "%" PRIu32);
  TRACE_FN_ARG(globalSizeY, "%" PRIu32);
  TRACE_FN_ARG(globalSizeZ, "%" PRIu32);
  TRACE_FN_ARG_PTR(groupSizeX);
  TRACE_FN_ARG_PTR(groupSizeY);
  TRACE_FN_ARG_PTR(groupSizeZ);
  TRACE_FN_ARG_END();
  return rc;
}

#define CALL_ZE(Rc, Fn, ...)                                                   \
  do {                                                                         \
    if (DebugLevel > 1) {                                                      \
      DP1("ZE_CALLER: %s %s\n", TO_STRING(Fn), TO_STRING(( __VA_ARGS__ )));    \
      Rc = TRACE_FN(Fn)(__VA_ARGS__);                                          \
    } else {                                                                   \
      Rc = Fn(__VA_ARGS__);                                                    \
    }                                                                          \
  } while (0)

#define CALL_ZE_RC(Rc, Fn, ...)                                                \
  do {                                                                         \
    CALL_ZE(Rc, Fn, __VA_ARGS__);                                              \
    if (Rc != ZE_RESULT_SUCCESS) {                                             \
      DP("Error: %s:%s failed with error code %d, %s\n", __func__, #Fn, rc,    \
         getZeErrorName(rc));                                                  \
    }                                                                          \
  } while(0)

/// For non-thread-safe functions
#define CALL_ZE_RET_MTX(Ret, Fn, Mtx, ...)                                     \
  do {                                                                         \
    Mtx.lock();                                                                \
    ze_result_t rc;                                                            \
    CALL_ZE(rc, Fn, __VA_ARGS__);                                              \
    Mtx.unlock();                                                              \
    if (rc != ZE_RESULT_SUCCESS) {                                             \
      DP("Error: %s:%s failed with error code %d, %s\n", __func__, #Fn, rc,    \
         getZeErrorName(rc));                                                  \
      return Ret;                                                              \
    }                                                                          \
  } while (0)

#define CALL_ZE_RET_FAIL_MTX(Fn, Mtx, ...)                                     \
  CALL_ZE_RET_MTX(OFFLOAD_FAIL, Fn, Mtx, __VA_ARGS__)
#define CALL_ZE_RET_NULL_MTX(Fn, Mtx, ...)                                     \
  CALL_ZE_RET_MTX(NULL, Fn, Mtx, __VA_ARGS__)
#define CALL_ZE_RET_ZERO_MTX(Fn, Mtx, ...)                                     \
  CALL_ZE_RET_MTX(0, Fn, Mtx, __VA_ARGS__)

/// For thread-safe functions
#define CALL_ZE_RET(Ret, Fn, ...)                                              \
  do {                                                                         \
    ze_result_t rc;                                                            \
    CALL_ZE(rc, Fn, __VA_ARGS__);                                              \
    if (rc != ZE_RESULT_SUCCESS) {                                             \
      DP("Error: %s:%s failed with error code %d, %s\n", __func__, #Fn, rc,    \
         getZeErrorName(rc));                                                  \
      return Ret;                                                              \
    }                                                                          \
  } while (0)

#define CALL_ZE_RET_FAIL(Fn, ...) CALL_ZE_RET(OFFLOAD_FAIL, Fn, __VA_ARGS__)
#define CALL_ZE_RET_NULL(Fn, ...) CALL_ZE_RET(NULL, Fn, __VA_ARGS__)
#define CALL_ZE_RET_ZERO(Fn, ...) CALL_ZE_RET(0, Fn, __VA_ARGS__)

#define CALL_ZE_EXIT_FAIL(Fn, ...)                                             \
  do {                                                                         \
    ze_result_t rc;                                                            \
    CALL_ZE(rc, Fn, __VA_ARGS__);                                              \
    if (rc != ZE_RESULT_SUCCESS) {                                             \
      DP("Error: %s:%s failed with error code %d, %s\n", __func__, #Fn, rc,    \
         getZeErrorName(rc));                                                  \
      std::exit(EXIT_FAILURE);                                                 \
    }                                                                          \
  } while (0)

#define FOREACH_ZE_ERROR_CODE(Fn)                                              \
  Fn(ZE_RESULT_SUCCESS)                                                        \
  Fn(ZE_RESULT_NOT_READY)                                                      \
  Fn(ZE_RESULT_ERROR_DEVICE_LOST)                                              \
  Fn(ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY)                                       \
  Fn(ZE_RESULT_ERROR_OUT_OF_DEVICE_MEMORY)                                     \
  Fn(ZE_RESULT_ERROR_MODULE_BUILD_FAILURE)                                     \
  Fn(ZE_RESULT_ERROR_INSUFFICIENT_PERMISSIONS)                                 \
  Fn(ZE_RESULT_ERROR_NOT_AVAILABLE)                                            \
  Fn(ZE_RESULT_ERROR_UNINITIALIZED)                                            \
  Fn(ZE_RESULT_ERROR_UNSUPPORTED_VERSION)                                      \
  Fn(ZE_RESULT_ERROR_UNSUPPORTED_FEATURE)                                      \
  Fn(ZE_RESULT_ERROR_INVALID_ARGUMENT)                                         \
  Fn(ZE_RESULT_ERROR_INVALID_NULL_HANDLE)                                      \
  Fn(ZE_RESULT_ERROR_HANDLE_OBJECT_IN_USE)                                     \
  Fn(ZE_RESULT_ERROR_INVALID_NULL_POINTER)                                     \
  Fn(ZE_RESULT_ERROR_INVALID_SIZE)                                             \
  Fn(ZE_RESULT_ERROR_UNSUPPORTED_SIZE)                                         \
  Fn(ZE_RESULT_ERROR_UNSUPPORTED_ALIGNMENT)                                    \
  Fn(ZE_RESULT_ERROR_INVALID_SYNCHRONIZATION_OBJECT)                           \
  Fn(ZE_RESULT_ERROR_INVALID_ENUMERATION)                                      \
  Fn(ZE_RESULT_ERROR_UNSUPPORTED_ENUMERATION)                                  \
  Fn(ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT)                                 \
  Fn(ZE_RESULT_ERROR_INVALID_NATIVE_BINARY)                                    \
  Fn(ZE_RESULT_ERROR_INVALID_GLOBAL_NAME)                                      \
  Fn(ZE_RESULT_ERROR_INVALID_KERNEL_NAME)                                      \
  Fn(ZE_RESULT_ERROR_INVALID_FUNCTION_NAME)                                    \
  Fn(ZE_RESULT_ERROR_INVALID_GROUP_SIZE_DIMENSION)                             \
  Fn(ZE_RESULT_ERROR_INVALID_GLOBAL_WIDTH_DIMENSION)                           \
  Fn(ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX)                            \
  Fn(ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE)                             \
  Fn(ZE_RESULT_ERROR_INVALID_KERNEL_ATTRIBUTE_VALUE)                           \
  Fn(ZE_RESULT_ERROR_INVALID_COMMAND_LIST_TYPE)                                \
  Fn(ZE_RESULT_ERROR_OVERLAPPING_REGIONS)                                      \
  Fn(ZE_RESULT_ERROR_UNKNOWN)

#define CASE_TO_STRING(Num) case Num: return #Num;
static const char *getZeErrorName(int32_t Error) {
  switch (Error) {
    FOREACH_ZE_ERROR_CODE(CASE_TO_STRING)
  default:
    return "ZE_RESULT_ERROR_UNKNOWN";
  }
}

#endif // !defined(RTL_TRACE_H)
#endif // INTEL_CUSTOMIZATION

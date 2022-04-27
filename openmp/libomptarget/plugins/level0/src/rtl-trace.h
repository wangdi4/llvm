/* INTEL_CUSTOMIZATION */
/*
 * INTEL CONFIDENTIAL
 *
 * Modifications, Copyright (C) 2021 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may not
 * use, modify, copy, publish, distribute, disclose or transmit this software or
 * the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 */
/* end INTEL_CUSTOMIZATION */
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
#include "Debug.h"

#define STR(x) #x
#define TO_STRING(x) STR(x)

#define TARGET_NAME LEVEL0
#define DEBUG_PREFIX "Target " GETNAME(TARGET_NAME) " RTL"

extern int DebugLevel;

#define DPCALL(...)                                                            \
  do {                                                                         \
    if (DebugLevel > 1)                                                        \
      DP(__VA_ARGS__);                                                         \
  } while (0)

#if INTEL_INTERNAL_BUILD
#define DPI(...) DP(__VA_ARGS__)
#else  // !INTEL_INTERNAL_BUILD
#define DPI(...)
#endif // !INTEL_INTERNAL_BUILD

#define FATAL_ERROR(Msg)                                                       \
  do {                                                                         \
    fprintf(stderr, "%s --> ", DEBUG_PREFIX);                                  \
    fprintf(stderr, "Error: %s failed (%s) -- exiting...\n", __func__, Msg);   \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

#define WARNING(...)                                                           \
  do {                                                                         \
    fprintf(stderr, "%s --> ", DEBUG_PREFIX);                                  \
    fprintf(stderr, "Warning: " __VA_ARGS__);                                  \
  } while (0)

///
/// Wrappers for tracing ze API calls.
///

#define TRACE_FN(Name) L0TR##Name
#define TRACE_FN_DEF(Name) ze_result_t TRACE_FN(Name)

#define TRACE_FN_ARG_BEGIN()                                                   \
  do {                                                                         \
    std::string fn(__func__);                                                  \
    DPCALL("ZE_CALLEE: %s (\n", fn.substr(4).c_str());                         \
  } while (0)

#define TRACE_FN_ARG_END() DPCALL(")\n")
#define TRACE_FN_ARG(Arg, Fmt) DPCALL("    %s = " Fmt "\n", TO_STRING(Arg), Arg)
#define TRACE_FN_ARG_PTR(Arg)                                                  \
  DPCALL("    %s = " DPxMOD "\n", TO_STRING(Arg), DPxPTR(Arg))
#define TRACE_FN_ARG_UINT(Arg) TRACE_FN_ARG(Arg, "%" PRIu32)
#define TRACE_FN_ARG_UINT64(Arg) TRACE_FN_ARG(Arg, "%" PRIu64)
#define TRACE_FN_ARG_SIZE(Arg) TRACE_FN_ARG(Arg, "%zu")

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
  TRACE_FN_ARG_UINT(numWaitEvents);
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
  TRACE_FN_ARG_UINT(numWaitEvents);
  TRACE_FN_ARG_PTR(phWaitEvents);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeCommandListAppendMemoryCopy)(
    ze_command_list_handle_t hCommandList,
    void *dstptr,
    const void *srcptr,
    size_t size,
    ze_event_handle_t hEvent,
    uint32_t numWaitEvents,
    ze_event_handle_t *phWaitEvents) {
  auto rc = zeCommandListAppendMemoryCopy(hCommandList, dstptr, srcptr, size,
                                          hEvent, numWaitEvents, phWaitEvents);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hCommandList);
  TRACE_FN_ARG_PTR(dstptr);
  TRACE_FN_ARG_PTR(srcptr);
  TRACE_FN_ARG_SIZE(size);
  TRACE_FN_ARG_PTR(hEvent);
  TRACE_FN_ARG_UINT(numWaitEvents);
  TRACE_FN_ARG_PTR(phWaitEvents);
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
    ze_context_handle_t hContext,
    ze_device_handle_t hDevice,
    const ze_command_list_desc_t *desc,
    ze_command_list_handle_t *phCommandList) {
  auto rc = zeCommandListCreate(hContext, hDevice, desc, phCommandList);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hContext);
  TRACE_FN_ARG_PTR(hDevice);
  TRACE_FN_ARG_PTR(desc);
  TRACE_FN_ARG_PTR(phCommandList);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeCommandListCreateImmediate)(
    ze_context_handle_t hContext,
    ze_device_handle_t hDevice,
    const ze_command_queue_desc_t *altdesc,
    ze_command_list_handle_t *phCommandList) {
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hContext);
  TRACE_FN_ARG_PTR(hDevice);
  TRACE_FN_ARG_PTR(altdesc);
  TRACE_FN_ARG_PTR(phCommandList);
  TRACE_FN_ARG_END();
  return zeCommandListCreateImmediate(hContext, hDevice, altdesc,
                                      phCommandList);
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
    ze_context_handle_t hContext,
    ze_device_handle_t hDevice,
    const ze_command_queue_desc_t *desc,
    ze_command_queue_handle_t *phCommandQueue) {
  auto rc = zeCommandQueueCreate(hContext, hDevice, desc, phCommandQueue);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hContext);
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
  TRACE_FN_ARG_UINT(numCommandLists);
  TRACE_FN_ARG_PTR(phCommandLists);
  TRACE_FN_ARG_PTR(hFence);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeCommandQueueSynchronize)(
    ze_command_queue_handle_t hCommandQueue,
    uint64_t timeout) {
  auto rc = zeCommandQueueSynchronize(hCommandQueue, timeout);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hCommandQueue);
  TRACE_FN_ARG_UINT64(timeout);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeContextCreate)(
    ze_driver_handle_t hDriver,
    const ze_context_desc_t *desc,
    ze_context_handle_t *phContext) {
  auto rc = zeContextCreate(hDriver, desc, phContext);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDriver);
  TRACE_FN_ARG_PTR(desc);
  TRACE_FN_ARG_PTR(phContext);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeContextDestroy)(
    ze_context_handle_t hContext) {
  auto rc = zeContextDestroy(hContext);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hContext);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeDeviceCanAccessPeer)(
    ze_device_handle_t hDevice,
    ze_device_handle_t hPeerDevice,
    ze_bool_t *value) {
  auto rc = zeDeviceCanAccessPeer(hDevice, hPeerDevice, value);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDevice);
  TRACE_FN_ARG_PTR(hPeerDevice);
  TRACE_FN_ARG_PTR(value);
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

TRACE_FN_DEF(zeDeviceGetCommandQueueGroupProperties)(
    ze_device_handle_t hDevice,
    uint32_t *pCount,
    ze_command_queue_group_properties_t *pCommandQueueGroupProperties) {
  auto rc = zeDeviceGetCommandQueueGroupProperties(
      hDevice, pCount, pCommandQueueGroupProperties);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDevice);
  TRACE_FN_ARG_PTR(pCount);
  TRACE_FN_ARG_PTR(pCommandQueueGroupProperties);
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

TRACE_FN_DEF(zeDeviceGetSubDevices)(
    ze_device_handle_t hDevice,
    uint32_t *pCount,
    ze_device_handle_t *phSubdevices) {
  auto rc = zeDeviceGetSubDevices(hDevice, pCount, phSubdevices);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDevice);
  TRACE_FN_ARG_PTR(pCount);
  TRACE_FN_ARG_PTR(phSubdevices);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeDeviceGetMemoryProperties)(
    ze_device_handle_t hDevice,
    uint32_t *pCount,
    ze_device_memory_properties_t *pMemProperties) {
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDevice);
  TRACE_FN_ARG_PTR(pCount);
  TRACE_FN_ARG_PTR(pMemProperties);
  TRACE_FN_ARG_END();
  return zeDeviceGetMemoryProperties(hDevice, pCount, pMemProperties);
}

TRACE_FN_DEF(zeDeviceGetCacheProperties)(
    ze_device_handle_t hDevice,
    uint32_t *pCount,
    ze_device_cache_properties_t *pCacheProperties) {
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDevice);
  TRACE_FN_ARG_PTR(pCount);
  TRACE_FN_ARG_PTR(pCacheProperties);
  TRACE_FN_ARG_END();
  return zeDeviceGetCacheProperties(hDevice, pCount, pCacheProperties);
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

TRACE_FN_DEF(zeDriverGetApiVersion)(
    ze_driver_handle_t hDriver,
    ze_api_version_t *version) {
  auto rc = zeDriverGetApiVersion(hDriver, version);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDriver);
  TRACE_FN_ARG_PTR(version);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeDriverGetExtensionFunctionAddress)(
    ze_driver_handle_t hDriver,
    const char *name,
    void **ppFunctionAddress) {
  auto rc = zeDriverGetExtensionFunctionAddress(
      hDriver, name, ppFunctionAddress);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDriver);
  TRACE_FN_ARG_PTR(name);
  TRACE_FN_ARG_PTR(ppFunctionAddress);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeDriverGetExtensionProperties)(
    ze_driver_handle_t hDriver,
    uint32_t *pCount,
    ze_driver_extension_properties_t *pExtensionProperties) {
  auto rc = zeDriverGetExtensionProperties(hDriver, pCount,
                                           pExtensionProperties);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hDriver);
  TRACE_FN_ARG_PTR(pCount);
  TRACE_FN_ARG_PTR(pExtensionProperties);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeEventCreate)(
    ze_event_pool_handle_t hEventPool,
    const ze_event_desc_t *desc,
    ze_event_handle_t *phEvent) {
  auto rc = zeEventCreate(hEventPool, desc, phEvent);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hEventPool);
  TRACE_FN_ARG_PTR(desc);
  TRACE_FN_ARG_PTR(phEvent);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeEventDestroy)(
    ze_event_handle_t hEvent) {
  auto rc = zeEventDestroy(hEvent);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hEvent);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeEventHostReset)(
    ze_event_handle_t hEvent) {
  auto rc = zeEventHostReset(hEvent);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hEvent);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeEventHostSynchronize)(
    ze_event_handle_t hEvent,
    uint64_t timeout) {
  auto rc = zeEventHostSynchronize(hEvent, timeout);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hEvent);
  TRACE_FN_ARG(timeout, "%" PRIu64);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeEventPoolCreate)(
    ze_context_handle_t hContext,
    const ze_event_pool_desc_t *desc,
    uint32_t numDevices,
    ze_device_handle_t *phDevices,
    ze_event_pool_handle_t *phEventPool) {
  auto rc = zeEventPoolCreate(hContext, desc, numDevices, phDevices,
                              phEventPool);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hContext);
  TRACE_FN_ARG_PTR(desc);
  TRACE_FN_ARG_UINT(numDevices);
  TRACE_FN_ARG_PTR(phDevices);
  TRACE_FN_ARG_PTR(phEventPool);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeEventPoolDestroy)(
    ze_event_pool_handle_t hEventPool) {
  auto rc = zeEventPoolDestroy(hEventPool);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hEventPool);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeEventQueryKernelTimestamp)(
    ze_event_handle_t hEvent,
    ze_kernel_timestamp_result_t *dstptr) {
  auto rc = zeEventQueryKernelTimestamp(hEvent, dstptr);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hEvent);
  TRACE_FN_ARG_PTR(dstptr);
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
    uint64_t timeout) {
  auto rc = zeFenceHostSynchronize(hFence, timeout);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hFence);
  TRACE_FN_ARG_UINT64(timeout);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeInit)(
    ze_init_flag_t flags) {
  auto rc = zeInit(flags);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_UINT(flags);
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

TRACE_FN_DEF(zeKernelGetName)(
    ze_kernel_handle_t hKernel,
    size_t *pSize,
    char *pName) {
  auto rc = zeKernelGetName(hKernel, pSize, pName);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hKernel);
  TRACE_FN_ARG_PTR(pSize);
  TRACE_FN_ARG_PTR(pName);
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
  TRACE_FN_ARG_UINT(argIndex);
  TRACE_FN_ARG_SIZE(argSize);
  TRACE_FN_ARG_PTR(pArgValue);
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
  TRACE_FN_ARG_UINT(groupSizeX);
  TRACE_FN_ARG_UINT(groupSizeY);
  TRACE_FN_ARG_UINT(groupSizeZ);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeKernelSetIndirectAccess)(
    ze_kernel_handle_t hKernel,
    ze_kernel_indirect_access_flags_t flags) {
  auto rc = zeKernelSetIndirectAccess(hKernel, flags);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hKernel);
  TRACE_FN_ARG_UINT(flags);
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
  TRACE_FN_ARG_UINT(globalSizeX);
  TRACE_FN_ARG_UINT(globalSizeY);
  TRACE_FN_ARG_UINT(globalSizeZ);
  TRACE_FN_ARG_PTR(groupSizeX);
  TRACE_FN_ARG_PTR(groupSizeY);
  TRACE_FN_ARG_PTR(groupSizeZ);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeMemAllocDevice)(
    ze_context_handle_t hContext,
    const ze_device_mem_alloc_desc_t *device_desc,
    size_t size,
    size_t alignment,
    ze_device_handle_t hDevice,
    void **pptr) {
  auto rc = zeMemAllocDevice(hContext, device_desc, size, alignment,
                             hDevice, pptr);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hContext);
  TRACE_FN_ARG_PTR(device_desc);
  TRACE_FN_ARG_SIZE(size);
  TRACE_FN_ARG_SIZE(alignment);
  TRACE_FN_ARG_PTR(hDevice);
  TRACE_FN_ARG_PTR(pptr);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeMemAllocHost)(
    ze_context_handle_t hContext,
    const ze_host_mem_alloc_desc_t *host_desc,
    size_t size,
    size_t alignment,
    void **pptr) {
  auto rc = zeMemAllocHost(hContext, host_desc, size, alignment, pptr);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hContext);
  TRACE_FN_ARG_PTR(host_desc);
  TRACE_FN_ARG_SIZE(size);
  TRACE_FN_ARG_SIZE(alignment);
  TRACE_FN_ARG_PTR(pptr);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeMemAllocShared)(
    ze_context_handle_t hContext,
    const ze_device_mem_alloc_desc_t *device_desc,
    const ze_host_mem_alloc_desc_t *host_desc,
    size_t size,
    size_t alignment,
    ze_device_handle_t hDevice,
    void **pptr) {
  auto rc = zeMemAllocShared(hContext, device_desc, host_desc, size,
                             alignment, hDevice, pptr);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hContext);
  TRACE_FN_ARG_PTR(device_desc);
  TRACE_FN_ARG_PTR(host_desc);
  TRACE_FN_ARG_SIZE(size);
  TRACE_FN_ARG_SIZE(alignment);
  TRACE_FN_ARG_PTR(hDevice);
  TRACE_FN_ARG_PTR(pptr);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeMemFree)(
    ze_context_handle_t hContext,
    void *ptr) {
  auto rc = zeMemFree(hContext, ptr);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hContext);
  TRACE_FN_ARG_PTR(ptr);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeMemGetAddressRange)(
    ze_context_handle_t hContext,
    const void *ptr,
    void **pBase,
    size_t *pSize) {
  auto rc = zeMemGetAddressRange(hContext, ptr, pBase, pSize);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hContext);
  TRACE_FN_ARG_PTR(ptr);
  TRACE_FN_ARG_PTR(pBase);
  TRACE_FN_ARG_PTR(pSize);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeMemGetAllocProperties)(
    ze_context_handle_t hContext,
    const void *ptr,
    ze_memory_allocation_properties_t *pMemAllocProperties,
    ze_device_handle_t *phDevice) {
  auto rc = zeMemGetAllocProperties(hContext, ptr, pMemAllocProperties,
                                    phDevice);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hContext);
  TRACE_FN_ARG_PTR(ptr);
  TRACE_FN_ARG_PTR(pMemAllocProperties);
  TRACE_FN_ARG_PTR(phDevice);
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
    ze_context_handle_t hContext,
    ze_device_handle_t hDevice,
    const ze_module_desc_t *desc,
    ze_module_handle_t *phModule,
    ze_module_build_log_handle_t *phBuildLog) {
  auto rc = zeModuleCreate(hContext, hDevice, desc, phModule, phBuildLog);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hContext);
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

TRACE_FN_DEF(zeModuleDynamicLink)(
    uint32_t numModules,
    ze_module_handle_t *phModules,
    ze_module_build_log_handle_t *phLinkLog) {
  auto rc = zeModuleDynamicLink(numModules, phModules, phLinkLog);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_UINT(numModules);
  TRACE_FN_ARG_PTR(phModules);
  TRACE_FN_ARG_PTR(phLinkLog);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeModuleGetGlobalPointer)(
    ze_module_handle_t hModule,
    const char *pGlobalName,
    size_t *pSize,
    void **pptr) {
  auto rc = zeModuleGetGlobalPointer(hModule, pGlobalName, pSize, pptr);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hModule);
  TRACE_FN_ARG_PTR(pGlobalName);
  TRACE_FN_ARG_PTR(pSize);
  TRACE_FN_ARG_PTR(pptr);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeModuleGetProperties)(
    ze_module_handle_t hModule,
    ze_module_properties_t *pModuleProperties) {
  auto rc = zeModuleGetProperties(hModule, pModuleProperties);
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hModule);
  TRACE_FN_ARG_PTR(pModuleProperties);
  TRACE_FN_ARG_END();
  return rc;
}

TRACE_FN_DEF(zeModuleGetKernelNames)(
    ze_module_handle_t hModule,
    uint32_t *pCount,
    const char **pNames) {
  TRACE_FN_ARG_BEGIN();
  TRACE_FN_ARG_PTR(hModule);
  TRACE_FN_ARG_PTR(pCount);
  TRACE_FN_ARG_PTR(pNames);
  TRACE_FN_ARG_END();
  return zeModuleGetKernelNames(hModule, pCount, pNames);
}

#define CALL_ZE(Rc, Fn, ...)                                                   \
  do {                                                                         \
    if (DebugLevel > 1) {                                                      \
      DPCALL("ZE_CALLER: %s %s\n", TO_STRING(Fn), TO_STRING(( __VA_ARGS__ ))); \
      Rc = TRACE_FN(Fn)(__VA_ARGS__);                                          \
    } else {                                                                   \
      Rc = Fn(__VA_ARGS__);                                                    \
    }                                                                          \
  } while (0)

#define CALL_ZE_RC(Rc, Fn, ...)                                                \
  do {                                                                         \
    CALL_ZE(Rc, Fn, __VA_ARGS__);                                              \
    if (Rc != ZE_RESULT_SUCCESS) {                                             \
      DP("Error: %s:%s failed with error code %d, %s\n", __func__, #Fn, Rc,    \
         getZeErrorName(Rc));                                                  \
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
#define CALL_ZE_RET_VOID(Fn, ...) CALL_ZE_RET(, Fn, __VA_ARGS__)

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
  Fn(ZE_RESULT_ERROR_MODULE_LINK_FAILURE)                                      \
  Fn(ZE_RESULT_ERROR_INSUFFICIENT_PERMISSIONS)                                 \
  Fn(ZE_RESULT_ERROR_NOT_AVAILABLE)                                            \
  Fn(ZE_RESULT_ERROR_DEPENDENCY_UNAVAILABLE)                                   \
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
  Fn(ZE_RESULT_ERROR_INVALID_MODULE_UNLINKED)                                  \
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

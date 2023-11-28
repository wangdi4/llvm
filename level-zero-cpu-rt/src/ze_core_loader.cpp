/*
 * Copyright (C) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <level_zero/include/ze_api.h>
#include <level_zero/include/ze_ddi.h>
#include <level_zero/include/zet_api.h>
#include <level_zero/include/zet_ddi.h>

ZE_APIEXPORT ze_result_t ZE_APICALL zeGetDriverProcAddrTable(
    ze_api_version_t version, ze_driver_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  ze_result_t result = ZE_RESULT_SUCCESS;
  pDdiTable->pfnGet = zeDriverGet;
  pDdiTable->pfnGetApiVersion = zeDriverGetApiVersion;
  pDdiTable->pfnGetProperties = zeDriverGetProperties;
  pDdiTable->pfnGetIpcProperties = zeDriverGetIpcProperties;
  pDdiTable->pfnGetExtensionProperties = zeDriverGetExtensionProperties;
  return result;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL
zeGetMemProcAddrTable(ze_api_version_t version, ze_mem_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;
  pDdiTable->pfnAllocShared = zeMemAllocShared;
  pDdiTable->pfnAllocDevice = zeMemAllocDevice;
  pDdiTable->pfnAllocHost = zeMemAllocHost;
  pDdiTable->pfnFree = zeMemFree;
  pDdiTable->pfnGetAllocProperties = zeMemGetAllocProperties;
  pDdiTable->pfnGetAddressRange = zeMemGetAddressRange;
  pDdiTable->pfnGetIpcHandle = zeMemGetIpcHandle;
  pDdiTable->pfnOpenIpcHandle = zeMemOpenIpcHandle;
  pDdiTable->pfnCloseIpcHandle = zeMemCloseIpcHandle;
  return result;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zeGetContextProcAddrTable(
    ze_api_version_t version, ze_context_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;
  pDdiTable->pfnCreate = zeContextCreate;
  pDdiTable->pfnDestroy = zeContextDestroy;
  pDdiTable->pfnGetStatus = zeContextGetStatus;
  pDdiTable->pfnSystemBarrier = zeContextSystemBarrier;
  pDdiTable->pfnMakeMemoryResident = zeContextMakeMemoryResident;
  pDdiTable->pfnEvictMemory = zeContextEvictMemory;
  pDdiTable->pfnMakeImageResident = zeContextMakeImageResident;
  pDdiTable->pfnEvictImage = zeContextEvictImage;
  return result;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zeGetPhysicalMemProcAddrTable(
    ze_api_version_t version, ze_physical_mem_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;
  pDdiTable->pfnCreate = zePhysicalMemCreate;
  pDdiTable->pfnDestroy = zePhysicalMemDestroy;
  return result;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zeGetVirtualMemProcAddrTable(
    ze_api_version_t version, ze_virtual_mem_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;
  pDdiTable->pfnReserve = zeVirtualMemReserve;
  pDdiTable->pfnFree = zeVirtualMemFree;
  pDdiTable->pfnQueryPageSize = zeVirtualMemQueryPageSize;
  pDdiTable->pfnMap = zeVirtualMemMap;
  pDdiTable->pfnUnmap = zeVirtualMemUnmap;
  pDdiTable->pfnSetAccessAttribute = zeVirtualMemSetAccessAttribute;
  pDdiTable->pfnGetAccessAttribute = zeVirtualMemGetAccessAttribute;

  return result;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeGetGlobalProcAddrTable(
    ze_api_version_t version, ze_global_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;
  pDdiTable->pfnInit = zeInit;
  return result;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeGetDeviceProcAddrTable(
    ze_api_version_t version, ze_device_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;
  pDdiTable->pfnGet = zeDeviceGet;
  pDdiTable->pfnGetCommandQueueGroupProperties =
      zeDeviceGetCommandQueueGroupProperties;
  pDdiTable->pfnGetSubDevices = zeDeviceGetSubDevices;
  pDdiTable->pfnGetProperties = zeDeviceGetProperties;
  pDdiTable->pfnGetComputeProperties = zeDeviceGetComputeProperties;
  pDdiTable->pfnGetModuleProperties = zeDeviceGetModuleProperties;
  pDdiTable->pfnGetMemoryProperties = zeDeviceGetMemoryProperties;
  pDdiTable->pfnGetMemoryAccessProperties = zeDeviceGetMemoryAccessProperties;
  pDdiTable->pfnGetCacheProperties = zeDeviceGetCacheProperties;
  pDdiTable->pfnGetImageProperties = zeDeviceGetImageProperties;
  pDdiTable->pfnGetP2PProperties = zeDeviceGetP2PProperties;
  pDdiTable->pfnCanAccessPeer = zeDeviceCanAccessPeer;
  pDdiTable->pfnGetStatus = zeDeviceGetStatus;
  pDdiTable->pfnGetExternalMemoryProperties =
      zeDeviceGetExternalMemoryProperties;
  return result;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeGetCommandQueueProcAddrTable(
    ze_api_version_t version, ze_command_queue_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;
  pDdiTable->pfnCreate = zeCommandQueueCreate;
  pDdiTable->pfnDestroy = zeCommandQueueDestroy;
  pDdiTable->pfnExecuteCommandLists = zeCommandQueueExecuteCommandLists;
  pDdiTable->pfnSynchronize = zeCommandQueueSynchronize;

  return result;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeGetCommandListProcAddrTable(
    ze_api_version_t version, ze_command_list_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;
  pDdiTable->pfnAppendBarrier = zeCommandListAppendBarrier;
  pDdiTable->pfnAppendMemoryRangesBarrier =
      zeCommandListAppendMemoryRangesBarrier;
  pDdiTable->pfnCreate = zeCommandListCreate;
  pDdiTable->pfnCreateImmediate = zeCommandListCreateImmediate;
  pDdiTable->pfnDestroy = zeCommandListDestroy;
  pDdiTable->pfnClose = zeCommandListClose;
  pDdiTable->pfnReset = zeCommandListReset;
  pDdiTable->pfnAppendMemoryCopy = zeCommandListAppendMemoryCopy;
  pDdiTable->pfnAppendMemoryCopyRegion = zeCommandListAppendMemoryCopyRegion;
  pDdiTable->pfnAppendMemoryFill = zeCommandListAppendMemoryFill;
  pDdiTable->pfnAppendImageCopy = zeCommandListAppendImageCopy;
  pDdiTable->pfnAppendImageCopyRegion = zeCommandListAppendImageCopyRegion;
  pDdiTable->pfnAppendImageCopyToMemory = zeCommandListAppendImageCopyToMemory;
  pDdiTable->pfnAppendImageCopyFromMemory =
      zeCommandListAppendImageCopyFromMemory;
  pDdiTable->pfnAppendMemoryPrefetch = zeCommandListAppendMemoryPrefetch;
  pDdiTable->pfnAppendMemAdvise = zeCommandListAppendMemAdvise;
  pDdiTable->pfnAppendSignalEvent = zeCommandListAppendSignalEvent;
  pDdiTable->pfnAppendWaitOnEvents = zeCommandListAppendWaitOnEvents;
  pDdiTable->pfnAppendEventReset = zeCommandListAppendEventReset;
  pDdiTable->pfnAppendLaunchKernel = zeCommandListAppendLaunchKernel;
  pDdiTable->pfnAppendLaunchCooperativeKernel =
      zeCommandListAppendLaunchCooperativeKernel;
  pDdiTable->pfnAppendLaunchKernelIndirect =
      zeCommandListAppendLaunchKernelIndirect;
  pDdiTable->pfnAppendLaunchMultipleKernelsIndirect =
      zeCommandListAppendLaunchMultipleKernelsIndirect;
  pDdiTable->pfnAppendWriteGlobalTimestamp =
      zeCommandListAppendWriteGlobalTimestamp;
  pDdiTable->pfnAppendMemoryCopyFromContext =
      zeCommandListAppendMemoryCopyFromContext;
  pDdiTable->pfnAppendQueryKernelTimestamps =
      zeCommandListAppendQueryKernelTimestamps;

  return result;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeGetFenceProcAddrTable(
    ze_api_version_t version, ze_fence_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;
  pDdiTable->pfnCreate = zeFenceCreate;
  pDdiTable->pfnDestroy = zeFenceDestroy;
  pDdiTable->pfnHostSynchronize = zeFenceHostSynchronize;
  pDdiTable->pfnQueryStatus = zeFenceQueryStatus;
  pDdiTable->pfnReset = zeFenceReset;

  return result;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeGetEventPoolProcAddrTable(
    ze_api_version_t version, ze_event_pool_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;
  pDdiTable->pfnCreate = zeEventPoolCreate;
  pDdiTable->pfnDestroy = zeEventPoolDestroy;
  pDdiTable->pfnGetIpcHandle = zeEventPoolGetIpcHandle;
  pDdiTable->pfnOpenIpcHandle = zeEventPoolOpenIpcHandle;
  pDdiTable->pfnCloseIpcHandle = zeEventPoolCloseIpcHandle;

  return result;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeGetEventProcAddrTable(
    ze_api_version_t version, ze_event_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;
  pDdiTable->pfnCreate = zeEventCreate;
  pDdiTable->pfnDestroy = zeEventDestroy;
  pDdiTable->pfnHostSignal = zeEventHostSignal;
  pDdiTable->pfnHostSynchronize = zeEventHostSynchronize;
  pDdiTable->pfnQueryStatus = zeEventQueryStatus;
  pDdiTable->pfnHostReset = zeEventHostReset;
  pDdiTable->pfnQueryKernelTimestamp = zeEventQueryKernelTimestamp;

  return result;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeGetImageProcAddrTable(
    ze_api_version_t version, ze_image_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;
  pDdiTable->pfnGetProperties = zeImageGetProperties;
  pDdiTable->pfnCreate = zeImageCreate;
  pDdiTable->pfnDestroy = zeImageDestroy;

  return result;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeGetModuleProcAddrTable(
    ze_api_version_t version, ze_module_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;
  pDdiTable->pfnCreate = zeModuleCreate;
  pDdiTable->pfnDestroy = zeModuleDestroy;
  pDdiTable->pfnDynamicLink = zeModuleDynamicLink;
  pDdiTable->pfnGetNativeBinary = zeModuleGetNativeBinary;
  pDdiTable->pfnGetGlobalPointer = zeModuleGetGlobalPointer;
  pDdiTable->pfnGetKernelNames = zeModuleGetKernelNames;
  pDdiTable->pfnGetFunctionPointer = zeModuleGetFunctionPointer;
  pDdiTable->pfnGetProperties = zeModuleGetProperties;

  return result;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeGetModuleBuildLogProcAddrTable(
    ze_api_version_t version, ze_module_build_log_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;
  pDdiTable->pfnDestroy = zeModuleBuildLogDestroy;
  pDdiTable->pfnGetString = zeModuleBuildLogGetString;

  return result;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeGetKernelProcAddrTable(
    ze_api_version_t version, ze_kernel_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;
  pDdiTable->pfnCreate = zeKernelCreate;
  pDdiTable->pfnDestroy = zeKernelDestroy;
  pDdiTable->pfnSetGroupSize = zeKernelSetGroupSize;
  pDdiTable->pfnSuggestGroupSize = zeKernelSuggestGroupSize;
  pDdiTable->pfnSuggestMaxCooperativeGroupCount =
      zeKernelSuggestMaxCooperativeGroupCount;
  pDdiTable->pfnSetArgumentValue = zeKernelSetArgumentValue;
  pDdiTable->pfnSetIndirectAccess = zeKernelSetIndirectAccess;
  pDdiTable->pfnGetIndirectAccess = zeKernelGetIndirectAccess;
  pDdiTable->pfnGetSourceAttributes = zeKernelGetSourceAttributes;
  pDdiTable->pfnGetProperties = zeKernelGetProperties;
  pDdiTable->pfnSetCacheConfig = zeKernelSetCacheConfig;
  pDdiTable->pfnGetName = zeKernelGetName;

  return result;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeGetSamplerProcAddrTable(
    ze_api_version_t version, ze_sampler_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;
  pDdiTable->pfnCreate = zeSamplerCreate;
  pDdiTable->pfnDestroy = zeSamplerDestroy;

  return result;
}

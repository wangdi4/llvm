#include "level_zero/include/ze_api.h"
ZE_APIEXPORT ze_result_t ZE_APICALL zeDriverGetIpcProperties(
    ze_driver_handle_t hDriver, ze_driver_ipc_properties_t *pIpcProperties) {
  (void)hDriver;
  (void)pIpcProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeDriverGetExtensionFunctionAddress(
    ze_driver_handle_t hDriver, const char *name, void **ppFunctionAddress) {
  (void)hDriver;
  (void)name;
  (void)ppFunctionAddress;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeDeviceGetModuleProperties(ze_device_handle_t hDevice,
                            ze_device_module_properties_t *pModuleProperties) {
  (void)hDevice;
  (void)pModuleProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeDeviceGetMemoryProperties(ze_device_handle_t hDevice, uint32_t *pCount,
                            ze_device_memory_properties_t *pMemProperties) {
  (void)hDevice;
  (void)pCount;
  (void)pMemProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeDeviceGetMemoryAccessProperties(
    ze_device_handle_t hDevice,
    ze_device_memory_access_properties_t *pMemAccessProperties) {
  (void)hDevice;
  (void)pMemAccessProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeDeviceGetCacheProperties(ze_device_handle_t hDevice, uint32_t *pCount,
                           ze_device_cache_properties_t *pCacheProperties) {
  (void)hDevice;
  (void)pCount;
  (void)pCacheProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeDeviceGetImageProperties(ze_device_handle_t hDevice,
                           ze_device_image_properties_t *pImageProperties) {
  (void)hDevice;
  (void)pImageProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeDeviceGetExternalMemoryProperties(
    ze_device_handle_t hDevice,
    ze_device_external_memory_properties_t *pExternalMemoryProperties) {
  (void)hDevice;
  (void)pExternalMemoryProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeDeviceGetP2PProperties(
    ze_device_handle_t hDevice, ze_device_handle_t hPeerDevice,
    ze_device_p2p_properties_t *pP2PProperties) {
  (void)hDevice;
  (void)hPeerDevice;
  (void)pP2PProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeDeviceCanAccessPeer(ze_device_handle_t hDevice,
                      ze_device_handle_t hPeerDevice, ze_bool_t *value) {
  (void)hDevice;
  (void)hPeerDevice;
  (void)value;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeDeviceGetStatus(ze_device_handle_t hDevice) {
  (void)hDevice;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeDeviceGetGlobalTimestamps(ze_device_handle_t hDevice, uint64_t *hostTimestamp,
                            uint64_t *deviceTimestamp) {
  (void)hDevice;
  (void)hostTimestamp;
  (void)deviceTimestamp;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeContextCreateEx(ze_driver_handle_t hDriver, const ze_context_desc_t *desc,
                  uint32_t numDevices, ze_device_handle_t *phDevices,
                  ze_context_handle_t *phContext) {
  (void)hDriver;
  (void)desc;
  (void)numDevices;
  (void)phDevices;
  (void)phContext;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeContextGetStatus(ze_context_handle_t hContext) {
  (void)hContext;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListAppendWriteGlobalTimestamp(
    ze_command_list_handle_t hCommandList, uint64_t *dstptr,
    ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
    ze_event_handle_t *phWaitEvents) {
  (void)hCommandList;
  (void)dstptr;
  (void)hSignalEvent;
  (void)numWaitEvents;
  (void)phWaitEvents;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListAppendMemoryRangesBarrier(
    ze_command_list_handle_t hCommandList, uint32_t numRanges,
    const size_t *pRangeSizes, const void **pRanges,
    ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
    ze_event_handle_t *phWaitEvents) {
  (void)hCommandList;
  (void)numRanges;
  (void)pRangeSizes;
  (void)pRanges;
  (void)hSignalEvent;
  (void)numWaitEvents;
  (void)phWaitEvents;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeContextSystemBarrier(
    ze_context_handle_t hContext, ze_device_handle_t hDevice) {
  (void)hContext;
  (void)hDevice;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListAppendMemoryFill(
    ze_command_list_handle_t hCommandList, void *ptr, const void *pattern,
    size_t pattern_size, size_t size, ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents) {
  (void)hCommandList;
  (void)ptr;
  (void)pattern;
  (void)pattern_size;
  (void)size;
  (void)hSignalEvent;
  (void)numWaitEvents;
  (void)phWaitEvents;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListAppendMemoryCopyFromContext(
    ze_command_list_handle_t hCommandList, void *dstptr,
    ze_context_handle_t hContextSrc, const void *srcptr, size_t size,
    ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
    ze_event_handle_t *phWaitEvents) {
  (void)hCommandList;
  (void)dstptr;
  (void)hContextSrc;
  (void)srcptr;
  (void)size;
  (void)hSignalEvent;
  (void)numWaitEvents;
  (void)phWaitEvents;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListAppendImageCopy(
    ze_command_list_handle_t hCommandList, ze_image_handle_t hDstImage,
    ze_image_handle_t hSrcImage, ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents) {
  (void)hCommandList;
  (void)hDstImage;
  (void)hSrcImage;
  (void)hSignalEvent;
  (void)numWaitEvents;
  (void)phWaitEvents;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListAppendImageCopyRegion(
    ze_command_list_handle_t hCommandList, ze_image_handle_t hDstImage,
    ze_image_handle_t hSrcImage, const ze_image_region_t *pDstRegion,
    const ze_image_region_t *pSrcRegion, ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents) {
  (void)hCommandList;
  (void)hDstImage;
  (void)hSrcImage;
  (void)pDstRegion;
  (void)pSrcRegion;
  (void)hSignalEvent;
  (void)numWaitEvents;
  (void)phWaitEvents;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListAppendImageCopyToMemory(
    ze_command_list_handle_t hCommandList, void *dstptr,
    ze_image_handle_t hSrcImage, const ze_image_region_t *pSrcRegion,
    ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
    ze_event_handle_t *phWaitEvents) {
  (void)hCommandList;
  (void)dstptr;
  (void)hSrcImage;
  (void)pSrcRegion;
  (void)hSignalEvent;
  (void)numWaitEvents;
  (void)phWaitEvents;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListAppendImageCopyFromMemory(
    ze_command_list_handle_t hCommandList, ze_image_handle_t hDstImage,
    const void *srcptr, const ze_image_region_t *pDstRegion,
    ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
    ze_event_handle_t *phWaitEvents) {
  (void)hCommandList;
  (void)hDstImage;
  (void)srcptr;
  (void)pDstRegion;
  (void)hSignalEvent;
  (void)numWaitEvents;
  (void)phWaitEvents;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListAppendMemoryPrefetch(
    ze_command_list_handle_t hCommandList, const void *ptr, size_t size) {
  (void)hCommandList;
  (void)ptr;
  (void)size;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListAppendMemAdvise(
    ze_command_list_handle_t hCommandList, ze_device_handle_t hDevice,
    const void *ptr, size_t size, ze_memory_advice_t advice) {
  (void)hCommandList;
  (void)hDevice;
  (void)ptr;
  (void)size;
  (void)advice;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeEventPoolGetIpcHandle(
    ze_event_pool_handle_t hEventPool, ze_ipc_event_pool_handle_t *phIpc) {
  (void)hEventPool;
  (void)phIpc;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeEventPoolOpenIpcHandle(
    ze_context_handle_t hContext, ze_ipc_event_pool_handle_t hIpc,
    ze_event_pool_handle_t *phEventPool) {
  (void)hContext;
  (void)hIpc;
  (void)phEventPool;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeEventPoolCloseIpcHandle(ze_event_pool_handle_t hEventPool) {
  (void)hEventPool;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListAppendQueryKernelTimestamps(
    ze_command_list_handle_t hCommandList, uint32_t numEvents,
    ze_event_handle_t *phEvents, void *dstptr, const size_t *pOffsets,
    ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
    ze_event_handle_t *phWaitEvents) {
  (void)hCommandList;
  (void)numEvents;
  (void)phEvents;
  (void)dstptr;
  (void)pOffsets;
  (void)hSignalEvent;
  (void)numWaitEvents;
  (void)phWaitEvents;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeFenceCreate(ze_command_queue_handle_t hCommandQueue,
              const ze_fence_desc_t *desc, ze_fence_handle_t *phFence) {
  (void)hCommandQueue;
  (void)desc;
  (void)phFence;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeFenceDestroy(ze_fence_handle_t hFence) {
  (void)hFence;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeFenceHostSynchronize(ze_fence_handle_t hFence, uint64_t timeout) {
  (void)hFence;
  (void)timeout;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeFenceQueryStatus(ze_fence_handle_t hFence) {
  (void)hFence;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeFenceReset(ze_fence_handle_t hFence) {
  (void)hFence;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeImageGetProperties(ze_device_handle_t hDevice, const ze_image_desc_t *desc,
                     ze_image_properties_t *pImageProperties) {
  (void)hDevice;
  (void)desc;
  (void)pImageProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeImageCreate(ze_context_handle_t hContext,
                                                  ze_device_handle_t hDevice,
                                                  const ze_image_desc_t *desc,
                                                  ze_image_handle_t *phImage) {
  (void)hContext;
  (void)hDevice;
  (void)desc;
  (void)phImage;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeImageDestroy(ze_image_handle_t hImage) {
  (void)hImage;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeMemAllocShared(
    ze_context_handle_t hContext, const ze_device_mem_alloc_desc_t *device_desc,
    const ze_host_mem_alloc_desc_t *host_desc, size_t size, size_t alignment,
    ze_device_handle_t hDevice, void **pptr) {
  (void)hContext;
  (void)device_desc;
  (void)host_desc;
  (void)size;
  (void)alignment;
  (void)hDevice;
  (void)pptr;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeMemAllocHost(
    ze_context_handle_t hContext, const ze_host_mem_alloc_desc_t *host_desc,
    size_t size, size_t alignment, void **pptr) {
  (void)hContext;
  (void)host_desc;
  (void)size;
  (void)alignment;
  (void)pptr;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeMemGetAddressRange(ze_context_handle_t hContext, const void *ptr,
                     void **pBase, size_t *pSize) {
  (void)hContext;
  (void)ptr;
  (void)pBase;
  (void)pSize;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeMemGetIpcHandle(ze_context_handle_t hContext, const void *ptr,
                  ze_ipc_mem_handle_t *pIpcHandle) {
  (void)hContext;
  (void)ptr;
  (void)pIpcHandle;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeMemOpenIpcHandle(
    ze_context_handle_t hContext, ze_device_handle_t hDevice,
    ze_ipc_mem_handle_t handle, ze_ipc_memory_flags_t flags, void **pptr) {
  (void)hContext;
  (void)hDevice;
  (void)handle;
  (void)flags;
  (void)pptr;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeMemCloseIpcHandle(ze_context_handle_t hContext, const void *ptr) {
  (void)hContext;
  (void)ptr;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeModuleDynamicLink(uint32_t numModules, ze_module_handle_t *phModules,
                    ze_module_build_log_handle_t *phLinkLog) {
  (void)numModules;
  (void)phModules;
  (void)phLinkLog;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeModuleBuildLogDestroy(ze_module_build_log_handle_t hModuleBuildLog) {
  (void)hModuleBuildLog;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeModuleBuildLogGetString(ze_module_build_log_handle_t hModuleBuildLog,
                          size_t *pSize, char *pBuildLog) {
  (void)hModuleBuildLog;
  (void)pSize;
  (void)pBuildLog;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeModuleGetNativeBinary(
    ze_module_handle_t hModule, size_t *pSize, uint8_t *pModuleNativeBinary) {
  (void)hModule;
  (void)pSize;
  (void)pModuleNativeBinary;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeModuleGetGlobalPointer(ze_module_handle_t hModule, const char *pGlobalName,
                         size_t *pSize, void **pptr) {
  (void)hModule;
  (void)pGlobalName;
  (void)pSize;
  (void)pptr;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeModuleGetKernelNames(
    ze_module_handle_t hModule, uint32_t *pCount, const char **pNames) {
  (void)hModule;
  (void)pCount;
  (void)pNames;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeModuleGetProperties(
    ze_module_handle_t hModule, ze_module_properties_t *pModuleProperties) {
  (void)hModule;
  (void)pModuleProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeModuleGetFunctionPointer(
    ze_module_handle_t hModule, const char *pFunctionName, void **pfnFunction) {
  (void)hModule;
  (void)pFunctionName;
  (void)pfnFunction;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeKernelSuggestMaxCooperativeGroupCount(
    ze_kernel_handle_t hKernel, uint32_t *totalGroupCount) {
  (void)hKernel;
  (void)totalGroupCount;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeKernelSetIndirectAccess(
    ze_kernel_handle_t hKernel, ze_kernel_indirect_access_flags_t flags) {
  (void)hKernel;
  (void)flags;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeKernelGetIndirectAccess(
    ze_kernel_handle_t hKernel, ze_kernel_indirect_access_flags_t *pFlags) {
  (void)hKernel;
  (void)pFlags;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeKernelGetSourceAttributes(
    ze_kernel_handle_t hKernel, uint32_t *pSize, char **pString) {
  (void)hKernel;
  (void)pSize;
  (void)pString;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeKernelSetCacheConfig(
    ze_kernel_handle_t hKernel, ze_cache_config_flags_t flags) {
  (void)hKernel;
  (void)flags;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeKernelGetProperties(
    ze_kernel_handle_t hKernel, ze_kernel_properties_t *pKernelProperties) {
  (void)hKernel;
  (void)pKernelProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeKernelGetName(ze_kernel_handle_t hKernel,
                                                    size_t *pSize,
                                                    char *pName) {
  (void)hKernel;
  (void)pSize;
  (void)pName;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListAppendLaunchCooperativeKernel(
    ze_command_list_handle_t hCommandList, ze_kernel_handle_t hKernel,
    const ze_group_count_t *pLaunchFuncArgs, ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents) {
  (void)hCommandList;
  (void)hKernel;
  (void)pLaunchFuncArgs;
  (void)hSignalEvent;
  (void)numWaitEvents;
  (void)phWaitEvents;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeCommandListAppendLaunchKernelIndirect(
    ze_command_list_handle_t hCommandList, ze_kernel_handle_t hKernel,
    const ze_group_count_t *pLaunchArgumentsBuffer,
    ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
    ze_event_handle_t *phWaitEvents) {
  (void)hCommandList;
  (void)hKernel;
  (void)pLaunchArgumentsBuffer;
  (void)hSignalEvent;
  (void)numWaitEvents;
  (void)phWaitEvents;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeCommandListAppendLaunchMultipleKernelsIndirect(
    ze_command_list_handle_t hCommandList, uint32_t numKernels,
    ze_kernel_handle_t *phKernels, const uint32_t *pCountBuffer,
    const ze_group_count_t *pLaunchArgumentsBuffer,
    ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
    ze_event_handle_t *phWaitEvents) {
  (void)hCommandList;
  (void)numKernels;
  (void)phKernels;
  (void)pCountBuffer;
  (void)pLaunchArgumentsBuffer;
  (void)hSignalEvent;
  (void)numWaitEvents;
  (void)phWaitEvents;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeContextMakeMemoryResident(
    ze_context_handle_t hContext, ze_device_handle_t hDevice, void *ptr,
    size_t size) {
  (void)hContext;
  (void)hDevice;
  (void)ptr;
  (void)size;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeContextEvictMemory(ze_context_handle_t hContext, ze_device_handle_t hDevice,
                     void *ptr, size_t size) {
  (void)hContext;
  (void)hDevice;
  (void)ptr;
  (void)size;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeContextMakeImageResident(
    ze_context_handle_t hContext, ze_device_handle_t hDevice,
    ze_image_handle_t hImage) {
  (void)hContext;
  (void)hDevice;
  (void)hImage;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeContextEvictImage(ze_context_handle_t hContext, ze_device_handle_t hDevice,
                    ze_image_handle_t hImage) {
  (void)hContext;
  (void)hDevice;
  (void)hImage;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeSamplerCreate(ze_context_handle_t hContext, ze_device_handle_t hDevice,
                const ze_sampler_desc_t *desc, ze_sampler_handle_t *phSampler) {
  (void)hContext;
  (void)hDevice;
  (void)desc;
  (void)phSampler;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeSamplerDestroy(ze_sampler_handle_t hSampler) {
  (void)hSampler;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeVirtualMemReserve(ze_context_handle_t hContext, const void *pStart,
                    size_t size, void **pptr) {
  (void)hContext;
  (void)pStart;
  (void)size;
  (void)pptr;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeVirtualMemFree(ze_context_handle_t hContext, const void *ptr, size_t size) {
  (void)hContext;
  (void)ptr;
  (void)size;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeVirtualMemQueryPageSize(
    ze_context_handle_t hContext, ze_device_handle_t hDevice, size_t size,
    size_t *pagesize) {
  (void)hContext;
  (void)hDevice;
  (void)size;
  (void)pagesize;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zePhysicalMemCreate(
    ze_context_handle_t hContext, ze_device_handle_t hDevice,
    ze_physical_mem_desc_t *desc, ze_physical_mem_handle_t *phPhysicalMemory) {
  (void)hContext;
  (void)hDevice;
  (void)desc;
  (void)phPhysicalMemory;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zePhysicalMemDestroy(
    ze_context_handle_t hContext, ze_physical_mem_handle_t hPhysicalMemory) {
  (void)hContext;
  (void)hPhysicalMemory;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeVirtualMemMap(ze_context_handle_t hContext, const void *ptr, size_t size,
                ze_physical_mem_handle_t hPhysicalMemory, size_t offset,
                ze_memory_access_attribute_t access) {
  (void)hContext;
  (void)ptr;
  (void)size;
  (void)hPhysicalMemory;
  (void)offset;
  (void)access;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeVirtualMemUnmap(ze_context_handle_t hContext, const void *ptr, size_t size) {
  (void)hContext;
  (void)ptr;
  (void)size;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeVirtualMemSetAccessAttribute(
    ze_context_handle_t hContext, const void *ptr, size_t size,
    ze_memory_access_attribute_t access) {
  (void)hContext;
  (void)ptr;
  (void)size;
  (void)access;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeVirtualMemGetAccessAttribute(
    ze_context_handle_t hContext, const void *ptr, size_t size,
    ze_memory_access_attribute_t *access, size_t *outSize) {
  (void)hContext;
  (void)ptr;
  (void)size;
  (void)access;
  (void)outSize;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeKernelSetGlobalOffsetExp(ze_kernel_handle_t hKernel, uint32_t offsetX,
                           uint32_t offsetY, uint32_t offsetZ) {
  (void)hKernel;
  (void)offsetX;
  (void)offsetY;
  (void)offsetZ;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeDeviceReserveCacheExt(ze_device_handle_t hDevice, size_t cacheLevel,
                        size_t cacheReservationSize) {
  (void)hDevice;
  (void)cacheLevel;
  (void)cacheReservationSize;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeDeviceSetCacheAdviceExt(
    ze_device_handle_t hDevice, void *ptr, size_t regionSize,
    ze_cache_ext_region_t cacheRegion) {
  (void)hDevice;
  (void)ptr;
  (void)regionSize;
  (void)cacheRegion;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeEventQueryTimestampsExp(
    ze_event_handle_t hEvent, ze_device_handle_t hDevice, uint32_t *pCount,
    ze_kernel_timestamp_result_t *pTimestamps) {
  (void)hEvent;
  (void)hDevice;
  (void)pCount;
  (void)pTimestamps;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeImageGetMemoryPropertiesExp(
    ze_image_handle_t hImage,
    ze_image_memory_properties_exp_t *pMemoryProperties) {
  (void)hImage;
  (void)pMemoryProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zeImageViewCreateExp(ze_context_handle_t hContext, ze_device_handle_t hDevice,
                     const ze_image_desc_t *desc, ze_image_handle_t hImage,
                     ze_image_handle_t *phImageView) {
  (void)hContext;
  (void)hDevice;
  (void)desc;
  (void)hImage;
  (void)phImageView;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zeKernelSchedulingHintExp(
    ze_kernel_handle_t hKernel, ze_scheduling_hint_exp_desc_t *pHint) {
  (void)hKernel;
  (void)pHint;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

#include "level_zero/include/zet_api.h"
ZE_APIEXPORT ze_result_t ZE_APICALL zetModuleGetDebugInfo(
    zet_module_handle_t hModule, zet_module_debug_info_format_t format,
    size_t *pSize, uint8_t *pDebugInfo) {
  (void)hModule;
  (void)format;
  (void)pSize;
  (void)pDebugInfo;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zetDeviceGetDebugProperties(zet_device_handle_t hDevice,
                            zet_device_debug_properties_t *pDebugProperties) {
  (void)hDevice;
  (void)pDebugProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zetDebugAttach(zet_device_handle_t hDevice, const zet_debug_config_t *config,
               zet_debug_session_handle_t *phDebug) {
  (void)hDevice;
  (void)config;
  (void)phDebug;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zetDebugDetach(zet_debug_session_handle_t hDebug) {
  (void)hDebug;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zetDebugReadEvent(zet_debug_session_handle_t hDebug, uint64_t timeout,
                  zet_debug_event_t *event) {
  (void)hDebug;
  (void)timeout;
  (void)event;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zetDebugAcknowledgeEvent(
    zet_debug_session_handle_t hDebug, const zet_debug_event_t *event) {
  (void)hDebug;
  (void)event;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zetDebugInterrupt(
    zet_debug_session_handle_t hDebug, ze_device_thread_t thread) {
  (void)hDebug;
  (void)thread;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zetDebugResume(zet_debug_session_handle_t hDebug, ze_device_thread_t thread) {
  (void)hDebug;
  (void)thread;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zetDebugReadMemory(
    zet_debug_session_handle_t hDebug, ze_device_thread_t thread,
    const zet_debug_memory_space_desc_t *desc, size_t size, void *buffer) {
  (void)hDebug;
  (void)thread;
  (void)desc;
  (void)size;
  (void)buffer;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zetDebugWriteMemory(
    zet_debug_session_handle_t hDebug, ze_device_thread_t thread,
    const zet_debug_memory_space_desc_t *desc, size_t size,
    const void *buffer) {
  (void)hDebug;
  (void)thread;
  (void)desc;
  (void)size;
  (void)buffer;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zetDebugGetRegisterSetProperties(
    zet_device_handle_t hDevice, uint32_t *pCount,
    zet_debug_regset_properties_t *pRegisterSetProperties) {
  (void)hDevice;
  (void)pCount;
  (void)pRegisterSetProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zetDebugReadRegisters(
    zet_debug_session_handle_t hDebug, ze_device_thread_t thread, uint32_t type,
    uint32_t start, uint32_t count, void *pRegisterValues) {
  (void)hDebug;
  (void)thread;
  (void)type;
  (void)start;
  (void)count;
  (void)pRegisterValues;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zetDebugWriteRegisters(
    zet_debug_session_handle_t hDebug, ze_device_thread_t thread, uint32_t type,
    uint32_t start, uint32_t count, void *pRegisterValues) {
  (void)hDebug;
  (void)thread;
  (void)type;
  (void)start;
  (void)count;
  (void)pRegisterValues;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zetMetricGroupGet(zet_device_handle_t hDevice, uint32_t *pCount,
                  zet_metric_group_handle_t *phMetricGroups) {
  (void)hDevice;
  (void)pCount;
  (void)phMetricGroups;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zetMetricGroupGetProperties(zet_metric_group_handle_t hMetricGroup,
                            zet_metric_group_properties_t *pProperties) {
  (void)hMetricGroup;
  (void)pProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zetMetricGroupCalculateMetricValues(
    zet_metric_group_handle_t hMetricGroup,
    zet_metric_group_calculation_type_t type, size_t rawDataSize,
    const uint8_t *pRawData, uint32_t *pMetricValueCount,
    zet_typed_value_t *pMetricValues) {
  (void)hMetricGroup;
  (void)type;
  (void)rawDataSize;
  (void)pRawData;
  (void)pMetricValueCount;
  (void)pMetricValues;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zetMetricGet(zet_metric_group_handle_t hMetricGroup, uint32_t *pCount,
             zet_metric_handle_t *phMetrics) {
  (void)hMetricGroup;
  (void)pCount;
  (void)phMetrics;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zetMetricGetProperties(
    zet_metric_handle_t hMetric, zet_metric_properties_t *pProperties) {
  (void)hMetric;
  (void)pProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zetContextActivateMetricGroups(
    zet_context_handle_t hContext, zet_device_handle_t hDevice, uint32_t count,
    zet_metric_group_handle_t *phMetricGroups) {
  (void)hContext;
  (void)hDevice;
  (void)count;
  (void)phMetricGroups;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zetMetricStreamerOpen(
    zet_context_handle_t hContext, zet_device_handle_t hDevice,
    zet_metric_group_handle_t hMetricGroup, zet_metric_streamer_desc_t *desc,
    ze_event_handle_t hNotificationEvent,
    zet_metric_streamer_handle_t *phMetricStreamer) {
  (void)hContext;
  (void)hDevice;
  (void)hMetricGroup;
  (void)desc;
  (void)hNotificationEvent;
  (void)phMetricStreamer;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zetCommandListAppendMetricStreamerMarker(
    zet_command_list_handle_t hCommandList,
    zet_metric_streamer_handle_t hMetricStreamer, uint32_t value) {
  (void)hCommandList;
  (void)hMetricStreamer;
  (void)value;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zetMetricStreamerClose(zet_metric_streamer_handle_t hMetricStreamer) {
  (void)hMetricStreamer;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zetMetricStreamerReadData(
    zet_metric_streamer_handle_t hMetricStreamer, uint32_t maxReportCount,
    size_t *pRawDataSize, uint8_t *pRawData) {
  (void)hMetricStreamer;
  (void)maxReportCount;
  (void)pRawDataSize;
  (void)pRawData;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zetMetricQueryPoolCreate(
    zet_context_handle_t hContext, zet_device_handle_t hDevice,
    zet_metric_group_handle_t hMetricGroup,
    const zet_metric_query_pool_desc_t *desc,
    zet_metric_query_pool_handle_t *phMetricQueryPool) {
  (void)hContext;
  (void)hDevice;
  (void)hMetricGroup;
  (void)desc;
  (void)phMetricQueryPool;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zetMetricQueryPoolDestroy(zet_metric_query_pool_handle_t hMetricQueryPool) {
  (void)hMetricQueryPool;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zetMetricQueryCreate(zet_metric_query_pool_handle_t hMetricQueryPool,
                     uint32_t index, zet_metric_query_handle_t *phMetricQuery) {
  (void)hMetricQueryPool;
  (void)index;
  (void)phMetricQuery;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zetMetricQueryDestroy(zet_metric_query_handle_t hMetricQuery) {
  (void)hMetricQuery;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zetMetricQueryReset(zet_metric_query_handle_t hMetricQuery) {
  (void)hMetricQuery;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zetCommandListAppendMetricQueryBegin(zet_command_list_handle_t hCommandList,
                                     zet_metric_query_handle_t hMetricQuery) {
  (void)hCommandList;
  (void)hMetricQuery;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zetCommandListAppendMetricQueryEnd(
    zet_command_list_handle_t hCommandList,
    zet_metric_query_handle_t hMetricQuery, ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents) {
  (void)hCommandList;
  (void)hMetricQuery;
  (void)hSignalEvent;
  (void)numWaitEvents;
  (void)phWaitEvents;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zetCommandListAppendMetricMemoryBarrier(
    zet_command_list_handle_t hCommandList) {
  (void)hCommandList;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zetMetricQueryGetData(zet_metric_query_handle_t hMetricQuery,
                      size_t *pRawDataSize, uint8_t *pRawData) {
  (void)hMetricQuery;
  (void)pRawDataSize;
  (void)pRawData;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zetKernelGetProfileInfo(
    zet_kernel_handle_t hKernel, zet_profile_properties_t *pProfileProperties) {
  (void)hKernel;
  (void)pProfileProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zetTracerExpCreate(
    zet_context_handle_t hContext, const zet_tracer_exp_desc_t *desc,
    zet_tracer_exp_handle_t *phTracer) {
  (void)hContext;
  (void)desc;
  (void)phTracer;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zetTracerExpDestroy(zet_tracer_exp_handle_t hTracer) {
  (void)hTracer;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zetTracerExpSetPrologues(
    zet_tracer_exp_handle_t hTracer, zet_core_callbacks_t *pCoreCbs) {
  (void)hTracer;
  (void)pCoreCbs;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zetTracerExpSetEpilogues(
    zet_tracer_exp_handle_t hTracer, zet_core_callbacks_t *pCoreCbs) {
  (void)hTracer;
  (void)pCoreCbs;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zetTracerExpSetEnabled(zet_tracer_exp_handle_t hTracer, ze_bool_t enable) {
  (void)hTracer;
  (void)enable;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

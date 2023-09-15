/*
 * Copyright (C) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <level_zero/include/ze_api.h>
#include <level_zero/include/ze_ddi.h>
#include <level_zero/include/zes_api.h>
#include <level_zero/include/zes_ddi.h>
#include <level_zero/include/zet_api.h>
#include <level_zero/include/zet_ddi.h>

ZE_DLLEXPORT ze_result_t ZE_APICALL zesGetDeviceProcAddrTable(
    ze_api_version_t version, zes_device_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  ze_result_t result = ZE_RESULT_SUCCESS;

  pDdiTable->pfnGetProperties = zesDeviceGetProperties;
  pDdiTable->pfnGetState = zesDeviceGetState;
  pDdiTable->pfnReset = zesDeviceReset;
  pDdiTable->pfnProcessesGetState = zesDeviceProcessesGetState;
  pDdiTable->pfnPciGetProperties = zesDevicePciGetProperties;
  pDdiTable->pfnPciGetState = zesDevicePciGetState;
  pDdiTable->pfnPciGetBars = zesDevicePciGetBars;
  pDdiTable->pfnPciGetStats = zesDevicePciGetStats;
  pDdiTable->pfnEnumDiagnosticTestSuites = zesDeviceEnumDiagnosticTestSuites;
  pDdiTable->pfnEnumEngineGroups = zesDeviceEnumEngineGroups;
  pDdiTable->pfnEventRegister = zesDeviceEventRegister;
  pDdiTable->pfnEnumFabricPorts = zesDeviceEnumFabricPorts;
  pDdiTable->pfnEnumFans = zesDeviceEnumFans;
  pDdiTable->pfnEnumFirmwares = zesDeviceEnumFirmwares;
  pDdiTable->pfnEnumFrequencyDomains = zesDeviceEnumFrequencyDomains;
  pDdiTable->pfnEnumLeds = zesDeviceEnumLeds;
  pDdiTable->pfnEnumMemoryModules = zesDeviceEnumMemoryModules;
  pDdiTable->pfnEnumPerformanceFactorDomains =
      zesDeviceEnumPerformanceFactorDomains;
  pDdiTable->pfnEnumPowerDomains = zesDeviceEnumPowerDomains;
  pDdiTable->pfnEnumPsus = zesDeviceEnumPsus;
  pDdiTable->pfnEnumRasErrorSets = zesDeviceEnumRasErrorSets;
  pDdiTable->pfnEnumSchedulers = zesDeviceEnumSchedulers;
  pDdiTable->pfnEnumStandbyDomains = zesDeviceEnumStandbyDomains;
  pDdiTable->pfnEnumTemperatureSensors = zesDeviceEnumTemperatureSensors;
  return result;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zesGetDriverProcAddrTable(
    ze_api_version_t version, zes_driver_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  ze_result_t result = ZE_RESULT_SUCCESS;

  pDdiTable->pfnEventListen = zesDriverEventListen;
  return result;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zesGetDiagnosticsProcAddrTable(
    ze_api_version_t version, zes_diagnostics_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  ze_result_t result = ZE_RESULT_SUCCESS;

  pDdiTable->pfnGetProperties = zesDiagnosticsGetProperties;
  pDdiTable->pfnGetTests = zesDiagnosticsGetTests;
  pDdiTable->pfnRunTests = zesDiagnosticsRunTests;

  return result;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zesGetEngineProcAddrTable(
    ze_api_version_t version, zes_engine_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;

  pDdiTable->pfnGetProperties = zesEngineGetProperties;
  pDdiTable->pfnGetActivity = zesEngineGetActivity;

  return result;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zesGetFabricPortProcAddrTable(
    ze_api_version_t version, zes_fabric_port_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;

  pDdiTable->pfnGetProperties = zesFabricPortGetProperties;
  pDdiTable->pfnGetLinkType = zesFabricPortGetLinkType;
  pDdiTable->pfnGetConfig = zesFabricPortGetConfig;
  pDdiTable->pfnSetConfig = zesFabricPortSetConfig;
  pDdiTable->pfnGetState = zesFabricPortGetState;
  pDdiTable->pfnGetThroughput = zesFabricPortGetThroughput;

  return result;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zesGetFanProcAddrTable(
    ze_api_version_t version, zes_fan_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;

  pDdiTable->pfnGetProperties = zesFanGetProperties;
  pDdiTable->pfnGetConfig = zesFanGetConfig;
  pDdiTable->pfnSetDefaultMode = zesFanSetDefaultMode;
  pDdiTable->pfnSetFixedSpeedMode = zesFanSetFixedSpeedMode;
  pDdiTable->pfnSetSpeedTableMode = zesFanSetSpeedTableMode;
  pDdiTable->pfnGetState = zesFanGetState;

  return result;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zesGetFirmwareProcAddrTable(
    ze_api_version_t version, zes_firmware_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;

  pDdiTable->pfnGetProperties = zesFirmwareGetProperties;
  pDdiTable->pfnFlash = zesFirmwareFlash;

  return result;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zesGetFrequencyProcAddrTable(
    ze_api_version_t version, zes_frequency_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;

  pDdiTable->pfnGetProperties = zesFrequencyGetProperties;
  pDdiTable->pfnGetAvailableClocks = zesFrequencyGetAvailableClocks;
  pDdiTable->pfnGetRange = zesFrequencyGetRange;
  pDdiTable->pfnSetRange = zesFrequencySetRange;
  pDdiTable->pfnGetState = zesFrequencyGetState;
  pDdiTable->pfnGetThrottleTime = zesFrequencyGetThrottleTime;
  pDdiTable->pfnOcGetCapabilities = zesFrequencyOcGetCapabilities;
  pDdiTable->pfnOcGetFrequencyTarget = zesFrequencyOcGetFrequencyTarget;
  pDdiTable->pfnOcSetFrequencyTarget = zesFrequencyOcSetFrequencyTarget;
  pDdiTable->pfnOcGetVoltageTarget = zesFrequencyOcGetVoltageTarget;
  pDdiTable->pfnOcSetVoltageTarget = zesFrequencyOcSetVoltageTarget;
  pDdiTable->pfnOcSetMode = zesFrequencyOcSetMode;
  pDdiTable->pfnOcGetMode = zesFrequencyOcGetMode;
  pDdiTable->pfnOcGetIccMax = zesFrequencyOcGetIccMax;
  pDdiTable->pfnOcSetIccMax = zesFrequencyOcSetIccMax;
  pDdiTable->pfnOcGetTjMax = zesFrequencyOcGetTjMax;
  pDdiTable->pfnOcSetTjMax = zesFrequencyOcSetTjMax;

  return result;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zesGetLedProcAddrTable(
    ze_api_version_t version, zes_led_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;

  pDdiTable->pfnGetProperties = zesLedGetProperties;
  pDdiTable->pfnGetState = zesLedGetState;
  pDdiTable->pfnSetState = zesLedSetState;
  pDdiTable->pfnSetColor = zesLedSetColor;

  return result;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zesGetMemoryProcAddrTable(
    ze_api_version_t version, zes_memory_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;

  pDdiTable->pfnGetProperties = zesMemoryGetProperties;
  pDdiTable->pfnGetState = zesMemoryGetState;
  pDdiTable->pfnGetBandwidth = zesMemoryGetBandwidth;

  return result;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zesGetPerformanceFactorProcAddrTable(
    ze_api_version_t version, zes_performance_factor_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;

  pDdiTable->pfnGetProperties = zesPerformanceFactorGetProperties;
  pDdiTable->pfnGetConfig = zesPerformanceFactorGetConfig;
  pDdiTable->pfnSetConfig = zesPerformanceFactorSetConfig;

  return result;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zesGetPowerProcAddrTable(
    ze_api_version_t version, zes_power_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;

  pDdiTable->pfnGetProperties = zesPowerGetProperties;
  pDdiTable->pfnGetEnergyCounter = zesPowerGetEnergyCounter;
  pDdiTable->pfnGetLimits = zesPowerGetLimits;
  pDdiTable->pfnSetLimits = zesPowerSetLimits;
  pDdiTable->pfnGetEnergyThreshold = zesPowerGetEnergyThreshold;
  pDdiTable->pfnSetEnergyThreshold = zesPowerSetEnergyThreshold;

  return result;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zesGetPsuProcAddrTable(
    ze_api_version_t version, zes_psu_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;

  pDdiTable->pfnGetProperties = zesPsuGetProperties;
  pDdiTable->pfnGetState = zesPsuGetState;

  return result;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zesGetRasProcAddrTable(
    ze_api_version_t version, zes_ras_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;

  pDdiTable->pfnGetProperties = zesRasGetProperties;
  pDdiTable->pfnGetConfig = zesRasGetConfig;
  pDdiTable->pfnSetConfig = zesRasSetConfig;
  pDdiTable->pfnGetState = zesRasGetState;

  return result;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zesGetSchedulerProcAddrTable(
    ze_api_version_t version, zes_scheduler_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;

  pDdiTable->pfnGetProperties = zesSchedulerGetProperties;
  pDdiTable->pfnGetCurrentMode = zesSchedulerGetCurrentMode;
  pDdiTable->pfnGetTimeoutModeProperties = zesSchedulerGetTimeoutModeProperties;
  pDdiTable->pfnGetTimesliceModeProperties =
      zesSchedulerGetTimesliceModeProperties;
  pDdiTable->pfnSetTimeoutMode = zesSchedulerSetTimeoutMode;
  pDdiTable->pfnSetTimesliceMode = zesSchedulerSetTimesliceMode;
  pDdiTable->pfnSetExclusiveMode = zesSchedulerSetExclusiveMode;
  pDdiTable->pfnSetComputeUnitDebugMode = zesSchedulerSetComputeUnitDebugMode;

  return result;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zesGetStandbyProcAddrTable(
    ze_api_version_t version, zes_standby_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;

  pDdiTable->pfnGetProperties = zesStandbyGetProperties;
  pDdiTable->pfnGetMode = zesStandbyGetMode;
  pDdiTable->pfnSetMode = zesStandbySetMode;

  return result;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zesGetTemperatureProcAddrTable(
    ze_api_version_t version, zes_temperature_dditable_t *pDdiTable) {
  (void)version;
  if (nullptr == pDdiTable)
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;

  ze_result_t result = ZE_RESULT_SUCCESS;

  pDdiTable->pfnGetProperties = zesTemperatureGetProperties;
  pDdiTable->pfnGetConfig = zesTemperatureGetConfig;
  pDdiTable->pfnSetConfig = zesTemperatureSetConfig;
  pDdiTable->pfnGetState = zesTemperatureGetState;

  return result;
}

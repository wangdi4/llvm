#include "level_zero/include/zes_api.h"
ZE_APIEXPORT ze_result_t ZE_APICALL zesDeviceGetProperties(
    zes_device_handle_t hDevice, zes_device_properties_t *pProperties) {
  (void)hDevice;
  (void)pProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesDeviceGetState(zes_device_handle_t hDevice, zes_device_state_t *pState) {
  (void)hDevice;
  (void)pState;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesDeviceReset(zes_device_handle_t hDevice,
                                                   ze_bool_t force) {
  (void)hDevice;
  (void)force;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesDeviceProcessesGetState(zes_device_handle_t hDevice, uint32_t *pCount,
                           zes_process_state_t *pProcesses) {
  (void)hDevice;
  (void)pCount;
  (void)pProcesses;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesDevicePciGetProperties(
    zes_device_handle_t hDevice, zes_pci_properties_t *pProperties) {
  (void)hDevice;
  (void)pProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesDevicePciGetState(zes_device_handle_t hDevice, zes_pci_state_t *pState) {
  (void)hDevice;
  (void)pState;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesDevicePciGetBars(zes_device_handle_t hDevice, uint32_t *pCount,
                    zes_pci_bar_properties_t *pProperties) {
  (void)hDevice;
  (void)pCount;
  (void)pProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesDevicePciGetStats(zes_device_handle_t hDevice, zes_pci_stats_t *pStats) {
  (void)hDevice;
  (void)pStats;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesDeviceEnumDiagnosticTestSuites(zes_device_handle_t hDevice, uint32_t *pCount,
                                  zes_diag_handle_t *phDiagnostics) {
  (void)hDevice;
  (void)pCount;
  (void)phDiagnostics;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesDiagnosticsGetProperties(
    zes_diag_handle_t hDiagnostics, zes_diag_properties_t *pProperties) {
  (void)hDiagnostics;
  (void)pProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesDiagnosticsGetTests(
    zes_diag_handle_t hDiagnostics, uint32_t *pCount, zes_diag_test_t *pTests) {
  (void)hDiagnostics;
  (void)pCount;
  (void)pTests;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesDiagnosticsRunTests(zes_diag_handle_t hDiagnostics, uint32_t start,
                       uint32_t end, zes_diag_result_t *pResult) {
  (void)hDiagnostics;
  (void)start;
  (void)end;
  (void)pResult;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesDeviceEnumEngineGroups(zes_device_handle_t hDevice, uint32_t *pCount,
                          zes_engine_handle_t *phEngine) {
  (void)hDevice;
  (void)pCount;
  (void)phEngine;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesEngineGetProperties(
    zes_engine_handle_t hEngine, zes_engine_properties_t *pProperties) {
  (void)hEngine;
  (void)pProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesEngineGetActivity(zes_engine_handle_t hEngine, zes_engine_stats_t *pStats) {
  (void)hEngine;
  (void)pStats;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesDeviceEventRegister(
    zes_device_handle_t hDevice, zes_event_type_flags_t events) {
  (void)hDevice;
  (void)events;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesDriverEventListen(
    ze_driver_handle_t hDriver, uint32_t timeout, uint32_t count,
    zes_device_handle_t *phDevices, uint32_t *pNumDeviceEvents,
    zes_event_type_flags_t *pEvents) {
  (void)hDriver;
  (void)timeout;
  (void)count;
  (void)phDevices;
  (void)pNumDeviceEvents;
  (void)pEvents;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesDriverEventListenEx(
    ze_driver_handle_t hDriver, uint64_t timeout, uint32_t count,
    zes_device_handle_t *phDevices, uint32_t *pNumDeviceEvents,
    zes_event_type_flags_t *pEvents) {
  (void)hDriver;
  (void)timeout;
  (void)count;
  (void)phDevices;
  (void)pNumDeviceEvents;
  (void)pEvents;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesDeviceEnumFabricPorts(zes_device_handle_t hDevice, uint32_t *pCount,
                         zes_fabric_port_handle_t *phPort) {
  (void)hDevice;
  (void)pCount;
  (void)phPort;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFabricPortGetProperties(
    zes_fabric_port_handle_t hPort, zes_fabric_port_properties_t *pProperties) {
  (void)hPort;
  (void)pProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFabricPortGetLinkType(
    zes_fabric_port_handle_t hPort, zes_fabric_link_type_t *pLinkType) {
  (void)hPort;
  (void)pLinkType;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFabricPortGetConfig(
    zes_fabric_port_handle_t hPort, zes_fabric_port_config_t *pConfig) {
  (void)hPort;
  (void)pConfig;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFabricPortSetConfig(
    zes_fabric_port_handle_t hPort, const zes_fabric_port_config_t *pConfig) {
  (void)hPort;
  (void)pConfig;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFabricPortGetState(
    zes_fabric_port_handle_t hPort, zes_fabric_port_state_t *pState) {
  (void)hPort;
  (void)pState;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFabricPortGetThroughput(
    zes_fabric_port_handle_t hPort, zes_fabric_port_throughput_t *pThroughput) {
  (void)hPort;
  (void)pThroughput;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesDeviceEnumFans(
    zes_device_handle_t hDevice, uint32_t *pCount, zes_fan_handle_t *phFan) {
  (void)hDevice;
  (void)pCount;
  (void)phFan;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesFanGetProperties(zes_fan_handle_t hFan, zes_fan_properties_t *pProperties) {
  (void)hFan;
  (void)pProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFanGetConfig(zes_fan_handle_t hFan,
                                                    zes_fan_config_t *pConfig) {
  (void)hFan;
  (void)pConfig;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesFanSetDefaultMode(zes_fan_handle_t hFan) {
  (void)hFan;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesFanSetFixedSpeedMode(zes_fan_handle_t hFan, const zes_fan_speed_t *speed) {
  (void)hFan;
  (void)speed;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFanSetSpeedTableMode(
    zes_fan_handle_t hFan, const zes_fan_speed_table_t *speedTable) {
  (void)hFan;
  (void)speedTable;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFanGetState(zes_fan_handle_t hFan,
                                                   zes_fan_speed_units_t units,
                                                   int32_t *pSpeed) {
  (void)hFan;
  (void)units;
  (void)pSpeed;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesDeviceEnumFirmwares(zes_device_handle_t hDevice, uint32_t *pCount,
                       zes_firmware_handle_t *phFirmware) {
  (void)hDevice;
  (void)pCount;
  (void)phFirmware;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFirmwareGetProperties(
    zes_firmware_handle_t hFirmware, zes_firmware_properties_t *pProperties) {
  (void)hFirmware;
  (void)pProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesFirmwareFlash(zes_firmware_handle_t hFirmware, void *pImage, uint32_t size) {
  (void)hFirmware;
  (void)pImage;
  (void)size;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesDeviceEnumFrequencyDomains(zes_device_handle_t hDevice, uint32_t *pCount,
                              zes_freq_handle_t *phFrequency) {
  (void)hDevice;
  (void)pCount;
  (void)phFrequency;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFrequencyGetProperties(
    zes_freq_handle_t hFrequency, zes_freq_properties_t *pProperties) {
  (void)hFrequency;
  (void)pProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFrequencyGetAvailableClocks(
    zes_freq_handle_t hFrequency, uint32_t *pCount, double *phFrequency) {
  (void)hFrequency;
  (void)pCount;
  (void)phFrequency;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesFrequencyGetRange(zes_freq_handle_t hFrequency, zes_freq_range_t *pLimits) {
  (void)hFrequency;
  (void)pLimits;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFrequencySetRange(
    zes_freq_handle_t hFrequency, const zes_freq_range_t *pLimits) {
  (void)hFrequency;
  (void)pLimits;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesFrequencyGetState(zes_freq_handle_t hFrequency, zes_freq_state_t *pState) {
  (void)hFrequency;
  (void)pState;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFrequencyGetThrottleTime(
    zes_freq_handle_t hFrequency, zes_freq_throttle_time_t *pThrottleTime) {
  (void)hFrequency;
  (void)pThrottleTime;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFrequencyOcGetCapabilities(
    zes_freq_handle_t hFrequency, zes_oc_capabilities_t *pOcCapabilities) {
  (void)hFrequency;
  (void)pOcCapabilities;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFrequencyOcGetFrequencyTarget(
    zes_freq_handle_t hFrequency, double *pCurrentOcFrequency) {
  (void)hFrequency;
  (void)pCurrentOcFrequency;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFrequencyOcSetFrequencyTarget(
    zes_freq_handle_t hFrequency, double CurrentOcFrequency) {
  (void)hFrequency;
  (void)CurrentOcFrequency;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFrequencyOcGetVoltageTarget(
    zes_freq_handle_t hFrequency, double *pCurrentVoltageTarget,
    double *pCurrentVoltageOffset) {
  (void)hFrequency;
  (void)pCurrentVoltageTarget;
  (void)pCurrentVoltageOffset;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFrequencyOcSetVoltageTarget(
    zes_freq_handle_t hFrequency, double CurrentVoltageTarget,
    double CurrentVoltageOffset) {
  (void)hFrequency;
  (void)CurrentVoltageTarget;
  (void)CurrentVoltageOffset;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFrequencyOcSetMode(
    zes_freq_handle_t hFrequency, zes_oc_mode_t CurrentOcMode) {
  (void)hFrequency;
  (void)CurrentOcMode;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesFrequencyOcGetMode(
    zes_freq_handle_t hFrequency, zes_oc_mode_t *pCurrentOcMode) {
  (void)hFrequency;
  (void)pCurrentOcMode;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesFrequencyOcGetIccMax(zes_freq_handle_t hFrequency, double *pOcIccMax) {
  (void)hFrequency;
  (void)pOcIccMax;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesFrequencyOcSetIccMax(zes_freq_handle_t hFrequency, double ocIccMax) {
  (void)hFrequency;
  (void)ocIccMax;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesFrequencyOcGetTjMax(zes_freq_handle_t hFrequency, double *pOcTjMax) {
  (void)hFrequency;
  (void)pOcTjMax;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesFrequencyOcSetTjMax(zes_freq_handle_t hFrequency, double ocTjMax) {
  (void)hFrequency;
  (void)ocTjMax;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesDeviceEnumLeds(
    zes_device_handle_t hDevice, uint32_t *pCount, zes_led_handle_t *phLed) {
  (void)hDevice;
  (void)pCount;
  (void)phLed;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesLedGetProperties(zes_led_handle_t hLed, zes_led_properties_t *pProperties) {
  (void)hLed;
  (void)pProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesLedGetState(zes_led_handle_t hLed,
                                                   zes_led_state_t *pState) {
  (void)hLed;
  (void)pState;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesLedSetState(zes_led_handle_t hLed,
                                                   ze_bool_t enable) {
  (void)hLed;
  (void)enable;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesLedSetColor(zes_led_handle_t hLed, const zes_led_color_t *pColor) {
  (void)hLed;
  (void)pColor;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesDeviceEnumMemoryModules(
    zes_device_handle_t hDevice, uint32_t *pCount, zes_mem_handle_t *phMemory) {
  (void)hDevice;
  (void)pCount;
  (void)phMemory;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesMemoryGetProperties(
    zes_mem_handle_t hMemory, zes_mem_properties_t *pProperties) {
  (void)hMemory;
  (void)pProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesMemoryGetState(zes_mem_handle_t hMemory,
                                                      zes_mem_state_t *pState) {
  (void)hMemory;
  (void)pState;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesMemoryGetBandwidth(
    zes_mem_handle_t hMemory, zes_mem_bandwidth_t *pBandwidth) {
  (void)hMemory;
  (void)pBandwidth;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesDeviceEnumPerformanceFactorDomains(
    zes_device_handle_t hDevice, uint32_t *pCount, zes_perf_handle_t *phPerf) {
  (void)hDevice;
  (void)pCount;
  (void)phPerf;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesPerformanceFactorGetProperties(
    zes_perf_handle_t hPerf, zes_perf_properties_t *pProperties) {
  (void)hPerf;
  (void)pProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesPerformanceFactorGetConfig(zes_perf_handle_t hPerf, double *pFactor) {
  (void)hPerf;
  (void)pFactor;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesPerformanceFactorSetConfig(zes_perf_handle_t hPerf, double factor) {
  (void)hPerf;
  (void)factor;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesDeviceEnumPowerDomains(
    zes_device_handle_t hDevice, uint32_t *pCount, zes_pwr_handle_t *phPower) {
  (void)hDevice;
  (void)pCount;
  (void)phPower;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesPowerGetProperties(
    zes_pwr_handle_t hPower, zes_power_properties_t *pProperties) {
  (void)hPower;
  (void)pProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesPowerGetEnergyCounter(
    zes_pwr_handle_t hPower, zes_power_energy_counter_t *pEnergy) {
  (void)hPower;
  (void)pEnergy;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesPowerGetLimits(
    zes_pwr_handle_t hPower, zes_power_sustained_limit_t *pSustained,
    zes_power_burst_limit_t *pBurst, zes_power_peak_limit_t *pPeak) {
  (void)hPower;
  (void)pSustained;
  (void)pBurst;
  (void)pPeak;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesPowerSetLimits(
    zes_pwr_handle_t hPower, const zes_power_sustained_limit_t *pSustained,
    const zes_power_burst_limit_t *pBurst,
    const zes_power_peak_limit_t *pPeak) {
  (void)hPower;
  (void)pSustained;
  (void)pBurst;
  (void)pPeak;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesPowerGetEnergyThreshold(
    zes_pwr_handle_t hPower, zes_energy_threshold_t *pThreshold) {
  (void)hPower;
  (void)pThreshold;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesPowerSetEnergyThreshold(zes_pwr_handle_t hPower, double threshold) {
  (void)hPower;
  (void)threshold;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesDeviceEnumPsus(
    zes_device_handle_t hDevice, uint32_t *pCount, zes_psu_handle_t *phPsu) {
  (void)hDevice;
  (void)pCount;
  (void)phPsu;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesPsuGetProperties(zes_psu_handle_t hPsu, zes_psu_properties_t *pProperties) {
  (void)hPsu;
  (void)pProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesPsuGetState(zes_psu_handle_t hPsu,
                                                   zes_psu_state_t *pState) {
  (void)hPsu;
  (void)pState;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesDeviceEnumRasErrorSets(
    zes_device_handle_t hDevice, uint32_t *pCount, zes_ras_handle_t *phRas) {
  (void)hDevice;
  (void)pCount;
  (void)phRas;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesRasGetProperties(zes_ras_handle_t hRas, zes_ras_properties_t *pProperties) {
  (void)hRas;
  (void)pProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesRasGetConfig(zes_ras_handle_t hRas,
                                                    zes_ras_config_t *pConfig) {
  (void)hRas;
  (void)pConfig;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesRasSetConfig(zes_ras_handle_t hRas, const zes_ras_config_t *pConfig) {
  (void)hRas;
  (void)pConfig;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesRasGetState(zes_ras_handle_t hRas,
                                                   ze_bool_t clear,
                                                   zes_ras_state_t *pState) {
  (void)hRas;
  (void)clear;
  (void)pState;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesDeviceEnumSchedulers(zes_device_handle_t hDevice, uint32_t *pCount,
                        zes_sched_handle_t *phScheduler) {
  (void)hDevice;
  (void)pCount;
  (void)phScheduler;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesSchedulerGetProperties(
    zes_sched_handle_t hScheduler, zes_sched_properties_t *pProperties) {
  (void)hScheduler;
  (void)pProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesSchedulerGetCurrentMode(
    zes_sched_handle_t hScheduler, zes_sched_mode_t *pMode) {
  (void)hScheduler;
  (void)pMode;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesSchedulerGetTimeoutModeProperties(
    zes_sched_handle_t hScheduler, ze_bool_t getDefaults,
    zes_sched_timeout_properties_t *pConfig) {
  (void)hScheduler;
  (void)getDefaults;
  (void)pConfig;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesSchedulerGetTimesliceModeProperties(
    zes_sched_handle_t hScheduler, ze_bool_t getDefaults,
    zes_sched_timeslice_properties_t *pConfig) {
  (void)hScheduler;
  (void)getDefaults;
  (void)pConfig;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesSchedulerSetTimeoutMode(
    zes_sched_handle_t hScheduler, zes_sched_timeout_properties_t *pProperties,
    ze_bool_t *pNeedReload) {
  (void)hScheduler;
  (void)pProperties;
  (void)pNeedReload;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesSchedulerSetTimesliceMode(
    zes_sched_handle_t hScheduler,
    zes_sched_timeslice_properties_t *pProperties, ze_bool_t *pNeedReload) {
  (void)hScheduler;
  (void)pProperties;
  (void)pNeedReload;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesSchedulerSetExclusiveMode(
    zes_sched_handle_t hScheduler, ze_bool_t *pNeedReload) {
  (void)hScheduler;
  (void)pNeedReload;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesSchedulerSetComputeUnitDebugMode(
    zes_sched_handle_t hScheduler, ze_bool_t *pNeedReload) {
  (void)hScheduler;
  (void)pNeedReload;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesDeviceEnumStandbyDomains(zes_device_handle_t hDevice, uint32_t *pCount,
                            zes_standby_handle_t *phStandby) {
  (void)hDevice;
  (void)pCount;
  (void)phStandby;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesStandbyGetProperties(
    zes_standby_handle_t hStandby, zes_standby_properties_t *pProperties) {
  (void)hStandby;
  (void)pProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesStandbyGetMode(
    zes_standby_handle_t hStandby, zes_standby_promo_mode_t *pMode) {
  (void)hStandby;
  (void)pMode;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesStandbySetMode(
    zes_standby_handle_t hStandby, zes_standby_promo_mode_t mode) {
  (void)hStandby;
  (void)mode;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesDeviceEnumTemperatureSensors(zes_device_handle_t hDevice, uint32_t *pCount,
                                zes_temp_handle_t *phTemperature) {
  (void)hDevice;
  (void)pCount;
  (void)phTemperature;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesTemperatureGetProperties(
    zes_temp_handle_t hTemperature, zes_temp_properties_t *pProperties) {
  (void)hTemperature;
  (void)pProperties;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesTemperatureGetConfig(
    zes_temp_handle_t hTemperature, zes_temp_config_t *pConfig) {
  (void)hTemperature;
  (void)pConfig;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL zesTemperatureSetConfig(
    zes_temp_handle_t hTemperature, const zes_temp_config_t *pConfig) {
  (void)hTemperature;
  (void)pConfig;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
zesTemperatureGetState(zes_temp_handle_t hTemperature, double *pTemperature) {
  (void)hTemperature;
  (void)pTemperature;
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

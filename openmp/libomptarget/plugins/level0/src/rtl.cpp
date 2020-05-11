#if INTEL_CUSTOMIZATION
//===--- Target RTLs Implementation ---------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// RTL for SPIR-V/Xe machine
//
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <limits>
#include <mutex>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include <ze_api.h>
#include "omptargetplugin.h"
#include "omptarget-tools.h"
#include "rtl-trace.h"

/// Host runtime routines being used
extern "C" {
#ifdef _WIN32
int __cdecl omp_get_thread_limit(void);
double __cdecl omp_get_wtime(void);
#else
int omp_get_thread_limit(void) __attribute__((weak));
double omp_get_wtime(void) __attribute__((weak));
#endif
} // extern "C"

/// OMPT support
extern thread_local OmptTraceTy *omptTracePtr;
extern void omptInitPlugin();
extern const char *omptDocument;
extern ompt_interface_fn_t omptLookupEntries(const char *);

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define ABS(x) ((x) < 0 ? -(x) : (x))

// Parameters
#define LEVEL0_ALIGNMENT 0 // Default alignmnet for allocation
#define LEVEL0_ND_GROUP_SIZE 16 // Default group size for ND partitioning
#define LEVEL0_MAX_GROUP_COUNT 64 // TODO: get it from HW

int DebugLevel = 0;

/// Per-device global entry table
struct FuncOrGblEntryTy {
  __tgt_target_table Table;
  std::vector<__tgt_offload_entry> Entries;
  std::vector<ze_kernel_handle_t> Kernels;
  ze_module_handle_t Module;
};

/// Module data to be initialized by plugin
struct ModuleDataTy {
  int Initialized = 0;
  int NumDevices = 0;
  int DeviceNum = -1;
};

/// RTL profile -- only host timer for now
class RTLProfileTy {
  struct TimeTy {
    double HostTime = 0.0;
    double DeviceTime = 0.0; // Not used for now
  };
  std::thread::id ThreadId;
  std::map<std::string, TimeTy> Data;
public:
  static const int64_t MSEC_PER_SEC = 1000;
  static const int64_t USEC_PER_SEC = 1000000;
  static int64_t Multiplier;

  RTLProfileTy() {
    ThreadId = std::this_thread::get_id();
  }

  void printData(int32_t DeviceId, const char *DeviceName) {
    std::ostringstream o;
    o << ThreadId;
    fprintf(stderr, "LIBOMPTARGET_PROFILE for OMP DEVICE(%" PRId32 ") %s"
            ", Thread %s\n", DeviceId, DeviceName, o.str().c_str());
    const char *unit = (Multiplier == MSEC_PER_SEC) ? "msec" : "usec";
    fprintf(stderr, "-- Name: Host Time (%s)\n", unit);
    double hostTotal = 0.0;
    for (const auto &d : Data) {
      double hostTime = d.second.HostTime * Multiplier;
      fprintf(stderr, "-- %s: %.3f\n", d.first.c_str(), hostTime);
      hostTotal += hostTime;
    }
    fprintf(stderr, "-- Total: %.3f\n", hostTotal);
  }

  void update(const char *Name, double Elapsed) {
    std::string key(Name);
    TimeTy &time = Data[key];
    time.HostTime += Elapsed;
  }

  void update(std::string &Name, double Elapsed) {
    TimeTy &time = Data[Name];
    time.HostTime += Elapsed;
  }
};
int64_t RTLProfileTy::Multiplier;

/// Handles/data to be created for each threads
struct PrivateHandlesTy {
  ze_command_list_handle_t CmdList = nullptr;
  ze_command_queue_handle_t CmdQueue = nullptr;
  RTLProfileTy *Profile = nullptr;
};

thread_local PrivateHandlesTy ThreadLocalHandles;

/// Create a command list
static ze_command_list_handle_t createCmdList(ze_device_handle_t device) {
  ze_command_list_desc_t cmdListDesc = {
    ZE_COMMAND_LIST_DESC_VERSION_CURRENT,
    ZE_COMMAND_LIST_FLAG_EXPLICIT_ONLY
  };
  ze_command_list_handle_t cmdList;
  CALL_ZE_RET_NULL(zeCommandListCreate, device, &cmdListDesc, &cmdList);
  return cmdList;
}

/// Create a command queue
static ze_command_queue_handle_t createCmdQueue(ze_device_handle_t device) {
  ze_command_queue_desc_t cmdQueueDesc = {
    ZE_COMMAND_QUEUE_DESC_VERSION_CURRENT,
    ZE_COMMAND_QUEUE_FLAG_NONE,
    ZE_COMMAND_QUEUE_MODE_ASYNCHRONOUS,
    ZE_COMMAND_QUEUE_PRIORITY_NORMAL,
    0 // ordinal
  };
  ze_command_queue_handle_t cmdQueue;
  CALL_ZE_RET_NULL(zeCommandQueueCreate, device, &cmdQueueDesc, &cmdQueue);
  return cmdQueue;
}

/// Create a fence
static ze_fence_handle_t createFence(ze_command_queue_handle_t cmdQueue) {
  ze_fence_desc_t fenceDesc = {
    ZE_FENCE_DESC_VERSION_CURRENT,
    ZE_FENCE_FLAG_NONE
  };
  ze_fence_handle_t fence;
  CALL_ZE_RET(0, zeFenceCreate, cmdQueue, &fenceDesc, &fence);
  return fence;
}

/// RTL flags
struct RTLFlagsTy {
  uint64_t EnableProfile : 1;
  uint64_t EnableTargetGlobals : 1;
  uint64_t Reserved : 62;
  RTLFlagsTy() : EnableProfile(0), EnableTargetGlobals(0), Reserved(0) {}
};

/// Device information
class RTLDeviceInfoTy {
  /// Type of the device version of the offload table.
  /// The type may not match the host offload table's type
  /// due to extensions.
  struct DeviceOffloadEntryTy {
    /// Common part with the host offload table.
    __tgt_offload_entry Base;
    /// Length of the Base.name string in bytes including
    /// the null terminator.
    size_t NameSize;
  };

  /// Looks up an external global variable with the given \p Name
  /// and \p Size in the device environment for device \p DeviceId.
  void *getVarDeviceAddr(int32_t DeviceId, const char *Name, size_t Size);

public:
  uint32_t NumDevices;

  // TODO: multiple device groups are required if two different types of devices
  // exist.
  ze_driver_handle_t Driver;
  ze_device_properties_t DeviceProperties;
  ze_device_compute_properties_t ComputeProperties;

  std::vector<ze_device_handle_t> Devices;
  // Use per-thread command list/queue
  std::vector<std::vector<ze_command_list_handle_t>> CmdLists;
  std::vector<std::vector<ze_command_queue_handle_t>> CmdQueues;
  std::vector<FuncOrGblEntryTy> FuncGblEntries;
  std::vector<std::vector<void *>> OwnedMemory; // Memory owned by the plugin
  std::vector<bool> Initialized;
  std::mutex *Mutexes;
  std::mutex *DataMutexes; // For internal data
  std::vector<std::vector<DeviceOffloadEntryTy>> OffloadTables;
  std::vector<std::vector<RTLProfileTy *>> Profiles;

  /// Flags, parameters, options
  RTLFlagsTy Flags;
  int64_t RequiresFlags = OMP_REQ_UNDEFINED;
  uint32_t DataTransferLatency; // Emulated data transfer latency in us
  int32_t DeviceType;
  uint32_t ThreadLimit; // Global thread limit
  std::string CompilationOptions; // Compilation options for IGC

  RTLDeviceInfoTy() {
    NumDevices = 0;
    DataTransferLatency = 0;
    DeviceType = ZE_DEVICE_TYPE_GPU;
    readEnvironmentVars();
  }

  void readEnvironmentVars() {
    // Debug level
    if (char *env = getenv("LIBOMPTARGET_DEBUG"))
      DebugLevel = std::stoi(env);

    // Data transfer latency
    if (char *env = getenv("LIBOMPTARGET_DATA_TRANSFER_LATENCY")) {
      std::string value(env);
      if (value.substr(0, 2) == "T,") {
        int32_t usec = std::stoi(value.substr(2).c_str());
        DataTransferLatency = (usec > 0) ? usec : 0;
      }
    }
    // Target device type
    if (char *env = getenv("LIBOMPTARGET_DEVICETYPE")) {
      std::string value(env);
      if (value == "GPU" || value == "gpu")
        DeviceType = ZE_DEVICE_TYPE_GPU;
      else if (value == "FPGA" || value == "fpga")
        DeviceType = ZE_DEVICE_TYPE_FPGA;
      else
        DP("Warning: Invalid LIBOMPTARGET_DEVICETYPE=%s!\n", env);
    }
    DP("Target device type is set to %s\n",
       (DeviceType == ZE_DEVICE_TYPE_GPU) ? "GPU" : "FPGA");
    // Global thread limit
    int threadLimit = omp_get_thread_limit();
    ThreadLimit = threadLimit > 0 ? threadLimit : 0;
    // Compilation options for IGC
    if (char *env = getenv("LIBOMPTARGET_LEVEL0_COMPILATION_OPTIONS"))
      CompilationOptions += env;

    if (DeviceType == ZE_DEVICE_TYPE_GPU) {
      // Intel Graphics compilers that do not support that option
      // silently ignore it. Other OpenCL compilers may fail.
      const char *env = std::getenv("LIBOMPTARGET_LEVEL0_TARGET_GLOBALS");
      if (env && (env[0] == 'T' || env[0] == 't' || env[0] == '1')) {
        CompilationOptions += " -cl-take-global-address ";
        Flags.EnableTargetGlobals = 1;
      }
    }
    // Profile
    if (char *env = getenv("LIBOMPTARGET_PROFILE")) {
      if ((env[0] == 'T' || env[0] == '1') &&
          (env[1] == ',' || env[1] == '\0')) {
        Flags.EnableProfile = 1;
        RTLProfileTy::Multiplier = RTLProfileTy::MSEC_PER_SEC;
        if (env[1] == ',') {
          std::string unit(&env[2]);
          if (unit == "usec" || unit == "unit_usec")
            RTLProfileTy::Multiplier = RTLProfileTy::USEC_PER_SEC;
        }
      }
    }
  }

  ze_command_list_handle_t getCmdList(int32_t DeviceId) {
    if (!ThreadLocalHandles.CmdList) {
      auto cmdList = createCmdList(Devices[DeviceId]);
      // Store it in the global list for clean up
      DataMutexes[DeviceId].lock();
      CmdLists[DeviceId].push_back(cmdList);
      DataMutexes[DeviceId].unlock();
      ThreadLocalHandles.CmdList = cmdList;
    }
    return ThreadLocalHandles.CmdList;
  }

  ze_command_queue_handle_t getCmdQueue(int32_t DeviceId) {
    if (!ThreadLocalHandles.CmdQueue) {
      auto cmdQueue = createCmdQueue(Devices[DeviceId]);
      // Store it in the global list for clean up
      DataMutexes[DeviceId].lock();
      CmdQueues[DeviceId].push_back(cmdQueue);
      DataMutexes[DeviceId].unlock();
      ThreadLocalHandles.CmdQueue = cmdQueue;
    }
    return ThreadLocalHandles.CmdQueue;
  }

  RTLProfileTy *getProfile(int32_t DeviceId) {
    if (!ThreadLocalHandles.Profile && Flags.EnableProfile) {
      ThreadLocalHandles.Profile = new RTLProfileTy();
      DataMutexes[DeviceId].lock();
      Profiles[DeviceId].push_back(ThreadLocalHandles.Profile);
      DataMutexes[DeviceId].unlock();
    }
    return ThreadLocalHandles.Profile;
  }

  /// Loads the device version of the offload table for device \p DeviceId.
  /// The table is expected to have \p NumEntries entries.
  /// Returns true, if the load was successful, false - otherwise.
  bool loadOffloadTable(int32_t DeviceId, size_t NumEntries);

  /// Deallocates resources allocated for the device offload table
  /// of \p DeviceId device.
  void unloadOffloadTable(int32_t DeviceId);

  /// Looks up an OpenMP declare target global variable with the given
  /// \p Name and \p Size in the device environment for device \p DeviceId.
  /// The lookup is first done via the device offload table. If it fails,
  /// then the lookup falls back to non-OpenMP specific lookup on the device.
  void *getOffloadVarDeviceAddr(
      int32_t DeviceId, const char *Name, size_t Size);
};

/// Libomptarget-defined handler and argument.
struct AsyncEventTy {
  void (*Handler)(void *);
  void *Arg;
};

/// Loop descriptor
typedef struct {
  int64_t Lb;     // The lower bound of the i-th loop
  int64_t Ub;     // The upper bound of the i-th loop
  int64_t Stride; // The stride of the i-th loop
} TgtLoopDescTy;

static RTLDeviceInfoTy DeviceInfo;

/// For scoped start/stop
class ScopedTimerTy {
  std::string Name;
  double TimeStamp = 0.0;
  bool Active = false;
  RTLProfileTy *Profile = nullptr;
public:
  ScopedTimerTy(int32_t DeviceId, const char *name) : Name(name) {
    Profile = DeviceInfo.getProfile(DeviceId);
    start();
  }
  ScopedTimerTy(int32_t DeviceId, std::string name) : Name(name) {
    Profile = DeviceInfo.getProfile(DeviceId);
    start();
  }
  ~ScopedTimerTy() {
    if (Active)
      stop();
  }
  void start() {
    if (!DeviceInfo.Flags.EnableProfile)
      return;
    if (!Profile) {
      WARNING("Profile data are invalid");
      return;
    }
    if (Active)
      WARNING("Timer restarted");
    TimeStamp = omp_get_wtime();
    Active = true;
  }
  void stop() {
    if (!DeviceInfo.Flags.EnableProfile)
      return;
    if (!Profile) {
      WARNING("Profile data are invalid");
      return;
    }
    if (!Active) {
      WARNING("Timer is invalid");
      return;
    }
    double currStamp = omp_get_wtime();
    Profile->update(Name, currStamp - TimeStamp);
    Active = false;
  }
};

static void addDataTransferLatency() {
  if (DeviceInfo.DataTransferLatency == 0)
    return;
  double goal = omp_get_wtime() + 1e-6 * DeviceInfo.DataTransferLatency;
  while (omp_get_wtime() < goal)
    ;
}

/// Clean-up routine to be registered by std::atexit().
static void closeRTL() {
  for (uint32_t i = 0; i < DeviceInfo.NumDevices; i++) {
    if (!DeviceInfo.Initialized[i])
      continue;
    if (DeviceInfo.Flags.EnableProfile) {
      for (auto profile : DeviceInfo.Profiles[i]) {
        profile->printData(i, DeviceInfo.DeviceProperties.name);
        delete profile;
      }
    }
    if (omptEnabled.enabled) {
      OMPT_CALLBACK(ompt_callback_device_unload, i, 0 /* module ID */);
      OMPT_CALLBACK(ompt_callback_device_finalize, i);
    }
#ifndef _WIN32
    DeviceInfo.Mutexes[i].lock();
    for (auto mem : DeviceInfo.OwnedMemory[i]) {
      CALL_ZE_EXIT_FAIL(zeDriverFreeMem, DeviceInfo.Driver, mem);
    }
    for (auto cmdQueue : DeviceInfo.CmdQueues[i])
      CALL_ZE_EXIT_FAIL(zeCommandQueueDestroy, cmdQueue);
    for (auto cmdList : DeviceInfo.CmdLists[i])
      CALL_ZE_EXIT_FAIL(zeCommandListDestroy, cmdList);
    for (auto kernel : DeviceInfo.FuncGblEntries[i].Kernels) {
      if (kernel)
        CALL_ZE_EXIT_FAIL(zeKernelDestroy, kernel);
    }
    CALL_ZE_EXIT_FAIL(zeModuleDestroy, DeviceInfo.FuncGblEntries[i].Module);
    DeviceInfo.Mutexes[i].unlock();
#endif
    if (DeviceInfo.Flags.EnableTargetGlobals)
      DeviceInfo.unloadOffloadTable(i);
  }
  delete[] DeviceInfo.Mutexes;
  delete[] DeviceInfo.DataMutexes;
  DP("Closed RTL successfully\n");
}

/// Initialize module data
static int32_t initModule(int32_t DeviceId) {
  // Prepare host data to copy
  ModuleDataTy hostData = {
    1,                              // Initialized
    (int32_t)DeviceInfo.NumDevices, // Number of devices
    DeviceId                        // Device ID
  };

  // Prepare device data location
  auto driver = DeviceInfo.Driver;
  auto device = DeviceInfo.Devices[DeviceId];
  ze_device_mem_alloc_desc_t allocDesc = {
    ZE_DEVICE_MEM_ALLOC_DESC_VERSION_CURRENT,
    ZE_DEVICE_MEM_ALLOC_FLAG_DEFAULT,
    0 /* ordinal */
  };
  void *deviceData = nullptr;

  std::unique_lock<std::mutex> kernelLock(DeviceInfo.Mutexes[DeviceId]);

  CALL_ZE_RET_FAIL(zeDriverAllocDeviceMem, driver, &allocDesc, sizeof(hostData),
                   LEVEL0_ALIGNMENT, device, &deviceData);

  // Prepare kernel to initialize data
  ze_kernel_desc_t kernelDesc = {
    ZE_KERNEL_DESC_VERSION_CURRENT,
    ZE_KERNEL_FLAG_NONE,
    "__kmpc_init_program"
  };
  auto module = DeviceInfo.FuncGblEntries[DeviceId].Module;
  ze_kernel_handle_t initModuleData;
  CALL_ZE_RET_FAIL(zeKernelCreate, module, &kernelDesc, &initModuleData);
  CALL_ZE_RET_FAIL(zeKernelSetArgumentValue, initModuleData, 0,
                   sizeof(deviceData), &deviceData);
  ze_group_count_t groupCounts = {1, 1, 1};
  CALL_ZE_RET_FAIL(zeKernelSetGroupSize, initModuleData, 1, 1, 1);

  // Invoke the kernel
  auto cmdList = DeviceInfo.getCmdList(DeviceId);
  auto cmdQueue = DeviceInfo.getCmdQueue(DeviceId);
  CALL_ZE_RET_FAIL(zeCommandListAppendMemoryCopy, cmdList, deviceData,
                   &hostData, sizeof(hostData), nullptr);
  CALL_ZE_RET_FAIL(zeCommandListAppendBarrier, cmdList, nullptr, 0, nullptr);
  CALL_ZE_RET_FAIL(zeCommandListAppendLaunchKernel, cmdList, initModuleData,
                   &groupCounts, nullptr, 0, nullptr);
  CALL_ZE_RET_FAIL(zeCommandListAppendBarrier, cmdList, nullptr, 0, nullptr);
  CALL_ZE_RET_FAIL(zeCommandListClose, cmdList);
  CALL_ZE_RET_FAIL(zeCommandQueueExecuteCommandLists, cmdQueue, 1, &cmdList,
                   nullptr);
  CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, cmdQueue, UINT32_MAX);
  CALL_ZE_RET_FAIL(zeCommandListReset, cmdList);
  CALL_ZE_RET_FAIL(zeKernelDestroy, initModuleData);
  CALL_ZE_RET_FAIL(zeDriverFreeMem, driver, deviceData);

  return OFFLOAD_SUCCESS;
}

EXTERN
int32_t __tgt_rtl_is_valid_binary(__tgt_device_image *Image) {
  uint32_t magicWord = *(uint32_t *)Image->ImageStart;
  // compare magic word in little endian and big endian:
  int32_t ret = (magicWord == 0x07230203 || magicWord == 0x03022307);
  DP("Target binary is %s\n", ret ? "VALID" : "INVALID");
  return ret;
}

EXTERN int64_t __tgt_rtl_init_requires(int64_t RequiresFlags) {
  DP("Initialize requires flags to %" PRId64 "\n", RequiresFlags);
  DeviceInfo.RequiresFlags = RequiresFlags;
  return RequiresFlags;
}

EXTERN
int32_t __tgt_rtl_number_of_devices() {
  DP("Looking for Level0 devices...\n");

  CALL_ZE_RET_ZERO(zeInit, ZE_INIT_FLAG_NONE);

  uint32_t numDrivers = 0;
  CALL_ZE_RET_ZERO(zeDriverGet, &numDrivers, nullptr);
  if (numDrivers == 0)
    return 0;

  std::vector<ze_driver_handle_t> driverHandles(numDrivers);
  CALL_ZE_RET_ZERO(zeDriverGet, &numDrivers, driverHandles.data());
  DP("Found %" PRIu32 " driver(s)!\n", numDrivers);
  DP("Looking for device type %" PRId32 "...\n", DeviceInfo.DeviceType);

  for (uint32_t i = 0; i < numDrivers; i++) {
    // Check available devices
    uint32_t numDevices = 0;
    CALL_ZE_RET_ZERO(zeDeviceGet, driverHandles[i], &numDevices, nullptr);
    if (numDevices == 0) {
      DP("Cannot find any devices!\n");
      continue;
    }

    // Check device type
    auto &devices = DeviceInfo.Devices;
    devices.resize(numDevices);
    CALL_ZE_RET_ZERO(zeDeviceGet, driverHandles[i], &numDevices,
                     devices.data());
    ze_device_properties_t properties = {};
    properties.version = ZE_DEVICE_PROPERTIES_VERSION_CURRENT;
    CALL_ZE_RET_ZERO(zeDeviceGetProperties, devices[0], &properties);
    if (properties.type != DeviceInfo.DeviceType) {
      DP("Skipping device type %" PRId32 "...\n", properties.type);
      devices.clear();
      continue;
    }

    DeviceInfo.Driver = driverHandles[i];
    DeviceInfo.NumDevices = numDevices;
    DeviceInfo.DeviceProperties = properties;
    break;
  }

  DeviceInfo.CmdLists.resize(DeviceInfo.NumDevices);
  DeviceInfo.CmdQueues.resize(DeviceInfo.NumDevices);
  DeviceInfo.FuncGblEntries.resize(DeviceInfo.NumDevices);
  DeviceInfo.OwnedMemory.resize(DeviceInfo.NumDevices);
  DeviceInfo.Initialized.resize(DeviceInfo.NumDevices);
  DeviceInfo.Mutexes = new std::mutex[DeviceInfo.NumDevices];
  DeviceInfo.DataMutexes = new std::mutex[DeviceInfo.NumDevices];
  DeviceInfo.OffloadTables.resize(DeviceInfo.NumDevices);
  DeviceInfo.Profiles.resize(DeviceInfo.NumDevices);

  ze_device_compute_properties_t computeProperties = {};
  computeProperties.version = ZE_DEVICE_COMPUTE_PROPERTIES_VERSION_CURRENT;
  CALL_ZE_RET_ZERO(zeDeviceGetComputeProperties, DeviceInfo.Devices[0],
                   &computeProperties);
  DeviceInfo.ComputeProperties = computeProperties;

  DP("Found %" PRIu32 " device(s)!\n", DeviceInfo.NumDevices);
  DP("Type = %" PRId32 ", Name = %s\n", DeviceInfo.DeviceProperties.type,
     DeviceInfo.DeviceProperties.name);

  if (std::atexit(closeRTL)) {
    FATAL_ERROR("Registration of clean-up function");
  }

  if (DeviceInfo.NumDevices > 0) {
    omptInitPlugin();
  }

  return DeviceInfo.NumDevices;
}

EXTERN
int32_t __tgt_rtl_init_device(int32_t DeviceId) {
  if (DeviceId < 0 || (uint32_t)DeviceId >= DeviceInfo.NumDevices) {
    DP("Bad device ID %" PRId32 "\n", DeviceId);
    return OFFLOAD_FAIL;
  }

  DeviceInfo.Initialized[DeviceId] = true;

  OMPT_CALLBACK(ompt_callback_device_initialize, DeviceId,
                DeviceInfo.DeviceProperties.name,
                DeviceInfo.Devices[DeviceId],
                omptLookupEntries, omptDocument);

  DP("Initialized Level0 device %" PRId32 "\n", DeviceId);
  return OFFLOAD_SUCCESS;
}

EXTERN
__tgt_target_table *__tgt_rtl_load_binary(int32_t DeviceId,
                                          __tgt_device_image *Image) {
  DP("Device %" PRId32 ": Loading binary from " DPxMOD "\n", DeviceId,
     DPxPTR(Image->ImageStart));

  size_t imageSize = (size_t)Image->ImageEnd - (size_t)Image->ImageStart;
  size_t numEntries = (size_t)(Image->EntriesEnd - Image->EntriesBegin);
  DP("Expecting to have %zu entries defined\n", numEntries);

  ze_module_desc_t moduleDesc = {
    ZE_MODULE_DESC_VERSION_CURRENT,
    ZE_MODULE_FORMAT_IL_SPIRV,
    imageSize,
    (uint8_t *)Image->ImageStart,
    DeviceInfo.CompilationOptions.c_str(),
    nullptr /* pointer to specialization constants */
  };
  ze_module_handle_t module;
  ze_module_build_log_handle_t buildLog;
  ScopedTimerTy tmModuleBuild(DeviceId, "ModuleBuild");
  ze_result_t rc;
  CALL_ZE_RC(rc, zeModuleCreate, DeviceInfo.Devices[DeviceId], &moduleDesc,
             &module, &buildLog);
  if (rc != ZE_RESULT_SUCCESS) {
    if (DebugLevel > 0) {
      size_t logSize;
      CALL_ZE_RET_NULL(zeModuleBuildLogGetString, buildLog, &logSize, nullptr);
      std::vector<char> logString(logSize);
      CALL_ZE_RET_NULL(zeModuleBuildLogGetString, buildLog, &logSize,
                       logString.data());
      const char *logFileName = "module_build_log.txt";
      std::ofstream logFile(logFileName);
      logFile << logString.data() << std::endl;
      logFile.close();
      DP("Error: module creation failed -- see %s for details.\n", logFileName);
    }
    CALL_ZE_RET_NULL(zeModuleBuildLogDestroy, buildLog);
    return nullptr;
  }
  CALL_ZE_RET_NULL(zeModuleBuildLogDestroy, buildLog);
  tmModuleBuild.stop();

  DeviceInfo.FuncGblEntries[DeviceId].Module = module;
  auto &entries = DeviceInfo.FuncGblEntries[DeviceId].Entries;
  auto &kernels = DeviceInfo.FuncGblEntries[DeviceId].Kernels;
  entries.resize(numEntries);
  kernels.resize(numEntries);

  if (DeviceInfo.Flags.EnableTargetGlobals &&
      !DeviceInfo.loadOffloadTable(DeviceId, numEntries))
    DP("Error: offload table loading failed.\n");

  for (uint32_t i = 0; i < numEntries; i++) {
    auto size = Image->EntriesBegin[i].size;

    if (size != 0) {
      // Entry is a global variable
      auto hstAddr = Image->EntriesBegin[i].addr;
      auto name = Image->EntriesBegin[i].name;
      void *tgtAddr = nullptr;
      if (DeviceInfo.Flags.EnableTargetGlobals)
        tgtAddr = DeviceInfo.getOffloadVarDeviceAddr(DeviceId, name, size);

      if (!tgtAddr) {
        tgtAddr = __tgt_rtl_data_alloc(DeviceId, size, hstAddr);
        __tgt_rtl_data_submit(DeviceId, tgtAddr, hstAddr, size);
        DeviceInfo.DataMutexes[DeviceId].lock();
        DeviceInfo.OwnedMemory[DeviceId].push_back(tgtAddr);
        DeviceInfo.DataMutexes[DeviceId].unlock();
        DP("Error: global variable '%s' allocated. "
          "Direct references will not work properly.\n", name);
      }

      DP("Global variable mapped: Name = %s, Size = %zu, "
         "HostPtr = " DPxMOD ", TgtPtr = " DPxMOD "\n",
         name, size, DPxPTR(hstAddr), DPxPTR(tgtAddr));
      entries[i].addr = tgtAddr;
      entries[i].name = name;
      entries[i].size = size;
      kernels[i] = nullptr;
      continue;
    }

    // Entry is a kernel
    char *name = Image->EntriesBegin[i].name;
#if _WIN32
    // FIXME: temporary allow zero padding bytes in the entries table
    //        added by MSVC linker (e.g. for incremental linking).
    if (!name) {
      // Initialize the members to be on the safe side.
      DP("Warning: Entry with a nullptr name!!!\n");
      entries[i].addr = nullptr;
      entries[i].name = nullptr;
      continue;
    }
#endif
    ze_kernel_desc_t kernelDesc = {
      ZE_KERNEL_DESC_VERSION_CURRENT,
      ZE_KERNEL_FLAG_NONE,
      name,
    };
    CALL_ZE_RET_NULL(zeKernelCreate, module, &kernelDesc, &kernels[i]);
    entries[i].addr = &kernels[i];
    entries[i].name = name;
    // TODO: show kernel information
  }

  if (initModule(DeviceId) != OFFLOAD_SUCCESS)
    return nullptr;
  __tgt_target_table &table = DeviceInfo.FuncGblEntries[DeviceId].Table;
  table.EntriesBegin = &(entries.data()[0]);
  table.EntriesEnd = &(entries.data()[entries.size()]);

  OMPT_CALLBACK(ompt_callback_device_load, DeviceId,
                "" /* filename */,
                -1 /* offset_in_file */,
                nullptr /* vma_in_file */,
                table.EntriesEnd - table.EntriesBegin /* bytes */,
                table.EntriesBegin /* host_addr */,
                nullptr /* device_addr */,
                0 /* module_id */);

  return &table;
}

static void *allocData(int32_t DeviceId, int64_t Size, void *HstPtr,
                       void *HstBase, int32_t IsImplicitArg) {
  // TODO: this seems necessary for now -- check with L0 driver team for details
  std::unique_lock<std::mutex> allocLock(DeviceInfo.Mutexes[DeviceId]);

  ScopedTimerTy tmDataAlloc(DeviceId, "DataAlloc");
  intptr_t offset = (intptr_t)HstPtr - (intptr_t)HstBase;
  size_t size = (offset < 0 && ABS(offset) >= Size) ? ABS(offset) + 1 : Size;

  offset = (offset >= 0) ? offset : 0;
  size += offset;

  ze_device_mem_alloc_desc_t allocDesc = {
    ZE_DEVICE_MEM_ALLOC_DESC_VERSION_CURRENT,
    ZE_DEVICE_MEM_ALLOC_FLAG_DEFAULT,
    0 /* ordinal */
  };
  void *base = nullptr;
  CALL_ZE_RET_NULL(zeDriverAllocDeviceMem, DeviceInfo.Driver, &allocDesc, size,
                   LEVEL0_ALIGNMENT, DeviceInfo.Devices[DeviceId], &base);
  void *mem = (void *)((intptr_t)base + offset);

  if (DebugLevel > 0) {
    void *actualBase = nullptr;
    size_t actualSize = 0;
    CALL_ZE_RET_NULL(zeDriverGetMemAddressRange, DeviceInfo.Driver, mem,
                     &actualBase, &actualSize);
    assert(base == actualBase && "Invalid memory address range!");
    DP("Allocated device memory " DPxMOD " (Base: " DPxMOD
       ", Size: %zu) for host ptr " DPxMOD "\n", DPxPTR(mem), DPxPTR(actualBase),
       actualSize, DPxPTR(HstPtr));
  }

  return mem;
}

EXTERN
void *__tgt_rtl_data_alloc(int32_t DeviceId, int64_t Size, void *HstPtr) {
  return allocData(DeviceId, Size, HstPtr, HstPtr, 0);
}

EXTERN
void *__tgt_rtl_data_alloc_base(int32_t DeviceId, int64_t Size, void *HstPtr,
                                void *HstBase) {
  return allocData(DeviceId, Size, HstPtr, HstBase, 0);
}

EXTERN void *__tgt_rtl_data_alloc_managed(int32_t DeviceId, int64_t Size) {
  ze_host_mem_alloc_desc_t allocDesc = {
    ZE_HOST_MEM_ALLOC_DESC_VERSION_CURRENT,
    ZE_HOST_MEM_ALLOC_FLAG_DEFAULT
  };
  void *mem = nullptr;
  CALL_ZE_RET_NULL(zeDriverAllocHostMem, DeviceInfo.Driver, &allocDesc, Size,
                   LEVEL0_ALIGNMENT, &mem);
  DP("Allocated a managed memory object " DPxMOD "\n", DPxPTR(mem));
  return mem;
}

EXTERN int32_t __tgt_rtl_data_delete_managed(int32_t DeviceId, void *Ptr) {
  auto &mutex = DeviceInfo.Mutexes[DeviceId];
  CALL_ZE_RET_FAIL_MTX(zeDriverFreeMem, mutex, DeviceInfo.Driver, Ptr);
  DP("Deleted a managed memory object " DPxMOD "\n", DPxPTR(Ptr));
  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_is_managed_ptr(int32_t DeviceId, void *Ptr) {
  ze_memory_allocation_properties_t properties = {
    ZE_MEMORY_ALLOCATION_PROPERTIES_VERSION_CURRENT,
    ZE_MEMORY_TYPE_UNKNOWN,
    0
  };
  CALL_ZE_RET_ZERO(zeDriverGetMemAllocProperties, DeviceInfo.Driver, Ptr,
                   &properties, nullptr /* associated device */);
  int32_t ret = (properties.type == ZE_MEMORY_TYPE_HOST ||
                 properties.type == ZE_MEMORY_TYPE_SHARED);
  DP("Ptr " DPxMOD " is %sa managed memory pointer.\n", DPxPTR(Ptr),
     ret ? "" : "not ");
  return ret;
}

// Tasks to be done when completing an asynchronous command.
static void endAsyncCommand(AsyncEventTy *Event,
                            ze_command_list_handle_t CmdList,
                            ze_fence_handle_t Fence) {
  if (!Event || !Event->Handler || !Event->Arg) {
    FATAL_ERROR("Invalid asynchronous offloading event");
  }

  DP("Calling asynchronous offloading event handler " DPxMOD " with argument "
     DPxMOD "\n", DPxPTR(Event->Handler), DPxPTR(Event->Arg));

  Event->Handler(Event->Arg);

  // Clean up internal data
  CALL_ZE_EXIT_FAIL(zeFenceDestroy, Fence);
  CALL_ZE_EXIT_FAIL(zeCommandListDestroy, CmdList);
}

// Template for Asynchronous command execution.
// We use a dedicated command list and a fence to invoke an asynchronous task.
// A separate detached thread submits commands to the queue, waits until the
// attached fence is signaled, and then invokes the clean-up routine.
// Two threads **cannot** call this function simultaneously.
static int32_t beginAsyncCommand(ze_command_list_handle_t CmdList,
                                 ze_command_queue_handle_t CmdQueue,
                                 AsyncEventTy *Event, ze_fence_handle_t Fence) {
  if (!Event || !Event->Handler || !Event->Arg) {
    DP("Error: Failed to start asynchronous command -- invalid argument\n");
    return OFFLOAD_FAIL;
  }

  CALL_ZE_RET_FAIL(zeCommandListClose, CmdList);

  // Spawn waiting thread
  std::thread waiter([](AsyncEventTy *event, ze_command_list_handle_t cmdList,
                        ze_fence_handle_t fence) {
    // Wait until the fence is signaled.
    CALL_ZE_EXIT_FAIL(zeFenceHostSynchronize, fence, UINT32_MAX);
    // Invoke clean-up routine
    endAsyncCommand(event, cmdList, fence);
  }, Event, CmdList, Fence);

  waiter.detach();

  // Fence to be signaled on command-list completion.
  CALL_ZE_RET_FAIL(zeCommandQueueExecuteCommandLists, CmdQueue, 1, &CmdList,
                   Fence);

  return OFFLOAD_SUCCESS;
}

static int32_t submitData(int32_t DeviceId, void *TgtPtr, void *HstPtr,
                          int64_t Size, void *AsyncEvent) {
  if (Size == 0)
    return OFFLOAD_SUCCESS;

  ScopedTimerTy tmDataWrite(DeviceId, "DataWrite");

  // Add synthetic delay for experiments
  addDataTransferLatency();

  std::unique_lock<std::mutex> copyLock(DeviceInfo.Mutexes[DeviceId]);

  auto cmdList = DeviceInfo.getCmdList(DeviceId);
  auto cmdQueue = DeviceInfo.getCmdQueue(DeviceId);

  if (AsyncEvent) {
    cmdList = createCmdList(DeviceInfo.Devices[DeviceId]);
    if (!cmdList) {
      DP("Error: Asynchronous data submit failed -- invalid command list\n");
      return OFFLOAD_FAIL;
    }
    CALL_ZE_RET_FAIL(zeCommandListAppendMemoryCopy, cmdList, TgtPtr, HstPtr,
                     Size, nullptr);
    auto fence = createFence(cmdQueue);
    if (!fence) {
      DP("Error: Asynchronous data submit failed -- invalid fence\n");
      return OFFLOAD_FAIL;
    }
    if (beginAsyncCommand(cmdList, cmdQueue,
        static_cast<AsyncEventTy *>(AsyncEvent), fence) == OFFLOAD_FAIL)
      return OFFLOAD_FAIL;
    DP("Asynchronous data submit started -- %" PRId64 " bytes (hst:"
       DPxMOD ") -> (tgt:" DPxMOD ")\n", Size, DPxPTR(HstPtr), DPxPTR(TgtPtr));
  } else {
    CALL_ZE_RET_FAIL(zeCommandListAppendMemoryCopy, cmdList, TgtPtr, HstPtr,
                     Size, nullptr);
    CALL_ZE_RET_FAIL(zeCommandListAppendBarrier, cmdList, nullptr, 0, nullptr);
    CALL_ZE_RET_FAIL(zeCommandListClose, cmdList);
    CALL_ZE_RET_FAIL(zeCommandQueueExecuteCommandLists, cmdQueue, 1, &cmdList,
                     nullptr);
    copyLock.unlock();
    CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, cmdQueue, UINT32_MAX);
    CALL_ZE_RET_FAIL(zeCommandListReset, cmdList);
    DP("Copied %" PRId64 " bytes (hst:" DPxMOD ") -> (tgt:" DPxMOD ")\n", Size,
       DPxPTR(HstPtr), DPxPTR(TgtPtr));
  }

  return OFFLOAD_SUCCESS;
}

EXTERN
int32_t __tgt_rtl_data_submit(int32_t DeviceId, void *TgtPtr, void *HstPtr,
                              int64_t Size) {
  return submitData(DeviceId, TgtPtr, HstPtr, Size, nullptr);
}

EXTERN
int32_t
__tgt_rtl_data_submit_async(int32_t DeviceId, void *TgtPtr, void *HstPtr,
                            int64_t Size,
                            __tgt_async_info *AsyncInfoPtr /*not used*/) {
  return submitData(DeviceId, TgtPtr, HstPtr, Size, nullptr);
}

EXTERN
int32_t __tgt_rtl_data_submit_nowait(int32_t DeviceId, void *TgtPtr,
                                     void *HstPtr, int64_t Size,
                                     void *AsyncEvent) {
  return submitData(DeviceId, TgtPtr, HstPtr, Size, AsyncEvent);
}

static int32_t retrieveData(int32_t DeviceId, void *HstPtr, void *TgtPtr,
                            int64_t Size, void *AsyncEvent) {
  if (Size == 0)
    return OFFLOAD_SUCCESS;

  ScopedTimerTy tmDataRead(DeviceId, "DataRead");

  // Add synthetic delay for experiments
  addDataTransferLatency();

  std::unique_lock<std::mutex> copyLock(DeviceInfo.Mutexes[DeviceId]);

  auto cmdList = DeviceInfo.getCmdList(DeviceId);
  auto cmdQueue = DeviceInfo.getCmdQueue(DeviceId);

  if (AsyncEvent) {
    cmdList = createCmdList(DeviceInfo.Devices[DeviceId]);
    if (!cmdList) {
      DP("Error: Asynchronous data retrieve failed -- invalid command list\n");
      return OFFLOAD_FAIL;
    }
    CALL_ZE_RET_FAIL(zeCommandListAppendMemoryCopy, cmdList, HstPtr, TgtPtr,
                     Size, nullptr);
    auto fence = createFence(cmdQueue);
    if (!fence) {
      DP("Error: Asynchronous data retrieve failed -- invalid fence\n");
      return OFFLOAD_FAIL;
    }
    if (beginAsyncCommand(cmdList, cmdQueue,
        static_cast<AsyncEventTy *>(AsyncEvent), fence) == OFFLOAD_FAIL)
      return OFFLOAD_FAIL;
    DP("Asynchronous data retrieve started -- %" PRId64 " bytes (tgt:"
       DPxMOD ") -> (hst:" DPxMOD ")\n", Size, DPxPTR(TgtPtr), DPxPTR(HstPtr));
  } else {
    CALL_ZE_RET_FAIL(zeCommandListAppendMemoryCopy, cmdList, HstPtr, TgtPtr,
                     Size, nullptr);
    CALL_ZE_RET_FAIL(zeCommandListAppendBarrier, cmdList, nullptr, 0, nullptr);
    CALL_ZE_RET_FAIL(zeCommandListClose, cmdList);
    CALL_ZE_RET_FAIL(zeCommandQueueExecuteCommandLists, cmdQueue, 1, &cmdList,
                     nullptr);
    copyLock.unlock();
    CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, cmdQueue, UINT32_MAX);
    CALL_ZE_RET_FAIL(zeCommandListReset, cmdList);
    DP("Copied %" PRId64 " bytes (tgt:" DPxMOD ") -> (hst:" DPxMOD ")\n", Size,
       DPxPTR(TgtPtr), DPxPTR(HstPtr));
  }

  return OFFLOAD_SUCCESS;
}

EXTERN
int32_t __tgt_rtl_data_retrieve(int32_t DeviceId, void *HstPtr, void *TgtPtr,
                                int64_t Size) {
  return retrieveData(DeviceId, HstPtr, TgtPtr, Size, nullptr);
}

EXTERN
int32_t
__tgt_rtl_data_retrieve_async(int32_t DeviceId, void *HstPtr, void *TgtPtr,
                              int64_t Size,
                              __tgt_async_info *AsyncInfoPtr /*not used*/) {
  return retrieveData(DeviceId, HstPtr, TgtPtr, Size, nullptr);
}

EXTERN
int32_t __tgt_rtl_data_retrieve_nowait(int32_t DeviceId, void *HstPtr,
                                       void *TgtPtr, int64_t Size,
                                       void *AsyncEvent) {
  return retrieveData(DeviceId, HstPtr, TgtPtr, Size, AsyncEvent);
}

EXTERN
int32_t __tgt_rtl_data_delete(int32_t DeviceId, void *TgtPtr) {
  void *base = nullptr;
  size_t size = 0;
  CALL_ZE_RET_FAIL(zeDriverGetMemAddressRange, DeviceInfo.Driver, TgtPtr, &base,
                   &size);
  CALL_ZE_RET_FAIL_MTX(zeDriverFreeMem, DeviceInfo.Mutexes[DeviceId],
                       DeviceInfo.Driver, base);
  DP("Deleted device memory " DPxMOD " (Base: " DPxMOD ", Size: %zu)\n",
     DPxPTR(TgtPtr), DPxPTR(base), size);
  return OFFLOAD_SUCCESS;
}

static void decideGroupArguments(uint32_t NumTeams, uint32_t ThreadLimit,
                                 int64_t *LoopLevels, uint32_t *Sizes,
                                 ze_group_count_t &Dimensions) {
  uint32_t maxGroupSize = DeviceInfo.ComputeProperties.maxTotalGroupSize;
  // maxGroupCountX does not suggest practically useful count (~4M)
  //uint32_t maxGroupCount = DeviceInfo.ComputeProperties.maxGroupCountX;
  uint32_t maxGroupCount = LEVEL0_MAX_GROUP_COUNT;

  // TODO: don't have group size suggestion from kernel

  if (ThreadLimit > 0 && ThreadLimit < maxGroupSize) {
    maxGroupSize = ThreadLimit;
    DP("Max group size is set to %" PRIu32 " (thread_limit clause)\n",
       maxGroupSize);
  }
  if (DeviceInfo.ThreadLimit > 0 && DeviceInfo.ThreadLimit < maxGroupSize) {
    // TODO: what if user wants to override this with thread_limit?
    maxGroupSize = DeviceInfo.ThreadLimit;
    DP("Max group size is set to %" PRIu32 " (OMP_THREAD_LIMIT)\n",
       maxGroupSize);
  }
  if (NumTeams > 0 && NumTeams < maxGroupCount) {
    maxGroupCount = NumTeams;
    DP("Max group count is set to %" PRIu32
       " (num_teams clause or no teams construct)\n",
       maxGroupCount);
  }

  if (LoopLevels && ThreadLimit == 0 && maxGroupSize > LEVEL0_ND_GROUP_SIZE &&
      (DeviceInfo.ThreadLimit == 0 ||
       DeviceInfo.ThreadLimit == (std::numeric_limits<int32_t>::max)())) {
    maxGroupSize = LEVEL0_ND_GROUP_SIZE;
  }

  Sizes[0] = maxGroupSize;
  Sizes[1] = 1;
  Sizes[2] = 1;
  uint32_t dimensions[3] = {maxGroupCount, 1, 1};

  if (LoopLevels) {
    assert(*LoopLevels > 0 && *LoopLevels <= 3 &&
           "ND-range parallelization requested "
           "with invalid number of dimensions.");
    int64_t numLevels = *LoopLevels;
    TgtLoopDescTy *level = (TgtLoopDescTy *)(&LoopLevels[1]);
    for (int32_t i = 0; i < numLevels; i++) {
      assert(level[i].Ub >= level[i].Lb && level[i].Stride > 0 &&
             "Invalid loop description for ND-range partitioning");
      DP("Level %" PRId32 ": Lb = %" PRId64 ", Ub = %" PRId64 ", Stride = %"
         PRId64 "\n", i, level[i].Lb, level[i].Ub, level[i].Stride);
      uint32_t trip =
          (level[i].Ub - level[i].Lb + level[i].Stride) / level[i].Stride;
      if (Sizes[i] >= trip)
        Sizes[i] = trip;
      dimensions[i] = (trip + Sizes[i] - 1) / Sizes[i];
    }
  }

  Dimensions.groupCountX = dimensions[0];
  Dimensions.groupCountY = dimensions[1];
  Dimensions.groupCountZ = dimensions[2];
  DP("Group sizes = {%" PRIu32 ", %" PRIu32 ", %" PRIu32 "}\n", Sizes[0],
     Sizes[1], Sizes[2]);
  DP("Group dimensions = {%" PRIu32 ", %" PRIu32 ", %" PRIu32 "}\n",
     dimensions[0], dimensions[1], dimensions[2]);
}

static int32_t runTargetTeamRegion(int32_t DeviceId, void *TgtEntryPtr,
                                   void **TgtArgs, ptrdiff_t *TgtOffsets,
                                   int32_t NumArgs, int32_t NumTeams,
                                   int32_t ThreadLimit, void *LoopDesc,
                                   void *AsyncEvent) {
  assert(TgtEntryPtr && "Invalid kernel");
  assert((NumTeams >= 0 && ThreadLimit >= 0) && "Invalid kernel work size");
  DP("Executing a kernel " DPxMOD "...\n", DPxPTR(TgtEntryPtr));

  // Protect from kernel preparation to submission as kernels are shared
  std::unique_lock<std::mutex> kernelLock(DeviceInfo.Mutexes[DeviceId]);

  ze_kernel_handle_t kernel = *((ze_kernel_handle_t *)TgtEntryPtr);
  ze_kernel_properties_t kernelProperties;
  CALL_ZE_RET_FAIL(zeKernelGetProperties, kernel, &kernelProperties);
  std::string tmName("Kernel#");
  ScopedTimerTy tmKernel(DeviceId, tmName + kernelProperties.name);

  // Set arguments
  std::vector<void *> args(NumArgs);
  for (int32_t i = 0; i < NumArgs; i++) {
    args[i] = (void *)((intptr_t)TgtArgs[i] + TgtOffsets[i]);
    CALL_ZE_RET_FAIL(zeKernelSetArgumentValue, kernel, i, sizeof(void *),
                     &args[i]);
    DP("Kernel argument %" PRId32 " (value: " DPxMOD ") was set successfully\n",
       i, DPxPTR(args[i]));
  }
  // TODO: implicit arguments -- zeKernelSetAttribute() ?

  // Decide group sizes and dimensions
  uint32_t groupSizes[3];
  ze_group_count_t groupCounts;
  decideGroupArguments((uint32_t )NumTeams, (uint32_t)ThreadLimit,
                       (int64_t *)LoopDesc, groupSizes, groupCounts);

  if (omptEnabled.enabled) {
    // Push current work size
    size_t finalNumTeams = groupCounts.groupCountX * groupCounts.groupCountY *
        groupCounts.groupCountZ;
    size_t finalThreadLimit = groupSizes[0] * groupSizes[1] * groupSizes[2];
    omptTracePtr->pushWorkSize(finalNumTeams, finalThreadLimit);
  }

  CALL_ZE_RET_FAIL(zeKernelSetGroupSize, kernel, groupSizes[0], groupSizes[1],
                   groupSizes[2]);
  auto cmdList = DeviceInfo.getCmdList(DeviceId);
  auto cmdQueue = DeviceInfo.getCmdQueue(DeviceId);

  if (AsyncEvent) {
    cmdList = createCmdList(DeviceInfo.Devices[DeviceId]);
    if (!cmdList) {
      DP("Error: Asynchronous execution failed -- invalid command list\n");
      return OFFLOAD_FAIL;
    }
    CALL_ZE_RET_FAIL(zeCommandListAppendLaunchKernel, cmdList, kernel,
                     &groupCounts, nullptr, 0, nullptr);
    auto fence = createFence(cmdQueue);
    if (!fence) {
      DP("Error: Asynchronous execution failed -- invalid fence\n");
      return OFFLOAD_FAIL;
    }
    if (beginAsyncCommand(cmdList, cmdQueue,
        static_cast<AsyncEventTy *>(AsyncEvent), fence) == OFFLOAD_FAIL)
      return OFFLOAD_FAIL;
    DP("Asynchronous execution started for kernel " DPxMOD "\n",
       DPxPTR(TgtEntryPtr));
  } else {
    CALL_ZE_RET_FAIL(zeCommandListAppendLaunchKernel, cmdList, kernel,
                     &groupCounts, nullptr, 0, nullptr);
    CALL_ZE_RET_FAIL(zeCommandListAppendBarrier, cmdList, nullptr, 0, nullptr);

    CALL_ZE_RET_FAIL(zeCommandListClose, cmdList);
    CALL_ZE_RET_FAIL(zeCommandQueueExecuteCommandLists, cmdQueue, 1, &cmdList,
                     nullptr);
    kernelLock.unlock();
    CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, cmdQueue, UINT32_MAX);
    // Make sure the command list is ready to accept next command
    CALL_ZE_RET_FAIL(zeCommandListReset, cmdList);
  }

  DP("Executed a kernel " DPxMOD "\n", DPxPTR(TgtEntryPtr));
  return OFFLOAD_SUCCESS;
}

EXTERN
int32_t __tgt_rtl_run_target_team_nd_region(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit, void *LoopDesc) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, NumTeams, ThreadLimit, LoopDesc, nullptr);
}

EXTERN
int32_t __tgt_rtl_run_target_team_nd_region_nowait(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit, void *LoopDesc,
    void *AsyncEvent) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, NumTeams, ThreadLimit, LoopDesc,
                             AsyncEvent);
}

EXTERN
int32_t __tgt_rtl_run_target_team_region(int32_t DeviceId, void *TgtEntryPtr,
                                         void **TgtArgs, ptrdiff_t *TgtOffsets,
                                         int32_t NumArgs, int32_t NumTeams,
                                         int32_t ThreadLimit,
                                         uint64_t LoopTripCount) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, NumTeams, ThreadLimit, nullptr, nullptr);
}

EXTERN
int32_t __tgt_rtl_run_target_team_region_async(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit,
    uint64_t LoopTripCount, __tgt_async_info *AsyncInfoPtr /*not used*/) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, NumTeams, ThreadLimit, nullptr, nullptr);
}

EXTERN
int32_t __tgt_rtl_run_target_team_region_nowait(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit,
    uint64_t LoopTripCount, void *AsyncEvent) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, NumTeams, ThreadLimit, nullptr,
                             AsyncEvent);
}

EXTERN
int32_t __tgt_rtl_run_target_region(int32_t DeviceId, void *TgtEntryPtr,
                                    void **TgtArgs, ptrdiff_t *TgtOffsets,
                                    int32_t NumArgs) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, 1, 0, nullptr, nullptr);
}

EXTERN
int32_t
__tgt_rtl_run_target_region_async(int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs,
                            ptrdiff_t *TgtOffsets, int32_t NumArgs,
                            __tgt_async_info *AsyncInfoPtr /*not used*/) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, 1, 0, nullptr, nullptr);
}

EXTERN
int32_t __tgt_rtl_run_target_region_nowait(int32_t DeviceId, void *TgtEntryPtr,
                                           void **TgtArgs,
                                           ptrdiff_t *TgtOffsets,
                                           int32_t NumArgs, void *AsyncEvent) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, 1, 0, nullptr, AsyncEvent);
}

EXTERN void *__tgt_rtl_create_offload_pipe(int32_t DeviceId, bool IsAsync) {
  // Create and return a new command queue for interop
  ze_command_queue_desc_t cmdQueueDesc = {
    ZE_COMMAND_QUEUE_DESC_VERSION_CURRENT,
    ZE_COMMAND_QUEUE_FLAG_NONE,
    ZE_COMMAND_QUEUE_MODE_ASYNCHRONOUS,
    ZE_COMMAND_QUEUE_PRIORITY_NORMAL,
    0 // ordinal
  };
  // TODO: check with MKL team and decide what to do with IsAsync

  ze_command_queue_handle_t pipe = nullptr;
  CALL_ZE_RET_NULL(zeCommandQueueCreate, DeviceInfo.Devices[DeviceId],
                   &cmdQueueDesc, &pipe);
  DP("%s returns a new asynchronous command queue " DPxMOD "\n", __func__,
     DPxPTR(pipe));
  return pipe;
}

EXTERN int32_t __tgt_rtl_release_offload_pipe(int32_t DeviceId, void *Pipe) {
  CALL_ZE_RET_FAIL(zeCommandQueueDestroy, (ze_command_queue_handle_t)Pipe);
  return OFFLOAD_SUCCESS;
}

EXTERN void *__tgt_rtl_create_buffer(int32_t DeviceId, void *TgtPtr) {
  return TgtPtr;
}

EXTERN int32_t __tgt_rtl_release_buffer(void *TgtPtr) {
  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_synchronize(int32_t device_id,
                                     __tgt_async_info *async_info_ptr) {
  return OFFLOAD_SUCCESS;
}

void *RTLDeviceInfoTy::getOffloadVarDeviceAddr(
    int32_t DeviceId, const char *Name, size_t Size) {
  DP("Looking up OpenMP global variable '%s' of size %zu bytes on device %d.\n",
     Name, Size, DeviceId);

  std::vector<DeviceOffloadEntryTy> &OffloadTable = OffloadTables[DeviceId];
  if (!OffloadTable.empty()) {
    size_t NameSize = strlen(Name) + 1;
    auto I = std::lower_bound(
        OffloadTable.begin(), OffloadTable.end(), Name,
        [NameSize](const DeviceOffloadEntryTy &E, const char *Name) {
          return strncmp(E.Base.name, Name, NameSize) < 0;
        });

    if (I != OffloadTable.end() &&
        strncmp(I->Base.name, Name, NameSize) == 0) {
      DP("Global variable '%s' found in the offload table at position %zu.\n",
         Name, std::distance(OffloadTable.begin(), I));
      return I->Base.addr;
    }

    DP("Error: global variable '%s' was not found in the offload table.\n",
       Name);
  } else
    DP("Error: offload table is not loaded for device %d.\n", DeviceId);

  // Fallback to the lookup by name.
  return getVarDeviceAddr(DeviceId, Name, Size);
}

void *RTLDeviceInfoTy::getVarDeviceAddr(
    int32_t DeviceId, const char *Name, size_t Size) {
  void *TgtAddr = nullptr;
  DP("Looking up device global variable '%s' of size %zu bytes on device %d.\n",
     Name, Size, DeviceId);
  CALL_ZE_RET_NULL(zeModuleGetGlobalPointer,
                   FuncGblEntries[DeviceId].Module, Name, &TgtAddr);
  DP("Global variable lookup succeeded.\n");
  return TgtAddr;
}

bool RTLDeviceInfoTy::loadOffloadTable(int32_t DeviceId, size_t NumEntries) {
  const char *OffloadTableSizeVarName = "__omp_offloading_entries_table_size";
  void *OffloadTableSizeVarAddr =
      getVarDeviceAddr(DeviceId, OffloadTableSizeVarName, sizeof(int64_t));

  if (!OffloadTableSizeVarAddr) {
    DP("Error: cannot get device value for global variable '%s'.\n",
       OffloadTableSizeVarName);
    return false;
  }

  int64_t TableSizeVal = 0;
  __tgt_rtl_data_retrieve(DeviceId, &TableSizeVal,
                          OffloadTableSizeVarAddr, sizeof(int64_t));
  size_t TableSize = (size_t)TableSizeVal;

  if ((TableSize % sizeof(DeviceOffloadEntryTy)) != 0) {
    DP("Error: offload table size (%zu) is not a multiple of %zu.\n",
       TableSize, sizeof(DeviceOffloadEntryTy));
    return false;
  }

  size_t DeviceNumEntries = TableSize / sizeof(DeviceOffloadEntryTy);

  if (NumEntries != DeviceNumEntries) {
    DP("Error: number of entries in host and device "
       "offload tables mismatch (%zu != %zu).\n",
       NumEntries, DeviceNumEntries);
    return false;
  }

  const char *OffloadTableVarName = "__omp_offloading_entries_table";
  void *OffloadTableVarAddr =
      getVarDeviceAddr(DeviceId, OffloadTableVarName, TableSize);
  if (!OffloadTableVarAddr) {
    DP("Error: cannot get device value for global variable '%s'.\n",
       OffloadTableVarName);
    return false;
  }

  OffloadTables[DeviceId].resize(DeviceNumEntries);
  __tgt_rtl_data_retrieve(DeviceId, OffloadTables[DeviceId].data(),
                          OffloadTableVarAddr, TableSize);
  std::vector<DeviceOffloadEntryTy> &DeviceTable = OffloadTables[DeviceId];

  size_t I = 0;
  const char *PreviousName = "";

  for (; I < DeviceNumEntries; ++I) {
    DeviceOffloadEntryTy &Entry = DeviceTable[I];
    size_t NameSize = Entry.NameSize;
    void *NameTgtAddr = Entry.Base.name;
    Entry.Base.name = nullptr;

    if (NameSize == 0) {
      DP("Error: offload entry (%zu) with 0 size.\n", I);
      break;
    }

    Entry.Base.name = new char[NameSize];
    __tgt_rtl_data_retrieve(DeviceId, Entry.Base.name,
                            NameTgtAddr, NameSize);
    if (strnlen(Entry.Base.name, NameSize) != NameSize - 1) {
      DP("Error: offload entry's name has wrong size.\n");
      break;
    }

    if (strncmp(PreviousName, Entry.Base.name, NameSize) >= 0) {
      DP("Error: offload table is not sorted.\n"
         "Error: previous name is '%s'.\n"
         "Error:  current name is '%s'.\n",
         PreviousName, Entry.Base.name);
      break;
    }
    PreviousName = Entry.Base.name;
  }

  if (I != DeviceNumEntries) {
    // Errors during the table processing.
    // Deallocate all memory allocated in the loop.
    for (size_t J = 0; J <= I; ++J) {
      DeviceOffloadEntryTy &Entry = DeviceTable[J];
      if (Entry.Base.name)
        delete[] Entry.Base.name;
    }

    OffloadTables[DeviceId].clear();
    return false;
  }

  if (DebugLevel > 0) {
    DP("Device offload table loaded:\n");
    for (size_t I = 0; I < DeviceNumEntries; ++I)
      DP("\t%zu:\t%s\n", I, DeviceTable[I].Base.name);
  }

  return true;
}

void RTLDeviceInfoTy::unloadOffloadTable(int32_t DeviceId) {
  for (auto &E : OffloadTables[DeviceId])
    delete[] E.Base.name;

  OffloadTables[DeviceId].clear();
}
#endif // INTEL_CUSTOMIZATION

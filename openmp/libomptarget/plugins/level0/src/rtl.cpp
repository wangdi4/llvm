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

#include <cassert>
#include <cstring>
#include <limits>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <ze_api.h>
#include "omptargetplugin.h"

/// Host runtime routines being used
#ifdef _WIN32
int __cdecl omp_get_thread_limit(void);
double __cdecl omp_get_wtime(void);
#else
int omp_get_thread_limit(void) __attribute__((weak));
double omp_get_wtime(void) __attribute__((weak));
#endif

#ifndef TARGET_NAME
#define TARGET_NAME LEVEL0
#endif

#define STR(x) #x
#define TO_STRING(x) STR(x)
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define ABS(x) ((x) < 0 ? -(x) : (x))

// Parameters
#define LEVEL0_ALIGNMENT 0 // Default alignmnet for allocation
#define LEVEL0_ND_GROUP_SIZE 16 // Default group size for ND partitioning
#define LEVEL0_MAX_GROUP_COUNT 64 // TODO: get it from HW

#ifdef OMPTARGET_LEVEL0_DEBUG
static int DebugLevel = 0;
#define DP(...)                                                                \
  do {                                                                         \
    if (DebugLevel > 0) {                                                      \
      DEBUGP("Target " TO_STRING(TARGET_NAME) " RTL", __VA_ARGS__);            \
    }                                                                          \
  } while (0)
#else
#define DP(...)
#endif // OMPTARGET_LEVEL0_DEBUG

#define FATAL_ERROR(Msg)                                                       \
  do {                                                                         \
    fprintf(stderr, "Error: %s failed (%s) -- exiting...\n", __func__, Msg);   \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

/// For non-thread-safe functions
#define CALL_ZE_RET_MTX(Ret, Fn, Mtx, ...)                                     \
  do {                                                                         \
    Mtx.lock();                                                                \
    ze_result_t rc = Fn(__VA_ARGS__);                                          \
    Mtx.unlock();                                                              \
    if (rc != ZE_RESULT_SUCCESS) {                                             \
      DP("Error: %s:%s failed with error code %d, %s\n", __func__, #Fn, rc,    \
         getZeErrorName(rc));                                                  \
      return Ret;                                                              \
    }                                                                          \
  } while (0)

#define CALL_ZE_RET_FAIL_MTX(Fn, ...)                                          \
  CALL_ZE_RET_MTX(OFFLOAD_FAIL, Fn, __VA_ARGS__)
#define CALL_ZE_RET_NULL_MTX(Fn, ...) CALL_ZE_RET_MTX(NULL, Fn, __VA_ARGS__)
#define CALL_ZE_RET_ZERO_MTX(Fn, ...) CALL_ZE_RET_MTX(0, Fn, __VA_ARGS__)

/// For thread-safe functions
#define CALL_ZE_RET(Ret, Fn, ...)                                              \
  do {                                                                         \
    ze_result_t rc = Fn(__VA_ARGS__);                                          \
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
    ze_result_t rc = Fn(__VA_ARGS__);                                          \
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

/// Per-device global entry table
struct FuncOrGblEntryTy {
  __tgt_target_table Table;
  std::vector<__tgt_offload_entry> Entries;
  std::vector<ze_kernel_handle_t> Kernels;
  ze_module_handle_t Module;
};

/// Device information
struct RTLDeviceInfoTy {
  uint32_t NumDevices;

  // TODO: multiple device groups are required if two different types of devices
  // exist.
  ze_driver_handle_t Driver;
  ze_device_properties_t DeviceProperties;
  ze_device_compute_properties_t ComputeProperties;

  std::vector<ze_device_handle_t> Devices;
  std::vector<ze_command_list_handle_t> CmdLists;
  std::vector<ze_command_queue_handle_t> CmdQueues;
  std::vector<FuncOrGblEntryTy> FuncGblEntries;
  std::vector<std::vector<void *>> OwnedMemory; // Memory owned by the plugin
  std::vector<bool> Initialized;
  std::mutex *Mutexes;

  /// Flags, parameters, options
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
#ifdef OMPTARGET_LEVEL0_DEBUG
    // Debug level
    if (char *env = getenv("LIBOMPTARGET_DEBUG"))
      DebugLevel = std::stoi(env);
#endif
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
  }
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

#ifdef OMPTARGET_LEVEL0_DEBUG
#define CASE_TO_STRING(Num) case Num: return #Num;
static const char *getZeErrorName(int32_t Error) {
  switch (Error) {
    FOREACH_ZE_ERROR_CODE(CASE_TO_STRING)
  default:
    return "ZE_RESULT_ERROR_UNKNOWN";
  }
}
#endif // OMPTARGET_LEVEL0_DEBUG

static void addDataTransferLatency() {
  if (DeviceInfo.DataTransferLatency == 0)
    return;
  double goal = omp_get_wtime() + 1e-6 * DeviceInfo.DataTransferLatency;
  while (omp_get_wtime() < goal)
    ;
}

/// Clean-up routine to be registered by std::atexit().
static void closeRTL() {
#ifndef _WIN32
  for (uint32_t i = 0; i < DeviceInfo.NumDevices; i++) {
    if (!DeviceInfo.Initialized[i])
      continue;
    DeviceInfo.Mutexes[i].lock();
    for (auto mem : DeviceInfo.OwnedMemory[i]) {
      CALL_ZE_EXIT_FAIL(zeDriverFreeMem, DeviceInfo.Driver, mem);
    }
    CALL_ZE_EXIT_FAIL(zeCommandQueueDestroy, DeviceInfo.CmdQueues[i]);
    CALL_ZE_EXIT_FAIL(zeCommandListDestroy, DeviceInfo.CmdLists[i]);
    for (auto kernel : DeviceInfo.FuncGblEntries[i].Kernels) {
      if (kernel)
        CALL_ZE_EXIT_FAIL(zeKernelDestroy, kernel);
    }
    CALL_ZE_EXIT_FAIL(zeModuleDestroy, DeviceInfo.FuncGblEntries[i].Module);
    DeviceInfo.Mutexes[i].unlock();
  }
#endif
  delete[] DeviceInfo.Mutexes;
  DP("Closed RTL successfully\n");
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

  ze_driver_handle_t driverHandles[numDrivers];
  CALL_ZE_RET_ZERO(zeDriverGet, &numDrivers, driverHandles);
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

  return DeviceInfo.NumDevices;
}

EXTERN
int32_t __tgt_rtl_init_device(int32_t DeviceId) {
  if (DeviceId < 0 || (uint32_t)DeviceId >= DeviceInfo.NumDevices) {
    DP("Bad device ID %" PRId32 "\n", DeviceId);
    return OFFLOAD_FAIL;
  }

  auto deviceHandle = DeviceInfo.Devices[DeviceId];

  // Create a command list
  ze_command_list_desc_t cmdListDesc = {
    ZE_COMMAND_LIST_DESC_VERSION_CURRENT,
    ZE_COMMAND_LIST_FLAG_EXPLICIT_ONLY
  };
  CALL_ZE_RET_FAIL(zeCommandListCreate, deviceHandle, &cmdListDesc,
                   &DeviceInfo.CmdLists[DeviceId]);

  // Create a command queue
  ze_command_queue_desc_t cmdQueueDesc = {
    ZE_COMMAND_QUEUE_DESC_VERSION_CURRENT,
    ZE_COMMAND_QUEUE_FLAG_NONE,
    ZE_COMMAND_QUEUE_MODE_ASYNCHRONOUS,
    ZE_COMMAND_QUEUE_PRIORITY_NORMAL,
    0 // ordinal
  };
  CALL_ZE_RET_FAIL(zeCommandQueueCreate, deviceHandle, &cmdQueueDesc,
                   &DeviceInfo.CmdQueues[DeviceId]);
  DeviceInfo.Initialized[DeviceId] = true;

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
  CALL_ZE_RET_NULL(zeModuleCreate, DeviceInfo.Devices[DeviceId], &moduleDesc,
                   &module, nullptr /* build log */);

  auto &mutex = DeviceInfo.Mutexes[DeviceId];
  auto &entries = DeviceInfo.FuncGblEntries[DeviceId].Entries;
  auto &kernels = DeviceInfo.FuncGblEntries[DeviceId].Kernels;
  entries.resize(numEntries);
  kernels.resize(numEntries);

  for (uint32_t i = 0; i < numEntries; i++) {
    auto size = Image->EntriesBegin[i].size;

    if (size != 0) {
      // Entry is a global variable
      void *tgtAddr = nullptr;
      auto hstAddr = Image->EntriesBegin[i].addr;
      auto name = Image->EntriesBegin[i].name;
      // FIXME: unsupported in v0.2.2
      //CALL_ZE_RET_NULL(zeModuleGetGlobalPointer, module, name, &tgtAddr);
      if (!tgtAddr) {
        tgtAddr = __tgt_rtl_data_alloc(DeviceId, size, hstAddr);
        __tgt_rtl_data_submit(DeviceId, tgtAddr, hstAddr, size);
        mutex.lock();
        DeviceInfo.OwnedMemory[DeviceId].push_back(tgtAddr);
        mutex.unlock();
      }
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
#ifdef OMPTARGET_LEVEL0_DEBUG
    // TODO: show kernel information
#endif
  }

  DeviceInfo.FuncGblEntries[DeviceId].Module = module;
  __tgt_target_table &table = DeviceInfo.FuncGblEntries[DeviceId].Table;
  table.EntriesBegin = &(entries.data()[0]);
  table.EntriesEnd = &(entries.data()[entries.size()]);
  return &table;
}

static void *allocData(int32_t DeviceId, int64_t Size, void *HstPtr,
                       void *HstBase, int32_t IsImplicitArg) {
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
  CALL_ZE_RET_NULL(zeDriverAllocDeviceMem, DeviceInfo.Driver, &allocDesc,
                   size, LEVEL0_ALIGNMENT, DeviceInfo.Devices[DeviceId], &base);
  void *mem = (void *)((intptr_t)base + offset);

#ifdef OMPTARGET_LEVEL0_DEBUG
  void *actualBase = nullptr;
  size_t actualSize = 0;
  CALL_ZE_RET_NULL(zeDriverGetMemAddressRange, DeviceInfo.Driver, mem,
                   &actualBase, &actualSize);
  assert(base == actualBase && "Invalid memory address range!");
  DP("Allocated device memory " DPxMOD " (Base: " DPxMOD
     ", Size: %zu) for host ptr " DPxMOD "\n", DPxPTR(mem), DPxPTR(actualBase),
     actualSize, DPxPTR(HstPtr));
#endif

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

// Template for synchronous command execution.
static int32_t executeCommand(ze_command_list_handle_t CmdList,
                              ze_command_queue_handle_t CmdQueue,
                              std::mutex &Mutex) {
  CALL_ZE_RET_FAIL_MTX(zeCommandListClose, Mutex, CmdList);
  CALL_ZE_RET_FAIL(zeCommandQueueExecuteCommandLists, CmdQueue, 1, &CmdList,
                   nullptr /* fence for completion signaling */);
  CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, CmdQueue, UINT32_MAX);
  // Make sure the command list is ready to accept next command
  CALL_ZE_RET_FAIL_MTX(zeCommandListReset, Mutex, CmdList);
  return OFFLOAD_SUCCESS;
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
  if (zeFenceDestroy(Fence) != ZE_RESULT_SUCCESS ||
      zeCommandListDestroy(CmdList) != ZE_RESULT_SUCCESS)
    FATAL_ERROR("Failed to finalize asynchronous command\n");
}

// Template for Asynchronous command execution.
// We use a dedicated command list and a fence to invoke an asynchronous task.
// A separate detached thread submits commands to the queue, waits until the
// attached fence is signaled, and then invokes the clean-up routine.
static int32_t beginAsyncCommand(ze_command_list_handle_t CmdList,
                                 ze_command_queue_handle_t CmdQueue,
                                 std::mutex &Mutex, AsyncEventTy *Event,
                                 ze_fence_handle_t Fence) {
  if (!Event || !Event->Handler || !Event->Arg) {
    DP("Error: Failed to start asynchronous command -- invalid argument\n");
    return OFFLOAD_FAIL;
  }

  CALL_ZE_RET_FAIL_MTX(zeCommandListClose, Mutex, CmdList);

  // Spawn waiting thread
  std::thread waiter([](AsyncEventTy *event, ze_command_list_handle_t cmdList,
                        ze_fence_handle_t fence) {
    // Wait until the fence is signaled.
    zeFenceHostSynchronize(fence, UINT32_MAX);
    // Invoke clean-up routine
    endAsyncCommand(event, cmdList, fence);
  }, Event, CmdList, Fence);

  waiter.detach();

  // Fence to be signaled on command-list completion.
  CALL_ZE_RET_FAIL(zeCommandQueueExecuteCommandLists, CmdQueue, 1, &CmdList,
                   Fence);

  return OFFLOAD_SUCCESS;
}

// Create a fence
static ze_fence_handle_t createFence(int32_t DeviceId) {
  ze_fence_desc_t fenceDesc = {
    ZE_FENCE_DESC_VERSION_CURRENT,
    ZE_FENCE_FLAG_NONE
  };
  auto cmdQueue = DeviceInfo.CmdQueues[DeviceId];
  ze_fence_handle_t fence;
  CALL_ZE_RET(0, zeFenceCreate, cmdQueue, &fenceDesc, &fence);
  return fence;
}

// Create a command list
static ze_command_list_handle_t createCmdList(int32_t DeviceId) {
  ze_command_list_desc_t cmdListDesc = {
    ZE_COMMAND_LIST_DESC_VERSION_CURRENT,
    ZE_COMMAND_LIST_FLAG_EXPLICIT_ONLY
  };
  ze_command_list_handle_t cmdList;
  auto deviceHandle = DeviceInfo.Devices[DeviceId];
  CALL_ZE_RET(0, zeCommandListCreate, deviceHandle, &cmdListDesc, &cmdList);
  return cmdList;
}

static int32_t submitData(int32_t DeviceId, void *TgtPtr, void *HstPtr,
                          int64_t Size, void *AsyncEvent) {
  if (Size == 0)
    return OFFLOAD_SUCCESS;

  // Add synthetic delay for experiments
  addDataTransferLatency();

  auto cmdList = DeviceInfo.CmdLists[DeviceId];
  auto cmdQueue = DeviceInfo.CmdQueues[DeviceId];
  auto &mutex = DeviceInfo.Mutexes[DeviceId];

  if (AsyncEvent) {
    cmdList = createCmdList(DeviceId);
    if (!cmdList) {
      DP("Error: Asynchronous data submit failed -- invalid command list\n");
      return OFFLOAD_FAIL;
    }
    CALL_ZE_RET_FAIL_MTX(zeCommandListAppendMemoryCopy, mutex, cmdList, TgtPtr,
                         HstPtr, Size, nullptr);
    auto fence = createFence(DeviceId);
    if (!fence) {
      DP("Error: Asynchronous data submit failed -- invalid fence\n");
      return OFFLOAD_FAIL;
    }
    if (beginAsyncCommand(cmdList, cmdQueue, mutex,
        static_cast<AsyncEventTy *>(AsyncEvent), fence) == OFFLOAD_FAIL)
      return OFFLOAD_FAIL;
    DP("Asynchronous data submit started -- %" PRId64 " bytes (hst:"
       DPxMOD ") -> (tgt:" DPxMOD ")\n", Size, DPxPTR(HstPtr), DPxPTR(TgtPtr));
  } else {
    CALL_ZE_RET_FAIL_MTX(zeCommandListAppendMemoryCopy, mutex, cmdList, TgtPtr,
                         HstPtr, Size, nullptr /* event handle */);
    CALL_ZE_RET_FAIL_MTX(zeCommandListAppendBarrier, mutex, cmdList, nullptr, 0,
                         nullptr);
    if (executeCommand(cmdList, cmdQueue, mutex) == OFFLOAD_FAIL)
      return OFFLOAD_FAIL;
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
int32_t __tgt_rtl_data_submit_nowait(int32_t DeviceId, void *TgtPtr,
                                     void *HstPtr, int64_t Size,
                                     void *AsyncEvent) {
  return submitData(DeviceId, TgtPtr, HstPtr, Size, AsyncEvent);
}

static int32_t retrieveData(int32_t DeviceId, void *HstPtr, void *TgtPtr,
                            int64_t Size, void *AsyncEvent) {
  if (Size == 0)
    return OFFLOAD_SUCCESS;

  // Add synthetic delay for experiments
  addDataTransferLatency();

  auto cmdList = DeviceInfo.CmdLists[DeviceId];
  auto cmdQueue = DeviceInfo.CmdQueues[DeviceId];
  auto &mutex = DeviceInfo.Mutexes[DeviceId];

  if (AsyncEvent) {
    cmdList = createCmdList(DeviceId);
    if (!cmdList) {
      DP("Error: Asynchronous data retrieve failed -- invalid command list\n");
      return OFFLOAD_FAIL;
    }
    CALL_ZE_RET_FAIL_MTX(zeCommandListAppendMemoryCopy, mutex, cmdList, HstPtr,
                         TgtPtr, Size, nullptr);
    auto fence = createFence(DeviceId);
    if (!fence) {
      DP("Error: Asynchronous data retrieve failed -- invalid fence\n");
      return OFFLOAD_FAIL;
    }
    if (beginAsyncCommand(cmdList, cmdQueue, mutex,
        static_cast<AsyncEventTy *>(AsyncEvent), fence) == OFFLOAD_FAIL)
      return OFFLOAD_FAIL;
    DP("Asynchronous data retrieve started -- %" PRId64 " bytes (tgt:"
       DPxMOD ") -> (hst:" DPxMOD ")\n", Size, DPxPTR(TgtPtr), DPxPTR(HstPtr));
  } else {
    CALL_ZE_RET_FAIL_MTX(zeCommandListAppendMemoryCopy, mutex, cmdList, HstPtr,
                         TgtPtr, Size, nullptr /* event handle */);
    CALL_ZE_RET_FAIL_MTX(zeCommandListAppendBarrier, mutex, cmdList, nullptr, 0,
                         nullptr);
    if (executeCommand(cmdList, cmdQueue, mutex) == OFFLOAD_FAIL)
      return OFFLOAD_FAIL;
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
  assert((TgtEntryPtr && TgtArgs && TgtOffsets) && "Invalid kernel");
  assert((NumTeams >= 0 && ThreadLimit >= 0) && "Invalid kernel work size");
  DP("Executing a kernel " DPxMOD "...\n", DPxPTR(TgtEntryPtr));

  ze_kernel_handle_t kernel = *((ze_kernel_handle_t *)TgtEntryPtr);
  auto &mutex = DeviceInfo.Mutexes[DeviceId];

  // Set arguments
  std::vector<void *> args(NumArgs);
  for (int32_t i = 0; i < NumArgs; i++) {
    args[i] = (void *)((intptr_t)TgtArgs[i] + TgtOffsets[i]);
    CALL_ZE_RET_FAIL_MTX(zeKernelSetArgumentValue, mutex, kernel, i,
                         sizeof(void *), &args[i]);
    DP("Kernel argument %" PRId32 " (value: " DPxMOD ") was set successfully\n",
       i, DPxPTR(args[i]));
  }
  // TODO: implicit arguments -- zeKernelSetAttribute() ?

  // Decide group sizes and dimensions
  uint32_t groupSizes[3];
  ze_group_count_t groupDimensions;
  decideGroupArguments((uint32_t )NumTeams, (uint32_t)ThreadLimit,
                       (int64_t *)LoopDesc, groupSizes, groupDimensions);

  CALL_ZE_RET_FAIL_MTX(zeKernelSetGroupSize, mutex, kernel, groupSizes[0],
                       groupSizes[1], groupSizes[2]);
  auto cmdList = DeviceInfo.CmdLists[DeviceId];
  auto cmdQueue = DeviceInfo.CmdQueues[DeviceId];

  if (AsyncEvent) {
    cmdList = createCmdList(DeviceId);
    if (!cmdList) {
      DP("Error: Asynchronous execution failed -- invalid command list\n");
      return OFFLOAD_FAIL;
    }
    CALL_ZE_RET_FAIL_MTX(zeCommandListAppendLaunchKernel, mutex, cmdList,
                         kernel, &groupDimensions, nullptr, 0, nullptr);
    auto fence = createFence(DeviceId);
    if (!fence) {
      DP("Error: Asynchronous execution failed -- invalid fence\n");
      return OFFLOAD_FAIL;
    }
    if (beginAsyncCommand(cmdList, cmdQueue, mutex,
        static_cast<AsyncEventTy *>(AsyncEvent), fence) == OFFLOAD_FAIL)
      return OFFLOAD_FAIL;
    DP("Asynchronous execution started for kernel " DPxMOD "\n",
       DPxPTR(TgtEntryPtr));
  } else {
    CALL_ZE_RET_FAIL_MTX(zeCommandListAppendLaunchKernel, mutex, cmdList,
                         kernel, &groupDimensions, nullptr, 0, nullptr);
    CALL_ZE_RET_FAIL_MTX(zeCommandListAppendBarrier, mutex, cmdList, nullptr, 0,
                         nullptr);
    if (executeCommand(cmdList, cmdQueue, mutex) == OFFLOAD_FAIL)
      return OFFLOAD_FAIL;
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
#endif // INTEL_CUSTOMIZATION

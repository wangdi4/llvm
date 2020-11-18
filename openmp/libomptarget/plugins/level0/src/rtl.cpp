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
#include <set>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include <ze_api.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#include <dlfcn.h>
#endif // !_WIN32

#include "omptargetplugin.h"
#include "omptarget-tools.h"
#include "rtl-trace.h"

#define TARGET_NAME LEVEL0
#define DEBUG_PREFIX "Target " GETNAME(TARGET_NAME) " RTL"

/// Host runtime routines being used
extern "C" {
#ifdef _WIN32
int __cdecl omp_get_max_teams(void);
int __cdecl omp_get_thread_limit(void);
double __cdecl omp_get_wtime(void);
int __cdecl omp_get_max_threads(void);
int __cdecl __kmpc_global_thread_num(void *);
#else
int omp_get_max_teams(void) __attribute__((weak));
int omp_get_thread_limit(void) __attribute__((weak));
double omp_get_wtime(void) __attribute__((weak));
int omp_get_max_threads(void) __attribute__((weak));
int __kmpc_global_thread_num(void *) __attribute__((weak));
#endif
} // extern "C"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define ABS(x) ((x) < 0 ? -(x) : (x))

// Parameters
#define LEVEL0_ALIGNMENT 0 // Default alignmnet for allocation
#define LEVEL0_ND_GROUP_SIZE 16 // Default group size for ND partitioning
#define LEVEL0_MAX_GROUP_COUNT 64 // TODO: get it from HW
#define LEVEL0_PAGE_SIZE (1 << 16) // L0 memory allocation unit

// Subdevice utilities
// Device encoding (MSB=63, LSB=0)
// 63..63: Has subdevice
// 62..58: Reserved
// 57..56: Subdevice level
// 55..48: Subdevice ID start
// 47..40: Subdevice ID count
// 39..32: Subdevice ID stride
// 31..00: Device ID
#define SUBDEVICE_GET_LEVEL(ID) ((uint32_t)EXTRACT_BITS(ID, 57, 56))
#define SUBDEVICE_GET_START(ID) ((uint32_t)EXTRACT_BITS(ID, 55, 48))
#define SUBDEVICE_GET_COUNT(ID) ((uint32_t)EXTRACT_BITS(ID, 47, 40))
#define SUBDEVICE_GET_STRIDE(ID) ((uint32_t)EXTRACT_BITS(ID, 39, 32))
#define SUBDEVICE_GET_ROOT(ID) ((uint32_t)EXTRACT_BITS(ID, 31, 0))

// Subdevice options
#ifndef SUBDEVICE_USE_ROOT_KERNELS
#define SUBDEVICE_USE_ROOT_KERNELS 1
#endif
#ifndef SUBDEVICE_USE_ROOT_MEMORY
#define SUBDEVICE_USE_ROOT_MEMORY 0
#endif

int DebugLevel = 0;

#if INTEL_INTERNAL_BUILD
/// Memory stats
struct MemStatTy {
  size_t Requested = 0; // Requested bytes
  size_t Allocated = 0; // Allocated bytes by L0
  size_t Freed = 0; // Freed bytes by L0
  size_t InUse = 0; // Current memory in use by L0
  size_t PeakUse = 0; // Peak bytes in use by L0
  void update(ze_context_handle_t Context, size_t requested, void *Mem) {
    void *base = nullptr;
    size_t size = 0;
    CALL_ZE_EXIT_FAIL(zeMemGetAddressRange, Context, Mem, &base, &size);
    if (requested > 0) {
      Requested += requested;
      Allocated += size;
      InUse += size;
    } else {
      Freed += size;
      InUse -= size;
    }
    if (InUse > PeakUse)
      PeakUse = InUse;
  }
  void print() {
    IDP("Memory usage:\n");
    IDP("-- Requested = %12zu\n", Requested);
    IDP("-- Allocated = %12zu\n", Allocated);
    IDP("-- Freed     = %12zu\n", Freed);
    IDP("-- InUse     = %12zu\n", InUse);
    IDP("-- PeakUse   = %12zu\n", PeakUse);
  }
}; // MemStatTy
// Per-device memory stats
static std::vector<MemStatTy> MemStats;
#define MEMSTAT_UPDATE(DeviceId, Context, Requested, Mem)                      \
  do {                                                                         \
    if (DebugLevel > 0)                                                        \
      MemStats[DeviceId].update(Context, Requested, Mem);                      \
  } while (0)
#define MEMSTAT_PRINT(DeviceId)                                                \
  do {                                                                         \
    if (DebugLevel > 0)                                                        \
      MemStats[DeviceId].print();                                              \
  } while (0)
#else // INTEL_INTERNAL_BUILD
#define MEMSTAT_UPDATE(DeviceId, Context, Requested, Mem)
#define MEMSTAT_PRINT(DeviceId)
#endif // INTEL_INTERNAL_BUILD

static void *allocDataExplicit(int32_t DeviceId, int64_t Size, int32_t Kind);

///
/// Page pool for small memory allocation.
/// It maintains buckets of page list for each supported memory size in the
/// specified range [MinSize, MaxSize].
///
class PagePoolTy {
  static const uint32_t MinSize = 5; // 32B (1 << 5)
  static const uint32_t MaxSize = 12; // 4KB (1 << 12)
  // FIXME: MaxSize >= 13 has consistency issue for some reason.

  /// Page split into small chunks for reuse.
  struct ChunkedPageTy {
    uint32_t ChunkSize = 0;
    uint32_t NumSlots = 0;
    uint32_t NumUsedSlots = 0;
    std::vector<bool> UsedSlots;
    uintptr_t Base = 0;

    ChunkedPageTy(uint32_t chunkSize, void *base) {
      ChunkSize = chunkSize;
      NumSlots = LEVEL0_PAGE_SIZE / ChunkSize;
      NumUsedSlots = 0;
      UsedSlots.resize(NumSlots, false);
      Base = (uintptr_t)base;
    }

    bool isFull() { return NumUsedSlots == NumSlots; }

    bool contains(void *Mem) {
      uintptr_t mem = (uintptr_t)Mem;
      return mem >= Base && mem < Base + LEVEL0_PAGE_SIZE;
    }

    void *allocate() {
      if (isFull())
        return nullptr;
      for (uint32_t i = 0; i < NumSlots; i++) {
        if (UsedSlots[i])
          continue;
        UsedSlots[i] = true;
        NumUsedSlots++;
        return (void *)(Base + i * ChunkSize);
      }
      // Should not reach here.
      FATAL_ERROR("Inconsistent page found while allocating from pool");
    }

    void deallocate(void *Mem) {
      if (!contains(Mem))
        FATAL_ERROR("Invalid memory while deallocating to pool");
      uint32_t slot = ((uintptr_t)Mem - Base) / ChunkSize;
      UsedSlots[slot] = false;
      NumUsedSlots--;
    }
  }; // ChunkedPageTy

  std::vector<std::vector<ChunkedPageTy>> Buckets;
  int32_t DeviceId = 0;
  int32_t TargetAllocKind = TARGET_ALLOC_SHARED;
  ze_context_handle_t Context = nullptr;
  ze_device_handle_t Device = nullptr;

  uint32_t getBucketId(size_t Size) {
    uint32_t i;
    for (i = 0; i <= MaxSize - MinSize; i++) {
      if (1ULL << (i + MinSize) < Size)
        continue;
      break;
    }
    return i;
  }

public:
  /// Initialize a page pool for the given device.
  /// Multiple threads cannot call this simultaneously.
  void initialize(int32_t deviceId, ze_context_handle_t context,
                  ze_device_handle_t device, int32_t allocKind) {
    Buckets.resize(MaxSize - MinSize + 1);
    DeviceId = deviceId;
    Context = context;
    Device = device;
    TargetAllocKind = allocKind;
  }

  /// Return supported max size in bytes.
  size_t getMaxSize() { return 1U << MaxSize; }

  /// Allocate from the existing buckets or call L0 allocator.
  /// Multiple threads cannot call this simultaneously.
  void *allocate(size_t Size) {
    uint32_t bucketId = getBucketId(Size);
    if (bucketId > MaxSize - MinSize)
      return nullptr; // Size is too large for the allocator
    for (auto &page : Buckets[bucketId]) {
      if (page.isFull())
        continue;
      void *mem = page.allocate();
      if (!mem)
        FATAL_ERROR("Invalid memory while allocating from pool");
      return mem;
    }
    // Bucket is empty or all pages in the bucket are full
    void *base = allocDataExplicit(DeviceId, LEVEL0_PAGE_SIZE, TargetAllocKind);
    IDP(
       "New allocation from page pool: base = " DPxMOD ", size = %" PRIu32 "\n",
       DPxPTR(base), LEVEL0_PAGE_SIZE);
    Buckets[bucketId].emplace_back(1U << (bucketId + MinSize), base);
    return Buckets[bucketId].back().allocate();
  }

  /// Deallocate the memory and return true if successful.
  /// Multiple threads cannot call this simultaneously.
  bool deallocate(void *Mem) {
    for (uint32_t i = 0; i <= MaxSize - MinSize; i++)
      for (auto &page : Buckets[i])
        if (page.contains(Mem)) {
          page.deallocate(Mem);
          return true;
        }
    return false;
  }

  /// Release all pages in the pool.
  /// Multiple threads cannot call this simultaneously.
  void clear() {
    for (uint32_t i = 0; i <= MaxSize - MinSize; i++) {
      for (auto &page : Buckets[i]) {
        MEMSTAT_UPDATE(DeviceId, Context, 0, (void *)page.Base);
        CALL_ZE_EXIT_FAIL(zeMemFree, Context, (void *)page.Base);
      }
    }
  }
}; // PagePoolTy

/// Per-device global entry table
struct FuncOrGblEntryTy {
  __tgt_target_table Table;
  std::vector<__tgt_offload_entry> Entries;
  std::vector<ze_kernel_handle_t> Kernels;
  std::vector<ze_module_handle_t> Modules;
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
  uint64_t TimestampFreq = 0;
  uint64_t TimestampMax = 0;
public:
  static const int64_t MSEC_PER_SEC = 1000;
  static const int64_t USEC_PER_SEC = 1000000;
  static const int64_t NSEC_PER_SEC = 1000000000;
  static int64_t Multiplier;

  RTLProfileTy(const ze_device_properties_t &DeviceProperties) {
    ThreadId = std::this_thread::get_id();
    TimestampFreq = DeviceProperties.timerResolution;
    auto validBits = DeviceProperties.kernelTimestampValidBits;
    if (validBits > 0 && validBits < 64)
      TimestampMax = ~(-1ULL << validBits);
    else
      WARNING("Invalid kernel timestamp bit width (%" PRIu32 "). "
              "Long-running kernels may report incorrect device time.\n",
              validBits);
  }

  void printData(const char *DeviceId, const char *DeviceName) {
    std::ostringstream o;
    o << ThreadId;
    fprintf(stderr, "LIBOMPTARGET_PROFILE for OMP DEVICE(%s) %s"
            ", Thread %s\n", DeviceId, DeviceName, o.str().c_str());
    const char *unit = (Multiplier == MSEC_PER_SEC) ? "msec" : "usec";
    fprintf(stderr, "-- Name: Host Time (%s), Device Time (%s)\n", unit, unit);
    double hostTotal = 0.0;
    double deviceTotal = 0.0;
    for (const auto &d : Data) {
      double hostTime = d.second.HostTime * Multiplier;
      double deviceTime = (d.first.find("Kernel#") == std::string::npos)
                              ? hostTime // device time is not available
                              : d.second.DeviceTime * Multiplier;
      fprintf(stderr, "-- %s: %.6f, %.6f\n", d.first.c_str(), hostTime,
              deviceTime);
      hostTotal += hostTime;
      deviceTotal += deviceTime;
    }
    fprintf(stderr, "-- Total: %.6f, %.6f\n", hostTotal, deviceTotal);
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

  void update(std::string &Name, ze_event_handle_t Event) {
    TimeTy &time = Data[Name];
    ze_kernel_timestamp_result_t ts;
    CALL_ZE_EXIT_FAIL(zeEventQueryKernelTimestamp, Event, &ts);
    double wallTime = 0;
    if (ts.global.kernelEnd >= ts.global.kernelStart)
      wallTime = ts.global.kernelEnd - ts.global.kernelStart;
    else if (TimestampMax > 0)
      wallTime = TimestampMax - ts.global.kernelStart + ts.global.kernelEnd + 1;
    else
      WARNING("Timestamp overflow cannot be handled for this device.\n");
    time.DeviceTime += wallTime * (double)TimestampFreq / NSEC_PER_SEC;
    CALL_ZE_EXIT_FAIL(zeEventHostReset, Event);
  }
};
int64_t RTLProfileTy::Multiplier;

/// Handles/data to be created for each threads
struct PrivateHandlesTy {
  ze_command_list_handle_t CmdList = nullptr;
  ze_command_queue_handle_t CmdQueue = nullptr;
  RTLProfileTy *Profile = nullptr;
};

/// Each thread should be able to handle multiple devices
thread_local std::map<int32_t, PrivateHandlesTy> ThreadLocalHandles;

/// Per-thread subdeivce encoding
thread_local int64_t SubDeviceCode = 0;

/// Get default command queue group ordinal
static uint32_t getCmdQueueGroupOrdinal(ze_device_handle_t Device) {
  uint32_t groupCount = 0;
  CALL_ZE_RET(UINT32_MAX, zeDeviceGetCommandQueueGroupProperties, Device,
              &groupCount, nullptr);
  std::vector<ze_command_queue_group_properties_t> groupProperties(groupCount);
  CALL_ZE_RET(UINT32_MAX, zeDeviceGetCommandQueueGroupProperties, Device,
              &groupCount, groupProperties.data());
  uint32_t groupOrdinal = groupCount;
  for (uint32_t i = 0; i < groupCount; i++) {
    auto &flags = groupProperties[i].flags;
    if (flags & ZE_COMMAND_QUEUE_GROUP_PROPERTY_FLAG_COMPUTE) {
      groupOrdinal = i;
      break;
    }
  }
  if (groupOrdinal == groupCount) {
    IDP("Error: no command queues are found\n");
    return UINT32_MAX;
  }
  return groupOrdinal;
}

/// Create a command list with given ordinal and flags
static ze_command_list_handle_t createCmdList(
    ze_context_handle_t Context,
    ze_device_handle_t Device,
    uint32_t Ordinal,
    ze_command_list_flags_t Flags,
    std::string &DeviceIdStr) {
  ze_command_list_desc_t cmdListDesc = {
    ZE_STRUCTURE_TYPE_COMMAND_LIST_DESC,
    nullptr, // extension
    Ordinal,
    Flags
  };
  ze_command_list_handle_t cmdList;
  CALL_ZE_RET_NULL(zeCommandListCreate, Context, Device, &cmdListDesc,
                   &cmdList);
  IDP("Created a command list " DPxMOD " for device %s.\n", DPxPTR(cmdList),
      DeviceIdStr.c_str());
  return cmdList;
}

/// Create a command list with default ordinal and flags
static ze_command_list_handle_t createCmdList(
    ze_context_handle_t Context, ze_device_handle_t Device,
    std::string &DeviceIdStr) {
  uint32_t ordinal = getCmdQueueGroupOrdinal(Device);
  return (ordinal == UINT32_MAX)
      ? nullptr
      : createCmdList(Context, Device, ordinal, 0, DeviceIdStr);
}

/// Create a command queue with given ordinal and flags
static ze_command_queue_handle_t createCmdQueue(
    ze_context_handle_t Context,
    ze_device_handle_t Device,
    uint32_t Ordinal,
    ze_command_queue_flags_t Flags,
    std::string &DeviceIdStr) {
  ze_command_queue_desc_t cmdQueueDesc = {
    ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC,
    nullptr, // extension
    Ordinal,
    0, // index: TODO: when can we use non-zero index?
    Flags, // flags
    ZE_COMMAND_QUEUE_MODE_ASYNCHRONOUS,
    ZE_COMMAND_QUEUE_PRIORITY_NORMAL
  };
  ze_command_queue_handle_t cmdQueue;
  CALL_ZE_RET_NULL(zeCommandQueueCreate, Context, Device, &cmdQueueDesc,
                   &cmdQueue);
  IDP("Created a command queue " DPxMOD " for device %s.\n", DPxPTR(cmdQueue),
      DeviceIdStr.c_str());
  return cmdQueue;
}

/// Create a command queue with default ordinal and flags
static ze_command_queue_handle_t createCmdQueue(
    ze_context_handle_t Context, ze_device_handle_t Device,
    std::string &DeviceIdStr) {
  uint32_t ordinal = getCmdQueueGroupOrdinal(Device);
  return (ordinal == UINT32_MAX)
      ? nullptr
      : createCmdQueue(Context, Device, ordinal, 0, DeviceIdStr);
}

/// Create a context
static ze_context_handle_t createContext(ze_driver_handle_t Driver) {
  ze_context_desc_t contextDesc = {
    ZE_STRUCTURE_TYPE_CONTEXT_DESC,
    nullptr, // extension
    0 // flags
  };
  ze_context_handle_t context;
  CALL_ZE_RET_NULL(zeContextCreate, Driver, &contextDesc, &context);
  return context;
}

/// Create a fence
static ze_fence_handle_t createFence(ze_command_queue_handle_t cmdQueue) {
  ze_fence_desc_t fenceDesc = {
    ZE_STRUCTURE_TYPE_FENCE_DESC,
    nullptr, // extension
    0 // flags
  };
  ze_fence_handle_t fence;
  CALL_ZE_RET(0, zeFenceCreate, cmdQueue, &fenceDesc, &fence);
  return fence;
}

/// Create a module from SPIR-V
static ze_module_handle_t createModule(
    ze_context_handle_t Context, ze_device_handle_t Device, size_t Size,
    const uint8_t *Image, const char *Flags) {
  ze_module_desc_t moduleDesc = {
    ZE_STRUCTURE_TYPE_MODULE_DESC,
    nullptr, // extension
    ZE_MODULE_FORMAT_IL_SPIRV,
    Size,
    Image,
    Flags,
    nullptr // pointer to specialization constants
  };
  ze_module_handle_t module;
  ze_module_build_log_handle_t buildLog;
  ze_result_t rc;
  CALL_ZE_RC(rc, zeModuleCreate, Context, Device, &moduleDesc, &module,
             &buildLog);
  if (rc != ZE_RESULT_SUCCESS) {
    if (DebugLevel > 0) {
      size_t logSize;
      CALL_ZE_RET_NULL(zeModuleBuildLogGetString, buildLog, &logSize, nullptr);
      std::vector<char> logString(logSize);
      CALL_ZE_RET_NULL(zeModuleBuildLogGetString, buildLog, &logSize,
                       logString.data());
      IDP("Error: module creation failed -- see below for details.\n");
      fprintf(stderr, "%s\n", logString.data());
    }
    CALL_ZE_RET_NULL(zeModuleBuildLogDestroy, buildLog);
    return nullptr;
  }
  CALL_ZE_RET_NULL(zeModuleBuildLogDestroy, buildLog);
  return module;
}

/// Create a module from a file name
static ze_module_handle_t createModule(
    ze_context_handle_t Context, ze_device_handle_t Device,
    const char *FileName, const char *Flags) {
  // Resolve full path using the location of the plugin
  std::string fullPath;
#ifdef _WIN32
  char rtlPath[_MAX_PATH];
  HMODULE rtlModule = nullptr;
  if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                          GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR) &DebugLevel, &rtlModule)) {
    IDP("Error: module creation failed -- cannot resolve full path\n");
    return nullptr;
  }
  if (!GetModuleFileNameA(rtlModule, rtlPath, sizeof(rtlPath))) {
    IDP("Error: module creation failed -- cannot resolve full path\n");
    return nullptr;
  }
  fullPath = rtlPath;
#else // !defined(_WIN32)
  Dl_info rtlInfo;
  if (!dladdr(&DebugLevel, &rtlInfo)) {
    IDP("Error: module creation failed -- cannot resolve full path\n");
    return nullptr;
  }
  fullPath = rtlInfo.dli_fname;
#endif // !defined(_WIN32)
  size_t split = fullPath.find_last_of("/\\");
  fullPath.replace(split + 1, std::string::npos, FileName);

  // Now read from the full path
  std::ifstream ifs(fullPath.c_str(), std::ios::binary);
  // Ignore files that are not supported.
  if (!ifs.good())
    return nullptr;
  ifs.seekg(0, ifs.end);
  size_t size = static_cast<size_t>(ifs.tellg());
  ifs.seekg(0);
  std::vector<char> image(size);
  if (!ifs.read(image.data(), size)) {
    IDP("Error: module creation failed -- cannot read %s\n", fullPath.c_str());
    return nullptr;
  }
  return createModule(Context, Device, size, (uint8_t *)image.data(), Flags);
}

/// RTL flags
struct RTLFlagsTy {
  uint64_t DumpTargetImage : 1;
  uint64_t EnableProfile : 1;
  uint64_t EnableTargetGlobals : 1;
  uint64_t LinkLibDevice : 1;
  uint64_t UseHostMemForUSM : 1;
  uint64_t UseMemoryPool : 1;
  uint64_t UseDriverGroupSizes : 1;
  uint64_t Reserved : 57;
  RTLFlagsTy() :
      DumpTargetImage(0),
      EnableProfile(0),
      EnableTargetGlobals(0),
      LinkLibDevice(0), // TODO: change it to 1 when L0 issue is resolved
      UseHostMemForUSM(0),
      UseMemoryPool(0),
      UseDriverGroupSizes(0),
      Reserved(0) {}
};

/// Kernel properties.
struct KernelPropertiesTy {
  uint32_t Width = 0;
  uint32_t MaxThreadGroupSize = 0;
  bool RequiresIndirectAccess = false;
};

/// Events for kernel profiling
struct KernelProfileEventsTy {
  ze_event_pool_handle_t Pool = nullptr;
  size_t Size = 0;
  std::map<int32_t, ze_event_handle_t> Events;  // Per-thread events
  std::mutex *EventLock = nullptr;

  void init(ze_context_handle_t Context) {
    Size = omp_get_max_threads();
    ze_event_pool_desc_t poolDesc = {
      ZE_STRUCTURE_TYPE_EVENT_POOL_DESC,
      nullptr,
      ZE_EVENT_POOL_FLAG_KERNEL_TIMESTAMP,
      (uint32_t)Size
    };
    CALL_ZE_EXIT_FAIL(zeEventPoolCreate, Context, &poolDesc, 0, nullptr, &Pool);
    EventLock = new std::mutex();
  }

  void deinit() {
    for (auto event : Events)
      CALL_ZE_EXIT_FAIL(zeEventDestroy, event.second);
    CALL_ZE_EXIT_FAIL(zeEventPoolDestroy, Pool);
    delete EventLock;
  }

  ze_event_handle_t getEvent() {
    int32_t gtid = __kmpc_global_thread_num(nullptr);
    std::unique_lock<std::mutex> lock(*EventLock);
    if (Events.count(gtid) == 0) {
      if (Events.size() == Size) {
        IDP("Cannot create profiling events more than %zu.\n", Size);
        return nullptr;
      }
      ze_event_desc_t eventDesc = {
        ZE_STRUCTURE_TYPE_EVENT_DESC,
        nullptr,
        (uint32_t)Events.size(), // index
        0,
        0
      };
      ze_event_handle_t event;
      CALL_ZE_RET_NULL(zeEventCreate, Pool, &eventDesc, &event);
      Events[gtid] = event;
    }
    return Events[gtid];
  }
};

/// Subdevice events
struct SubDeviceEventTy {
  ze_event_pool_handle_t Pool = nullptr;
  std::vector<ze_event_handle_t> Events;
};

typedef std::vector<std::vector<ze_device_handle_t>> SubDeviceListsTy;
typedef std::vector<std::vector<int32_t>> SubDeviceIdsTy;

/// Device modes for multi-tile devices
enum DeviceMode {
  DEVICE_MODE_TOP = 0,  // Use only top-level devices with subdevice clause
  DEVICE_MODE_SUB,      // Use only tiles
  DEVICE_MODE_SUBSUB,   // Use only c-slices
  DEVICE_MODE_ALL       // Use all
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
  uint32_t NumDevices = 0;
  uint32_t SubDeviceLevels = 2;
  uint32_t NumRootDevices = 0;
  ze_driver_handle_t Driver = nullptr;
  ze_context_handle_t Context = nullptr;
  // Events for kernel profiling
  KernelProfileEventsTy ProfileEvents;
  std::vector<ze_device_properties_t> DeviceProperties;
  std::vector<ze_device_compute_properties_t> ComputeProperties;
  std::vector<ze_device_handle_t> Devices;
  // Subdevice IDs. It maps users' subdevice IDs to internal subdevice IDs
  std::vector<SubDeviceIdsTy> SubDeviceIds;
  // Events for synchronization between subdevices.
  std::vector<SubDeviceEventTy> SubDeviceEvents;
  // User-friendly form of device ID string
  std::vector<std::string> DeviceIdStr;
  // Use per-thread command list/queue
  std::vector<std::vector<ze_command_list_handle_t>> CmdLists;
  std::vector<std::vector<ze_command_queue_handle_t>> CmdQueues;
  std::vector<FuncOrGblEntryTy> FuncGblEntries;
  std::vector<std::map<ze_kernel_handle_t, KernelPropertiesTy>>
      KernelProperties;
  std::vector<std::vector<void *>> OwnedMemory; // Memory owned by the plugin
  std::vector<std::set<void *>> ImplicitArgsDevice;
  std::vector<std::set<void *>> ImplicitArgsHost;
  std::vector<std::set<void *>> ImplicitArgsShared;
  std::vector<bool> Initialized;
  std::mutex *Mutexes;
  std::mutex *DataMutexes; // For internal data
  std::vector<std::vector<DeviceOffloadEntryTy>> OffloadTables;
  std::vector<std::vector<RTLProfileTy *>> Profiles;
  std::vector<PagePoolTy> PagePools; // Internal memory pool

  /// Flags, parameters, options
  RTLFlagsTy Flags;
  int64_t RequiresFlags = OMP_REQ_UNDEFINED;
  uint32_t DataTransferLatency; // Emulated data transfer latency in us
  int32_t DeviceType;
  uint32_t ThreadLimit = 0; // Global thread limit
  uint32_t NumTeams = 0; // Global max number of teams
  int32_t TargetAllocKind = TARGET_ALLOC_SHARED;
  uint32_t SubscriptionRate = 4;
  uint32_t ForcedKernelWidth = 0;
  int32_t DeviceMode = DEVICE_MODE_TOP;
#if INTEL_INTERNAL_BUILD
  uint32_t ForcedLocalSizes[3] = {0, 0, 0};
  uint32_t ForcedGlobalSizes[3] = {0, 0, 0};
#endif // INTEL_INTERNAL_BUILD

  // Compilation options for IGC
  // OpenCL 2.0 builtins (like atomic_load_explicit and etc.) are used by
  // runtime, so we have to explicitly specify the "-cl-std=CL2.0" compilation
  // option. With it, the SPIR-V will be converted to LLVM IR with OpenCL 2.0
  // builtins. Otherwise, SPIR-V will be converted to LLVM IR with OpenCL 1.2
  // builtins.
  std::string CompilationOptions = "-cl-std=CL2.0 ";
  std::string InternalCompilationOptions;

  RTLDeviceInfoTy() {
    NumDevices = 0;
    Mutexes = nullptr;
    DataMutexes = nullptr;
    DataTransferLatency = 0;
    DeviceType = ZE_DEVICE_TYPE_GPU;
    readEnvironmentVars();
  }

  /// Read environment variable value with optional deprecated name
  char *readEnvVar(const char *Name, const char *OldName = nullptr) {
    if (!Name)
      return nullptr;
    char *value = std::getenv(Name);
    if (value || !OldName) {
      if (value)
        IDP("ENV: %s=%s\n", Name, value);
      return value;
    }
    value = std::getenv(OldName);
    if (value) {
      IDP("ENV: %s=%s\n", OldName, value);
      WARNING("%s is being deprecated. Use %s instead.\n", OldName, Name);
    }
    return value;
  }

#if INTEL_INTERNAL_BUILD
  void parseGroupSizes(const char *Name, const char *Value, uint32_t *Sizes) {
    std::string str(Value);
    if (str.front() != '{' || str.back() != '}') {
      WARNING("Ignoring invalid %s=%s\n", Name, Value);
      return;
    }
    std::istringstream strm(str.substr(1, str.size() - 2));
    uint32_t i = 0;
    for (std::string token; std::getline(strm, token, ','); i++)
      if (i < 3)
        Sizes[i] = std::stoi(token);
  }
#endif // INTEL_INTERNAL_BUILD

  void readEnvironmentVars() {
    // Debug level
    if (char *env = readEnvVar("LIBOMPTARGET_DEBUG"))
      DebugLevel = std::stoi(env);

    // Data transfer latency
    if (char *env = readEnvVar("LIBOMPTARGET_DATA_TRANSFER_LATENCY")) {
      std::string value(env);
      if (value.substr(0, 2) == "T,") {
        int32_t usec = std::stoi(value.substr(2).c_str());
        DataTransferLatency = (usec > 0) ? usec : 0;
      }
    }

    // Target device type
    if (char *env = readEnvVar("LIBOMPTARGET_DEVICETYPE")) {
      std::string value(env);
      if (value == "GPU" || value == "gpu" || value == "")
        DeviceType = ZE_DEVICE_TYPE_GPU;
      else
        WARNING("Invalid LIBOMPTARGET_DEVICETYPE=%s\n", env);
    }
    IDP("Target device type is set to GPU\n");

    // Global thread limit
    int threadLimit = omp_get_thread_limit();
    IDP("omp_get_thread_limit() returned %" PRId32 "\n", threadLimit);
    // omp_get_thread_limit() would return INT_MAX by default.
    // NOTE: Windows.h defines max() macro, so we have to guard
    //       the call with parentheses.
    ThreadLimit = (threadLimit > 0 &&
        threadLimit != (std::numeric_limits<int32_t>::max)()) ?
        threadLimit : 0;

    // Global max number of teams.
    int numTeams = omp_get_max_teams();
    IDP("omp_get_max_teams() returned %" PRId32 "\n", numTeams);
    // omp_get_max_teams() would return INT_MAX by default.
    // NOTE: Windows.h defines max() macro, so we have to guard
    //       the call with parentheses.
    NumTeams = (numTeams > 0 &&
        numTeams != (std::numeric_limits<int32_t>::max)()) ?
        numTeams : 0;

    // Compilation options for IGC
    if (char *env = readEnvVar("LIBOMPTARGET_LEVEL0_COMPILATION_OPTIONS"))
      CompilationOptions += env;

    if (DeviceType == ZE_DEVICE_TYPE_GPU) {
      // Intel Graphics compilers that do not support that option
      // silently ignore it. Other OpenCL compilers may fail.
      const char *env = readEnvVar("LIBOMPTARGET_LEVEL0_TARGET_GLOBALS");
      if (!env || (env[0] != 'F' && env[0] != 'f' && env[0] != '0')) {
        InternalCompilationOptions += " -cl-take-global-address ";
        Flags.EnableTargetGlobals = 1;
      }
      env = readEnvVar("LIBOMPTARGET_LEVEL0_MATCH_SINCOSPI");
      if (!env || (env[0] != 'F' && env[0] != 'f' && env[0] != '0')) {
        InternalCompilationOptions += " -cl-match-sincospi ";
      }
      env = readEnvVar("LIBOMPTARGET_LEVEL0_USE_DRIVER_GROUP_SIZES");
      if (env && (env[0] == 'T' || env[0] == 't' || env[0] == '1')) {
        Flags.UseDriverGroupSizes = 1;
      }
    }

    // Device mode
    if (char *env = readEnvVar("LIBOMPTARGET_DEVICES")) {
      std::string value(env);
      if (value == "DEVICE" || value == "device") {
        IDP("Device mode is %s -- using top-level devices with subdevice "
            "clause support\n", value.c_str());
        DeviceMode = DEVICE_MODE_TOP;
      } else if (value == "SUBDEVICE" || value == "subdevice") {
        IDP("Device mode is %s -- using 1st-level sub-devices\n",
            value.c_str());
        DeviceMode = DEVICE_MODE_SUB;
      } else if (value == "SUBSUBDEVICE" || value == "subsubdevice") {
        IDP("Device mode is %s -- using 2nd-level sub-devices\n",
            value.c_str());
        DeviceMode = DEVICE_MODE_SUBSUB;
      } else if (value == "ALL" || value == "all") {
        IDP("Device mode is %s -- using all sub-devices\n", value.c_str());
        DeviceMode = DEVICE_MODE_ALL;
      } else {
        IDP("Unknown device mode %s\n", value.c_str());
      }
    }

    // Profile
    if (char *env = readEnvVar("LIBOMPTARGET_PROFILE")) {
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

    // Managed memory allocator
    if (char *env = readEnvVar("LIBOMPTARGET_USM_HOST_MEM")) {
      if (env[0] == 'T' || env[0] == 't' || env[0] == '1')
        Flags.UseHostMemForUSM = 1;
    }

    // Memory pool
    if (char *env = readEnvVar("LIBOMPTARGET_LEVEL0_MEMORY_POOL")) {
      if (env[0] == 'T' || env[0] == 't' || env[0] == '1')
        Flags.UseMemoryPool = 1;
    }

    // Target image dump
    if (char *env = readEnvVar("LIBOMPTARGET_DUMP_TARGET_IMAGE",
                               "LIBOMPTARGET_SAVE_TEMPS")) {
      if (env[0] == 'T' || env[0] == 't' || env[0] == '1')
        Flags.DumpTargetImage = 1;
    }

    // Target allocation kind
    if (char *env = readEnvVar("LIBOMPTARGET_LEVEL0_USE_DEVICE_MEM")) {
      if (env[0] == 'T' || env[0] == 't' || env[0] == '1')
        TargetAllocKind = TARGET_ALLOC_DEVICE;
    }

    // Subscription rate
    if (char *env = readEnvVar("LIBOMPTARGET_LEVEL0_SUBSCRIPTION_RATE")) {
      int32_t value = std::stoi(env);
      if (value > 0 || value <= 0xFFFF)
        SubscriptionRate = value;
    }

    // Forced kernel width
    if (char *env = readEnvVar("LIBOMPTARGET_LEVEL0_KERNEL_WIDTH")) {
      int32_t value = std::stoi(env);
      if (value == 8 || value == 16 || value == 32)
        ForcedKernelWidth = value;
    }

#if INTEL_INTERNAL_BUILD
    // Force work group sizes -- for internal experiments
    if (char *env = readEnvVar("LIBOMPTARGET_LOCAL_WG_SIZE")) {
      parseGroupSizes("LIBOMPTARGET_LOCAL_WG_SIZE", env, ForcedLocalSizes);
    }
    if (char *env = readEnvVar("LIBOMPTARGET_GLOBAL_WG_SIZE")) {
      parseGroupSizes("LIBOMPTARGET_GLOBAL_WG_SIZE", env, ForcedGlobalSizes);
    }
#endif // INTEL_INTERNAL_BUILD
#if ENABLE_LIBDEVICE_LINKING
    // Link libdevice
    if (char *env = readEnvVar("LIBOMPTARGET_LEVEL0_LINK_LIBDEVICE")) {
      // TODO: turn this on by default when L0 issue is resolved.
      if (env[0] == 'T' || env[0] == 't' || env[0] == '1')
        Flags.LinkLibDevice = 1;
    }
#endif // ENABLE_LIBDEVICE_LINKING
  }

  ze_command_list_handle_t getCmdList(int32_t DeviceId) {
    if (ThreadLocalHandles.count(DeviceId) == 0) {
      ThreadLocalHandles.emplace(DeviceId, PrivateHandlesTy());
    }
    if (!ThreadLocalHandles[DeviceId].CmdList) {
      auto cmdList = createCmdList(Context, Devices[DeviceId],
                                   DeviceIdStr[DeviceId]);
      // Store it in the global list for clean up
      DataMutexes[DeviceId].lock();
      CmdLists[DeviceId].push_back(cmdList);
      DataMutexes[DeviceId].unlock();
      ThreadLocalHandles[DeviceId].CmdList = cmdList;
    }
    return ThreadLocalHandles[DeviceId].CmdList;
  }

  ze_command_queue_handle_t getCmdQueue(int32_t DeviceId) {
    if (ThreadLocalHandles.count(DeviceId) == 0) {
      ThreadLocalHandles.emplace(DeviceId, PrivateHandlesTy());
    }
    if (!ThreadLocalHandles[DeviceId].CmdQueue) {
      auto cmdQueue = createCmdQueue(Context, Devices[DeviceId],
                                     DeviceIdStr[DeviceId]);
      // Store it in the global list for clean up
      DataMutexes[DeviceId].lock();
      CmdQueues[DeviceId].push_back(cmdQueue);
      DataMutexes[DeviceId].unlock();
      ThreadLocalHandles[DeviceId].CmdQueue = cmdQueue;
    }
    return ThreadLocalHandles[DeviceId].CmdQueue;
  }

  RTLProfileTy *getProfile(int32_t DeviceId) {
    if (ThreadLocalHandles.count(DeviceId) == 0) {
      ThreadLocalHandles.emplace(DeviceId, PrivateHandlesTy());
    }
    if (!ThreadLocalHandles[DeviceId].Profile && Flags.EnableProfile) {
      auto &deviceProperties = DeviceProperties[DeviceId];
      ThreadLocalHandles[DeviceId].Profile = new RTLProfileTy(deviceProperties);
      DataMutexes[DeviceId].lock();
      Profiles[DeviceId].push_back(ThreadLocalHandles[DeviceId].Profile);
      DataMutexes[DeviceId].unlock();
    }
    return ThreadLocalHandles[DeviceId].Profile;
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

  /// Initialize program data on device
  int32_t initProgramData(int32_t DeviceId);

  /// Add implicit arguments
  void addImplicitArgs(int32_t DeviceId, void *Ptr, int32_t Kind);

  /// Remove implicit argumnets
  void removeImplicitArgs(int32_t DeviceId, void *Ptr);

  /// Get kernel indirect access flags
  ze_kernel_indirect_access_flags_t getKernelIndirectAccessFlags(
      ze_kernel_handle_t Kernel, uint32_t DeviceId);
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

typedef struct {
  int32_t NumLoops;        // Number of loops/dimensions
  int32_t DistributeDim;   // Dimensions lower than this one
                           // must end up in one WG
  TgtLoopDescTy Levels[3]; // Up to 3 loops
} TgtNDRangeDescTy;

static RTLDeviceInfoTy *DeviceInfo;

/// Init/deinit DeviceInfo
#ifdef _WIN32
#define ATTRIBUTE(X)
#else
#define ATTRIBUTE(X) __attribute__((X))
#endif // _WIN32

ATTRIBUTE(constructor(101)) void init() {
  IDP("Init Level0 plugin!\n");
  DeviceInfo = new RTLDeviceInfoTy();
}

ATTRIBUTE(destructor(101)) void deinit() {
  IDP("Deinit Level0 plugin!\n");
  delete DeviceInfo;
}

#ifdef _WIN32
static void closeRTL();
extern "C" BOOL WINAPI
DllMain(HINSTANCE const instance, // handle to DLL module
        DWORD const reason,       // reason for calling function
        LPVOID const reserved)    // reserved
{
  // Perform actions based on the reason for calling.
  switch (reason) {
  case DLL_PROCESS_ATTACH:
    // Initialize once for each new process.
    // Return FALSE to fail DLL load.
    init();
    break;

  case DLL_THREAD_ATTACH:
    // Do thread-specific initialization.
    break;

  case DLL_THREAD_DETACH:
    // Do thread-specific cleanup.
    break;

  case DLL_PROCESS_DETACH:
    // Perform any necessary cleanup.
    closeRTL();
    deinit();
    break;
  }
  return TRUE; // Successful DLL_PROCESS_ATTACH.
}
#endif // _WIN32

/// For scoped start/stop
class ScopedTimerTy {
  std::string Name;
  double TimeStamp = 0.0;
  bool Active = false;
  RTLProfileTy *Profile = nullptr;
public:
  ScopedTimerTy(int32_t DeviceId, const char *name) : Name(name) {
    Profile = DeviceInfo->getProfile(DeviceId);
    start();
  }
  ScopedTimerTy(int32_t DeviceId, std::string name) : Name(name) {
    Profile = DeviceInfo->getProfile(DeviceId);
    start();
  }
  ~ScopedTimerTy() {
    if (Active)
      stop();
  }
  void start() {
    if (!DeviceInfo->Flags.EnableProfile)
      return;
    if (!Profile) {
      WARNING("Profile data are invalid.\n");
      return;
    }
    if (Active)
      WARNING("Timer restarted.\n");
    TimeStamp = omp_get_wtime();
    Active = true;
  }
  void stop() {
    if (!DeviceInfo->Flags.EnableProfile)
      return;
    if (!Profile) {
      WARNING("Profile data are invalid.\n");
      return;
    }
    if (!Active) {
      WARNING("Timer is invalid.\n");
      return;
    }
    double currStamp = omp_get_wtime();
    Profile->update(Name, currStamp - TimeStamp);
    Active = false;
  }
  void updateDeviceTime(ze_event_handle_t Event) {
    if (!DeviceInfo->Flags.EnableProfile)
      return;
    if (!Profile) {
      WARNING("Profile data are invalid.\n");
      return;
    }
    Profile->update(Name, Event);
  }
};

static void addDataTransferLatency() {
  if (DeviceInfo->DataTransferLatency == 0)
    return;
  double goal = omp_get_wtime() + 1e-6 * DeviceInfo->DataTransferLatency;
  while (omp_get_wtime() < goal)
    ;
}

/// Clean-up routine to be registered by std::atexit().
static void closeRTL() {
  for (uint32_t i = 0; i < DeviceInfo->NumDevices; i++) {
    if (!DeviceInfo->Initialized[i])
      continue;
    if (DeviceInfo->Flags.EnableProfile) {
      for (auto profile : DeviceInfo->Profiles[i]) {
        profile->printData(DeviceInfo->DeviceIdStr[i].c_str(),
                           DeviceInfo->DeviceProperties[i].name);
        delete profile;
      }
    }
#ifndef _WIN32
    if (OMPT_ENABLED) {
      // Disabled for Windows to alleviate dll finalization issue.
      OMPT_CALLBACK(ompt_callback_device_unload, i, 0 /* module ID */);
      OMPT_CALLBACK(ompt_callback_device_finalize, i);
    }
    DeviceInfo->Mutexes[i].lock();
    for (auto mem : DeviceInfo->OwnedMemory[i]) {
      MEMSTAT_UPDATE(i, DeviceInfo->Context, 0, mem);
      CALL_ZE_EXIT_FAIL(zeMemFree, DeviceInfo->Context, mem);
    }
    for (auto cmdQueue : DeviceInfo->CmdQueues[i])
      CALL_ZE_EXIT_FAIL(zeCommandQueueDestroy, cmdQueue);
    for (auto cmdList : DeviceInfo->CmdLists[i])
      CALL_ZE_EXIT_FAIL(zeCommandListDestroy, cmdList);
    for (auto kernel : DeviceInfo->FuncGblEntries[i].Kernels) {
      if (kernel)
        CALL_ZE_EXIT_FAIL(zeKernelDestroy, kernel);
    }
    for (auto module : DeviceInfo->FuncGblEntries[i].Modules)
      CALL_ZE_EXIT_FAIL(zeModuleDestroy, module);
    if (DeviceInfo->DeviceMode == DEVICE_MODE_TOP &&
        DeviceInfo->NumDevices > DeviceInfo->NumRootDevices &&
        i < DeviceInfo->NumRootDevices) {
      auto &subDeviceEvent = DeviceInfo->SubDeviceEvents[i];
      for (auto event : subDeviceEvent.Events)
        CALL_ZE_EXIT_FAIL(zeEventDestroy, event);
      CALL_ZE_EXIT_FAIL(zeEventPoolDestroy, subDeviceEvent.Pool);
    }
    if (DeviceInfo->Flags.UseMemoryPool)
      DeviceInfo->PagePools[i].clear();
    DeviceInfo->Mutexes[i].unlock();
#endif // !defined(_WIN32)
    if (DeviceInfo->Flags.EnableTargetGlobals)
      DeviceInfo->unloadOffloadTable(i);
    MEMSTAT_PRINT(i);
  }
#ifndef _WIN32
  if (DeviceInfo->Flags.EnableProfile)
    DeviceInfo->ProfileEvents.deinit();
  if (DeviceInfo->Context)
    CALL_ZE_EXIT_FAIL(zeContextDestroy, DeviceInfo->Context);
#endif // !defined(_WIN32)
  delete[] DeviceInfo->Mutexes;
  delete[] DeviceInfo->DataMutexes;
  IDP("Closed RTL successfully\n");
}

static int32_t copyData(int32_t DeviceId, void *Dest, void *Src, size_t Size,
                        std::unique_lock<std::mutex> &copyLock) {
  if (DeviceInfo->TargetAllocKind == TARGET_ALLOC_SHARED) {
    char *src = static_cast<char *>(Src);
    std::copy(src, src + Size, static_cast<char *>(Dest));
  } else {
    auto cmdList = DeviceInfo->getCmdList(DeviceId);
    auto cmdQueue = DeviceInfo->getCmdQueue(DeviceId);
    bool ownsLock = false;
    if (!copyLock.owns_lock()) {
      copyLock.lock();
      ownsLock = true;
    }
    CALL_ZE_RET_FAIL(zeCommandListAppendMemoryCopy, cmdList, Dest, Src, Size,
                     nullptr, 0, nullptr);
    CALL_ZE_RET_FAIL(zeCommandListClose, cmdList);
    CALL_ZE_RET_FAIL(zeCommandQueueExecuteCommandLists, cmdQueue, 1, &cmdList,
                     nullptr);
    if (ownsLock)
      copyLock.unlock();
    CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, cmdQueue, UINT64_MAX);
    CALL_ZE_RET_FAIL(zeCommandListReset, cmdList);
  }
  return OFFLOAD_SUCCESS;
}

/// Allocate data explicitly
static void *allocDataExplicit(int32_t DeviceId, int64_t Size, int32_t Kind) {
  void *mem = nullptr;
  ze_device_mem_alloc_desc_t deviceDesc = {
    ZE_STRUCTURE_TYPE_DEVICE_MEM_ALLOC_DESC,
    nullptr, // extension
    0, // flags
    0 // ordinal, TODO: when do we use non-zero ordinal?
  };
  ze_host_mem_alloc_desc_t hostDesc = {
    ZE_STRUCTURE_TYPE_HOST_MEM_ALLOC_DESC,
    nullptr, // extension
    0 // flags
  };
  auto device = DeviceInfo->Devices[DeviceId];
  auto context = DeviceInfo->Context;

  switch (Kind) {
  case TARGET_ALLOC_DEVICE:
    CALL_ZE_RET_NULL(zeMemAllocDevice, context, &deviceDesc, Size,
                     LEVEL0_ALIGNMENT, device, &mem);
    IDP("Allocated a device memory object " DPxMOD "\n", DPxPTR(mem));
    break;
  case TARGET_ALLOC_HOST:
    CALL_ZE_RET_NULL(zeMemAllocHost, context, &hostDesc, Size,
                     LEVEL0_ALIGNMENT, &mem);
    IDP("Allocated a host memory object " DPxMOD "\n", DPxPTR(mem));
    break;
  case TARGET_ALLOC_SHARED:
    CALL_ZE_RET_NULL(zeMemAllocShared, context, &deviceDesc, &hostDesc,
                     Size, LEVEL0_ALIGNMENT, device, &mem);
    IDP("Allocated a shared memory object " DPxMOD "\n", DPxPTR(mem));
    break;
  default:
    FATAL_ERROR("Invalid target data allocation kind");
  }

  MEMSTAT_UPDATE(DeviceId, context, Size, mem);
  return mem;
}

/// Initialize module data
int32_t RTLDeviceInfoTy::initProgramData(int32_t DeviceId) {
  // Prepare host data to copy
  ModuleDataTy hostData = {
    1,                   // Initialized
    (int32_t)NumDevices, // Number of devices
    DeviceId             // Device ID
  };

  // Look up program data location on device
  void *dataPtr = getVarDeviceAddr(DeviceId, "__omp_spirv_program_data",
                  sizeof(hostData));
  if (!dataPtr) {
    IDP("Warning: cannot find module data location on device.\n");
    return OFFLOAD_SUCCESS;
  }

  auto cmdList = getCmdList(DeviceId);
  auto cmdQueue = getCmdQueue(DeviceId);
  CALL_ZE_RET_FAIL(zeCommandListAppendMemoryCopy, cmdList, dataPtr, &hostData,
                   sizeof(hostData), nullptr, 0, nullptr);
  CALL_ZE_RET_FAIL(zeCommandListClose, cmdList);
  CALL_ZE_RET_FAIL(zeCommandQueueExecuteCommandLists, cmdQueue, 1, &cmdList,
                   nullptr);
  CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, cmdQueue, UINT64_MAX);
  CALL_ZE_RET_FAIL(zeCommandListReset, cmdList);

  return OFFLOAD_SUCCESS;
}

/// Add implicit arguments
void RTLDeviceInfoTy::addImplicitArgs(
    int32_t DeviceId, void *Ptr, int32_t Kind) {
  if (Kind == TARGET_ALLOC_DEVICE)
    ImplicitArgsDevice[DeviceId].insert(Ptr);
  else if (Kind == TARGET_ALLOC_HOST)
    ImplicitArgsHost[DeviceId].insert(Ptr);
  else
    ImplicitArgsShared[DeviceId].insert(Ptr);
}

/// Remove implicit arguments
void RTLDeviceInfoTy::removeImplicitArgs(int32_t DeviceId, void *Ptr) {
  if (ImplicitArgsDevice[DeviceId].erase(Ptr) > 0)
    return;
  if (ImplicitArgsHost[DeviceId].erase(Ptr) > 0)
    return;
  if (ImplicitArgsShared[DeviceId].erase(Ptr) > 0)
    return;
}

/// Get kernel indirect access flags
ze_kernel_indirect_access_flags_t RTLDeviceInfoTy::getKernelIndirectAccessFlags(
    ze_kernel_handle_t Kernel, uint32_t DeviceId) {
  ze_kernel_indirect_access_flags_t flags = 0;

  if (KernelProperties[DeviceId][Kernel].RequiresIndirectAccess)
    flags |= TargetAllocKind == TARGET_ALLOC_SHARED
             ? ZE_KERNEL_INDIRECT_ACCESS_FLAG_SHARED
             : ZE_KERNEL_INDIRECT_ACCESS_FLAG_DEVICE;

  if (!ImplicitArgsDevice[DeviceId].empty())
    flags |= ZE_KERNEL_INDIRECT_ACCESS_FLAG_DEVICE;
  if (!ImplicitArgsHost[DeviceId].empty())
    flags |= ZE_KERNEL_INDIRECT_ACCESS_FLAG_HOST;
  if (!ImplicitArgsShared[DeviceId].empty())
    flags |= ZE_KERNEL_INDIRECT_ACCESS_FLAG_SHARED;

  return flags;
}

static void dumpImageToFile(
    const void *Image, size_t ImageSize, const char *Type) {
#if INTEL_INTERNAL_BUILD
  if (DebugLevel <= 0)
    return;

  if (!DeviceInfo->Flags.DumpTargetImage)
    return;

  char TmpFileName[] = "omptarget_spir64_image_XXXXXX";
#if _WIN32
  errno_t CErr = _mktemp_s(TmpFileName, sizeof(TmpFileName));
  if (CErr) {
    IDPI("Error creating temporary file template name.\n");
    return;
  }
  int TmpFileFd;
  _sopen_s(&TmpFileFd, TmpFileName, _O_RDWR | _O_CREAT | _O_BINARY,
           _SH_DENYNO, _S_IREAD | _S_IWRITE);
#else  // !_WIN32
  int TmpFileFd = mkstemp(TmpFileName);
#endif  // !_WIN32
  IDPI("Dumping %s image of size %d from address " DPxMOD " to file %s\n",
      Type, static_cast<int32_t>(ImageSize), DPxPTR(Image), TmpFileName);

  if (TmpFileFd < 0) {
    IDPI("Error creating temporary file: %s\n", strerror(errno));
    return;
  }

#if _WIN32
  int WErr = _write(TmpFileFd, Image, ImageSize);
#else  // !_WIN32
  int WErr = write(TmpFileFd, Image, ImageSize);
#endif  // !_WIN32
  if (WErr < 0) {
    IDPI("Error writing temporary file %s: %s\n", TmpFileName, strerror(errno));
  }

#if _WIN32
  int CloseErr = _close(TmpFileFd);
#else  // !_WIN32
  int CloseErr = close(TmpFileFd);
#endif  // !_WIN32
  if (CloseErr < 0) {
    IDPI("Error closing temporary file %s: %s\n", TmpFileName, strerror(errno));
  }
#endif  // INTEL_INTERNAL_BUILD
}

EXTERN
int32_t __tgt_rtl_is_valid_binary(__tgt_device_image *Image) {
  uint32_t magicWord = *(uint32_t *)Image->ImageStart;
  // compare magic word in little endian and big endian:
  int32_t ret = (magicWord == 0x07230203 || magicWord == 0x03022307);
  IDP("Target binary is %s\n", ret ? "VALID" : "INVALID");
  return ret;
}

EXTERN int64_t __tgt_rtl_init_requires(int64_t RequiresFlags) {
  IDP("Initialize requires flags to %" PRId64 "\n", RequiresFlags);
  DeviceInfo->RequiresFlags = RequiresFlags;
  return RequiresFlags;
}

/// Check if the subdevice IDs are valid
static bool isValidSubDevice(int64_t DeviceIds) {
  if (DeviceIds >= 0) {
    IDP("Invalid non-negative subdevice encoding %" PRId64 "\n", DeviceIds);
    return false;
  }

  uint32_t rootId = SUBDEVICE_GET_ROOT(DeviceIds);

  if (rootId >= DeviceInfo->NumRootDevices) {
    IDP("Invalid root device ID %" PRIu32 "\n", rootId);
  }

  uint32_t subLevel = SUBDEVICE_GET_LEVEL(DeviceIds);
  uint32_t subStart = SUBDEVICE_GET_START(DeviceIds);
  uint32_t subCount = SUBDEVICE_GET_COUNT(DeviceIds);
  uint32_t subStride = SUBDEVICE_GET_STRIDE(DeviceIds);

  auto &subDeviceIds = DeviceInfo->SubDeviceIds[rootId];
  if (subLevel >= subDeviceIds.size()) {
    IDP("Invalid subdevice level %" PRIu32 "\n", subLevel);
    return false;
  }
  for (uint32_t i = 0; i < subCount; i++) {
    uint32_t subId = subStart + i * subStride;
    if (subId >= DeviceInfo->SubDeviceIds[rootId][subLevel].size()) {
      IDP("Invalid subdevice ID %" PRIu32 " at level %" PRIu32 "\n",
         subId, subLevel);
      return false;
    }
  }
  return true;
}

/// Find subdevice handles
static int32_t getSubDevices(
    uint32_t Level, ze_device_handle_t Parent, SubDeviceListsTy &Lists) {
  if (Level >= DeviceInfo->SubDeviceLevels) {
    IDP("Finished checking %" PRIu32 " levels of subdevices.\n", Level);
    return OFFLOAD_SUCCESS;
  }

  uint32_t numDevices = 0;
  CALL_ZE_RET_FAIL(zeDeviceGetSubDevices, Parent, &numDevices, nullptr);

  if (numDevices == 0) {
    IDP("No subdevices are found for device " DPxMOD " at level %" PRIu32 "\n",
        DPxPTR(Parent), Level);
    return OFFLOAD_SUCCESS;
  }

  std::vector<ze_device_handle_t> devices(numDevices);
  CALL_ZE_RET_FAIL(zeDeviceGetSubDevices, Parent, &numDevices, devices.data());
  if (Lists.size() > Level)
    Lists[Level].insert(Lists[Level].end(), devices.begin(), devices.end());
  else
    Lists.push_back(devices);

  for (auto device : devices) {
    IDP("Found subdevice " DPxMOD " for device " DPxMOD " at level %" PRIu32
        "\n", DPxPTR(device), DPxPTR(Parent), Level);
    if (getSubDevices(Level + 1, device, Lists) != OFFLOAD_SUCCESS)
      return OFFLOAD_FAIL;
  }

  return OFFLOAD_SUCCESS;
}

EXTERN
int32_t __tgt_rtl_number_of_devices() {
  IDP("Looking for Level0 devices...\n");

  CALL_ZE_RET_ZERO(zeInit, ZE_INIT_FLAG_GPU_ONLY);
  IDP("Initialized L0, API %" PRIx32 "\n", ZE_API_VERSION_CURRENT);

  uint32_t numDrivers = 0;
  CALL_ZE_RET_ZERO(zeDriverGet, &numDrivers, nullptr);
  if (numDrivers == 0)
    return 0;

  std::vector<ze_driver_handle_t> driverHandles(numDrivers);
  CALL_ZE_RET_ZERO(zeDriverGet, &numDrivers, driverHandles.data());
  IDP("Found %" PRIu32 " driver(s)!\n", numDrivers);

  auto deviceMode = DeviceInfo->DeviceMode;

  for (uint32_t i = 0; i < numDrivers; i++) {
    // Check available devices
    uint32_t numDevices = 0;
    CALL_ZE_RET_ZERO(zeDeviceGet, driverHandles[i], &numDevices, nullptr);
    if (numDevices == 0) {
      IDP("Cannot find any devices for driver %" PRIu32 "!\n", i);
      continue;
    }

    // Get device handles and check device type
    std::vector<ze_device_handle_t> devices(numDevices);
    CALL_ZE_RET_ZERO(zeDeviceGet, driverHandles[i], &numDevices,
                     devices.data());

    for (uint32_t i = 0; i < numDevices; i++) {
      auto device = devices[i];
      ze_device_properties_t properties = {};
      CALL_ZE_RET_ZERO(zeDeviceGetProperties, device, &properties);
      IDP("Found a GPU device, Name = %s\n", properties.name);

      ze_device_compute_properties_t computeProperties;
      CALL_ZE_RET_ZERO(zeDeviceGetComputeProperties, device,
                       &computeProperties);

      if (deviceMode == DEVICE_MODE_TOP || deviceMode == DEVICE_MODE_ALL) {
        DeviceInfo->Devices.push_back(device);
        DeviceInfo->DeviceProperties.push_back(properties);
        DeviceInfo->ComputeProperties.push_back(computeProperties);
        DeviceInfo->DeviceIdStr.push_back(std::to_string(i));
      }

      // Find subdevices, add them to the device list, mark where thye are.
      // Collect lists of subdevice handles first.
      SubDeviceListsTy subDeviceLists;
      if (getSubDevices(0, device, subDeviceLists) != OFFLOAD_SUCCESS)
        return 0;
      DeviceInfo->SubDeviceIds.emplace_back(SubDeviceIdsTy());
      auto &subDeviceIds = DeviceInfo->SubDeviceIds.back();

      // Fill internal data using the list of subdevice handles.
      // Internally, all devices/subdevices are listed as follows for N devices
      // where Subdevices(i,j) is a list of subdevices for device i at level j.
      // [0..N-1][Subdevices(0,0),Subdevices(0,1)]..[Subdevices(N-1,0)..]
      for (size_t j = 0; j < subDeviceLists.size(); j++) {
        subDeviceIds.emplace_back(std::vector<int32_t>());
        for (size_t k = 0; k < subDeviceLists[j].size(); k++) {
          if (deviceMode == DEVICE_MODE_SUB && j != 0 ||
              deviceMode == DEVICE_MODE_SUBSUB && j != 1)
            continue;
          // All other cases require these internal data.
          auto subDevice = subDeviceLists[j][k];
          subDeviceIds.back().push_back(DeviceInfo->Devices.size());
          DeviceInfo->Devices.push_back(subDevice);
          CALL_ZE_RET_ZERO(zeDeviceGetProperties, subDevice, &properties);
          DeviceInfo->DeviceProperties.push_back(properties);
          CALL_ZE_RET_ZERO(zeDeviceGetComputeProperties, subDevice,
                           &computeProperties);
          DeviceInfo->ComputeProperties.push_back(computeProperties);
          DeviceInfo->DeviceIdStr.push_back(std::to_string(i) + "." +
                                            std::to_string(j) + "." +
                                            std::to_string(k));
        }
      }
    }

    DeviceInfo->Driver = driverHandles[i];
    DeviceInfo->NumRootDevices = numDevices;
    DeviceInfo->NumDevices = DeviceInfo->Devices.size();
    IDP("Found %" PRIu32 " root devices, %" PRIu32 " total devices.\n",
        DeviceInfo->NumRootDevices, DeviceInfo->NumDevices);
    IDP("List of devices (DeviceID[.SubDeviceLevel.SubDeviceID])\n");
    for (auto &str : DeviceInfo->DeviceIdStr)
      IDP("-- %s\n", str.c_str());
    break;
  }

  DeviceInfo->CmdLists.resize(DeviceInfo->NumDevices);
  DeviceInfo->CmdQueues.resize(DeviceInfo->NumDevices);
  DeviceInfo->FuncGblEntries.resize(DeviceInfo->NumDevices);
  DeviceInfo->KernelProperties.resize(DeviceInfo->NumDevices);
  DeviceInfo->OwnedMemory.resize(DeviceInfo->NumDevices);
  DeviceInfo->ImplicitArgsDevice.resize(DeviceInfo->NumDevices);
  DeviceInfo->ImplicitArgsHost.resize(DeviceInfo->NumDevices);
  DeviceInfo->ImplicitArgsShared.resize(DeviceInfo->NumDevices);
  DeviceInfo->Initialized.resize(DeviceInfo->NumDevices);
  DeviceInfo->Mutexes = new std::mutex[DeviceInfo->NumDevices];
  DeviceInfo->DataMutexes = new std::mutex[DeviceInfo->NumDevices];
  DeviceInfo->OffloadTables.resize(DeviceInfo->NumDevices);
  DeviceInfo->Profiles.resize(DeviceInfo->NumDevices);
  if (DeviceInfo->Flags.UseMemoryPool)
    DeviceInfo->PagePools.resize(DeviceInfo->NumDevices);
#if INTEL_INTERNAL_BUILD
  if (DebugLevel > 0)
    MemStats.resize(DeviceInfo->NumDevices);
#endif // INTEL_INTERNAL_BUILD
  DeviceInfo->Context = createContext(DeviceInfo->Driver);
  if (DeviceInfo->Flags.EnableProfile)
    DeviceInfo->ProfileEvents.init(DeviceInfo->Context);
  DeviceInfo->SubDeviceEvents.resize(DeviceInfo->NumRootDevices);

#ifndef _WIN32
  if (std::atexit(closeRTL)) {
    FATAL_ERROR("Registration of clean-up function");
  }
#endif // _WIN32

  if (deviceMode == DEVICE_MODE_TOP) {
    IDP("Returning %" PRIu32 " top-level devices\n",
        DeviceInfo->NumRootDevices);
    return DeviceInfo->NumRootDevices;
  } else {
    // Just keep empty internal ID mapping in this case.
    DeviceInfo->SubDeviceIds.clear();
    DeviceInfo->SubDeviceIds.resize(DeviceInfo->NumDevices);
    IDP("Returning %" PRIu32 " devices including sub-devices\n",
        DeviceInfo->NumDevices);
    return DeviceInfo->NumDevices;
  }
}

EXTERN
int32_t __tgt_rtl_init_device(int32_t DeviceId) {
  if (DeviceId < 0 || DeviceId >= (int32_t)DeviceInfo->NumDevices ||
      DeviceInfo->DeviceMode == DEVICE_MODE_TOP &&
          DeviceId >= (int32_t)DeviceInfo->NumRootDevices) {
    IDP("Bad device ID %" PRId32 "\n", DeviceId);
    return OFFLOAD_FAIL;
  }

  uint32_t numSubDevices = 0;
  for (auto &subIds : DeviceInfo->SubDeviceIds[DeviceId]) {
    numSubDevices += subIds.size();
    for (auto subId : subIds) {
      DeviceInfo->Initialized[subId] = true;
      if (DeviceInfo->Flags.UseMemoryPool)
        DeviceInfo->PagePools[subId].initialize(subId, DeviceInfo->Context,
            DeviceInfo->Devices[subId], DeviceInfo->TargetAllocKind);
    }
  }
  if (numSubDevices > 0) {
    // Create Events for subdevices commands
    auto &subDeviceEvent = DeviceInfo->SubDeviceEvents[DeviceId];
    ze_event_pool_desc_t eventPoolDesc = {
      ZE_STRUCTURE_TYPE_EVENT_POOL_DESC,
      nullptr,
      ZE_EVENT_POOL_FLAG_HOST_VISIBLE,
      numSubDevices
    };
    CALL_ZE_RET_FAIL(zeEventPoolCreate, DeviceInfo->Context, &eventPoolDesc,
                     0, nullptr, &subDeviceEvent.Pool);
    ze_event_desc_t eventDesc = {
      ZE_STRUCTURE_TYPE_EVENT_DESC,
      nullptr,
      0, // index
      0,
      0
    };
    for (uint32_t i = 0; i < numSubDevices; i++) {
      eventDesc.index = i;
      ze_event_handle_t event;
      CALL_ZE_RET_FAIL(zeEventCreate, subDeviceEvent.Pool, &eventDesc, &event);
      subDeviceEvent.Events.push_back(event);
    }
  }
  if (DeviceInfo->Flags.UseMemoryPool)
    DeviceInfo->PagePools[DeviceId].initialize(DeviceId, DeviceInfo->Context,
        DeviceInfo->Devices[DeviceId], DeviceInfo->TargetAllocKind);

  DeviceInfo->Initialized[DeviceId] = true;

  OMPT_CALLBACK(ompt_callback_device_initialize, DeviceId,
                DeviceInfo->DeviceProperties[DeviceId].name,
                DeviceInfo->Devices[DeviceId],
                omptLookupEntries, OmptDocument);

  IDP("Initialized Level0 device %" PRId32 "\n", DeviceId);
  return OFFLOAD_SUCCESS;
}

EXTERN
__tgt_target_table *__tgt_rtl_load_binary(int32_t DeviceId,
                                          __tgt_device_image *Image) {
  IDP("Device %" PRId32 ": Loading binary from " DPxMOD "\n", DeviceId,
     DPxPTR(Image->ImageStart));

  size_t imageSize = (size_t)Image->ImageEnd - (size_t)Image->ImageStart;
  size_t numEntries = (size_t)(Image->EntriesEnd - Image->EntriesBegin);
  IDP("Expecting to have %zu entries defined\n", numEntries);

  std::string compilationOptions(DeviceInfo->CompilationOptions);
  IDP("Module compilation options: %s\n", compilationOptions.c_str());
  compilationOptions += " " + DeviceInfo->InternalCompilationOptions;
  IDPI("Final module compilation options: %s\n", compilationOptions.c_str());

  dumpImageToFile(Image->ImageStart, imageSize, "OpenMP");

  auto context = DeviceInfo->Context;
  auto device = DeviceInfo->Devices[DeviceId];

  ScopedTimerTy tmModuleBuild(DeviceId, "ModuleBuild");

  auto &modules = DeviceInfo->FuncGblEntries[DeviceId].Modules;
  auto mainModule = createModule(context, device, imageSize,
                                 (uint8_t *)Image->ImageStart,
                                 compilationOptions.c_str());
  if (!mainModule) {
    IDP("Error: failed to create main module\n");
    return nullptr;
  }

  modules.push_back(mainModule);

#if ENABLE_LIBDEVICE_LINKING
  std::vector<const char *> deviceLibNames {
    "libomp-fallback-cassert.spv",
    "libomp-fallback-cmath.spv",
    "libomp-fallback-cmath-fp64.spv",
    "libomp-fallback-complex.spv",
    "libomp-fallback-complex-fp64.spv"
  };

  for (auto name : deviceLibNames) {
    auto deviceLibModule = createModule(context, device, name,
                                        compilationOptions.c_str());
    if (deviceLibModule) {
      IDP("Created a module for %s\n", name);
      modules.push_back(deviceLibModule);
    }
  }

  if (DeviceInfo->Flags.LinkLibDevice) {
    int32_t rc;
    ze_module_build_log_handle_t linkLog;
    CALL_ZE_RC(rc, zeModuleDynamicLink, modules.size(), modules.data(), &linkLog);
    if (rc != ZE_RESULT_SUCCESS) {
      if (DebugLevel > 0) {
        size_t logSize;
        CALL_ZE_RET_NULL(zeModuleBuildLogGetString, linkLog, &logSize, nullptr);
        std::vector<char> logString(logSize);
        CALL_ZE_RET_NULL(zeModuleBuildLogGetString, linkLog, &logSize,
                         logString.data());
        IDP("Error: module link failed -- see below for details.\n");
        fprintf(stderr, "%s\n", logString.data());
      }
      CALL_ZE_RET_NULL(zeModuleBuildLogDestroy, linkLog);
      return nullptr;
    }
    CALL_ZE_RET_NULL(zeModuleBuildLogDestroy, linkLog);
  }
#endif // ENABLE_LIBDEVICE_LINKING

  tmModuleBuild.stop();

  auto &entries = DeviceInfo->FuncGblEntries[DeviceId].Entries;
  auto &kernels = DeviceInfo->FuncGblEntries[DeviceId].Kernels;
  entries.resize(numEntries);
  kernels.resize(numEntries);

  if (DeviceInfo->Flags.EnableTargetGlobals &&
      !DeviceInfo->loadOffloadTable(DeviceId, numEntries))
    IDP("Warning: offload table loading failed.\n");

  for (uint32_t i = 0; i < numEntries; i++) {
    auto size = Image->EntriesBegin[i].size;

    if (size != 0) {
      // Entry is a global variable
      auto hstAddr = Image->EntriesBegin[i].addr;
      auto name = Image->EntriesBegin[i].name;
      void *tgtAddr = nullptr;
      if (DeviceInfo->Flags.EnableTargetGlobals)
        tgtAddr = DeviceInfo->getOffloadVarDeviceAddr(DeviceId, name, size);

      if (!tgtAddr) {
        tgtAddr = __tgt_rtl_data_alloc(DeviceId, size, hstAddr);
        __tgt_rtl_data_submit(DeviceId, tgtAddr, hstAddr, size);
        if (!DeviceInfo->Flags.UseMemoryPool ||
            size > DeviceInfo->PagePools[DeviceId].getMaxSize()) {
          DeviceInfo->DataMutexes[DeviceId].lock();
          DeviceInfo->OwnedMemory[DeviceId].push_back(tgtAddr);
          DeviceInfo->DataMutexes[DeviceId].unlock();
        }
        IDP("Warning: global variable '%s' allocated. "
          "Direct references will not work properly.\n", name);
      }

      IDP("Global variable mapped: Name = %s, Size = %zu, "
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
      IDP("Warning: Entry with a nullptr name!!!\n");
      entries[i].addr = nullptr;
      entries[i].name = nullptr;
      continue;
    }
#endif
    ze_kernel_desc_t kernelDesc = {
      ZE_STRUCTURE_TYPE_KERNEL_DESC,
      nullptr, // extension
      0, // flags
      name,
    };
    ze_result_t rc = ZE_RESULT_ERROR_UNKNOWN;
    CALL_ZE_RC(rc, zeKernelCreate, mainModule, &kernelDesc, &kernels[i]);
    if (rc != ZE_RESULT_SUCCESS) {
      // If a kernel was deleted by optimizations (e.g. DCE), then
      // zeCreateKernel will fail. We expect that such a kernel
      // will never be actually invoked.
      IDP("Warning: Failed to create kernel %s\n", name);
      kernels[i] = nullptr;
    }
    entries[i].addr = &kernels[i];
    entries[i].name = name;

    // Do not try to query information for deleted kernels.
    if (!kernels[i])
      continue;

    // Retrieve kernel group size info.
    ze_kernel_properties_t kernelProperties;
    CALL_ZE(rc, zeKernelGetProperties, kernels[i], &kernelProperties);
    if (DeviceInfo->ForcedKernelWidth > 0) {
      DeviceInfo->KernelProperties[DeviceId][kernels[i]].Width =
          DeviceInfo->ForcedKernelWidth;
    } else {
      DeviceInfo->KernelProperties[DeviceId][kernels[i]].Width =
          kernelProperties.maxSubgroupSize;
    }
    if (DebugLevel > 0) {
      void *entryAddr = Image->EntriesBegin[i].addr;
      const char *entryName = Image->EntriesBegin[i].name;
      IDP("Kernel %" PRIu32 ": Entry = " DPxMOD ", Name = %s, NumArgs = %"
         PRIu32 ", Handle = " DPxMOD "\n", i, DPxPTR(entryAddr), entryName,
         kernelProperties.numKernelArgs, DPxPTR(kernels[i]));
    }
#if 0
    // Enable this with 0.95.55 Level Zero.
    DeviceInfo->KernelProperties[DeviceId][kernels[i]].MaxThreadGroupSize =
        kernelProperties.maxSubgroupSize * kernelProperties.maxNumSubgroups;
#else
    DeviceInfo->KernelProperties[DeviceId][kernels[i]].MaxThreadGroupSize =
        (std::numeric_limits<uint32_t>::max)();
#endif
    // TODO: show kernel information
  }

  if (DeviceInfo->initProgramData(DeviceId) != OFFLOAD_SUCCESS)
    return nullptr;
  __tgt_target_table &table = DeviceInfo->FuncGblEntries[DeviceId].Table;
  table.EntriesBegin = &(entries.data()[0]);
  table.EntriesEnd = &(entries.data()[entries.size()]);

  if ((uint32_t)DeviceId < DeviceInfo->NumRootDevices) {
    for (auto &subIdList : DeviceInfo->SubDeviceIds[DeviceId])
      for (auto subId : subIdList)
#if SUBDEVICE_USE_ROOT_KERNELS
        // Use root module while copying kernel properties from root.
        DeviceInfo->KernelProperties[subId] =
            DeviceInfo->KernelProperties[DeviceId];
#else // !SUBDEVICE_USE_ROOT_KERNELS
        // Create modules for subdevices. We don't need to return the table
        // created for subdevices.
        if (__tgt_rtl_load_binary(subId, Image) == nullptr)
          return nullptr;
#endif // !SUBDEVICE_USE_ROOT_KERNELS
  }

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
#if !SUBDEVICE_USE_ROOT_MEMORY
  if (SubDeviceCode < 0 && SUBDEVICE_GET_COUNT(SubDeviceCode) == 1) {
    auto subLevel = SUBDEVICE_GET_LEVEL(SubDeviceCode);
    auto subStart = SUBDEVICE_GET_START(SubDeviceCode);
    DeviceId = DeviceInfo->SubDeviceIds[DeviceId][subLevel][subStart];
  }
#endif

  // TODO: this seems necessary for now -- check with L0 driver team for details
  std::unique_lock<std::mutex> allocLock(DeviceInfo->Mutexes[DeviceId]);

  ScopedTimerTy tmDataAlloc(DeviceId, "DataAlloc");
  intptr_t offset = (intptr_t)HstPtr - (intptr_t)HstBase;
  size_t size = (offset < 0 && ABS(offset) >= Size) ? ABS(offset) + 1 : Size;

  offset = (offset >= 0) ? offset : 0;
  size += offset;

  void *base = nullptr;
  void *mem = nullptr;
  if (DeviceInfo->Flags.UseMemoryPool &&
      (base = DeviceInfo->PagePools[DeviceId].allocate(size))) {
    mem = (void *)((intptr_t)base + offset);
    IDP("Allocated target memory " DPxMOD " (Base: " DPxMOD ", Size: %zu) "
       "from memory pool for host ptr " DPxMOD "\n",
       DPxPTR(mem), DPxPTR(base), size, DPxPTR(HstPtr));
    if (IsImplicitArg)
      DeviceInfo->addImplicitArgs(DeviceId, mem, DeviceInfo->TargetAllocKind);

    return mem;
  }

  // We use shared USM to avoid overheads coming from explicit data copy to
  // device memory.
  base = allocDataExplicit(DeviceId, size, DeviceInfo->TargetAllocKind);
  mem = (void *)((intptr_t)base + offset);

  if (DebugLevel > 0) {
    void *actualBase = nullptr;
    size_t actualSize = 0;
    CALL_ZE_RET_NULL(zeMemGetAddressRange, DeviceInfo->Context, mem,
                     &actualBase, &actualSize);
    assert(base == actualBase && "Invalid memory address range!");
    IDP("Allocated target memory " DPxMOD " (Base: " DPxMOD
       ", Size: %zu) for host ptr " DPxMOD "\n", DPxPTR(mem), DPxPTR(actualBase),
       actualSize, DPxPTR(HstPtr));
  }
  if (IsImplicitArg)
    DeviceInfo->addImplicitArgs(DeviceId, mem, DeviceInfo->TargetAllocKind);

  return mem;
}

EXTERN
void *__tgt_rtl_data_alloc(int32_t DeviceId, int64_t Size, void *HstPtr) {
  return allocData(DeviceId, Size, HstPtr, HstPtr, 0);
}

EXTERN void *__tgt_rtl_data_alloc_user(
    int32_t DeviceId, int64_t Size, void *HstPtr) {
  return allocData(DeviceId, Size, HstPtr, HstPtr, 1);
}

EXTERN
void *__tgt_rtl_data_alloc_base(int32_t DeviceId, int64_t Size, void *HstPtr,
                                void *HstBase) {
  return allocData(DeviceId, Size, HstPtr, HstBase, 0);
}

EXTERN void *__tgt_rtl_data_alloc_managed(int32_t DeviceId, int64_t Size) {
  int32_t kind = DeviceInfo->Flags.UseHostMemForUSM ? TARGET_ALLOC_HOST
                                                    : TARGET_ALLOC_SHARED;
  return __tgt_rtl_data_alloc_explicit(DeviceId, Size, kind);
}

EXTERN int32_t __tgt_rtl_data_delete_managed(int32_t DeviceId, void *Ptr) {
  return __tgt_rtl_data_delete(DeviceId, Ptr);
}

EXTERN int32_t __tgt_rtl_is_device_accessible_ptr(int32_t DeviceId, void *Ptr) {
  ze_memory_allocation_properties_t properties = {
    ZE_STRUCTURE_TYPE_MEMORY_ALLOCATION_PROPERTIES,
    nullptr, // extension
    ZE_MEMORY_TYPE_UNKNOWN, // type
    0, // id
    0, // page size
  };
  ze_result_t rc;
  auto context = DeviceInfo->Context;
  CALL_ZE(rc, zeMemGetAllocProperties, context, Ptr, &properties, nullptr);
  // Host pointer is classified as an error -- skip error reporting
  // TODO: check if this checking is still required
  if (rc == ZE_RESULT_ERROR_INVALID_ARGUMENT)
    return 0;
  CALL_ZE_RET_ZERO(zeMemGetAllocProperties, context, Ptr, &properties,
                   nullptr);
  int32_t ret = properties.type != ZE_MEMORY_TYPE_UNKNOWN;

  IDP("Ptr " DPxMOD " is %sa device accessible memory pointer.\n", DPxPTR(Ptr),
     ret ? "" : "not ");
  return ret;
}

EXTERN void *__tgt_rtl_data_alloc_explicit(
    int32_t DeviceId, int64_t Size, int32_t Kind) {
  void *mem = allocDataExplicit(DeviceId, Size, Kind);
  if (mem) {
    std::unique_lock<std::mutex>(DeviceInfo->DataMutexes[DeviceId]);
    DeviceInfo->addImplicitArgs(DeviceId, mem, Kind);
  }
  return mem;
}

// Tasks to be done when completing an asynchronous command.
static void endAsyncCommand(AsyncEventTy *Event,
                            ze_command_list_handle_t CmdList,
                            ze_fence_handle_t Fence) {
  if (!Event || !Event->Handler || !Event->Arg) {
    FATAL_ERROR("Invalid asynchronous offloading event");
  }

  IDP("Calling asynchronous offloading event handler " DPxMOD " with argument "
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
    IDP("Error: Failed to start asynchronous command -- invalid argument\n");
    return OFFLOAD_FAIL;
  }

  CALL_ZE_RET_FAIL(zeCommandListClose, CmdList);

  // Spawn waiting thread
  std::thread waiter([](AsyncEventTy *event, ze_command_list_handle_t cmdList,
                        ze_fence_handle_t fence) {
    // Wait until the fence is signaled.
    CALL_ZE_EXIT_FAIL(zeFenceHostSynchronize, fence, UINT64_MAX);
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

#if !SUBDEVICE_USE_ROOT_MEMORY
  if (SubDeviceCode < 0 && SUBDEVICE_GET_COUNT(SubDeviceCode) == 1) {
    auto subLevel = SUBDEVICE_GET_LEVEL(SubDeviceCode);
    auto subStart = SUBDEVICE_GET_START(SubDeviceCode);
    DeviceId = DeviceInfo->SubDeviceIds[DeviceId][subLevel][subStart];
  }
#endif

  ScopedTimerTy tmDataWrite(DeviceId, "DataWrite");

  // Add synthetic delay for experiments
  addDataTransferLatency();

  std::unique_lock<std::mutex> copyLock(DeviceInfo->Mutexes[DeviceId],
                                        std::defer_lock);

  if (AsyncEvent) {
    copyLock.lock();
    auto context = DeviceInfo->Context;
    auto cmdList = createCmdList(context, DeviceInfo->Devices[DeviceId],
                                 DeviceInfo->DeviceIdStr[DeviceId]);
    auto cmdQueue = DeviceInfo->getCmdQueue(DeviceId);
    if (!cmdList) {
      IDP("Error: Asynchronous data submit failed -- invalid command list\n");
      return OFFLOAD_FAIL;
    }
    auto fence = createFence(cmdQueue);
    if (!fence) {
      IDP("Error: Asynchronous data submit failed -- invalid fence\n");
      return OFFLOAD_FAIL;
    }
    CALL_ZE_RET_FAIL(zeCommandListAppendMemoryCopy, cmdList, TgtPtr, HstPtr,
                     Size, nullptr, 0, nullptr);
    if (beginAsyncCommand(cmdList, cmdQueue,
        static_cast<AsyncEventTy *>(AsyncEvent), fence) == OFFLOAD_FAIL)
      return OFFLOAD_FAIL;
    IDP("Asynchronous data submit started -- %" PRId64 " bytes (hst:"
       DPxMOD ") -> (tgt:" DPxMOD ")\n", Size, DPxPTR(HstPtr), DPxPTR(TgtPtr));
  } else {
    if (copyData(DeviceId, TgtPtr, HstPtr, Size, copyLock) != OFFLOAD_SUCCESS)
      return OFFLOAD_FAIL;
    IDP("Copied %" PRId64 " bytes (hst:" DPxMOD ") -> (tgt:" DPxMOD ")\n", Size,
       DPxPTR(HstPtr), DPxPTR(TgtPtr));
  }

  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_data_submit(
    int32_t DeviceId, void *TgtPtr, void *HstPtr, int64_t Size) {
  return submitData(DeviceId, TgtPtr, HstPtr, Size, nullptr);
}

EXTERN int32_t __tgt_rtl_data_submit_async(
    int32_t DeviceId, void *TgtPtr, void *HstPtr, int64_t Size,
    __tgt_async_info *AsyncInfoPtr /*not used*/) {
  return submitData(DeviceId, TgtPtr, HstPtr, Size, nullptr);
}

EXTERN int32_t __tgt_rtl_data_submit_nowait(
    int32_t DeviceId, void *TgtPtr, void *HstPtr, int64_t Size,
    void *AsyncEvent) {
  return submitData(DeviceId, TgtPtr, HstPtr, Size, AsyncEvent);
}

static int32_t retrieveData(
    int32_t DeviceId, void *HstPtr, void *TgtPtr, int64_t Size,
    void *AsyncEvent) {
  if (Size == 0)
    return OFFLOAD_SUCCESS;

#if !SUBDEVICE_USE_ROOT_MEMORY
  if (SubDeviceCode < 0 && SUBDEVICE_GET_COUNT(SubDeviceCode) == 1) {
    auto subLevel = SUBDEVICE_GET_LEVEL(SubDeviceCode);
    auto subStart = SUBDEVICE_GET_START(SubDeviceCode);
    DeviceId = DeviceInfo->SubDeviceIds[DeviceId][subLevel][subStart];
  }
#endif

  ScopedTimerTy tmDataRead(DeviceId, "DataRead");

  // Add synthetic delay for experiments
  addDataTransferLatency();

  std::unique_lock<std::mutex> copyLock(DeviceInfo->Mutexes[DeviceId],
                                        std::defer_lock);

  if (AsyncEvent) {
    copyLock.lock();
    auto context = DeviceInfo->Context;
    auto cmdList = createCmdList(context, DeviceInfo->Devices[DeviceId],
                                 DeviceInfo->DeviceIdStr[DeviceId]);
    auto cmdQueue = DeviceInfo->getCmdQueue(DeviceId);
    if (!cmdList) {
      IDP("Error: Asynchronous data retrieve failed -- invalid command list\n");
      return OFFLOAD_FAIL;
    }
    auto fence = createFence(cmdQueue);
    if (!fence) {
      IDP("Error: Asynchronous data retrieve failed -- invalid fence\n");
      return OFFLOAD_FAIL;
    }
    CALL_ZE_RET_FAIL(zeCommandListAppendMemoryCopy, cmdList, HstPtr, TgtPtr,
                     Size, nullptr, 0, nullptr);
    if (beginAsyncCommand(cmdList, cmdQueue,
        static_cast<AsyncEventTy *>(AsyncEvent), fence) == OFFLOAD_FAIL)
      return OFFLOAD_FAIL;
    IDP("Asynchronous data retrieve started -- %" PRId64 " bytes (tgt:"
       DPxMOD ") -> (hst:" DPxMOD ")\n", Size, DPxPTR(TgtPtr), DPxPTR(HstPtr));
  } else {
    if (copyData(DeviceId, HstPtr, TgtPtr, Size, copyLock) != OFFLOAD_SUCCESS)
      return OFFLOAD_FAIL;
    IDP("Copied %" PRId64 " bytes (tgt:" DPxMOD ") -> (hst:" DPxMOD ")\n", Size,
       DPxPTR(TgtPtr), DPxPTR(HstPtr));
  }

  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_data_retrieve(
    int32_t DeviceId, void *HstPtr, void *TgtPtr, int64_t Size) {
  return retrieveData(DeviceId, HstPtr, TgtPtr, Size, nullptr);
}

EXTERN int32_t __tgt_rtl_data_retrieve_async(
    int32_t DeviceId, void *HstPtr, void *TgtPtr, int64_t Size,
    __tgt_async_info *AsyncInfoPtr /*not used*/) {
  return retrieveData(DeviceId, HstPtr, TgtPtr, Size, nullptr);
}

EXTERN int32_t __tgt_rtl_data_retrieve_nowait(
    int32_t DeviceId, void *HstPtr, void *TgtPtr, int64_t Size,
    void *AsyncEvent) {
  return retrieveData(DeviceId, HstPtr, TgtPtr, Size, AsyncEvent);
}

EXTERN int32_t __tgt_rtl_data_delete(int32_t DeviceId, void *TgtPtr) {
#if !SUBDEVICE_USE_ROOT_MEMORY
  if (SubDeviceCode < 0 && SUBDEVICE_GET_COUNT(SubDeviceCode) == 1) {
    auto subLevel = SUBDEVICE_GET_LEVEL(SubDeviceCode);
    auto subStart = SUBDEVICE_GET_START(SubDeviceCode);
    DeviceId = DeviceInfo->SubDeviceIds[DeviceId][subLevel][subStart];
  }
#endif

  void *base = nullptr;
  size_t size = 0;

  auto &mutex = DeviceInfo->Mutexes[DeviceId];
  auto context = DeviceInfo->Context;

  mutex.lock();
  DeviceInfo->removeImplicitArgs(DeviceId, TgtPtr);
  mutex.unlock();

  if (DeviceInfo->Flags.UseMemoryPool) {
    auto &pool = DeviceInfo->PagePools[DeviceId];
    mutex.lock();
    bool deallocated = pool.deallocate(TgtPtr);
    mutex.unlock();
    if (deallocated) {
      IDP("Returned device memory " DPxMOD " to memory pool\n", DPxPTR(TgtPtr));
      return OFFLOAD_SUCCESS;
    }
  }
  CALL_ZE_RET_FAIL(zeMemGetAddressRange, context, TgtPtr, &base, &size);
  MEMSTAT_UPDATE(DeviceId, context, 0, base);
  CALL_ZE_RET_FAIL_MTX(zeMemFree, mutex, context, base);
  IDP("Deleted device memory " DPxMOD " (Base: " DPxMOD ", Size: %zu)\n",
     DPxPTR(TgtPtr), DPxPTR(base), size);
  return OFFLOAD_SUCCESS;
}

static void decideLoopKernelGroupArguments(
    int32_t DeviceId, uint32_t ThreadLimit, TgtNDRangeDescTy *LoopLevels,
    ze_kernel_handle_t Kernel, uint32_t *GroupSizes,
    ze_group_count_t &GroupCounts) {

  auto &computeProperties = DeviceInfo->ComputeProperties[DeviceId];
  uint32_t maxGroupSize = computeProperties.maxTotalGroupSize;
  uint32_t kernelWidth = DeviceInfo->KernelProperties[DeviceId][Kernel].Width;
  IDP("Assumed kernel SIMD width is %" PRIu32 "\n", kernelWidth);

  uint32_t kernelMaxThreadGroupSize =
      DeviceInfo->KernelProperties[DeviceId][Kernel].MaxThreadGroupSize;
  if (kernelMaxThreadGroupSize < maxGroupSize) {
    maxGroupSize = kernelMaxThreadGroupSize;
    IDP("Capping maximum thread group size to %" PRIu32
       " due to kernel constraints.\n", maxGroupSize);
  }

  bool maxGroupSizeForced = false;

  if (ThreadLimit > 0) {
    maxGroupSizeForced = true;

    if (ThreadLimit <= maxGroupSize) {
      maxGroupSize = ThreadLimit;
      IDP("Max group size is set to %" PRIu32 " (thread_limit clause)\n",
         maxGroupSize);
    } else {
      IDP("thread_limit(%" PRIu32 ") exceeds current maximum %"
         PRIu32 "\n", ThreadLimit, maxGroupSize);
    }
  }

  if (DeviceInfo->ThreadLimit > 0) {
    maxGroupSizeForced = true;

    if (DeviceInfo->ThreadLimit <= maxGroupSize) {
      maxGroupSize = DeviceInfo->ThreadLimit;
      IDP("Max group size is set to %" PRIu32 " (OMP_THREAD_LIMIT)\n",
         maxGroupSize);
    } else {
      IDP("OMP_THREAD_LIMIT(%" PRIu32 ") exceeds current maximum %"
         PRIu32 "\n", DeviceInfo->ThreadLimit, maxGroupSize);
    }
  }

  if (DeviceInfo->NumTeams > 0)
    IDP("OMP_NUM_TEAMS(%" PRIu32 ") is ignored\n", DeviceInfo->NumTeams);

  uint32_t groupCounts[3] = {1, 1, 1};
  uint32_t groupSizes[3] = {maxGroupSize, 1, 1};
  TgtLoopDescTy *level = LoopLevels->Levels;
  int32_t distributeDim = LoopLevels->DistributeDim;
  assert(distributeDim >= 0 && distributeDim <= 2 &&
         "Invalid distribute dimension.");
  int32_t numLoopLevels = LoopLevels->NumLoops;
  assert((numLoopLevels > 0 && numLoopLevels <= 3) &&
         "Invalid loop nest description for ND partitioning");

  // Compute global widths for X/Y/Z dimensions.
  uint32_t tripCounts[3] = {1, 1, 1};

  for (int32_t i = 0; i < numLoopLevels; i++) {
    assert(level[i].Stride > 0 && "Invalid loop stride for ND partitioning");
    IDP("Level %" PRIu32 ": Lb = %" PRId64 ", Ub = %" PRId64 ", Stride = %"
       PRId64 "\n", i, level[i].Lb, level[i].Ub, level[i].Stride);
    if (level[i].Ub < level[i].Lb) {
      GroupCounts.groupCountX = 1;
      GroupCounts.groupCountY = 1;
      GroupCounts.groupCountZ = 1;
      std::fill(GroupSizes, GroupSizes + 3, 1);
      return;
    }
    tripCounts[i] =
        (level[i].Ub - level[i].Lb + level[i].Stride) / level[i].Stride;
  }

  if (!maxGroupSizeForced) {
    // Use zeKernelSuggestGroupSize to compute group sizes,
    // or fallback to setting dimension 0 width to SIMDWidth.
    // Note that in case of user-specified LWS groupSizes[0]
    // is already set according to the specified value.
    uint32_t globalSizes[3] = { tripCounts[0], tripCounts[1], tripCounts[2] };
    if (distributeDim > 0) {
      // There is a distribute dimension.
      globalSizes[distributeDim - 1] *= globalSizes[distributeDim];
      globalSizes[distributeDim] = 1;
    }

    ze_result_t rc = ZE_RESULT_ERROR_UNKNOWN;
    uint32_t suggestedGroupSizes[3];
    if (DeviceInfo->Flags.UseDriverGroupSizes)
      CALL_ZE_RC(rc, zeKernelSuggestGroupSize,
                 Kernel, globalSizes[0], globalSizes[1], globalSizes[2],
                 &suggestedGroupSizes[0], &suggestedGroupSizes[1],
                 &suggestedGroupSizes[2]);

    if (rc == ZE_RESULT_SUCCESS) {
      groupSizes[0] = suggestedGroupSizes[0];
      groupSizes[1] = suggestedGroupSizes[1];
      groupSizes[2] = suggestedGroupSizes[2];
    } else if (maxGroupSize > kernelWidth) {
      groupSizes[0] = kernelWidth;
    }
  }

  for (int32_t i = 0; i < numLoopLevels; i++) {
    if (i < distributeDim) {
      groupCounts[i] = 1;
      continue;
    }
    uint32_t trip = tripCounts[i];
    if (groupSizes[i] >= trip)
      groupSizes[i] = trip;
    groupCounts[i] = (trip + groupSizes[i] - 1) / groupSizes[i];
  }

  GroupCounts.groupCountX = groupCounts[0];
  GroupCounts.groupCountY = groupCounts[1];
  GroupCounts.groupCountZ = groupCounts[2];
  std::copy(groupSizes, groupSizes + 3, GroupSizes);
}

static void decideKernelGroupArguments(
    int32_t DeviceId, uint32_t NumTeams, uint32_t ThreadLimit,
    ze_kernel_handle_t Kernel, uint32_t *GroupSizes,
    ze_group_count_t &GroupCounts) {

  auto &computeProperties = DeviceInfo->ComputeProperties[DeviceId];
  auto &deviceProperties = DeviceInfo->DeviceProperties[DeviceId];
  uint32_t maxGroupSize = computeProperties.maxTotalGroupSize;
  uint32_t numEUsPerSubslice = deviceProperties.numEUsPerSubslice;
  uint32_t numSubslices = deviceProperties.numSlices *
                          deviceProperties.numSubslicesPerSlice;
  uint32_t numThreadsPerEU = deviceProperties.numThreadsPerEU;
  bool maxGroupSizeForced = false;
  bool maxGroupCountForced = false;

  uint32_t kernelWidth = DeviceInfo->KernelProperties[DeviceId][Kernel].Width;
  IDP("Assumed kernel SIMD width is %" PRIu32 "\n", kernelWidth);

  uint32_t kernelMaxThreadGroupSize =
      DeviceInfo->KernelProperties[DeviceId][Kernel].MaxThreadGroupSize;
  if (kernelMaxThreadGroupSize < maxGroupSize) {
    maxGroupSize = kernelMaxThreadGroupSize;
    IDP("Capping maximum thread group size to %" PRIu32
       " due to kernel constraints.\n", maxGroupSize);
  }

  if (ThreadLimit > 0) {
    maxGroupSizeForced = true;

    if (ThreadLimit <= maxGroupSize) {
      maxGroupSize = ThreadLimit;
      IDP("Max group size is set to %" PRIu32 " (thread_limit clause)\n",
         maxGroupSize);
    } else {
      IDP("thread_limit(%" PRIu32 ") exceeds current maximum %"
         PRIu32 "\n", ThreadLimit, maxGroupSize);
    }
  }

  if (DeviceInfo->ThreadLimit > 0) {
    maxGroupSizeForced = true;

    if (DeviceInfo->ThreadLimit <= maxGroupSize) {
      maxGroupSize = DeviceInfo->ThreadLimit;
      IDP("Max group size is set to %" PRIu32 " (OMP_THREAD_LIMIT)\n",
         maxGroupSize);
    } else {
      IDP("OMP_THREAD_LIMIT(%" PRIu32 ") exceeds current maximum %"
         PRIu32 "\n", DeviceInfo->ThreadLimit, maxGroupSize);
    }
  }

  uint32_t maxGroupCount = 0;

  if (NumTeams > 0) {
    maxGroupCount = NumTeams;
    maxGroupCountForced = true;
    IDP("Max group count is set to %" PRIu32
       " (num_teams clause or no teams construct)\n", maxGroupCount);
  } else if (DeviceInfo->NumTeams > 0) {
    // OMP_NUM_TEAMS only matters, if num_teams() clause is absent.
    maxGroupCount = DeviceInfo->NumTeams;
    maxGroupCountForced = true;
    IDP("Max group count is set to %" PRIu32 " (OMP_NUM_TEAMS)\n",
       maxGroupCount);
  }

  if (maxGroupCountForced) {
    // If number of teams is specified by the user, then use kernelWidth
    // WIs per WG by default, so that it matches
    // decideLoopKernelGroupArguments() behavior.
    if (!maxGroupSizeForced) {
      maxGroupSize = kernelWidth;
    }
  } else {
    uint32_t numThreadsPerSubslice = numEUsPerSubslice * numThreadsPerEU;
    maxGroupCount = numSubslices * numThreadsPerSubslice;
    if (maxGroupSizeForced) {
      // Set group size for the HW capacity
      uint32_t numThreadsPerGroup =
          (maxGroupSize + kernelWidth - 1) / kernelWidth;
      uint32_t numGroupsPerSubslice =
          (numThreadsPerSubslice + numThreadsPerGroup - 1) / numThreadsPerGroup;
      maxGroupCount = numGroupsPerSubslice * numSubslices;
    } else {
      assert(!maxGroupSizeForced && !maxGroupCountForced);
      assert((maxGroupSize <= kernelWidth ||
             maxGroupSize % kernelWidth == 0) && "Invalid maxGroupSize");
      // Maximize group size
      while (maxGroupSize > kernelWidth) {
        uint32_t numThreadsPerGroup = maxGroupSize / kernelWidth;
        if (numThreadsPerSubslice % numThreadsPerGroup == 0) {
          uint32_t numGroupsPerSubslice =
              numThreadsPerSubslice / numThreadsPerGroup;
          maxGroupCount = numGroupsPerSubslice * numSubslices;
          break;
        }
        maxGroupSize -= kernelWidth;
      }
    }
  }

  uint32_t groupCounts[3] = {maxGroupCount, 1, 1};
  uint32_t groupSizes[3] = {maxGroupSize, 1, 1};
  if (!maxGroupCountForced)
    groupCounts[0] *= DeviceInfo->SubscriptionRate;

  GroupCounts.groupCountX = groupCounts[0];
  GroupCounts.groupCountY = groupCounts[1];
  GroupCounts.groupCountZ = groupCounts[2];
  std::copy(groupSizes, groupSizes + 3, GroupSizes);
}

static void forceGroupSizes(
    uint32_t *GroupSizes, ze_group_count_t &GroupCounts) {
#if INTEL_INTERNAL_BUILD
  // Use forced group sizes. This is only for internal experiments, and we
  // don't want to plug these numbers into the decision logic.
  auto userLWS = DeviceInfo->ForcedLocalSizes;
  auto userGWS = DeviceInfo->ForcedGlobalSizes;
  if (userLWS[0] > 0) {
    std::copy(userLWS, userLWS + 3, GroupSizes);
    IDP("Forced LWS = {%" PRIu32 ", %" PRIu32 ", %" PRIu32 "}\n", userLWS[0],
        userLWS[1], userLWS[2]);
  }
  if (userGWS[0] > 0) {
    GroupCounts.groupCountX = (userGWS[0] + GroupSizes[0] - 1) / GroupSizes[0];
    GroupCounts.groupCountY = (userGWS[1] + GroupSizes[1] - 1) / GroupSizes[1];
    GroupCounts.groupCountZ = (userGWS[2] + GroupSizes[2] - 1) / GroupSizes[2];
    IDP("Forced GWS = {%" PRIu32 ", %" PRIu32 ", %" PRIu32 "}\n", userGWS[0],
        userGWS[1], userGWS[2]);
  }
#endif // INTEL_INTERNAL_BUILD
}

static int32_t runTargetTeamRegionSub(
    int64_t DeviceIds, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit, void *LoopDesc,
    void *AsyncEvent) {

  // NOTE: We expect one sub device now (user data partitioning)
  //       We haven't discussed anything about partitioning by compiler
  //       We always get the same kernel for all sub devices

  uint32_t subLevel = SUBDEVICE_GET_LEVEL(DeviceIds);
  uint32_t subStart = SUBDEVICE_GET_START(DeviceIds);
  uint32_t subCount = SUBDEVICE_GET_COUNT(DeviceIds);
  uint32_t subStride = SUBDEVICE_GET_STRIDE(DeviceIds);
  uint32_t rootId = SUBDEVICE_GET_ROOT(DeviceIds);

  auto &subDeviceIds = DeviceInfo->SubDeviceIds[rootId];
  auto subIdBase = subDeviceIds[0][0]; // internal ID of the first subdevice

  std::vector<ze_event_handle_t> usedEvents;
  std::vector<ze_command_list_handle_t> usedCmdLists;

  ze_kernel_handle_t rootKernel = *(ze_kernel_handle_t *)TgtEntryPtr;
  uint32_t kernelId = 0;
  auto &rootEntries = DeviceInfo->FuncGblEntries[rootId].Entries;

  // Find kernel ID
  while (rootEntries[kernelId].addr != TgtEntryPtr &&
         kernelId < rootEntries.size())
    kernelId++;
  if (kernelId == rootEntries.size()) {
    IDP("Could not find a kernel for entry " DPxMOD " in the table\n",
        DPxPTR(TgtEntryPtr));
    return OFFLOAD_FAIL;
  }

  ze_event_handle_t profileEvent = nullptr;
  std::string tmName("Kernel#");
  tmName = tmName + rootEntries[kernelId].name;
  std::vector<ScopedTimerTy> tmKernels;

  if (DeviceInfo->Flags.EnableProfile) {
    for (uint32_t i = 0; i < subCount; i++) {
      auto subId = subDeviceIds[subLevel][subStart + i * subStride];
      tmKernels.emplace_back(ScopedTimerTy(subId, tmName));
    }
  }

  for (uint32_t i = 0; i < subCount; i++) {
    auto userId = subStart + i * subStride;
    auto subId = subDeviceIds[subLevel][userId];

#if SUBDEVICE_USE_ROOT_KERNELS
    std::unique_lock<std::mutex> kernelLock(DeviceInfo->Mutexes[rootId]);
    auto kernel = rootKernel;
#else // !SUBDEVICE_USE_ROOT_KERNELS
    std::unique_lock<std::mutex> kernelLock(DeviceInfo->Mutexes[subId]);
    auto kernel = DeviceInfo->FuncGblEntries[subId].Kernels[kernelId];
#endif // !SUBDEVICE_USE_ROOT_KERNELS

    for (int32_t argId = 0; argId < NumArgs; argId++) {
      void *arg;
      if (TgtOffsets[argId] == (std::numeric_limits<ptrdiff_t>::max)())
        arg = TgtArgs[argId];
      else
        arg = (void *)((intptr_t)TgtArgs[argId] + TgtOffsets[argId]);
      CALL_ZE_RET_FAIL(zeKernelSetArgumentValue, kernel, argId, sizeof(void *),
                       arg == nullptr ? nullptr : &arg);
      IDP("Kernel argument %" PRId32 " (value: " DPxMOD
          ") was set successfully for device %s.\n",
          argId, DPxPTR(arg), DeviceInfo->DeviceIdStr[subId].c_str());
    }

    auto flags = DeviceInfo->getKernelIndirectAccessFlags(rootKernel, rootId);
    CALL_ZE_RET_FAIL(zeKernelSetIndirectAccess, kernel, flags);
    IDP("Setting indirect access flags " DPxMOD "\n", DPxPTR(flags));

    // Decide group sizes and counts
    uint32_t groupSizes[3];
    ze_group_count_t groupCounts;
    if (LoopDesc) {
      decideLoopKernelGroupArguments(subId, (uint32_t)ThreadLimit,
          (TgtNDRangeDescTy *)LoopDesc, kernel, groupSizes, groupCounts);
    } else {
      decideKernelGroupArguments(subId, (uint32_t )NumTeams,
          (uint32_t)ThreadLimit, kernel, groupSizes, groupCounts);
    }

    forceGroupSizes(groupSizes, groupCounts);

    IDP("Group sizes = {%" PRIu32 ", %" PRIu32 ", %" PRIu32 "}\n",
        groupSizes[0], groupSizes[1], groupSizes[2]);
    IDP("Group counts = {%" PRIu32 ", %" PRIu32 ", %" PRIu32 "}\n",
        groupCounts.groupCountX, groupCounts.groupCountY,
        groupCounts.groupCountZ);

    auto cmdList = DeviceInfo->getCmdList(subId);
    auto cmdQueue = DeviceInfo->getCmdQueue(subId);

    // Only get device time for the last subdevice for now.
    if (DeviceInfo->Flags.EnableProfile && i == subCount - 1)
      profileEvent = DeviceInfo->ProfileEvents.getEvent();

    CALL_ZE_RET_FAIL(zeCommandListAppendLaunchKernel, cmdList, kernel,
                     &groupCounts, profileEvent, 0, nullptr);

    // Last event waits for other events
    auto event = DeviceInfo->SubDeviceEvents[rootId].Events[subId - subIdBase];
    if (i == subCount - 1)
      CALL_ZE_RET_FAIL(zeCommandListAppendBarrier, cmdList, event,
                       usedEvents.size(), usedEvents.data());
    else
      CALL_ZE_RET_FAIL(zeCommandListAppendBarrier, cmdList, event, 0, nullptr);

    CALL_ZE_RET_FAIL(zeCommandListClose, cmdList);
    CALL_ZE_RET_FAIL(zeCommandQueueExecuteCommandLists, cmdQueue, 1, &cmdList,
                     nullptr);

    IDP("Submitted kernel " DPxMOD " to subdevice %s\n", DPxPTR(kernel),
        DeviceInfo->DeviceIdStr[subId].c_str());

    kernelLock.unlock();

    usedCmdLists.push_back(cmdList);
    usedEvents.push_back(event);
  }

  CALL_ZE_RET_FAIL(zeEventHostSynchronize, usedEvents.back(), UINT64_MAX);

  if (DeviceInfo->Flags.EnableProfile && profileEvent)
    tmKernels.back().updateDeviceTime(profileEvent);

  for (uint32_t i = 0; i < subCount; i++) {
    CALL_ZE_RET_FAIL(zeCommandListReset, usedCmdLists[i]);
    CALL_ZE_RET_FAIL(zeEventHostReset, usedEvents[i]);
  }

  IDP("Executed kernel entry " DPxMOD " on subdevices\n", DPxPTR(TgtEntryPtr));
  return OFFLOAD_SUCCESS;
}

static int32_t runTargetTeamRegion(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit, void *LoopDesc,
    void *AsyncEvent) {
  assert(TgtEntryPtr && "Invalid kernel");
  assert((NumTeams >= 0 && ThreadLimit >= 0) && "Invalid kernel work size");
  IDP("Executing a kernel " DPxMOD "...\n", DPxPTR(TgtEntryPtr));

  if (SubDeviceCode < 0 && isValidSubDevice(SubDeviceCode))
    return runTargetTeamRegionSub(SubDeviceCode, TgtEntryPtr, TgtArgs,
                                  TgtOffsets, NumArgs, NumTeams, ThreadLimit,
                                  LoopDesc, AsyncEvent);

  // Protect from kernel preparation to submission as kernels are shared
  std::unique_lock<std::mutex> kernelLock(DeviceInfo->Mutexes[DeviceId]);

  ze_kernel_handle_t kernel = *((ze_kernel_handle_t *)TgtEntryPtr);
  if (!kernel) {
    REPORT("Failed to invoke deleted kernel.\n");
    return OFFLOAD_FAIL;
  }
  std::string tmName("Kernel#");
  size_t kernelNameSize = 0;
  CALL_ZE_RET_FAIL(zeKernelGetName, kernel, &kernelNameSize, nullptr);
  std::vector<char> kernelName(kernelNameSize);
  CALL_ZE_RET_FAIL(zeKernelGetName, kernel, &kernelNameSize, kernelName.data());
  ScopedTimerTy tmKernel(DeviceId, tmName + kernelName.data());

  // Set arguments
  std::vector<void *> args(NumArgs);
  for (int32_t i = 0; i < NumArgs; i++) {
    ptrdiff_t offset = TgtOffsets[i];
    // Offset equal to MAX(ptrdiff_t) means that the argument
    // must be passed as literal, and the offset should be ignored.
    if (offset == (std::numeric_limits<ptrdiff_t>::max)())
      args[i] = TgtArgs[i];
    else
      args[i] = (void *)((intptr_t)TgtArgs[i] + offset);
    CALL_ZE_RET_FAIL(zeKernelSetArgumentValue, kernel, i, sizeof(void *),
                     args[i] == nullptr ? nullptr : &args[i]);
    IDP("Kernel argument %" PRId32 " (value: " DPxMOD ") was set successfully\n",
       i, DPxPTR(args[i]));
  }

  auto flags = DeviceInfo->getKernelIndirectAccessFlags(kernel, DeviceId);
  CALL_ZE_RET_FAIL(zeKernelSetIndirectAccess, kernel, flags);
  IDP("Setting indirect access flags " DPxMOD "\n", DPxPTR(flags));

  // Decide group sizes and counts
  uint32_t groupSizes[3];
  ze_group_count_t groupCounts;
  if (LoopDesc) {
    decideLoopKernelGroupArguments(DeviceId, (uint32_t)ThreadLimit,
        (TgtNDRangeDescTy *)LoopDesc, kernel, groupSizes, groupCounts);
  } else {
    decideKernelGroupArguments(DeviceId, (uint32_t )NumTeams,
        (uint32_t)ThreadLimit, kernel, groupSizes, groupCounts);
  }

  forceGroupSizes(groupSizes, groupCounts);

  IDP("Group sizes = {%" PRIu32 ", %" PRIu32 ", %" PRIu32 "}\n",
     groupSizes[0], groupSizes[1], groupSizes[2]);
  IDP("Group counts = {%" PRIu32 ", %" PRIu32 ", %" PRIu32 "}\n",
     groupCounts.groupCountX, groupCounts.groupCountY, groupCounts.groupCountZ);

  if (OMPT_ENABLED) {
    // Push current work size
    size_t finalNumTeams = groupCounts.groupCountX * groupCounts.groupCountY *
        groupCounts.groupCountZ;
    size_t finalThreadLimit = groupSizes[0] * groupSizes[1] * groupSizes[2];
    OmptGlobal->getTrace().pushWorkSize(finalNumTeams, finalThreadLimit);
  }

  CALL_ZE_RET_FAIL(zeKernelSetGroupSize, kernel, groupSizes[0], groupSizes[1],
                   groupSizes[2]);
  auto context = DeviceInfo->Context;
  auto cmdList = DeviceInfo->getCmdList(DeviceId);
  auto cmdQueue = DeviceInfo->getCmdQueue(DeviceId);

  if (AsyncEvent) {
    cmdList = createCmdList(context, DeviceInfo->Devices[DeviceId],
                            DeviceInfo->DeviceIdStr[DeviceId]);
    if (!cmdList) {
      IDP("Error: Asynchronous execution failed -- invalid command list\n");
      return OFFLOAD_FAIL;
    }
    CALL_ZE_RET_FAIL(zeCommandListAppendLaunchKernel, cmdList, kernel,
                     &groupCounts, nullptr, 0, nullptr);
    auto fence = createFence(cmdQueue);
    if (!fence) {
      IDP("Error: Asynchronous execution failed -- invalid fence\n");
      return OFFLOAD_FAIL;
    }
    if (beginAsyncCommand(cmdList, cmdQueue,
        static_cast<AsyncEventTy *>(AsyncEvent), fence) == OFFLOAD_FAIL)
      return OFFLOAD_FAIL;
    IDP("Asynchronous execution started for kernel " DPxMOD "\n",
       DPxPTR(TgtEntryPtr));
  } else {
    ze_event_handle_t event = nullptr;
    if (DeviceInfo->Flags.EnableProfile)
      event = DeviceInfo->ProfileEvents.getEvent();
    CALL_ZE_RET_FAIL(zeCommandListAppendLaunchKernel, cmdList, kernel,
                     &groupCounts, event, 0, nullptr);
    CALL_ZE_RET_FAIL(zeCommandListAppendBarrier, cmdList, nullptr, 0, nullptr);

    CALL_ZE_RET_FAIL(zeCommandListClose, cmdList);
    CALL_ZE_RET_FAIL(zeCommandQueueExecuteCommandLists, cmdQueue, 1, &cmdList,
                     nullptr);
    kernelLock.unlock();
    CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, cmdQueue, UINT64_MAX);
    tmKernel.updateDeviceTime(event);
    // Make sure the command list is ready to accept next command
    CALL_ZE_RET_FAIL(zeCommandListReset, cmdList);
  }

  IDP("Executed a kernel " DPxMOD "\n", DPxPTR(TgtEntryPtr));
  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_run_target_team_nd_region(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit, void *LoopDesc) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, NumTeams, ThreadLimit, LoopDesc, nullptr);
}

EXTERN int32_t __tgt_rtl_run_target_team_nd_region_nowait(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit, void *LoopDesc,
    void *AsyncEvent) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, NumTeams, ThreadLimit, LoopDesc,
                             AsyncEvent);
}

EXTERN int32_t __tgt_rtl_run_target_team_region(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit,
    uint64_t LoopTripCount) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, NumTeams, ThreadLimit, nullptr, nullptr);
}

EXTERN int32_t __tgt_rtl_run_target_team_region_async(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit,
    uint64_t LoopTripCount, __tgt_async_info *AsyncInfoPtr /*not used*/) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, NumTeams, ThreadLimit, nullptr, nullptr);
}

EXTERN int32_t __tgt_rtl_run_target_team_region_nowait(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit,
    uint64_t LoopTripCount, void *AsyncEvent) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, NumTeams, ThreadLimit, nullptr,
                             AsyncEvent);
}

EXTERN int32_t __tgt_rtl_run_target_region(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, 1, 0, nullptr, nullptr);
}

EXTERN int32_t __tgt_rtl_run_target_region_async(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, __tgt_async_info *AsyncInfoPtr /*not used*/) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, 1, 0, nullptr, nullptr);
}

EXTERN int32_t __tgt_rtl_run_target_region_nowait(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, void *AsyncEvent) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, 1, 0, nullptr, AsyncEvent);
}

EXTERN int32_t __tgt_rtl_manifest_data_for_region(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtPtrs, size_t NumPtrs) {
  if (NumPtrs == 0)
    return OFFLOAD_SUCCESS;
  // We only have a binary switch.
  ze_kernel_handle_t kernel = *static_cast<ze_kernel_handle_t *>(TgtEntryPtr);
  std::unique_lock<std::mutex> dataLock(DeviceInfo->Mutexes[DeviceId]);
  DeviceInfo->KernelProperties[DeviceId][kernel].RequiresIndirectAccess = true;
  return OFFLOAD_SUCCESS;
}

EXTERN void *__tgt_rtl_create_offload_queue(int32_t DeviceId, bool IsAsync) {
  // Create and return a new command queue for interop
  // TODO: check with MKL team and decide what to do with IsAsync
  ze_command_queue_handle_t cmdQueue =
      createCmdQueue(DeviceInfo->Context, DeviceInfo->Devices[DeviceId],
                     DeviceInfo->DeviceIdStr[DeviceId]);
  IDP("%s returns a new asynchronous command queue " DPxMOD "\n", __func__,
     DPxPTR(cmdQueue));
  return cmdQueue;
}

EXTERN int32_t __tgt_rtl_release_offload_queue(int32_t DeviceId, void *Queue) {
  CALL_ZE_RET_FAIL(zeCommandQueueDestroy, (ze_command_queue_handle_t)Queue);
  return OFFLOAD_SUCCESS;
}

EXTERN void *__tgt_rtl_get_platform_handle(int32_t DeviceId) {
  auto driver = DeviceInfo->Driver;
  return (void *)driver;
}

EXTERN void *__tgt_rtl_get_device_handle(int32_t DeviceId) {
  auto device = DeviceInfo->Devices[DeviceId];
  return (void *)device;
}

EXTERN void *__tgt_rtl_get_context_handle() {
  auto context = DeviceInfo->Context;
  return (void *)context;
}

EXTERN int32_t __tgt_rtl_synchronize(
    int32_t DeviceId, __tgt_async_info *async_info_ptr) {
  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_push_subdevice(int64_t DeviceIds) {
  // Unsupported subdevice request is ignored
  if (!isValidSubDevice(DeviceIds))
    IDP("Warning: Invalid subdevice encoding " DPxMOD " is ignored\n",
        DPxPTR(DeviceIds));
  else
    SubDeviceCode = DeviceIds;
  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_pop_subdevice() {
  SubDeviceCode = 0;
  return OFFLOAD_SUCCESS;
}

void *RTLDeviceInfoTy::getOffloadVarDeviceAddr(
    int32_t DeviceId, const char *Name, size_t Size) {
  IDP("Looking up OpenMP global variable '%s' of size %zu bytes on device %d.\n",
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
      IDP("Global variable '%s' found in the offload table at position %zu.\n",
         Name, std::distance(OffloadTable.begin(), I));
      return I->Base.addr;
    }

    IDP("Warning: global variable '%s' was not found in the offload table.\n",
       Name);
  } else
    IDP("Warning: offload table is not loaded for device %d.\n", DeviceId);

  // Fallback to the lookup by name.
  return getVarDeviceAddr(DeviceId, Name, Size);
}

void *RTLDeviceInfoTy::getVarDeviceAddr(
    int32_t DeviceId, const char *Name, size_t Size) {
  void *TgtAddr = nullptr;
  size_t TgtSize = 0;
  IDP(
     "Looking up device global variable '%s' of size %zu bytes on device %d.\n",
     Name, Size, DeviceId);
  CALL_ZE_RET_NULL(zeModuleGetGlobalPointer,
                   FuncGblEntries[DeviceId].Modules[0], Name, &TgtSize,
                   &TgtAddr);
  if (Size != TgtSize) {
    IDP("Warning: requested size %zu does not match %zu\n", Size, TgtSize);
#if 0
    // FIXME: when L0 reports correct size.
    return nullptr;
#endif
  }
  IDP("Global variable lookup succeeded.\n");
  return TgtAddr;
}

bool RTLDeviceInfoTy::loadOffloadTable(int32_t DeviceId, size_t NumEntries) {
  const char *OffloadTableSizeVarName = "__omp_offloading_entries_table_size";
  void *OffloadTableSizeVarAddr =
      getVarDeviceAddr(DeviceId, OffloadTableSizeVarName, sizeof(int64_t));

  if (!OffloadTableSizeVarAddr) {
    IDP("Warning: cannot get device value for global variable '%s'.\n",
       OffloadTableSizeVarName);
    return false;
  }

  int64_t TableSizeVal = 0;
  __tgt_rtl_data_retrieve(DeviceId, &TableSizeVal,
                          OffloadTableSizeVarAddr, sizeof(int64_t));
  size_t TableSize = (size_t)TableSizeVal;

  if ((TableSize % sizeof(DeviceOffloadEntryTy)) != 0) {
    IDP("Warning: offload table size (%zu) is not a multiple of %zu.\n",
       TableSize, sizeof(DeviceOffloadEntryTy));
    return false;
  }

  size_t DeviceNumEntries = TableSize / sizeof(DeviceOffloadEntryTy);

  if (NumEntries != DeviceNumEntries) {
    IDP("Warning: number of entries in host and device "
       "offload tables mismatch (%zu != %zu).\n",
       NumEntries, DeviceNumEntries);
  }

  const char *OffloadTableVarName = "__omp_offloading_entries_table";
  void *OffloadTableVarAddr =
      getVarDeviceAddr(DeviceId, OffloadTableVarName, TableSize);
  if (!OffloadTableVarAddr) {
    IDP("Warning: cannot get device value for global variable '%s'.\n",
       OffloadTableVarName);
    return false;
  }

  OffloadTables[DeviceId].resize(DeviceNumEntries);
  __tgt_rtl_data_retrieve(DeviceId, OffloadTables[DeviceId].data(),
                          OffloadTableVarAddr, TableSize);
  std::vector<DeviceOffloadEntryTy> &DeviceTable = OffloadTables[DeviceId];

  size_t I = 0;
  const char *PreviousName = "";
  bool PreviousIsVar = false;

  for (; I < DeviceNumEntries; ++I) {
    DeviceOffloadEntryTy &Entry = DeviceTable[I];
    size_t NameSize = Entry.NameSize;
    void *NameTgtAddr = Entry.Base.name;
    Entry.Base.name = nullptr;

    if (NameSize == 0) {
      IDP("Warning: offload entry (%zu) with 0 size.\n", I);
      break;
    }

    Entry.Base.name = new char[NameSize];
    __tgt_rtl_data_retrieve(DeviceId, Entry.Base.name,
                            NameTgtAddr, NameSize);
    if (strnlen(Entry.Base.name, NameSize) != NameSize - 1) {
      IDP("Warning: offload entry's name has wrong size.\n");
      break;
    }

    int Cmp = strncmp(PreviousName, Entry.Base.name, NameSize);
    if (Cmp > 0) {
      IDP("Warning: offload table is not sorted.\n"
         "Warning: previous name is '%s'.\n"
         "Warning:  current name is '%s'.\n",
         PreviousName, Entry.Base.name);
      break;
    } else if (Cmp == 0 && (PreviousIsVar || Entry.Base.addr)) {
      // The names are equal. This should never happen for
      // offload variables, but we allow this for offload functions.
      IDP("Warning: duplicate names (%s) in offload table.\n", PreviousName);
      break;
    }
    PreviousName = Entry.Base.name;
    PreviousIsVar = (Entry.Base.addr != nullptr);
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
    IDP("Device offload table loaded:\n");
    for (size_t I = 0; I < DeviceNumEntries; ++I)
      IDP("\t%zu:\t%s\n", I, DeviceTable[I].Base.name);
  }

  return true;
}

void RTLDeviceInfoTy::unloadOffloadTable(int32_t DeviceId) {
  for (auto &E : OffloadTables[DeviceId])
    delete[] E.Base.name;

  OffloadTables[DeviceId].clear();
}
#endif // INTEL_CUSTOMIZATION

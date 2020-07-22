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
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#else
#include <unistd.h>
#endif // !_WIN32
#include "omptargetplugin.h"
#include "omptarget-tools.h"
#include "rtl-trace.h"

/// Host runtime routines being used
extern "C" {
#ifdef _WIN32
int __cdecl omp_get_max_teams(void);
int __cdecl omp_get_thread_limit(void);
double __cdecl omp_get_wtime(void);
#else
int omp_get_max_teams(void) __attribute__((weak));
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
#define LEVEL0_PAGE_SIZE (1 << 16) // L0 memory allocation unit

int DebugLevel = 0;

#if INTEL_INTERNAL_BUILD
/// Memory stats
struct MemStatTy {
  size_t Requested = 0; // Requested bytes
  size_t Allocated = 0; // Allocated bytes by L0
  size_t Freed = 0; // Freed bytes by L0
  size_t InUse = 0; // Current memory in use by L0
  size_t PeakUse = 0; // Peak bytes in use by L0
  void update(ze_driver_handle_t Driver, size_t requested, void *Mem) {
    void *base = nullptr;
    size_t size = 0;
    CALL_ZE_EXIT_FAIL(zeDriverGetMemAddressRange, Driver, Mem, &base, &size);
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
    DP("Memory usage:\n");
    DP("-- Requested = %12zu\n", Requested);
    DP("-- Allocated = %12zu\n", Allocated);
    DP("-- Freed     = %12zu\n", Freed);
    DP("-- InUse     = %12zu\n", InUse);
    DP("-- PeakUse   = %12zu\n", PeakUse);
  }
}; // MemStatTy
// Per-device memory stats
static std::vector<MemStatTy> MemStats;
#define MEMSTAT_UPDATE(DeviceId, Driver, Requested, Mem)                       \
  do {                                                                         \
    if (DebugLevel > 0)                                                        \
      MemStats[DeviceId].update(Driver, Requested, Mem);                       \
  } while (0)
#define MEMSTAT_PRINT(DeviceId)                                                \
  do {                                                                         \
    if (DebugLevel > 0)                                                        \
      MemStats[DeviceId].print();                                              \
  } while (0)
#else // INTEL_INTERNAL_BUILD
#define MEMSTAT_UPDATE(DeviceId, Driver, Requested, Mem)
#define MEMSTAT_PRINT(DeviceId)
#endif // INTEL_INTERNAL_BUILD

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
  ze_driver_handle_t Driver = nullptr;
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
  void initialize(int32_t deviceId, ze_driver_handle_t driver,
                  ze_device_handle_t device, int32_t allocKind) {
    Buckets.resize(MaxSize - MinSize + 1);
    DeviceId = deviceId;
    Driver = driver;
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
    void *base = __tgt_rtl_data_alloc_explicit(
        DeviceId, LEVEL0_PAGE_SIZE, TargetAllocKind);
    DP("New allocation from page pool: base = " DPxMOD ", size = %" PRIu32 "\n",
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
        MEMSTAT_UPDATE(DeviceId, Driver, 0, (void *)page.Base);
        CALL_ZE_EXIT_FAIL(zeDriverFreeMem, Driver, (void *)page.Base);
      }
    }
  }
}; // PagePoolTy

/// Per-device global entry table
struct FuncOrGblEntryTy {
  __tgt_target_table Table;
  std::vector<__tgt_offload_entry> Entries;
  std::vector<ze_kernel_handle_t> Kernels;
  ze_module_handle_t Module = nullptr;
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

/// Each thread should be able to handle multiple devices
thread_local std::map<int32_t, PrivateHandlesTy> ThreadLocalHandles;

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
  uint64_t DumpTargetImage : 1;
  uint64_t EnableProfile : 1;
  uint64_t EnableTargetGlobals : 1;
  uint64_t UseHostMemForUSM : 1;
  uint64_t UseMemoryPool : 1;
  uint64_t UseDriverGroupSizes : 1;
  uint64_t Reserved : 58;
  RTLFlagsTy() :
      DumpTargetImage(0),
      EnableProfile(0),
      EnableTargetGlobals(0),
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

  ze_driver_handle_t Driver;
  std::vector<ze_device_properties_t> DeviceProperties;
  std::vector<ze_device_compute_properties_t> ComputeProperties;

  std::vector<ze_device_handle_t> Devices;
  // Use per-thread command list/queue
  std::vector<std::vector<ze_command_list_handle_t>> CmdLists;
  std::vector<std::vector<ze_command_queue_handle_t>> CmdQueues;
  std::vector<FuncOrGblEntryTy> FuncGblEntries;
  std::vector<std::map<ze_kernel_handle_t, KernelPropertiesTy>>
      KernelProperties;
  std::vector<std::vector<void *>> OwnedMemory; // Memory owned by the plugin
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
        DP("ENV: %s=%s\n", Name, value);
      return value;
    }
    value = std::getenv(OldName);
    if (value) {
      DP("ENV: %s=%s\n", OldName, value);
      WARNING("%s is being deprecated. Use %s instead.\n", OldName, Name);
    }
    return value;
  }

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
      if (value == "GPU" || value == "gpu")
        DeviceType = ZE_DEVICE_TYPE_GPU;
      else
        WARNING("Invalid LIBOMPTARGET_DEVICETYPE=%s\n", env);
    }
    DP("Target device type is set to GPU\n");

    // Global thread limit
    int threadLimit = omp_get_thread_limit();
    DP("omp_get_thread_limit() returned %" PRId32 "\n", threadLimit);
    // omp_get_thread_limit() would return INT_MAX by default.
    // NOTE: Windows.h defines max() macro, so we have to guard
    //       the call with parentheses.
    ThreadLimit = (threadLimit > 0 &&
        threadLimit != (std::numeric_limits<int32_t>::max)()) ?
        threadLimit : 0;

    // Global max number of teams.
    int numTeams = omp_get_max_teams();
    DP("omp_get_max_teams() returned %" PRId32 "\n", numTeams);
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
      if (env && (env[0] == 'T' || env[0] == 't' || env[0] == '1')) {
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
  }

  ze_command_list_handle_t getCmdList(int32_t DeviceId) {
    if (ThreadLocalHandles.count(DeviceId) == 0) {
      ThreadLocalHandles.emplace(DeviceId, PrivateHandlesTy());
    }
    if (!ThreadLocalHandles[DeviceId].CmdList) {
      auto cmdList = createCmdList(Devices[DeviceId]);
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
      auto cmdQueue = createCmdQueue(Devices[DeviceId]);
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
      ThreadLocalHandles[DeviceId].Profile = new RTLProfileTy();
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
  DP("Init Level0 plugin!\n");
  DeviceInfo = new RTLDeviceInfoTy();
}

ATTRIBUTE(destructor(101)) void deinit() {
  DP("Deinit Level0 plugin!\n");
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
        profile->printData(i, DeviceInfo->DeviceProperties[i].name);
        delete profile;
      }
    }
    if (omptEnabled.enabled) {
      OMPT_CALLBACK(ompt_callback_device_unload, i, 0 /* module ID */);
      OMPT_CALLBACK(ompt_callback_device_finalize, i);
    }
#ifndef _WIN32
    DeviceInfo->Mutexes[i].lock();
    for (auto mem : DeviceInfo->OwnedMemory[i]) {
      MEMSTAT_UPDATE(i, DeviceInfo->Driver, 0, mem);
      CALL_ZE_EXIT_FAIL(zeDriverFreeMem, DeviceInfo->Driver, mem);
    }
    for (auto cmdQueue : DeviceInfo->CmdQueues[i])
      CALL_ZE_EXIT_FAIL(zeCommandQueueDestroy, cmdQueue);
    for (auto cmdList : DeviceInfo->CmdLists[i])
      CALL_ZE_EXIT_FAIL(zeCommandListDestroy, cmdList);
    for (auto kernel : DeviceInfo->FuncGblEntries[i].Kernels) {
      if (kernel)
        CALL_ZE_EXIT_FAIL(zeKernelDestroy, kernel);
    }
    if (DeviceInfo->FuncGblEntries[i].Module)
      CALL_ZE_EXIT_FAIL(zeModuleDestroy, DeviceInfo->FuncGblEntries[i].Module);
    if (DeviceInfo->Flags.UseMemoryPool)
      DeviceInfo->PagePools[i].clear();
    DeviceInfo->Mutexes[i].unlock();
#endif
    if (DeviceInfo->Flags.EnableTargetGlobals)
      DeviceInfo->unloadOffloadTable(i);
    MEMSTAT_PRINT(i);
  }
  delete[] DeviceInfo->Mutexes;
  delete[] DeviceInfo->DataMutexes;
  DP("Closed RTL successfully\n");
}

static int32_t copyData(int32_t DeviceId, void *Dest, void *Src, size_t Size,
                        std::unique_lock<std::mutex> &copyLock) {
  // Mtx == nullptr means we already own the lock.
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
    CALL_ZE_RET_FAIL(
        zeCommandListAppendMemoryCopy, cmdList, Dest, Src, Size, nullptr);
    CALL_ZE_RET_FAIL(zeCommandListClose, cmdList);
    CALL_ZE_RET_FAIL(
        zeCommandQueueExecuteCommandLists, cmdQueue, 1, &cmdList, nullptr);
    if (ownsLock)
      copyLock.unlock();
    CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, cmdQueue, UINT32_MAX);
    CALL_ZE_RET_FAIL(zeCommandListReset, cmdList);
  }
  return OFFLOAD_SUCCESS;
}

/// Initialize module data
static int32_t initModule(int32_t DeviceId) {
  // Prepare host data to copy
  ModuleDataTy hostData = {
    1,                               // Initialized
    (int32_t)DeviceInfo->NumDevices, // Number of devices
    DeviceId                         // Device ID
  };

  // Prepare device data location
  auto driver = DeviceInfo->Driver;
  void *deviceData = nullptr;

  std::unique_lock<std::mutex> kernelLock(DeviceInfo->Mutexes[DeviceId]);

  if (DeviceInfo->Flags.UseMemoryPool)
    deviceData = DeviceInfo->PagePools[DeviceId].allocate(sizeof(hostData));

  if (!deviceData)
    deviceData = __tgt_rtl_data_alloc_explicit(
        DeviceId, sizeof(hostData), DeviceInfo->TargetAllocKind);

  // Prepare kernel to initialize data
  ze_kernel_desc_t kernelDesc = {
    ZE_KERNEL_DESC_VERSION_CURRENT,
    ZE_KERNEL_FLAG_NONE,
    "__kmpc_init_program"
  };
  auto module = DeviceInfo->FuncGblEntries[DeviceId].Module;
  ze_kernel_handle_t initModuleData;
  CALL_ZE_RET_FAIL(zeKernelCreate, module, &kernelDesc, &initModuleData);
  CALL_ZE_RET_FAIL(zeKernelSetArgumentValue, initModuleData, 0,
                   sizeof(deviceData), &deviceData);
  ze_group_count_t groupCounts = {1, 1, 1};
  CALL_ZE_RET_FAIL(zeKernelSetGroupSize, initModuleData, 1, 1, 1);

  // Copy data
  int32_t rc = copyData(DeviceId, deviceData, &hostData, sizeof(hostData),
                        kernelLock);
  if (rc != OFFLOAD_SUCCESS)
    return OFFLOAD_FAIL;

  // Invoke the kernel
  auto cmdList = DeviceInfo->getCmdList(DeviceId);
  auto cmdQueue = DeviceInfo->getCmdQueue(DeviceId);
  CALL_ZE_RET_FAIL(zeCommandListAppendLaunchKernel, cmdList, initModuleData,
                   &groupCounts, nullptr, 0, nullptr);
  CALL_ZE_RET_FAIL(zeCommandListAppendBarrier, cmdList, nullptr, 0, nullptr);
  CALL_ZE_RET_FAIL(zeCommandListClose, cmdList);
  CALL_ZE_RET_FAIL(zeCommandQueueExecuteCommandLists, cmdQueue, 1, &cmdList,
                   nullptr);
  CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, cmdQueue, UINT32_MAX);
  CALL_ZE_RET_FAIL(zeCommandListReset, cmdList);
  CALL_ZE_RET_FAIL(zeKernelDestroy, initModuleData);
  if (!DeviceInfo->Flags.UseMemoryPool) {
    MEMSTAT_UPDATE(DeviceId, driver, 0, deviceData);
    CALL_ZE_RET_FAIL(zeDriverFreeMem, driver, deviceData);
  }

  return OFFLOAD_SUCCESS;
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
    DPI("Error creating temporary file template name.\n");
    return;
  }
  int TmpFileFd;
  _sopen_s(&TmpFileFd, TmpFileName, _O_RDWR | _O_CREAT | _O_BINARY,
           _SH_DENYNO, _S_IREAD | _S_IWRITE);
#else  // !_WIN32
  int TmpFileFd = mkstemp(TmpFileName);
#endif  // !_WIN32
  DPI("Dumping %s image of size %d from address " DPxMOD " to file %s\n",
      Type, static_cast<int32_t>(ImageSize), DPxPTR(Image), TmpFileName);

  if (TmpFileFd < 0) {
    DPI("Error creating temporary file: %s\n", strerror(errno));
    return;
  }

#if _WIN32
  int WErr = _write(TmpFileFd, Image, ImageSize);
#else  // !_WIN32
  int WErr = write(TmpFileFd, Image, ImageSize);
#endif  // !_WIN32
  if (WErr < 0) {
    DPI("Error writing temporary file %s: %s\n", TmpFileName, strerror(errno));
  }

#if _WIN32
  int CloseErr = _close(TmpFileFd);
#else  // !_WIN32
  int CloseErr = close(TmpFileFd);
#endif  // !_WIN32
  if (CloseErr < 0) {
    DPI("Error closing temporary file %s: %s\n", TmpFileName, strerror(errno));
  }
#endif  // INTEL_INTERNAL_BUILD
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
  DeviceInfo->RequiresFlags = RequiresFlags;
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
  DP("Looking for device type %" PRId32 "...\n", DeviceInfo->DeviceType);

  for (uint32_t i = 0; i < numDrivers; i++) {
    // Check available devices
    uint32_t numDevices = 0;
    CALL_ZE_RET_ZERO(zeDeviceGet, driverHandles[i], &numDevices, nullptr);
    if (numDevices == 0) {
      DP("Cannot find any devices for driver %" PRIu32 "!\n", i);
      continue;
    }

    // Get device handles and check device type
    std::vector<ze_device_handle_t> devices(numDevices);
    CALL_ZE_RET_ZERO(zeDeviceGet, driverHandles[i], &numDevices,
                     devices.data());

    for (auto device : devices) {
      ze_device_properties_t properties = {};
      properties.version = ZE_DEVICE_PROPERTIES_VERSION_CURRENT;
      CALL_ZE_RET_ZERO(zeDeviceGetProperties, device, &properties);
      if (properties.type != DeviceInfo->DeviceType) {
        DP("Skipping device type %" PRId32 "...\n", properties.type);
        continue;
      }
      DP("Found a device, Type = %" PRId32 ", Name = %s\n",
         properties.type, properties.name);

      ze_device_compute_properties_t computeProperties = {};
      computeProperties.version = ZE_DEVICE_COMPUTE_PROPERTIES_VERSION_CURRENT;
      CALL_ZE_RET_ZERO(zeDeviceGetComputeProperties, device,
                       &computeProperties);

      DeviceInfo->Devices.push_back(device);
      DeviceInfo->DeviceProperties.push_back(properties);
      DeviceInfo->ComputeProperties.push_back(computeProperties);
    }

    DeviceInfo->Driver = driverHandles[i];
    DeviceInfo->NumDevices += numDevices;
    DP("Found %" PRIu32 " available devices.\n", DeviceInfo->NumDevices);
    break;
  }

  DeviceInfo->CmdLists.resize(DeviceInfo->NumDevices);
  DeviceInfo->CmdQueues.resize(DeviceInfo->NumDevices);
  DeviceInfo->FuncGblEntries.resize(DeviceInfo->NumDevices);
  DeviceInfo->KernelProperties.resize(DeviceInfo->NumDevices);
  DeviceInfo->OwnedMemory.resize(DeviceInfo->NumDevices);
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

#ifndef _WIN32
  if (std::atexit(closeRTL)) {
    FATAL_ERROR("Registration of clean-up function");
  }
#endif // _WIN32

  if (DeviceInfo->NumDevices > 0) {
    omptInitPlugin();
  }

  return DeviceInfo->NumDevices;
}

EXTERN
int32_t __tgt_rtl_init_device(int32_t DeviceId) {
  if (DeviceId < 0 || (uint32_t)DeviceId >= DeviceInfo->NumDevices) {
    DP("Bad device ID %" PRId32 "\n", DeviceId);
    return OFFLOAD_FAIL;
  }

  if (DeviceInfo->Flags.UseMemoryPool)
    DeviceInfo->PagePools[DeviceId].initialize(DeviceId, DeviceInfo->Driver,
        DeviceInfo->Devices[DeviceId], DeviceInfo->TargetAllocKind);

  DeviceInfo->Initialized[DeviceId] = true;

  OMPT_CALLBACK(ompt_callback_device_initialize, DeviceId,
                DeviceInfo->DeviceProperties[DeviceId].name,
                DeviceInfo->Devices[DeviceId],
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

  std::string compilationOptions(DeviceInfo->CompilationOptions);
  DP("Module compilation options: %s\n", compilationOptions.c_str());
  compilationOptions += " " + DeviceInfo->InternalCompilationOptions;
  DPI("Final module compilation options: %s\n", compilationOptions.c_str());

  dumpImageToFile(Image->ImageStart, imageSize, "OpenMP");

  ze_module_desc_t moduleDesc = {
    ZE_MODULE_DESC_VERSION_CURRENT,
    ZE_MODULE_FORMAT_IL_SPIRV,
    imageSize,
    (uint8_t *)Image->ImageStart,
    compilationOptions.c_str(),
    nullptr /* pointer to specialization constants */
  };
  ze_module_handle_t module;
  ze_module_build_log_handle_t buildLog;
  ScopedTimerTy tmModuleBuild(DeviceId, "ModuleBuild");
  ze_result_t rc;
  CALL_ZE_RC(rc, zeModuleCreate, DeviceInfo->Devices[DeviceId], &moduleDesc,
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

  DeviceInfo->FuncGblEntries[DeviceId].Module = module;
  auto &entries = DeviceInfo->FuncGblEntries[DeviceId].Entries;
  auto &kernels = DeviceInfo->FuncGblEntries[DeviceId].Kernels;
  entries.resize(numEntries);
  kernels.resize(numEntries);

  if (DeviceInfo->Flags.EnableTargetGlobals &&
      !DeviceInfo->loadOffloadTable(DeviceId, numEntries))
    DP("Error: offload table loading failed.\n");

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

    // Retrieve kernel group size info.
    // L0 does not have API for accessing kernel's sub group size, so it is
    // decided from kernel property's "requiredGroupSize"; it returns values
    // that appear to be sub group size in most cases, but can also return other
    // values that do not belong to the supported sub group sizes.
    auto &computeProperties = DeviceInfo->ComputeProperties[DeviceId];
    auto &subGroupSizes = computeProperties.subGroupSizes;
    auto numSubGroupSizes = computeProperties.numSubGroupSizes;
    ze_kernel_properties_t kernelProperties;
    uint32_t rc;
    CALL_ZE(rc, zeKernelGetProperties, kernels[i], &kernelProperties);
    if (DeviceInfo->ForcedKernelWidth > 0) {
      DeviceInfo->KernelProperties[DeviceId][kernels[i]].Width =
          DeviceInfo->ForcedKernelWidth;
    } else {
      uint32_t kernelWidth = subGroupSizes[numSubGroupSizes / 2]; // default
      if (rc == ZE_RESULT_SUCCESS) {
        auto requiredGroupSize = kernelProperties.requiredGroupSizeX;
        for (uint32_t k = numSubGroupSizes - 1; k >= 0; k--) {
          // Choose the greatest sub group size that divides the required group
          // size evenly
          if (requiredGroupSize > 0 &&
              requiredGroupSize % subGroupSizes[k] == 0) {
            kernelWidth = subGroupSizes[k];
            break;
          }
        }
      }
      DeviceInfo->KernelProperties[DeviceId][kernels[i]].Width = kernelWidth;
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

  if (initModule(DeviceId) != OFFLOAD_SUCCESS)
    return nullptr;
  __tgt_target_table &table = DeviceInfo->FuncGblEntries[DeviceId].Table;
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
    DP("Allocated target memory " DPxMOD " (Base: " DPxMOD ", Size: %zu) "
       "from memory pool for host ptr " DPxMOD "\n",
       DPxPTR(mem), DPxPTR(base), size, DPxPTR(HstPtr));
    return mem;
  }

  // We use shared USM to avoid overheads coming from explicit data copy to
  // device memory.
  base = __tgt_rtl_data_alloc_explicit(
      DeviceId, size, DeviceInfo->TargetAllocKind);
  mem = (void *)((intptr_t)base + offset);

  if (DebugLevel > 0) {
    void *actualBase = nullptr;
    size_t actualSize = 0;
    CALL_ZE_RET_NULL(zeDriverGetMemAddressRange, DeviceInfo->Driver, mem,
                     &actualBase, &actualSize);
    assert(base == actualBase && "Invalid memory address range!");
    DP("Allocated target memory " DPxMOD " (Base: " DPxMOD
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
  int32_t kind = DeviceInfo->Flags.UseHostMemForUSM ? TARGET_ALLOC_HOST
                                                    : TARGET_ALLOC_SHARED;
  return __tgt_rtl_data_alloc_explicit(DeviceId, Size, kind);
}

EXTERN int32_t __tgt_rtl_data_delete_managed(int32_t DeviceId, void *Ptr) {
  return __tgt_rtl_data_delete(DeviceId, Ptr);
}

EXTERN int32_t __tgt_rtl_is_managed_ptr(int32_t DeviceId, void *Ptr) {
  ze_memory_allocation_properties_t properties = {
    ZE_MEMORY_ALLOCATION_PROPERTIES_VERSION_CURRENT,
    ZE_MEMORY_TYPE_UNKNOWN,
    0
  };
  ze_result_t rc;
  CALL_ZE(rc, zeDriverGetMemAllocProperties, DeviceInfo->Driver, Ptr,
          &properties, nullptr);
  // Host pointer is classified as an error -- skip error reporting
  if (rc == ZE_RESULT_ERROR_INVALID_ARGUMENT)
    return 0;
  CALL_ZE_RET_ZERO(zeDriverGetMemAllocProperties, DeviceInfo->Driver, Ptr,
                   &properties, nullptr);
  int32_t ret = (properties.type == ZE_MEMORY_TYPE_HOST ||
                 properties.type == ZE_MEMORY_TYPE_SHARED);
  DP("Ptr " DPxMOD " is %sa managed memory pointer.\n", DPxPTR(Ptr),
     ret ? "" : "not ");
  return ret;
}

EXTERN void *__tgt_rtl_data_alloc_explicit(
    int32_t DeviceId, int64_t Size, int32_t Kind) {
  void *mem = nullptr;
  ze_device_mem_alloc_desc_t deviceDesc = {
    ZE_DEVICE_MEM_ALLOC_DESC_VERSION_CURRENT,
    ZE_DEVICE_MEM_ALLOC_FLAG_DEFAULT,
    0
  };
  ze_host_mem_alloc_desc_t hostDesc = {
    ZE_HOST_MEM_ALLOC_DESC_VERSION_CURRENT,
    ZE_HOST_MEM_ALLOC_FLAG_DEFAULT
  };
  auto driver = DeviceInfo->Driver;
  auto device = DeviceInfo->Devices[DeviceId];

  switch (Kind) {
  case TARGET_ALLOC_DEVICE:
    CALL_ZE_RET_NULL(zeDriverAllocDeviceMem, driver, &deviceDesc, Size,
                     LEVEL0_ALIGNMENT, device, &mem);
    DP("Allocated a device memory object " DPxMOD "\n", DPxPTR(mem));
    break;
  case TARGET_ALLOC_HOST:
    CALL_ZE_RET_NULL(zeDriverAllocHostMem, driver, &hostDesc, Size,
                     LEVEL0_ALIGNMENT, &mem);
    DP("Allocated a host memory object " DPxMOD "\n", DPxPTR(mem));
    break;
  case TARGET_ALLOC_SHARED:
    CALL_ZE_RET_NULL(zeDriverAllocSharedMem, driver, &deviceDesc, &hostDesc,
                     Size, LEVEL0_ALIGNMENT, device, &mem);
    DP("Allocated a shared memory object " DPxMOD "\n", DPxPTR(mem));
    break;
  default:
    FATAL_ERROR("Invalid target data allocation kind");
  }

  MEMSTAT_UPDATE(DeviceId, driver, Size, mem);

  return mem;
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

  std::unique_lock<std::mutex> copyLock(DeviceInfo->Mutexes[DeviceId],
                                        std::defer_lock);

  if (AsyncEvent) {
    copyLock.lock();
    auto cmdList = createCmdList(DeviceInfo->Devices[DeviceId]);
    auto cmdQueue = DeviceInfo->getCmdQueue(DeviceId);
    if (!cmdList) {
      DP("Error: Asynchronous data submit failed -- invalid command list\n");
      return OFFLOAD_FAIL;
    }
    auto fence = createFence(cmdQueue);
    if (!fence) {
      DP("Error: Asynchronous data submit failed -- invalid fence\n");
      return OFFLOAD_FAIL;
    }
    CALL_ZE_RET_FAIL(zeCommandListAppendMemoryCopy, cmdList, TgtPtr, HstPtr,
                     Size, nullptr);
    if (beginAsyncCommand(cmdList, cmdQueue,
        static_cast<AsyncEventTy *>(AsyncEvent), fence) == OFFLOAD_FAIL)
      return OFFLOAD_FAIL;
    DP("Asynchronous data submit started -- %" PRId64 " bytes (hst:"
       DPxMOD ") -> (tgt:" DPxMOD ")\n", Size, DPxPTR(HstPtr), DPxPTR(TgtPtr));
  } else {
    if (copyData(DeviceId, TgtPtr, HstPtr, Size, copyLock) != OFFLOAD_SUCCESS)
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

  std::unique_lock<std::mutex> copyLock(DeviceInfo->Mutexes[DeviceId],
                                        std::defer_lock);

  if (AsyncEvent) {
    copyLock.lock();
    auto cmdList = createCmdList(DeviceInfo->Devices[DeviceId]);
    auto cmdQueue = DeviceInfo->getCmdQueue(DeviceId);
    if (!cmdList) {
      DP("Error: Asynchronous data retrieve failed -- invalid command list\n");
      return OFFLOAD_FAIL;
    }
    auto fence = createFence(cmdQueue);
    if (!fence) {
      DP("Error: Asynchronous data retrieve failed -- invalid fence\n");
      return OFFLOAD_FAIL;
    }
    CALL_ZE_RET_FAIL(zeCommandListAppendMemoryCopy, cmdList, HstPtr, TgtPtr,
                     Size, nullptr);
    if (beginAsyncCommand(cmdList, cmdQueue,
        static_cast<AsyncEventTy *>(AsyncEvent), fence) == OFFLOAD_FAIL)
      return OFFLOAD_FAIL;
    DP("Asynchronous data retrieve started -- %" PRId64 " bytes (tgt:"
       DPxMOD ") -> (hst:" DPxMOD ")\n", Size, DPxPTR(TgtPtr), DPxPTR(HstPtr));
  } else {
    if (copyData(DeviceId, HstPtr, TgtPtr, Size, copyLock) != OFFLOAD_SUCCESS)
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
  if (DeviceInfo->Flags.UseMemoryPool) {
    auto &pool = DeviceInfo->PagePools[DeviceId];
    std::unique_lock<std::mutex> deallocLock(DeviceInfo->Mutexes[DeviceId]);
    if (pool.deallocate(TgtPtr)) {
      DP("Returned device memory " DPxMOD " to memory pool\n", DPxPTR(TgtPtr));
      return OFFLOAD_SUCCESS;
    }
  }
  CALL_ZE_RET_FAIL(zeDriverGetMemAddressRange, DeviceInfo->Driver, TgtPtr, &base,
                   &size);
  MEMSTAT_UPDATE(DeviceId, DeviceInfo->Driver, 0, base);
  CALL_ZE_RET_FAIL_MTX(zeDriverFreeMem, DeviceInfo->Mutexes[DeviceId],
                       DeviceInfo->Driver, base);
  DP("Deleted device memory " DPxMOD " (Base: " DPxMOD ", Size: %zu)\n",
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
  DP("Assumed kernel SIMD width is %" PRIu32 "\n", kernelWidth);

  uint32_t kernelMaxThreadGroupSize =
      DeviceInfo->KernelProperties[DeviceId][Kernel].MaxThreadGroupSize;
  if (kernelMaxThreadGroupSize < maxGroupSize) {
    maxGroupSize = kernelMaxThreadGroupSize;
    DP("Capping maximum thread group size to %" PRIu32
       " due to kernel constraints.\n", maxGroupSize);
  }

  bool maxGroupSizeForced = false;

  if (ThreadLimit > 0) {
    maxGroupSizeForced = true;

    if (ThreadLimit <= maxGroupSize) {
      maxGroupSize = ThreadLimit;
      DP("Max group size is set to %" PRIu32 " (thread_limit clause)\n",
         maxGroupSize);
    } else {
      DP("thread_limit(%" PRIu32 ") exceeds current maximum %"
         PRIu32 "\n", ThreadLimit, maxGroupSize);
    }
  }

  if (DeviceInfo->ThreadLimit > 0) {
    maxGroupSizeForced = true;

    if (DeviceInfo->ThreadLimit <= maxGroupSize) {
      maxGroupSize = DeviceInfo->ThreadLimit;
      DP("Max group size is set to %" PRIu32 " (OMP_THREAD_LIMIT)\n",
         maxGroupSize);
    } else {
      DP("OMP_THREAD_LIMIT(%" PRIu32 ") exceeds current maximum %"
         PRIu32 "\n", DeviceInfo->ThreadLimit, maxGroupSize);
    }
  }

  if (DeviceInfo->NumTeams > 0)
    DP("OMP_NUM_TEAMS(%" PRIu32 ") is ignored\n", DeviceInfo->NumTeams);

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
    assert((level[i].Ub >= level[i].Lb && level[i].Stride > 0) &&
           "Invalid loop nest description for ND partitioning");
    DP("Level %" PRIu32 ": Lb = %" PRId64 ", Ub = %" PRId64 ", Stride = %"
       PRId64 "\n", i, level[i].Lb, level[i].Ub, level[i].Stride);
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
  DP("Assumed kernel SIMD width is %" PRIu32 "\n", kernelWidth);

  uint32_t kernelMaxThreadGroupSize =
      DeviceInfo->KernelProperties[DeviceId][Kernel].MaxThreadGroupSize;
  if (kernelMaxThreadGroupSize < maxGroupSize) {
    maxGroupSize = kernelMaxThreadGroupSize;
    DP("Capping maximum thread group size to %" PRIu32
       " due to kernel constraints.\n", maxGroupSize);
  }

  if (ThreadLimit > 0) {
    maxGroupSizeForced = true;

    if (ThreadLimit <= maxGroupSize) {
      maxGroupSize = ThreadLimit;
      DP("Max group size is set to %" PRIu32 " (thread_limit clause)\n",
         maxGroupSize);
    } else {
      DP("thread_limit(%" PRIu32 ") exceeds current maximum %"
         PRIu32 "\n", ThreadLimit, maxGroupSize);
    }
  }

  if (DeviceInfo->ThreadLimit > 0) {
    maxGroupSizeForced = true;

    if (DeviceInfo->ThreadLimit <= maxGroupSize) {
      maxGroupSize = DeviceInfo->ThreadLimit;
      DP("Max group size is set to %" PRIu32 " (OMP_THREAD_LIMIT)\n",
         maxGroupSize);
    } else {
      DP("OMP_THREAD_LIMIT(%" PRIu32 ") exceeds current maximum %"
         PRIu32 "\n", DeviceInfo->ThreadLimit, maxGroupSize);
    }
  }

  uint32_t maxGroupCount = 0;

  if (NumTeams > 0) {
    maxGroupCount = NumTeams;
    maxGroupCountForced = true;
    DP("Max group count is set to %" PRIu32
       " (num_teams clause or no teams construct)\n", maxGroupCount);
  } else if (DeviceInfo->NumTeams > 0) {
    // OMP_NUM_TEAMS only matters, if num_teams() clause is absent.
    maxGroupCount = DeviceInfo->NumTeams;
    maxGroupCountForced = true;
    DP("Max group count is set to %" PRIu32 " (OMP_NUM_TEAMS)\n",
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

static int32_t runTargetTeamRegion(int32_t DeviceId, void *TgtEntryPtr,
                                   void **TgtArgs, ptrdiff_t *TgtOffsets,
                                   int32_t NumArgs, int32_t NumTeams,
                                   int32_t ThreadLimit, void *LoopDesc,
                                   void *AsyncEvent) {
  assert(TgtEntryPtr && "Invalid kernel");
  assert((NumTeams >= 0 && ThreadLimit >= 0) && "Invalid kernel work size");
  DP("Executing a kernel " DPxMOD "...\n", DPxPTR(TgtEntryPtr));

  // Protect from kernel preparation to submission as kernels are shared
  std::unique_lock<std::mutex> kernelLock(DeviceInfo->Mutexes[DeviceId]);

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
                     args[i] == nullptr ? nullptr : &args[i]);
    DP("Kernel argument %" PRId32 " (value: " DPxMOD ") was set successfully\n",
       i, DPxPTR(args[i]));
  }

  // Set attributes
  bool indirectAccess =
      DeviceInfo->KernelProperties[DeviceId][kernel].RequiresIndirectAccess;
  if (indirectAccess) {
    ze_kernel_attribute_t attribute =
        DeviceInfo->TargetAllocKind == TARGET_ALLOC_SHARED
        ? ZE_KERNEL_ATTR_INDIRECT_SHARED_ACCESS
        : ZE_KERNEL_ATTR_INDIRECT_DEVICE_ACCESS;
    CALL_ZE_RET_FAIL(zeKernelSetAttribute, kernel, attribute, sizeof(bool),
                     &indirectAccess);
  }

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
  DP("Group sizes = {%" PRIu32 ", %" PRIu32 ", %" PRIu32 "}\n",
     groupSizes[0], groupSizes[1], groupSizes[2]);
  DP("Group counts = {%" PRIu32 ", %" PRIu32 ", %" PRIu32 "}\n",
     groupCounts.groupCountX, groupCounts.groupCountY, groupCounts.groupCountZ);

  if (omptEnabled.enabled) {
    // Push current work size
    size_t finalNumTeams = groupCounts.groupCountX * groupCounts.groupCountY *
        groupCounts.groupCountZ;
    size_t finalThreadLimit = groupSizes[0] * groupSizes[1] * groupSizes[2];
    omptTracePtr->pushWorkSize(finalNumTeams, finalThreadLimit);
  }

  CALL_ZE_RET_FAIL(zeKernelSetGroupSize, kernel, groupSizes[0], groupSizes[1],
                   groupSizes[2]);
  auto cmdList = DeviceInfo->getCmdList(DeviceId);
  auto cmdQueue = DeviceInfo->getCmdQueue(DeviceId);

  if (AsyncEvent) {
    cmdList = createCmdList(DeviceInfo->Devices[DeviceId]);
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
  ze_command_queue_desc_t cmdQueueDesc = {
    ZE_COMMAND_QUEUE_DESC_VERSION_CURRENT,
    ZE_COMMAND_QUEUE_FLAG_NONE,
    ZE_COMMAND_QUEUE_MODE_ASYNCHRONOUS,
    ZE_COMMAND_QUEUE_PRIORITY_NORMAL,
    0 // ordinal
  };
  // TODO: check with MKL team and decide what to do with IsAsync

  ze_command_queue_handle_t queue = nullptr;
  CALL_ZE_RET_NULL(zeCommandQueueCreate, DeviceInfo->Devices[DeviceId],
                   &cmdQueueDesc, &queue);
  DP("%s returns a new asynchronous command queue " DPxMOD "\n", __func__,
     DPxPTR(queue));
  return queue;
}

EXTERN int32_t __tgt_rtl_release_offload_queue(int32_t DeviceId, void *Pipe) {
  CALL_ZE_RET_FAIL(zeCommandQueueDestroy, (ze_command_queue_handle_t)Pipe);
  return OFFLOAD_SUCCESS;
}

EXTERN void *__tgt_rtl_get_platform_handle(int32_t DeviceId) {
  auto driver = DeviceInfo->Driver;
  return (void *) driver;
}

EXTERN void *__tgt_rtl_get_device_handle(int32_t DeviceId) {
  auto device = DeviceInfo->Devices[DeviceId];
  return (void *) device;
}


EXTERN void *__tgt_rtl_create_buffer(int32_t DeviceId, void *TgtPtr) {
  return TgtPtr;
}

EXTERN int32_t __tgt_rtl_release_buffer(void *TgtPtr) {
  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_synchronize(int32_t DeviceId,
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
  bool PreviousIsVar = false;

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

    int Cmp = strncmp(PreviousName, Entry.Base.name, NameSize);
    if (Cmp > 0) {
      DP("Error: offload table is not sorted.\n"
         "Error: previous name is '%s'.\n"
         "Error:  current name is '%s'.\n",
         PreviousName, Entry.Base.name);
      break;
    } else if (Cmp == 0 && (PreviousIsVar || Entry.Base.addr)) {
      // The names are equal. This should never happen for
      // offload variables, but we allow this for offload functions.
      DP("Error: duplicate names (%s) in offload table.\n", PreviousName);
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

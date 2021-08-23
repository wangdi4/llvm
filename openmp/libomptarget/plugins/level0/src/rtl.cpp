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
#include <list>
#include <mutex>
#include <set>
#include <string>
#include <sstream>
#include <thread>
#include <unordered_map>
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

#include "elf_light.h"
#include "omptargetplugin.h"
#include "omptarget-tools.h"
#include "rtl-trace.h"

#include "llvm/Support/Endian.h"

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

/// Default alignmnet for allocation
#define LEVEL0_ALIGNMENT 0
/// Default staging buffer size for host to device copy
#define LEVEL0_STAGING_BUFFER_SIZE (1 << 12)
/// Default staging buffer count
#define LEVEL0_STAGING_BUFFER_COUNT 64

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

/// Misc. macros
#define LOG_MEM_USAGE(Device, Requested, Ptr)                                  \
do {                                                                           \
  if (DebugLevel > 0)                                                          \
    logMemUsage(Device, Requested, Ptr);                                       \
} while (0)

#define LOG_MEM_USAGE_POOL(Device, Requested, Ptr, Size)                       \
do {                                                                           \
  if (DebugLevel > 0)                                                          \
    logMemUsage(Device, Requested, Ptr, Size);                                 \
} while (0)

#define ALLOC_KIND_TO_STR(Kind)                                                \
  (Kind == TARGET_ALLOC_HOST ? "host memory"                                   \
      : (Kind == TARGET_ALLOC_SHARED ? "shared memory"                         \
      : (Kind == TARGET_ALLOC_DEVICE ? "device memory" : "unknown memory")))

/// Device type enumeration common to compiler and runtime
enum DeviceArch : uint64_t {
  DeviceArch_None   = 0,
  DeviceArch_Gen9   = 0x0001,
  DeviceArch_XeLP   = 0x0002,
  DeviceArch_XeHP   = 0x0004,
  DeviceArch_x86_64 = 0x0100
};

/// Mapping from device arch to GPU runtime's device identifiers
std::map<uint64_t, std::vector<uint32_t>> DeviceArchMap {
  {
    DeviceArch_Gen9, {
      0x1900, // SKL
      0x5900, // KBL
      0x3E00, 0x9B00, // CFL
    }
  },
  {
    DeviceArch_XeLP, {
      0xFF20, 0x9A00, // TGL
      0x4900, // DG1
      0x4C00, // RKL
      0x4600, // ADLS
    }
  },
  {
    DeviceArch_XeHP, {
      0x0200, // ATS
    }
  }
};

/// Interop support
namespace L0Interop {
  // ID and names from openmp.org
  const int32_t Vendor = 8;
  const char *VendorName = GETNAME(intel);
  const int32_t FrId = 6;
  const char *FrName = GETNAME(level_zero);

  // targetsync = -9, device_context = -8, ...,  fr_id = -1
  std::vector<const char *> IprNames {
    "targetsync",
    "device_context",
    "device",
    "platform",
    "device_num",
    "vendor_name",
    "vendor",
    "fr_name",
    "fr_id",
    "device_num_eus",
    "device_num_threads_per_eu",
    "device_eu_simd_width",
    "device_num_eus_per_subslice",
    "device_num_subslices_per_slice",
    "device_num_slices"
  };

  std::vector<const char *> IprTypeDescs {
    "ze_command_queue_handle_t, level_zero command queue handle",
    "ze_context_handle_t, level_zero context handle",
    "ze_device_handle_t, level_zero device handle",
    "ze_driver_handle_t, level_zero driver handle",
    "intptr_t, OpenMP device ID",
    "const char *, vendor name",
    "intptr_t, vendor ID",
    "const char *, foreign runtime name",
    "intptr_t, foreign runtime ID",
    "intptr_t, total number of EUs",
    "intptr_t, number of threads per EU",
    "intptr_t, physical EU simd width",
    "intptr_t, number of EUs per sub-slice",
    "intptr_t, number of sub-slices per slice",
    "intptr_t, number of slices"
  };

  // Level Zero property ID
  enum IprIDTy : int32_t {
    device_num_eus = 0,
    device_num_threads_per_eu,
    device_eu_simd_width,
    device_num_eus_per_subslice,
    device_num_subslices_per_slice,
    device_num_slices
  };

  /// Level Zero interop property
  struct Property {
    // TODO: define implementation-defined properties
  };

  /// Dump implementation-defined properties
  static void printInteropProperties(void) {
    DP("Interop property IDs, Names, Descriptions\n");
    for (int I = -omp_ipr_first; I < (int)IprNames.size(); I++)
      DP("-- %" PRId32 ", %s, %s\n", I + omp_ipr_first, IprNames[I],
         IprTypeDescs[I]);
  }
}

/// Staging buffer
/// A single staging buffer is not enough when batching is enabled since there
/// can be multiple pending copy operations.
class StagingBufferTy {
  ze_context_handle_t Context = nullptr;
  size_t Size = LEVEL0_STAGING_BUFFER_SIZE;
  size_t Count = LEVEL0_STAGING_BUFFER_COUNT;
  std::vector<void *> Buffers;
  size_t Offset = 0;

  void *addBuffers() {
    ze_host_mem_alloc_desc_t AllocDesc = {
      ZE_STRUCTURE_TYPE_HOST_MEM_ALLOC_DESC, nullptr, 0
    };
    void *Ret = nullptr;
    CALL_ZE_RET_NULL(zeMemAllocHost, Context, &AllocDesc, Size * Count,
                     LEVEL0_ALIGNMENT, &Ret);
    Buffers.push_back(Ret);
    return Ret;
  }

public:
  ~StagingBufferTy() {
    ze_result_t Rc;
    for (auto Ptr : Buffers)
      CALL_ZE(Rc, zeMemFree, Context, Ptr);
  }

  bool initialized() { return Context != nullptr; }

  void init(ze_context_handle_t _Context, size_t _Size, size_t _Count) {
    Context = _Context;
    Size = _Size;
    Count = _Count;
  }

  void reset() { Offset = 0; }

  /// Always return the first buffer
  void *get() {
    if (Size == 0 || Count == 0)
      return nullptr;
    if (Buffers.empty())
      return addBuffers();
    else
      return Buffers[0];
  }

  /// Return the next available buffer
  void *getNext() {
    void *Ret = nullptr;
    if (Size == 0 || Count == 0)
      return Ret;
    if (Buffers.empty() || Offset >= Buffers.size() * Size * Count) {
      Ret = addBuffers();
      if (!Ret)
        return Ret;
    } else {
      Ret = (void *)((uintptr_t)Buffers.back() + (Offset % (Size * Count)));
    }
    Offset += Size;
    return Ret;
  }
};

/// Command batch manager
class CommandBatchTy {
  struct MemCopyTy {
    void *Dst;
    const void *Src;
    size_t Size;
    MemCopyTy(void *_Dst, const void *_Src, size_t _Size) :
        Dst(_Dst), Src(_Src), Size(_Size) {}
  };

  /// For device-to-host copy with staging buffer
  std::list<MemCopyTy> MemCopyList;

  /// For delayed data delete
  std::list<void *> MemFreeList;

  /// Internal device ID
  int32_t DeviceId = -1;

  /// Current batch state
  /// State increments when batch begins and decrements when it ends
  int32_t State = 0;

  /// Number of enqueued copy commands
  uint32_t NumCopyTo = 0;
  uint32_t NumCopyFrom = 0;

  /// Kernel information
  ze_kernel_handle_t Kernel = nullptr;
  ze_event_handle_t KernelEvent = nullptr;

  /// Command list/queue
  ze_command_list_handle_t CmdList = nullptr;
  ze_command_queue_handle_t CmdQueue = nullptr;

public:
  int32_t begin(int32_t DeviceId);

  int32_t end();

  int32_t commit(bool Always = false);

  int32_t enqueueMemCopyTo(int32_t DeviceId, void *Dst, void *Src, size_t Size);

  int32_t enqueueMemCopyFrom(int32_t DeviceId, void *Dst, void *Src,
                             size_t Size);

  int32_t enqueueLaunchKernel(int32_t DeviceId, ze_kernel_handle_t Kernel,
                              ze_group_count_t *GroupCounts);

  int32_t enqueueMemFree(int32_t DeviceId, void *Ptr);

  bool isActive() { return State > 0; }
};

class RTLProfileTy;

/// All thread-local data used by RTL
class TLSTy {
  /// Command list for each device
  std::map<int32_t, ze_command_list_handle_t> CmdLists;

  /// Copy command list for each device
  std::map<int32_t, ze_command_list_handle_t> CopyCmdLists;

  /// Command queue for each device
  std::map<int32_t, ze_command_queue_handle_t> CmdQueues;

  /// Copy command queue for each device
  std::map<int32_t, ze_command_queue_handle_t> CopyCmdQueues;

  /// Run profile for each device
  std::map<int32_t, RTLProfileTy *> Profiles;

  /// Staging buffer
  StagingBufferTy StagingBuffer;

  /// Batch manager
  CommandBatchTy CommandBatch;

  /// Subdevice encoding
  int64_t SubDeviceCode = 0;

public:
  ~TLSTy();

  ze_command_list_handle_t getCmdList(int32_t ID) {
    return (CmdLists.count(ID) > 0) ? CmdLists.at(ID) : nullptr;
  }

  ze_command_list_handle_t getCopyCmdList(int32_t ID) {
    return (CopyCmdLists.count(ID) > 0) ? CopyCmdLists.at(ID) : nullptr;
  }

  ze_command_queue_handle_t getCmdQueue(int32_t ID) {
    return (CmdQueues.count(ID) > 0) ? CmdQueues.at(ID) : nullptr;
  }

  ze_command_queue_handle_t getCopyCmdQueue(int32_t ID) {
    return (CopyCmdQueues.count(ID) > 0) ? CopyCmdQueues.at(ID) : nullptr;
  }

  RTLProfileTy *getProfile(int32_t ID) {
    return (Profiles.count(ID) > 0) ? Profiles.at(ID) : nullptr;
  }

  int64_t getSubDeviceCode() { return SubDeviceCode; }

  StagingBufferTy &getStagingBuffer() { return StagingBuffer; }

  CommandBatchTy &getCommandBatch() { return CommandBatch; }

  void setCmdList(int32_t ID, ze_command_list_handle_t CmdList) {
    CmdLists[ID] = CmdList;
  }

  void setCopyCmdList(int32_t ID, ze_command_list_handle_t CmdList) {
    CopyCmdLists[ID] = CmdList;
  }

  void setCmdQueue(int32_t ID, ze_command_queue_handle_t CmdQueue) {
    CmdQueues[ID] = CmdQueue;
  }

  void setCopyCmdQueue(int32_t ID, ze_command_queue_handle_t CmdQueue) {
    CopyCmdQueues[ID] = CmdQueue;
  }

  void setProfile(int32_t ID, RTLProfileTy *Profile) {
    Profiles[ID] = Profile;
  }

  void setSubDeviceCode(int64_t Code) { SubDeviceCode = Code; }
};

/// Global list for clean-up
std::list<TLSTy *> *TLSList = nullptr;

/// Returns thread-local storage while adding a new instance to the global list.
static TLSTy *getTLS() {
  static thread_local TLSTy *TLS = nullptr;
  static std::mutex Mtx;
  if (TLS)
    return TLS;
  TLS = new TLSTy();
  std::lock_guard<std::mutex> Lock(Mtx);
  TLSList->push_back(TLS);
  return TLS;
}

/// Get initialized ZE structure
template <typename T, ze_structure_type_t SType>
static T getZEStructure() {
  T Ret;
  Ret.stype = SType;
  Ret.pNext = nullptr;
  return Ret;
}
const auto CommandQueueGroupPropertiesInit =
    getZEStructure<ze_command_queue_group_properties_t,
                   ZE_STRUCTURE_TYPE_COMMAND_QUEUE_GROUP_PROPERTIES>();
const auto DevicePropertiesInit =
    getZEStructure<ze_device_properties_t,
                   ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES>();
const auto DeviceComputePropertiesInit =
    getZEStructure<ze_device_compute_properties_t,
                   ZE_STRUCTURE_TYPE_DEVICE_COMPUTE_PROPERTIES>();
const auto KernelPropertiesInit =
    getZEStructure<ze_kernel_properties_t,
                   ZE_STRUCTURE_TYPE_KERNEL_PROPERTIES>();

int DebugLevel = getDebugLevel();

/// Forward declarations
static void *allocDataExplicit(ze_device_handle_t Device, int64_t Size,
                               int32_t Kind, bool LargeMem = false,
                               bool LogMemAlloc = true);
static void logMemUsage(ze_device_handle_t Device, size_t Requested, void *Ptr,
                        size_t MemSize = 0);
class RTLDeviceInfoTy;

/// Memory pool which enables reuse of already allocated blocks
/// -- Pool maintains a list of buckets each of which can allocate fixed-size
///    memory.
/// -- Each bucket maintains a list of memory blocks allocated by GPU RT.
/// -- Each memory block can allocate multiple fixed-size memory requested by
///    offload RT or user.
/// -- Memory allocation falls back to GPU RT allocation when the pool size
///    (total memory used by pool) reaches a threshold.
class MemoryPoolTy {

  /// Memory block maintained in each bucket
  struct BlockTy {
    /// Base adddress of this block
    uintptr_t Base = 0;

    /// Size of the block
    uint32_t Size = 0;

    /// Supported allocation size by this block
    uint32_t ChunkSize = 0;

    /// Number of slots in use
    uint32_t NumUsedSlots = 0;

    /// Free slot returned by the last dealloc() call
    uint32_t FreeSlot = UINT32_MAX;

    /// Marker for the currently used slots
    std::vector<bool> UsedSlots;

    BlockTy(void *base, uint32_t size, uint32_t chunkSize) {
      Base = reinterpret_cast<uintptr_t>(base);
      Size = size;
      ChunkSize = chunkSize;
      NumUsedSlots = 0;
      UsedSlots.resize(Size / ChunkSize, false);
    }

    bool isFull() { return NumUsedSlots == Size / ChunkSize; }

    bool contains(void *Mem) {
      auto mem = reinterpret_cast<uintptr_t>(Mem);
      return mem >= Base && mem < Base + Size;
    }

    /// Allocate memory from this block
    void *alloc() {
      if (isFull())
        return nullptr;

      if (FreeSlot != UINT32_MAX) {
        auto slot = FreeSlot;
        FreeSlot = UINT32_MAX;
        UsedSlots[slot] = true;
        NumUsedSlots++;
        return reinterpret_cast<void *>(Base + slot * ChunkSize);
      }

      auto numSlots = Size / ChunkSize;
      for (uint32_t i = 0; i < numSlots; i++) {
        if (UsedSlots[i])
          continue;
        UsedSlots[i] = true;
        NumUsedSlots++;
        return reinterpret_cast<void *>(Base + i * ChunkSize);
      }

      // Should not reach here.
      FATAL_ERROR("Inconsistent state while allocating memory from pool");
    }

    /// Return the memory to the block
    void dealloc(void *Mem) {
      if (!contains(Mem))
        FATAL_ERROR("Inconsistent state while returning memory to pool");
      uint32_t slot = (reinterpret_cast<uintptr_t>(Mem) - Base) / ChunkSize;
      UsedSlots[slot] = false;
      NumUsedSlots--;
      FreeSlot = slot;
    }
  }; /// BlockTy

  /// Initialized
  int32_t Initialized = 0;

  /// Allocation unit size decided by the GPU RT
  size_t AllocUnit = 0;

  /// Minimum supported memory allocation size from pool
  size_t AllocMin = 1 << 5; // 32B

  /// Maximum supported memory allocation size from pool
  size_t AllocMax = 0;

  /// Capacity of each block in the buckets which decides number of allocatable
  /// chunks from the block. Each block in the bucket can serve at least
  /// BlockCapacity chunks.
  /// If ChunkSize * BlockCapacity <= AllocUnit
  ///   BlockSize = AllocUnit
  /// Otherwise,
  ///   BlockSize = ChunkSize * BlockCapacity
  /// This simply means how much memory is over-allocated.
  uint32_t BlockCapacity = 0;

  /// Total memory in use from allocation by GPU RT
  size_t PoolSize = 0;

  /// Maximum memory the pool can manage
  size_t PoolSizeMax = 0;

  /// Memory buckets for each allocatable size
  std::vector<std::vector<BlockTy *>> Buckets;

  /// Bucket paramters
  std::vector<std::pair<size_t, size_t>> BucketParams;

  /// Memory allocated from pool
  std::unordered_map<intptr_t, BlockTy *> PtrToBlock;

  /// Associated L0 device ID
  ze_device_handle_t Device = nullptr;

  /// Associated context
  ze_context_handle_t Context = nullptr;

  /// Associated allocation kind
  int32_t AllocKind = TARGET_ALLOC_DEFAULT;

  /// Pool lock
  std::mutex PoolMtx;

  uint32_t getBucketId(size_t Size) {
    uint32_t count = 0;
    for (size_t sz = AllocMin; sz < Size; count++)
      sz <<= 1;
    return count;
  }

public:
  MemoryPoolTy() = default;

  /// Construct with device
  MemoryPoolTy(ze_device_handle_t device) : Device(device) {}

  /// Initialize the pool with parameters
  void init(int32_t AllocKind, RTLDeviceInfoTy *RTL);

  /// Allocate memory from the pool with Size and Offset
  void *alloc(size_t Size, intptr_t Offset = 0) {
    if (!Initialized)
      return nullptr;

    std::unique_lock<std::mutex> lock(PoolMtx);

    if (Size == 0 || Size > AllocMax)
      return nullptr;

    uint32_t bucketId = getBucketId(Size);
    void *mem = nullptr;
    auto &blocks = Buckets[bucketId];

    for (auto block : blocks) {
      if (block->isFull())
        continue;

      mem = block->alloc();

      if (mem == nullptr)
        FATAL_ERROR("Inconsistent state while allocating memory from pool");

      PtrToBlock.emplace(reinterpret_cast<intptr_t>(mem) + Offset, block);
    }

    if (mem == nullptr) {
      // Check if current pool size exceeded max pool size
      if (PoolSize > PoolSizeMax)
        return nullptr;
      // Bucket is empty or all blocks in the bucket are full
      auto chunkSize = BucketParams[bucketId].first;
      auto blockSize = BucketParams[bucketId].second;
      void *base = allocDataExplicit(Device, blockSize, AllocKind);

      BlockTy *block = new BlockTy(base, blockSize, chunkSize);
      blocks.push_back(block);
      mem = block->alloc();
      PtrToBlock.emplace(reinterpret_cast<intptr_t>(mem) + Offset, block);
      PoolSize += blockSize;
      DP("New block allocation for %s pool: base = " DPxMOD
         ", size = %zu, pool size = %zu\n", ALLOC_KIND_TO_STR(AllocKind),
         DPxPTR(base), blockSize, PoolSize);
    }

    LOG_MEM_USAGE_POOL(Device, Size, mem, AllocMin << bucketId);

    return mem;
  }

  /// Return the memory to the pool
  bool dealloc(void *Mem) {
    if (!Initialized)
      return false;

    std::unique_lock<std::mutex> lock(PoolMtx);

    auto key = reinterpret_cast<intptr_t>(Mem);
    if (PtrToBlock.count(key) == 0)
      return false;

    PtrToBlock[key]->dealloc(Mem);
    LOG_MEM_USAGE_POOL(Device, 0, Mem, PtrToBlock[key]->ChunkSize);

    PtrToBlock.erase(key);

    return true;
  }

  /// Release all memory in the pool
  void deinit() {
    for (auto &bucket : Buckets) {
      for (auto block : bucket) {
        LOG_MEM_USAGE(Device, 0, (void *)block->Base);
        CALL_ZE_EXIT_FAIL(zeMemFree, Context, (void *)block->Base);
        delete block;
      }
    }
  }
}; /// MemoryPoolTy

class KernelInfoTy {
  uint32_t Version = 0;

  struct KernelArgInfoTy {
    bool IsLiteral = false;
    uint32_t Size = 0;
    KernelArgInfoTy(bool IsLiteral, uint32_t Size)
      : IsLiteral(IsLiteral), Size(Size) {}
  };
  std::vector<KernelArgInfoTy> ArgsInfo;

  void checkVersion(uint32_t MinVer) const {
    assert(Version >= MinVer &&
           "API is not supported for this version of KernelInfoTy.");
    (void)Version;
  }

public:
  KernelInfoTy(uint32_t Version) : Version(Version) {}
  void addArgInfo(bool IsLiteral, uint32_t Size) {
    checkVersion(1);
    ArgsInfo.emplace_back(IsLiteral, Size);
  }
  size_t getArgsNum() const {
    checkVersion(1);
    return ArgsInfo.size();
  }
  bool isArgLiteral(uint32_t Idx) const {
    checkVersion(1);
    return ArgsInfo[Idx].IsLiteral;
  }
  uint32_t getArgSize(uint32_t Idx) const {
    checkVersion(1);
    return ArgsInfo[Idx].Size;
  }
};

/// Per-device global entry table
struct FuncOrGblEntryTy {
  __tgt_target_table Table;
  std::vector<__tgt_offload_entry> Entries;
  std::vector<ze_kernel_handle_t> Kernels;
  std::vector<ze_module_handle_t> Modules;
  std::unordered_map<ze_kernel_handle_t, KernelInfoTy> KernelInfo;
};

/// Module data to be initialized by plugin
struct ModuleDataTy {
  int Initialized = 0;
  int NumDevices = 0;
  int DeviceNum = -1;
  uint32_t TotalEUs = 0;
  uint32_t HWThreadsPerEU = 0;
  uintptr_t DynamicMemoryLB = 0;
  uintptr_t DynamicMemoryUB = 0;
  int DeviceType = 0;
};

/// RTL profile -- only host timer for now
class RTLProfileTy {
  struct TimeTy {
    double HostTime = 0.0;
    double DeviceTime = 0.0; // Not used for now
  };
  int ThreadId;
  std::string DeviceIdStr;
  std::string DeviceName;
  std::map<std::string, TimeTy> Data;
  // L0 RT will keep UseCyclesPerSecondTimer=1 to enable new timer resolution
  // during transition period (until 20210504).
  uint64_t TimestampNsec = 0; // For version < ZE_API_VERSION_1_1
  uint64_t TimestampCyclePerSec = 0; // For version >= ZE_API_VERSION_1_1
  uint64_t TimestampMax = 0;
public:
  static const int64_t MSEC_PER_SEC = 1000;
  static const int64_t USEC_PER_SEC = 1000000;
  static const int64_t NSEC_PER_SEC = 1000000000;
  static int64_t Multiplier;

  RTLProfileTy(const ze_device_properties_t &DeviceProperties,
               const std::string &DeviceId, bool UseCyclePerSec) {
    ThreadId = __kmpc_global_thread_num(nullptr);
    DeviceIdStr = DeviceId;
    DeviceName = DeviceProperties.name;

    // TODO: this is an extra check to be on safe side for all driver versions.
    // Remove this heuristic when it is not necessary any more.
    if (DeviceProperties.timerResolution < 1000)
      UseCyclePerSec = false;

    if (UseCyclePerSec)
      TimestampCyclePerSec = DeviceProperties.timerResolution;
    else
      TimestampNsec = DeviceProperties.timerResolution;
    auto validBits = DeviceProperties.kernelTimestampValidBits;
    if (validBits > 0 && validBits < 64)
      TimestampMax = ~(-1ULL << validBits);
    else
      WARNING("Invalid kernel timestamp bit width (%" PRIu32 "). "
              "Long-running kernels may report incorrect device time.\n",
              validBits);
  }

  ~RTLProfileTy() {
    printData();
  }

  std::string alignLeft(size_t Width, std::string Str) {
    if (Str.size() < Width)
      return Str + std::string(Width - Str.size(), ' ');
    return Str;
  }

  void printData() {
    std::string profileSep(80, '=');
    std::string lineSep(80, '-');

    fprintf(stderr, "%s\n", profileSep.c_str());

    fprintf(stderr, "LIBOMPTARGET_PLUGIN_PROFILE(%s) for OMP DEVICE(%s) %s"
            ", Thread %" PRId32 "\n", GETNAME(TARGET_NAME), DeviceIdStr.c_str(),
            DeviceName.c_str(), ThreadId);
    const char *unit = (Multiplier == MSEC_PER_SEC) ? "msec" : "usec";

    fprintf(stderr, "%s\n", lineSep.c_str());

    std::string kernelPrefix("Kernel ");
    size_t maxKeyLength = kernelPrefix.size() + 3;
    for (const auto &d : Data)
      if (d.first.substr(0, kernelPrefix.size()) != kernelPrefix &&
          maxKeyLength < d.first.size())
        maxKeyLength = d.first.size();

    // Print kernel key and name
    int kernelId = 0;
    for (const auto &d: Data) {
      if (d.first.substr(0, kernelPrefix.size()) == kernelPrefix)
        fprintf(stderr, "-- %s: %s\n",
                alignLeft(maxKeyLength, kernelPrefix +
                          std::to_string(kernelId++)).c_str(),
                d.first.substr(kernelPrefix.size()).c_str());
    }

    fprintf(stderr, "%s\n", lineSep.c_str());

    fprintf(stderr, "-- %s:     Host Time (%s)   Device Time (%s)\n",
            alignLeft(maxKeyLength, "Name").c_str(), unit, unit);

    double hostTotal = 0.0;
    double deviceTotal = 0.0;
    kernelId = 0;
    for (const auto &d : Data) {
      double hostTime = d.second.HostTime * Multiplier;
      double deviceTime = hostTime;
      std::string key(d.first);
      if (d.first.substr(0, kernelPrefix.size()) == kernelPrefix) {
        deviceTime = d.second.DeviceTime * Multiplier;
        key = kernelPrefix + std::to_string(kernelId++);
      }
      fprintf(stderr, "-- %s: %20.3f %20.3f\n",
              alignLeft(maxKeyLength, key).c_str(), hostTime, deviceTime);
      hostTotal += hostTime;
      deviceTotal += deviceTime;
    }
    fprintf(stderr, "-- %s: %20.3f %20.3f\n",
            alignLeft(maxKeyLength, "Total").c_str(), hostTotal, deviceTotal);
    fprintf(stderr, "%s\n", profileSep.c_str());
  }

  void update(const char *Name, double HostTime, double DeviceTime = 0) {
    std::string Key(Name);
    update(Key, HostTime, DeviceTime);
  }

  void update(std::string &Name, double HostTime, double DeviceTime = 0) {
    auto &Time = Data[Name];
    Time.HostTime += HostTime;
    Time.DeviceTime += DeviceTime;
  }

  /// Return elapsed time from the given profile event
  double getEventTime(ze_event_handle_t Event) {
    ze_kernel_timestamp_result_t TS;
    CALL_ZE_EXIT_FAIL(zeEventQueryKernelTimestamp, Event, &TS);
    double WallTime = 0;

    if (TS.global.kernelEnd >= TS.global.kernelStart)
      WallTime = TS.global.kernelEnd - TS.global.kernelStart;
    else if (TimestampMax > 0)
      WallTime = TimestampMax - TS.global.kernelStart + TS.global.kernelEnd + 1;
    else
      WARNING("Timestamp overflow cannot be handled for this device.\n");

    if (TimestampNsec > 0)
      WallTime *= (double)TimestampNsec / NSEC_PER_SEC;
    else
      WallTime /= (double)TimestampCyclePerSec;

    CALL_ZE_EXIT_FAIL(zeEventHostReset, Event);

    return WallTime;
  }

  void update(std::string &Name, ze_event_handle_t Event) {
    Data[Name].DeviceTime += getEventTime(Event);
  }
};
int64_t RTLProfileTy::Multiplier;

/// Get default command queue group ordinal
static uint32_t getCmdQueueGroupOrdinal(ze_device_handle_t Device) {
  uint32_t groupCount = 0;
  CALL_ZE_RET(UINT32_MAX, zeDeviceGetCommandQueueGroupProperties, Device,
              &groupCount, nullptr);
  std::vector<ze_command_queue_group_properties_t> groupProperties(
      groupCount, CommandQueueGroupPropertiesInit);
  CALL_ZE_RET(UINT32_MAX, zeDeviceGetCommandQueueGroupProperties, Device,
              &groupCount, groupProperties.data());
  uint32_t groupOrdinal = UINT32_MAX;
  for (uint32_t i = 0; i < groupCount; i++) {
    auto &flags = groupProperties[i].flags;
    if (flags & ZE_COMMAND_QUEUE_GROUP_PROPERTY_FLAG_COMPUTE) {
      groupOrdinal = i;
      break;
    }
  }

  if (groupOrdinal == UINT32_MAX)
    DP("Error: no command queues are found\n");

  return groupOrdinal;
}

/// Get multi-context command queue group ordinal and number of queues
static uint32_t getCmdQueueGroupOrdinalCCS(ze_device_handle_t Device,
                                           uint32_t &NumQueues) {
  uint32_t groupCount = 0;
  CALL_ZE_RET(UINT32_MAX, zeDeviceGetCommandQueueGroupProperties, Device,
              &groupCount, nullptr);
  std::vector<ze_command_queue_group_properties_t> groupProperties(
      groupCount, CommandQueueGroupPropertiesInit);
  CALL_ZE_RET(UINT32_MAX, zeDeviceGetCommandQueueGroupProperties, Device,
              &groupCount, groupProperties.data());
  uint32_t groupOrdinal = UINT32_MAX;
  for (uint32_t i = 0; i < groupCount; i++) {
    auto &flags = groupProperties[i].flags;
    if (flags & ZE_COMMAND_QUEUE_GROUP_PROPERTY_FLAG_COMPUTE &&
        groupProperties[i].numQueues > 1) {
      groupOrdinal = i;
      NumQueues = groupProperties[i].numQueues;
      DP("Found multi-context command queue group for deivce " DPxMOD
         ", ordinal = %" PRIu32 ", number of queues = %" PRIu32 "\n",
         DPxPTR(Device), groupOrdinal, NumQueues);
      break;
    }
  }
  if (groupOrdinal == UINT32_MAX)
    DP("Could not find multi-context command queue group for device " DPxMOD
       "\n", DPxPTR(Device));
  return groupOrdinal;
}

/// Get copy command queue group ordinal
static uint32_t getCmdQueueGroupOrdinalCopy(ze_device_handle_t Device) {
  uint32_t groupCount = 0;
  CALL_ZE_RET(UINT32_MAX, zeDeviceGetCommandQueueGroupProperties, Device,
              &groupCount, nullptr);
  std::vector<ze_command_queue_group_properties_t> groupProperties(
      groupCount, CommandQueueGroupPropertiesInit);
  CALL_ZE_RET(UINT32_MAX, zeDeviceGetCommandQueueGroupProperties, Device,
              &groupCount, groupProperties.data());
  uint32_t groupOrdinal = UINT32_MAX;
  for (uint32_t i = 0; i < groupCount; i++) {
    auto &flags = groupProperties[i].flags;
    if ((flags & ZE_COMMAND_QUEUE_GROUP_PROPERTY_FLAG_COPY) &&
        (flags & ZE_COMMAND_QUEUE_GROUP_PROPERTY_FLAG_COMPUTE) == 0) {
      groupOrdinal = i;
      DP("Found copy command queue for device " DPxMOD ", ordinal = %" PRIu32
         "\n", DPxPTR(Device), groupOrdinal);
      break;
    }
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
  DP("Created a command list " DPxMOD " for device %s.\n", DPxPTR(cmdList),
     DeviceIdStr.c_str());
  return cmdList;
}

/// Create a command list with default flags
static ze_command_list_handle_t createCmdList(
    ze_context_handle_t Context, ze_device_handle_t Device, uint32_t Ordinal,
    std::string &DeviceIdStr) {
  return (Ordinal == UINT32_MAX)
      ? nullptr
      : createCmdList(Context, Device, Ordinal, 0, DeviceIdStr);
}

/// Create a command queue with given ordinal and flags
static ze_command_queue_handle_t createCmdQueue(
    ze_context_handle_t Context,
    ze_device_handle_t Device,
    uint32_t Ordinal,
    uint32_t Index,
    ze_command_queue_flags_t Flags,
    std::string &DeviceIdStr) {
  ze_command_queue_desc_t cmdQueueDesc = {
    ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC,
    nullptr, // extension
    Ordinal,
    Index,
    Flags, // flags
    ZE_COMMAND_QUEUE_MODE_ASYNCHRONOUS,
    ZE_COMMAND_QUEUE_PRIORITY_NORMAL
  };
  ze_command_queue_handle_t cmdQueue;
  CALL_ZE_RET_NULL(zeCommandQueueCreate, Context, Device, &cmdQueueDesc,
                   &cmdQueue);
  DP("Created a command queue " DPxMOD " for device %s.\n", DPxPTR(cmdQueue),
     DeviceIdStr.c_str());
  return cmdQueue;
}

/// Create a command queue with default flags
static ze_command_queue_handle_t createCmdQueue(
    ze_context_handle_t Context, ze_device_handle_t Device,
    uint32_t Ordinal, uint32_t Index, std::string &DeviceIdStr) {
  return (Ordinal == UINT32_MAX)
      ? nullptr
      : createCmdQueue(Context, Device, Ordinal, Index, 0, DeviceIdStr);
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

/// RTL flags
struct RTLFlagsTy {
  uint64_t DumpTargetImage : 1;
  uint64_t EnableProfile : 1;
  uint64_t EnableTargetGlobals : 1;
  uint64_t LinkLibDevice : 1;
  uint64_t UseHostMemForUSM : 1;
  uint64_t UseMemoryPool : 1;
  uint64_t UseDriverGroupSizes : 1;
  uint64_t UseCopyEngine : 1;
  uint64_t UseImageOptions : 1;
  uint64_t Reserved : 55;
  RTLFlagsTy() :
      DumpTargetImage(0),
      EnableProfile(0),
      EnableTargetGlobals(0),
      LinkLibDevice(0), // TODO: change it to 1 when L0 issue is resolved
      UseHostMemForUSM(0),
      UseMemoryPool(1),
      UseDriverGroupSizes(0),
      UseCopyEngine(1),
      UseImageOptions(1),
      Reserved(0) {}
};

/// Kernel properties.
struct KernelPropertiesTy {
  const char *Name = nullptr;
  uint32_t Width = 0;
  uint32_t SIMDWidth = 0;
  uint32_t MaxThreadGroupSize = 0;
  ze_kernel_indirect_access_flags_t IndirectAccessFlags = 0;
};

/// Events for kernel profiling
struct KernelProfileEventsTy {
  std::vector<ze_event_pool_handle_t> Pools;
  size_t Size = 0;
  std::map<int32_t, ze_event_handle_t> Events;  // Per-thread events
  std::mutex *EventLock = nullptr;
  ze_context_handle_t Context = nullptr;

  void init(ze_context_handle_t ContextHandle) {
    Size = omp_get_max_threads();
    EventLock = new std::mutex();
    Context = ContextHandle;
  }

  void deinit() {
    for (auto event : Events)
      CALL_ZE_EXIT_FAIL(zeEventDestroy, event.second);
    for (auto pool : Pools)
      CALL_ZE_EXIT_FAIL(zeEventPoolDestroy, pool);
    delete EventLock;
  }

  ze_event_handle_t getEvent() {
    int32_t gtid = __kmpc_global_thread_num(nullptr);
    std::unique_lock<std::mutex> lock(*EventLock);
    if (Events.count(gtid) == 0) {
      size_t eventId = Events.size() % Size;
      if (eventId == 0) {
        // This is the first event, or the pools are fully used.
        DP("Creating a new profiling event pool.\n");
        ze_event_pool_desc_t poolDesc = {
          ZE_STRUCTURE_TYPE_EVENT_POOL_DESC,
          nullptr,
          ZE_EVENT_POOL_FLAG_HOST_VISIBLE | ZE_EVENT_POOL_FLAG_KERNEL_TIMESTAMP,
          (uint32_t)Size
        };
        ze_event_pool_handle_t pool = nullptr;
        CALL_ZE_RET_NULL(
            zeEventPoolCreate, Context, &poolDesc, 0, nullptr, &pool);
        Pools.push_back(pool);
      }
      auto pool = Pools.back();
      ze_event_desc_t eventDesc = {
        ZE_STRUCTURE_TYPE_EVENT_DESC,
        nullptr,
        (uint32_t)eventId,
        0,
        0
      };
      ze_event_handle_t event;
      CALL_ZE_RET_NULL(zeEventCreate, pool, &eventDesc, &event);
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

/// Specialization constants used for a module compilation.
class SpecConstantsTy {
  std::vector<uint32_t> ConstantIds;
  std::vector<const void *> ConstantValues;

public:
  SpecConstantsTy() = default;
  SpecConstantsTy(const SpecConstantsTy &) = delete;
  SpecConstantsTy(const SpecConstantsTy &&Other)
    : ConstantIds(std::move(Other.ConstantIds)),
      ConstantValues(std::move(Other.ConstantValues)) {}

  ~SpecConstantsTy() {
    for (auto I : ConstantValues) {
      const char *ValuePtr = reinterpret_cast<const char *>(I);
      delete[] ValuePtr;
    }
  }

  template <typename T>
  void addConstant(uint32_t Id, T Val) {
    const size_t ValSize = sizeof(Val);
    char *ValuePtr = new char[ValSize];
    *reinterpret_cast<T *>(ValuePtr) = Val;

    ConstantIds.push_back(Id);
    ConstantValues.push_back(reinterpret_cast<void *>(ValuePtr));
  }

  ze_module_constants_t getModuleConstants() const {
    ze_module_constants_t Tmp{static_cast<uint32_t>(ConstantValues.size()),
                              ConstantIds.data(),
                              // Unfortunately we have to const_cast it.
                              // L0 data type should probably be fixed.
                              const_cast<const void **>(ConstantValues.data())};
    return Tmp;
  }
};

/// Memory ranges for tracking allocated memory region
class MemRangeTy {
  std::multimap<void *, size_t> Ranges;
  std::mutex Mtx;

public:
  void add(void *Ptr, size_t Size) {
    std::lock_guard<std::mutex> Lock(Mtx);
    Ranges.insert({Ptr, Size});
  }

  void remove(void *Ptr) {
    std::lock_guard<std::mutex> Lock(Mtx);
    Ranges.erase(Ptr);
  }

  bool contains(const void *Ptr, size_t Size) {
    std::lock_guard<std::mutex> Lock(Mtx);
    auto I = Ranges.insert({const_cast<void *>(Ptr), 0});
    auto J = I--;
    bool Ret = (uintptr_t)I->first <= (uintptr_t)Ptr &&
        (uintptr_t)Ptr + (uintptr_t)Size <=
        (uintptr_t)I->first + (uintptr_t)I->second;
    Ranges.erase(J);
    return Ret;
  }
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

  /// Memory stats
  struct MemStatTy {
    size_t Requested[2] = {0, 0};   // Requested bytes
    size_t Allocated[2] = {0, 0};   // Allocated bytes
    size_t Freed[2] = {0, 0};       // Freed bytes
    size_t InUse[2] = {0, 0};       // Current memory in use
    size_t PeakUse[2] = {0, 0};     // Peak bytes used
    size_t NumAllocs[2] = {0, 0};   // Number of allocations

    int32_t DeviceId = 0;
    int32_t AllocKind = TARGET_ALLOC_DEFAULT;
    std::mutex *Mtx = nullptr;

    MemStatTy() = default;

    MemStatTy(int32_t deviceId, int32_t allocKind) {
      DeviceId = deviceId;
      AllocKind = allocKind;
      Mtx = new std::mutex();
    }

    void update(size_t requested, size_t size, bool Pool = false) {
      std::unique_lock<std::mutex> lock(*Mtx);

      int32_t I = Pool ? 1 : 0;

      if (requested > 0) {
        Requested[I] += requested;
        Allocated[I] += size;
        InUse[I] += size;
        NumAllocs[I]++;
      } else {
        Freed[I] += size;
        InUse[I] -= size;
      }

      if (InUse[I] > PeakUse[I])
        PeakUse[I] = InUse[I];
    }

    void print() {
      DP("Memory usage for %s, device %" PRId32 ":\n",
         ALLOC_KIND_TO_STR(AllocKind), DeviceId);
      DP("-- Allocator: %12s, %12s\n", "Native", "Pool");
      DP("-- Requested: %12zu, %12zu\n", Requested[0], Requested[1]);
      DP("-- Allocated: %12zu, %12zu\n", Allocated[0], Allocated[1]);
      DP("-- Freed    : %12zu, %12zu\n", Freed[0], Freed[1]);
      DP("-- InUse    : %12zu, %12zu\n", InUse[0], InUse[1]);
      DP("-- PeakUse  : %12zu, %12zu\n", PeakUse[0], PeakUse[1]);
      DP("-- NumAllocs: %12zu, %12zu\n", NumAllocs[0], NumAllocs[1]);
    }
  }; // MemStatTy

  /// Looks up an external global variable with the given \p Name
  /// and \p Size in the device environment for device \p DeviceId.
  void *getVarDeviceAddr(int32_t DeviceId, const char *Name, size_t Size);

  /// Looks up an external global variable with the given \p Name
  /// in the device environment for device \p DeviceId.
  /// \p Size must not be null. If (*Size) is not zero, then
  /// the lookup verifies that the found variable's size matches
  /// (*Size), otherwise, the found variable's size is returned
  /// via \p Size.
  void *getVarDeviceAddr(int32_t DeviceId, const char *Name, size_t *Size);
public:
  uint32_t NumDevices = 0;
  uint32_t SubDeviceLevels = 1; // L0 API does not support recursive queries
  uint32_t NumRootDevices = 0;
  ze_driver_handle_t Driver = nullptr;
  ze_context_handle_t Context = nullptr;
  ze_api_version_t DriverAPIVersion = ZE_API_VERSION_CURRENT;

  // Events for kernel profiling
  KernelProfileEventsTy ProfileEvents;

  std::vector<ze_device_properties_t> DeviceProperties;
  std::vector<ze_device_compute_properties_t> ComputeProperties;

  // Internal device type ID
  std::vector<uint64_t> DeviceArchs;

  // Devices' default target allocation kinds for internal allocation
  std::vector<int32_t> AllocKinds;

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
  std::vector<std::vector<ze_command_list_handle_t>> CopyCmdLists;
  std::vector<std::vector<ze_command_queue_handle_t>> CopyCmdQueues;

  // Command queue group ordinals for each device
  std::vector<uint32_t> CmdQueueGroupOrdinals;

  // Command queue group ordinals for copying
  std::vector<uint32_t> CopyCmdQueueGroupOrdinals;

  // Command queue index for each device
  std::vector<uint32_t> CmdQueueIndices;

  std::vector<std::list<FuncOrGblEntryTy>> FuncGblEntries;
  std::vector<std::map<ze_kernel_handle_t, KernelPropertiesTy>>
      KernelProperties;

  // Memory owned by the plugin
  std::vector<std::vector<void *>> OwnedMemory;

  std::vector<std::set<void *>> ImplicitArgsDevice;
  std::vector<std::set<void *>> ImplicitArgsHost;
  std::vector<std::set<void *>> ImplicitArgsShared;
  std::vector<bool> Initialized;
  std::mutex *Mutexes;
  std::mutex *DataMutexes; // For internal data
  std::mutex *RTLMutex;
  std::vector<std::list<std::vector<DeviceOffloadEntryTy>>> OffloadTables;
  std::vector<std::vector<RTLProfileTy *>> Profiles;

  /// Host memory pool for all devices
  MemoryPoolTy MemPoolHost;
  /// Shared memory pools per L0 device
  std::map<ze_device_handle_t, MemoryPoolTy> MemPoolShared;
  /// Device memory pools per L0 device
  std::map<ze_device_handle_t, MemoryPoolTy> MemPoolDevice;

  /// Memory stats for each memory type
  MemStatTy MemStatHost;
  std::map<ze_device_handle_t, MemStatTy> MemStatShared;
  std::map<ze_device_handle_t, MemStatTy> MemStatDevice;

  /// Allocated USM host/shared memory range
  /// We need to track the valid address ranges known to libomptarget and users.
  std::map<ze_device_handle_t, std::unique_ptr<MemRangeTy>> MemHostAccessible;

  /// GITS function address for notifying indirect accesses
  void *GitsIndirectAllocationOffsets = nullptr;

  /// Flags, parameters, options
  RTLFlagsTy Flags;
  int64_t RequiresFlags = OMP_REQ_UNDEFINED;
  uint32_t DataTransferLatency; // Emulated data transfer latency in us
  int32_t DeviceType;
  uint32_t ThreadLimit = 0; // Global thread limit
  uint32_t NumTeams = 0; // Global max number of teams

  /// Dynamic kernel memory size
  size_t KernelDynamicMemorySize = 0; // Turned off by default

  /// Staging buffer size
  size_t StagingBufferSize = LEVEL0_STAGING_BUFFER_SIZE;

  /// Staging buffer count
  size_t StagingBufferCount = LEVEL0_STAGING_BUFFER_COUNT;

  /// Command batch support
  int32_t CommandBatchLevel = 0;
  int32_t CommandBatchCount = INT32_MAX;

  /// Memory pool parameters
  /// MemPoolInfo[MemType] = {AllocMax(MB), Capacity, PoolSize(MB)}
  std::map<int32_t, std::vector<int32_t>> MemPoolInfo = {
      {TARGET_ALLOC_DEVICE, {1, 4, 256}},
      {TARGET_ALLOC_HOST, {1, 4, 256}},
      {TARGET_ALLOC_SHARED, {1, 4, 256}}
  };

  /// User-directed allocation kind
  int32_t TargetAllocKind = TARGET_ALLOC_DEFAULT;

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
  std::string UserCompilationOptions;

  // Spec constants used for all modules.
  SpecConstantsTy CommonSpecConstants;

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
      if (value == "GPU" || value == "gpu" || value == "") {
        DeviceType = ZE_DEVICE_TYPE_GPU;
      } else if (value == "CPU" || value == "cpu") {
        DeviceType = ZE_DEVICE_TYPE_CPU;
        DP("Warning: CPU device is not supported\n");
      } else {
        DP("Warning: Invalid LIBOMPTARGET_DEVICETYPE=%s\n", env);
      }
    }

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
      UserCompilationOptions += std::string(" ") + env;

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
        DP("Device mode is %s -- using top-level devices with subdevice "
           "clause support\n", value.c_str());
        DeviceMode = DEVICE_MODE_TOP;
      } else if (value == "SUBDEVICE" || value == "subdevice") {
        DP("Device mode is %s -- using 1st-level sub-devices\n", value.c_str());
        DeviceMode = DEVICE_MODE_SUB;
      } else if (value == "SUBSUBDEVICE" || value == "subsubdevice") {
        DP("Device mode is %s -- using 2nd-level sub-devices\n", value.c_str());
        DeviceMode = DEVICE_MODE_SUBSUB;
      } else if (value == "ALL" || value == "all") {
        DP("Device mode is %s -- using all sub-devices\n", value.c_str());
        DeviceMode = DEVICE_MODE_ALL;
      } else {
        DP("Unknown device mode %s\n", value.c_str());
      }
    }

    // Profile
    if (char *env = readEnvVar("LIBOMPTARGET_PLUGIN_PROFILE")) {
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
    // LIBOMPTARGET_LEVEL0_MEMORY_POOL=<Option>
    //  <Option>       := 0 | <PoolInfoList>
    //  <PoolInfoList> := <PoolInfo>[,<PoolInfoList>]
    //  <PoolInfo>     := <MemType>[,<AllocMax>[,<Capacity>[,<PoolSize>]]]
    //  <MemType>      := all | device | host | shared
    //  <AllocMax>     := positive integer or empty, max allocation size in MB
    //                    (default: 1)
    //  <Capacity>     := positive integer or empty, number of allocations from
    //                    a single block (default: 4)
    //  <PoolSize>     := positive integer or empty, max pool size in MB
    //                    (default: 256)
    if (char *env = readEnvVar("LIBOMPTARGET_LEVEL0_MEMORY_POOL")) {
      if (env[0] == '0' && env[1] == '\0') {
        Flags.UseMemoryPool = 0;
      } else {
        std::istringstream str(env);
        int32_t memType = -1;
        int32_t offset = 0;
        int32_t valid = 1;
        const std::vector<int32_t> defaultValue{1, 4, 256};
        const int32_t allMemType = INT32_MAX;
        std::vector<int32_t> allInfo{1, 4, 256};
        std::map<int32_t, std::vector<int32_t>> poolInfo;
        for (std::string token; std::getline(str, token, ',') && valid > 0;) {
          if (token == "device") {
            memType = TARGET_ALLOC_DEVICE;
            poolInfo.emplace(memType, defaultValue);
            offset = 0;
          } else if (token == "host") {
            memType = TARGET_ALLOC_HOST;
            poolInfo.emplace(memType, defaultValue);
            offset = 0;
          } else if (token == "shared") {
            memType = TARGET_ALLOC_SHARED;
            poolInfo.emplace(memType, defaultValue);
            offset = 0;
          } else if (token == "all") {
            memType = allMemType;
            offset = 0;
            valid = 2;
          } else if (offset < 3 && memType >= 0) {
            int32_t num = std::atoi(token.c_str());
            if (num > 0 && memType == allMemType)
              allInfo[offset++] = num;
            else if (num > 0)
              poolInfo[memType][offset++] = num;
            else if (token.size() == 0)
              offset++;
            else
              valid = 0;
          } else {
            valid = 0;
          }
        }
        if (valid > 0) {
          MemPoolInfo.clear();
          if (valid == 2) {
            // "all" is specified -- ignore other inputs
            MemPoolInfo.emplace(TARGET_ALLOC_DEVICE, allInfo);
            MemPoolInfo.emplace(TARGET_ALLOC_HOST, allInfo);
            MemPoolInfo.emplace(TARGET_ALLOC_SHARED, allInfo);
          } else {
            // Only enable what user specified
            for (auto &I : poolInfo)
              MemPoolInfo.emplace(I.first, I.second);
          }
          // Set total pool size large enough (2 * AllocMax * Capacity)
          for (auto &I : MemPoolInfo) {
            int32_t poolSize = 2 * I.second[0] * I.second[1];
            if (poolSize > I.second[2]) {
              I.second[2] = poolSize;
              DP("Adjusted memory pool size to %" PRId32 "MB\n", poolSize);
            }
          }
        } else {
          DP("Ignoring incorrect memory pool configuration "
             "LIBOMPTARGET_LEVEL0_MEMORY_POOL=%s\n", env);
          DP("LIBOMPTARGET_LEVEL0_MEMORY_POOL=<Option>\n");
          DP("  <Option>       := 0 | <PoolInfoList>\n");
          DP("  <PoolInfoList> := <PoolInfo>[,<PoolInfoList>]\n");
          DP("  <PoolInfo>     := "
             "<MemType>[,<AllocMax>[,<Capacity>[,<PoolSize>]]]\n");
          DP("  <MemType>      := all | device | host | shared\n");
          DP("  <AllocMax>     := positive integer or empty, "
             "max allocation size in MB (default: 1)\n");
          DP("  <Capacity>     := positive integer or empty, "
             "number of allocations from a single block (default: 4)\n");
          DP("  <PoolSize>     := positive integer or empty, "
             "max pool size in MB (default: 256)\n");
        }
      }
    }

    // Use copy engine if available
    if (char *env = readEnvVar("LIBOMPTARGET_LEVEL0_USE_COPY_ENGINE")) {
      if (env[0] == 'F' || env[0] == 'f' || env[0] == '0') {
        Flags.UseCopyEngine = 0;
      } else {
        DP("Ignoring incorrect definition, "
           "LIBOMPTARGET_LEVEL0_USE_COPY_ENGINE=%s\n", env);
        DP("LIBOMPTARGET_LEVEL0_USE_COPY_ENGINE=<Value>\n");
        DP("  <Value> := 0 | F | f\n");
      }
    }

    // Target image dump
    if (char *env = readEnvVar("LIBOMPTARGET_DUMP_TARGET_IMAGE",
                               "LIBOMPTARGET_SAVE_TEMPS")) {
      if (env[0] == 'T' || env[0] == 't' || env[0] == '1')
        Flags.DumpTargetImage = 1;
    }

    // Global default target memory that overrides device-specific default
    if (char *env = readEnvVar("LIBOMPTARGET_LEVEL0_DEFAULT_TARGET_MEM")) {
      std::string mem(env);
      if (mem == "host" || mem == "HOST")
        TargetAllocKind = TARGET_ALLOC_HOST;
      else if (mem == "shared" || mem == "SHARED")
        TargetAllocKind = TARGET_ALLOC_SHARED;
      else if (mem == "device" || mem == "DEVICE")
        TargetAllocKind = TARGET_ALLOC_DEVICE;
      else
        DP("Warning: Ignoring unknown target memory kind %s.\n", env);
    }

    // Target allocation kind
    if (char *env = readEnvVar("LIBOMPTARGET_LEVEL0_USE_DEVICE_MEM")) {
      if (env[0] == 'T' || env[0] == 't' || env[0] == '1')
        TargetAllocKind = TARGET_ALLOC_DEVICE;
    }

    // Subscription rate
    if (char *env = readEnvVar("LIBOMPTARGET_LEVEL0_SUBSCRIPTION_RATE")) {
      int32_t value = std::stoi(env);
      if (value > 0 && value <= 0xFFFF)
        SubscriptionRate = value;
    }

    // Forced kernel width
    if (char *env = readEnvVar("LIBOMPTARGET_LEVEL0_KERNEL_WIDTH")) {
      int32_t value = std::stoi(env);
      if (value == 8 || value == 16 || value == 32)
        ForcedKernelWidth = value;
    }

    // Dynamic memory size
    // LIBOMPTARGET_DYNAMIC_MEMORY_SIZE=<SizeInMB>
    if (char *env = readEnvVar("LIBOMPTARGET_DYNAMIC_MEMORY_SIZE")) {
      size_t value = std::stoi(env);
      const size_t maxValue = 2048;
      if (value > maxValue) {
        DP("Adjusted dynamic memory size to %zu MB\n", maxValue);
        value = maxValue;
      }
      KernelDynamicMemorySize = value << 20;
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
    if (readEnvVar("INTEL_ENABLE_OFFLOAD_ANNOTATIONS")) {
      // To match SYCL RT behavior, we just need to check whether
      // INTEL_ENABLE_OFFLOAD_ANNOTATIONS is set. The actual value
      // does not matter.
      CommonSpecConstants.addConstant<char>(0xFF747469, 1);
    }
    if (char *env = readEnvVar("LIBOMPTARGET_ONEAPI_USE_IMAGE_OPTIONS")) {
      if (env[0] == 'T' || env[0] == 't' || env[0] == '1')
        Flags.UseImageOptions = 1;
      else if (env[0] == 'F' || env[0] == 'f' || env[0] == '0')
        Flags.UseImageOptions = 0;
    }
    // LIBOMPTARGET_LEVEL0_STAGING_BUFFER_SIZE=<SizeInKB>
    if (char *env = readEnvVar("LIBOMPTARGET_LEVEL0_STAGING_BUFFER_SIZE")) {
      size_t SizeInKB = std::stoi(env);
      StagingBufferSize = SizeInKB << 10;
    }
    // LIBOMPTARGET_LEVEL_ZERO_COMMAND_BATCH=<Type>[,<Count>]
    if (char *env = readEnvVar("LIBOMPTARGET_LEVEL_ZERO_COMMAND_BATCH")) {
      std::string Value(env);
      int32_t Count = 0;
      auto I = Value.find(",");
      if (I != std::string::npos) {
        Count = std::atoi(Value.substr(I + 1).c_str());
        Value.erase(I);
      }
      if (Count > 0) {
        CommandBatchCount = Count;
        if ((size_t)Count < StagingBufferCount)
          StagingBufferCount = Count;
      }
      if (Value == "none" || Value == "NONE") {
        CommandBatchLevel = 0;
        DP("Disabled command batching.\n");
      } else if (Value == "copy" || Value == "COPY") {
        CommandBatchLevel = 1;
        DP("Enabled command batching up to %" PRId32 " copy commands.\n",
           Count);
      } else if (Value == "compute" || Value == "COMPUTE") {
        CommandBatchLevel = 2;
        DP("Enabled command batching up to %" PRId32
           " copy and compute commands.\n", Count);
      } else {
        CommandBatchLevel = 0;
        DP("Disabled command batching due to unknown input \"%s\".\n", env);
      }
    }
  }

  ze_command_list_handle_t getCmdList(int32_t DeviceId) {
    auto TLS = getTLS();
    auto CmdList = TLS->getCmdList(DeviceId);
    if (!CmdList) {
      CmdList = createCmdList(Context, Devices[DeviceId],
          CmdQueueGroupOrdinals[DeviceId], DeviceIdStr[DeviceId]);
      TLS->setCmdList(DeviceId, CmdList);
    }
    return CmdList;
  }

  ze_command_queue_handle_t getCmdQueue(int32_t DeviceId) {
    auto TLS = getTLS();
    auto CmdQueue = TLS->getCmdQueue(DeviceId);
    if (!CmdQueue) {
      CmdQueue = createCommandQueue(DeviceId);
      TLS->setCmdQueue(DeviceId, CmdQueue);
    }
    return CmdQueue;
  }

  ze_command_list_handle_t getCopyCmdList(int32_t DeviceId) {
    if (!Flags.UseCopyEngine ||
        CopyCmdQueueGroupOrdinals[DeviceId] == UINT32_MAX)
      return getCmdList(DeviceId);
    // Use copy engine
    auto TLS = getTLS();
    auto CmdList = TLS->getCopyCmdList(DeviceId);
    if (!CmdList) {
      CmdList = createCmdList(Context, Devices[DeviceId],
          CopyCmdQueueGroupOrdinals[DeviceId], DeviceIdStr[DeviceId]);
      TLS->setCopyCmdList(DeviceId, CmdList);
    }
    return CmdList;
  }

  bool usingCopyCmdQueue(int32_t DeviceId) {
    if (!Flags.UseCopyEngine ||
        CopyCmdQueueGroupOrdinals[DeviceId] == UINT32_MAX)
       return  false;
    return true;
  }

  ze_command_queue_handle_t getCopyCmdQueue(int32_t DeviceId) {
    if (!Flags.UseCopyEngine ||
        CopyCmdQueueGroupOrdinals[DeviceId] == UINT32_MAX)
      return getCmdQueue(DeviceId);
    // Use copy engine
    auto TLS = getTLS();
    auto CmdQueue = TLS->getCopyCmdQueue(DeviceId);
    if (!CmdQueue) {
      CmdQueue = createCmdQueue(Context, Devices[DeviceId],
          CopyCmdQueueGroupOrdinals[DeviceId], 0, DeviceIdStr[DeviceId]);
      TLS->setCopyCmdQueue(DeviceId, CmdQueue);
    }
    return CmdQueue;
  }

  RTLProfileTy *getProfile(int32_t DeviceId) {
    if (!Flags.EnableProfile)
      return nullptr;
    auto TLS = getTLS();
    auto Profile = TLS->getProfile(DeviceId);
    if (!Profile) {
      Profile = new RTLProfileTy(DeviceProperties[DeviceId],
                                 DeviceIdStr[DeviceId],
                                 DriverAPIVersion >= ZE_API_VERSION_1_1);
      TLS->setProfile(DeviceId, Profile);
    }
    return Profile;
  }

  int64_t getSubDeviceCode() { return getTLS()->getSubDeviceCode(); }

  void setSubDeviceCode(int64_t Code) {
    getTLS()->setSubDeviceCode(Code);
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

  /// Enqueue copy command
  int32_t enqueueMemCopy(int32_t DeviceId, void *Dst, void *Src, size_t Size);

  /// Allocate memory from pool
  void *poolAlloc(int32_t DeviceId, size_t Size, int32_t Kind,
                  intptr_t Offset = 0);

  /// Return memory to pool
  bool poolFree(int32_t DeviceId, void *Ptr);

  /// Initialize all memory pools
  void initMemoryPool();

  /// Initialize memory stats
  void initMemoryStat();

  /// Allocate data
  void *allocData(int32_t DeviceId, int64_t Size, void *HstPtr, void *HstBase,
                  bool *PoolAllocated = nullptr);

  /// Return memory allocation type
  uint32_t getMemAllocType(const void *Ptr);

  /// Create command queue with the given device ID
  ze_command_queue_handle_t createCommandQueue(int32_t DeviceId);

  /// Get thread-local staging buffer for copying
  StagingBufferTy &getStagingBuffer();

  /// Add host-accessible memory range
  void addHostAccessible(int32_t DeviceId, void *Ptr, size_t Size,
                         int32_t Kind = TARGET_ALLOC_DEFAULT);

  /// Remove host-accessible memory range
  void removeHostAccessible(int32_t DeviceId, void *Ptr);

  /// Read KernelInfo auxiliary information for the specified kernel.
  /// The information is stored in FuncGblEntries array.
  /// The function is called during the binary loading.
  bool readKernelInfo(int32_t DeviceId, const __tgt_offload_entry &KernelEntry);

  /// For the given kernel return its KernelInfo auxiliary information
  /// that was previously read by readKernelInfo().
  const KernelInfoTy *
      getKernelInfo(int32_t DeviceId, const ze_kernel_handle_t &Kernel) const;

  /// Check if the device is discrete
  bool isDiscreteDevice(int32_t DeviceId);

  /// Get internal device ID from subdevice encoding
  int32_t getInternalDeviceId(int32_t DeviceId);

  /// Data delete
  int32_t dataDelete(int32_t DeviceId, void *Ptr);
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

static RTLDeviceInfoTy *DeviceInfo = nullptr;

/// Create a module from SPIR-V
static ze_module_handle_t createModule(
    ze_context_handle_t Context, ze_device_handle_t Device, size_t Size,
    const uint8_t *Image, const char *Flags, ze_module_format_t Format) {
  ze_module_constants_t specConstants =
      DeviceInfo->CommonSpecConstants.getModuleConstants();
  ze_module_desc_t moduleDesc = {
    ZE_STRUCTURE_TYPE_MODULE_DESC,
    nullptr, // extension
    Format,
    Size,
    Image,
    Flags,
    &specConstants
  };
  ze_module_handle_t module;
  ze_module_build_log_handle_t buildLog;
  ze_result_t rc;
  CALL_ZE_RC(rc, zeModuleCreate, Context, Device, &moduleDesc, &module,
             &buildLog);
  if (rc != ZE_RESULT_SUCCESS) {
    DP("Warning: module creation failed\n");
    if (DebugLevel > 0) {
      size_t logSize = 0;
      CALL_ZE_RET_NULL(zeModuleBuildLogGetString, buildLog, &logSize, nullptr);
      DP("Target build log:\n");
      if (logSize > 0) {
        std::vector<char> logString(logSize);
        CALL_ZE_RET_NULL(zeModuleBuildLogGetString, buildLog, &logSize,
                         logString.data());
        std::stringstream Str(logString.data());
        std::string Line;
        while (std::getline(Str, Line, '\n'))
          DP("  %s\n", Line.c_str());
      } else {
        DP("  empty\n");
      }
    }
    CALL_ZE_RET_NULL(zeModuleBuildLogDestroy, buildLog);
    return nullptr;
  }
  CALL_ZE_RET_NULL(zeModuleBuildLogDestroy, buildLog);
  return module;
}

#if ENABLE_LIBDEVICE_LINKING
/// Create a module from a file name
static ze_module_handle_t createModule(
    ze_context_handle_t Context, ze_device_handle_t Device,
    const char *FileName, const char *Flags, ze_module_format_t Format) {
  // Resolve full path using the location of the plugin
  std::string fullPath;
#ifdef _WIN32
  char rtlPath[_MAX_PATH];
  HMODULE rtlModule = nullptr;
  if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                          GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR) &DebugLevel, &rtlModule)) {
    DP("Error: module creation failed -- cannot resolve full path\n");
    return nullptr;
  }
  if (!GetModuleFileNameA(rtlModule, rtlPath, sizeof(rtlPath))) {
    DP("Error: module creation failed -- cannot resolve full path\n");
    return nullptr;
  }
  fullPath = rtlPath;
#else // !defined(_WIN32)
  Dl_info rtlInfo;
  if (!dladdr(&DebugLevel, &rtlInfo)) {
    DP("Error: module creation failed -- cannot resolve full path\n");
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
    DP("Error: module creation failed -- cannot read %s\n", fullPath.c_str());
    return nullptr;
  }
  return createModule(Context, Device, size, (uint8_t *)image.data(), Flags,
                      Format);
}
#endif // ENABLE_LIBDEVICE_LINKING

/// Init/deinit DeviceInfo
#ifdef _WIN32
#define ATTRIBUTE(X)
#else
#define ATTRIBUTE(X) __attribute__((X))
#endif // _WIN32

static void closeRTL();

ATTRIBUTE(constructor(101)) void init() {
  DP("Init Level0 plugin!\n");
  DeviceInfo = new RTLDeviceInfoTy();
  TLSList = new std::list<TLSTy *>();
}

ATTRIBUTE(destructor(101)) void deinit() {
  DP("Deinit Level0 plugin!\n");
#ifndef _WIN32
  closeRTL();
#endif
  delete TLSList;
  TLSList = nullptr;
  delete DeviceInfo;
  DeviceInfo = nullptr;
}

#ifdef _WIN32
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
    if (!DeviceInfo->Flags.EnableProfile)
      return;
    Profile = DeviceInfo->getProfile(DeviceId);
    start();
  }
  ScopedTimerTy(int32_t DeviceId, std::string name) : Name(name) {
    if (!DeviceInfo->Flags.EnableProfile)
      return;
    Profile = DeviceInfo->getProfile(DeviceId);
    start();
  }
  ScopedTimerTy(int32_t DeviceId, const char *Prefix, const char *name) {
    if (!DeviceInfo->Flags.EnableProfile)
      return;
    Name = Prefix;
    Name += name;
    Profile = DeviceInfo->getProfile(DeviceId);
    start();
  }
  ~ScopedTimerTy() {
    if (!DeviceInfo->Flags.EnableProfile)
      return;
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

static uint64_t getDeviceArch(uint32_t L0DeviceId) {
  for (auto &arch : DeviceArchMap)
    for (auto id : arch.second)
      if (L0DeviceId == id || (L0DeviceId & 0xFF00) == id)
        return arch.first; // Exact match or prefix match

  DP("Warning: Cannot decide device arch for %" PRIx32 ".\n", L0DeviceId);
  return DeviceArch_None;
}

static bool isDiscrete(uint32_t L0DeviceId) {
  uint32_t prefix = L0DeviceId & 0xFF00;
  return prefix == 0x4900 || prefix == 0x0200 || prefix == 0x0b00;
}

// Decide device's default memory kind for internal allocation (e.g., map)
static int32_t getAllocKinds(uint32_t L0DeviceId) {
  return isDiscrete(L0DeviceId) ? TARGET_ALLOC_DEVICE : TARGET_ALLOC_SHARED;
}

static void logMemUsage(ze_device_handle_t Device, size_t Requested,
                        void *Ptr, size_t MemSize) {
  size_t size = 0;
  bool pool = false;
  if (MemSize > 0) {
    /// Pool is being used
    size = MemSize;
    pool = true;
  } else {
    void *base = nullptr;
    CALL_ZE_EXIT_FAIL(zeMemGetAddressRange, DeviceInfo->Context, Ptr, &base,
                      &size);
  }
  auto memType = DeviceInfo->getMemAllocType(Ptr);
  if (memType == ZE_MEMORY_TYPE_HOST)
    DeviceInfo->MemStatHost.update(Requested, size, pool);
  else if (memType == ZE_MEMORY_TYPE_SHARED)
    DeviceInfo->MemStatShared[Device].update(Requested, size, pool);
  else // ZE_MEMORY_TYPE_DEVICE
    DeviceInfo->MemStatDevice[Device].update(Requested, size, pool);
}

static void addDataTransferLatency() {
  if (DeviceInfo->DataTransferLatency == 0)
    return;
  double goal = omp_get_wtime() + 1e-6 * DeviceInfo->DataTransferLatency;
  while (omp_get_wtime() < goal)
    ;
}

/// Clean-up routine to be invoked by the destructor or __tgt_rtl_deinit.
static void closeRTL() {
  // Nothing to clean up
  if (DeviceInfo->NumDevices == 0)
    return;

  for (uint32_t i = 0; i < DeviceInfo->NumDevices; i++) {
    if (!DeviceInfo->Initialized[i])
      continue;
    if (OMPT_ENABLED) {
      OMPT_CALLBACK(ompt_callback_device_unload, i, 0 /* module ID */);
      OMPT_CALLBACK(ompt_callback_device_finalize, i);
    }
    DeviceInfo->Mutexes[i].lock();
    for (auto mem : DeviceInfo->OwnedMemory[i]) {
      LOG_MEM_USAGE(DeviceInfo->Devices[i], 0, mem);
      CALL_ZE_EXIT_FAIL(zeMemFree, DeviceInfo->Context, mem);
    }

    for (auto &Entries : DeviceInfo->FuncGblEntries[i]) {
      for (auto Kernel : Entries.Kernels)
        if (Kernel)
          CALL_ZE_EXIT_FAIL(zeKernelDestroy, Kernel);
      for (auto Module : Entries.Modules)
        CALL_ZE_EXIT_FAIL(zeModuleDestroy, Module);
    }

    if (DeviceInfo->DeviceMode == DEVICE_MODE_TOP &&
        DeviceInfo->NumDevices > DeviceInfo->NumRootDevices &&
        i < DeviceInfo->NumRootDevices) {
      auto &subDeviceEvent = DeviceInfo->SubDeviceEvents[i];
      for (auto event : subDeviceEvent.Events)
        CALL_ZE_EXIT_FAIL(zeEventDestroy, event);
      CALL_ZE_EXIT_FAIL(zeEventPoolDestroy, subDeviceEvent.Pool);
    }
    DeviceInfo->Mutexes[i].unlock();

    if (DeviceInfo->Flags.EnableTargetGlobals)
      DeviceInfo->unloadOffloadTable(i);
  }

  if (TLSList)
    for (auto TLSPtr : *TLSList)
      delete TLSPtr;

  if (DeviceInfo->Flags.UseMemoryPool) {
    DeviceInfo->MemPoolHost.deinit();
    for (auto &pool : DeviceInfo->MemPoolShared)
      pool.second.deinit();
    for (auto &pool : DeviceInfo->MemPoolDevice)
      pool.second.deinit();
  }

  if (DebugLevel > 0) {
    DeviceInfo->MemStatHost.print();
    for (auto &stat : DeviceInfo->MemStatShared)
      stat.second.print();
    for (auto &stat : DeviceInfo->MemStatDevice)
      stat.second.print();
  }

  if (DeviceInfo->Flags.EnableProfile)
    DeviceInfo->ProfileEvents.deinit();

  if (DeviceInfo->Context)
    CALL_ZE_EXIT_FAIL(zeContextDestroy, DeviceInfo->Context);

  delete[] DeviceInfo->Mutexes;
  delete[] DeviceInfo->DataMutexes;
  delete DeviceInfo->RTLMutex;
  DP("Closed RTL successfully\n");
}

static int32_t copyData(int32_t DeviceId, void *Dest, void *Src, size_t Size,
                        std::unique_lock<std::mutex> &copyLock) {
  auto destType = DeviceInfo->getMemAllocType(Dest);
  auto srcType = DeviceInfo->getMemAllocType(Src);

  // Global variable address is classified as unknown memory, and it should be
  // handled as device memory (i.e., use memcpy command).
  if (isDiscrete(DeviceInfo->DeviceProperties[DeviceId].deviceId) ||
      destType == ZE_MEMORY_TYPE_DEVICE || srcType == ZE_MEMORY_TYPE_DEVICE ||
      (destType == ZE_MEMORY_TYPE_UNKNOWN &&
          srcType == ZE_MEMORY_TYPE_UNKNOWN)) {
    auto cmdList = DeviceInfo->getCopyCmdList(DeviceId);
    auto cmdQueue = DeviceInfo->getCopyCmdQueue(DeviceId);
    bool ownsLock = false;
    if (!copyLock.owns_lock()) {
      copyLock.lock();
      ownsLock = true;
    }
    DP("Copy Engine is %s for data transfer\n",
       DeviceInfo->usingCopyCmdQueue(DeviceId) ? "used" : "not used");
    CALL_ZE_RET_FAIL(zeCommandListAppendMemoryCopy, cmdList, Dest, Src, Size,
                     nullptr, 0, nullptr);
    CALL_ZE_RET_FAIL(zeCommandListClose, cmdList);
    CALL_ZE_RET_FAIL(zeCommandQueueExecuteCommandLists, cmdQueue, 1, &cmdList,
                     nullptr);
    if (ownsLock)
      copyLock.unlock();
    CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, cmdQueue, UINT64_MAX);
    CALL_ZE_RET_FAIL(zeCommandListReset, cmdList);
  } else {
    char *src = static_cast<char *>(Src);
    std::copy(src, src + Size, static_cast<char *>(Dest));
  }
  return OFFLOAD_SUCCESS;
}

/// Allocate data explicitly
static void *allocDataExplicit(ze_device_handle_t Device, int64_t Size,
                               int32_t Kind, bool LargeMem,
                               bool LogMemAlloc) {
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

  // Use relaxed allocation limit if driver supports
  ze_relaxed_allocation_limits_exp_desc_t relaxedAllocDesc = {
    ZE_STRUCTURE_TYPE_RELAXED_ALLOCATION_LIMITS_EXP_DESC,
    nullptr,
    ZE_RELAXED_ALLOCATION_LIMITS_EXP_FLAG_MAX_SIZE
  };
  if (LargeMem && DeviceInfo->DriverAPIVersion >= ZE_API_VERSION_1_1) {
    deviceDesc.pNext = &relaxedAllocDesc;
    hostDesc.pNext = &relaxedAllocDesc;
  }

  auto context = DeviceInfo->Context;

  switch (Kind) {
  case TARGET_ALLOC_DEVICE:
    CALL_ZE_RET_NULL(zeMemAllocDevice, context, &deviceDesc, Size,
                     LEVEL0_ALIGNMENT, Device, &mem);
    DP("Allocated a device memory object " DPxMOD "\n", DPxPTR(mem));
    break;
  case TARGET_ALLOC_HOST:
    CALL_ZE_RET_NULL(zeMemAllocHost, context, &hostDesc, Size,
                     LEVEL0_ALIGNMENT, &mem);
    DP("Allocated a host memory object " DPxMOD "\n", DPxPTR(mem));
    break;
  case TARGET_ALLOC_SHARED:
    CALL_ZE_RET_NULL(zeMemAllocShared, context, &deviceDesc, &hostDesc,
                     Size, LEVEL0_ALIGNMENT, Device, &mem);
    DP("Allocated a shared memory object " DPxMOD "\n", DPxPTR(mem));
    break;
  default:
    FATAL_ERROR("Invalid target data allocation kind");
  }

  if (LogMemAlloc)
    LOG_MEM_USAGE(Device, Size, mem);

  return mem;
}

static void *allocDataExplicit(int32_t DeviceId, int64_t Size, int32_t Kind) {
  auto device = DeviceInfo->Devices[DeviceId];
  auto DeviceMaxMem = DeviceInfo->DeviceProperties[DeviceId].maxMemAllocSize;
  return allocDataExplicit(device, Size, Kind, (uint64_t)Size > DeviceMaxMem);
}

TLSTy::~TLSTy() {
  for (auto CmdList : CmdLists)
    CALL_ZE_EXIT_FAIL(zeCommandListDestroy, CmdList.second);
  for (auto CmdList : CopyCmdLists)
    CALL_ZE_EXIT_FAIL(zeCommandListDestroy, CmdList.second);
  for (auto CmdQueue : CmdQueues)
    CALL_ZE_EXIT_FAIL(zeCommandQueueDestroy, CmdQueue.second);
  for (auto CmdQueue : CopyCmdQueues)
    CALL_ZE_EXIT_FAIL(zeCommandQueueDestroy, CmdQueue.second);
  for (auto Profile : Profiles)
    delete Profile.second;
}

int32_t CommandBatchTy::begin(int32_t ID) {
  if (State < 0 || (DeviceId >= 0 && ID != DeviceId)) {
    DP("Invalid command batching state\n");
    return OFFLOAD_FAIL;
  }
  DP("Command batching begins\n");
  DeviceId = ID;
  if (CmdList == nullptr || CmdQueue == nullptr) {
    if (DeviceInfo->CommandBatchLevel > 1) {
      CmdList = DeviceInfo->getCmdList(DeviceId);
      CmdQueue = DeviceInfo->getCmdQueue(DeviceId);
    } else {
      CmdList = DeviceInfo->getCopyCmdList(DeviceId);
      CmdQueue = DeviceInfo->getCopyCmdQueue(DeviceId);
    }
  }
  State++;
  return OFFLOAD_SUCCESS;
}

int32_t CommandBatchTy::end() {
  if (State <= 0 || DeviceId < 0) {
    DP("Invalid command batching state\n");
    return OFFLOAD_FAIL;
  }
  DP("Command batching ends\n");
  State--;
  if (State > 0) {
    // Batching is still in progress
    return OFFLOAD_SUCCESS;
  }
  if (NumCopyTo == 0 && NumCopyFrom == 0 && Kernel == nullptr) {
    // Nothing was enqueued
    return OFFLOAD_SUCCESS;
  }

  if (commit(true) != OFFLOAD_SUCCESS)
    return OFFLOAD_FAIL;

  // Commit enqueued memory free
  for (auto Ptr : MemFreeList)
    if (DeviceInfo->dataDelete(DeviceId, Ptr) != OFFLOAD_SUCCESS)
      return OFFLOAD_FAIL;
  MemFreeList.clear();

  DeviceId = -1;

  DP("Command batching completed\n");

  return OFFLOAD_SUCCESS;
}

int32_t CommandBatchTy::commit(bool Always) {
  int32_t BatchCount = NumCopyTo + NumCopyFrom + (Kernel ? 1 : 0);
  if (!Always && BatchCount < DeviceInfo->CommandBatchCount)
    return OFFLOAD_SUCCESS;

  DP("Command batching commits %" PRId32 " enqueued commands\n", BatchCount);

  double BatchTime = 0;
  if (DeviceInfo->Flags.EnableProfile)
    BatchTime = omp_get_wtime();

  // Launch enqueued commands
  CALL_ZE_RET_FAIL(zeCommandListClose, CmdList);
  CALL_ZE_RET_FAIL(zeCommandQueueExecuteCommandLists, CmdQueue, 1, &CmdList,
                   nullptr);
  CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, CmdQueue, UINT64_MAX);
  CALL_ZE_RET_FAIL(zeCommandListReset, CmdList);

  if (DeviceInfo->Flags.EnableProfile) {
    BatchTime = omp_get_wtime() - BatchTime;
    auto *Profile = DeviceInfo->getProfile(DeviceId);
    if (Kernel) {
      double DeviceTime = Profile->getEventTime(KernelEvent);
      std::string KernelName = "Kernel ";
      KernelName += DeviceInfo->KernelProperties[DeviceId][Kernel].Name;
      if (NumCopyTo > 0 || NumCopyFrom > 0) {
        // Batch includes copy and kernel launch
        BatchTime -= DeviceTime;
        Profile->update(KernelName, DeviceTime, DeviceTime);
      } else {
        // Batch only includes kernel launch
        Profile->update(KernelName, BatchTime, DeviceTime);
      }
    }
    if (NumCopyTo > 0 && NumCopyFrom > 0)
      Profile->update("DataCopy", BatchTime, BatchTime);
    else if (NumCopyTo > 0)
      Profile->update("DataWrite (Host to Device)", BatchTime, BatchTime);
    else if (NumCopyFrom > 0)
      Profile->update("DataRead (Device to Host)", BatchTime, BatchTime);
  }

  // Commit enqueued memory copy from staging buffer to host buffer
  for (auto &Arg : MemCopyList)
    std::copy_n((const char *)Arg.Src, Arg.Size, (char *)Arg.Dst);
  MemCopyList.clear();

  NumCopyTo = 0;
  NumCopyFrom = 0;
  Kernel = nullptr;
  KernelEvent = nullptr;

  // Reset staging buffer
  getTLS()->getStagingBuffer().reset();

  return OFFLOAD_SUCCESS;
}

int32_t CommandBatchTy::enqueueMemCopyTo(
    int32_t ID, void *Dst, void *Src, size_t Size) {
  if (DeviceId != ID) {
    DP("Invalid device ID %" PRId32 " while performing command batching\n", ID);
    return OFFLOAD_FAIL;
  }

  void *SrcPtr = Src;
  if (Size <= DeviceInfo->StagingBufferSize &&
      DeviceInfo->getMemAllocType(Src) == ZE_MEMORY_TYPE_UNKNOWN) {
    SrcPtr = DeviceInfo->getStagingBuffer().getNext();
    std::copy_n(static_cast<char *>(Src), Size, static_cast<char *>(SrcPtr));
  }

  CALL_ZE_RET_FAIL(zeCommandListAppendMemoryCopy, CmdList, Dst, SrcPtr, Size,
                   nullptr, 0, nullptr);
  CALL_ZE_RET_FAIL(zeCommandListAppendBarrier, CmdList, nullptr, 0, nullptr);
  DP("Enqueued memory copy " DPxMOD " --> " DPxMOD "\n", DPxPTR(Src),
     DPxPTR(Dst));

  NumCopyTo++;

  return commit();
}

int32_t CommandBatchTy::enqueueMemCopyFrom(
    int32_t ID, void *Dst, void *Src, size_t Size) {
  if (DeviceId != ID) {
    DP("Invalid device ID %" PRId32 " while performing command batching\n", ID);
    return OFFLOAD_FAIL;
  }

  void *DstPtr = Dst;
  if (Size <= DeviceInfo->StagingBufferSize &&
      DeviceInfo->getMemAllocType(Dst) == ZE_MEMORY_TYPE_UNKNOWN) {
    DstPtr = DeviceInfo->getStagingBuffer().getNext();
    // Delayed copy from staging buffer to host buffer
    MemCopyList.emplace_back(Dst, DstPtr, Size);
  }

  CALL_ZE_RET_FAIL(zeCommandListAppendMemoryCopy, CmdList, DstPtr, Src, Size,
                   nullptr, 0, nullptr);
  CALL_ZE_RET_FAIL(zeCommandListAppendBarrier, CmdList, nullptr, 0, nullptr);
  DP("Enqueued memory copy " DPxMOD " --> " DPxMOD "\n", DPxPTR(Src),
     DPxPTR(Dst));

  NumCopyFrom++;

  return commit();
}

int32_t CommandBatchTy::enqueueLaunchKernel(
    int32_t ID, ze_kernel_handle_t _Kernel, ze_group_count_t *GroupCounts) {
  if (DeviceId != ID) {
    DP("Invalid device ID %" PRId32 " while performing command batching\n", ID);
    return OFFLOAD_FAIL;
  }

  Kernel = _Kernel;
  if (DeviceInfo->Flags.EnableProfile)
    KernelEvent = DeviceInfo->ProfileEvents.getEvent();

  CALL_ZE_RET_FAIL(zeCommandListAppendLaunchKernel, CmdList, Kernel,
                   GroupCounts, KernelEvent, 0, nullptr);
  CALL_ZE_RET_FAIL(zeCommandListAppendBarrier, CmdList, nullptr, 0, nullptr);
  DP("Enqueued launch kernel " DPxMOD "\n", DPxPTR(Kernel));

  return commit();
}

int32_t CommandBatchTy::enqueueMemFree(int32_t ID, void *Ptr) {
  if (DeviceId != ID) {
    DP("Invalid device ID %" PRId32 " while performing command batching\n", ID);
    return OFFLOAD_FAIL;
  }

  MemFreeList.push_back(Ptr);

  return OFFLOAD_SUCCESS;
}

/// Initialize memory pool with the parameters
void MemoryPoolTy::init(int32_t allocKind, RTLDeviceInfoTy *RTL) {
  if (Initialized)
    return;

  if (RTL->MemPoolInfo.count(allocKind) == 0) {
    DP("Memory pool is disabled for %s\n", ALLOC_KIND_TO_STR(allocKind));
    return;
  }

  // Use fixed parameters for shared memory pool on discrete device.
  ze_device_properties_t properties = DevicePropertiesInit;
  bool fixedParams = false;
  if (allocKind == TARGET_ALLOC_SHARED) {
    CALL_ZE_EXIT_FAIL(zeDeviceGetProperties, Device, &properties);
    fixedParams = isDiscrete(properties.deviceId);
  }

  size_t userAllocMax = RTL->MemPoolInfo[allocKind][0];
  size_t userCapacity = fixedParams ? 1 : RTL->MemPoolInfo[allocKind][1];
  size_t userPoolSize = RTL->MemPoolInfo[allocKind][2];

  Context = RTL->Context;
  AllocKind = allocKind;
  BlockCapacity = userCapacity;
  PoolSizeMax = userPoolSize << 20; // MB to B
  PoolSize = 0;

  // Decide AllocUnit. Do not log this allocation.
  void *mem = allocDataExplicit(Device, 8, AllocKind, false, false);
  CALL_ZE_EXIT_FAIL(zeMemGetAddressRange, Context, mem, nullptr,
                    &AllocUnit);
  CALL_ZE_EXIT_FAIL(zeMemFree, Context, mem);

  // Convert MB to B and round up to power of 2
  AllocMax = fixedParams ? AllocUnit / BlockCapacity
                         : AllocMin << getBucketId(userAllocMax * (1 << 20));
  auto minSize = getBucketId(AllocMin);
  auto maxSize = getBucketId(AllocMax);
  Buckets.resize(maxSize - minSize + 1);
  assert(AllocMin < AllocMax && AllocMax < PoolSizeMax &&
         "Invalid parameters while initializing memory pool");

  // Set bucket parameters
  for (size_t i = 0; i < Buckets.size(); i++) {
    size_t chunkSize = AllocMin << i;
    size_t blockSize = (chunkSize * BlockCapacity <= AllocUnit)
        ? AllocUnit
        : chunkSize * BlockCapacity;
    BucketParams.emplace_back(chunkSize, blockSize);
  }

  Initialized = 1;

  DP("Initialized %s pool for device " DPxMOD ": AllocMax = %zu, "
     "Capacity = %" PRIu32 ", PoolSizeMax = %zu\n",
     ALLOC_KIND_TO_STR(AllocKind), DPxPTR(Device), AllocMax, BlockCapacity,
     PoolSizeMax);
}

/// Initialize module data
int32_t RTLDeviceInfoTy::initProgramData(int32_t DeviceId) {
  // Prepare host data to copy
  auto &P = DeviceProperties[DeviceId];
  uint32_t totalEUs =
      P.numSlices * P.numSubslicesPerSlice * P.numEUsPerSubslice;

  // Allocate dynamic memory for in-kernel allocation
  void *memLB = 0;
  uintptr_t memUB = 0;
  if (KernelDynamicMemorySize > 0)
    memLB = allocDataExplicit(Devices[DeviceId], KernelDynamicMemorySize,
                              TARGET_ALLOC_DEVICE);
  if (memLB) {
    OwnedMemory[DeviceId].push_back(memLB);
    memUB = (uintptr_t)memLB + KernelDynamicMemorySize;
  }

  ModuleDataTy hostData = {
    1,                   // Initialized
    (int32_t)NumDevices, // Number of devices
    DeviceId,            // Device ID
    totalEUs,            // Total EUs
    P.numThreadsPerEU,   // HW threads per EU
    (uintptr_t)memLB,    // Dynamic memory LB
    memUB,               // Dynamic memory UB
    0                    // Device type (0 for GPU, 1 for CPU)
  };

  // Look up program data location on device
  void *dataPtr = getVarDeviceAddr(DeviceId, "__omp_spirv_program_data",
                  sizeof(hostData));
  if (!dataPtr) {
    DP("Warning: cannot find module data location on device.\n");
    return OFFLOAD_SUCCESS;
  }

  return enqueueMemCopy(DeviceId, dataPtr, &hostData, sizeof(hostData));
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
  // Kernel-dependent flags
  auto flags = KernelProperties[DeviceId][Kernel].IndirectAccessFlags;

  // Other flags due to users' memory allocation
  if (!ImplicitArgsDevice[DeviceId].empty())
    flags |= ZE_KERNEL_INDIRECT_ACCESS_FLAG_DEVICE;
  if (!ImplicitArgsHost[DeviceId].empty())
    flags |= ZE_KERNEL_INDIRECT_ACCESS_FLAG_HOST;
  if (!ImplicitArgsShared[DeviceId].empty())
    flags |= ZE_KERNEL_INDIRECT_ACCESS_FLAG_SHARED;

  return flags;
}

/// Enqueue memory copy
int32_t RTLDeviceInfoTy::enqueueMemCopy(
    int32_t DeviceId, void *Dst, void *Src, size_t Size) {
  auto cmdList = getCopyCmdList(DeviceId);
  auto cmdQueue = getCopyCmdQueue(DeviceId);

  CALL_ZE_RET_FAIL(zeCommandListAppendMemoryCopy, cmdList, Dst, Src, Size,
                   nullptr, 0, nullptr);
  CALL_ZE_RET_FAIL(zeCommandListClose, cmdList);
  CALL_ZE_RET_FAIL(zeCommandQueueExecuteCommandLists, cmdQueue, 1, &cmdList,
                   nullptr);
  CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, cmdQueue, UINT64_MAX);
  CALL_ZE_RET_FAIL(zeCommandListReset, cmdList);

  return OFFLOAD_SUCCESS;
}

/// Return the memory allocation type for the specified memory location.
uint32_t RTLDeviceInfoTy::getMemAllocType(const void *Ptr) {
  ze_memory_allocation_properties_t properties = {
    ZE_STRUCTURE_TYPE_MEMORY_ALLOCATION_PROPERTIES,
    nullptr, // extension
    ZE_MEMORY_TYPE_UNKNOWN, // type
    0, // id
    0, // page size
  };

  ze_result_t rc;
  CALL_ZE(rc, zeMemGetAllocProperties, Context, Ptr, &properties, nullptr);

  if (rc == ZE_RESULT_ERROR_INVALID_ARGUMENT)
    return ZE_MEMORY_TYPE_UNKNOWN;
  else
    return properties.type;
}

/// Allocate memory from pool
void *RTLDeviceInfoTy::poolAlloc(int32_t DeviceId, size_t Size, int32_t Kind,
                                 intptr_t Offset) {
  auto device = Devices[DeviceId];
  void *mem = nullptr;

  switch (Kind) {
  case TARGET_ALLOC_HOST:
    mem = MemPoolHost.alloc(Size, Offset);
    break;
  case TARGET_ALLOC_SHARED:
    mem = MemPoolShared[device].alloc(Size, Offset);
    break;
  case TARGET_ALLOC_DEVICE:
    mem = MemPoolDevice[device].alloc(Size, Offset);
    break;
  default:
    DP("Invalid allocation kind while allocating memory from pool\n");
  }

  return mem;
}

/// Return memory to pool
bool RTLDeviceInfoTy::poolFree(int32_t DeviceId, void *Ptr) {
  auto memType = getMemAllocType(Ptr);
  auto device = Devices[DeviceId];
  bool ret = false;

  switch (memType) {
  case ZE_MEMORY_TYPE_HOST:
    ret = MemPoolHost.dealloc(Ptr);
    break;
  case ZE_MEMORY_TYPE_SHARED:
    ret = MemPoolShared[device].dealloc(Ptr);
    break;
  case ZE_MEMORY_TYPE_DEVICE:
    ret = MemPoolDevice[device].dealloc(Ptr);
    break;
  default:
    DP("Invalid memory type while freeing memory to pool\n");
  }

  return ret;
}

/// Initialize all memory pool
void RTLDeviceInfoTy::initMemoryPool() {
  MemPoolHost.init(TARGET_ALLOC_HOST, this);
  for (auto &pool : MemPoolShared)
    pool.second.init(TARGET_ALLOC_SHARED, this);
  for (auto &pool : MemPoolDevice)
    pool.second.init(TARGET_ALLOC_DEVICE, this);
}

/// Initialize memory stats
void RTLDeviceInfoTy::initMemoryStat() {
  MemStatHost = MemStatTy(0, TARGET_ALLOC_HOST);
  for (uint32_t I = 0; I < NumDevices; I++) {
    if (MemStatShared.count(Devices[I]) == 0)
      MemStatShared.emplace(Devices[I], MemStatTy(I, TARGET_ALLOC_SHARED));
    if (MemStatDevice.count(Devices[I]) == 0)
      MemStatDevice.emplace(Devices[I], MemStatTy(I, TARGET_ALLOC_DEVICE));
  }
}

/// Create a new command queue for the given OpenMP device ID
ze_command_queue_handle_t
RTLDeviceInfoTy::createCommandQueue(int32_t DeviceId) {
  auto cmdQueue = createCmdQueue(Context, Devices[DeviceId],
                                 CmdQueueGroupOrdinals[DeviceId],
                                 CmdQueueIndices[DeviceId],
                                 DeviceIdStr[DeviceId]);
  return cmdQueue;
}

/// Get thread-local staging buffer for copying
StagingBufferTy &RTLDeviceInfoTy::getStagingBuffer() {
  auto &Buffer = getTLS()->getStagingBuffer();
  if (!Buffer.initialized())
    Buffer.init(Context, StagingBufferSize, StagingBufferCount);

  return Buffer;
}

void RTLDeviceInfoTy::addHostAccessible(
    int32_t DeviceId, void *Ptr, size_t Size, int32_t Kind) {
  uint32_t MemType = ZE_MEMORY_TYPE_UNKNOWN;
  if (Kind == TARGET_ALLOC_DEFAULT)
    MemType = getMemAllocType(Ptr);

  if (Kind == TARGET_ALLOC_HOST || MemType == ZE_MEMORY_TYPE_HOST)
    MemHostAccessible.at(nullptr)->add(Ptr, Size);
  else if (Kind == TARGET_ALLOC_SHARED || MemType == ZE_MEMORY_TYPE_SHARED)
    MemHostAccessible.at(Devices[DeviceId])->add(Ptr, Size);
}

void RTLDeviceInfoTy::removeHostAccessible(int32_t DeviceId, void *Ptr) {
  auto MemType = getMemAllocType(Ptr);

  if (MemType == ZE_MEMORY_TYPE_HOST)
    MemHostAccessible.at(nullptr)->remove(Ptr);
  else if (MemType == ZE_MEMORY_TYPE_SHARED)
    MemHostAccessible.at(Devices[DeviceId])->remove(Ptr);
}

bool RTLDeviceInfoTy::readKernelInfo(
    int32_t DeviceId, const __tgt_offload_entry &KernelEntry) {
  const ze_kernel_handle_t *KernelPtr =
      reinterpret_cast<const ze_kernel_handle_t *>(KernelEntry.addr);
  const char *Name = KernelEntry.name;
  std::string InfoVarName(Name);
  InfoVarName += "_kernel_info";
  size_t InfoVarSize = 0;
  void *InfoVarAddr =
      getVarDeviceAddr(DeviceId, InfoVarName.c_str(), &InfoVarSize);
  // If there is no kernel info variable, then the kernel might have been
  // produced by older toolchain - this is acceptable, so return success.
  if (!InfoVarAddr)
    return true;
  if (InfoVarSize == 0) {
    DP("Error: kernel info variable cannot have 0 size.\n");
    return false;
  }
  std::vector<char> InfoBuffer;
  InfoBuffer.resize(InfoVarSize);
  auto RC = enqueueMemCopy(DeviceId, InfoBuffer.data(), InfoVarAddr,
                           InfoVarSize);
  if (RC != OFFLOAD_SUCCESS)
    return false;
  // TODO: add support for big-endian devices, if needed.
  //       Currently supported devices are little-endian.
  char *ReadPtr = InfoBuffer.data();
  uint32_t Version = llvm::support::endian::read32le(ReadPtr);
  if (Version == 0) {
    DP("Error: version 0 of kernel info structure is illegal.\n");
    return false;
  }
  if (Version > 1) {
    DP("Error: unsupported version (%" PRIu32 ") of kernel info structure.\n",
       Version);
    DP("Error: please use newer OpenMP offload runtime.\n");
    return false;
  }
  ReadPtr += 4;
  uint32_t KernelArgsNum = llvm::support::endian::read32le(ReadPtr);
  size_t ExpectedInfoVarSize = static_cast<size_t>(KernelArgsNum) * 8 + 8;
  if (InfoVarSize != ExpectedInfoVarSize) {
    DP("Error: expected kernel info variable size %zu - got %zu\n",
       ExpectedInfoVarSize, InfoVarSize);
    return false;
  }
  KernelInfoTy Info(Version);
  for (uint64_t I = 0; I < KernelArgsNum; ++I) {
    ReadPtr += 4;
    bool ArgIsLiteral = (llvm::support::endian::read32le(ReadPtr) != 0);
    ReadPtr += 4;
    uint32_t ArgSize = llvm::support::endian::read32le(ReadPtr);
    Info.addArgInfo(ArgIsLiteral, ArgSize);
  }

  FuncGblEntries[DeviceId].back().KernelInfo.emplace(
      std::make_pair(*KernelPtr, std::move(Info)));
  return true;
}

const KernelInfoTy *RTLDeviceInfoTy::getKernelInfo(
    int32_t DeviceId, const ze_kernel_handle_t &Kernel) const {
  for (auto &Entry : FuncGblEntries[DeviceId]) {
    auto &KernelInfo = Entry.KernelInfo;
    auto It = KernelInfo.find(Kernel);
    if (It != KernelInfo.end())
      return &(It->second);
  }

  return nullptr;
}

bool RTLDeviceInfoTy::isDiscreteDevice(int32_t DeviceId) {
  return isDiscrete(DeviceProperties[DeviceId].deviceId);
}

int32_t RTLDeviceInfoTy::getInternalDeviceId(int32_t DeviceId) {
#if !SUBDEVICE_USE_ROOT_MEMORY
  auto SubDeviceCode = DeviceInfo->getSubDeviceCode();
  if (SubDeviceCode < 0 && SUBDEVICE_GET_COUNT(SubDeviceCode) == 1) {
    auto subLevel = SUBDEVICE_GET_LEVEL(SubDeviceCode);
    auto subStart = SUBDEVICE_GET_START(SubDeviceCode);
    DeviceId = DeviceInfo->SubDeviceIds[DeviceId][subLevel][subStart];
  }
#endif
  return DeviceId;
}

int32_t RTLDeviceInfoTy::dataDelete(int32_t DeviceId, void *Ptr) {
  void *Base = nullptr;
  size_t Size = 0;

  auto &Mtx = Mutexes[DeviceId];

  Mtx.lock();
  removeImplicitArgs(DeviceId, Ptr);
  Mtx.unlock();

  removeHostAccessible(DeviceId, Ptr);

  if (Flags.UseMemoryPool) {
    bool Deallocated = poolFree(DeviceId, Ptr);
    if (Deallocated) {
      DP("Returned device memory " DPxMOD " to memory pool\n", DPxPTR(Ptr));
      return OFFLOAD_SUCCESS;
    }
  }
  CALL_ZE_RET_FAIL(zeMemGetAddressRange, Context, Ptr, &Base, &Size);
  LOG_MEM_USAGE(Devices[DeviceId], 0, Base);
  CALL_ZE_RET_FAIL_MTX(zeMemFree, Mtx, Context, Base);

  DP("Deleted device memory " DPxMOD " (Base: " DPxMOD ", Size: %zu)\n",
     DPxPTR(Ptr), DPxPTR(Base), Size);

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

// FIXME: move this to llvm/BinaryFormat/ELF.h and elf.h:
#define NT_INTEL_ONEOMP_OFFLOAD_VERSION 1
#define NT_INTEL_ONEOMP_OFFLOAD_IMAGE_COUNT 2
#define NT_INTEL_ONEOMP_OFFLOAD_IMAGE_AUX 3

static bool isValidOneOmpImage(__tgt_device_image *Image,
                               uint64_t &MajorVer,
                               uint64_t &MinorVer) {
  char *ImgBegin = reinterpret_cast<char *>(Image->ImageStart);
  char *ImgEnd = reinterpret_cast<char *>(Image->ImageEnd);
  size_t ImgSize = ImgEnd - ImgBegin;
  ElfL E(ImgBegin, ImgSize);
  if (!E.isValidElf()) {
    DP("Warning: unable to get ELF handle: %s!\n", E.getErrmsg(-1));
    return false;
  }

  for (auto I = E.section_notes_begin(), IE = E.section_notes_end(); I != IE;
       ++I) {
    ElfLNote Note = *I;
    if (Note.getNameSize() == 0)
      continue;
    std::string NameStr(Note.getName(), Note.getNameSize());
    if (NameStr != "INTELONEOMPOFFLOAD")
      continue;
    uint64_t Type = Note.getType();
    if (Type != NT_INTEL_ONEOMP_OFFLOAD_VERSION)
      continue;
    std::string DescStr(reinterpret_cast<const char *>(Note.getDesc()),
                        Note.getDescSize());
    auto DelimPos = DescStr.find('.');
    if (DelimPos == std::string::npos) {
      // The version has to look like "Major#.Minor#".
      DP("Invalid NT_INTEL_ONEOMP_OFFLOAD_VERSION: '%s'\n", DescStr.c_str());
      return false;
    }
    std::string MajorVerStr = DescStr.substr(0, DelimPos);
    DescStr.erase(0, DelimPos + 1);
    MajorVer = std::stoull(MajorVerStr);
    MinorVer = std::stoull(DescStr);
    bool isSupported = (MajorVer == 1 && MinorVer == 0);
    return isSupported;
  }

  return false;
}

EXTERN
int32_t __tgt_rtl_is_valid_binary(__tgt_device_image *Image) {
  uint64_t MajorVer, MinorVer;
  if (isValidOneOmpImage(Image, MajorVer, MinorVer)) {
    DP("Target binary is a valid oneAPI OpenMP image.\n");
    return 1;
  }

  DP("Target binary is *not* a valid oneAPI OpenMP image.\n");

  // Fallback to legacy behavior, when the image is a plain
  // SPIR-V file.
  uint32_t MagicWord = *(uint32_t *)Image->ImageStart;
  // compare magic word in little endian and big endian:
  int32_t Ret = (MagicWord == 0x07230203 || MagicWord == 0x03022307);
  DP("Target binary is %s\n", Ret ? "VALID" : "INVALID");

  return Ret;
}

EXTERN int64_t __tgt_rtl_init_requires(int64_t RequiresFlags) {
  DP("Initialize requires flags to %" PRId64 "\n", RequiresFlags);
  DeviceInfo->RequiresFlags = RequiresFlags;
  return RequiresFlags;
}

/// Check if the subdevice IDs are valid
static bool isValidSubDevice(int64_t DeviceIds) {
  if (DeviceIds >= 0) {
    DP("Invalid non-negative subdevice encoding %" PRId64 "\n", DeviceIds);
    return false;
  }

  uint32_t rootId = SUBDEVICE_GET_ROOT(DeviceIds);

  if (rootId >= DeviceInfo->NumRootDevices) {
    DP("Invalid root device ID %" PRIu32 "\n", rootId);
  }

  uint32_t subLevel = SUBDEVICE_GET_LEVEL(DeviceIds);
  uint32_t subStart = SUBDEVICE_GET_START(DeviceIds);
  uint32_t subCount = SUBDEVICE_GET_COUNT(DeviceIds);
  uint32_t subStride = SUBDEVICE_GET_STRIDE(DeviceIds);

  auto &subDeviceIds = DeviceInfo->SubDeviceIds[rootId];
  if (subLevel >= subDeviceIds.size()) {
    DP("Invalid subdevice level %" PRIu32 "\n", subLevel);
    return false;
  }
  for (uint32_t i = 0; i < subCount; i++) {
    uint32_t subId = subStart + i * subStride;
    if (subId >= DeviceInfo->SubDeviceIds[rootId][subLevel].size()) {
      DP("Invalid subdevice ID %" PRIu32 " at level %" PRIu32 "\n",
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
    DP("Finished checking %" PRIu32 " levels of subdevices.\n", Level);
    return OFFLOAD_SUCCESS;
  }

  uint32_t numDevices = 0;
  CALL_ZE_RET_FAIL(zeDeviceGetSubDevices, Parent, &numDevices, nullptr);

  if (numDevices == 0) {
    DP("No subdevices are found for device " DPxMOD " at level %" PRIu32 "\n",
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
    DP("Found subdevice " DPxMOD " for device " DPxMOD " at level %" PRIu32
       "\n", DPxPTR(device), DPxPTR(Parent), Level);
    if (getSubDevices(Level + 1, device, Lists) != OFFLOAD_SUCCESS)
      return OFFLOAD_FAIL;
  }

  return OFFLOAD_SUCCESS;
}

static int32_t appendDeviceProperties(
    ze_device_handle_t Device, std::string IdStr,
    uint32_t QueueOrdinal = UINT32_MAX, uint32_t QueueIndex = 0) {
  ze_device_properties_t properties = DevicePropertiesInit;
  ze_device_compute_properties_t computeProperties =
      DeviceComputePropertiesInit;

  DeviceInfo->Devices.push_back(Device);

  CALL_ZE_RET_FAIL(zeDeviceGetProperties, Device, &properties);
  DeviceInfo->DeviceProperties.push_back(properties);
  DeviceInfo->DeviceArchs.push_back(getDeviceArch(properties.deviceId));
  DeviceInfo->AllocKinds.push_back(getAllocKinds(properties.deviceId));

  CALL_ZE_RET_FAIL(zeDeviceGetComputeProperties, Device, &computeProperties);
  DeviceInfo->ComputeProperties.push_back(computeProperties);

  DeviceInfo->DeviceIdStr.push_back(IdStr);
  if (QueueOrdinal == UINT32_MAX)
    QueueOrdinal = getCmdQueueGroupOrdinal(Device);
  DeviceInfo->CmdQueueGroupOrdinals.push_back(QueueOrdinal);
  DeviceInfo->CmdQueueIndices.push_back(QueueIndex);
  DeviceInfo->CopyCmdQueueGroupOrdinals.push_back(
      getCmdQueueGroupOrdinalCopy(Device));

  DP("Found a GPU device, Name = %s\n", properties.name);

  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_number_of_devices() {
  DP("Looking for Level0 devices...\n");

  if (DeviceInfo->DeviceType != ZE_DEVICE_TYPE_GPU)
    return 0;

  CALL_ZE_RET_ZERO(zeInit, ZE_INIT_FLAG_GPU_ONLY);
  DP("Initialized L0, API %" PRIx32 "\n", ZE_API_VERSION_CURRENT);

  uint32_t numDrivers = 0;
  CALL_ZE_RET_ZERO(zeDriverGet, &numDrivers, nullptr);
  if (numDrivers == 0)
    return 0;

  std::vector<ze_driver_handle_t> driverHandles(numDrivers);
  CALL_ZE_RET_ZERO(zeDriverGet, &numDrivers, driverHandles.data());
  DP("Found %" PRIu32 " driver(s)!\n", numDrivers);

  auto deviceMode = DeviceInfo->DeviceMode;

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

    for (uint32_t i = 0; i < numDevices; i++) {
      auto device = devices[i];

      if (deviceMode == DEVICE_MODE_TOP || deviceMode == DEVICE_MODE_ALL) {
        auto Rc = appendDeviceProperties(device, std::to_string(i));
        if (Rc != OFFLOAD_SUCCESS)
          return 0;
      }

      if (DeviceInfo->Flags.UseMemoryPool) {
        DeviceInfo->MemPoolShared.emplace(device, device);
        DeviceInfo->MemPoolDevice.emplace(device, device);
      }
    }

    for (uint32_t i = 0; i < numDevices; i++) {
      ze_device_handle_t device = devices[i];

      // Find subdevices, add them to the device list, mark where they are.
      // Collect lists of subdevice handles first.
      SubDeviceListsTy subDeviceLists;
      if (getSubDevices(0, device, subDeviceLists) != OFFLOAD_SUCCESS)
        return 0;

      // Memory pool for L0 sub-devices
      if (DeviceInfo->Flags.UseMemoryPool && !subDeviceLists.empty()) {
        for (auto subDevice : subDeviceLists[0]) {
          DeviceInfo->MemPoolShared.emplace(subDevice, subDevice);
          DeviceInfo->MemPoolDevice.emplace(subDevice, subDevice);
        }
      }

      DeviceInfo->SubDeviceIds.emplace_back();
      auto &subDeviceIds = DeviceInfo->SubDeviceIds.back();

      // Fill internal data using the list of subdevice handles.
      // Internally, all devices/subdevices are listed as follows for N devices
      // where Subdevices(i,j) is a list of subdevices for device i at level j.
      // [0..N-1][Subdevices(0,0),Subdevices(0,1)]..[Subdevices(N-1,0)..]
      // Recursive subdevice query is not supported, so use existing query only
      // for the first-level subdevice, and use multi-context queue/list for the
      // second-level subdevice.
      if (!subDeviceLists.empty()) {
        // Fill per-device data for subdevice
        if (deviceMode != DEVICE_MODE_SUBSUB) {
          subDeviceIds.emplace_back();
          for (size_t k = 0; k < subDeviceLists[0].size(); k++) {
            auto subDevice = subDeviceLists[0][k];
            subDeviceIds.back().push_back(DeviceInfo->Devices.size());
            auto IdStr = std::to_string(i) + ".0." + std::to_string(k);
            auto Rc = appendDeviceProperties(subDevice, IdStr);
            if (Rc != OFFLOAD_SUCCESS)
              return 0;
          }
        }
        // Fill per-device data for subsubdevice
        if (deviceMode != DEVICE_MODE_SUB) {
          subDeviceIds.emplace_back();
          for (size_t k = 0; k < subDeviceLists[0].size(); k++) {
            auto subDevice = subDeviceLists[0][k];
            uint32_t numQueues = 0;
            uint32_t ordinal = getCmdQueueGroupOrdinalCCS(subDevice, numQueues);
            for (uint32_t j = 0; j < numQueues; j++) {
              subDeviceIds.back().push_back(DeviceInfo->Devices.size());
              auto IdStr =
                  std::to_string(i) + ".1." + std::to_string(k * numQueues + j);
              auto Rc = appendDeviceProperties(subDevice, IdStr, ordinal, j);
              if (Rc != OFFLOAD_SUCCESS)
                return 0;
            }
          }
        }
      } else {
        // Try to find second-level subdevices of the root device. Tile can be
        // exposed as root device, so we need to setup second-level subdevices.
        if (deviceMode != DEVICE_MODE_SUB) {
          // Only subDeviceIds[1] will be used
          subDeviceIds.resize(2);
          uint32_t numQueues = 0;
          uint32_t ordinal = getCmdQueueGroupOrdinalCCS(device, numQueues);
          if (ordinal != UINT32_MAX) {
            for (uint32_t j = 0; j < numQueues; j++) {
              subDeviceIds.back().push_back(DeviceInfo->Devices.size());
              auto IdStr = std::to_string(i) + ".1." + std::to_string(j);
              auto Rc = appendDeviceProperties(device, IdStr, ordinal, j);
              if (Rc != OFFLOAD_SUCCESS)
                return 0;
            }
          }
        }
      }
    }

    DeviceInfo->Driver = driverHandles[i];
    DeviceInfo->NumRootDevices = numDevices;
    DeviceInfo->NumDevices = DeviceInfo->Devices.size();
    DP("Found %" PRIu32 " root devices, %" PRIu32 " total devices.\n",
       DeviceInfo->NumRootDevices, DeviceInfo->NumDevices);
    DP("List of devices (DeviceID[.SubDeviceLevel.SubDeviceID])\n");
    for (auto &str : DeviceInfo->DeviceIdStr)
      DP("-- %s\n", str.c_str());
    break;
  }

  // Return early if no devices are available
  if (DeviceInfo->NumDevices == 0)
    return 0;

  CALL_ZE_RET_ZERO(zeDriverGetApiVersion, DeviceInfo->Driver,
                   &DeviceInfo->DriverAPIVersion);
  DP("Driver API version is %" PRIx32 "\n", DeviceInfo->DriverAPIVersion);
  L0Interop::printInteropProperties();

  DeviceInfo->CmdLists.resize(DeviceInfo->NumDevices);
  DeviceInfo->CmdQueues.resize(DeviceInfo->NumDevices);
  DeviceInfo->CopyCmdLists.resize(DeviceInfo->NumDevices);
  DeviceInfo->CopyCmdQueues.resize(DeviceInfo->NumDevices);
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
  DeviceInfo->Context = createContext(DeviceInfo->Driver);
  if (DeviceInfo->Flags.EnableProfile)
    DeviceInfo->ProfileEvents.init(DeviceInfo->Context);
  DeviceInfo->SubDeviceEvents.resize(DeviceInfo->NumRootDevices);
  DeviceInfo->RTLMutex = new std::mutex();

  // null-key for host memory type
  DeviceInfo->MemHostAccessible.emplace(nullptr,
                                        std::make_unique<MemRangeTy>());
  for (auto D : DeviceInfo->Devices)
    DeviceInfo->MemHostAccessible.emplace(D, std::make_unique<MemRangeTy>());

  if (DebugLevel > 0)
    DeviceInfo->initMemoryStat();

  // Look up GITS notification function
  ze_result_t Rc;
  CALL_ZE(Rc, zeDriverGetExtensionFunctionAddress, DeviceInfo->Driver,
          "zeGitsIndirectAllocationOffsets",
          &DeviceInfo->GitsIndirectAllocationOffsets);
  if (Rc != ZE_RESULT_SUCCESS)
    DeviceInfo->GitsIndirectAllocationOffsets = nullptr;

  if (deviceMode == DEVICE_MODE_TOP) {
    DP("Returning %" PRIu32 " top-level devices\n", DeviceInfo->NumRootDevices);
    return DeviceInfo->NumRootDevices;
  } else {
    // Just keep empty internal ID mapping in this case.
    DeviceInfo->SubDeviceIds.clear();
    DeviceInfo->SubDeviceIds.resize(DeviceInfo->NumDevices);
    DP("Returning %" PRIu32 " devices including sub-devices\n",
       DeviceInfo->NumDevices);
    return DeviceInfo->NumDevices;
  }
}

EXTERN int32_t __tgt_rtl_init_device(int32_t DeviceId) {
  if (DeviceId < 0 || DeviceId >= (int32_t)DeviceInfo->NumDevices ||
      (DeviceInfo->DeviceMode == DEVICE_MODE_TOP &&
       DeviceId >= (int32_t)DeviceInfo->NumRootDevices)) {
    DP("Bad device ID %" PRId32 "\n", DeviceId);
    return OFFLOAD_FAIL;
  }

  if (DeviceInfo->Flags.UseMemoryPool)
    DeviceInfo->initMemoryPool();

  uint32_t numSubDevices = 0;
  for (auto &subIds : DeviceInfo->SubDeviceIds[DeviceId]) {
    numSubDevices += subIds.size();
    for (auto subId : subIds)
      DeviceInfo->Initialized[subId] = true;
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

  DeviceInfo->Initialized[DeviceId] = true;

  OMPT_CALLBACK(ompt_callback_device_initialize, DeviceId,
                DeviceInfo->DeviceProperties[DeviceId].name,
                DeviceInfo->Devices[DeviceId],
                omptLookupEntries, OmptDocument);

  DP("Initialized Level0 device %" PRId32 "\n", DeviceId);
  return OFFLOAD_SUCCESS;
}

static ze_module_handle_t getModuleForImage(ze_context_handle_t Context,
                                            ze_device_handle_t Device,
                                            __tgt_device_image *Image,
                                            std::string &CompilationOptions) {
  uint64_t MajorVer, MinorVer;
  if (!isValidOneOmpImage(Image, MajorVer, MinorVer)) {
    // Handle legacy plain SPIR-V image.
    uint8_t *ImgBegin = reinterpret_cast<uint8_t *>(Image->ImageStart);
    uint8_t *ImgEnd = reinterpret_cast<uint8_t *>(Image->ImageEnd);
    size_t ImgSize = ImgEnd - ImgBegin;
    dumpImageToFile(ImgBegin, ImgSize, "OpenMP");
    return createModule(Context, Device, ImgSize,
                        ImgBegin, CompilationOptions.c_str(),
                        ZE_MODULE_FORMAT_IL_SPIRV);
  }

  // Iterate over the images and pick the first one that fits.
  char *ImgBegin = reinterpret_cast<char *>(Image->ImageStart);
  char *ImgEnd = reinterpret_cast<char *>(Image->ImageEnd);
  size_t ImgSize = ImgEnd - ImgBegin;
  ElfL E(ImgBegin, ImgSize);
  assert(E.isValidElf() &&
         "isValidOneOmpImage() returns true for invalid ELF image.");
  assert(MajorVer == 1 && MinorVer == 0 &&
         "FIXME: update image processing for new oneAPI OpenMP version.");
  // Collect auxiliary information.
  uint64_t ImageCount = 0;
  uint64_t MaxImageIdx = 0;
  struct V1ImageInfo {
    // 0 - native, 1 - SPIR-V
    uint64_t Format =  (std::numeric_limits<uint64_t>::max)();
    std::string CompileOpts;
    std::string LinkOpts;
    const uint8_t *Begin;
    uint64_t Size;

    V1ImageInfo(uint64_t Format, std::string CompileOpts,
                std::string LinkOpts, const uint8_t *Begin, uint64_t Size)
      : Format(Format), CompileOpts(CompileOpts),
        LinkOpts(LinkOpts), Begin(Begin), Size(Size) {}
  };

  std::unordered_map<uint64_t, V1ImageInfo> AuxInfo;

  for (auto I = E.section_notes_begin(), IE = E.section_notes_end(); I != IE;
       ++I) {
    ElfLNote Note = *I;
    if (Note.getNameSize() == 0)
      continue;
    std::string NameStr(Note.getName(), Note.getNameSize());
    if (NameStr != "INTELONEOMPOFFLOAD")
      continue;
    uint64_t Type = Note.getType();
    std::string DescStr(reinterpret_cast<const char *>(Note.getDesc()),
                        Note.getDescSize());
    switch (Type) {
    default:
      DP("Warning: unrecognized INTELONEOMPOFFLOAD note.\n");
      break;
    case NT_INTEL_ONEOMP_OFFLOAD_VERSION:
      break;
    case NT_INTEL_ONEOMP_OFFLOAD_IMAGE_COUNT:
      ImageCount = std::stoull(DescStr);
      break;
    case NT_INTEL_ONEOMP_OFFLOAD_IMAGE_AUX: {
      std::vector<std::string> Parts;
      do {
        auto DelimPos = DescStr.find('\0');
        if (DelimPos == std::string::npos) {
          Parts.push_back(DescStr);
          break;
        }
        Parts.push_back(DescStr.substr(0, DelimPos));
        DescStr.erase(0, DelimPos + 1);
      } while (Parts.size() < 4);

      // Ignore records with less than 4 strings.
      if (Parts.size() != 4) {
        DP("Warning: short NT_INTEL_ONEOMP_OFFLOAD_IMAGE_AUX "
           "record is ignored.\n");
        continue;
      }

      uint64_t Idx = std::stoull(Parts[0]);
      MaxImageIdx = (std::max)(MaxImageIdx, Idx);
      if (AuxInfo.find(Idx) != AuxInfo.end()) {
        DP("Warning: duplicate auxiliary information for image %" PRIu64
           " is ignored.\n", Idx);
        continue;
      }
      AuxInfo.emplace(std::piecewise_construct,
                      std::forward_as_tuple(Idx),
                      std::forward_as_tuple(std::stoull(Parts[1]),
                                            Parts[2], Parts[3],
                                            // Image pointer and size
                                            // will be initialized later.
                                            nullptr, 0));
    }
    }
  }

  if (MaxImageIdx >= ImageCount)
    DP("Warning: invalid image index found in auxiliary information.\n");

  for (auto I = E.sections_begin(), IE = E.sections_end(); I != IE; ++I) {
    const char *Prefix = "__openmp_offload_spirv_";
    std::string SectionName((*I).getName() ? (*I).getName() : "");
    if (SectionName.find(Prefix) != 0)
      continue;
    SectionName.erase(0, std::strlen(Prefix));
    uint64_t Idx = std::stoull(SectionName);
    if (Idx >= ImageCount) {
      DP("Warning: ignoring image section (index %" PRIu64
         " is out of range).\n", Idx);
      continue;
    }

    auto AuxInfoIt = AuxInfo.find(Idx);
    if (AuxInfoIt == AuxInfo.end()) {
      DP("Warning: ignoring image section (no aux info).\n");
      continue;
    }

    AuxInfoIt->second.Begin = (*I).getContents();
    AuxInfoIt->second.Size = (*I).getSize();
  }

  for (uint64_t Idx = 0; Idx < ImageCount; ++Idx) {
    auto It = AuxInfo.find(Idx);
    if (It == AuxInfo.end()) {
      DP("Warning: image %" PRIu64
         " without auxiliary information is ingored.\n", Idx);
      continue;
    }

    const unsigned char *ImgBegin =
        reinterpret_cast<const unsigned char *>(It->second.Begin);
    size_t ImgSize = It->second.Size;
    dumpImageToFile(ImgBegin, ImgSize, "OpenMP");
    ze_module_handle_t Module = nullptr;
    bool IsBinary = false;
    std::string Options = CompilationOptions;
    if (DeviceInfo->Flags.UseImageOptions)
      Options += " " + It->second.CompileOpts + " " + It->second.LinkOpts;

    if (It->second.Format == 0) {
      // Native format.
      IsBinary = true;
      Module = createModule(Context, Device, ImgSize,
                            ImgBegin, Options.c_str(),
                            ZE_MODULE_FORMAT_NATIVE);
    } else if (It->second.Format == 1) {
      // SPIR-V format.
      Module = createModule(Context, Device, ImgSize,
                            ImgBegin, Options.c_str(),
                            ZE_MODULE_FORMAT_IL_SPIRV);
    } else {
      DP("Warning: image %" PRIu64 "is ignored due to unknown format.\n", Idx);
      continue;
    }

    if (!Module) {
      DP("Warning: failed to create program from %s (%" PRIu64 ").\n",
         IsBinary ? "binary" : "SPIR-V", Idx);
      continue;
    }

    DP("Created module from image #%" PRIu64 ".\n", Idx);
    CompilationOptions = Options;
    return Module;
  }

  return nullptr;
}

EXTERN
__tgt_target_table *__tgt_rtl_load_binary(int32_t DeviceId,
                                          __tgt_device_image *Image) {
  DP("Device %" PRId32 ": Loading binary from " DPxMOD "\n", DeviceId,
     DPxPTR(Image->ImageStart));

  size_t imageSize = (size_t)Image->ImageEnd - (size_t)Image->ImageStart;
  size_t numEntries = (size_t)(Image->EntriesEnd - Image->EntriesBegin);
  DP("Expecting to have %zu entries defined\n", numEntries);

  std::string compilationOptions(
      DeviceInfo->CompilationOptions + " " +
      DeviceInfo->UserCompilationOptions);
  DP("Base L0 module compilation options: %s\n", compilationOptions.c_str());
  compilationOptions += " " + DeviceInfo->InternalCompilationOptions;

  dumpImageToFile(Image->ImageStart, imageSize, "OpenMP");

  auto context = DeviceInfo->Context;
  auto device = DeviceInfo->Devices[DeviceId];

  ScopedTimerTy tmModuleCompile(DeviceId, "Compiling");

  DeviceInfo->FuncGblEntries[DeviceId].emplace_back();
  auto &FuncGblEntries = DeviceInfo->FuncGblEntries[DeviceId].back();
  auto &modules = FuncGblEntries.Modules;
  auto mainModule = getModuleForImage(context, device, Image,
                                      compilationOptions);
  if (!mainModule) {
    DP("Error: failed to create main module\n");
    return nullptr;
  }

  DPI("Final L0 module compilation options: %s\n", compilationOptions.c_str());
  modules.push_back(mainModule);

  tmModuleCompile.stop();

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
                                        compilationOptions.c_str(),
                                        ZE_MODULE_FORMAT_IL_SPIRV);
    if (deviceLibModule) {
      DP("Created a module for %s\n", name);
      modules.push_back(deviceLibModule);
    }
  }

  ScopedTimerTy tmModuleLink(DeviceId, "Linking");

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
        DP("Error: module link failed -- see below for details.\n");
        fprintf(stderr, "%s\n", logString.data());
      }
      CALL_ZE_RET_NULL(zeModuleBuildLogDestroy, linkLog);
      return nullptr;
    }
    CALL_ZE_RET_NULL(zeModuleBuildLogDestroy, linkLog);
  }

  tmModuleLink.stop();
#endif // ENABLE_LIBDEVICE_LINKING

  auto &entries = FuncGblEntries.Entries;
  auto &kernels = FuncGblEntries.Kernels;
  entries.resize(numEntries);
  kernels.resize(numEntries);

  // FIXME: table loading does not work at all on XeLP.
  // Enable it after CMPLRLIBS-33285 is fixed.
  ScopedTimerTy tmOffloadEntriesInit(DeviceId, "OffloadEntriesInit");
  if (DeviceInfo->Flags.EnableTargetGlobals &&
      DeviceInfo->DeviceArchs[DeviceId] != DeviceArch_XeLP &&
      !DeviceInfo->loadOffloadTable(DeviceId, numEntries))
    DP("Warning: offload table loading failed.\n");
  tmOffloadEntriesInit.stop();

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
        bool poolAllocated = false;
        tgtAddr = DeviceInfo->allocData(DeviceId, size, hstAddr, hstAddr,
                                        &poolAllocated);
        __tgt_rtl_data_submit(DeviceId, tgtAddr, hstAddr, size);
        if (!poolAllocated) {
          std::unique_lock<std::mutex> lock(DeviceInfo->DataMutexes[DeviceId]);
          DeviceInfo->OwnedMemory[DeviceId].push_back(tgtAddr);
        }
        DP("Warning: global variable '%s' allocated. "
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
      DP("Warning: Failed to create kernel %s\n", name);
      kernels[i] = nullptr;
    }
    entries[i].addr = &kernels[i];
    entries[i].name = name;

    // Do not try to query information for deleted kernels.
    if (!kernels[i])
      continue;

    if (!DeviceInfo->readKernelInfo(DeviceId, entries[i])) {
      DP("Error: failed to read kernel info for kernel %s\n", name);
      return nullptr;
    }

    // Store kernel name in the property.
    auto &RTLKernelProperties =
        DeviceInfo->KernelProperties[DeviceId][kernels[i]];
    RTLKernelProperties.Name = name;

    // Retrieve kernel group size info.
    ze_kernel_properties_t kernelProperties = KernelPropertiesInit;
    CALL_ZE(rc, zeKernelGetProperties, kernels[i], &kernelProperties);
    if (DeviceInfo->ForcedKernelWidth > 0) {
      RTLKernelProperties.Width = DeviceInfo->ForcedKernelWidth;
    } else {
      RTLKernelProperties.Width = kernelProperties.maxSubgroupSize;
      RTLKernelProperties.SIMDWidth = kernelProperties.maxSubgroupSize;
      uint32_t HWId = DeviceInfo->DeviceProperties[DeviceId].deviceId;
      HWId &= 0xFF00;
      // Temporary workaround before ze_kernel_preferred_group_size_properties_t
      // becomes available with L0 1.2.
      // Here we try to match OpenCL kernel property
      // CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE.
      if (HWId == 0x200 || HWId == 0xb00 || HWId == 0x4900)
        RTLKernelProperties.Width *= 2;
    }
    if (DebugLevel > 0) {
      void *entryAddr = Image->EntriesBegin[i].addr;
      const char *entryName = Image->EntriesBegin[i].name;
      DP("Kernel %" PRIu32 ": Entry = " DPxMOD ", Name = %s, NumArgs = %"
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
  __tgt_target_table &table = FuncGblEntries.Table;
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

void *RTLDeviceInfoTy::allocData(int32_t DeviceId, int64_t Size, void *HstPtr,
                                 void *HstBase, bool *PoolAllocated) {
#if !SUBDEVICE_USE_ROOT_MEMORY
  auto SubDeviceCode = getSubDeviceCode();
  if (SubDeviceCode < 0 && SUBDEVICE_GET_COUNT(SubDeviceCode) == 1) {
    auto subLevel = SUBDEVICE_GET_LEVEL(SubDeviceCode);
    auto subStart = SUBDEVICE_GET_START(SubDeviceCode);
    DeviceId = SubDeviceIds[DeviceId][subLevel][subStart];
  }
#endif

  // TODO: this seems necessary for now -- check with L0 driver team for details
  std::unique_lock<std::mutex> allocLock(Mutexes[DeviceId]);

  ScopedTimerTy tmDataAlloc(DeviceId, "DataAlloc");
  intptr_t offset = (intptr_t)HstPtr - (intptr_t)HstBase;
  size_t size = (offset < 0 && ABS(offset) >= Size) ? ABS(offset) + 1 : Size;

  offset = (offset >= 0) ? offset : 0;
  size += offset;

  void *base = nullptr;
  void *mem = nullptr;
  if (Flags.UseMemoryPool) {
    base = DeviceInfo->poolAlloc(DeviceId, size, AllocKinds[DeviceId], offset);
    if (base != nullptr) {
      mem = (void *)((intptr_t)base + offset);
      DP("Allocated target memory " DPxMOD " (Base: " DPxMOD ", Size: %zu) "
         "from memory pool for host ptr " DPxMOD "\n", DPxPTR(mem),
         DPxPTR(base), size, DPxPTR(HstPtr));
      if (PoolAllocated)
        *PoolAllocated = true;
      return mem;
    }
  }

  // Use device-specific allocation type
  base = allocDataExplicit(DeviceId, size, AllocKinds[DeviceId]);
  mem = (void *)((intptr_t)base + offset);

  if (DebugLevel > 0) {
    void *actualBase = nullptr;
    size_t actualSize = 0;
    CALL_ZE_RET_NULL(zeMemGetAddressRange, Context, mem, &actualBase,
                     &actualSize);
    assert(base == actualBase && "Invalid memory address range!");
    DP("Allocated target memory " DPxMOD " (Base: " DPxMOD
       ", Size: %zu) for host ptr " DPxMOD "\n", DPxPTR(mem),
       DPxPTR(actualBase), actualSize, DPxPTR(HstPtr));
  }

  return mem;
}

EXTERN void *__tgt_rtl_data_alloc(int32_t DeviceId, int64_t Size, void *HstPtr,
                                  int32_t Kind) {
  void *Mem = DeviceInfo->allocData(DeviceId, Size, HstPtr, HstPtr);
  DeviceInfo->addHostAccessible(DeviceId, Mem, Size);
  return Mem;
}

EXTERN void *__tgt_rtl_data_alloc_user(
    int32_t DeviceId, int64_t Size, void *HstPtr) {
  // Device memory by default respecting LIBOMPTARGET_LEVEL0_DEFAULT_TARGET_MEM
  int32_t allocKind = TARGET_ALLOC_DEVICE;

  if (DeviceInfo->TargetAllocKind != TARGET_ALLOC_DEFAULT)
    allocKind = DeviceInfo->TargetAllocKind;

  return __tgt_rtl_data_alloc_explicit(DeviceId, Size, allocKind);
}

EXTERN void *__tgt_rtl_data_alloc_base(int32_t DeviceId, int64_t Size,
                                       void *HstPtr, void *HstBase) {
  void *Mem = DeviceInfo->allocData(DeviceId, Size, HstPtr, HstBase);
  DeviceInfo->addHostAccessible(DeviceId, Mem, Size);
  return Mem;
}

EXTERN void *__tgt_rtl_data_alloc_managed(int32_t DeviceId, int64_t Size) {
  int32_t kind = DeviceInfo->Flags.UseHostMemForUSM ? TARGET_ALLOC_HOST
                                                    : TARGET_ALLOC_SHARED;
  return __tgt_rtl_data_alloc_explicit(DeviceId, Size, kind);
}

EXTERN int32_t __tgt_rtl_is_device_accessible_ptr(int32_t DeviceId, void *Ptr) {
  int32_t ret = DeviceInfo->getMemAllocType(Ptr) != ZE_MEMORY_TYPE_UNKNOWN;

  DP("Ptr " DPxMOD " is %sa device accessible memory pointer.\n", DPxPTR(Ptr),
     ret ? "" : "not ");
  return ret;
}

EXTERN void *__tgt_rtl_data_alloc_explicit(
    int32_t DeviceId, int64_t Size, int32_t Kind) {
  void *mem = nullptr;

  if (DeviceInfo->Flags.UseMemoryPool)
    mem = DeviceInfo->poolAlloc(DeviceId, Size, Kind);

  if (mem == nullptr)
    mem = allocDataExplicit(DeviceId, Size, Kind);

  if (mem) {
    DeviceInfo->addHostAccessible(DeviceId, mem, Size, Kind);
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

  DeviceId = DeviceInfo->getInternalDeviceId(DeviceId);

  auto &Batch = getTLS()->getCommandBatch();
  if (Batch.isActive() && DeviceInfo->isDiscreteDevice(DeviceId))
    return Batch.enqueueMemCopyTo(DeviceId, TgtPtr, HstPtr, Size);

  ScopedTimerTy tmDataWrite(DeviceId, "DataWrite (Host to Device)");

  // Add synthetic delay for experiments
  addDataTransferLatency();

  std::unique_lock<std::mutex> copyLock(DeviceInfo->Mutexes[DeviceId],
                                        std::defer_lock);

  if (AsyncEvent) {
    copyLock.lock();
    auto context = DeviceInfo->Context;
    auto cmdList = createCmdList(context, DeviceInfo->Devices[DeviceId],
                                 DeviceInfo->CmdQueueGroupOrdinals[DeviceId],
                                 DeviceInfo->DeviceIdStr[DeviceId]);
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
                     Size, nullptr, 0, nullptr);
    if (beginAsyncCommand(cmdList, cmdQueue,
        static_cast<AsyncEventTy *>(AsyncEvent), fence) == OFFLOAD_FAIL)
      return OFFLOAD_FAIL;
    DP("Asynchronous data submit started -- %" PRId64 " bytes (hst:"
       DPxMOD ") -> (tgt:" DPxMOD ")\n", Size, DPxPTR(HstPtr), DPxPTR(TgtPtr));
  } else {
    void *SrcPtr = HstPtr;
    if (static_cast<size_t>(Size) <= DeviceInfo->StagingBufferSize &&
        DeviceInfo->getMemAllocType(HstPtr) == ZE_MEMORY_TYPE_UNKNOWN &&
        DeviceInfo->isDiscreteDevice(DeviceId)) {
      SrcPtr = DeviceInfo->getStagingBuffer().get();
      std::copy_n(static_cast<char *>(HstPtr), Size,
                  static_cast<char *>(SrcPtr));
    }
    if (copyData(DeviceId, TgtPtr, SrcPtr, Size, copyLock) != OFFLOAD_SUCCESS)
      return OFFLOAD_FAIL;
    DP("Copied %" PRId64 " bytes (hst:" DPxMOD ") -> (tgt:" DPxMOD ")\n", Size,
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

  DeviceId = DeviceInfo->getInternalDeviceId(DeviceId);

  auto &Batch = getTLS()->getCommandBatch();
  if (Batch.isActive() && DeviceInfo->isDiscreteDevice(DeviceId))
    return Batch.enqueueMemCopyFrom(DeviceId, HstPtr, TgtPtr, Size);

  ScopedTimerTy tmDataRead(DeviceId, "DataRead (Device to Host)");

  // Add synthetic delay for experiments
  addDataTransferLatency();

  std::unique_lock<std::mutex> copyLock(DeviceInfo->Mutexes[DeviceId],
                                        std::defer_lock);

  if (AsyncEvent) {
    copyLock.lock();
    auto context = DeviceInfo->Context;
    auto cmdList = createCmdList(context, DeviceInfo->Devices[DeviceId],
                                 DeviceInfo->CmdQueueGroupOrdinals[DeviceId],
                                 DeviceInfo->DeviceIdStr[DeviceId]);
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
                     Size, nullptr, 0, nullptr);
    if (beginAsyncCommand(cmdList, cmdQueue,
        static_cast<AsyncEventTy *>(AsyncEvent), fence) == OFFLOAD_FAIL)
      return OFFLOAD_FAIL;
    DP("Asynchronous data retrieve started -- %" PRId64 " bytes (tgt:"
       DPxMOD ") -> (hst:" DPxMOD ")\n", Size, DPxPTR(TgtPtr), DPxPTR(HstPtr));
  } else {
    void *DstPtr = HstPtr;
    if (static_cast<size_t>(Size) <= DeviceInfo->StagingBufferSize &&
        DeviceInfo->getMemAllocType(HstPtr) == ZE_MEMORY_TYPE_UNKNOWN &&
        DeviceInfo->isDiscreteDevice(DeviceId))
      DstPtr = DeviceInfo->getStagingBuffer().get();
    if (copyData(DeviceId, DstPtr, TgtPtr, Size, copyLock) != OFFLOAD_SUCCESS)
      return OFFLOAD_FAIL;
    if (DstPtr != HstPtr)
        std::copy_n(static_cast<char *>(DstPtr), Size,
                    static_cast<char *>(HstPtr));
    DP("Copied %" PRId64 " bytes (tgt:" DPxMOD ") -> (hst:" DPxMOD ")\n", Size,
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

EXTERN int32_t __tgt_rtl_is_data_exchangable(int32_t SrcId, int32_t DstId) {
  ze_bool_t ret = false;
  ze_result_t rc;

  CALL_ZE(rc, zeDeviceCanAccessPeer, DeviceInfo->Devices[DstId],
          DeviceInfo->Devices[SrcId], &ret);
  if (rc == ZE_RESULT_SUCCESS && ret)
    return 1;

  return 0;
}

EXTERN int32_t __tgt_rtl_data_exchange(
    int32_t SrcId, void *SrcPtr, int32_t DstId, void *DstPtr, int64_t Size) {
  auto cmdList = DeviceInfo->getCopyCmdList(DstId);
  auto cmdQueue = DeviceInfo->getCopyCmdQueue(DstId);

  CALL_ZE_RET_FAIL(zeCommandListAppendMemoryCopy, cmdList, DstPtr, SrcPtr, Size,
                   nullptr, 0, nullptr);
  CALL_ZE_RET_FAIL(zeCommandListClose, cmdList);
  CALL_ZE_RET_FAIL(zeCommandQueueExecuteCommandLists, cmdQueue, 1, &cmdList,
                   nullptr);
  CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, cmdQueue, UINT64_MAX);
  CALL_ZE_RET_FAIL(zeCommandListReset, cmdList);

  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_data_delete(int32_t DeviceId, void *TgtPtr) {
  DeviceId = DeviceInfo->getInternalDeviceId(DeviceId);

  auto &Batch = getTLS()->getCommandBatch();
  if (Batch.isActive())
    return Batch.enqueueMemFree(DeviceId, TgtPtr);

  return DeviceInfo->dataDelete(DeviceId, TgtPtr);
}

static void decideLoopKernelGroupArguments(
    int32_t DeviceId, uint32_t ThreadLimit, TgtNDRangeDescTy *LoopLevels,
    ze_kernel_handle_t Kernel, uint32_t *GroupSizes,
    ze_group_count_t &GroupCounts) {

  auto &computeProperties = DeviceInfo->ComputeProperties[DeviceId];
  uint32_t maxGroupSize = computeProperties.maxTotalGroupSize;
  auto &KernelProperty = DeviceInfo->KernelProperties[DeviceId][Kernel];
  uint32_t kernelWidth = KernelProperty.Width;
  DP("Assumed kernel SIMD width is %" PRIu32 "\n", KernelProperty.SIMDWidth);
  DP("Preferred group size is multiple of %" PRIu32 "\n", kernelWidth);

  uint32_t kernelMaxThreadGroupSize = KernelProperty.MaxThreadGroupSize;
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
      DP("thread_limit(%" PRIu32 ") exceeds current maximum %" PRIu32 "\n",
         ThreadLimit, maxGroupSize);
    }
  }

  if (DeviceInfo->ThreadLimit > 0) {
    maxGroupSizeForced = true;

    if (DeviceInfo->ThreadLimit <= maxGroupSize) {
      maxGroupSize = DeviceInfo->ThreadLimit;
      DP("Max group size is set to %" PRIu32 " (OMP_THREAD_LIMIT)\n",
         maxGroupSize);
    } else {
      DP("OMP_THREAD_LIMIT(%" PRIu32 ") exceeds current maximum %" PRIu32 "\n",
         DeviceInfo->ThreadLimit, maxGroupSize);
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
    assert(level[i].Stride > 0 && "Invalid loop stride for ND partitioning");
    DP("Level %" PRIu32 ": Lb = %" PRId64 ", Ub = %" PRId64 ", Stride = %"
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

  // Dump input data for the occupancy calculation to ease triaging.
  DPI("numEUsPerSubslice: %" PRIu32 "\n", numEUsPerSubslice);
  DPI("numSubslices: %" PRIu32 "\n", numSubslices);
  DPI("numThreadsPerEU: %" PRIu32 "\n", numThreadsPerEU);
  DPI("totalEUs: %" PRIu32 "\n", numEUsPerSubslice * numSubslices);
  auto &KernelProperty = DeviceInfo->KernelProperties[DeviceId][Kernel];
  uint32_t kernelWidth = KernelProperty.Width;
  DP("Assumed kernel SIMD width is %" PRIu32 "\n", KernelProperty.SIMDWidth);
  DP("Preferred group size is multiple of %" PRIu32 "\n", kernelWidth);

  uint32_t kernelMaxThreadGroupSize = KernelProperty.MaxThreadGroupSize;
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
      DP("thread_limit(%" PRIu32 ") exceeds current maximum %" PRIu32 "\n",
         ThreadLimit, maxGroupSize);
    }
  }

  if (DeviceInfo->ThreadLimit > 0) {
    maxGroupSizeForced = true;

    if (DeviceInfo->ThreadLimit <= maxGroupSize) {
      maxGroupSize = DeviceInfo->ThreadLimit;
      DP("Max group size is set to %" PRIu32 " (OMP_THREAD_LIMIT)\n",
         maxGroupSize);
    } else {
      DP("OMP_THREAD_LIMIT(%" PRIu32 ") exceeds current maximum %" PRIu32 "\n",
         DeviceInfo->ThreadLimit, maxGroupSize);
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

static void forceGroupSizes(
    uint32_t *GroupSizes, ze_group_count_t &GroupCounts) {
#if INTEL_INTERNAL_BUILD
  // Use forced group sizes. This is only for internal experiments, and we
  // don't want to plug these numbers into the decision logic.
  auto userLWS = DeviceInfo->ForcedLocalSizes;
  auto userGWS = DeviceInfo->ForcedGlobalSizes;
  if (userLWS[0] > 0) {
    std::copy(userLWS, userLWS + 3, GroupSizes);
    DP("Forced LWS = {%" PRIu32 ", %" PRIu32 ", %" PRIu32 "}\n", userLWS[0],
       userLWS[1], userLWS[2]);
  }
  if (userGWS[0] > 0) {
    GroupCounts.groupCountX = (userGWS[0] + GroupSizes[0] - 1) / GroupSizes[0];
    GroupCounts.groupCountY = (userGWS[1] + GroupSizes[1] - 1) / GroupSizes[1];
    GroupCounts.groupCountZ = (userGWS[2] + GroupSizes[2] - 1) / GroupSizes[2];
    DP("Forced GWS = {%" PRIu32 ", %" PRIu32 ", %" PRIu32 "}\n", userGWS[0],
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
  int32_t subIdBase = 0;
  // Internal ID of the first subdevice
  if (subDeviceIds[0].size() > 0)
    subIdBase = subDeviceIds[0][0];
  else
    subIdBase = subDeviceIds[1][0];

  std::vector<ze_event_handle_t> usedEvents;
  std::vector<ze_command_list_handle_t> usedCmdLists;

  ze_kernel_handle_t rootKernel = *(ze_kernel_handle_t *)TgtEntryPtr;
  uint32_t KernelId = 0;
  auto &RootEntries = DeviceInfo->FuncGblEntries[rootId];

  // Find kernel ID
  uint32_t EntryId = 0;
  for (auto &E : RootEntries) {
    KernelId = 0;
    for (; KernelId < E.Entries.size(); KernelId++)
      if (E.Entries[KernelId].addr == TgtEntryPtr)
        break;
    if (KernelId != E.Entries.size())
      break;
    EntryId++;
  }
  if (EntryId == RootEntries.size() &&
      KernelId == RootEntries.back().Entries.size()) {
    DP("Could not find a kernel for entry " DPxMOD " in the table\n",
       DPxPTR(TgtEntryPtr));
    return OFFLOAD_FAIL;
  }

  ze_event_handle_t profileEvent = nullptr;
  std::string tmName("Kernel ");
  auto EntryItr = RootEntries.begin();
  std::advance(EntryItr, EntryId);
  tmName = tmName + EntryItr->Entries[KernelId].name;
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
    auto SubItr = DeviceInfo->FuncGblEntries[subId].begin();
    std::advance(SubItr, EntryId);
    auto kernel = SubItr->Kernels[KernelId];
#endif // !SUBDEVICE_USE_ROOT_KERNELS

    auto *KernelInfo = DeviceInfo->getKernelInfo(subId, kernel);
    for (int32_t argId = 0; argId < NumArgs; argId++) {
      if (KernelInfo && KernelInfo->isArgLiteral(argId)) {
        uint32_t Size = KernelInfo->getArgSize(argId);
        CALL_ZE_RET_FAIL(zeKernelSetArgumentValue, kernel, argId, Size,
                         TgtArgs[argId]);

        DP("Kernel ByVal argument %" PRId32
           " was set successfully for device %s.\n",
           argId, DeviceInfo->DeviceIdStr[subId].c_str());
      } else if (TgtOffsets[argId] == (std::numeric_limits<ptrdiff_t>::max)()) {
        intptr_t arg = reinterpret_cast<intptr_t>(TgtArgs[argId]);
        CALL_ZE_RET_FAIL(zeKernelSetArgumentValue, kernel, argId, sizeof(arg),
                         &arg);
        DP("Kernel Scalar argument %" PRId32 " (value: " DPxMOD
           ") was set successfully for device %s.\n",
           argId, DPxPTR(arg), DeviceInfo->DeviceIdStr[subId].c_str());
      } else {
        void *arg = (void *)((intptr_t)TgtArgs[argId] + TgtOffsets[argId]);
        CALL_ZE_RET_FAIL(zeKernelSetArgumentValue, kernel, argId, sizeof(arg),
                         arg == nullptr ? nullptr : &arg);
        DP("Kernel Pointer argument %" PRId32 " (value: " DPxMOD
           ") was set successfully for device %s.\n",
           argId, DPxPTR(arg), DeviceInfo->DeviceIdStr[subId].c_str());
      }
    }

    auto flags = DeviceInfo->getKernelIndirectAccessFlags(rootKernel, rootId);
    CALL_ZE_RET_FAIL(zeKernelSetIndirectAccess, kernel, flags);
    DP("Setting indirect access flags " DPxMOD "\n", DPxPTR(flags));

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

    DP("Group sizes = {%" PRIu32 ", %" PRIu32 ", %" PRIu32 "}\n",
       groupSizes[0], groupSizes[1], groupSizes[2]);
    DP("Group counts = {%" PRIu32 ", %" PRIu32 ", %" PRIu32 "}\n",
       groupCounts.groupCountX, groupCounts.groupCountY,
       groupCounts.groupCountZ);

    CALL_ZE_RET_FAIL(zeKernelSetGroupSize, kernel, groupSizes[0], groupSizes[1],
                     groupSizes[2]);

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

    DP("Submitted kernel " DPxMOD " to subdevice %s\n", DPxPTR(kernel),
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

  DP("Executed kernel entry " DPxMOD " on subdevices\n", DPxPTR(TgtEntryPtr));
  return OFFLOAD_SUCCESS;
}

static int32_t runTargetTeamRegion(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit, void *LoopDesc,
    void *AsyncEvent) {
  assert(TgtEntryPtr && "Invalid kernel");
  assert((NumTeams >= 0 && ThreadLimit >= 0) && "Invalid kernel work size");
  DP("Executing a kernel " DPxMOD "...\n", DPxPTR(TgtEntryPtr));

  auto SubDeviceCode = DeviceInfo->getSubDeviceCode();
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
  ScopedTimerTy tmKernel(DeviceId, "Kernel ",
                         DeviceInfo->KernelProperties[DeviceId][kernel].Name);

  // Set arguments
  auto *KernelInfo = DeviceInfo->getKernelInfo(DeviceId, kernel);
  for (int32_t i = 0; i < NumArgs; i++) {
    ptrdiff_t offset = TgtOffsets[i];
    if (KernelInfo && KernelInfo->isArgLiteral(i)) {
      uint32_t Size = KernelInfo->getArgSize(i);
      CALL_ZE_RET_FAIL(zeKernelSetArgumentValue, kernel, i, Size, TgtArgs[i]);

      DP("Kernel ByVal argument %" PRId32 " was set successfully.\n", i);
    } else if (offset == (std::numeric_limits<ptrdiff_t>::max)()) {
      // Offset equal to MAX(ptrdiff_t) means that the argument
      // must be passed as literal, and the offset should be ignored.
      intptr_t arg = reinterpret_cast<intptr_t>(TgtArgs[i]);
      CALL_ZE_RET_FAIL(zeKernelSetArgumentValue, kernel, i, sizeof(arg), &arg);
      DP("Kernel Scalar argument %" PRId32 " (value: " DPxMOD
         ") was set successfully.\n", i, DPxPTR(arg));
    } else {
      void *arg = (void *)((intptr_t)TgtArgs[i] + offset);
      CALL_ZE_RET_FAIL(zeKernelSetArgumentValue, kernel, i, sizeof(arg),
                       arg == nullptr ? nullptr : &arg);
      DP("Kernel Pointer argument %" PRId32 " (value: " DPxMOD
         ") was set successfully.\n", i, DPxPTR(arg));
    }
  }

  auto flags = DeviceInfo->getKernelIndirectAccessFlags(kernel, DeviceId);
  CALL_ZE_RET_FAIL(zeKernelSetIndirectAccess, kernel, flags);
  DP("Setting indirect access flags " DPxMOD "\n", DPxPTR(flags));

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

  DP("Group sizes = {%" PRIu32 ", %" PRIu32 ", %" PRIu32 "}\n",
     groupSizes[0], groupSizes[1], groupSizes[2]);
  DP("Group counts = {%" PRIu32 ", %" PRIu32 ", %" PRIu32 "}\n",
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

  auto &Batch = getTLS()->getCommandBatch();
  if (Batch.isActive() && DeviceInfo->isDiscreteDevice(DeviceId))
    return Batch.enqueueLaunchKernel(DeviceId, kernel, &groupCounts);

  if (AsyncEvent) {
    cmdList = createCmdList(context, DeviceInfo->Devices[DeviceId],
                            DeviceInfo->CmdQueueGroupOrdinals[DeviceId],
                            DeviceInfo->DeviceIdStr[DeviceId]);
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
    ze_event_handle_t event = nullptr;
    if (DeviceInfo->Flags.EnableProfile)
      event = DeviceInfo->ProfileEvents.getEvent();
    CALL_ZE_RET_FAIL(zeCommandListAppendLaunchKernel, cmdList, kernel,
                     &groupCounts, event, 0, nullptr);
    CALL_ZE_RET_FAIL(zeCommandListClose, cmdList);
    CALL_ZE_RET_FAIL(zeCommandQueueExecuteCommandLists, cmdQueue, 1, &cmdList,
                     nullptr);
    kernelLock.unlock();
    CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, cmdQueue, UINT64_MAX);
    tmKernel.updateDeviceTime(event);
    // Make sure the command list is ready to accept next command
    CALL_ZE_RET_FAIL(zeCommandListReset, cmdList);
  }

  DP("Executed a kernel " DPxMOD "\n", DPxPTR(TgtEntryPtr));
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
  ze_kernel_handle_t kernel = *static_cast<ze_kernel_handle_t *>(TgtEntryPtr);
  std::unique_lock<std::mutex> dataLock(DeviceInfo->Mutexes[DeviceId]);

  auto &flags =
      DeviceInfo->KernelProperties[DeviceId][kernel].IndirectAccessFlags;

  ScopedTimerTy tmManifestData(DeviceId, "DataManifest");

  uint32_t flagsAll = ZE_KERNEL_INDIRECT_ACCESS_FLAG_HOST |
                      ZE_KERNEL_INDIRECT_ACCESS_FLAG_SHARED |
                      ZE_KERNEL_INDIRECT_ACCESS_FLAG_DEVICE;

  for (size_t i = 0; i < NumPtrs && flags != flagsAll; i++) {
    auto memType = DeviceInfo->getMemAllocType(TgtPtrs[i]);
    if (memType == ZE_MEMORY_TYPE_HOST)
      flags |= ZE_KERNEL_INDIRECT_ACCESS_FLAG_HOST;
    else if (memType == ZE_MEMORY_TYPE_SHARED)
      flags |= ZE_KERNEL_INDIRECT_ACCESS_FLAG_SHARED;
    else if (memType == ZE_MEMORY_TYPE_DEVICE)
      flags |= ZE_KERNEL_INDIRECT_ACCESS_FLAG_DEVICE;
  }
  return OFFLOAD_SUCCESS;
}

EXTERN void __tgt_rtl_create_offload_queue(int32_t DeviceId, void *Interop) {
  if (Interop == nullptr) {
    DP("Invalid interop object in %s\n", __func__);
    return;
  }

  __tgt_interop_obj *obj = static_cast<__tgt_interop_obj *>(Interop);

  int32_t deviceId = DeviceId;
  int64_t deviceCode = obj->device_code;
  if (deviceCode < 0) {
    if (isValidSubDevice(deviceCode)) {
      uint32_t subId = SUBDEVICE_GET_START(deviceCode);
      uint32_t subLevel = SUBDEVICE_GET_LEVEL(deviceCode);
      deviceId = DeviceInfo->SubDeviceIds[DeviceId][subLevel][subId];
    } else {
      DP("Ignoring invalid sub-device encoding " DPxMOD "\n",
         DPxPTR(deviceCode));
    }
  }

  // Create and return a new command queue for interop
  // TODO: check with MKL team and decide what to do with IsAsync
  auto cmdQueue = DeviceInfo->createCommandQueue(deviceId);
  obj->queue = cmdQueue;
  DP("%s returns a new asynchronous command queue " DPxMOD "\n", __func__,
     DPxPTR(obj->queue));
}

EXTERN int32_t __tgt_rtl_release_offload_queue(int32_t DeviceId, void *Queue) {
  CALL_ZE_RET_FAIL(zeCommandQueueDestroy, (ze_command_queue_handle_t)Queue);
  return OFFLOAD_SUCCESS;
}

EXTERN void *__tgt_rtl_get_platform_handle(int32_t DeviceId) {
  auto driver = DeviceInfo->Driver;
  return (void *)driver;
}

EXTERN void __tgt_rtl_set_device_handle(int32_t DeviceId, void *Interop) {
  if (Interop == nullptr) {
    DP("Invalid interop object in %s\n", __func__);
    return;
  }

  __tgt_interop_obj *obj = static_cast<__tgt_interop_obj *>(Interop);

  obj->device_handle = DeviceInfo->Devices[DeviceId];

  int64_t deviceCode = obj->device_code;
  if (deviceCode < 0) {
    if (isValidSubDevice(deviceCode)) {
      auto &subDeviceIds = DeviceInfo->SubDeviceIds[DeviceId];
      uint32_t subId = SUBDEVICE_GET_START(deviceCode);
      uint32_t subLevel = SUBDEVICE_GET_LEVEL(deviceCode);
      obj->device_handle = DeviceInfo->Devices[subDeviceIds[subLevel][subId]];
    } else {
      DP("Ignoring invalid sub-device encoding " DPxMOD "\n",
         DPxPTR(deviceCode));
    }
  }

  DP("Returns device handle " DPxMOD "\n", DPxPTR(obj->device_handle));
}

EXTERN void *__tgt_rtl_get_context_handle(int32_t DeviceId) {
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
    DP("Warning: Invalid subdevice encoding " DPxMOD " is ignored\n",
       DPxPTR(DeviceIds));
  else
    DeviceInfo->setSubDeviceCode(DeviceIds);
  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_pop_subdevice() {
  DeviceInfo->setSubDeviceCode(0);
  return OFFLOAD_SUCCESS;
}

EXTERN void __tgt_rtl_add_build_options(
    const char *CompileOptions, const char *LinkOptions) {
  auto &options = DeviceInfo->UserCompilationOptions;
  if (!options.empty()) {
    DP("Respecting LIBOMPTARGET_LEVEL0_COMPILATION_OPTIONS=%s\n",
       options.c_str());
    return;
  }
  if (CompileOptions)
    options = std::string(CompileOptions) + " ";
  if (LinkOptions)
    options += std::string(LinkOptions) + " ";
}

EXTERN int32_t __tgt_rtl_is_supported_device(int32_t DeviceId,
                                             void *DeviceType) {
  if (!DeviceType)
    return true;

  uint64_t deviceArch = DeviceInfo->DeviceArchs[DeviceId];
  int32_t ret = (uint64_t)(deviceArch & (uint64_t)DeviceType) == deviceArch;
  DP("Device %" PRIu32 " does%s match the requested device types " DPxMOD "\n",
     DeviceId, ret ? "" : " not", DPxPTR(DeviceType));
  return ret;
}

EXTERN void __tgt_rtl_deinit(void) {
  // No-op on Linux
#ifdef _WIN32
  if (DeviceInfo) {
    closeRTL();
    deinit();
  }
#endif // _WIN32
}

EXTERN __tgt_interop *__tgt_rtl_create_interop(
    int32_t DeviceId, int32_t InteropContext, int32_t NumPrefers,
    intptr_t *PreferIDs) {
  // Preference-list is ignored since we cannot have multiple runtimes.
  auto ret = new __tgt_interop();
  ret->FrId = L0Interop::FrId;
  ret->FrName = L0Interop::FrName;
  ret->Vendor = L0Interop::Vendor;
  ret->VendorName = L0Interop::VendorName;
  ret->DeviceNum = DeviceId;

  if (InteropContext == OMP_INTEROP_CONTEXT_TARGET ||
      InteropContext == OMP_INTEROP_CONTEXT_TARGETSYNC) {
    ret->Platform = DeviceInfo->Driver;
    ret->Device = DeviceInfo->Devices[DeviceId];
    ret->DeviceContext = DeviceInfo->Context;
  }
  if (InteropContext == OMP_INTEROP_CONTEXT_TARGETSYNC) {
    ret->TargetSync = DeviceInfo->createCommandQueue(DeviceId);
  }

  // TODO: define implementation-defined interop properties
  ret->RTLProperty = new L0Interop::Property();

  return ret;
}

EXTERN int32_t __tgt_rtl_release_interop(
    int32_t DeviceId, __tgt_interop *Interop) {
  if (!Interop || Interop->DeviceNum != (intptr_t)DeviceId ||
      Interop->FrId != L0Interop::FrId) {
    DP("Invalid/inconsistent OpenMP interop " DPxMOD "\n", DPxPTR(Interop));
    return OFFLOAD_FAIL;
  }

  if (Interop->TargetSync) {
    auto cmdQueue = static_cast<ze_command_queue_handle_t>(Interop->TargetSync);
    CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, cmdQueue, UINT64_MAX);
    CALL_ZE_RET_FAIL(zeCommandQueueDestroy, cmdQueue);
  }

  auto L0 = static_cast<L0Interop::Property *>(Interop->RTLProperty);
  delete L0;
  delete Interop;

  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_use_interop(int32_t DeviceId, __tgt_interop *Interop) {
  if (!Interop || Interop->DeviceNum != (intptr_t)DeviceId ||
      Interop->FrId != L0Interop::FrId) {
    DP("Invalid/inconsistent OpenMP interop " DPxMOD "\n", DPxPTR(Interop));
    return OFFLOAD_FAIL;
  }

  if (Interop->TargetSync) {
    auto cmdQueue = static_cast<ze_command_queue_handle_t>(Interop->TargetSync);
    CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, cmdQueue, UINT64_MAX);
  }

  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_get_num_interop_properties(int32_t DeviceId) {
  // omp_ipr_first == -9
  return (int32_t)L0Interop::IprNames.size() + omp_ipr_first;
}

/// Return the value of the requested property
EXTERN int32_t __tgt_rtl_get_interop_property_value(
    int32_t DeviceId, __tgt_interop *Interop, int32_t Ipr, int32_t ValueType,
    size_t Size, void *Value) {

  int32_t retCode = omp_irc_success;
  auto &deviceProperties = DeviceInfo->DeviceProperties[DeviceId];

  switch (Ipr) {
  case L0Interop::device_num_eus:
    if (ValueType == OMP_IPR_VALUE_INT)
      *static_cast<intptr_t *>(Value) = deviceProperties.numEUsPerSubslice *
          deviceProperties.numSubslicesPerSlice *
          deviceProperties.numSlices;
    else
      retCode = omp_irc_type_int;
    break;
  case L0Interop::device_num_threads_per_eu:
    if (ValueType == OMP_IPR_VALUE_INT)
      *static_cast<intptr_t *>(Value) = deviceProperties.numThreadsPerEU;
    else
      retCode = omp_irc_type_int;
    break;
  case L0Interop::device_eu_simd_width:
    if (ValueType == OMP_IPR_VALUE_INT)
      *static_cast<intptr_t *>(Value) = deviceProperties.physicalEUSimdWidth;
    else
      retCode = omp_irc_type_int;
    break;
  case L0Interop::device_num_eus_per_subslice:
    if (ValueType == OMP_IPR_VALUE_INT)
      *static_cast<intptr_t *>(Value) = deviceProperties.numEUsPerSubslice;
    else
      retCode = omp_irc_type_int;
    break;
  case L0Interop::device_num_subslices_per_slice:
    if (ValueType == OMP_IPR_VALUE_INT)
      *static_cast<intptr_t *>(Value) = deviceProperties.numSubslicesPerSlice;
    else
      retCode = omp_irc_type_int;
    break;
  case L0Interop::device_num_slices:
    if (ValueType == OMP_IPR_VALUE_INT)
      *static_cast<intptr_t *>(Value) = deviceProperties.numSlices;
    else
      retCode = omp_irc_type_int;
    break;
  default:
    retCode = omp_irc_out_of_range;
    break;
  }

  return retCode;
}

EXTERN const char *__tgt_rtl_get_interop_property_info(
    int32_t DeviceId, int32_t Ipr, int32_t InfoType) {
  int32_t offset = Ipr - omp_ipr_first;
  if (offset < 0 || (size_t)offset >= L0Interop::IprNames.size())
    return nullptr;

  if (InfoType == OMP_IPR_INFO_NAME)
    return L0Interop::IprNames[offset];
  else if (InfoType == OMP_IPR_INFO_TYPE_DESC)
    return L0Interop::IprTypeDescs[offset];

  return nullptr;
}

EXTERN const char *__tgt_rtl_get_interop_rc_desc(int32_t DeviceId,
                                                 int32_t RetCode) {
  // TODO: decide implementation-defined return code.
  return nullptr;
}

EXTERN int32_t __tgt_rtl_get_num_sub_devices(int32_t DeviceId, int32_t Level) {
  int32_t ret = 0;
  if (Level >= 0 && DeviceInfo->SubDeviceIds[DeviceId].size() > (size_t)Level)
    ret = DeviceInfo->SubDeviceIds[DeviceId][Level].size();

  DP("%s returns %" PRId32 " sub-devices at level %" PRId32 "\n", __func__,
     ret, Level);
  return ret;
}

EXTERN int32_t __tgt_rtl_is_accessible_addr_range(
    int32_t DeviceId, const void *Ptr, size_t Size) {
  if (!Ptr || Size == 0)
    return 0;

  auto MemType = DeviceInfo->getMemAllocType(Ptr);
  auto Device = DeviceInfo->Devices[DeviceId];
  if (MemType == ZE_MEMORY_TYPE_HOST)
    Device = nullptr;
  else if (MemType != ZE_MEMORY_TYPE_SHARED)
    return 0;

  if (DeviceInfo->MemHostAccessible.at(Device)->contains(Ptr, Size))
    return 1;
  else
    return 0;
}

EXTERN int32_t __tgt_rtl_notify_indirect_access(
    int32_t DeviceId, const void *Ptr, size_t Offset) {
  using FnTy = void(*)(void *, uint32_t, size_t *);
  auto Fn = reinterpret_cast<FnTy>(DeviceInfo->GitsIndirectAllocationOffsets);
  void *PtrBase = (void *)((uintptr_t)Ptr - Offset);
  // This DP is only for testability
  DP("Notifying indirect access: " DPxMOD " + %zu\n", DPxPTR(PtrBase), Offset);
  if (Fn) {
    Fn(PtrBase, 1, &Offset);
  }
  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_is_private_arg_on_host(
    int32_t DeviceId, const void *TgtEntryPtr, uint32_t Idx) {
  const ze_kernel_handle_t *Kernel =
      reinterpret_cast<const ze_kernel_handle_t *>(TgtEntryPtr);
  if (!*Kernel) {
    REPORT("Querying information about a deleted kernel.\n");
    return 0;
  }
  auto *KernelInfo = DeviceInfo->getKernelInfo(DeviceId, *Kernel);
  if (!KernelInfo)
    return 0;

  if (KernelInfo->isArgLiteral(Idx))
    return 1;

  return 0;
}

EXTERN int32_t __tgt_rtl_command_batch_begin(
    int32_t DeviceId, int32_t BatchLevel) {
  // Do not try command batching in these cases
  // -- Integrated devices
  // -- Kernel batching on subdevices
  // -- Allowed batch level is lower than BatchLevel
  if (!DeviceInfo->isDiscreteDevice(DeviceId) ||
      (BatchLevel > 1 && DeviceInfo->getSubDeviceCode() != 0) ||
      DeviceInfo->CommandBatchLevel < BatchLevel)
    return OFFLOAD_SUCCESS;

  DeviceId = DeviceInfo->getInternalDeviceId(DeviceId);

  return getTLS()->getCommandBatch().begin(DeviceId);
}

EXTERN int32_t __tgt_rtl_command_batch_end(
    int32_t DeviceId, int32_t BatchLevel) {
  // Do not try command batching in these cases
  // -- Integrated devices
  // -- Kernel batching on subdevices
  // -- Allowed batch level is lower than BatchLevel
  if (!DeviceInfo->isDiscreteDevice(DeviceId) ||
      (BatchLevel > 1 && DeviceInfo->getSubDeviceCode() != 0) ||
      DeviceInfo->CommandBatchLevel < BatchLevel)
    return OFFLOAD_SUCCESS;

  return getTLS()->getCommandBatch().end();
}

void *RTLDeviceInfoTy::getOffloadVarDeviceAddr(
    int32_t DeviceId, const char *Name, size_t Size) {
  DP("Looking up OpenMP global variable '%s' of size %zu bytes on device %d.\n",
     Name, Size, DeviceId);

  std::vector<DeviceOffloadEntryTy> &OffloadTable =
      OffloadTables[DeviceId].back();
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

    DP("Warning: global variable '%s' was not found in the offload table.\n",
       Name);
  } else
    DP("Warning: offload table is not loaded for device %d.\n", DeviceId);

  // Fallback to the lookup by name.
  return getVarDeviceAddr(DeviceId, Name, Size);
}

void *RTLDeviceInfoTy::getVarDeviceAddr(
    int32_t DeviceId, const char *Name, size_t *SizePtr) {
  void *TgtAddr = nullptr;
  size_t TgtSize = 0;
  size_t Size = *SizePtr;
  bool SizeIsKnown = (Size != 0);
  if (SizeIsKnown)
    DP("Looking up device global variable '%s' of size %zu bytes "
       "on device %d.\n", Name, Size, DeviceId);
  else
    DP("Looking up device global variable '%s' of unknown size "
       "on device %d.\n", Name, DeviceId);
  CALL_ZE_RET_NULL(zeModuleGetGlobalPointer,
                   FuncGblEntries[DeviceId].back().Modules[0], Name, &TgtSize,
                   &TgtAddr);

  if (Size != TgtSize && SizeIsKnown) {
    DP("Warning: requested size %zu does not match %zu\n", Size, TgtSize);
    return nullptr;
  }

  if (TgtSize == 0) {
    DP("Warning: global variable lookup failed.\n");
    return nullptr;
  }

  DP("Global variable lookup succeeded (size: %zu bytes).\n", TgtSize);
  *SizePtr = TgtSize;
  return TgtAddr;
}

void *RTLDeviceInfoTy::getVarDeviceAddr(
    int32_t DeviceId, const char *Name, size_t Size) {
  return getVarDeviceAddr(DeviceId, Name, &Size);
}

bool RTLDeviceInfoTy::loadOffloadTable(int32_t DeviceId, size_t NumEntries) {
  const char *OffloadTableSizeVarName = "__omp_offloading_entries_table_size";
  void *OffloadTableSizeVarAddr =
      getVarDeviceAddr(DeviceId, OffloadTableSizeVarName, sizeof(int64_t));

  if (!OffloadTableSizeVarAddr) {
    DP("Warning: cannot get device value for global variable '%s'.\n",
       OffloadTableSizeVarName);
    return false;
  }

  int64_t TableSizeVal = 0;
  auto rc = enqueueMemCopy(DeviceId, &TableSizeVal, OffloadTableSizeVarAddr,
                           sizeof(int64_t));
  if (rc != OFFLOAD_SUCCESS)
    return false;
  size_t TableSize = (size_t)TableSizeVal;

  if ((TableSize % sizeof(DeviceOffloadEntryTy)) != 0) {
    DP("Warning: offload table size (%zu) is not a multiple of %zu.\n",
       TableSize, sizeof(DeviceOffloadEntryTy));
    return false;
  }

  size_t DeviceNumEntries = TableSize / sizeof(DeviceOffloadEntryTy);

  if (NumEntries != DeviceNumEntries) {
    DP("Warning: number of entries in host and device "
       "offload tables mismatch (%zu != %zu).\n", NumEntries, DeviceNumEntries);
  }

  const char *OffloadTableVarName = "__omp_offloading_entries_table";
  void *OffloadTableVarAddr =
      getVarDeviceAddr(DeviceId, OffloadTableVarName, TableSize);
  if (!OffloadTableVarAddr) {
    DP("Warning: cannot get device value for global variable '%s'.\n",
       OffloadTableVarName);
    return false;
  }

  OffloadTables[DeviceId].emplace_back();
  auto &DeviceTable = OffloadTables[DeviceId].back();
  DeviceTable.resize(DeviceNumEntries);
  rc = enqueueMemCopy(DeviceId, DeviceTable.data(),
                      OffloadTableVarAddr, TableSize);
  if (rc != OFFLOAD_SUCCESS)
    return false;

  size_t I = 0;
  const char *PreviousName = "";
  bool PreviousIsVar = false;

  for (; I < DeviceNumEntries; ++I) {
    DeviceOffloadEntryTy &Entry = DeviceTable[I];
    size_t NameSize = Entry.NameSize;
    void *NameTgtAddr = Entry.Base.name;
    Entry.Base.name = nullptr;

    if (NameSize == 0) {
      DP("Warning: offload entry (%zu) with 0 size.\n", I);
      break;
    }
    if (NameTgtAddr == nullptr) {
      DP("Warning: offload entry (%zu) with invalid name.\n", I);
      break;
    }

    Entry.Base.name = new char[NameSize];
    rc = enqueueMemCopy(DeviceId, Entry.Base.name, NameTgtAddr, NameSize);
    if (rc != OFFLOAD_SUCCESS)
      break;

    if (strnlen(Entry.Base.name, NameSize) != NameSize - 1) {
      DP("Warning: offload entry's name has wrong size.\n");
      break;
    }

    int Cmp = strncmp(PreviousName, Entry.Base.name, NameSize);
    if (Cmp > 0) {
      DP("Warning: offload table is not sorted.\n"
         "Warning: previous name is '%s'.\n"
         "Warning:  current name is '%s'.\n",
         PreviousName, Entry.Base.name);
      break;
    } else if (Cmp == 0 && (PreviousIsVar || Entry.Base.addr)) {
      // The names are equal. This should never happen for
      // offload variables, but we allow this for offload functions.
      DP("Warning: duplicate names (%s) in offload table.\n", PreviousName);
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

    DeviceTable.clear();
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
  for (auto &T : OffloadTables[DeviceId])
    for (auto &E : T)
      delete[] E.Base.name;

  OffloadTables[DeviceId].clear();
}
#endif // INTEL_CUSTOMIZATION

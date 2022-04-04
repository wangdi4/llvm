// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
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
#include <cctype>
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
/// Default staging buffer size for host to device copy (16KB)
#define LEVEL0_STAGING_BUFFER_SIZE (1 << 14)
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

#ifdef _WIN32
// TODO: enable again if XDEPS-3027 is resolved
#define LEVEL0_KERNEL_BEGIN(ID)
#define LEVEL0_KERNEL_END(ID)
#else // _WIN32
#define LEVEL0_KERNEL_BEGIN(ID)                                                \
  do {                                                                         \
    if (DeviceInfo->Option.KernelDynamicMemorySize > 0) {                      \
      DeviceInfo->Mutexes[ID].lock();                                          \
      DeviceInfo->NumActiveKernels[ID]++;                                      \
      DeviceInfo->Mutexes[ID].unlock();                                        \
    }                                                                          \
  } while (0)

#define LEVEL0_KERNEL_END(ID)                                                  \
  do {                                                                         \
    if (DeviceInfo->Option.KernelDynamicMemorySize > 0) {                      \
      DeviceInfo->Mutexes[ID].lock();                                          \
      DeviceInfo->NumActiveKernels[ID]--;                                      \
      if (DeviceInfo->NumActiveKernels[ID] == 0)                               \
        DeviceInfo->resetProgramData(ID);                                      \
      DeviceInfo->Mutexes[ID].unlock();                                        \
    }                                                                          \
  } while (0)
#endif // _WIN32

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
      0x8A00, // ICX
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
      // Putting PVC here for now.
      // We may decide to add another arch type if needed in the future.
      0x0b00, // PVC
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
    "device_num_slices",
    "device_local_mem_size",
    "device_global_mem_size",
    "device_global_mem_cache_size",
    "device_max_clock_frequency"
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
    "intptr_t, number of slices",
    "intptr_t, local memory size in bytes",
    "intptr_t, global memory size in bytes",
    "intptr_t, global memory cache size in bytes",
    "intptr_t, max clock frequency in MHz"
  };

  // Level Zero property ID
  enum IprIDTy : int32_t {
    device_num_eus = 0,
    device_num_threads_per_eu,
    device_eu_simd_width,
    device_num_eus_per_subslice,
    device_num_subslices_per_slice,
    device_num_slices,
    device_local_mem_size,
    device_global_mem_size,
    device_global_mem_cache_size,
    device_max_clock_frequency
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
    (void)Rc; // GCC build compiler thinks Rc is unused for some reason.
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
                              ze_group_count_t *GroupCounts,
                              std::unique_lock<std::mutex> &KernelLock);

  int32_t enqueueMemFree(int32_t DeviceId, void *Ptr);

  bool isActive() { return State > 0; }
};

struct KernelBatchTy {
  uint32_t MaxCommands = 0;
  uint32_t NumCommands = 0;
  ze_command_list_handle_t CmdList = nullptr;
  ze_command_queue_handle_t CmdQueue = nullptr;
  ze_event_pool_handle_t EventPool = nullptr;
  ze_event_handle_t Event = nullptr;
  bool UseImmCmdList = false;

  ~KernelBatchTy() {
    if (CmdList)
      CALL_ZE_RET_VOID(zeCommandListDestroy, CmdList);
    if (CmdQueue)
      CALL_ZE_RET_VOID(zeCommandQueueDestroy, CmdQueue);
    if (EventPool) {
      CALL_ZE_RET_VOID(zeEventDestroy, Event);
      CALL_ZE_RET_VOID(zeEventPoolDestroy, EventPool);
    }
  }

  int32_t enqueueKernel(const ze_kernel_handle_t Kernel,
                        const ze_group_count_t &GroupCounts) {
    // Already locked
    CALL_ZE_RET_FAIL(zeCommandListAppendLaunchKernel, CmdList, Kernel,
                     &GroupCounts, nullptr, 0, nullptr);
    NumCommands++;

    if (UseImmCmdList && NumCommands >= MaxCommands) {
      CALL_ZE_RET_FAIL(zeCommandListAppendBarrier, CmdList, Event, 0, nullptr);
    }

    return OFFLOAD_SUCCESS;
  }

  int32_t run(std::mutex &DeviceMtx) {
    std::lock_guard<std::mutex> Lock(DeviceMtx);
    if (NumCommands >= MaxCommands) {
      if (UseImmCmdList) {
        // Use barrier + event
        CALL_ZE_RET_FAIL(zeEventHostSynchronize, Event, UINT64_MAX);
        CALL_ZE_RET_FAIL(zeEventHostReset, Event);
      } else {
        CALL_ZE_RET_FAIL(zeCommandListClose, CmdList);
        CALL_ZE_RET_FAIL(zeCommandQueueExecuteCommandLists, CmdQueue, 1,
                         &CmdList, nullptr);
        DP("Submitted %" PRIu32 " kernels to command queue " DPxMOD "\n",
           NumCommands, DPxPTR(CmdQueue));
        CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, CmdQueue, UINT64_MAX);
        CALL_ZE_RET_FAIL(zeCommandListReset, CmdList);
      }
      NumCommands = 0;
    }
    return OFFLOAD_SUCCESS;
  }
};

class RTLProfileTy;

/// All thread-local data used by RTL
class TLSTy {
  /// Command list for each device
  std::map<int32_t, ze_command_list_handle_t> CmdLists;

  /// Main copy command list for each device
  std::map<int32_t, ze_command_list_handle_t> CopyCmdLists;

  /// Link copy command list for each device
  std::map<int32_t, ze_command_list_handle_t> LinkCopyCmdLists;

  /// Command queue for each device
  std::map<int32_t, ze_command_queue_handle_t> CmdQueues;

  /// CCS Command queue for each device
  std::map<int32_t, ze_command_queue_handle_t> CCSCmdQueues;

  /// Main copy command queue for each device
  std::map<int32_t, ze_command_queue_handle_t> CopyCmdQueues;

  /// Link copy command queues for each device
  std::map<int32_t, ze_command_queue_handle_t> LinkCopyCmdQueues;

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

  ze_command_list_handle_t getLinkCopyCmdList(int32_t ID) {
    return (LinkCopyCmdLists.count(ID) > 0) ? LinkCopyCmdLists.at(ID): nullptr;
  }

  ze_command_queue_handle_t getCmdQueue(int32_t ID) {
    return (CmdQueues.count(ID) > 0) ? CmdQueues.at(ID) : nullptr;
  }

  ze_command_queue_handle_t getCCSCmdQueue(int32_t ID) {
    return (CCSCmdQueues.count(ID) > 0) ? CCSCmdQueues.at(ID) : nullptr;
  }

  ze_command_queue_handle_t getCopyCmdQueue(int32_t ID) {
    return (CopyCmdQueues.count(ID) > 0) ? CopyCmdQueues.at(ID) : nullptr;
  }

  ze_command_queue_handle_t getLinkCopyCmdQueue(int32_t ID) {
    return
        (LinkCopyCmdQueues.count(ID) > 0) ? LinkCopyCmdQueues.at(ID) : nullptr;
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

  void setLinkCopyCmdList(int32_t ID, ze_command_list_handle_t CmdList) {
    LinkCopyCmdLists[ID] = CmdList;
  }

  void setCmdQueue(int32_t ID, ze_command_queue_handle_t CmdQueue) {
    CmdQueues[ID] = CmdQueue;
  }

  void setCCSCmdQueue(int32_t ID, ze_command_queue_handle_t CmdQueue) {
    CCSCmdQueues[ID] = CmdQueue;
  }

  void setCopyCmdQueue(int32_t ID, ze_command_queue_handle_t CmdQueue) {
    CopyCmdQueues[ID] = CmdQueue;
  }

  void setLinkCopyCmdQueue(int32_t ID, ze_command_queue_handle_t CmdQueue) {
    LinkCopyCmdQueues[ID] = CmdQueue;
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
                               bool LogMemAlloc = true,
                               size_t Align = LEVEL0_ALIGNMENT);
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

  /// Allocation unit size decided by the GPU RT.
  /// Fall-back size is 64KB in case GPU RT does not provide the information.
  size_t AllocUnit = 1 << 16; // 64KB

  /// Minimum supported memory allocation size from pool
  size_t AllocMin = 1 << 6; // 64B (cache line size by default)

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
  uint64_t Attributes1 = 0;
  uint64_t WGNum = 0;
  uint64_t WINum = 0;

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
  void setAttributes1(uint64_t Val) {
    Attributes1 = Val;
  }
  bool getHasTeamsReduction() const {
    return (Attributes1 & 1);
  }
  void setWGNum(uint64_t Val) {
    WGNum = Val;
  }
  uint64_t getWGNum() const {
    return WGNum;
  }
  void setWINum(uint64_t Val) {
    WINum = Val;
  }
  uint64_t getWINum() const {
    return WINum;
  }
  bool isAtomicFreeReduction() const {
    return getWGNum();
  }
};

/// Program data to be initialized by plugin
struct ProgramDataTy {
  int Initialized = 0;
  int NumDevices = 0;
  int DeviceNum = -1;
  uint32_t TotalEUs = 0;
  uint32_t HWThreadsPerEU = 0;
  uintptr_t DynamicMemoryLB = 0;
  uintptr_t DynamicMemoryUB = 0;
  int DeviceType = 0;
};

/// Level Zero program that can contain multiple modules.
class LevelZeroProgramTy {
  struct DeviceOffloadEntryTy {
    /// Common part with the host offload table.
    __tgt_offload_entry Base;
    /// Length of the Base.name string in bytes including
    /// the null terminator.
    size_t NameSize;
  };

  /// Cached device image
  __tgt_device_image *Image = nullptr;

  /// Cached Level Zero context
  ze_context_handle_t Context = nullptr;

  /// Cached Level Zero device
  ze_device_handle_t Device = nullptr;

  /// Cached OpenMP device ID
  int32_t DeviceId = 0;

  /// Target table
  __tgt_target_table Table;

  /// Target entries
  std::vector<__tgt_offload_entry> Entries;

  /// Internal offload entries
  std::vector<DeviceOffloadEntryTy> OffloadEntries;

  /// Handle multiple modules within a single target image
  std::vector<ze_module_handle_t> Modules;

  /// Kernels created from the target image
  std::vector<ze_kernel_handle_t> Kernels;

  /// Kernel info added by compiler
  std::unordered_map<ze_kernel_handle_t, KernelInfoTy> KernelInfo;

  /// Program data copied to device
  ProgramDataTy PGMData;

  /// Cached address of the program data on device
  void *PGMDataPtr = nullptr;

  /// Module that contains global data including device RTL
  ze_module_handle_t GlobalModule = nullptr;

  /// Requires module link
  bool RequiresModuleLink = false;

  /// Is this module library
  bool IsLibModule = false;

  /// Loads the device version of the offload table for device \p DeviceId.
  /// The table is expected to have \p NumEntries entries.
  /// Returns true, if the load was successful, false - otherwise.
  bool loadOffloadTable(size_t NumEntries);

  /// Build a single module with the given image, build option, and format.
  int32_t addModule(const size_t Size, const uint8_t *Image,
                    const std::string &BuildOption, ze_module_format_t Format);

  /// Looks up an OpenMP declare target global variable with the given
  /// \p Name and \p Size in the device environment for the current device.
  /// The lookup is first done via the device offload table. If it fails,
  /// then the lookup falls back to non-OpenMP specific lookup on the device.
  void *getOffloadVarDeviceAddr(const char *Name, size_t Size);

  /// Read KernelInfo auxiliary information for the specified kernel.
  /// The information is stored in \p KernelInfo.
  /// The function is called during the binary loading.
  bool readKernelInfo(const __tgt_offload_entry &KernelEntry);

public:
  LevelZeroProgramTy() = default;

  LevelZeroProgramTy(__tgt_device_image *Image_, ze_context_handle_t Context_,
                     ze_device_handle_t Device_, int32_t DeviceId_) :
      Image(Image_), Context(Context_), Device(Device_), DeviceId(DeviceId_) {}

  ~LevelZeroProgramTy();

  /// Build modules from the target image description
  int32_t buildModules(std::string &BuildOptions);

  /// Link modules stored in \p Modules.
  int32_t linkModules();

  /// Looks up an external global variable with the given \p Name
  /// in the device environment for device \p DeviceId.
  /// \p Size must not be null. If (*SizePtr) is not zero, then
  /// the lookup verifies that the found variable's size matches
  /// (*SizePtr), otherwise, the found variable's size is returned
  /// via \p Size.
  void *getVarDeviceAddr(const char *Name, size_t *SizePtr);

  /// Looks up an external global variable with the given \p Name
  /// and \p Size in the device environment for device \p DeviceId.
  void *getVarDeviceAddr(const char *Name, size_t Size);

  /// Build kernels from all modules.
  int32_t buildKernels();

  /// Initialize program data on device.
  int32_t initProgramData();

  /// Reset program data on device.
  int32_t resetProgramData();

  /// Return the pointer to the offload table.
  __tgt_target_table *getTablePtr() { return &Table; }

  /// Returns the auxiliary kernel information for the specified kernel.
  const KernelInfoTy *getKernelInfo(ze_kernel_handle_t Kernel) const;
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

    return WallTime;
  }

  void update(std::string &Name, ze_event_handle_t Event) {
    Data[Name].DeviceTime += getEventTime(Event);
  }
};
int64_t RTLProfileTy::Multiplier;

/// Get default command queue group ordinal
static uint32_t getCmdQueueGroupOrdinal(ze_device_handle_t Device,
                                        uint32_t &NumQueues) {
  uint32_t groupCount = 0;
  CALL_ZE_RET(UINT32_MAX, zeDeviceGetCommandQueueGroupProperties, Device,
              &groupCount, nullptr);
  std::vector<ze_command_queue_group_properties_t> groupProperties(
      groupCount, CommandQueueGroupPropertiesInit);
  CALL_ZE_RET(UINT32_MAX, zeDeviceGetCommandQueueGroupProperties, Device,
              &groupCount, groupProperties.data());
  uint32_t groupOrdinal = UINT32_MAX;
  NumQueues = 0;
  for (uint32_t i = 0; i < groupCount; i++) {
    auto &flags = groupProperties[i].flags;
    if (flags & ZE_COMMAND_QUEUE_GROUP_PROPERTY_FLAG_COMPUTE) {
      groupOrdinal = i;
      NumQueues = groupProperties[i].numQueues;
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
static std::pair<uint32_t, uint32_t> getCopyCmdQueueGroupOrdinal(
    ze_device_handle_t Device, bool LinkCopy = false) {
  uint32_t groupCount = 0;
  std::pair<uint32_t, uint32_t> Ordinal(UINT32_MAX, 0);
  CALL_ZE_RET(Ordinal, zeDeviceGetCommandQueueGroupProperties, Device,
              &groupCount, nullptr);
  std::vector<ze_command_queue_group_properties_t> groupProperties(
      groupCount, CommandQueueGroupPropertiesInit);
  CALL_ZE_RET(Ordinal, zeDeviceGetCommandQueueGroupProperties, Device,
              &groupCount, groupProperties.data());


  for (uint32_t i = 0; i < groupCount; i++) {
    auto &flags = groupProperties[i].flags;
    if ((flags & ZE_COMMAND_QUEUE_GROUP_PROPERTY_FLAG_COPY) &&
        (flags & ZE_COMMAND_QUEUE_GROUP_PROPERTY_FLAG_COMPUTE) == 0) {
      auto NumQueues = groupProperties[i].numQueues;
      if (LinkCopy && NumQueues > 1) {
        Ordinal = {i, NumQueues};
        DP("Found link copy command queue for device " DPxMOD ", ordinal = %"
           PRIu32 ", number of queues = %" PRIu32 "\n", DPxPTR(Device),
           Ordinal.first, Ordinal.second);
        break;
      } else if (!LinkCopy && NumQueues == 1) {
        Ordinal = {i, NumQueues};
        DP("Found copy command queue for device " DPxMOD ", ordinal = %" PRIu32
           "\n", DPxPTR(Device), Ordinal.first);
        break;
      }
    }
  }

  return Ordinal;
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
  DP("Created a command list " DPxMOD " (Ordinal: %" PRIu32
     ") for device %s.\n", DPxPTR(cmdList), Ordinal, DeviceIdStr.c_str());
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
  DP("Created a command queue " DPxMOD " (Ordinal: %" PRIu32 ", Index: %" PRIu32
     ") for device %s.\n", DPxPTR(cmdQueue), Ordinal, Index,
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

/// RTL flags
struct RTLFlagsTy {
  uint64_t DumpTargetImage : 1;
  uint64_t EnableProfile : 1;
  uint64_t EnableTargetGlobals : 1;
  uint64_t LinkLibDevice : 1;
  uint64_t UseHostMemForUSM : 1;
  uint64_t UseMemoryPool : 1;
  uint64_t UseDriverGroupSizes : 1;
  uint64_t UseImageOptions : 1;
  uint64_t UseMultipleComputeQueues : 1;
  uint64_t ShowBuildLog : 1;
  uint64_t UseImmCmdList : 1;
  uint64_t Reserved : 53;
  RTLFlagsTy() :
      DumpTargetImage(0),
      EnableProfile(0),
      EnableTargetGlobals(0),
      LinkLibDevice(0), // TODO: change it to 1 when L0 issue is resolved
      UseHostMemForUSM(0),
      UseMemoryPool(1),
      UseDriverGroupSizes(0),
      UseImageOptions(1),
      UseMultipleComputeQueues(0),
      ShowBuildLog(0),
      UseImmCmdList(0),
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

/// Common event pool used in the plugin. This event pool assumes all evnets
/// from the pool are host-visible and use the same event pool flag.
class EventPoolTy {
  /// Size of L0 event pool created on demand
  size_t PoolSize = 64;

  /// Context of the events
  ze_context_handle_t Context = nullptr;

  /// Additional event pool flags common to this pull
  uint32_t Flags = 0;

  /// Protection
  std::unique_ptr<std::mutex> Mtx;

  /// List of created L0 event pools
  std::list<ze_event_pool_handle_t> Pools;

  /// List of free L0 events
  std::list<ze_event_handle_t> Events;

public:
  /// Initialize context, flags, and mutex
  void init(ze_context_handle_t _Context, uint32_t _Flags) {
    Context = _Context;
    Flags = _Flags;
    Mtx.reset(new std::mutex);
  }

  /// Destroys L0 resources
  void deinit() {
    for (auto E : Events)
      CALL_ZE_RET_VOID(zeEventDestroy, E);
    for (auto P : Pools)
      CALL_ZE_RET_VOID(zeEventPoolDestroy, P);
  }

  /// Get a free event from the pool
  ze_event_handle_t getEvent() {
    std::lock_guard<std::mutex> Lock(*Mtx);

    if (Events.empty()) {
      // Need to create a new L0 pool
      ze_event_pool_desc_t Desc{ZE_STRUCTURE_TYPE_EVENT_POOL_DESC, nullptr};
      Desc.flags = ZE_EVENT_POOL_FLAG_HOST_VISIBLE | Flags;
      Desc.count = PoolSize;
      ze_event_pool_handle_t Pool;
      CALL_ZE_RET_NULL(zeEventPoolCreate, Context, &Desc, 0, nullptr, &Pool);
      Pools.push_back(Pool);

      // Create events
      ze_event_desc_t EventDesc{ZE_STRUCTURE_TYPE_EVENT_DESC, nullptr};
      EventDesc.signal = 0;
      EventDesc.wait = ZE_EVENT_SCOPE_FLAG_HOST;
      for (uint32_t I = 0; I < PoolSize; I++) {
        EventDesc.index = I;
        ze_event_handle_t Event;
        CALL_ZE_RET_NULL(zeEventCreate, Pool, &EventDesc, &Event);
        Events.push_back(Event);
      }
    }

    auto Ret = Events.back();
    Events.pop_back();

    return Ret;
  }

  /// Return an event to the pool
  void releaseEvent(ze_event_handle_t Event) {
    std::lock_guard<std::mutex> Lock(*Mtx);

    CALL_ZE_RET_VOID(zeEventHostReset, Event);
    Events.push_back(Event);
  }
};

typedef std::vector<std::vector<ze_device_handle_t>> SubDeviceListsTy;
typedef std::vector<std::vector<int32_t>> SubDeviceIdsTy;

/// Device modes for multi-tile devices
enum DeviceMode {
  DEVICE_MODE_TOP = 0,  // Use only top-level devices with subdevice clause
  DEVICE_MODE_SUB,      // Use only tiles
  DEVICE_MODE_SUBSUB    // Use only c-slices
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

struct MemAllocInfoTy {
  /// Base address allocated from compute runtime
  void *Base = nullptr;
  /// Allocation size known to users/libomptarget
  size_t Size = 0;
  /// TARGET_ALLOC kind
  int32_t Kind = TARGET_ALLOC_DEFAULT;
  /// Allocation from pool?
  bool InPool = false;
  /// Is implicit argument
  bool ImplicitArg = false;

  MemAllocInfoTy() = default;

  MemAllocInfoTy(void *_Base, size_t _Size, int32_t _Kind, bool _InPool,
                 bool _ImplicitArg) :
      Base(_Base), Size(_Size), Kind(_Kind), InPool(_InPool),
      ImplicitArg(_ImplicitArg) {}
};

/// Allocation information maintained in RTL
class MemAllocInfoMapTy {
  /// Map from allocated pointer to allocation information
  std::map<void *, MemAllocInfoTy> Map;
  /// Map from target alloc kind to number of implicit arguments
  std::map<int32_t, uint32_t> NumImplicitArgs;
  /// Mutex for guarding the internal data
  std::mutex Mtx;

public:
  void add(void *Ptr, void *_Base, size_t _Size, int32_t _Kind,
           bool _InPool = false, bool _ImplicitArg = false) {
    std::lock_guard<std::mutex> Lock(Mtx);
    auto Inserted = Map.emplace(
        Ptr, MemAllocInfoTy{_Base, _Size, _Kind, _InPool, _ImplicitArg});
#if INTEL_INTERNAL_BUILD
    // Check if we keep valid disjoint memory ranges.
    bool Valid = Inserted.second;
    if (Valid) {
      if (Inserted.first != Map.begin()) {
        auto I = std::prev(Inserted.first, 1);
        Valid = Valid && (uintptr_t)I->first + I->second.Size <= (uintptr_t)Ptr;
      }
      if (Valid) {
        auto I = std::next(Inserted.first, 1);
        if (I != Map.end())
          Valid = Valid && (uintptr_t)Ptr + _Size <= (uintptr_t)I->first;
      }
    }
    assert(Valid && "Invalid overlapping memory allocation");
#else // INTEL_INTERNAL_BUILD
    (void)Inserted;
#endif // INTEL_INTERNAL_BUILD
    if (_ImplicitArg)
      NumImplicitArgs[_Kind]++;
  }

  bool remove(void *Ptr, MemAllocInfoTy *Removed = nullptr) {
    std::lock_guard<std::mutex> Lock(Mtx);
    auto AllocInfo = Map.find(Ptr);
    if (AllocInfo == Map.end())
      return false;
    if (AllocInfo->second.ImplicitArg)
      NumImplicitArgs[AllocInfo->second.Kind]--;
    if (Removed)
      *Removed = AllocInfo->second;
    Map.erase(AllocInfo);
    return true;
  }

  const MemAllocInfoTy *find(void *Ptr) {
    std::lock_guard<std::mutex> Lock(Mtx);
    auto AllocInfo = Map.find(Ptr);
    if (AllocInfo == Map.end())
      return nullptr;
    else
      return &AllocInfo->second;
  }

  bool contains(const void *Ptr, size_t Size) {
    std::lock_guard<std::mutex> Lock(Mtx);
    if (Map.size() == 0)
      return false;
    auto I = Map.upper_bound(const_cast<void *>(Ptr));
    if (I == Map.begin())
      return false;
    --I;
    bool Ret = (uintptr_t)I->first <= (uintptr_t)Ptr &&
        (uintptr_t)Ptr + (uintptr_t)Size <=
        (uintptr_t)I->first + (uintptr_t)I->second.Size;
    return Ret;
  }

  size_t getNumImplicitArgs(int32_t Kind) {
    std::lock_guard<std::mutex> Lock(Mtx);
    return NumImplicitArgs[Kind];
  }
};

/// RTL options and flags users can override
struct RTLOptionTy {
  /// Binary flags
  RTLFlagsTy Flags;

  /// Emulated data transfer latency in microsecond
  uint32_t DataTransferLatency = 0;

  /// Device type
  int32_t DeviceType = ZE_DEVICE_TYPE_GPU;

  /// Global thread limit obtained from host runtime
  uint32_t ThreadLimit = 0;

  /// Global num teams obtained from host runtime
  uint32_t NumTeams = 0;

  /// Dynamic kernel memory size
  size_t KernelDynamicMemorySize = 0;

  /// Staging buffer size
  size_t StagingBufferSize = LEVEL0_STAGING_BUFFER_SIZE;

  /// Staging buffer count
  size_t StagingBufferCount = LEVEL0_STAGING_BUFFER_COUNT;

  /// Command batch support
  int32_t CommandBatchLevel = 0;
  int32_t CommandBatchCount = INT32_MAX;

  /// Copy engine option
  /// 0: disabled, 1: main, 2: link, 3: all (default)
  int32_t UseCopyEngine = 3;

  /// Memory pool parameters
  /// MemPoolInfo[MemType] = {AllocMax(MB), Capacity, PoolSize(MB)}
  std::map<int32_t, std::vector<int32_t>> MemPoolInfo = {
      {TARGET_ALLOC_DEVICE, {1, 4, 256}},
      {TARGET_ALLOC_HOST, {1, 4, 256}},
      {TARGET_ALLOC_SHARED, {1, 4, 256}}
  };

  /// User-directed allocation kind
  int32_t TargetAllocKind = TARGET_ALLOC_DEFAULT;

  /// Oversubscription rate for normal kernels
  uint32_t SubscriptionRate = 4;

  /// Oversubscription rate for reduction kernels
  uint32_t ReductionSubscriptionRate = 16;
  bool ReductionSubscriptionRateIsDefault = true;

  /// Forced kernel width only for internal experiments
  uint32_t ForcedKernelWidth = 0;

  /// Loop kernels with known ND-range may be known to have
  /// few iterations and they may not exploit the offload device
  /// to the fullest extent.
  /// Let's assume a device has N total HW threads available,
  /// and the kernel requires M hardware threads with LWS set to L.
  /// If (M < N * ThinThreadsThreshold), then we will try
  /// to iteratively divide L by 2 to increase the number of HW
  /// threads used for executing the kernel. Effectively, we will
  /// end up with L less than the kernel's SIMD width, so the HW
  /// threads will not use all their SIMD lanes. This (presumably) should
  /// allow more parallelism, because the stalls in the SIMD lanes
  /// will be distributed across more HW threads, and the probability
  /// of having a stall (or a sequence of stalls) on a critical path
  /// in the kernel should decrease.
  /// Anyway, this is just a heuristics that seems to work well for some
  /// kernels (which poorly expose parallelism in the first place).
  double ThinThreadsThreshold = 0.1;

  /// Decides how subdevices are exposed as OpenMP devices
  int32_t DeviceMode = DEVICE_MODE_TOP;

#if INTEL_INTERNAL_BUILD
  /// Forced GWS/LWS only for internal experiments
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
  std::string InternalCompilationOptions = "";
  std::string UserCompilationOptions = "";

  // Spec constants used for all modules.
  SpecConstantsTy CommonSpecConstants;

  /// Read environment variables
  RTLOptionTy() {
    const char *Env = nullptr;

    // Data transfer latency
    if ((Env = readEnvVar("LIBOMPTARGET_DATA_TRANSFER_LATENCY"))) {
      std::string Value(Env);
      if (Value.substr(0, 2) == "T,") {
        int32_t Usec = std::stoi(Value.substr(2).c_str());
        DataTransferLatency = (Usec > 0) ? Usec : 0;
      }
    }

    // Target device type
    if ((Env = readEnvVar("LIBOMPTARGET_DEVICETYPE"))) {
      std::string Value(Env);
      if (Value == "GPU" || Value == "gpu" || Value == "") {
        DeviceType = ZE_DEVICE_TYPE_GPU;
      } else if (Value == "CPU" || Value == "cpu") {
        DeviceType = ZE_DEVICE_TYPE_CPU;
        DP("Warning: CPU device is not supported\n");
      } else {
        DP("Warning: Invalid LIBOMPTARGET_DEVICETYPE=%s\n", Env);
      }
    }

    // Global thread limit
    int ThrLimit = omp_get_thread_limit();
    DP("omp_get_thread_limit() returned %" PRId32 "\n", ThrLimit);
    // omp_get_thread_limit() would return INT_MAX by default.
    // NOTE: Windows.h defines max() macro, so we have to guard
    //       the call with parentheses.
    ThreadLimit = (ThrLimit > 0 &&
        ThrLimit != (std::numeric_limits<int32_t>::max)()) ?
        ThrLimit : 0;

    // Global max number of teams.
    int NTeams = omp_get_max_teams();
    DP("omp_get_max_teams() returned %" PRId32 "\n", NTeams);
    // omp_get_max_teams() would return INT_MAX by default.
    // NOTE: Windows.h defines max() macro, so we have to guard
    //       the call with parentheses.
    NumTeams = (NTeams > 0 &&
        NTeams != (std::numeric_limits<int32_t>::max)()) ?
        NTeams : 0;

    // Compilation options for IGC
    if ((Env = readEnvVar("LIBOMPTARGET_LEVEL0_COMPILATION_OPTIONS")))
      UserCompilationOptions += std::string(" ") + Env;

    if (DeviceType == ZE_DEVICE_TYPE_GPU) {
      // Intel Graphics compilers that do not support that option
      // silently ignore it. Other OpenCL compilers may fail.
      Env = readEnvVar("LIBOMPTARGET_LEVEL0_TARGET_GLOBALS");
      if (!Env || parseBool(Env) != 0) {
        InternalCompilationOptions += " -cl-take-global-address ";
        Flags.EnableTargetGlobals = 1;
      }
      Env = readEnvVar("LIBOMPTARGET_LEVEL0_MATCH_SINCOSPI");
      if (!Env || parseBool(Env) != 0) {
        InternalCompilationOptions += " -cl-match-sincospi ";
      }
      Env = readEnvVar("LIBOMPTARGET_LEVEL0_USE_DRIVER_GROUP_SIZES");
      if (Env && parseBool(Env) == 1) {
        Flags.UseDriverGroupSizes = 1;
      }
    }

    // Device mode
    if ((Env = readEnvVar("LIBOMPTARGET_DEVICES"))) {
      std::string Value(Env);
      if (Value == "DEVICE" || Value == "device") {
        DP("Device mode is %s -- using top-level devices with subdevice "
           "clause support\n", Value.c_str());
        DeviceMode = DEVICE_MODE_TOP;
      } else if (Value == "SUBDEVICE" || Value == "subdevice") {
        DP("Device mode is %s -- using 1st-level sub-devices\n", Value.c_str());
        DeviceMode = DEVICE_MODE_SUB;
      } else if (Value == "SUBSUBDEVICE" || Value == "subsubdevice") {
        DP("Device mode is %s -- using 2nd-level sub-devices\n", Value.c_str());
        DeviceMode = DEVICE_MODE_SUBSUB;
      } else {
        DP("Unknown device mode %s\n", Value.c_str());
      }
    }

    // Plugin Profile
    if ((Env = readEnvVar("LIBOMPTARGET_PLUGIN_PROFILE"))) {
      if ((Env[0] == 'T' || Env[0] == '1') &&
          (Env[1] == ',' || Env[1] == '\0')) {
        Flags.EnableProfile = 1;
        RTLProfileTy::Multiplier = RTLProfileTy::MSEC_PER_SEC;
        if (Env[1] == ',') {
          std::string Unit(&Env[2]);
          if (Unit == "usec" || Unit == "unit_usec")
            RTLProfileTy::Multiplier = RTLProfileTy::USEC_PER_SEC;
        }
      }
    }

    // Managed memory allocator
    if ((Env = readEnvVar("LIBOMPTARGET_USM_HOST_MEM"))) {
      if (parseBool(Env) == 1)
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
    if ((Env = readEnvVar("LIBOMPTARGET_LEVEL0_MEMORY_POOL"))) {
      if (Env[0] == '0' && Env[1] == '\0') {
        Flags.UseMemoryPool = 0;
      } else {
        std::istringstream Str(Env);
        int32_t MemType = -1;
        int32_t Offset = 0;
        int32_t Valid = 1;
        const std::vector<int32_t> DefaultValue{1, 4, 256};
        const int32_t AllMemType = INT32_MAX;
        std::vector<int32_t> AllInfo{1, 4, 256};
        std::map<int32_t, std::vector<int32_t>> PoolInfo;
        for (std::string Token; std::getline(Str, Token, ',') && Valid > 0;) {
          if (Token == "device") {
            MemType = TARGET_ALLOC_DEVICE;
            PoolInfo.emplace(MemType, DefaultValue);
            Offset = 0;
          } else if (Token == "host") {
            MemType = TARGET_ALLOC_HOST;
            PoolInfo.emplace(MemType, DefaultValue);
            Offset = 0;
          } else if (Token == "shared") {
            MemType = TARGET_ALLOC_SHARED;
            PoolInfo.emplace(MemType, DefaultValue);
            Offset = 0;
          } else if (Token == "all") {
            MemType = AllMemType;
            Offset = 0;
            Valid = 2;
          } else if (Offset < 3 && MemType >= 0) {
            int32_t Num = std::atoi(Token.c_str());
            if (Num > 0 && MemType == AllMemType)
              AllInfo[Offset++] = Num;
            else if (Num > 0)
              PoolInfo[MemType][Offset++] = Num;
            else if (Token.size() == 0)
              Offset++;
            else
              Valid = 0;
          } else {
            Valid = 0;
          }
        }
        if (Valid > 0) {
          MemPoolInfo.clear();
          if (Valid == 2) {
            // "all" is specified -- ignore other inputs
            MemPoolInfo.emplace(TARGET_ALLOC_DEVICE, AllInfo);
            MemPoolInfo.emplace(TARGET_ALLOC_HOST, AllInfo);
            MemPoolInfo.emplace(TARGET_ALLOC_SHARED, AllInfo);
          } else {
            // Only enable what user specified
            for (auto &I : PoolInfo)
              MemPoolInfo.emplace(I.first, I.second);
          }
          // Set total pool size large enough (2 * AllocMax * Capacity)
          for (auto &I : MemPoolInfo) {
            int32_t PoolSize = 2 * I.second[0] * I.second[1];
            if (PoolSize > I.second[2]) {
              I.second[2] = PoolSize;
              DP("Adjusted memory pool size to %" PRId32 "MB\n", PoolSize);
            }
          }
        } else {
          DP("Ignoring incorrect memory pool configuration "
             "LIBOMPTARGET_LEVEL0_MEMORY_POOL=%s\n", Env);
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
    if ((Env = readEnvVar("LIBOMPTARGET_LEVEL0_USE_COPY_ENGINE"))) {
      if (Env[0] == 'F' || Env[0] == 'f' || Env[0] == '0') {
        UseCopyEngine = 0;
      } else {
        std::string Value(Env);
        if (Value == "main")
          UseCopyEngine = 1;
        else if (Value == "link")
          UseCopyEngine = 2;
        else if (Value == "all")
          UseCopyEngine = 3;
        else {
          DP("Ignoring incorrect definition, "
             "LIBOMPTARGET_LEVEL0_USE_COPY_ENGINE=%s\n", Env);
          DP("LIBOMPTARGET_LEVEL0_USE_COPY_ENGINE=<Value>\n");
          DP("  <Value>   := <Disable> | <Type>\n");
          DP("  <Disable> := 0 | F | f\n");
          DP("  <Type>    := main | link | all\n");
        }
      }
    }

    // Target image dump
    if ((Env = readEnvVar("LIBOMPTARGET_DUMP_TARGET_IMAGE",
                          "LIBOMPTARGET_SAVE_TEMPS"))) {
      if (parseBool(Env) == 1)
        Flags.DumpTargetImage = 1;
    }

    // Global default target memory that overrides device-specific default
    if ((Env = readEnvVar("LIBOMPTARGET_LEVEL0_DEFAULT_TARGET_MEM"))) {
      std::string Mem(Env);
      if (Mem == "host" || Mem == "HOST")
        TargetAllocKind = TARGET_ALLOC_HOST;
      else if (Mem == "shared" || Mem == "SHARED")
        TargetAllocKind = TARGET_ALLOC_SHARED;
      else if (Mem == "device" || Mem == "DEVICE")
        TargetAllocKind = TARGET_ALLOC_DEVICE;
      else
        DP("Warning: Ignoring unknown target memory kind %s.\n", Env);
    }

    // Target allocation kind
    if ((Env = readEnvVar("LIBOMPTARGET_LEVEL0_USE_DEVICE_MEM"))) {
      if (parseBool(Env) == 1)
        TargetAllocKind = TARGET_ALLOC_DEVICE;
    }

    // Subscription rate
    if ((Env = readEnvVar("LIBOMPTARGET_LEVEL0_SUBSCRIPTION_RATE"))) {
      int32_t Value = std::stoi(Env);
      if (Value > 0 && Value <= 0xFFFF)
        SubscriptionRate = Value;
    }

    // Reduction subscription rate: number of computed WGs is reduced
    // by this factor.
    if ((Env = readEnvVar("LIBOMPTARGET_ONEAPI_REDUCTION_SUBSCRIPTION_RATE"))) {
      int32_t Value = std::stoi(Env);
      // '0' is a special value meaning to use regular default ND-range
      // for kernels with reductions.
      if (Value >= 0 && Value <= 0xFFFF) {
        ReductionSubscriptionRate = Value;
        ReductionSubscriptionRateIsDefault = false;
      }
    }

    // Forced kernel width
    if ((Env = readEnvVar("LIBOMPTARGET_LEVEL0_KERNEL_WIDTH"))) {
      int32_t Value = std::stoi(Env);
      if (Value == 8 || Value == 16 || Value == 32)
        ForcedKernelWidth = Value;
    }

    // Dynamic memory size
    // LIBOMPTARGET_DYNAMIC_MEMORY_SIZE=<SizeInMB>
    if ((Env = readEnvVar("LIBOMPTARGET_DYNAMIC_MEMORY_SIZE"))) {
      size_t Value = std::stoi(Env);
      const size_t MaxValue = 2048;
      if (Value > MaxValue) {
        WARNING("Requested dynamic memory size %zu MB exceeds allowed limit -- "
                "setting it to %zu MB\n", Value, MaxValue);
        Value = MaxValue;
      }
      KernelDynamicMemorySize = Value << 20;
    }

#if INTEL_INTERNAL_BUILD
    // Force work group sizes -- for internal experiments
    if ((Env = readEnvVar("LIBOMPTARGET_LOCAL_WG_SIZE"))) {
      parseGroupSizes("LIBOMPTARGET_LOCAL_WG_SIZE", Env, ForcedLocalSizes);
    }
    if ((Env = readEnvVar("LIBOMPTARGET_GLOBAL_WG_SIZE"))) {
      parseGroupSizes("LIBOMPTARGET_GLOBAL_WG_SIZE", Env, ForcedGlobalSizes);
    }
#endif // INTEL_INTERNAL_BUILD

    // LIBOMPTARGET_ONEAPI_LINK_LIBDEVICE
    if ((Env = readEnvVar("LIBOMPTARGET_ONEAPI_LINK_LIBDEVICE"))) {
      int32_t Value = parseBool(Env);
      if (Value == 1)
        Flags.LinkLibDevice = 1;
      else if (Value == 0)
        Flags.LinkLibDevice = 0;
    }

    // INTEL_ENABLE_OFFLOAD_ANNOTATIONS
    if (readEnvVar("INTEL_ENABLE_OFFLOAD_ANNOTATIONS")) {
      // To match SYCL RT behavior, we just need to check whether
      // INTEL_ENABLE_OFFLOAD_ANNOTATIONS is set. The actual value
      // does not matter.
      CommonSpecConstants.addConstant<char>(0xFF747469, 1);
    }

    // LIBOMPTARGET_ONEAPI_USE_IMAGE_OPTIONS=<Bool>
    if ((Env = readEnvVar("LIBOMPTARGET_ONEAPI_USE_IMAGE_OPTIONS"))) {
      int32_t Value = parseBool(Env);
      if (Value == 1)
        Flags.UseImageOptions = 1;
      else if (Value == 0)
        Flags.UseImageOptions = 0;
    }

    // LIBOMPTARGET_LEVEL0_STAGING_BUFFER_SIZE=<SizeInKB>
    if ((Env = readEnvVar("LIBOMPTARGET_LEVEL0_STAGING_BUFFER_SIZE"))) {
      size_t SizeInKB = std::stoi(Env);
      StagingBufferSize = SizeInKB << 10;
    }

    // LIBOMPTARGET_LEVEL_ZERO_COMMAND_BATCH=<Type>[,<Count>]
    if ((Env = readEnvVar("LIBOMPTARGET_LEVEL_ZERO_COMMAND_BATCH"))) {
      std::string Value(Env);
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
        DP("Disabled command batching due to unknown input \"%s\".\n", Env);
      }
    }

    // LIBOMPTARGET_LEVEL_ZERO_USE_MULTIPLE_COMPUTE_QUEUES=<Bool>
    if ((Env =
        readEnvVar("LIBOMPTARGET_LEVEL_ZERO_USE_MULTIPLE_COMPUTE_QUEUES"))) {
      int32_t Value = parseBool(Env);
      if (Value == 1) {
        Flags.UseMultipleComputeQueues = 1;
        DP("Enabled using multiple compute queues.\n");
      } else if (Value == 0) {
        Flags.UseMultipleComputeQueues = 0;
        DP("Disabled using multiple compute queues.\n");
      }
    }

    // LIBOMPTARGET_ONEAPI_SHOW_BUILD_LOG=<Bool>
    if ((Env = readEnvVar("LIBOMPTARGET_ONEAPI_SHOW_BUILD_LOG"))) {
      int32_t Value = parseBool(Env);
      if (Value == 1)
        Flags.ShowBuildLog = 1;
      else if (Value == 0)
        Flags.ShowBuildLog = 0;
    }

    if ((Env = readEnvVar("LIBOMPTARGET_ONEAPI_THIN_THREADS_THRESHOLD"))) {
      char *StrEnd;
      double Value = std::strtod(Env, &StrEnd);
      if (errno == 0 && StrEnd != Env &&
          Value >= 0.0 && Value <= 1.0) {
        ThinThreadsThreshold = Value;
      } else {
        if (errno != 0)
          DP("Error parsing value of "
             "LIBOMPTARGET_ONEAPI_THIN_THREADS_THRESHOLD: %s\n",
             strerror(errno));
        DP("Value of LIBOMPTARGET_ONEAPI_THIN_THREADS_THRESHOLD must "
           "be a non-negative floating-point number not greater than 1.0.\n");
        DP("Using default value: %f\n", ThinThreadsThreshold);
      }
    }
    // LIBOMPTARGET_LEVEL_ZERO_USE_IMMEDIATE_COMMAND_LIST=<Bool>
    if ((Env =
        readEnvVar("LIBOMPTARGET_LEVEL_ZERO_USE_IMMEDIATE_COMMAND_LIST"))) {
      int32_t Value = parseBool(Env);
      if (Value >= 0 && Value <= 1)
        Flags.UseImmCmdList = Value;
    }
  }

  /// Read environment variable value with optional deprecated name
  const char *readEnvVar(const char *Name, const char *OldName = nullptr) {
    if (!Name)
      return nullptr;
    const char *Value = std::getenv(Name);
    if (Value || !OldName) {
      if (Value)
        DP("ENV: %s=%s\n", Name, Value);
      return Value;
    }
    Value = std::getenv(OldName);
    if (Value) {
      DP("ENV: %s=%s\n", OldName, Value);
      WARNING("%s is being deprecated. Use %s instead.\n", OldName, Name);
    }
    return Value;
  }

#if INTEL_INTERNAL_BUILD
  void parseGroupSizes(const char *Name, const char *Value, uint32_t *Sizes) {
    std::string Str(Value);
    if (Str.front() != '{' || Str.back() != '}') {
      WARNING("Ignoring invalid %s=%s\n", Name, Value);
      return;
    }
    std::istringstream Strm(Str.substr(1, Str.size() - 2));
    uint32_t I = 0;
    for (std::string Token; std::getline(Strm, Token, ','); I++)
      if (I < 3)
        Sizes[I] = std::stoi(Token);
  }
#endif // INTEL_INTERNAL_BUILD

  /// Parse boolean value
  /// Return 1 for: TRUE, T, 1, ON, YES, ENABLED (case insensitive)
  /// Return 0 for: FALSE, F, 0, OFF, NO, DISABLED (case insensitive)
  /// Return -1 for failed match
  /// NOTE: we can later simplify the document to just TRUE or FALSE like what
  /// OpenMP host runtime does.
  int32_t parseBool(const char *Value) {
    std::string Str(Value);
    std::transform(Str.begin(), Str.end(), Str.begin(),
                   [](unsigned char C) {return std::tolower(C);});
    if (Str == "true" || Str == "t" || Str == "1" || Str == "on" ||
        Str == "yes" || Str == "enabled")
      return 1;
    if (Str == "false" || Str == "f" || Str == "0" || Str == "off" ||
        Str == "no" || Str == "disabled")
      return 0;
    return -1;
  }
}; // RTLOptionTy

/// Device information
class RTLDeviceInfoTy {
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

public:
  uint32_t NumDevices = 0;

  uint32_t SubDeviceLevels = 1; // L0 API does not support recursive queries

  uint32_t NumRootDevices = 0;

  ze_driver_handle_t Driver = nullptr;

  ze_context_handle_t Context = nullptr;

  ze_api_version_t DriverAPIVersion = ZE_API_VERSION_CURRENT;

  // Driver extensions
  std::vector<ze_driver_extension_properties_t> DriverExtensions;

  // Common event pool
  EventPoolTy EventPool;

  std::vector<ze_device_properties_t> DeviceProperties;

  std::vector<ze_device_compute_properties_t> ComputeProperties;
  std::vector<ze_device_memory_properties_t> MemoryProperties;
  std::vector<ze_device_cache_properties_t> CacheProperties;

  // Internal device type ID
  std::vector<uint64_t> DeviceArchs;

  // Devices' default target allocation kinds for internal allocation
  std::vector<int32_t> AllocKinds;

  std::vector<ze_device_handle_t> Devices;

  // Subdevice IDs. It maps users' subdevice IDs to internal subdevice IDs
  std::vector<SubDeviceIdsTy> SubDeviceIds;

  // User-friendly form of device ID string
  std::vector<std::string> DeviceIdStr;

  // Command queue group ordinals for each device
  std::vector<uint32_t> CmdQueueGroupOrdinals;

  // Number of compute command queues for the device
  std::vector<uint32_t> NumCCSQueues;

  // Command queue group ordinals for copying
  std::vector<uint32_t> CopyCmdQueueGroupOrdinals;

  // Command queue group ordinals and number of queues for link copy engines
  std::vector<std::pair<uint32_t, uint32_t>> LinkCopyCmdQueueGroupOrdinals;

  // Command queue index for each device
  std::vector<uint32_t> CmdQueueIndices;

  // Command lists/queues specialized for kernel batching
  std::vector<KernelBatchTy> BatchCmdQueues;

  std::vector<ze_command_list_handle_t> ImmCmdLists;

  std::vector<std::list<LevelZeroProgramTy>> Programs;

  /// Contains all modules (possibly from multiple device images) to handle
  /// dynamic link across multiple images
  std::vector<ze_module_handle_t> GlobalModules;

  std::vector<std::map<ze_kernel_handle_t, KernelPropertiesTy>>
      KernelProperties;

  // Memory owned by the plugin
  std::vector<std::vector<void *>> OwnedMemory;

  std::vector<bool> Initialized;

  /// Default device mutexes
  std::unique_ptr<std::mutex[]> Mutexes;

  /// For internal data protection
  std::unique_ptr<std::mutex[]> DataMutexes;

  /// For kernel preparation
  std::unique_ptr<std::mutex[]> KernelMutexes;

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

  /// Internal allocation information
  std::vector<std::unique_ptr<MemAllocInfoMapTy>> MemAllocInfo;

  /// GITS function address for notifying indirect accesses
  void *GitsIndirectAllocationOffsets = nullptr;

  int64_t RequiresFlags = OMP_REQ_UNDEFINED;

  /// Number of active kernel launches for each device
  std::vector<uint32_t> NumActiveKernels;

  /// Cached target program data
  std::vector<ProgramDataTy> ProgramData;

  /// RTL option
  RTLOptionTy Option;

  RTLDeviceInfoTy() = default;

  /// Find L0 devices and initialize device properties.
  /// Returns number of devices reported to omptarget.
  int32_t findDevices();

  /// Start a user-guided kernel-batching region
  void beginKernelBatch(int32_t DeviceId, uint32_t MaxKernels);

  /// End a user-guided kernel-batching region
  void endKernelBatch(int32_t DeviceId);

  /// Return the internal device ID for the specified subdevice
  int32_t getSubDeviceId(int32_t DeviceId, uint32_t Level, uint32_t SubId);

  /// Check if the device has access to copy engines (main or link)
  bool hasCopyEngineAccess(int32_t DeviceId, bool IsMain);

  ze_command_list_handle_t getCmdList(int32_t DeviceId) {
    auto TLS = getTLS();
    auto CmdList = TLS->getCmdList(DeviceId);
    if (!CmdList) {
      CmdList = createCmdList(Context, Devices[DeviceId],
                              CmdQueueGroupOrdinals[DeviceId],
                              DeviceIdStr[DeviceId]);
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

  ze_command_queue_handle_t getCCSCmdQueue(int32_t DeviceId) {
    auto TLS = getTLS();
    auto CmdQueue = TLS->getCCSCmdQueue(DeviceId);
    if (!CmdQueue) {
      // Distribute to CCS queues
      uint32_t Index = __kmpc_global_thread_num(nullptr) %
          NumCCSQueues[DeviceId];
      CmdQueue = createCmdQueue(Context, Devices[DeviceId],
                                CmdQueueGroupOrdinals[DeviceId], Index,
                                DeviceIdStr[DeviceId]);
      TLS->setCCSCmdQueue(DeviceId, CmdQueue);
    }
    return CmdQueue;
  }

  ze_command_list_handle_t getCopyCmdList(int32_t DeviceId) {
    if (!hasCopyEngineAccess(DeviceId, true /* IsMain */)) {
      // Return link copy engine if it is enabled, first subdevice's copy engine
      // if available, compute engine otherwise.
      if (hasCopyEngineAccess(DeviceId, false))
        return getLinkCopyCmdList(DeviceId);
      int32_t FirstSubId = getSubDeviceId(DeviceId, 0, 0);
      if (FirstSubId >= 0 && hasCopyEngineAccess(FirstSubId, true))
        return getCopyCmdList(FirstSubId);
      else
        return getCmdList(DeviceId);
    }

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
    if (CopyCmdQueueGroupOrdinals[DeviceId] == UINT32_MAX &&
        LinkCopyCmdQueueGroupOrdinals[DeviceId].second == 0)
       return  false;
    return true;
  }

  ze_command_queue_handle_t getCopyCmdQueue(int32_t DeviceId) {
    if (!hasCopyEngineAccess(DeviceId, true /* IsMain */)) {
      // Return link copy engine if it is enabled, first subdevice's copy engine
      // if available, compute engine otherwise.
      if (hasCopyEngineAccess(DeviceId, false))
        return getLinkCopyCmdQueue(DeviceId);
      int32_t FirstSubId = getSubDeviceId(DeviceId, 0, 0);
      if (FirstSubId >= 0 && hasCopyEngineAccess(FirstSubId, true))
        return getCopyCmdQueue(FirstSubId);
      else
        return getCmdQueue(DeviceId);
    }

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

  ze_command_list_handle_t getLinkCopyCmdList(int32_t DeviceId) {
    if (!hasCopyEngineAccess(DeviceId, false /* IsMain */)) {
      // Return main copy engine if it is enabled, first subdevice's copy engine
      // if available, compute engine otherwise
      if (hasCopyEngineAccess(DeviceId, true))
        return getCopyCmdList(DeviceId);
      int32_t FirstSubId = getSubDeviceId(DeviceId, 0, 0);
      if (FirstSubId >= 0 && hasCopyEngineAccess(FirstSubId, false))
        return getLinkCopyCmdList(FirstSubId);
      else
        return getCmdList(DeviceId);
    }

    auto &Ordinal = LinkCopyCmdQueueGroupOrdinals[DeviceId];
    auto TLS = getTLS();
    auto CmdList = TLS->getLinkCopyCmdList(DeviceId);
    if (!CmdList) {
      CmdList = createCmdList(Context, Devices[DeviceId], Ordinal.first,
          ZE_COMMAND_LIST_FLAG_EXPLICIT_ONLY, DeviceIdStr[DeviceId]);
      TLS->setLinkCopyCmdList(DeviceId, CmdList);
    }
    return CmdList;
  }

  ze_command_queue_handle_t getLinkCopyCmdQueue(int32_t DeviceId) {
    if (!hasCopyEngineAccess(DeviceId, false /* IsMain */)) {
      // Return main copy engine if it is enabled, first subdevice's copy engine
      // if available, compute engine otherwise
      if (hasCopyEngineAccess(DeviceId, true))
        return getCopyCmdQueue(DeviceId);
      int32_t FirstSubId = getSubDeviceId(DeviceId, 0, 0);
      if (FirstSubId >= 0 && hasCopyEngineAccess(FirstSubId, false))
        return getLinkCopyCmdQueue(FirstSubId);
      else
        return getCmdQueue(DeviceId);
    }

    auto &Ordinal = LinkCopyCmdQueueGroupOrdinals[DeviceId];
    auto TLS = getTLS();
    auto CmdQueue = TLS->getLinkCopyCmdQueue(DeviceId);
    if (!CmdQueue) {
      // Try to use different copy engines for multiple threads
      uint32_t Index = __kmpc_global_thread_num(nullptr) % Ordinal.second;
      CmdQueue = createCmdQueue(Context, Devices[DeviceId], Ordinal.first,
          Index, ZE_COMMAND_QUEUE_FLAG_EXPLICIT_ONLY, DeviceIdStr[DeviceId]);
      TLS->setLinkCopyCmdQueue(DeviceId, CmdQueue);
    }
    return CmdQueue;
  }

  RTLProfileTy *getProfile(int32_t DeviceId) {
    if (!Option.Flags.EnableProfile)
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

  /// Reset program data
  int32_t resetProgramData(int32_t DeviceId);

  /// Get kernel indirect access flags
  ze_kernel_indirect_access_flags_t getKernelIndirectAccessFlags(
      ze_kernel_handle_t Kernel, uint32_t DeviceId);

  /// Enqueue copy command
  int32_t enqueueMemCopy(int32_t DeviceId, void *Dst, const void *Src,
                         size_t Size, bool Locked = false);

  /// Allocate memory from pool
  void *poolAlloc(int32_t DeviceId, size_t Size, int32_t Kind,
                  intptr_t Offset = 0);

  /// Return memory to pool
  bool poolFree(int32_t DeviceId, void *Ptr);

  /// Initialize all memory pools for the device
  void initMemoryPool(int32_t DeviceId);

  /// Initialize memory stats
  void initMemoryStat();

  /// Allocate data
  void *allocData(int32_t DeviceId, int64_t Size, void *HstPtr, void *HstBase);

  /// Return memory allocation type
  uint32_t getMemAllocType(const void *Ptr);

  /// Create command queue with the given device ID
  ze_command_queue_handle_t createCommandQueue(int32_t DeviceId);

  /// Get thread-local staging buffer for copying
  StagingBufferTy &getStagingBuffer();

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

  /// Check if the driver supports the specified extension
  bool isExtensionSupported(const char *ExtName);

  /// Initialize immediate command lists
  void initImmCmdList(int32_t DeviceId);
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

/// Read SPV from file name
static int32_t readSPVFile(const char *FileName, std::vector<uint8_t> &OutSPV) {
  // Resolve full path using the location of the plugin
  std::string FullPath;
#ifdef _WIN32
  char RTLPath[_MAX_PATH];
  HMODULE RTLModule = nullptr;
  if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                          GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR) &DebugLevel, &RTLModule)) {
    DP("Error: module creation failed -- cannot resolve full path\n");
    return OFFLOAD_FAIL;
  }
  if (!GetModuleFileNameA(RTLModule, RTLPath, sizeof(RTLPath))) {
    DP("Error: module creation failed -- cannot resolve full path\n");
    return OFFLOAD_FAIL;
  }
  FullPath = RTLPath;
#else // _WIN32
  Dl_info RTLInfo;
  if (!dladdr(&DebugLevel, &RTLInfo)) {
    DP("Error: module creation failed -- cannot resolve full path\n");
    return OFFLOAD_FAIL;
  }
  FullPath = RTLInfo.dli_fname;
#endif // _WIN32
  size_t PathSep = FullPath.find_last_of("/\\");
  FullPath.replace(PathSep + 1, std::string::npos, FileName);
  // Read from the full path
  std::ifstream IFS(FullPath.c_str(), std::ios::binary);
  if (!IFS.good())
    return OFFLOAD_FAIL;
  IFS.seekg(0, IFS.end);
  size_t SPVSize = static_cast<size_t>(IFS.tellg());
  OutSPV.resize(SPVSize);
  IFS.seekg(0);
  if (!IFS.read((char *)OutSPV.data(), SPVSize)) {
    DP("Error: module creation failed -- cannot read %s\n", FullPath.c_str());
    return OFFLOAD_FAIL;
  }
  return OFFLOAD_SUCCESS;
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
    if (!DeviceInfo->Option.Flags.EnableProfile)
      return;
    Profile = DeviceInfo->getProfile(DeviceId);
    start();
  }
  ScopedTimerTy(int32_t DeviceId, std::string name) : Name(name) {
    if (!DeviceInfo->Option.Flags.EnableProfile)
      return;
    Profile = DeviceInfo->getProfile(DeviceId);
    start();
  }
  ScopedTimerTy(int32_t DeviceId, const char *Prefix, const char *name) {
    if (!DeviceInfo->Option.Flags.EnableProfile)
      return;
    Name = Prefix;
    Name += name;
    Profile = DeviceInfo->getProfile(DeviceId);
    start();
  }
  ~ScopedTimerTy() {
    if (!DeviceInfo->Option.Flags.EnableProfile)
      return;
    if (Active)
      stop();
  }
  void start() {
    if (!DeviceInfo->Option.Flags.EnableProfile)
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
    if (!DeviceInfo->Option.Flags.EnableProfile)
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
    if (!DeviceInfo->Option.Flags.EnableProfile)
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
  if (DeviceInfo->Option.DataTransferLatency == 0)
    return;
  double goal = omp_get_wtime() + 1e-6 * DeviceInfo->Option.DataTransferLatency;
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

    if (DeviceInfo->Option.Flags.UseImmCmdList)
      CALL_ZE_RET_VOID(zeCommandListDestroy, DeviceInfo->ImmCmdLists[i]);

    DeviceInfo->Programs[i].clear();

    DeviceInfo->Mutexes[i].unlock();
  }

  if (TLSList)
    for (auto TLSPtr : *TLSList)
      delete TLSPtr;

  if (DeviceInfo->Option.Flags.UseMemoryPool) {
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

  DeviceInfo->EventPool.deinit();

  DeviceInfo->BatchCmdQueues.clear();

  if (DeviceInfo->Context)
    CALL_ZE_EXIT_FAIL(zeContextDestroy, DeviceInfo->Context);

  DP("Closed RTL successfully\n");
}

/// Allocate data explicitly
static void *allocDataExplicit(ze_device_handle_t Device, int64_t Size,
                               int32_t Kind, bool LargeMem, bool LogMemAlloc,
                               size_t Align) {
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
    CALL_ZE_RET_NULL(zeMemAllocDevice, context, &deviceDesc, Size, Align,
                     Device, &mem);
    DP("Allocated a device memory object " DPxMOD "\n", DPxPTR(mem));
    break;
  case TARGET_ALLOC_HOST:
    CALL_ZE_RET_NULL(zeMemAllocHost, context, &hostDesc, Size, Align, &mem);
    DP("Allocated a host memory object " DPxMOD "\n", DPxPTR(mem));
    break;
  case TARGET_ALLOC_SHARED:
    CALL_ZE_RET_NULL(zeMemAllocShared, context, &deviceDesc, &hostDesc, Size,
                     Align, Device, &mem);
    DP("Allocated a shared memory object " DPxMOD "\n", DPxPTR(mem));
    break;
  default:
    FATAL_ERROR("Invalid target data allocation kind");
  }

  if (LogMemAlloc)
    LOG_MEM_USAGE(Device, Size, mem);

  return mem;
}

static void *allocDataExplicit(int32_t DeviceId, int64_t Size, int32_t Kind,
                               size_t Align = LEVEL0_ALIGNMENT) {
  auto device = DeviceInfo->Devices[DeviceId];
  auto DeviceMaxMem = DeviceInfo->DeviceProperties[DeviceId].maxMemAllocSize;
  return allocDataExplicit(device, Size, Kind, (uint64_t)Size > DeviceMaxMem,
                           true /* LogMemAlloc */, Align);
}

TLSTy::~TLSTy() {
  for (auto CmdList : CmdLists)
    CALL_ZE_EXIT_FAIL(zeCommandListDestroy, CmdList.second);
  for (auto CmdList : CopyCmdLists)
    CALL_ZE_EXIT_FAIL(zeCommandListDestroy, CmdList.second);
  for (auto CmdList : LinkCopyCmdLists)
    CALL_ZE_EXIT_FAIL(zeCommandListDestroy, CmdList.second);
  for (auto CmdQueue : CmdQueues)
    CALL_ZE_EXIT_FAIL(zeCommandQueueDestroy, CmdQueue.second);
  for (auto CmdQueue : CCSCmdQueues)
    CALL_ZE_EXIT_FAIL(zeCommandQueueDestroy, CmdQueue.second);
  for (auto CmdQueue : CopyCmdQueues)
    CALL_ZE_EXIT_FAIL(zeCommandQueueDestroy, CmdQueue.second);
  for (auto CmdQueue : LinkCopyCmdQueues)
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
    if (DeviceInfo->Option.CommandBatchLevel > 1) {
      CmdList = DeviceInfo->getCmdList(DeviceId);
      CmdQueue = DeviceInfo->getCmdQueue(DeviceId);
    } else {
      CmdList = DeviceInfo->getLinkCopyCmdList(DeviceId);
      CmdQueue = DeviceInfo->getLinkCopyCmdQueue(DeviceId);
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
  if (!Always && BatchCount < DeviceInfo->Option.CommandBatchCount)
    return OFFLOAD_SUCCESS;

  DP("Command batching commits %" PRId32 " enqueued commands\n", BatchCount);

  double BatchTime = 0;
  if (DeviceInfo->Option.Flags.EnableProfile)
    BatchTime = omp_get_wtime();

  // Launch enqueued commands
  CALL_ZE_RET_FAIL(zeCommandListClose, CmdList);
  if (Kernel)
    LEVEL0_KERNEL_BEGIN(DeviceId);
  CALL_ZE_RET_FAIL_MTX(zeCommandQueueExecuteCommandLists,
                       DeviceInfo->Mutexes[DeviceId], CmdQueue, 1,
                       &CmdList, nullptr);
  CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, CmdQueue, UINT64_MAX);
  CALL_ZE_RET_FAIL(zeCommandListReset, CmdList);
  if (Kernel)
    LEVEL0_KERNEL_END(DeviceId);

  auto *Profile = DeviceInfo->getProfile(DeviceId);
  if (DeviceInfo->Option.Flags.EnableProfile && Profile) {
    BatchTime = omp_get_wtime() - BatchTime;
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
      if (KernelEvent)
        DeviceInfo->EventPool.releaseEvent(KernelEvent);
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
  if (Size <= DeviceInfo->Option.StagingBufferSize &&
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
  if (Size <= DeviceInfo->Option.StagingBufferSize &&
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
    int32_t ID, ze_kernel_handle_t _Kernel, ze_group_count_t *GroupCounts,
    std::unique_lock<std::mutex> &KernelLock) {
  if (DeviceId != ID) {
    DP("Invalid device ID %" PRId32 " while performing command batching\n", ID);
    return OFFLOAD_FAIL;
  }

  Kernel = _Kernel;
  if (DeviceInfo->Option.Flags.EnableProfile)
    KernelEvent = DeviceInfo->EventPool.getEvent();

  CALL_ZE_RET_FAIL(zeCommandListAppendLaunchKernel, CmdList, Kernel,
                   GroupCounts, KernelEvent, 0, nullptr);
  KernelLock.unlock();
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

  if (RTL->Option.MemPoolInfo.count(allocKind) == 0) {
    DP("Memory pool is disabled for %s\n", ALLOC_KIND_TO_STR(allocKind));
    return;
  }

  ze_device_properties_t properties = DevicePropertiesInit;

  size_t userAllocMax = RTL->Option.MemPoolInfo[allocKind][0];
  size_t userCapacity = RTL->Option.MemPoolInfo[allocKind][1];
  size_t userPoolSize = RTL->Option.MemPoolInfo[allocKind][2];

  Context = RTL->Context;
  AllocKind = allocKind;
  BlockCapacity = userCapacity;
  PoolSizeMax = userPoolSize << 20; // MB to B
  PoolSize = 0;

  // Decide AllocUnit. Do not log this allocation.
  void *mem = allocDataExplicit(Device, 8, AllocKind, false, false);
  ze_memory_allocation_properties_t AP = {
    ZE_STRUCTURE_TYPE_MEMORY_ALLOCATION_PROPERTIES, nullptr
  };
  CALL_ZE_EXIT_FAIL(zeMemGetAllocProperties, Context, mem, &AP, nullptr);
  AllocUnit = (std::max)(AP.pageSize, AllocUnit);
  CALL_ZE_EXIT_FAIL(zeMemFree, Context, mem);

  if (allocKind == TARGET_ALLOC_SHARED) {
    CALL_ZE_EXIT_FAIL(zeDeviceGetProperties, Device, &properties);
    if (isDiscrete(properties.deviceId)) {
      // Use page size as minimum chunk size for USM shared on discrete device.
      // FIXME: pageSize is not returned correctly (=0) on some new devices, so
      //        use fallback value for now.
      AllocMin = (std::max)(AP.pageSize, AllocUnit);
      AllocUnit = AllocMin * BlockCapacity;
    }
  }

  // Convert MB to B and round up to power of 2
  AllocMax = AllocMin << getBucketId(userAllocMax * (1 << 20));
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

  DP("Initialized %s pool for device " DPxMOD ": AllocUnit = %zu, "
     "AllocMax = %zu, " "Capacity = %" PRIu32 ", PoolSizeMax = %zu\n",
     ALLOC_KIND_TO_STR(AllocKind), DPxPTR(Device), AllocUnit, AllocMax,
     BlockCapacity, PoolSizeMax);
}

/// Reset target program data
int32_t RTLDeviceInfoTy::resetProgramData(int32_t DeviceId) {
  for (auto &PGM : Programs[DeviceId])
    if (PGM.resetProgramData() != OFFLOAD_SUCCESS)
      return OFFLOAD_FAIL;

  return OFFLOAD_SUCCESS;
}

/// Get kernel indirect access flags
ze_kernel_indirect_access_flags_t RTLDeviceInfoTy::getKernelIndirectAccessFlags(
    ze_kernel_handle_t Kernel, uint32_t DeviceId) {
  // Kernel-dependent flags
  auto KernelFlags = KernelProperties[DeviceId][Kernel].IndirectAccessFlags;

  // Tracking of individually "map" variables is disabled.
  // Unconditionally set indirect memory access, may be an overkill
  // for now.  Future level0 would auto detect which memory is
  // needs to be resident
  KernelFlags |= (AllocKinds[DeviceId] == TARGET_ALLOC_DEVICE) ?
                    ZE_KERNEL_INDIRECT_ACCESS_FLAG_DEVICE :
                    ZE_KERNEL_INDIRECT_ACCESS_FLAG_SHARED;

  // Other flags due to users' memory allocation
  if (MemAllocInfo[DeviceId]->getNumImplicitArgs(TARGET_ALLOC_DEVICE) > 0)
    KernelFlags |= ZE_KERNEL_INDIRECT_ACCESS_FLAG_DEVICE;
  if (MemAllocInfo[DeviceId]->getNumImplicitArgs(TARGET_ALLOC_SHARED) > 0)
    KernelFlags |= ZE_KERNEL_INDIRECT_ACCESS_FLAG_SHARED;
  if (MemAllocInfo[NumDevices]->getNumImplicitArgs(TARGET_ALLOC_HOST) > 0)
    KernelFlags |= ZE_KERNEL_INDIRECT_ACCESS_FLAG_HOST;

  return KernelFlags;
}

/// Enqueue memory copy
int32_t RTLDeviceInfoTy::enqueueMemCopy(
    int32_t DeviceId, void *Dst, const void *Src, size_t Size, bool Locked) {
  auto cmdList = getLinkCopyCmdList(DeviceId);
  auto cmdQueue = getLinkCopyCmdQueue(DeviceId);

  CALL_ZE_RET_FAIL(zeCommandListAppendMemoryCopy, cmdList, Dst, Src, Size,
                   nullptr, 0, nullptr);
  CALL_ZE_RET_FAIL(zeCommandListClose, cmdList);
  if (Locked) {
    CALL_ZE_RET_FAIL(zeCommandQueueExecuteCommandLists, cmdQueue, 1, &cmdList,
                     nullptr);
  } else {
    CALL_ZE_RET_FAIL_MTX(zeCommandQueueExecuteCommandLists,
                         Mutexes[DeviceId], cmdQueue, 1, &cmdList, nullptr);
  }
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

/// Initialize all memory pool for the specified device
void RTLDeviceInfoTy::initMemoryPool(int32_t DeviceId) {
  MemPoolHost.init(TARGET_ALLOC_HOST, this);
  auto &Device = Devices[DeviceId];
  if (MemPoolShared.count(Device) == 0) {
    MemPoolShared.emplace(Device, Device);
    MemPoolShared[Device].init(TARGET_ALLOC_SHARED, this);
  }
  if (MemPoolDevice.count(Device) == 0) {
    MemPoolDevice.emplace(Device, Device);
    MemPoolDevice[Device].init(TARGET_ALLOC_DEVICE, this);
  }
  // Also initialize subdevices' pool
  if (SubDeviceIds[DeviceId].size() > 0)
    for (auto SubId : SubDeviceIds[DeviceId][0])
      initMemoryPool(SubId);
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
    Buffer.init(Context, Option.StagingBufferSize, Option.StagingBufferCount);

  return Buffer;
}

const KernelInfoTy *RTLDeviceInfoTy::getKernelInfo(
    int32_t DeviceId, const ze_kernel_handle_t &Kernel) const {
  for (auto &Program : Programs[DeviceId]) {
    auto *KernelInfo = Program.getKernelInfo(Kernel);
    if (KernelInfo)
      return KernelInfo;
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
  MemAllocInfoTy Info;
  auto Removed = MemAllocInfo[DeviceId]->remove(Ptr, &Info);
  // Try again with device-independent allocation information (host USM)
  if (!Removed)
    Removed = MemAllocInfo[NumDevices]->remove(Ptr, &Info);
  if (!Removed) {
    DP("Error: Cannot find memory allocation information for " DPxMOD "\n",
       DPxPTR(Ptr));
    return OFFLOAD_FAIL;
  }

  if (Option.Flags.UseMemoryPool && Info.InPool) {
    bool Deallocated = poolFree(DeviceId, Info.Base ? Info.Base : Ptr);
    if (Deallocated) {
      DP("Returned device memory " DPxMOD " to memory pool\n", DPxPTR(Ptr));
      return OFFLOAD_SUCCESS;
    } else {
      DP("Error: Cannot return memory " DPxMOD " to memory pool\n",
         DPxPTR(Ptr));
      return OFFLOAD_FAIL;
    }
  }

  if (!Info.Base) {
    DP("Error: Cannot find base address of " DPxMOD "\n", DPxPTR(Ptr));
    return OFFLOAD_FAIL;
  }
  LOG_MEM_USAGE(Devices[DeviceId], 0, Info.Base);
  CALL_ZE_RET_FAIL_MTX(zeMemFree, Mutexes[DeviceId], Context, Info.Base);

  DP("Deleted device memory " DPxMOD " (Base: " DPxMOD ", Size: %zu)\n",
     DPxPTR(Ptr), DPxPTR(Info.Base), Info.Size);

  return OFFLOAD_SUCCESS;
}

static void dumpImageToFile(
    const void *Image, size_t ImageSize, const char *Type) {
#if INTEL_INTERNAL_BUILD
  if (DebugLevel <= 0)
    return;

  if (!DeviceInfo->Option.Flags.DumpTargetImage)
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

static int32_t appendDeviceProperties(
    ze_device_handle_t Device, std::string IdStr, uint32_t QueueIndex = 0) {
  ze_device_properties_t properties = DevicePropertiesInit;
  ze_device_compute_properties_t computeProperties =
      DeviceComputePropertiesInit;
  ze_device_memory_properties_t MemoryProperties
    { ZE_STRUCTURE_TYPE_DEVICE_MEMORY_PROPERTIES, nullptr };
  ze_device_cache_properties_t CacheProperties
    { ZE_STRUCTURE_TYPE_DEVICE_CACHE_PROPERTIES, nullptr };

  DeviceInfo->Devices.push_back(Device);

  CALL_ZE_RET_FAIL(zeDeviceGetProperties, Device, &properties);
  DeviceInfo->DeviceProperties.push_back(properties);
  DeviceInfo->DeviceArchs.push_back(getDeviceArch(properties.deviceId));
  DeviceInfo->AllocKinds.push_back(getAllocKinds(properties.deviceId));

  CALL_ZE_RET_FAIL(zeDeviceGetComputeProperties, Device, &computeProperties);
  DeviceInfo->ComputeProperties.push_back(computeProperties);

  uint32_t Count = 1;
  CALL_ZE_RET_FAIL(zeDeviceGetMemoryProperties, Device, &Count,
                   &MemoryProperties);
  DeviceInfo->MemoryProperties.push_back(MemoryProperties);
  CALL_ZE_RET_FAIL(zeDeviceGetCacheProperties, Device, &Count,
                   &CacheProperties);
  DeviceInfo->CacheProperties.push_back(CacheProperties);

  DeviceInfo->DeviceIdStr.push_back(IdStr);
  uint32_t NumQueues = 0;
  uint32_t QueueOrdinal = getCmdQueueGroupOrdinal(Device, NumQueues);
  DeviceInfo->NumCCSQueues.push_back(NumQueues);
  DeviceInfo->CmdQueueGroupOrdinals.push_back(QueueOrdinal);
  DeviceInfo->CmdQueueIndices.push_back(QueueIndex);

  auto UseCopyEngine = DeviceInfo->Option.UseCopyEngine;

  // Get main copy command queue ordinal if enabled
  std::pair<uint32_t, uint32_t> CopyOrdinal{UINT32_MAX, 0};
  if (UseCopyEngine == 1 || UseCopyEngine == 3)
    CopyOrdinal = getCopyCmdQueueGroupOrdinal(Device);
  DeviceInfo->CopyCmdQueueGroupOrdinals.push_back(CopyOrdinal.first);

  // Get link copy command queue ordinal if enabled
  CopyOrdinal = {UINT32_MAX, 0};
  if (UseCopyEngine == 2 || UseCopyEngine == 3)
    CopyOrdinal = getCopyCmdQueueGroupOrdinal(Device, true);
  DeviceInfo->LinkCopyCmdQueueGroupOrdinals.push_back(CopyOrdinal);

  DP("Found a GPU device, Name = %s\n", properties.name);

  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_number_of_devices() {
  int32_t NumDevices = DeviceInfo->findDevices();
  if (NumDevices <= 0)
    return 0;

  if (DeviceInfo->Option.DeviceMode == DEVICE_MODE_TOP)
    DP("Returning %" PRIu32 " top-level devices\n", NumDevices);
  else
    DP("Returning %" PRIu32 " devices including sub-devices\n", NumDevices);

  return NumDevices;
}

EXTERN int32_t __tgt_rtl_init_device(int32_t DeviceId) {
  if (DeviceId < 0 || DeviceId >= (int32_t)DeviceInfo->NumDevices ||
      (DeviceInfo->Option.DeviceMode == DEVICE_MODE_TOP &&
       DeviceId >= (int32_t)DeviceInfo->NumRootDevices)) {
    DP("Bad device ID %" PRId32 "\n", DeviceId);
    return OFFLOAD_FAIL;
  }

  if (DeviceInfo->Option.Flags.UseMemoryPool)
    DeviceInfo->initMemoryPool(DeviceId);

  if (DeviceInfo->Option.Flags.UseImmCmdList)
    DeviceInfo->initImmCmdList(DeviceId);

  for (auto &SubIds : DeviceInfo->SubDeviceIds[DeviceId])
    for (auto SubId : SubIds)
      DeviceInfo->Initialized[SubId] = true;

  DeviceInfo->Initialized[DeviceId] = true;

  OMPT_CALLBACK(ompt_callback_device_initialize, DeviceId,
                DeviceInfo->DeviceProperties[DeviceId].name,
                DeviceInfo->Devices[DeviceId],
                omptLookupEntries, OmptDocument);

  DP("Initialized Level0 device %" PRId32 "\n", DeviceId);
  return OFFLOAD_SUCCESS;
}

EXTERN __tgt_target_table *__tgt_rtl_load_binary(
    int32_t DeviceId, __tgt_device_image *Image) {
  DP("Device %" PRId32 ": Loading binary from " DPxMOD "\n", DeviceId,
     DPxPTR(Image->ImageStart));

  size_t ImageSize = (size_t)Image->ImageEnd - (size_t)Image->ImageStart;
  size_t NumEntries = (size_t)(Image->EntriesEnd - Image->EntriesBegin);

  DP("Expecting to have %zu entries defined\n", NumEntries);

  auto &Option = DeviceInfo->Option;
  std::string CompilationOptions(Option.CompilationOptions + " " +
                                 Option.UserCompilationOptions);

  DP("Base L0 module compilation options: %s\n", CompilationOptions.c_str());

  CompilationOptions += " " + Option.InternalCompilationOptions;

  dumpImageToFile(Image->ImageStart, ImageSize, "OpenMP");

  auto Context = DeviceInfo->Context;
  auto Device = DeviceInfo->Devices[DeviceId];
  DeviceInfo->Programs[DeviceId].emplace_back(Image, Context, Device, DeviceId);
  auto &Program = DeviceInfo->Programs[DeviceId].back();

  int32_t RC = Program.buildModules(CompilationOptions);
  if (RC != OFFLOAD_SUCCESS)
    return nullptr;

  RC = Program.linkModules();
  if (RC != OFFLOAD_SUCCESS)
    return nullptr;

  RC = Program.buildKernels();
  if (RC != OFFLOAD_SUCCESS)
    return nullptr;

  RC = Program.initProgramData();
  if (RC != OFFLOAD_SUCCESS)
    return nullptr;

  auto *Table = Program.getTablePtr();

  // Handle subdevice clause
  if ((uint32_t)DeviceId < DeviceInfo->NumRootDevices) {
    for (auto &SubIdList : DeviceInfo->SubDeviceIds[DeviceId])
      for (auto SubId : SubIdList)
        // Use root module while copying kernel properties from root.
        DeviceInfo->KernelProperties[SubId] =
            DeviceInfo->KernelProperties[DeviceId];
  }

  OMPT_CALLBACK(ompt_callback_device_load, DeviceId,
                "" /* filename */,
                -1 /* offset_in_file */,
                nullptr /* vma_in_file */,
                Table->EntriesEnd - Table->EntriesBegin /* bytes */,
                Table->EntriesBegin /* host_addr */,
                nullptr /* device_addr */,
                0 /* module_id */);

  return Table;
}

void *RTLDeviceInfoTy::allocData(int32_t DeviceId, int64_t Size, void *HstPtr,
                                 void *HstBase) {
  DeviceId = getInternalDeviceId(DeviceId);

  // TODO: this seems necessary for now -- check with L0 driver team for details
  std::unique_lock<std::mutex> AllocLock(Mutexes[DeviceId]);

  ScopedTimerTy TmDataAlloc(DeviceId, "DataAlloc");
  intptr_t Offset = (intptr_t)HstPtr - (intptr_t)HstBase;
  size_t AllocSize =
      (Offset < 0 && ABS(Offset) >= Size) ? ABS(Offset) + 1 : Size;

  Offset = (Offset >= 0) ? Offset : 0;
  AllocSize += Offset;

  void *Base = nullptr;
  void *Mem = nullptr;
  auto AllocKind = AllocKinds[DeviceId];
  auto ID = (AllocKind != TARGET_ALLOC_HOST) ? DeviceId : NumDevices;

  if (Option.Flags.UseMemoryPool) {
    Base = DeviceInfo->poolAlloc(DeviceId, AllocSize, AllocKind, Offset);
    if (Base != nullptr) {
      Mem = (void *)((intptr_t)Base + Offset);
      DP("Allocated target memory " DPxMOD " (Base: " DPxMOD ", Size: %zu) "
         "from memory pool for host ptr " DPxMOD "\n", DPxPTR(Mem),
         DPxPTR(Base), AllocSize, DPxPTR(HstPtr));
      MemAllocInfo[ID]->add(Mem, nullptr /* Base */, Size, AllocKind,
                            true /* InPool */);
      return Mem;
    }
  }

  // Use device-specific allocation type
  Base = allocDataExplicit(DeviceId, AllocSize, AllocKind);
  if (Base) {
    Mem = (void *)((intptr_t)Base + Offset);
    MemAllocInfo[ID]->add(Mem, Base, Size, AllocKind);
    if (DebugLevel > 0) {
      void *ActualBase = nullptr;
      size_t ActualSize = 0;
      CALL_ZE_RET_NULL(zeMemGetAddressRange, Context, Mem, &ActualBase,
                       &ActualSize);
      assert(Base == ActualBase && "Invalid memory address range!");
      DP("Allocated target memory " DPxMOD " (Base: " DPxMOD
         ", Size: %zu) for host ptr " DPxMOD "\n", DPxPTR(Mem),
         DPxPTR(ActualBase), ActualSize, DPxPTR(HstPtr));
    }
  }

  return Mem;
}

static void *dataAllocExplicit(
    int32_t DeviceId, int64_t Size, int32_t Kind, size_t Align = 0) {
  void *Mem = nullptr;
  void *Base = nullptr;
  bool InPool = false;

  if (DeviceInfo->Option.Flags.UseMemoryPool)
    Mem = DeviceInfo->poolAlloc(DeviceId, Size + Align, Kind);

  if (Mem) {
    InPool = true;
    if (Align > 0) {
      Base = Mem; // Use Base address in later poolFree() operation
      Mem = (void *)(((uintptr_t)Mem + Align) & ~(Align - 1));
    }
  } else {
    Mem = allocDataExplicit(DeviceId, Size, Kind, Align);
    Base = Mem;
  }

  if (Mem) {
    auto ID = (Kind == TARGET_ALLOC_HOST) ? DeviceInfo->NumDevices : DeviceId;
    DeviceInfo->MemAllocInfo[ID]->add(Mem, Base, Size, Kind, InPool,
                                      true /* IsImplicitArg */);
  }

  return Mem;
}

EXTERN void *__tgt_rtl_data_alloc(int32_t DeviceId, int64_t Size, void *HstPtr,
                                  int32_t Kind) {
  void *Mem = nullptr;

  if (HstPtr) {
    Mem = DeviceInfo->allocData(DeviceId, Size, HstPtr, HstPtr);
  } else {
    // User allocation
    int32_t AllocKind = Kind;
    if (AllocKind == TARGET_ALLOC_DEFAULT) {
      // Set allocation kind to device while respecting
      // LIBOMPTARGET_LEVEL0_DEFAULT_TARGET_MEM.
      AllocKind = DeviceInfo->Option.TargetAllocKind;
      if (AllocKind == TARGET_ALLOC_DEFAULT)
        AllocKind = TARGET_ALLOC_DEVICE;
    }
    Mem = dataAllocExplicit(DeviceId, Size, AllocKind);
  }

  return Mem;
}

EXTERN void *__tgt_rtl_data_alloc_base(int32_t DeviceId, int64_t Size,
                                       void *HstPtr, void *HstBase) {
  void *Mem = DeviceInfo->allocData(DeviceId, Size, HstPtr, HstBase);
  return Mem;
}

EXTERN void *__tgt_rtl_data_alloc_managed(int32_t DeviceId, int64_t Size) {
  int32_t Kind = DeviceInfo->Option.Flags.UseHostMemForUSM
                    ? TARGET_ALLOC_HOST : TARGET_ALLOC_SHARED;
  void *Mem = dataAllocExplicit(DeviceId, Size, Kind);

  return Mem;
}

EXTERN void *__tgt_rtl_data_realloc(
    int32_t DeviceId, void *Ptr, size_t Size, int32_t Kind) {
  const MemAllocInfoTy *Info = nullptr;

  if (Ptr) {
    if (DeviceInfo->getMemAllocType(Ptr) == ZE_MEMORY_TYPE_HOST)
      Info = DeviceInfo->MemAllocInfo[DeviceInfo->NumDevices]->find(Ptr);
    else
      Info = DeviceInfo->MemAllocInfo[DeviceId]->find(Ptr);
    if (!Info) {
      DP("Error: Cannot find allocation information for pointer " DPxMOD "\n",
         DPxPTR(Ptr));
      return nullptr;
    }
    if (Size <= Info->Size && Kind == Info->Kind) {
      DP("Returning the same pointer " DPxMOD " as reallocation is unneeded\n",
         DPxPTR(Ptr));
      return Ptr;
    }
  }

  int32_t AllocKind = Kind;
  if (AllocKind == TARGET_ALLOC_DEFAULT) {
    AllocKind = DeviceInfo->Option.TargetAllocKind;
    if (AllocKind == TARGET_ALLOC_DEFAULT)
      AllocKind = TARGET_ALLOC_DEVICE;
  }

  void *Mem = dataAllocExplicit(DeviceId, Size, AllocKind);

  if (Mem && Info) {
    int32_t Rc = OFFLOAD_SUCCESS;
    if (AllocKind == TARGET_ALLOC_DEVICE || Info->Kind == TARGET_ALLOC_DEVICE)
      Rc = DeviceInfo->enqueueMemCopy(DeviceId, Mem, Ptr, Info->Size);
    else
      std::copy_n((char *)Ptr, Info->Size, (char *)Mem);
    if (Rc != OFFLOAD_SUCCESS)
      return nullptr;
    Rc = DeviceInfo->dataDelete(DeviceId, Ptr);
    if (Rc != OFFLOAD_SUCCESS)
      return nullptr;
  }

  return Mem;
}

EXTERN void *__tgt_rtl_data_aligned_alloc(
    int32_t DeviceId, size_t Align, size_t Size, int32_t Kind) {
  if (Align != 0 && (Align & (Align - 1)) != 0) {
    DP("Error: Alignment %zu is not power of two.\n", Align);
    return nullptr;
  }

  int32_t AllocKind = Kind;
  if (AllocKind == TARGET_ALLOC_DEFAULT) {
    AllocKind = DeviceInfo->Option.TargetAllocKind;
    if (AllocKind == TARGET_ALLOC_DEFAULT)
      AllocKind = TARGET_ALLOC_DEVICE;
  }

  return dataAllocExplicit(DeviceId, Size, AllocKind, Align);
}

EXTERN int32_t __tgt_rtl_is_device_accessible_ptr(int32_t DeviceId, void *Ptr) {
  int32_t ret = DeviceInfo->getMemAllocType(Ptr) != ZE_MEMORY_TYPE_UNKNOWN;

  DP("Ptr " DPxMOD " is %sa device accessible memory pointer.\n", DPxPTR(Ptr),
     ret ? "" : "not ");
  return ret;
}

static int32_t submitData(int32_t DeviceId, void *TgtPtr, void *HstPtr,
                          int64_t Size) {
  if (Size == 0)
    return OFFLOAD_SUCCESS;

  DeviceId = DeviceInfo->getInternalDeviceId(DeviceId);

  if (DeviceInfo->Option.CommandBatchLevel > 0) {
    auto &Batch = getTLS()->getCommandBatch();
    if (Batch.isActive())
      return Batch.enqueueMemCopyTo(DeviceId, TgtPtr, HstPtr, Size);
  }

  ScopedTimerTy tmDataWrite(DeviceId, "DataWrite (Host to Device)");

  // Add synthetic delay for experiments
  addDataTransferLatency();

  auto DiscreteDevice = DeviceInfo->isDiscreteDevice(DeviceId);
  auto TgtPtrType = DeviceInfo->getMemAllocType(TgtPtr);
  if (DiscreteDevice || TgtPtrType == ZE_MEMORY_TYPE_DEVICE) {
    void *SrcPtr = HstPtr;
    if (DiscreteDevice &&
        static_cast<size_t>(Size) <= DeviceInfo->Option.StagingBufferSize) {
      SrcPtr = DeviceInfo->getStagingBuffer().get();
      std::copy_n(
          static_cast<char *>(HstPtr), Size, static_cast<char *>(SrcPtr));
    }
    if (DeviceInfo->enqueueMemCopy(DeviceId, TgtPtr, SrcPtr, Size) !=
        OFFLOAD_SUCCESS)
      return OFFLOAD_FAIL;
  } else {
    std::copy_n(
        static_cast<char *>(HstPtr), Size, static_cast<char *>(TgtPtr));
  }
  DP("Copied %" PRId64 " bytes (hst:" DPxMOD ") -> (tgt:" DPxMOD ")\n", Size,
     DPxPTR(HstPtr), DPxPTR(TgtPtr));

  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_data_submit(
    int32_t DeviceId, void *TgtPtr, void *HstPtr, int64_t Size) {
  return submitData(DeviceId, TgtPtr, HstPtr, Size);
}

EXTERN int32_t __tgt_rtl_data_submit_async(
    int32_t DeviceId, void *TgtPtr, void *HstPtr, int64_t Size,
    __tgt_async_info *AsyncInfoPtr /*not used*/) {
  return submitData(DeviceId, TgtPtr, HstPtr, Size);
}

static int32_t retrieveData(int32_t DeviceId, void *HstPtr, void *TgtPtr,
                            int64_t Size) {
  if (Size == 0)
    return OFFLOAD_SUCCESS;

  DeviceId = DeviceInfo->getInternalDeviceId(DeviceId);

  if (DeviceInfo->Option.CommandBatchLevel > 0) {
    auto &Batch = getTLS()->getCommandBatch();
    if (Batch.isActive())
      return Batch.enqueueMemCopyFrom(DeviceId, HstPtr, TgtPtr, Size);
  }

  ScopedTimerTy tmDataRead(DeviceId, "DataRead (Device to Host)");

  // Add synthetic delay for experiments
  addDataTransferLatency();

  auto DiscreteDevice = DeviceInfo->isDiscreteDevice(DeviceId);
  auto TgtPtrType = DeviceInfo->getMemAllocType(TgtPtr);
  if (DiscreteDevice || TgtPtrType == ZE_MEMORY_TYPE_DEVICE) {
    void *DstPtr = HstPtr;
    if (DiscreteDevice &&
        static_cast<size_t>(Size) <= DeviceInfo->Option.StagingBufferSize) {
      DstPtr = DeviceInfo->getStagingBuffer().get();
    }
    if (OFFLOAD_SUCCESS !=
        DeviceInfo->enqueueMemCopy(DeviceId, DstPtr, TgtPtr, Size))
      return OFFLOAD_FAIL;
    if (DstPtr != HstPtr)
      std::copy_n(
          static_cast<char *>(DstPtr), Size, static_cast<char *>(HstPtr));
  } else {
    std::copy_n(
        static_cast<char *>(TgtPtr), Size, static_cast<char *>(HstPtr));
  }
  DP("Copied %" PRId64 " bytes (tgt:" DPxMOD ") -> (hst:" DPxMOD ")\n", Size,
     DPxPTR(TgtPtr), DPxPTR(HstPtr));

  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_data_retrieve(
    int32_t DeviceId, void *HstPtr, void *TgtPtr, int64_t Size) {
  return retrieveData(DeviceId, HstPtr, TgtPtr, Size);
}

EXTERN int32_t __tgt_rtl_data_retrieve_async(
    int32_t DeviceId, void *HstPtr, void *TgtPtr, int64_t Size,
    __tgt_async_info *AsyncInfoPtr /*not used*/) {
  return retrieveData(DeviceId, HstPtr, TgtPtr, Size);
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
  // TODO: D2D copy with copy engine is slower than using the default queue as
  // reported in CMPLRLIBS-33721. Use the default queue for now until we find
  // different result.
  auto cmdList = DeviceInfo->getCmdList(DstId);
  auto cmdQueue = DeviceInfo->getCmdQueue(DstId);

  CALL_ZE_RET_FAIL(zeCommandListAppendMemoryCopy, cmdList, DstPtr, SrcPtr, Size,
                   nullptr, 0, nullptr);
  CALL_ZE_RET_FAIL(zeCommandListClose, cmdList);
  CALL_ZE_RET_FAIL_MTX(zeCommandQueueExecuteCommandLists,
                       DeviceInfo->Mutexes[DstId], cmdQueue, 1, &cmdList,
                       nullptr);
  CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, cmdQueue, UINT64_MAX);
  CALL_ZE_RET_FAIL(zeCommandListReset, cmdList);

  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_data_delete(int32_t DeviceId, void *TgtPtr) {
  DeviceId = DeviceInfo->getInternalDeviceId(DeviceId);

  if (DeviceInfo->Option.CommandBatchLevel > 0) {
    auto &Batch = getTLS()->getCommandBatch();
    if (Batch.isActive())
      return Batch.enqueueMemFree(DeviceId, TgtPtr);
  }

  return DeviceInfo->dataDelete(DeviceId, TgtPtr);
}

// Return the number of total HW threads required to execute
// a loop kernel compiled with the given simdWidth, and the given
// loop(s) trip counts and group sizes.
// Returns UINT64_MAX, if computations overflow.
static uint64_t computeThreadsNeeded(
    const size_t (&tripCounts)[3], const uint32_t (&groupSizes)[3],
    uint32_t simdWidth) {
  uint64_t groupCount[3];
  for (int i = 0; i < 3; ++i) {
    if (tripCounts[i] == 0 || groupSizes[i] == 0)
      return (std::numeric_limits<uint64_t>::max)();
    groupCount[i] =
        (uint64_t(tripCounts[i]) + groupSizes[i] - 1) / groupSizes[i];
    if (groupCount[i] > (std::numeric_limits<uint32_t>::max)())
      return (std::numeric_limits<uint64_t>::max)();
  }
  for (int i = 1; i < 3; ++i) {
    if ((std::numeric_limits<uint64_t>::max)() / groupCount[0] <
	groupCount[i])
      return (std::numeric_limits<uint64_t>::max)();
    groupCount[0] *= groupCount[i];
  }
  // Multiplication of the group sizes must never overflow uint64_t
  // for any existing device.
  uint64_t localWorkSize =
      uint64_t(groupSizes[0]) * groupSizes[1] * groupSizes[2];
  uint64_t threadsPerWG = ((localWorkSize + simdWidth - 1) / simdWidth);

  // Check that the total number of threads fits uint64_t.
  if ((std::numeric_limits<uint64_t>::max)() / groupCount[0] <
      threadsPerWG)
    return (std::numeric_limits<uint64_t>::max)();

  return groupCount[0] * threadsPerWG;
}

static int32_t decideLoopKernelGroupArguments(
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

  if (DeviceInfo->Option.ThreadLimit > 0) {
    maxGroupSizeForced = true;

    if (DeviceInfo->Option.ThreadLimit <= maxGroupSize) {
      maxGroupSize = DeviceInfo->Option.ThreadLimit;
      DP("Max group size is set to %" PRIu32 " (OMP_THREAD_LIMIT)\n",
         maxGroupSize);
    } else {
      DP("OMP_THREAD_LIMIT(%" PRIu32 ") exceeds current maximum %" PRIu32 "\n",
         DeviceInfo->Option.ThreadLimit, maxGroupSize);
    }
  }

  if (DeviceInfo->Option.NumTeams > 0)
    DP("OMP_NUM_TEAMS(%" PRIu32 ") is ignored\n", DeviceInfo->Option.NumTeams);

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
  size_t tripCounts[3] = {1, 1, 1};

  for (int32_t i = 0; i < numLoopLevels; i++) {
    assert(level[i].Stride > 0 && "Invalid loop stride for ND partitioning");
    DP("Level %" PRIu32 ": Lb = %" PRId64 ", Ub = %" PRId64 ", Stride = %"
       PRId64 "\n", i, level[i].Lb, level[i].Ub, level[i].Stride);
    if (level[i].Ub < level[i].Lb)
      tripCounts[i] = 0;
    else
      tripCounts[i] =
          (level[i].Ub - level[i].Lb + level[i].Stride) / level[i].Stride;
  }

  // Check if any of the loop has zero iterations.
  if (tripCounts[0] == 0 || tripCounts[1] == 0 || tripCounts[2] == 0) {
    std::fill(GroupSizes, GroupSizes + 3, 1);
    std::fill(groupCounts, groupCounts + 3, 1);
    if (distributeDim > 0 && tripCounts[distributeDim] != 0) {
      // There is a distribute dimension, and the distribute loop
      // has non-zero iterations, but some inner parallel loop
      // has zero iterations. We still want to split the distribute
      // loop's iterations between many WGs (of size 1), but the inner/lower
      // dimensions should be 1x1.
      // Note that this code is currently dead, because we are not
      // hoisting the inner loops' bounds outside of the target regions.
      // The code is here just for completeness.
      size_t distributeTripCount = tripCounts[distributeDim];
      if (distributeTripCount > UINT32_MAX) {
        DP("Invalid group count %zu due to large loop trip count\n",
           distributeTripCount);
        return OFFLOAD_FAIL;
      }
      groupCounts[distributeDim] = distributeTripCount;
    }
    GroupCounts.groupCountX = groupCounts[0];
    GroupCounts.groupCountY = groupCounts[1];
    GroupCounts.groupCountZ = groupCounts[2];
    return OFFLOAD_SUCCESS;
  }

  if (!maxGroupSizeForced) {
    // Use zeKernelSuggestGroupSize to compute group sizes,
    // or fallback to setting dimension 0 width to SIMDWidth.
    // Note that in case of user-specified LWS groupSizes[0]
    // is already set according to the specified value.
    size_t globalSizes[3] = { tripCounts[0], tripCounts[1], tripCounts[2] };
    if (distributeDim > 0) {
      // There is a distribute dimension.
      globalSizes[distributeDim - 1] *= globalSizes[distributeDim];
      globalSizes[distributeDim] = 1;
    }
    bool LargeGlobalSize = globalSizes[0] > UINT32_MAX ||
                           globalSizes[1] > UINT32_MAX ||
                           globalSizes[2] > UINT32_MAX;

    ze_result_t rc = ZE_RESULT_ERROR_UNKNOWN;
    uint32_t suggestedGroupSizes[3];
    if (DeviceInfo->Option.Flags.UseDriverGroupSizes && !LargeGlobalSize) {
      // Call this only when global sizes satisfy API requirement.
      CALL_ZE_RC(rc, zeKernelSuggestGroupSize,
                 Kernel, (uint32_t)globalSizes[0], (uint32_t)globalSizes[1],
                 (uint32_t)globalSizes[2], &suggestedGroupSizes[0],
                 &suggestedGroupSizes[1], &suggestedGroupSizes[2]);
    }

    if (rc == ZE_RESULT_SUCCESS) {
      groupSizes[0] = suggestedGroupSizes[0];
      groupSizes[1] = suggestedGroupSizes[1];
      groupSizes[2] = suggestedGroupSizes[2];
    } else {
      if (maxGroupSize > kernelWidth) {
	groupSizes[0] = kernelWidth;
      }
      if (distributeDim == 0) {
        // If there is a distribute dimension, then we do not use
        // thin HW threads, since we do not know anything about
        // the iteration space of the inner parallel loop regions.
        //
        // If there is no distribute dimension, then try to use thiner
        // HW threads to get more independent HW threads executing
        // the kernel - this may allow more parallelism due to
        // the stalls being distributed across multiple HW threads rather
        // than across SIMD lanes within one HW thread.
	assert(groupSizes[1] == 1 && groupSizes[2] == 1 &&
	       "Unexpected group sizes for dimensions 1 or/and 2.");
	uint32_t simdWidth = KernelProperty.SIMDWidth;
	auto &deviceProperties = DeviceInfo->DeviceProperties[DeviceId];
	uint32_t numEUsPerSubslice = deviceProperties.numEUsPerSubslice;
	uint32_t numSubslices = deviceProperties.numSlices *
	    deviceProperties.numSubslicesPerSlice;
	uint32_t numThreadsPerEU = deviceProperties.numThreadsPerEU;
	uint64_t totalThreads = uint64_t(numThreadsPerEU) * numEUsPerSubslice *
	    numSubslices;
        totalThreads *= DeviceInfo->Option.ThinThreadsThreshold;

	uint32_t groupSizePrev = groupSizes[0];
	uint64_t threadsNeeded =
	    computeThreadsNeeded(tripCounts, groupSizes, simdWidth);
	while (threadsNeeded < totalThreads) {
	  groupSizePrev = groupSizes[0];
          // Try to half the local work size (if possible) and see
          // how many HW threads the kernel will require with this
          // new local work size.
          // In most implementations the initial groupSizes[0]
          // will be a power-of-two.
	  if (groupSizes[0] <= 1)
	    break;
	  groupSizes[0] >>= 1;
	  threadsNeeded =
	      computeThreadsNeeded(tripCounts, groupSizes, simdWidth);
	}
	groupSizes[0] = groupSizePrev;
      }
    }
  }

  for (int32_t i = 0; i < numLoopLevels; i++) {
    if (i < distributeDim) {
      groupCounts[i] = 1;
      continue;
    }
    size_t trip = tripCounts[i];
    if (groupSizes[i] >= trip)
      groupSizes[i] = trip;
    size_t Count = (trip + groupSizes[i] - 1) / groupSizes[i];
    if (Count > UINT32_MAX) {
      DP("Invalid group count %zu due to large loop trip count\n", Count);
      return OFFLOAD_FAIL;
    }
    groupCounts[i] = (uint32_t)Count;
  }

  GroupCounts.groupCountX = groupCounts[0];
  GroupCounts.groupCountY = groupCounts[1];
  GroupCounts.groupCountZ = groupCounts[2];
  std::copy(groupSizes, groupSizes + 3, GroupSizes);

  return OFFLOAD_SUCCESS;
}

static void decideKernelGroupArguments(
    int32_t DeviceId, uint32_t NumTeams, uint32_t ThreadLimit,
    ze_kernel_handle_t Kernel, uint32_t *GroupSizes,
    ze_group_count_t &GroupCounts) {

  const KernelInfoTy *KInfo = DeviceInfo->getKernelInfo(DeviceId, Kernel);
  if (!KInfo) {
    DP("Warning: Cannot find kernel information for kernel " DPxMOD ".\n",
       DPxPTR(Kernel));
  }
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
  uint32_t simdWidth = KernelProperty.SIMDWidth;
  DP("Assumed kernel SIMD width is %" PRIu32 "\n", simdWidth);
  DP("Preferred group size is multiple of %" PRIu32 "\n", kernelWidth);
  assert(simdWidth <= kernelWidth && "Invalid SIMD width.");

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

  if (DeviceInfo->Option.ThreadLimit > 0) {
    maxGroupSizeForced = true;

    if (DeviceInfo->Option.ThreadLimit <= maxGroupSize) {
      maxGroupSize = DeviceInfo->Option.ThreadLimit;
      DP("Max group size is set to %" PRIu32 " (OMP_THREAD_LIMIT)\n",
         maxGroupSize);
    } else {
      DP("OMP_THREAD_LIMIT(%" PRIu32 ") exceeds current maximum %" PRIu32 "\n",
         DeviceInfo->Option.ThreadLimit, maxGroupSize);
    }
  }

  uint32_t maxGroupCount = 0;

  if (NumTeams > 0) {
    maxGroupCount = NumTeams;
    maxGroupCountForced = true;
    DP("Max group count is set to %" PRIu32
       " (num_teams clause or no teams construct)\n", maxGroupCount);
  } else if (DeviceInfo->Option.NumTeams > 0) {
    // OMP_NUM_TEAMS only matters, if num_teams() clause is absent.
    maxGroupCount = DeviceInfo->Option.NumTeams;
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
          (maxGroupSize + simdWidth - 1) / simdWidth;
      uint32_t numGroupsPerSubslice =
          (numThreadsPerSubslice + numThreadsPerGroup - 1) / numThreadsPerGroup;
      maxGroupCount = numGroupsPerSubslice * numSubslices;
    } else {
      // For kernels with cross-WG reductions use LWS equal
      // to kernelWidth. This is just a performance heuristic.
      if (KInfo && KInfo->getHasTeamsReduction() &&
          // Only do this for discrete devices.
          DeviceInfo->isDiscreteDevice(DeviceId) &&
          DeviceInfo->Option.ReductionSubscriptionRate &&
          // Do not use this heuristic for kernels that use
          // atomic-free reductions. We want to maximize
          // LWS for such kernels.
          !KInfo->isAtomicFreeReduction())
        maxGroupSize = kernelWidth;

      assert(!maxGroupSizeForced && !maxGroupCountForced);
      assert((maxGroupSize <= kernelWidth ||
              maxGroupSize % kernelWidth == 0) && "Invalid maxGroupSize");
      // Maximize group size
      while (maxGroupSize >= kernelWidth) {
        uint32_t numThreadsPerGroup =
            (maxGroupSize + simdWidth - 1) / simdWidth;
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
  if (KInfo && KInfo->getWINum()) {
    groupSizes[0] =
        (std::min)(KInfo->getWINum(), static_cast<uint64_t>(groupSizes[0]));
    DP("Capping maximum thread group size to %" PRIu64
       " due to kernel constraints (reduction).\n", KInfo->getWINum());
  }
  if (!maxGroupCountForced) {
    if (KInfo && KInfo->getHasTeamsReduction() &&
        DeviceInfo->Option.ReductionSubscriptionRate) {
      if (DeviceInfo->isDiscreteDevice(DeviceId) &&
          (!KInfo->isAtomicFreeReduction() ||
           !DeviceInfo->Option.ReductionSubscriptionRateIsDefault)) {
        // Do not apply ReductionSubscriptionRate for non-discrete devices.
        // But we have to avoid the regular SubscriptionRate in the else
        // clause. Basically, for non-discrete devices, the reduction
        // subscription rate is 1.
        //
        // Also use reduction subscription rate 1 for kernels using
        // atomic-free reductions, unless user forced reduction subscription
        // rate via environment.
        groupCounts[0] /= DeviceInfo->Option.ReductionSubscriptionRate;
        groupCounts[0] = (std::max)(groupCounts[0], 1u);
      }
    } else {
      groupCounts[0] *= DeviceInfo->Option.SubscriptionRate;
    }
  }
  if (KInfo && KInfo->getWGNum()) {
    groupCounts[0] =
        (std::min)(KInfo->getWGNum(), static_cast<uint64_t>(groupCounts[0]));
    DP("Capping maximum thread groups count to %" PRIu64
       " due to kernel constraints (reduction).\n", KInfo->getWGNum());
  }

  GroupCounts.groupCountX = groupCounts[0];
  GroupCounts.groupCountY = groupCounts[1];
  GroupCounts.groupCountZ = groupCounts[2];
  std::copy(groupSizes, groupSizes + 3, GroupSizes);
}

#if INTEL_INTERNAL_BUILD
static void forceGroupSizes(
    uint32_t *GroupSizes, ze_group_count_t &GroupCounts) {
  // Use forced group sizes. This is only for internal experiments, and we
  // don't want to plug these numbers into the decision logic.
  auto userLWS = DeviceInfo->Option.ForcedLocalSizes;
  auto userGWS = DeviceInfo->Option.ForcedGlobalSizes;
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
}
#endif // INTEL_INTERNAL_BUILD

static int32_t runTargetTeamRegion(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit, void *LoopDesc) {
  assert(TgtEntryPtr && "Invalid kernel");
  assert((NumTeams >= 0 && ThreadLimit >= 0) && "Invalid kernel work size");
  DP("Executing a kernel " DPxMOD "...\n", DPxPTR(TgtEntryPtr));

  int32_t RootId = DeviceId;
  int32_t SubId = DeviceId;
  auto SubDeviceCode = DeviceInfo->getSubDeviceCode();

  if (SubDeviceCode < 0 && isValidSubDevice(SubDeviceCode)) {
    uint32_t SubLevel = SUBDEVICE_GET_LEVEL(SubDeviceCode);
    uint32_t SubStart = SUBDEVICE_GET_START(SubDeviceCode);
    RootId = SUBDEVICE_GET_ROOT(SubDeviceCode);
    SubId = DeviceInfo->SubDeviceIds[RootId][SubLevel][SubStart];
  }

  auto *SubIdStr = DeviceInfo->DeviceIdStr[SubId].c_str();
  bool OnRoot = (RootId == SubId);

  ze_kernel_handle_t Kernel = *((ze_kernel_handle_t *)TgtEntryPtr);
  if (!Kernel) {
    REPORT("Failed to invoke deleted kernel.\n");
    return OFFLOAD_FAIL;
  }
  ScopedTimerTy KernelTimer(SubId, "Kernel ",
                            DeviceInfo->KernelProperties[RootId][Kernel].Name);

  // Decide group sizes and counts
  uint32_t GroupSizes[3];
  ze_group_count_t GroupCounts;
  if (LoopDesc) {
    auto RC = decideLoopKernelGroupArguments(SubId, (uint32_t)ThreadLimit,
        (TgtNDRangeDescTy *)LoopDesc, Kernel, GroupSizes, GroupCounts);
    if (RC != OFFLOAD_SUCCESS)
      return OFFLOAD_FAIL;
  } else {
    decideKernelGroupArguments(SubId, (uint32_t )NumTeams,
        (uint32_t)ThreadLimit, Kernel, GroupSizes, GroupCounts);
  }

#if INTEL_INTERNAL_BUILD
  forceGroupSizes(GroupSizes, GroupCounts);
#endif // INTEL_INTERNAL_BUILD

  DP("Group sizes = {%" PRIu32 ", %" PRIu32 ", %" PRIu32 "}\n",
     GroupSizes[0], GroupSizes[1], GroupSizes[2]);
  DP("Group counts = {%" PRIu32 ", %" PRIu32 ", %" PRIu32 "}\n",
     GroupCounts.groupCountX, GroupCounts.groupCountY, GroupCounts.groupCountZ);

  if (OMPT_ENABLED) {
    // Push current work size
    size_t FinalNumTeams = GroupCounts.groupCountX * GroupCounts.groupCountY *
        GroupCounts.groupCountZ;
    size_t FinalThreadLimit = GroupSizes[0] * GroupSizes[1] * GroupSizes[2];
    OmptGlobal->getTrace().pushWorkSize(FinalNumTeams, FinalThreadLimit);
  }

  // Protect from kernel preparation to submission as kernels are shared.
  std::unique_lock<std::mutex> KernelLock(DeviceInfo->KernelMutexes[RootId]);

  // Set arguments
  auto *KernelInfo = DeviceInfo->getKernelInfo(RootId, Kernel);
  for (int32_t I = 0; I < NumArgs; I++) {
    if (KernelInfo && KernelInfo->isArgLiteral(I)) {
      uint32_t Size = KernelInfo->getArgSize(I);
      CALL_ZE_RET_FAIL(zeKernelSetArgumentValue, Kernel, I, Size, TgtArgs[I]);
      DP("Kernel ByVal argument %" PRId32
         " was set successfully for device %s.\n", I, SubIdStr);
    } else if (TgtOffsets[I] == (std::numeric_limits<ptrdiff_t>::max)()) {
      // Offset equal to MAX(ptrdiff_t) means that the argument
      // must be passed as literal, and the offset should be ignored.
      intptr_t Arg = reinterpret_cast<intptr_t>(TgtArgs[I]);
      CALL_ZE_RET_FAIL(zeKernelSetArgumentValue, Kernel, I, sizeof(Arg), &Arg);
      DP("Kernel Scalar argument %" PRId32 " (value: " DPxMOD
         ") was set successfully for device %s.\n", I, DPxPTR(Arg), SubIdStr);
    } else {
      void *Arg = (void *)((intptr_t)TgtArgs[I] + TgtOffsets[I]);
      CALL_ZE_RET_FAIL(zeKernelSetArgumentValue, Kernel, I, sizeof(Arg),
                       Arg == nullptr ? nullptr : &Arg);
      DP("Kernel Pointer argument %" PRId32 " (value: " DPxMOD
         ") was set successfully for device %s.\n", I, DPxPTR(Arg), SubIdStr);
    }
  }

  auto Flags = DeviceInfo->getKernelIndirectAccessFlags(Kernel, RootId);
  // Kernel dynamic memory is also indirect access
  if (DeviceInfo->Option.KernelDynamicMemorySize > 0)
    Flags |= ZE_KERNEL_INDIRECT_ACCESS_FLAG_DEVICE;
  CALL_ZE_RET_FAIL(zeKernelSetIndirectAccess, Kernel, Flags);
  DP("Setting indirect access flags " DPxMOD "\n", DPxPTR(Flags));

  CALL_ZE_RET_FAIL(zeKernelSetGroupSize, Kernel, GroupSizes[0], GroupSizes[1],
                   GroupSizes[2]);

  if (DeviceInfo->Option.CommandBatchLevel > 0) {
    auto &Batch = getTLS()->getCommandBatch();
    if (Batch.isActive())
      return Batch.enqueueLaunchKernel(SubId, Kernel, &GroupCounts, KernelLock);
  }

  ze_command_list_handle_t CmdList = nullptr;
  ze_command_queue_handle_t CmdQueue = nullptr;

  if (DeviceInfo->Option.Flags.UseImmCmdList) {
    CmdList = DeviceInfo->ImmCmdLists[SubId];
    // Command queue is not used with immediate command list
  } else {
    CmdList = DeviceInfo->getCmdList(SubId);
    if (DeviceInfo->Option.Flags.UseMultipleComputeQueues && OnRoot)
      CmdQueue = DeviceInfo->getCCSCmdQueue(RootId);
    else
      CmdQueue = DeviceInfo->getCmdQueue(SubId);
  }

  if (DeviceInfo->BatchCmdQueues[RootId].MaxCommands > 0 && OnRoot) {
    // Enable only for OpenMP device ID
    auto &BatchQueue = DeviceInfo->BatchCmdQueues[RootId];
    BatchQueue.enqueueKernel(Kernel, GroupCounts);
    KernelLock.unlock();
    DP("Appended kernel " DPxMOD " to command list " DPxMOD "\n",
       DPxPTR(Kernel), DPxPTR(BatchQueue.CmdList));
    BatchQueue.run(DeviceInfo->Mutexes[RootId]);
  } else if (DeviceInfo->Option.Flags.UseImmCmdList) {
    // Kernel batching with immediate command list is handled first by the
    // previous branch.
    DP("Using immediate command list for kernel submission.\n");
    auto Event = DeviceInfo->EventPool.getEvent();
    CALL_ZE_RET_FAIL(zeCommandListAppendLaunchKernel, CmdList,
                     Kernel, &GroupCounts, Event, 0, nullptr);
    KernelLock.unlock();
    CALL_ZE_RET_FAIL(zeEventHostSynchronize, Event, UINT64_MAX);
    if (DeviceInfo->Option.Flags.EnableProfile)
      KernelTimer.updateDeviceTime(Event);
    DeviceInfo->EventPool.releaseEvent(Event);
  } else {
    ze_event_handle_t Event = nullptr;
    if (DeviceInfo->Option.Flags.EnableProfile)
      Event = DeviceInfo->EventPool.getEvent();
    CALL_ZE_RET_FAIL(zeCommandListAppendLaunchKernel, CmdList, Kernel,
                     &GroupCounts, Event, 0, nullptr);
    KernelLock.unlock();
    CALL_ZE_RET_FAIL(zeCommandListClose, CmdList);
    LEVEL0_KERNEL_BEGIN(RootId);
    CALL_ZE_RET_FAIL_MTX(zeCommandQueueExecuteCommandLists,
                         DeviceInfo->Mutexes[SubId], CmdQueue, 1, &CmdList,
                         nullptr);
    DP("Submitted kernel " DPxMOD " to device %s\n", DPxPTR(Kernel), SubIdStr);
    CALL_ZE_RET_FAIL(zeCommandQueueSynchronize, CmdQueue, UINT64_MAX);
    CALL_ZE_RET_FAIL(zeCommandListReset, CmdList);
    if (DeviceInfo->Option.Flags.EnableProfile) {
      KernelTimer.updateDeviceTime(Event);
      DeviceInfo->EventPool.releaseEvent(Event);
    }
    LEVEL0_KERNEL_END(RootId);
  }

  DP("Executed kernel entry " DPxMOD " on device %s\n", DPxPTR(TgtEntryPtr),
     SubIdStr);

  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_run_target_team_nd_region(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit, void *LoopDesc) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, NumTeams, ThreadLimit, LoopDesc);
}

EXTERN int32_t __tgt_rtl_run_target_team_region(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit,
    uint64_t LoopTripCount) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, NumTeams, ThreadLimit, nullptr);
}

EXTERN int32_t __tgt_rtl_run_target_team_region_async(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit,
    uint64_t LoopTripCount, __tgt_async_info *AsyncInfoPtr /*not used*/) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, NumTeams, ThreadLimit, nullptr);
}

EXTERN int32_t __tgt_rtl_run_target_region(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, 1, 0, nullptr);
}

EXTERN int32_t __tgt_rtl_run_target_region_async(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, __tgt_async_info *AsyncInfoPtr /*not used*/) {
  return runTargetTeamRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                             NumArgs, 1, 0, nullptr);
}

EXTERN void __tgt_rtl_get_offload_queue(int32_t DeviceId, void *Interop,
       bool CreateNew) {
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

  if (CreateNew) {
     // Create and return a new command queue for interop
     // TODO: check with MKL team and decide what to do with IsAsync
     obj->queue = DeviceInfo->createCommandQueue(deviceId);
     DP("%s returns a new asynchronous command queue " DPxMOD "\n", __func__,
        DPxPTR(obj->queue));
  } else {
     // return a existing command queue for interop
     obj->queue = DeviceInfo->getCmdQueue(deviceId);
     DP("%s returns existing command queue " DPxMOD "\n", __func__,
        DPxPTR(obj->queue));
  }
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
  auto &options = DeviceInfo->Option.UserCompilationOptions;
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

  int32_t RC = omp_irc_success;
  auto &DeviceProperties = DeviceInfo->DeviceProperties[DeviceId];
  auto &ComputeProperties = DeviceInfo->ComputeProperties[DeviceId];
  auto &MemoryProperties = DeviceInfo->MemoryProperties[DeviceId];
  auto &CacheProperties = DeviceInfo->CacheProperties[DeviceId];

  switch (Ipr) {
  case L0Interop::device_num_eus:
    if (ValueType == OMP_IPR_VALUE_INT)
      *static_cast<intptr_t *>(Value) = DeviceProperties.numEUsPerSubslice *
          DeviceProperties.numSubslicesPerSlice *
          DeviceProperties.numSlices;
    else
      RC = omp_irc_type_int;
    break;
  case L0Interop::device_num_threads_per_eu:
    if (ValueType == OMP_IPR_VALUE_INT)
      *static_cast<intptr_t *>(Value) = DeviceProperties.numThreadsPerEU;
    else
      RC = omp_irc_type_int;
    break;
  case L0Interop::device_eu_simd_width:
    if (ValueType == OMP_IPR_VALUE_INT)
      *static_cast<intptr_t *>(Value) = DeviceProperties.physicalEUSimdWidth;
    else
      RC = omp_irc_type_int;
    break;
  case L0Interop::device_num_eus_per_subslice:
    if (ValueType == OMP_IPR_VALUE_INT)
      *static_cast<intptr_t *>(Value) = DeviceProperties.numEUsPerSubslice;
    else
      RC = omp_irc_type_int;
    break;
  case L0Interop::device_num_subslices_per_slice:
    if (ValueType == OMP_IPR_VALUE_INT)
      *static_cast<intptr_t *>(Value) = DeviceProperties.numSubslicesPerSlice;
    else
      RC = omp_irc_type_int;
    break;
  case L0Interop::device_num_slices:
    if (ValueType == OMP_IPR_VALUE_INT)
      *static_cast<intptr_t *>(Value) = DeviceProperties.numSlices;
    else
      RC = omp_irc_type_int;
    break;
  case L0Interop::device_local_mem_size:
    if (ValueType == OMP_IPR_VALUE_INT)
      *static_cast<intptr_t *>(Value) = ComputeProperties.maxSharedLocalMemory;
    else
      RC = omp_irc_type_int;
    break;
  case L0Interop::device_global_mem_size:
    if (ValueType == OMP_IPR_VALUE_INT)
      *static_cast<intptr_t *>(Value) = MemoryProperties.totalSize;
    else
      RC = omp_irc_type_int;
    break;
  case L0Interop::device_global_mem_cache_size:
    if (ValueType == OMP_IPR_VALUE_INT)
      *static_cast<intptr_t *>(Value) = CacheProperties.cacheSize;
    else
      RC = omp_irc_type_int;
    break;
  case L0Interop::device_max_clock_frequency:
    if (ValueType == OMP_IPR_VALUE_INT)
      *static_cast<intptr_t *>(Value) = DeviceProperties.coreClockRate;
    else
      RC = omp_irc_type_int;
    break;
  default:
    RC = omp_irc_out_of_range;
    break;
  }

  return RC;
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
  auto ID = DeviceId;
  if (MemType == ZE_MEMORY_TYPE_HOST)
    ID = DeviceInfo->NumDevices;
  else if (MemType != ZE_MEMORY_TYPE_SHARED)
    return 0;

  if (DeviceInfo->MemAllocInfo[ID]->contains(Ptr, Size))
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
  // -- Allowed batch level is lower than BatchLevel
  if (!DeviceInfo->isDiscreteDevice(DeviceId) ||
      DeviceInfo->Option.CommandBatchLevel < BatchLevel)
    return OFFLOAD_SUCCESS;

  DeviceId = DeviceInfo->getInternalDeviceId(DeviceId);

  return getTLS()->getCommandBatch().begin(DeviceId);
}

EXTERN int32_t __tgt_rtl_command_batch_end(
    int32_t DeviceId, int32_t BatchLevel) {
  // Do not try command batching in these cases
  // -- Integrated devices
  // -- Allowed batch level is lower than BatchLevel
  if (!DeviceInfo->isDiscreteDevice(DeviceId) ||
      DeviceInfo->Option.CommandBatchLevel < BatchLevel)
    return OFFLOAD_SUCCESS;

  return getTLS()->getCommandBatch().end();
}

EXTERN void __tgt_rtl_kernel_batch_begin(int32_t DeviceId,
                                         uint32_t MaxKernels) {
  DeviceInfo->beginKernelBatch(DeviceId, MaxKernels);
}

EXTERN void __tgt_rtl_kernel_batch_end(int32_t DeviceId) {
  DeviceInfo->endKernelBatch(DeviceId);
}

EXTERN int32_t __tgt_rtl_set_function_ptr_map(
    int32_t DeviceId, uint64_t Size,
    const __omp_offloading_fptr_map_t *FnPtrs) {
  if (Size == 0)
    return OFFLOAD_SUCCESS;

  ScopedTimerTy Timer(DeviceId, "Function pointers init");
  // FIXME: What happens if we have multiple programs?
  auto &Program = DeviceInfo->Programs[DeviceId].back();
  void *DeviceMapSizeVarAddr = Program.getVarDeviceAddr(
      "__omp_offloading_fptr_map_size", sizeof(uint64_t));

  // getVarDeviceAddr() will return the device pointer size
  // in DeviceMapPtrVarSize.
  size_t DeviceMapPtrVarSize = 0;
  void *DeviceMapPtrVarAddr = Program.getVarDeviceAddr(
      "__omp_offloading_fptr_map_p", &DeviceMapPtrVarSize);
  if (!DeviceMapSizeVarAddr || !DeviceMapPtrVarAddr)
    return OFFLOAD_FAIL;

  // Allocate memory for the function pointers map on the device,
  // and transfer the host map to the allocated memory.
  size_t FnPtrMapSizeInBytes = Size * sizeof(__omp_offloading_fptr_map_t);
  void *FnPtrMapMem = allocDataExplicit(DeviceId, FnPtrMapSizeInBytes,
                                        DeviceInfo->AllocKinds[DeviceId]);
  if (!FnPtrMapMem)
    return OFFLOAD_FAIL;

  DeviceInfo->DataMutexes[DeviceId].lock();
  DeviceInfo->OwnedMemory[DeviceId].push_back(FnPtrMapMem);
  DeviceInfo->DataMutexes[DeviceId].unlock();

  if (DebugLevel >= 2) {
    DP("Transferring function pointers table (%" PRIu64
       " entries) to the device: {\n", Size);
    // Limit the number of printed entries with (DebugLevel * 5).
    uint64_t PrintEntriesNum =
        std::min<uint64_t>(Size, static_cast<uint64_t>(DebugLevel) * 5);
    for (uint64_t I = 0; I < PrintEntriesNum; ++I) {
      DP("\t{ " DPxMOD ", " DPxMOD " }\n",
         DPxPTR(FnPtrs[I].host_ptr), DPxPTR(FnPtrs[I].tgt_ptr));
    }
    if (PrintEntriesNum < Size)
      DP("\t... increase LIBOMPTARGET_DEBUG to see more entries ...\n");
    DP("}\n");
  }

  if (DeviceInfo->enqueueMemCopy(
          DeviceId, FnPtrMapMem, FnPtrs, FnPtrMapSizeInBytes) !=
      OFFLOAD_SUCCESS)
    return OFFLOAD_FAIL;

  // Initialize __omp_offloading_fptr_map_p global with the value of
  // FnPtrMapMem.
  if (DeviceMapPtrVarSize != sizeof(void *)) {
    // Device pointer size is different from the host pointer size.
    // This is worth to mention, but the address transfer below
    // should be correct (given that __omp_offloading_fptr_map_p is
    // nullptr initially).
    DP("Warning: device pointer size is %zu, host pointer size is %zu.\n",
       DeviceMapPtrVarSize, static_cast<size_t>(sizeof(void *)));
  }
  size_t PtrTransferSize =
      std::min<size_t>(DeviceMapPtrVarSize, sizeof(void *));
  if (DeviceInfo->enqueueMemCopy(
          DeviceId, DeviceMapPtrVarAddr, &FnPtrMapMem, PtrTransferSize) !=
      OFFLOAD_SUCCESS)
    return OFFLOAD_FAIL;

  // Initialize __omp_offloading_fptr_map_size with the table size.
  if (DeviceInfo->enqueueMemCopy(
          DeviceId, DeviceMapSizeVarAddr, &Size, sizeof(uint64_t)) !=
      OFFLOAD_SUCCESS)
    return OFFLOAD_FAIL;

  return OFFLOAD_SUCCESS;
}

EXTERN void *__tgt_rtl_alloc_per_hw_thread_scratch(
    int32_t DeviceId, size_t ObjSize, int32_t AllocKind) {
  void *Mem = nullptr;
  auto &P = DeviceInfo->DeviceProperties[DeviceId];
  uint32_t NumHWThreads = P.numThreadsPerEU * P.numEUsPerSubslice *
      P.numSubslicesPerSlice * P.numSlices;
  size_t AllocSize = ObjSize * NumHWThreads;
  if (AllocKind == TARGET_ALLOC_DEFAULT)
    AllocKind = TARGET_ALLOC_DEVICE;

  // Use common pool for now. We can consider using a dedicated pool if desired.
  if (DeviceInfo->Option.Flags.UseMemoryPool)
    Mem = DeviceInfo->poolAlloc(DeviceId, AllocSize, AllocKind);

  bool InPool = false;
  if (Mem)
    InPool = true;
  else
    Mem = allocDataExplicit(DeviceId, ObjSize * NumHWThreads, AllocKind);

  if (Mem) {
    DeviceInfo->MemAllocInfo[DeviceId]->add(
        Mem, Mem, AllocSize, AllocKind, InPool);
    DP("Allocated %zu byte per-hw-thread scratch space at " DPxMOD "\n",
       AllocSize, DPxPTR(Mem));
  }

  return Mem;
}

EXTERN void __tgt_rtl_free_per_hw_thread_scratch(int32_t DeviceId, void *Ptr) {
  DeviceInfo->dataDelete(DeviceId, Ptr);
}

int32_t __tgt_rtl_supports_empty_images() { return 1; }

bool RTLDeviceInfoTy::isExtensionSupported(const char *ExtName) {
  for (auto &E : DriverExtensions) {
    std::string Supported(E.name);
    if (Supported.find(ExtName) != std::string::npos)
      return true;
  }
  return false;
}

void RTLDeviceInfoTy::beginKernelBatch(int32_t DeviceId, uint32_t MaxKernels) {
  auto &Batch = BatchCmdQueues[DeviceId];
  Batch.MaxCommands = MaxKernels;
  Batch.UseImmCmdList = Option.Flags.UseImmCmdList;
  if (Batch.CmdList != nullptr)
    return;

  // Requires initialization
  if (Batch.UseImmCmdList) {
    ze_command_queue_desc_t QueueDesc = {
      ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC, nullptr,
      CmdQueueGroupOrdinals[DeviceId], 0, 0,
      ZE_COMMAND_QUEUE_MODE_ASYNCHRONOUS, ZE_COMMAND_QUEUE_PRIORITY_NORMAL,
    };
    CALL_ZE_RET_VOID(zeCommandListCreateImmediate, Context, Devices[DeviceId],
                     &QueueDesc, &Batch.CmdList);

    // Event pool with a single event needs to be initialized
    ze_event_pool_desc_t PoolDesc = {
      ZE_STRUCTURE_TYPE_EVENT_POOL_DESC, nullptr,
      ZE_EVENT_POOL_FLAG_HOST_VISIBLE, 1,
    };
    ze_event_desc_t EventDesc = {
      ZE_STRUCTURE_TYPE_EVENT_DESC, nullptr, 0, 0, 0
    };
    CALL_ZE_RET_VOID(zeEventPoolCreate, Context, &PoolDesc, 0, nullptr,
                     &Batch.EventPool);
    CALL_ZE_RET_VOID(zeEventCreate, Batch.EventPool, &EventDesc, &Batch.Event);
    DP("Initialized kernel batching with IMM.\n");
  } else {
    Batch.CmdList = createCmdList(Context, Devices[DeviceId],
                                  CmdQueueGroupOrdinals[DeviceId],
                                  DeviceIdStr[DeviceId]);
    Batch.CmdQueue = createCommandQueue(DeviceId);
    DP("Initialized kernel batching.\n");
  }
}

void RTLDeviceInfoTy::endKernelBatch(int32_t DeviceId) {
  // Just reset allowed number of commands.
  // Allocated resources will be reused and released later.
  BatchCmdQueues[DeviceId].MaxCommands = 0;
  assert(BatchCmdQueues[DeviceId].NumCommands == 0 &&
         "Kernel batch queue has incomplete commands.");
}

void RTLDeviceInfoTy::initImmCmdList(int32_t DeviceId) {
  // Initialize immediate command list
  ze_command_queue_desc_t QueueDesc = {
    ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC, nullptr,
    CmdQueueGroupOrdinals[DeviceId], 0, 0,
    ZE_COMMAND_QUEUE_MODE_ASYNCHRONOUS, ZE_COMMAND_QUEUE_PRIORITY_NORMAL,
  };
  ze_command_list_handle_t CmdList;
  CALL_ZE_RET_VOID(zeCommandListCreateImmediate, Context, Devices[DeviceId],
                   &QueueDesc, &CmdList);
  ImmCmdLists[DeviceId] = CmdList;
  // For subdevices
  for (auto &SubLevel : SubDeviceIds[DeviceId]) {
    for (auto SubId : SubLevel) {
      QueueDesc.ordinal = CmdQueueGroupOrdinals[SubId];
      QueueDesc.index = CmdQueueIndices[SubId];
      CALL_ZE_RET_VOID(zeCommandListCreateImmediate, Context, Devices[SubId],
                       &QueueDesc, &CmdList);
      ImmCmdLists[SubId] = CmdList;
    }
  }
}

/// Return the internal device ID for the specified subdevice
int32_t RTLDeviceInfoTy::getSubDeviceId(int32_t DeviceId, uint32_t Level,
                                        uint32_t SubId) {
  if (SubDeviceIds[DeviceId].size() > Level &&
      SubDeviceIds[DeviceId][Level].size() > SubId)
    return SubDeviceIds[DeviceId][Level][SubId];
  else
    return -1;
}

/// Check if the device has access to copy engines (either main or link engines)
bool RTLDeviceInfoTy::hasCopyEngineAccess(int32_t DeviceId, bool IsMain) {
  if (IsMain)
    return (CopyCmdQueueGroupOrdinals[DeviceId] != UINT32_MAX);
  else
    return (LinkCopyCmdQueueGroupOrdinals[DeviceId].second > 0);
}

int32_t RTLDeviceInfoTy::findDevices() {
  DP("Looking for Level0 devices...\n");

  if (Option.DeviceType != ZE_DEVICE_TYPE_GPU) {
    DP("Only GPU device is supported\n");
    return 0;
  }

  CALL_ZE_RET_ZERO(zeInit, ZE_INIT_FLAG_GPU_ONLY);

  uint32_t NumDrivers = 0;
  CALL_ZE_RET_ZERO(zeDriverGet, &NumDrivers, nullptr);
  if (NumDrivers == 0) {
    DP("Cannot find any drivers.\n");
    return 0;
  }

  // We will use the first driver found
  NumDrivers = 1;
  CALL_ZE_RET_ZERO(zeDriverGet, &NumDrivers, &Driver);

  uint32_t NumFoundDevices = 0;
  std::vector<ze_device_handle_t> RootDevices;
  CALL_ZE_RET_ZERO(zeDeviceGet, Driver, &NumFoundDevices, nullptr);
  if (NumFoundDevices == 0) {
    DP("Cannot find any devices.\n");
    return 0;
  }
  RootDevices.resize(NumFoundDevices);
  CALL_ZE_RET_ZERO(zeDeviceGet, Driver, &NumFoundDevices, RootDevices.data());

  // Find minimal information to initialize device properties.
  // List of device handle, root ID, sub ID, CCS ID
  std::list<std::tuple<ze_device_handle_t, int32_t, int32_t, int32_t>> Tuples;

  for (uint32_t I = 0; I < NumFoundDevices; I++) {
    Tuples.emplace_back(RootDevices[I], I, -1, -1);
    // Try to find subdevices.
    uint32_t NumSub = 0;
    std::vector<ze_device_handle_t> SubDevices;
    CALL_ZE_RET_ZERO(zeDeviceGetSubDevices, RootDevices[I], &NumSub, nullptr);
    if (NumSub == 0) {
      // Try to find CCS directly in this case
      SubDevices.push_back(RootDevices[I]);
    } else {
      SubDevices.resize(NumSub);
      CALL_ZE_RET_ZERO(zeDeviceGetSubDevices, RootDevices[I], &NumSub,
                       SubDevices.data());
    }
    for (uint32_t J = 0; J < SubDevices.size(); J++) {
      if (NumSub > 0)
        Tuples.emplace_back(SubDevices[J], I, J, -1);
      uint32_t NumCCS = 0;
      (void)getCmdQueueGroupOrdinalCCS(SubDevices[J], NumCCS);
      for (uint32_t K = 0; K < NumCCS; K++)
        Tuples.emplace_back(SubDevices[J], I, J, K);
    }
  }

  auto getIdStr = [](int32_t RootID, int32_t SubID, int32_t CCSID) {
    std::string Ret;
    std::string Sep{"."};
    if (RootID >= 0)
      Ret += std::to_string(RootID);
    if (SubID >= 0)
      Ret += Sep + std::to_string(SubID);
    if (CCSID >= 0)
      Ret += Sep + std::to_string(CCSID);
    return Ret;
  };

  // Initialize device properties respecting LIBOMPTARGET_DEVICES.
  // Fill internal data using the list of subdevice handles.
  // Internally, all devices/subdevices are listed as follows for N devices
  // where Subdevices(i,j) is a list of subdevices for device i at level j.
  // [0..N-1][Subdevices(0,0),Subdevices(0,1)]..[Subdevices(N-1,0)..]
  // Recursive subdevice query is not supported, so use existing query only
  // for the first-level subdevice, and use multi-context queue/list for the
  // second-level subdevice.
  bool SupportsClause = (Option.DeviceMode == DEVICE_MODE_TOP);
  if (SupportsClause) {
    for (auto &T : Tuples) {
      if (std::get<2>(T) >= 0 || std::get<3>(T) >= 0)
        continue;
      SubDeviceIds.emplace_back(2); // Prepare for subdevice clause support
      auto IdStr = getIdStr(std::get<1>(T), std::get<2>(T), std::get<3>(T));
      appendDeviceProperties(std::get<0>(T), IdStr);
    }
  }
  for (int32_t I = 0; I < (int)NumFoundDevices; I++) {
    if (Option.DeviceMode != DEVICE_MODE_SUBSUB) {
      // Initialize first-level subdevices properties
      for (auto &T : Tuples) {
        if (std::get<1>(T) != I || std::get<2>(T) < 0 || std::get<3>(T) >= 0)
          continue;
        if (SupportsClause)
          SubDeviceIds[I][0].push_back(Devices.size());
        auto IdStr = getIdStr(std::get<1>(T), std::get<2>(T), std::get<3>(T));
        SubDeviceIds.emplace_back(); // Put empty list for subdevices
        appendDeviceProperties(std::get<0>(T), IdStr);
      }
    }
    if (Option.DeviceMode != DEVICE_MODE_SUB) {
      // Initialize second-level subdevice properties
      for (auto &T : Tuples) {
        if (std::get<1>(T) != I || std::get<3>(T) < 0)
          continue;
        if (SupportsClause)
          SubDeviceIds[I][1].push_back(Devices.size());
        auto IdStr = getIdStr(std::get<1>(T), std::get<2>(T), std::get<3>(T));
        SubDeviceIds.emplace_back(); // Put empty list for subdevices
        appendDeviceProperties(std::get<0>(T), IdStr, std::get<3>(T));
      }
    }
  }

  NumDevices = Devices.size();
  NumRootDevices = SupportsClause ? NumFoundDevices : NumDevices;

  DP("Found %" PRIu32 " root devices, %" PRIu32 " total devices.\n",
     NumRootDevices, NumDevices);
  DP("List of devices (DeviceID[.SubID[.CCSID]])\n");
  for (auto &Str : DeviceIdStr)
    DP("-- %s\n", Str.c_str());

  // Prepare space for internal data
  Programs.resize(NumDevices);
  KernelProperties.resize(NumDevices);
  OwnedMemory.resize(NumDevices);
  Initialized.resize(NumDevices);
  Profiles.resize(NumDevices);
  Context = createContext(Driver);
  NumActiveKernels.resize(NumRootDevices, 0);
  ProgramData.resize(NumRootDevices);
  BatchCmdQueues.resize(NumRootDevices);
  ImmCmdLists.resize(NumDevices);
  Mutexes.reset(new std::mutex[NumDevices]);
  DataMutexes.reset(new std::mutex[NumDevices]);
  KernelMutexes.reset(new std::mutex[NumDevices]);

  // Common event pool
  uint32_t EventFlag = 0;
  if (Option.Flags.EnableProfile)
    EventFlag = ZE_EVENT_POOL_FLAG_KERNEL_TIMESTAMP;
  EventPool.init(Context, EventFlag);

  // Supported API version
  CALL_ZE_RET_ZERO(zeDriverGetApiVersion, Driver, &DriverAPIVersion);
  DP("Driver API version is %" PRIx32 "\n", DriverAPIVersion);

  // Supported interop properties
  L0Interop::printInteropProperties();

  // Check driver extensions.
  uint32_t NumExtensions = 0;
  CALL_ZE_RET_ZERO(zeDriverGetExtensionProperties, Driver,
                   &NumExtensions, nullptr);
  if (NumExtensions > 0) {
    auto &Extensions = DriverExtensions;
    Extensions.resize(NumExtensions);
    CALL_ZE_RET_ZERO(zeDriverGetExtensionProperties, Driver,
                     &NumExtensions, Extensions.data());
    DP("Found driver extensions:\n");
    for (auto &E : Extensions)
      DP("-- %s\n", E.name);
  }

  // Host allocation information needs one additional slot
  for (uint32_t I = 0; I < NumDevices + 1; I++)
    MemAllocInfo.emplace_back(new MemAllocInfoMapTy());

  if (DebugLevel > 0)
    initMemoryStat();

  // Look up GITS notification function
  ze_result_t Rc;
  CALL_ZE(Rc, zeDriverGetExtensionFunctionAddress, Driver,
          "zeGitsIndirectAllocationOffsets", &GitsIndirectAllocationOffsets);
  if (Rc != ZE_RESULT_SUCCESS)
    GitsIndirectAllocationOffsets = nullptr;

  return NumRootDevices;
}

LevelZeroProgramTy::~LevelZeroProgramTy() {
  for (auto Kernel : Kernels) {
    if (Kernel)
      CALL_ZE_RET_VOID(zeKernelDestroy, Kernel);
  }
  for (auto Module : Modules) {
    CALL_ZE_RET_VOID(zeModuleDestroy, Module);
  }
  // Unload offload entries
  for (auto &Entry : OffloadEntries)
    delete[] Entry.Base.name;
}

int32_t LevelZeroProgramTy::addModule(
    size_t Size, const uint8_t *Image, const std::string &CommonBuildOptions,
    ze_module_format_t Format) {
  ze_module_constants_t SpecConstants =
      DeviceInfo->Option.CommonSpecConstants.getModuleConstants();
  std::string BuildOptions(CommonBuildOptions);
  // Add required flag to enable dynamic linking. We can do this only if the
  // module does not contain any kernels or globals.
  // FIXME: module build with "-library-compilation" does not work on iGPU now.
  // Keep the device check until XDEPS-3954 is resolved.
  if (IsLibModule && DeviceInfo->isDiscreteDevice(DeviceId))
    BuildOptions += " -library-compilation ";

  ze_module_desc_t ModuleDesc{ZE_STRUCTURE_TYPE_MODULE_DESC, nullptr, Format};

  if (DeviceInfo->Option.Flags.LinkLibDevice &&
      Format == ZE_MODULE_FORMAT_IL_SPIRV && Modules.size() == 0) {
    // Handle link libdevice option. Do this only for the first moudle build

    // Check if driver is capable of creating module from multiple SPV images.
    if (!DeviceInfo->isExtensionSupported("ZE_experimental_module_program")) {
      DP("Error: Module creation from multiple images is not supported\n");
      return OFFLOAD_FAIL;
    }
    std::vector<const char *> LibDeviceNames {
      "libomp-fallback-cassert.spv",
      "libomp-fallback-cmath.spv",
      "libomp-fallback-cmath-fp64.spv",
      "libomp-fallback-complex.spv",
      "libomp-fallback-complex-fp64.spv",
      "libomp-fallback-cstring.spv"
    };

    uint32_t NumImages = LibDeviceNames.size() + 1;
    std::vector<std::vector<uint8_t>> LibImages;
    std::vector<size_t> SPVSizes;
    std::vector<const uint8_t *> SPVImages;
    std::vector<const char *> SPVFlags(NumImages, BuildOptions.c_str());
    std::vector<const ze_module_constants_t *> SPVConstants(NumImages, nullptr);

    // Main program
    SPVSizes.push_back(Size);
    SPVImages.push_back(Image);
    SPVConstants[0] = &SpecConstants;

    // Read Library SPV files
    for (auto *Name : LibDeviceNames) {
      LibImages.emplace_back();
      if (readSPVFile(Name, LibImages.back()) != OFFLOAD_SUCCESS)
        return OFFLOAD_FAIL;
      SPVSizes.push_back(LibImages.back().size());
      SPVImages.push_back(LibImages.back().data());
    }

    DP("Building module with %" PRIu32 " SPIR-V images\n", NumImages);

    ze_module_program_exp_desc_t ProgramDesc = {
      ZE_STRUCTURE_TYPE_MODULE_PROGRAM_EXP_DESC,
      nullptr,
      NumImages,
      SPVSizes.data(),
      SPVImages.data(),
      SPVFlags.data(),
      SPVConstants.data()
    };
    ModuleDesc.pNext = &ProgramDesc;
    ModuleDesc.inputSize = 0;
    ModuleDesc.pInputModule = nullptr;
    ModuleDesc.pBuildFlags = nullptr;
    ModuleDesc.pConstants = nullptr;
  } else {
    // Build a single module from a single image
    ModuleDesc.inputSize = Size;
    ModuleDesc.pInputModule = Image;
    ModuleDesc.pBuildFlags = BuildOptions.c_str();
    ModuleDesc.pConstants = &SpecConstants;
  }

  ze_module_handle_t Module = nullptr;
  ze_module_build_log_handle_t BuildLog = nullptr;
  ze_result_t RC;
  CALL_ZE_RC(RC, zeModuleCreate, Context, Device, &ModuleDesc, &Module,
             &BuildLog);

  bool BuildFailed = (RC != ZE_RESULT_SUCCESS);
  bool ShowBuildLog = DeviceInfo->Option.Flags.ShowBuildLog;
  if (BuildFailed || ShowBuildLog) {
    if (BuildFailed)
      DP("Error: module creation failed\n");
    if (DebugLevel > 0 || ShowBuildLog) {
      MESSAGE0("Target build log:");
      size_t LogSize = 0;
      CALL_ZE_RET_FAIL(zeModuleBuildLogGetString, BuildLog, &LogSize, nullptr);
      if (LogSize > 1) {
        std::vector<char> LogString(LogSize);
        CALL_ZE_RET_FAIL(zeModuleBuildLogGetString, BuildLog, &LogSize,
                         LogString.data());
        std::stringstream Str(LogString.data());
        std::string Line;
        while (std::getline(Str, Line, '\n'))
          MESSAGE("  '%s'", Line.c_str());
      } else {
        MESSAGE0("  <empty>");
      }
    }
  }
  CALL_ZE_RET_FAIL(zeModuleBuildLogDestroy, BuildLog);

  if (BuildFailed) {
    return OFFLOAD_FAIL;
  } else {
    // Check if module link is required
    if (!RequiresModuleLink) {
      ze_module_properties_t Properties = {
        ZE_STRUCTURE_TYPE_MODULE_PROPERTIES, nullptr, 0
      };
      CALL_ZE_RET_FAIL(zeModuleGetProperties, Module, &Properties);
      RequiresModuleLink = Properties.flags & ZE_MODULE_PROPERTY_FLAG_IMPORTS;
    }
    // For now, assume the first module contains libraries, globals.
    if (Modules.empty())
      GlobalModule = Module;
    Modules.push_back(Module);
    DeviceInfo->GlobalModules.push_back(Module);
    return OFFLOAD_SUCCESS;
  }
}

int32_t LevelZeroProgramTy::linkModules() {
  ScopedTimerTy MoudleLinkTimer(DeviceId, "Linking");

  if (!RequiresModuleLink) {
    DP("Module link is not required\n");
    return OFFLOAD_SUCCESS;
  }

  if (Modules.empty()) {
    DP("Invalid number of modules when linking modules\n");
    return OFFLOAD_FAIL;
  }

  ze_result_t RC;
  ze_module_build_log_handle_t LinkLog = nullptr;
  auto &AllModules = DeviceInfo->GlobalModules;
  CALL_ZE_RC(RC, zeModuleDynamicLink, (uint32_t)AllModules.size(),
             AllModules.data(), &LinkLog);
  bool LinkFailed = (RC != ZE_RESULT_SUCCESS);
  bool ShowBuildLog = DeviceInfo->Option.Flags.ShowBuildLog;

  if (LinkFailed || ShowBuildLog) {
    if (LinkFailed)
      DP("Error: module link failed\n");
    if (DebugLevel > 0 || ShowBuildLog) {
      MESSAGE0("Target link log:");
      size_t LogSize = 0;
      CALL_ZE_RET_FAIL(zeModuleBuildLogGetString, LinkLog, &LogSize, nullptr);
      if (LogSize > 1) {
        std::vector<char> LogString(LogSize);
        CALL_ZE_RET_FAIL(zeModuleBuildLogGetString, LinkLog, &LogSize,
                         LogString.data());
        std::stringstream Str(LogString.data());
        std::string Line;
        while (std::getline(Str, Line, '\n'))
          MESSAGE("  '%s'", Line.c_str());
      } else {
        MESSAGE0("  <empty>");
      }
    }
  }
  CALL_ZE_RET_FAIL(zeModuleBuildLogDestroy, LinkLog);

  if (LinkFailed)
    return OFFLOAD_FAIL;
  else
    return OFFLOAD_SUCCESS;
}

int32_t LevelZeroProgramTy::buildModules(std::string &BuildOptions) {
  ScopedTimerTy ModuleBuildTimer(DeviceId, "Compiling");
  uint64_t MajorVer, MinorVer;
  if (!isValidOneOmpImage(Image, MajorVer, MinorVer)) {
    // Handle legacy plain SPIR-V image.
    uint8_t *ImgBegin = reinterpret_cast<uint8_t *>(Image->ImageStart);
    uint8_t *ImgEnd = reinterpret_cast<uint8_t *>(Image->ImageEnd);
    size_t ImgSize = ImgEnd - ImgBegin;
    dumpImageToFile(ImgBegin, ImgSize, "OpenMP");
    return addModule(ImgSize, ImgBegin, BuildOptions,
                     ZE_MODULE_FORMAT_IL_SPIRV);
  }

  // Check if the program only contains libraries
  size_t NumEntries = (size_t)(Image->EntriesEnd - Image->EntriesBegin);
  IsLibModule = (NumEntries == 0);

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
    // We may have multiple sections created from split-kernel mode
    std::vector<const uint8_t *> PartBegin;
    std::vector<uint64_t> PartSize;

    V1ImageInfo(uint64_t Format, std::string CompileOpts, std::string LinkOpts)
      : Format(Format), CompileOpts(CompileOpts), LinkOpts(LinkOpts) {}
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
                                            Parts[2], Parts[3]));
                                            // Image pointer and size
                                            // will be initialized later.
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

    // Expected section name in split-kernel mode:
    // __openmp_offload_spirv_<image_id>_<part_id>
    auto PartIdLoc = SectionName.find("_");
    if (PartIdLoc != std::string::npos) {
      DP("Found a split section in the image\n");
      // It seems that we do not need part ID as long as they are ordered
      // in the image and we keep the ordering in the runtime.
      SectionName.erase(PartIdLoc);
    } else {
      DP("Found a single section in the image\n");
    }

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

    AuxInfoIt->second.PartBegin.push_back((*I).getContents());
    AuxInfoIt->second.PartSize.push_back((*I).getSize());
  }

  for (uint64_t Idx = 0; Idx < ImageCount; ++Idx) {
    auto It = AuxInfo.find(Idx);
    if (It == AuxInfo.end()) {
      DP("Warning: image %" PRIu64
         " without auxiliary information is ingored.\n", Idx);
      continue;
    }

    auto NumParts = It->second.PartBegin.size();
    // Split-kernel is not supported in SPIRV format
    if (NumParts > 1 && It->second.Format != 0) {
      DP("Warning: split-kernel images are not supported in SPIRV format\n");
      continue;
    }

    // Skip unknown image format
    if (It->second.Format != 0 && It->second.Format != 1) {
      DP("Warning: image %" PRIu64 "is ignored due to unknown format.\n", Idx);
      continue;
    }

    bool IsBinary = (It->second.Format == 0);
    auto ModuleFormat =
        IsBinary ? ZE_MODULE_FORMAT_NATIVE : ZE_MODULE_FORMAT_IL_SPIRV;
    std::string Options = BuildOptions;
    if (DeviceInfo->Option.Flags.UseImageOptions)
      Options += " " + It->second.CompileOpts + " " + It->second.LinkOpts;

    for (size_t I = 0; I < NumParts; I++) {
      const unsigned char *ImgBegin =
          reinterpret_cast<const unsigned char *>(It->second.PartBegin[I]);
      size_t ImgSize = It->second.PartSize[I];
      dumpImageToFile(ImgBegin, ImgSize, "OpenMP");

      auto RC = addModule(ImgSize, ImgBegin, Options, ModuleFormat);

      if (RC != OFFLOAD_SUCCESS) {
        DP("Error: failed to create program from %s " "(%" PRIu64 "-%zu).\n",
           IsBinary ? "Binary" : "SPIR-V", Idx, I);
        return OFFLOAD_FAIL;
      }
    }

    DP("Created module from image #%" PRIu64 ".\n", Idx);
    BuildOptions = Options;

    return OFFLOAD_SUCCESS;
  }

  return OFFLOAD_FAIL;
}

void *LevelZeroProgramTy::getVarDeviceAddr(const char *Name, size_t *SizePtr) {
  if (!Name || !SizePtr)
    return nullptr;

  void *TgtAddr = nullptr;
  size_t TgtSize = 0;
  size_t Size = *SizePtr;
  bool SizeIsKnown = (Size != 0);

  if (SizeIsKnown) {
    DP("Looking up device global variable '%s' of size %zu bytes "
       "on device %d.\n", Name, Size, DeviceId);
  } else {
    DP("Looking up device global variable '%s' of unknown size "
       "on device %d.\n", Name, DeviceId);
  }
  CALL_ZE_RET_NULL(zeModuleGetGlobalPointer, GlobalModule, Name, &TgtSize,
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

void *LevelZeroProgramTy::getVarDeviceAddr(const char *Name, size_t Size) {
  return getVarDeviceAddr(Name, &Size);
}

void *LevelZeroProgramTy::getOffloadVarDeviceAddr(
    const char *Name, size_t Size) {
  DP("Looking up OpenMP global variable '%s' of size %zu bytes.\n", Name, Size);

  if (!OffloadEntries.empty()) {
    size_t NameSize = strlen(Name) + 1;
    auto I = std::lower_bound(
        OffloadEntries.begin(), OffloadEntries.end(), Name,
        [NameSize](const DeviceOffloadEntryTy &E, const char *Name) {
          return strncmp(E.Base.name, Name, NameSize) < 0;
        });

    if (I != OffloadEntries.end() &&
        strncmp(I->Base.name, Name, NameSize) == 0) {
      DP("Global variable '%s' found in the offload table at position %zu.\n",
         Name, std::distance(OffloadEntries.begin(), I));
      return I->Base.addr;
    }

    DP("Warning: global variable '%s' was not found in the offload table.\n",
       Name);
  } else {
    DP("Warning: offload table is not loaded for device %d.\n", DeviceId);
  }

  // Fallback to the lookup by name.
  return getVarDeviceAddr(Name, Size);
}

bool LevelZeroProgramTy::loadOffloadTable(size_t NumEntries) {
  ScopedTimerTy OffloadTableInitTimer(DeviceId, "OffloadEntriesInit");

  const char *OffloadTableSizeVarName = "__omp_offloading_entries_table_size";
  void *OffloadTableSizeVarAddr =
      getVarDeviceAddr(OffloadTableSizeVarName, sizeof(int64_t));

  if (!OffloadTableSizeVarAddr) {
    DP("Warning: cannot get device value for global variable '%s'.\n",
       OffloadTableSizeVarName);
    return false;
  }

  int64_t TableSizeVal = 0;
  auto RC = DeviceInfo->enqueueMemCopy(
      DeviceId, &TableSizeVal, OffloadTableSizeVarAddr, sizeof(int64_t));
  if (RC != OFFLOAD_SUCCESS)
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
      getVarDeviceAddr(OffloadTableVarName, TableSize);
  if (!OffloadTableVarAddr) {
    DP("Warning: cannot get device value for global variable '%s'.\n",
       OffloadTableVarName);
    return false;
  }

  OffloadEntries.resize(DeviceNumEntries);
  RC = DeviceInfo->enqueueMemCopy(DeviceId, OffloadEntries.data(),
                                  OffloadTableVarAddr, TableSize);
  if (RC != OFFLOAD_SUCCESS)
    return false;

  size_t I = 0;
  const char *PreviousName = "";
  bool PreviousIsVar = false;

  for (; I < DeviceNumEntries; ++I) {
    DeviceOffloadEntryTy &Entry = OffloadEntries[I];
    size_t NameSize = Entry.NameSize;
    void *NameTgtAddr = Entry.Base.name;
    Entry.Base.name = nullptr;

    if (NameSize == 0) {
      DP("Warning: offload entry (%zu) with 0 name size.\n", I);
      break;
    }
    if (NameTgtAddr == nullptr) {
      DP("Warning: offload entry (%zu) with invalid name.\n", I);
      break;
    }

    Entry.Base.name = new char[NameSize];
    RC = DeviceInfo->enqueueMemCopy(DeviceId, Entry.Base.name, NameTgtAddr,
                                    NameSize);
    if (RC != OFFLOAD_SUCCESS)
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
    } else if (Cmp == 0 && (PreviousIsVar || Entry.Base.size)) {
      // The names are equal. This should never happen for
      // offload variables, but we allow this for offload functions.
      DP("Warning: duplicate names (%s) in offload table.\n", PreviousName);
      break;
    }
    PreviousName = Entry.Base.name;
    PreviousIsVar = (Entry.Base.size != 0);
  }

  if (I != DeviceNumEntries) {
    // Errors during the table processing.
    // Deallocate all memory allocated in the loop.
    for (size_t J = 0; J <= I; ++J) {
      DeviceOffloadEntryTy &Entry = OffloadEntries[J];
      if (Entry.Base.name)
        delete[] Entry.Base.name;
    }

    OffloadEntries.clear();
    return false;
  }

  if (DebugLevel > 0) {
    DP("Device offload table loaded:\n");
    for (size_t I = 0; I < DeviceNumEntries; ++I)
      DP("\t%zu:\t%s\n", I, OffloadEntries[I].Base.name);
  }

  return true;
}

bool LevelZeroProgramTy::readKernelInfo(
    const __tgt_offload_entry &KernelEntry) {
  const ze_kernel_handle_t *KernelPtr =
      reinterpret_cast<const ze_kernel_handle_t *>(KernelEntry.addr);
  const char *Name = KernelEntry.name;
  std::string InfoVarName(Name);
  InfoVarName += "_kernel_info";
  size_t InfoVarSize = 0;
  void *InfoVarAddr = getVarDeviceAddr(InfoVarName.c_str(), &InfoVarSize);
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
  auto RC = DeviceInfo->enqueueMemCopy(DeviceId, InfoBuffer.data(), InfoVarAddr,
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
  if (Version > 4) {
    DP("Error: unsupported version (%" PRIu32 ") of kernel info structure.\n",
       Version);
    DP("Error: please use newer OpenMP offload runtime.\n");
    return false;
  }
  ReadPtr += 4;
  uint32_t KernelArgsNum = llvm::support::endian::read32le(ReadPtr);
  size_t ExpectedInfoVarSize = static_cast<size_t>(KernelArgsNum) * 8 + 8;
  // Support Attributes1 since version 2.
  if (Version > 1)
    ExpectedInfoVarSize += 8;
  // Support WGNum since version 3.
  if (Version > 2)
    ExpectedInfoVarSize += 8;
  if (Version > 3)
    ExpectedInfoVarSize += 8;
  if (InfoVarSize != ExpectedInfoVarSize) {
    DP("Error: expected kernel info variable size %zu - got %zu\n",
       ExpectedInfoVarSize, InfoVarSize);
    return false;
  }
  KernelInfoTy Info(Version);
  ReadPtr += 4;
  for (uint64_t I = 0; I < KernelArgsNum; ++I) {
    bool ArgIsLiteral = (llvm::support::endian::read32le(ReadPtr) != 0);
    ReadPtr += 4;
    uint32_t ArgSize = llvm::support::endian::read32le(ReadPtr);
    ReadPtr += 4;
    Info.addArgInfo(ArgIsLiteral, ArgSize);
  }

  if (Version > 1) {
    // Read 8-byte Attributes1 since version 2.
    uint64_t Attributes1 = llvm::support::endian::read64le(ReadPtr);
    Info.setAttributes1(Attributes1);
    ReadPtr += 8;
  }

  if (Version > 2) {
    // Read 8-byte WGNum since version 3.
    uint32_t WGNum = llvm::support::endian::read64le(ReadPtr);
    Info.setWGNum(WGNum);
    ReadPtr += 8;
  }

  if (Version > 3) {
    // Read 8-byte WGNum since version 3.
    uint32_t WINum = llvm::support::endian::read64le(ReadPtr);
    Info.setWINum(WINum);
    ReadPtr += 8;
  }

  KernelInfo.emplace(std::make_pair(*KernelPtr, std::move(Info)));
  return true;
}

const KernelInfoTy *LevelZeroProgramTy::getKernelInfo(
    const ze_kernel_handle_t Kernel) const {
  auto I = KernelInfo.find(Kernel);
  if (I != KernelInfo.end())
    return &(I->second);
  else
    return nullptr;
}

int32_t LevelZeroProgramTy::buildKernels() {
  size_t NumEntries = (size_t)(Image->EntriesEnd - Image->EntriesBegin);

  Entries.resize(NumEntries);
  Kernels.resize(NumEntries);

  auto EnableTargetGlobals = DeviceInfo->Option.Flags.EnableTargetGlobals;
  if (NumEntries > 0 && EnableTargetGlobals &&
      DeviceInfo->DeviceArchs[DeviceId] != DeviceArch_XeLP &&
      !loadOffloadTable(NumEntries)) {
    DP("Warning: could not load offload table.\n");
  }

  // We need to build kernels here before filling the offload entries since we
  // don't know which module contains a specific kernel with a name.
  std::unordered_map<std::string, ze_kernel_handle_t> ModuleKernels;
  for (auto Module : Modules) {
    uint32_t Count = 0;
    CALL_ZE_RET_FAIL(zeModuleGetKernelNames, Module, &Count, nullptr);
    if (Count == 0)
      continue;

    std::vector<const char *> Names(Count);
    CALL_ZE_RET_FAIL(zeModuleGetKernelNames, Module, &Count, Names.data());

    ze_kernel_desc_t KernelDesc = {ZE_STRUCTURE_TYPE_KERNEL_DESC, nullptr, 0};
    for (auto *Name : Names) {
      KernelDesc.pKernelName = Name;
      ze_kernel_handle_t Kernel = nullptr;
      CALL_ZE_RET_FAIL(zeKernelCreate, Module, &KernelDesc, &Kernel);
      ModuleKernels.emplace(Name, Kernel);
    }
  }

  for (auto I = 0; I < NumEntries; I++) {
    auto Size = Image->EntriesBegin[I].size;
    auto *Name = Image->EntriesBegin[I].name;

    if (Size != 0) {
      // Entry is a global variable
      auto HstAddr = Image->EntriesBegin[I].addr;
      void *TgtAddr = nullptr;

      if (EnableTargetGlobals)
        TgtAddr = getOffloadVarDeviceAddr(Name, Size);

      if (!TgtAddr) {
        TgtAddr = allocDataExplicit(DeviceId, Size,
                                    DeviceInfo->AllocKinds[DeviceId]);
        __tgt_rtl_data_submit(DeviceId, TgtAddr, HstAddr, Size);
        std::unique_lock<std::mutex> Lock(DeviceInfo->DataMutexes[DeviceId]);
        DeviceInfo->OwnedMemory[DeviceId].push_back(TgtAddr);
        DP("Warning: global variable '%s' allocated. "
           "Direct references will not work properly.\n", Name);
      }

      DP("Global variable mapped: Name = %s, Size = %zu, "
         "HostPtr = " DPxMOD ", TgtPtr = " DPxMOD "\n",
         Name, Size, DPxPTR(HstAddr), DPxPTR(TgtAddr));
      Entries[I].addr = TgtAddr;
      Entries[I].name = Name;
      Entries[I].size = Size;
      Kernels[I] = nullptr;
      continue;
    }

    // Entry is a kernel
#if _WIN32
    // FIXME: temporary allow zero padding bytes in the entries table
    //        added by MSVC linker (e.g. for incremental linking).
    if (!Name) {
      // Initialize the members to be on the safe side.
      DP("Warning: Entry with a nullptr name!!!\n");
      Entries[I].addr = nullptr;
      Entries[I].name = nullptr;
      continue;
    }
#endif

    Entries[I].addr = &Kernels[I];
    Entries[I].name = Name;

    std::string KernelName(Name);
    auto K = ModuleKernels.find(KernelName);
    if (K == ModuleKernels.end()) {
      // If a kernel was deleted by optimizations (e.g. DCE), then
      // zeCreateKernel will fail. We expect that such a kernel
      // will never be actually invoked.
      DP("Warning: cannot find kernel %s\n", Name);
      Kernels[I] = nullptr;
      continue;
    } else {
      Kernels[I] = K->second;
      ModuleKernels.erase(KernelName);
    }

    // Retrieve kernel info generated by compiler
    if (!readKernelInfo(Entries[I])) {
      DP("Error: failed to read kernel info for kernel %s\n", Name);
      return OFFLOAD_FAIL;
    }

    // Retrieve kernel properties
    auto &KernelProperties = DeviceInfo->KernelProperties[DeviceId][Kernels[I]];
    KernelProperties.Name = Name;
    ze_kernel_properties_t KP = KernelPropertiesInit;
    CALL_ZE_RET_FAIL(zeKernelGetProperties, Kernels[I], &KP);
    if (DeviceInfo->Option.ForcedKernelWidth > 0) {
      KernelProperties.Width = DeviceInfo->Option.ForcedKernelWidth;
    } else {
      KernelProperties.Width = KP.maxSubgroupSize;
      KernelProperties.SIMDWidth = KP.maxSubgroupSize;
      // Temporary workaround before ze_kernel_preferred_group_size_properties_t
      // becomes available with L0 1.2.
      // Here we try to match OpenCL kernel property
      // CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE.
      if (DeviceInfo->DeviceArchs[DeviceId] != DeviceArch_Gen9)
        KernelProperties.Width *= 2;
    }
    DP("Kernel %" PRIu32 ": Entry = " DPxMOD ", Name = %s, "
       "NumArgs = %" PRIu32 ", Handle = " DPxMOD "\n", I,
       DPxPTR(Image->EntriesBegin[I].addr), Image->EntriesBegin[I].name,
       KP.numKernelArgs, DPxPTR(Kernels[I]));
#if 0
    // Enable this with 0.95.55 Level Zero.
    KernelProperties.MaxThreadGroupSize =
        kernelProperties.maxSubgroupSize * kernelProperties.maxNumSubgroups;
#else
    KernelProperties.MaxThreadGroupSize =
        (std::numeric_limits<uint32_t>::max)();
#endif
  }

  // Release unused kernels
  for (auto &K : ModuleKernels)
    CALL_ZE_RET_FAIL(zeKernelDestroy, K.second);

  Table.EntriesBegin = &(Entries.data()[0]);
  Table.EntriesEnd = &(Entries.data()[Entries.size()]);

  return OFFLOAD_SUCCESS;
}

int32_t LevelZeroProgramTy::initProgramData() {
  // Look up program data location on device
  PGMDataPtr = getVarDeviceAddr("__omp_spirv_program_data", sizeof(PGMData));
  if (!PGMDataPtr) {
    DP("Warning: cannot find program data location on device.\n");
    return OFFLOAD_SUCCESS;
  }

  // Prepare host data to copy
  auto &P = DeviceInfo->DeviceProperties[DeviceId];
  uint32_t TotalEUs =
      P.numSlices * P.numSubslicesPerSlice * P.numEUsPerSubslice;
  // If CCS is exposed as an OpenMP device, adjust EU count
  if (DeviceInfo->Option.DeviceMode == DEVICE_MODE_SUBSUB) {
    uint32_t NumCCS = DeviceInfo->NumCCSQueues[DeviceId];
    if (NumCCS > 0)
      TotalEUs /= NumCCS;
  }

  // Allocate dynamic memory for in-kernel allocation
  void *MemLB = 0;
  uintptr_t MemUB = 0;
  size_t MemSize = DeviceInfo->Option.KernelDynamicMemorySize;
  if (MemSize > 0)
    MemLB = allocDataExplicit(Device, MemSize, TARGET_ALLOC_DEVICE);

  if (MemLB) {
    DeviceInfo->OwnedMemory[DeviceId].push_back(MemLB);
    MemUB = (uintptr_t)MemLB + MemSize;
  }

  PGMData = {
    1,                   // Initialized
    (int32_t)DeviceInfo->NumRootDevices,
                         // Number of OpenMP devices
    DeviceId,            // Device ID
    TotalEUs,            // Total EUs
    P.numThreadsPerEU,   // HW threads per EU
    (uintptr_t)MemLB,    // Dynamic memory LB
    MemUB,               // Dynamic memory UB
    0                    // Device type (0 for GPU, 1 for CPU)
  };

  return DeviceInfo->enqueueMemCopy(DeviceId, PGMDataPtr, &PGMData,
                                    sizeof(PGMData));
}

int32_t LevelZeroProgramTy::resetProgramData() {
  if (!PGMDataPtr)
    return OFFLOAD_SUCCESS;

  return DeviceInfo->enqueueMemCopy(DeviceId, PGMDataPtr, &PGMData,
                                    sizeof(PGMData), true /* Locked */);
}
#endif // INTEL_CUSTOMIZATION

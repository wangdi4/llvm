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
#if INTEL_COLLAB
//===--- Target RTLs Implementation ---------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file is modified from https://github.com/daniel-schuermann/openmp.git.
// Thanks to Daniel Scheuermann, the author of rtl.cpp.
//
// RTL for SPIR-V/OpenCL machine
//
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <Windows.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

#include "elf_light.h"
#include "omptargetplugin.h"
#if INTEL_CUSTOMIZATION
#include "omptarget-tools.h"
#endif // INTEL_CUSTOMIZATION
#include "rtl-trace.h"

#include "llvm/Support/Endian.h"

#if INTEL_CUSTOMIZATION
// FIXME: when this is upstreamed for OpenCL.
#define CL_MEM_FLAGS_INTEL                                               0x10001
#define CL_MEM_ALLOW_UNRESTRICTED_SIZE_INTEL                           (1 << 23)

#ifdef _WIN32
// TODO: enable again if XDEPS-3027 is resolved
#define OCL_KERNEL_BEGIN(ID)
#define OCL_KERNEL_END(ID)
#else // _WIN32
#define OCL_KERNEL_BEGIN(ID)                                                   \
  do {                                                                         \
    if (DeviceInfo->Option.KernelDynamicMemorySize > 0 &&                      \
        DeviceInfo->Option.KernelDynamicMemoryMethod == 0) {                   \
      /* Already in critical section */                                        \
      DeviceInfo->NumActiveKernels[ID]++;                                      \
    }                                                                          \
  } while (0)

#define OCL_KERNEL_END(ID)                                                     \
  do {                                                                         \
    if (DeviceInfo->Option.KernelDynamicMemorySize > 0 &&                      \
        DeviceInfo->Option.KernelDynamicMemoryMethod == 0) {                   \
      DeviceInfo->Mutexes[ID].lock();                                          \
      DeviceInfo->NumActiveKernels[ID]--;                                      \
      if (DeviceInfo->NumActiveKernels[ID] == 0)                               \
        DeviceInfo->resetProgramData(ID);                                      \
      DeviceInfo->Mutexes[ID].unlock();                                        \
    }                                                                          \
  } while (0)
#endif // _WIN32
#endif // INTEL_CUSTOMIZATION

/// Additional TARGET_ALLOC* definition for the plugin
constexpr int32_t TARGET_ALLOC_SVM = INT32_MAX;

/// Device type enumeration common to compiler and runtime
enum DeviceArch : uint64_t {
  DeviceArch_None   = 0,
  DeviceArch_Gen9   = 0x0001,
  DeviceArch_XeLP   = 0x0002,
  DeviceArch_XeHP   = 0x0004,
  DeviceArch_x86_64 = 0x0100
};

/// Mapping from device arch to GPU runtime's device identifiers
#ifdef _WIN32
/// For now, we need to depend on known published product names
std::map<uint64_t, std::vector<const char *>> DeviceArchStrMap {
  {
    DeviceArch_Gen9, {
      "HD Graphics",
      "UHD Graphics",
      "Pro Graphics",
      "Plus Graphics",
      "Iris(TM) Graphics",
    }
  },
  {
    DeviceArch_XeLP, {
      "Xe Graphics",
      "Xe MAX Graphics"
    }
  }
  // TODO: how to detect XeHP?
  // Using XeHP on Windows seems to be a rare case.
};
#endif // _WIN32
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
#if INTEL_CUSTOMIZATION
      0x0200, // ATS
      // Putting PVC here for now.
      // We may decide to add another arch type if needed in the future.
      0x0b00, // PVC
#endif // INTEL_CUSTOMIZATION
    }
  }
};

#if INTEL_CUSTOMIZATION
/// Interop support
namespace OCLInterop {
  // ID and names from openmp.org
  const int32_t Vendor = 8;
  const char *VendorName = GETNAME(intel);
  const int32_t FrId = 3;
  const char *FrName = GETNAME(opencl);

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
    "fr_id"
  };

  std::vector<const char *> IprTypeDescs {
    "cl_command_queue, opencl command queue handle",
    "cl_context, opencl context handle",
    "cl_device_id, opencl device handle",
    "cl_platform_id, opencl platform handle",
    "intptr_t, OpenMP device ID",
    "const char *, vendor name",
    "intptr_t, vendor ID",
    "const char *, foreign runtime name",
    "intptr_t, foreign runtime ID"
  };

  struct Property {
    // TODO
  };
}
#endif // INTEL_CUSTOMIZATION

int DebugLevel = getDebugLevel();

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// FIXME: we should actually include omp.h instead of declaring
//        these ourselves.
#if _WIN32
int __cdecl omp_get_max_teams(void);
int __cdecl omp_get_thread_limit(void);
double __cdecl omp_get_wtime(void);
int __cdecl __kmpc_global_thread_num(void *);
#else   // !_WIN32
int omp_get_max_teams(void) __attribute__((weak));
int omp_get_thread_limit(void) __attribute__((weak));
double omp_get_wtime(void) __attribute__((weak));
int __kmpc_global_thread_num(void *) __attribute__((weak));
#endif  // !_WIN32

#ifdef __cplusplus
}
#endif  // __cplusplus

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

/// Profile data
struct ProfileDataTy {
  struct TimingsTy {
    double host = 0.0;
    double device = 0.0;
  };

  std::map<std::string, TimingsTy> data;

  std::string alignLeft(size_t Width, std::string Str) {
    if (Str.size() < Width)
      return Str + std::string(Width - Str.size(), ' ');
    return Str;
  }

  void printData(int32_t deviceId, int32_t threadId, const char *deviceName,
                 int64_t resolution) {
    std::string profileSep(80, '=');
    std::string lineSep(80, '-');

    fprintf(stderr, "%s\n", profileSep.c_str());

    fprintf(stderr, "LIBOMPTARGET_PLUGIN_PROFILE(%s) for OMP DEVICE(%" PRId32
            ") %s, Thread %" PRId32 "\n", GETNAME(TARGET_NAME), deviceId,
            deviceName, threadId);

    fprintf(stderr, "%s\n", lineSep.c_str());

    const char *unit = resolution == 1000 ? "msec" : "usec";

    std::string kernelPrefix("Kernel ");
    size_t maxKeyLength = kernelPrefix.size() + 3;
    for (const auto &d : data)
      if (d.first.substr(0, kernelPrefix.size()) != kernelPrefix &&
          maxKeyLength < d.first.size())
        maxKeyLength = d.first.size();

    // Print kernel key and name
    int kernelId = 0;
    for (const auto &d: data) {
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
    for (const auto &d : data) {
      double hostTime = 1e-9 * d.second.host * resolution;
      double deviceTime = 0.0;
      std::string key(d.first);

      if (d.first.substr(0, kernelPrefix.size()) == kernelPrefix) {
        key = kernelPrefix + std::to_string(kernelId++);
        deviceTime = 1e-9 * d.second.device * resolution;
      } else if (d.first.substr(0, 8) == "DataRead" ||
                 d.first.substr(0, 9) == "DataWrite") {
        deviceTime = 1e-9 * d.second.device * resolution;
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

  // for non-event profile
  void update(
      const char *name, cl_ulong host_elapsed, cl_ulong device_elapsed) {
    std::string key(name);
    TimingsTy &timings = data[key];
    timings.host += host_elapsed;
    timings.device += device_elapsed;
  }

  void update(
      const char *name, double host_elapsed, double device_elapsed) {
    std::string key(name);
    TimingsTy &timings = data[key];
    timings.host += host_elapsed;
    timings.device += device_elapsed;
  }

  // for event profile
  void update(const char *name, cl_event event) {
    cl_ulong host_begin = 0, host_end = 0;
    CALL_CLW_RET_VOID(clGetEventProfilingInfo, event,
        CL_PROFILING_COMMAND_QUEUED, sizeof(cl_ulong), &host_begin, nullptr);
    CALL_CLW_RET_VOID(clGetEventProfilingInfo, event,
        CL_PROFILING_COMMAND_COMPLETE, sizeof(cl_ulong), &host_end, nullptr);
    cl_ulong device_begin = 0, device_end = 0;
    CALL_CLW_RET_VOID(clGetEventProfilingInfo, event,
        CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &device_begin, nullptr);
    CALL_CLW_RET_VOID(clGetEventProfilingInfo, event,
        CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &device_end, nullptr);
    update(name, host_end - host_begin, device_end - device_begin);
  }
}; // ProfileDataTy

// Platform-dependent information -- context and INTEL extension API
struct PlatformInfoTy {
  cl_platform_id Platform = nullptr;
  cl_context Context = nullptr;
  std::vector<const char *> ExtensionFunctionNames {
#define EXTENSION_FN_NAME(Fn) TO_STRING(Fn),
      FOR_EACH_EXTENSION_FN(EXTENSION_FN_NAME)
  };
  std::vector<void *> ExtensionFunctionPointers;

  PlatformInfoTy() = default;

  PlatformInfoTy(cl_platform_id platform, cl_context context) {
    Platform = platform;
    Context = context;
    ExtensionFunctionPointers.resize(ExtensionFunctionNames.size(), nullptr);
    for (int i = 0; i < ExtensionIdLast; i++) {
      CALL_CL_RV(ExtensionFunctionPointers[i],
                 clGetExtensionFunctionAddressForPlatform, platform,
                 ExtensionFunctionNames[i]);
      if (ExtensionFunctionPointers[i]) {
        DP("Extension %s is found.\n", ExtensionFunctionNames[i]);
      } else {
        DP("Warning: Extension %s is not found.\n", ExtensionFunctionNames[i]);
      }
    }
  }
};

// OpenCL extensions status.
enum ExtensionStatusTy : uint8_t {
  // Default value.  It is unknown if the extension is supported.
  ExtensionStatusUnknown = 0,

  // Extension is disabled (either because it is unsupported or
  // due to user environment control).
  ExtensionStatusDisabled,

  // Extenstion is enabled.  An extension can only be used,
  // if it has this status after __tgt_rtl_load_binary.
  ExtensionStatusEnabled,
};

// A descriptor of OpenCL extensions with their statuses.
struct ExtensionsTy {
  ExtensionStatusTy UnifiedSharedMemory = ExtensionStatusUnknown;
  ExtensionStatusTy DeviceAttributeQuery = ExtensionStatusUnknown;
  ExtensionStatusTy GetDeviceGlobalVariablePointer = ExtensionStatusUnknown;
  ExtensionStatusTy SuggestedGroupSize = ExtensionStatusUnknown;
#if INTEL_CUSTOMIZATION
  ExtensionStatusTy GitsIndirectAllocationOffsets = ExtensionStatusUnknown;
#endif  // INTEL_CUSTOMIZATION

  // Libdevice extensions that may be supported by device runtime.
  struct LibdeviceExtDescTy {
    const char *Name;
    const char *FallbackLibName;
    ExtensionStatusTy Status;
  };

  std::vector<LibdeviceExtDescTy> LibdeviceExtensions = {
    {
      "cl_intel_devicelib_cassert",
      "libomp-fallback-cassert.spv",
      ExtensionStatusUnknown
    },
    {
      "cl_intel_devicelib_math",
      "libomp-fallback-cmath.spv",
      ExtensionStatusUnknown
    },
    {
      "cl_intel_devicelib_math_fp64",
      "libomp-fallback-cmath-fp64.spv",
      ExtensionStatusUnknown
    },
    {
      "cl_intel_devicelib_complex",
      "libomp-fallback-complex.spv",
      ExtensionStatusUnknown
    },
    {
      "cl_intel_devicelib_complex_fp64",
      "libomp-fallback-complex-fp64.spv",
      ExtensionStatusUnknown
    },
    {
      "cl_intel_devicelib_cstring",
      "libomp-fallback-cstring.spv",
      ExtensionStatusUnknown
    },
  };

  // Initialize extensions' statuses for the given device.
  int32_t getExtensionsInfoForDevice(int32_t DeviceId);
};

/// Data transfer method
enum DataTransferMethodTy {
  DATA_TRANSFER_METHOD_INVALID = -1,   // Invalid
  DATA_TRANSFER_METHOD_CLMEM = 0,      // Use Buffer on SVM
  DATA_TRANSFER_METHOD_SVMMAP,         // Use SVMMap/Unmap
  DATA_TRANSFER_METHOD_SVMMEMCPY,      // Use SVMMemcpy
  DATA_TRANSFER_METHOD_LAST,
};

#if INTEL_CUSTOMIZATION
struct DynamicMemHeapTy {
  /// Base address memory is allocated from
  uintptr_t AllocBase = 0;
  /// Minimal size served by the current heap
  size_t BlockSize = 0;
  /// Max size served by the current heap
  size_t MaxSize = 0;
  /// Available memory blocks
  uint32_t NumBlocks = 0;
  /// Number of block descriptors
  uint32_t NumBlockDesc = 0;
  /// Number of block counters
  uint32_t NumBlockCounter = 0;
  /// List of memory block descriptors
  uint64_t *BlockDesc = nullptr;
  /// List of memory block counters
  uint32_t *BlockCounter = nullptr;
};

struct DynamicMemPoolTy {
  /// Location of device memory blocks
  void *PoolBase = nullptr;
  /// Heap size common to all heaps
  size_t HeapSize = 0;
  /// Number of heaps available
  uint32_t NumHeaps = 0;
  /// Heap descriptors (using fixed-size array to simplify memory allocation)
  DynamicMemHeapTy HeapDesc[8];
};

/// Program data to be initialized.
/// TODO: include other runtime parameters if necessary.
struct ProgramDataTy {
  int Initialized = 0;
  int NumDevices = 0;
  int DeviceNum = -1;
  uint32_t TotalEUs = 0;
  uint32_t HWThreadsPerEU = 0;
  uintptr_t DynamicMemoryLB = 0;
  uintptr_t DynamicMemoryUB = 0;
  int DeviceType = 0;
  void *DynamicMemPool = nullptr;
  int TeamsThreadLimit = 0;
};
#endif // INTEL_CUSTOMIZATION

/// OpenCL program that can contain multiple OCL programs
class OpenCLProgramTy {
  struct DeviceOffloadEntryTy {
    /// Common part with the host offload table.
    __tgt_offload_entry Base;
    /// Length of the Base.name string in bytes including
    /// the null terminator.
    size_t NameSize;
  };

  /// Cached device image
  __tgt_device_image *Image = nullptr;

  /// Cached OpenCL context
  cl_context Context = nullptr;

  /// Cached OpenDL device
  cl_device_id Device = nullptr;

  /// Cached OpenMP device ID
  int32_t DeviceId = 0;

  /// Program is created from binary?
  bool IsBinary = false;

  /// Target table
  __tgt_target_table Table;

  /// Target entries
  std::vector<__tgt_offload_entry> Entries;

  /// Internal offload entries
  std::vector<DeviceOffloadEntryTy> OffloadEntries;

  /// Handle multiple modules within a single target image
  std::vector<cl_program> Programs;

  /// Kernels created from the target image
  std::vector<cl_kernel> Kernels;

  /// Kernel info added by compiler
  std::unordered_map<cl_kernel, KernelInfoTy> KernelInfo;

#if INTEL_CUSTOMIZATION
  /// Program data copied to device
  ProgramDataTy PGMData;

  /// Cached address of the program data on device
  void *PGMDataPtr = nullptr;
#endif // INTEL_CUSTOMIZATION

  /// Final OpenCL program
  cl_program FinalProgram = nullptr;

  /// Requires program link
  bool RequiresProgramLink = false;

  /// Loads the device version of the offload table for device \p DeviceId.
  /// The table is expected to have \p NumEntries entries.
  /// Returns true, if the load was successful, false - otherwise.
  bool loadOffloadTable(size_t NumEntries);

  /// Add a single OpenCL program created from the given SPIR-V image
  int32_t addProgramIL(const size_t Size, const unsigned char *Image);

  /// Add a single OpenCL program created from the given native image
  int32_t addProgramBIN(const size_t Size, const unsigned char *Image);

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
  OpenCLProgramTy() = default;

  OpenCLProgramTy(__tgt_device_image *Image_, cl_context Context_,
                  cl_device_id Device_, int32_t DeviceId_) :
      Image(Image_), Context(Context_), Device(Device_), DeviceId(DeviceId_) {}

  ~OpenCLProgramTy();

  int32_t buildPrograms(std::string &CompilationOptions,
                        std::string &LinkingOptions);

  int32_t compilePrograms(std::string &CompilationOptions,
                          std::string &LinkingOptions);

  int32_t linkPrograms(std::string &LinkingOptions);

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

#if INTEL_CUSTOMIZATION
  /// Initialize program data on device.
  int32_t initProgramData();

  /// Reset program data on device.
  int32_t resetProgramData();

  /// Initialize dynamic memory pool for device.
  void *initDynamicMemPool();

  /// Return the cached program data
  const ProgramDataTy &getPGMData() { return PGMData; }
#endif // INTEL_CUSTOMIZATION

  /// Return the pointer to the offload table.
  __tgt_target_table *getTablePtr() { return &Table; }

  /// Returns the auxiliary kernel information for the specified kernel.
  const KernelInfoTy *getKernelInfo(cl_kernel Kernel) const;
};

/// RTL flags
struct RTLFlagsTy {
  uint64_t CollectDataTransferLatency : 1;
  uint64_t EnableProfile : 1;
  uint64_t UseInteropQueueInorderAsync : 1;
  uint64_t UseInteropQueueInorderSharedSync : 1;
  uint64_t UseHostMemForUSM : 1;
  uint64_t UseDriverGroupSizes : 1;
  uint64_t EnableSimd : 1;
  uint64_t UseSVM : 1;
  uint64_t UseBuffer : 1;
  uint64_t UseSingleContext : 1;
  uint64_t UseImageOptions : 1;
  uint64_t ShowBuildLog : 1;
  uint64_t LinkLibDevice : 1;
  // Add new flags here
  uint64_t Reserved : 51;
  RTLFlagsTy() :
      CollectDataTransferLatency(0),
      EnableProfile(0),
      UseInteropQueueInorderAsync(0),
      UseInteropQueueInorderSharedSync(0),
      UseHostMemForUSM(0),
      UseDriverGroupSizes(0),
      EnableSimd(0),
      UseSVM(0),
      UseBuffer(0),
      UseSingleContext(0),
      UseImageOptions(1),
      ShowBuildLog(0),
      LinkLibDevice(0),
      Reserved(0) {}
};

/// Kernel properties.
struct KernelPropertiesTy {
  size_t Width = 0;
  size_t SIMDWidth = 0;
  size_t MaxThreadGroupSize = 0;
  /// Kernel-specific implicit arguments
  std::set<void *> ImplicitArgs;
};

/// Specialization constants used for an OpenCL program compilation.
class SpecConstantsTy {
  std::vector<uint32_t> ConstantIds;
  std::vector<size_t> ConstantValueSizes;
  std::vector<const void *> ConstantValues;

public:
  SpecConstantsTy() = default;
  SpecConstantsTy(const SpecConstantsTy &) = delete;
  SpecConstantsTy(const SpecConstantsTy &&Other)
    : ConstantIds(std::move(Other.ConstantIds)),
      ConstantValueSizes(std::move(Other.ConstantValueSizes)),
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
    ConstantValueSizes.push_back(ValSize);
    ConstantValues.push_back(reinterpret_cast<void *>(ValuePtr));
  }

  void setProgramConstants(int32_t DeviceId, cl_program Program) const;
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

  /// Return allocation information if Ptr belongs to any allocation range.
  const MemAllocInfoTy *search(void *Ptr) {
    std::lock_guard<std::mutex> Lock(Mtx);
    if (Map.size() == 0)
      return nullptr;
    auto I = Map.upper_bound(const_cast<void *>(Ptr));
    // Key pointer (I->first) may be greater than Ptr, so both I and --I need to
    // be checked.
    int J = 0;
    do {
      std::advance(I, J);
      if (I == Map.end())
        continue;
      uintptr_t AllocBase = (uintptr_t)I->second.Base;
      size_t AllocSize = I->second.Size + (uintptr_t)I->first - AllocBase;
      if (AllocBase <= (uintptr_t)Ptr && (uintptr_t)Ptr < AllocBase + AllocSize)
        return &I->second;
    } while (J-- > -1 && I != Map.begin());

    return nullptr;
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

  /// Add a list of implicit arguments to the output vector.
  void getImplicitArgs(
      std::vector<void *> &SVMArgs, std::vector<void *> &USMArgs) {
    std::lock_guard<std::mutex> Lock(Mtx);
    for (auto &AllocInfo : Map) {
      if (AllocInfo.second.ImplicitArg) {
        if (AllocInfo.second.Kind == TARGET_ALLOC_SVM)
          SVMArgs.push_back(AllocInfo.first);
        else
          USMArgs.push_back(AllocInfo.first);
      }
    }
  }

  bool hasImplicitUSMArg(int32_t Kind = TARGET_ALLOC_DEFAULT) {
    std::lock_guard<std::mutex> Lock(Mtx);
    if (Kind == TARGET_ALLOC_DEFAULT) {
      uint32_t Num = NumImplicitArgs[TARGET_ALLOC_HOST] +
          NumImplicitArgs[TARGET_ALLOC_DEVICE] +
          NumImplicitArgs[TARGET_ALLOC_SHARED];
      return Num > 0;
    } else {
      return NumImplicitArgs[Kind] > 0;
    }
  }
};

/// RTL options and flags users can override
struct RTLOptionTy {
  /// Binary flags
  RTLFlagsTy Flags;

  /// Emulated data transfer latency in microsecond
  int32_t DataTransferLatency = 0;

  /// Data transfer method when SVM is used
  int32_t DataTransferMethod = DATA_TRANSFER_METHOD_SVMMAP;

  /// Plugin profiling resolution (msec by default)
  int64_t ProfileResolution = 1000;

  /// Used device type
  cl_device_type DeviceType = CL_DEVICE_TYPE_GPU;

  // OpenCL 2.0 builtins (like atomic_load_explicit and etc.) are used by
  // runtime, so we have to explicitly specify the "-cl-std=CL2.0" compilation
  // option. With it, the SPIR-V will be converted to LLVM IR with OpenCL 2.0
  // builtins. Otherwise, SPIR-V will be converted to LLVM IR with OpenCL 1.2
  // builtins.
  std::string CompilationOptions = "-cl-std=CL2.0 ";
  std::string UserCompilationOptions = "";
  std::string UserLinkingOptions = "";

#if INTEL_CUSTOMIZATION
  std::string InternalCompilationOptions = "";
  std::string InternalLinkingOptions = "";
#endif  // INTEL_CUSTOMIZATION

  /// Limit for the number of WIs in a WG.
  uint32_t ThreadLimit = 0;

  /// Limit for the number of WGs.
  uint32_t NumTeams = 0;

#if INTEL_CUSTOMIZATION
  /// Dynamic kernel memory size
  size_t KernelDynamicMemorySize = 0; // Turned off by default
  /// Dynamic kernel memory allocation method
  /// 0: atomic_add with no free()
  /// 1: pool-based allocator with free() support
  uint32_t KernelDynamicMemoryMethod = 1;
#endif // INTEL_CUSTOMIZATION

  // This is a factor applied to the number of WGs computed
  // for the execution, based on the HW characteristics.
  size_t SubscriptionRate = 1;

  // For kernels that compute cross-WG reductions the number of computed WGs
  // is reduced by this factor.
  size_t ReductionSubscriptionRate = 1;
  bool ReductionSubscriptionRateIsDefault = true;

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

#if INTEL_INTERNAL_BUILD
  /// Forced GWS/LWS only for internal experiments
  size_t ForcedLocalSizes[3] = {0, 0, 0};
  size_t ForcedGlobalSizes[3] = {0, 0, 0};
#endif // INTEL_INTERNAL_BUILD

  // Spec constants used for all OpenCL programs.
  SpecConstantsTy CommonSpecConstants;

  RTLOptionTy() {
    const char *Env;

    // Get global OMP_THREAD_LIMIT for SPMD parallelization.
    int ThrLimit = omp_get_thread_limit();
    DP("omp_get_thread_limit() returned %" PRId32 "\n", ThrLimit);
    // omp_get_thread_limit() would return INT_MAX by default.
    // NOTE: Windows.h defines max() macro, so we have to guard
    //       the call with parentheses.
    ThreadLimit = (ThrLimit > 0 &&
        ThrLimit != (std::numeric_limits<int32_t>::max)()) ? ThrLimit : 0;

    // Global max number of teams.
    int NTeams = omp_get_max_teams();
    DP("omp_get_max_teams() returned %" PRId32 "\n", NTeams);
    // omp_get_max_teams() would return INT_MAX by default.
    // NOTE: Windows.h defines max() macro, so we have to guard
    //       the call with parentheses.
    NumTeams = (NTeams > 0 &&
        NTeams != (std::numeric_limits<int32_t>::max)()) ? NTeams : 0;

    // Read LIBOMPTARGET_DATA_TRANSFER_LATENCY (experimental input)
    if ((Env = readEnvVar("LIBOMPTARGET_DATA_TRANSFER_LATENCY"))) {
      std::string Value(Env);
      if (Value.substr(0, 2) == "T,") {
        Flags.CollectDataTransferLatency = 1;
        int32_t Usec = std::stoi(Value.substr(2).c_str());
        DataTransferLatency = (Usec > 0) ? Usec : 0;
      }
    }

    // Read LIBOMPTARGET_OPENCL_DATA_TRANSFER_METHOD
    if ((Env = readEnvVar("LIBOMPTARGET_OPENCL_DATA_TRANSFER_METHOD"))) {
      std::string Value(Env);
      DataTransferMethod = DATA_TRANSFER_METHOD_INVALID;
      if (Value.size() == 1 && std::isdigit(Value.c_str()[0])) {
        int Method = std::stoi(Env);
        if (Method < DATA_TRANSFER_METHOD_LAST)
          DataTransferMethod = Method;
      }
      if (DataTransferMethod == DATA_TRANSFER_METHOD_INVALID) {
        WARNING("Invalid data transfer method (%s) selected"
                " -- using default method.\n", Env);
        DataTransferMethod = DATA_TRANSFER_METHOD_SVMMAP;
      }
    }

    // Read LIBOMPTARGET_DEVICETYPE
    if ((Env = readEnvVar("LIBOMPTARGET_DEVICETYPE"))) {
      std::string Value(Env);
      if (Value == "GPU" || Value == "gpu" || Value == "")
        DeviceType = CL_DEVICE_TYPE_GPU;
      else if (Value == "CPU" || Value == "cpu")
        DeviceType = CL_DEVICE_TYPE_CPU;
      else
        WARNING("Invalid or unsupported LIBOMPTARGET_DEVICETYPE=%s\n", Env);
    }
    DP("Target device type is set to %s\n",
       (DeviceType == CL_DEVICE_TYPE_CPU) ? "CPU" : "GPU");

#if INTEL_CUSTOMIZATION
    if (DeviceType == CL_DEVICE_TYPE_GPU) {
      // Default subscription rate is heuristically set to 4 for GPU.
      // It only matters for the default ND-range parallelization,
      // i.e. when the global size is unknown on the host.
      SubscriptionRate = 4;
      ReductionSubscriptionRate = 16;
    }
#endif  // INTEL_CUSTOMIZATION

    /// Oversubscription rate for normal kernels
    if ((Env = readEnvVar("LIBOMPTARGET_OPENCL_SUBSCRIPTION_RATE"))) {
      int32_t Value = std::stoi(Env);
      // Set some reasonable limits.
      if (Value > 0 && Value <= 0xFFFF)
        SubscriptionRate = Value;
    }

    /// Oversubscription rate for reduction kernels
    if ((Env = readEnvVar("LIBOMPTARGET_ONEAPI_REDUCTION_SUBSCRIPTION_RATE"))) {
      int32_t Value = std::stoi(Env);
      // Set some reasonable limits.
      // '0' is a special value meaning to use regular default ND-range
      // for kernels with reductions.
      if (Value >= 0 && Value <= 0xFFFF) {
        ReductionSubscriptionRate = Value;
        ReductionSubscriptionRateIsDefault = false;
      }
    }

    /// Read LIBOMPTARGET_PLUGIN_PROFILE
    if ((Env = readEnvVar("LIBOMPTARGET_PLUGIN_PROFILE"))) {
      std::istringstream Value(Env);
      std::string Token;
      while (std::getline(Value, Token, ',')) {
        if (Token == "T" || Token == "1")
          Flags.EnableProfile = 1;
        else if (Token == "unit_usec" || Token == "usec")
          ProfileResolution = 1000000;
      }
    }

    if ((Env = readEnvVar("LIBOMPTARGET_ENABLE_SIMD"))) {
      if (parseBool(Env) == 1)
        Flags.EnableSimd = 1;
      else
        WARNING("Invalid or unsupported LIBOMPTARGET_ENABLE_SIMD=%s\n", Env);
    }

    // TODO: deprecate this variable since the default behavior is equivalent
    //       to "inorder_async" and "inorder_shared_sync".
    // Read LIBOMPTARGET_OPENCL_INTEROP_QUEUE
    // Two independent options can be specified as follows.
    // -- inorder_async: use a new in-order queue for asynchronous case
    //    (default: shared out-of-order queue)
    // -- inorder_shared_sync: use the existing shared in-order queue for
    //    synchronous case (default: new in-order queue).
    if ((Env = readEnvVar("LIBOMPTARGET_OPENCL_INTEROP_QUEUE",
                          "LIBOMPTARGET_INTEROP_PIPE"))) {
      std::istringstream Value(Env);
      std::string Token;
      while (std::getline(Value, Token, ',')) {
        if (Token == "inorder_async") {
          Flags.UseInteropQueueInorderAsync = 1;
          DP("    enabled in-order asynchronous separate queue\n");
        } else if (Token == "inorder_shared_sync") {
          Flags.UseInteropQueueInorderSharedSync = 1;
          DP("    enabled in-order synchronous shared queue\n");
        }
      }
    }

    if ((Env = readEnvVar("LIBOMPTARGET_OPENCL_COMPILATION_OPTIONS"))) {
      UserCompilationOptions += Env;
    }

    if ((Env = readEnvVar("LIBOMPTARGET_OPENCL_LINKING_OPTIONS"))) {
      UserLinkingOptions += Env;
    }

#if INTEL_CUSTOMIZATION
    // OpenCL CPU compiler complains about unsupported option.
    // Intel Graphics compilers that do not support that option
    // silently ignore it.
    if (DeviceType == CL_DEVICE_TYPE_GPU) {
      Env = readEnvVar("LIBOMPTARGET_OPENCL_TARGET_GLOBALS");
      if (!Env || parseBool(Env) != 0)
        InternalLinkingOptions += " -cl-take-global-address ";
      Env = readEnvVar("LIBOMPTARGET_OPENCL_MATCH_SINCOSPI");
      if (!Env || parseBool(Env) != 0)
        InternalLinkingOptions += " -cl-match-sincospi ";
      Env = readEnvVar("LIBOMPTARGET_OPENCL_USE_DRIVER_GROUP_SIZES");
      if (Env && parseBool(Env) == 1)
        Flags.UseDriverGroupSizes = 1;
    }
#endif  // INTEL_CUSTOMIZATION

    // Read LIBOMPTARGET_USM_HOST_MEM
    if ((Env = readEnvVar("LIBOMPTARGET_USM_HOST_MEM"))) {
      if (parseBool(Env) == 1)
        Flags.UseHostMemForUSM = 1;
    }

    // Read LIBOMPTARGET_OPENCL_USE_SVM
    if ((Env = readEnvVar("LIBOMPTARGET_OPENCL_USE_SVM"))) {
      int32_t Value = parseBool(Env);
      if (Value == 1)
        Flags.UseSVM = 1;
      else if (Value == 0)
        Flags.UseSVM = 0;
    }

    // Read LIBOMPTARGET_OPENCL_USE_BUFFER
    if ((Env = readEnvVar("LIBOMPTARGET_OPENCL_USE_BUFFER"))) {
      if (parseBool(Env) == 1)
        Flags.UseBuffer = 1;
    }

    // Read LIBOMPTARGET_USE_SINGLE_CONTEXT
    if ((Env = readEnvVar("LIBOMPTARGET_USE_SINGLE_CONTEXT"))) {
      if (parseBool(Env) == 1)
        Flags.UseSingleContext = 1;
    }

#if INTEL_CUSTOMIZATION
    // Read LIBOMPTARGET_DYNAMIC_MEMORY_SIZE=<SizeInMB>[,<Method>]
    if ((Env = readEnvVar("LIBOMPTARGET_DYNAMIC_MEMORY_SIZE"))) {
      std::string Value(Env);
      size_t Size = 0;
      auto Pos = Value.find(",");
      if (Pos == std::string::npos) {
        Size = std::stoi(Value);
      } else if (Value.substr(Pos + 1) == "0") {
        KernelDynamicMemoryMethod = 0;
        Size = std::stoi(Value.substr(0, Pos));
      } else if (Value.substr(Pos + 1) == "1") {
        KernelDynamicMemoryMethod = 1;
        Size = std::stoi(Value.substr(0, Pos));
      } else {
        DP("Ignoring incorrect value for LIBOMPTARGET_DYNAMIC_MEMORY_SIZE\n");
        Size = 0;
      }
      if (Size > 0) {
        size_t MaxSize;
        if (KernelDynamicMemoryMethod == 0) {
          MaxSize = 2048;
        } else {
          // Round up to power of 2
          uint32_t P = 1;
          while (P < Size)
            P *= 2;
          Size = P;
          MaxSize = 512;
        }
        if (Size > MaxSize) {
          WARNING("Requested dynamic memory size %zu MB exceeds allowed limit "
                  " -- setting it to %zu MB\n", Size, MaxSize);
          Size = MaxSize;
        }
        KernelDynamicMemorySize = Size << 20;
      }
    }
#endif // INTEL_CUSTOMIZATION

#if INTEL_INTERNAL_BUILD
    // Force work group sizes -- for internal experiments
    if ((Env = readEnvVar("LIBOMPTARGET_LOCAL_WG_SIZE"))) {
      parseGroupSizes("LIBOMPTARGET_LOCAL_WG_SIZE", Env, ForcedLocalSizes);
    }
    if ((Env = readEnvVar("LIBOMPTARGET_GLOBAL_WG_SIZE"))) {
      parseGroupSizes("LIBOMPTARGET_GLOBAL_WG_SIZE", Env, ForcedGlobalSizes);
    }
#endif // INTEL_INTERNAL_BUILD

    if (readEnvVar("INTEL_ENABLE_OFFLOAD_ANNOTATIONS")) {
      // To match SYCL RT behavior, we just need to check whether
      // INTEL_ENABLE_OFFLOAD_ANNOTATIONS is set. The actual value
      // does not matter.
      CommonSpecConstants.addConstant<char>(0xFF747469, 1);
    }

    if ((Env = readEnvVar("LIBOMPTARGET_ONEAPI_USE_IMAGE_OPTIONS"))) {
      int32_t Value = parseBool(Env);
      if (Value == 1)
        Flags.UseImageOptions = 1;
      else if (Value == 0)
        Flags.UseImageOptions = 0;
    }

    if ((Env = readEnvVar("LIBOMPTARGET_ONEAPI_SHOW_BUILD_LOG"))) {
      int32_t Value = parseBool(Env);
      if (Value == 1)
        Flags.ShowBuildLog = 1;
      else if (Value == 0)
        Flags.ShowBuildLog = 0;
    }

    // LIBOMPTARGET_ONEAPI_LINK_LIBDEVICE
    if ((Env = readEnvVar("LIBOMPTARGET_ONEAPI_LINK_LIBDEVICE"))) {
      int32_t Value = parseBool(Env);
      if (Value == 1)
        Flags.LinkLibDevice = 1;
      else if (Value == 0)
        Flags.LinkLibDevice = 0;
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
  void parseGroupSizes(const char *Name, const char *Value, size_t *Sizes) {
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

/// Device property
struct DevicePropertiesTy {
  cl_uint DeviceId = 0;
  cl_uint NumSlices = 0;
  cl_uint NumSubslicesPerSlice = 0;
  cl_uint NumEUsPerSubslice = 0;
  cl_uint NumThreadsPerEU = 0;
  cl_uint NumHWThreads = 0;

  int32_t getDeviceProperties(cl_device_id ID);
};

/// Class containing all the device information.
class RTLDeviceInfoTy {
public:
  /// Number of OpenMP devices
  cl_uint NumDevices = 0;

  /// List of OpenCLProgramTy objects
  std::vector<std::list<OpenCLProgramTy>> Programs;

  /// Contains context and extension API
  std::map<cl_platform_id, PlatformInfoTy> PlatformInfos;

  /// Platform that each device belongs to
  std::vector<cl_platform_id> Platforms;

  /// Contexts used by each device
  std::vector<cl_context> Contexts;

  /// OpenCL device
  std::vector<cl_device_id> Devices;

  // Internal device type ID
  std::vector<uint64_t> DeviceArchs;

  /// Device properties
  std::vector<int32_t> maxExecutionUnits;
  std::vector<size_t> maxWorkGroupSize;
  std::vector<cl_ulong> MaxMemAllocSize;
  std::vector<DevicePropertiesTy> DeviceProperties;

  /// A vector of descriptors of OpenCL extensions for each device.
  std::vector<ExtensionsTy> Extensions;

  /// Default command queues for each devices
  std::vector<cl_command_queue> Queues;

  /// Inorder command queues for each devices
  std::vector<cl_command_queue> QueuesInOrder;

  /// Kernel properties for each devices
  std::vector<std::map<cl_kernel, KernelPropertiesTy>> KernelProperties;

  /// Kernel-specific implicit arguments
  std::vector<std::map<cl_kernel, std::set<void *>>> ImplicitArgs;

  /// Thread-private profile information for each devices
  std::vector<std::map<int32_t, ProfileDataTy>> Profiles;

  std::vector<std::vector<char>> Names;

  /// Whether each devices are initialized
  std::vector<bool> Initialized;

  std::vector<cl_ulong> SLMSize;

  std::mutex *Mutexes;

  std::mutex *ProfileLocks;

  std::vector<std::set<void *>> ClMemBuffers;

  /// Memory owned by the plugin
  std::vector<std::vector<void *>> OwnedMemory;

  /// Internal allocation information
  std::vector<std::unique_ptr<MemAllocInfoMapTy>> MemAllocInfo;

  /// Requires flags
  int64_t RequiresFlags = OMP_REQ_UNDEFINED;

  /// Number of active kernel launches for each device
  std::vector<uint32_t> NumActiveKernels;

  /// RTL option
  RTLOptionTy Option;

  RTLDeviceInfoTy() = default;

  /// Return per-thread profile data
  ProfileDataTy &getProfiles(int32_t DeviceId) {
    int32_t gtid = __kmpc_global_thread_num(nullptr);
    ProfileLocks[DeviceId].lock();
    auto &profiles = Profiles[DeviceId];
    if (profiles.count(gtid) == 0)
      profiles.emplace(gtid, ProfileDataTy());
    auto &profileData = profiles[gtid];
    ProfileLocks[DeviceId].unlock();
    return profileData;
  }

  /// Return context for the given device ID
  cl_context getContext(int32_t DeviceId) {
    if (Option.Flags.UseSingleContext)
      return PlatformInfos[Platforms[DeviceId]].Context;
    else
      return Contexts[DeviceId];
  }

  /// Return the extension function pointer for the given ID
  void *getExtensionFunctionPtr(int32_t DeviceId, int32_t ExtensionId) {
    auto platformId = Platforms[DeviceId];
    return PlatformInfos[platformId].ExtensionFunctionPointers[ExtensionId];
  }

  /// Return the extension function name for the given ID
  const char *getExtensionFunctionName(int32_t DeviceId, int32_t ExtensionId) {
    auto platformId = Platforms[DeviceId];
    return PlatformInfos[platformId].ExtensionFunctionNames[ExtensionId];
  }

  /// Check if extension function is available and enabled.
  bool isExtensionFunctionEnabled(int32_t DeviceId, int32_t ExtensionId) {
    if (!getExtensionFunctionPtr(DeviceId, ExtensionId))
      return false;

    switch (ExtensionId) {
    case clGetMemAllocInfoINTELId:
    case clHostMemAllocINTELId:
    case clDeviceMemAllocINTELId:
    case clSharedMemAllocINTELId:
    case clMemFreeINTELId:
    case clSetKernelArgMemPointerINTELId:
    case clEnqueueMemcpyINTELId:
      return Extensions[DeviceId].UnifiedSharedMemory == ExtensionStatusEnabled;
    case clGetDeviceGlobalVariablePointerINTELId:
      return Extensions[DeviceId].GetDeviceGlobalVariablePointer ==
          ExtensionStatusEnabled;
    case clGetKernelSuggestedLocalWorkSizeINTELId:
      return Extensions[DeviceId].SuggestedGroupSize == ExtensionStatusEnabled;
#if INTEL_CUSTOMIZATION
    case clGitsIndirectAllocationOffsetsId:
      return Extensions[DeviceId].GitsIndirectAllocationOffsets ==
          ExtensionStatusEnabled;
#endif // INTEL_CUSTOMIZATION
    default:
      return true;
    }
  }

  /// Reset program data
  int32_t resetProgramData(int32_t DeviceId);

  /// Allocate cl_mem data
  void *allocDataClMem(int32_t DeviceId, size_t Size);

  /// Get PCI device ID
  uint32_t getPCIDeviceId(int32_t DeviceId);

  /// Get device arch
  uint64_t getDeviceArch(int32_t DeviceId);

  /// Get allocated memory type
  cl_unified_shared_memory_type_intel getMemAllocType(
      int32_t DeviceId, const void *Ptr);

  /// For the given kernel return its KernelInfo auxiliary information
  /// that was previously read by readKernelInfo().
  const KernelInfoTy *
      getKernelInfo(int32_t DeviceId, const cl_kernel &Kernel) const;

  /// Get memory allocation properties to be used in memory allocation
  std::unique_ptr<std::vector<cl_mem_properties_intel>>
  getAllocMemProperties(int32_t DeviceId, size_t Size);

#if INTEL_CUSTOMIZATION
  /// Check if the device is discrete.
  bool isDiscreteDevice(int32_t DeviceId) const;
#endif // INTEL_CUSTOMIZATION
};

#ifdef _WIN32
#define __ATTRIBUTE__(X)
#else
#define __ATTRIBUTE__(X)  __attribute__((X))
#endif // _WIN32

static RTLDeviceInfoTy *DeviceInfo = nullptr;

__ATTRIBUTE__(constructor(101)) void init() {
  DP("Init OpenCL plugin!\n");
  DeviceInfo = new RTLDeviceInfoTy();
}

__ATTRIBUTE__(destructor(101)) void deinit() {
  DP("Deinit OpenCL plugin!\n");
  delete DeviceInfo;
  DeviceInfo = nullptr;
}

#if _WIN32
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

// Helper class to collect time intervals for host and device.
// The interval is managed by start()/stop() methods.
// The automatic flush of the time interval happens at the object's
// destruction.
class ProfileIntervalTy {
  // A timer interval may be either disabled, paused or running.
  // Interval may switch from paused to running and from running
  // to paused. Interval may switch to disabled start from any state,
  // and it cannot switch to disabled to anything else.
  enum TimerStatusTy {
    Disabled,
    Paused,
    Running
  };

  // Cumulative times collected by this interval so far.
  double DeviceElapsed = 0.0;
  double HostElapsed = 0.0;

  // Temporary timer values initialized at each interval
  // (re)start.
  cl_ulong DeviceTimeTemp = 0;
  cl_ulong HostTimeTemp = 0;

  // The interval name as seen in the profile data output.
  std::string Name;

  // OpenMP device id.
  int32_t DeviceId;

  // OpenCL device id.
  cl_device_id ClDeviceId;

  // Current status of the interval.
  TimerStatusTy Status;

public:
  // Create new timer interval for the given OpenMP device
  // and with the given name (which will be used for the profile
  // data output).
  ProfileIntervalTy(const char *Name, int32_t DeviceId)
    : Name(Name), DeviceId(DeviceId),
      ClDeviceId(DeviceInfo->Devices[DeviceId]) {
    if (DeviceInfo->Option.Flags.EnableProfile)
      // Start the interval paused.
      Status = TimerStatusTy::Paused;
    else
      // Disable the interval for good.
      Status = TimerStatusTy::Disabled;
  }

  // The destructor automatically updates the profile data.
  ~ProfileIntervalTy() {
    if (Status == TimerStatusTy::Disabled)
      return;
    if (Status == TimerStatusTy::Running) {
      Status = TimerStatusTy::Disabled;
      WARNING("profiling timer '%s' for OpenMP device (%" PRId32 ") %s "
              "is disabled due to start/stop mismatch.\n",
              Name.c_str(), DeviceId, DeviceInfo->Names[DeviceId].data());
      return;
    }

    DeviceInfo->getProfiles(DeviceId).update(
        Name.c_str(), HostElapsed, DeviceElapsed);
  }

  // Trigger interval start.
  void start() {
    if (Status == TimerStatusTy::Disabled)
      return;
    if (Status == TimerStatusTy::Running) {
      Status = TimerStatusTy::Disabled;
      WARNING("profiling timer '%s' for OpenMP device (%" PRId32 ") %s "
              "is disabled due to start/stop mismatch.\n",
              Name.c_str(), DeviceId, DeviceInfo->Names[DeviceId].data());
      return;
    }
    cl_int rc;
    CALL_CL(rc, clGetDeviceAndHostTimer, ClDeviceId, &DeviceTimeTemp,
            &HostTimeTemp);
    if (rc != CL_SUCCESS) {
      Status = TimerStatusTy::Disabled;
      WARNING("profiling timer '%s' for OpenMP device (%" PRId32 ") %s "
              "is disabled due to invalid OpenCL timer.\n",
              Name.c_str(), DeviceId, DeviceInfo->Names[DeviceId].data());
      return;
    }
    Status = TimerStatusTy::Running;
  }

  // Trigger interval stop (actually, a pause).
  void stop() {
    if (Status == TimerStatusTy::Disabled)
      return;
    if (Status == TimerStatusTy::Paused) {
      Status = TimerStatusTy::Disabled;
      WARNING("profiling timer '%s' for OpenMP device (%" PRId32 ") %s "
              "is disabled due to start/stop mismatch.\n",
              Name.c_str(), DeviceId, DeviceInfo->Names[DeviceId].data());
      return;
    }

    cl_ulong DeviceTime, HostTime;
    cl_int rc;
    CALL_CL(rc, clGetDeviceAndHostTimer, ClDeviceId, &DeviceTime, &HostTime);
    if (rc != CL_SUCCESS) {
      Status = TimerStatusTy::Disabled;
      WARNING("profiling timer '%s' for OpenMP device (%" PRId32 ") %s "
              "is disabled due to invalid OpenCL timer.\n",
              Name.c_str(), DeviceId, DeviceInfo->Names[DeviceId].data());
      return;
    }

    if (DeviceTime < DeviceTimeTemp || HostTime < HostTimeTemp) {
      Status = TimerStatusTy::Disabled;
      WARNING("profiling timer '%s' for OpenMP device (%" PRId32 ") %s "
              "is disabled due to timer overflow.\n",
              Name.c_str(), DeviceId, DeviceInfo->Names[DeviceId].data());
      return;
    }

    DeviceElapsed +=
        static_cast<double>(DeviceTime) - static_cast<double>(DeviceTimeTemp);
    HostElapsed +=
        static_cast<double>(HostTime) - static_cast<double>(HostTimeTemp);
    Status = TimerStatusTy::Paused;
  }
}; // ProfileIntervalTy

/// Clean-up routine to be registered by std::atexit().
static void closeRTL() {
  for (uint32_t i = 0; i < DeviceInfo->NumDevices; i++) {
    if (!DeviceInfo->Initialized[i])
      continue;
    if (DeviceInfo->Option.Flags.EnableProfile) {
      for (auto &profile : DeviceInfo->Profiles[i])
        profile.second.printData(i, profile.first, DeviceInfo->Names[i].data(),
                                 DeviceInfo->Option.ProfileResolution);
    }
#if INTEL_CUSTOMIZATION
    if (OMPT_ENABLED) {
      OMPT_CALLBACK(ompt_callback_device_unload, i, 0 /* module ID */);
      OMPT_CALLBACK(ompt_callback_device_finalize, i);
    }
#endif // INTEL_CUSTOMIZATION

    CALL_CL_EXIT_FAIL(clReleaseCommandQueue, DeviceInfo->Queues[i]);

    if (DeviceInfo->QueuesInOrder[i])
      CALL_CL_EXIT_FAIL(clReleaseCommandQueue, DeviceInfo->QueuesInOrder[i]);

    for (auto mem : DeviceInfo->OwnedMemory[i])
      CALL_CL_EXT_VOID(i, clMemFreeINTEL, DeviceInfo->getContext(i), mem);

    if (!DeviceInfo->Option.Flags.UseSingleContext)
      CALL_CL_EXIT_FAIL(clReleaseContext, DeviceInfo->Contexts[i]);

    DeviceInfo->Programs[i].clear();
  }

  if (DeviceInfo->Option.Flags.UseSingleContext)
    for (auto platformInfo : DeviceInfo->PlatformInfos)
      CALL_CL_EXIT_FAIL(clReleaseContext, platformInfo.second.Context);

  delete[] DeviceInfo->Mutexes;
  delete[] DeviceInfo->ProfileLocks;
  DP("Closed RTL successfully\n");
}

static std::string getDeviceRTLPath(const char *BaseName) {
  std::string RTLPath;
#ifdef _WIN32
  char Path[_MAX_PATH];
  HMODULE Module = nullptr;
  if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                          GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR) &DeviceInfo, &Module))
    return RTLPath;
  if (!GetModuleFileNameA(Module, Path, sizeof(Path)))
    return RTLPath;
  RTLPath = Path;
#else
  Dl_info RTLInfo;
  if (!dladdr(&DeviceInfo, &RTLInfo))
    return RTLPath;
  RTLPath = RTLInfo.dli_fname;
#endif
  size_t Split = RTLPath.find_last_of("/\\");
  RTLPath.replace(Split + 1, std::string::npos, BaseName);
  return RTLPath;
}

static inline void addDataTransferLatency() {
  if (!DeviceInfo->Option.Flags.CollectDataTransferLatency)
    return;
  double goal = omp_get_wtime() + 1e-6 * DeviceInfo->Option.DataTransferLatency;
  // Naive spinning should be enough
  while (omp_get_wtime() < goal)
    ;
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

static void dumpImageToFile(
    const void *Image, size_t ImageSize, const char *Type) {
#if INTEL_CUSTOMIZATION
#if INTEL_INTERNAL_BUILD
  if (DebugLevel <= 0)
    return;

  if (!std::getenv("LIBOMPTARGET_SAVE_TEMPS"))
    return;

  char TmpFileName[] = "omptarget_opencl_image_XXXXXX";
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
#endif  // INTEL_CUSTOMIZATION
}

static void debugPrintBuildLog(cl_program program, cl_device_id did) {
  if (DebugLevel <= 0 && !DeviceInfo->Option.Flags.ShowBuildLog)
    return;

  size_t len = 0;
  CALL_CL_RET_VOID(clGetProgramBuildInfo, program, did, CL_PROGRAM_BUILD_LOG, 0,
                   nullptr, &len);
  // The len must actually be bigger than 0 always, because the log string
  // is null-terminated.
  if (len == 0)
    return;

  std::vector<char> buffer(len);
  CALL_CL_RET_VOID(clGetProgramBuildInfo, program, did, CL_PROGRAM_BUILD_LOG,
                   len, buffer.data(), nullptr);
  const char *buildLog = (len > 1) ? buffer.data() : "<empty>";
  MESSAGE0("Target build log:");
  std::stringstream Str(buildLog);
  std::string Line;
  while(std::getline(Str, Line, '\n'))
    MESSAGE("  %s", Line.c_str());
}

static cl_program createProgramFromFile(const char *BaseName,
                                        int32_t DeviceId) {
  std::string RTLPath = getDeviceRTLPath(BaseName);
  std::ifstream RTLFile(RTLPath, std::ios::binary);

  if (RTLFile.is_open()) {
    DP("Found device RTL: %s\n", RTLPath.c_str());
    RTLFile.seekg(0, RTLFile.end);
    int RTLSize = RTLFile.tellg();
    std::string RTL(RTLSize, '\0');
    RTLFile.seekg(0);
    if (!RTLFile.read(&RTL[0], RTLSize)) {
      DP("I/O Error: Failed to read device RTL.\n");
      return nullptr;
    }

    dumpImageToFile(RTL.c_str(), RTLSize, BaseName);

    cl_int RC;
    cl_program PGM;
    CALL_CL_RVRC(PGM, clCreateProgramWithIL, RC,
                 DeviceInfo->getContext(DeviceId), RTL.c_str(), RTLSize);
    if (RC != CL_SUCCESS) {
      DP("Error: Failed to create device RTL from IL: %d\n", RC);
      return nullptr;
    }

    DeviceInfo->Option.CommonSpecConstants.setProgramConstants(DeviceId, PGM);

    return PGM;
  }

  DP("Cannot find device RTL: %s\n", RTLPath.c_str());
  return nullptr;
}

static inline void *dataAlloc(int32_t DeviceId, int64_t Size, void *HstPtr,
    void *HstBase, bool ImplicitArg, cl_uint Align = 0) {
  intptr_t Offset = (intptr_t)HstPtr - (intptr_t)HstBase;
  // If the offset is negative, then for our practical purposes it can be
  // considered 0 because the base address of an array will be contained
  // within or after the allocated memory.
  intptr_t MeaningfulOffset = Offset >= 0 ? Offset : 0;
  // If the offset is negative and the size we map is not large enough to reach
  // the base, then we must allocate extra memory up to the base (+1 to include
  // at least the first byte the base is pointing to).
  int64_t MeaningfulSize =
      Offset < 0 && abs(Offset) >= Size ? abs(Offset) + 1 : Size;

  void *Base = nullptr;
  auto Context = DeviceInfo->getContext(DeviceId);
  size_t AllocSize = MeaningfulSize + MeaningfulOffset;
  int32_t AllocKind = TARGET_ALLOC_DEVICE;

  ProfileIntervalTy DataAllocTimer("DataAlloc", DeviceId);
  DataAllocTimer.start();

  if (DeviceInfo->Option.Flags.UseSVM) {
    AllocKind = TARGET_ALLOC_SVM;
    CALL_CL_RV(Base, clSVMAlloc, Context, CL_MEM_READ_WRITE, AllocSize, Align);
  } else {
    if (!DeviceInfo->isExtensionFunctionEnabled(DeviceId,
                                                clDeviceMemAllocINTELId)) {
      DP("Error: Extension %s is not supported\n",
         DeviceInfo->getExtensionFunctionName(DeviceId,
                                              clDeviceMemAllocINTELId));
      return nullptr;
    }
    cl_int RC;
    auto AllocProp = DeviceInfo->getAllocMemProperties(DeviceId, AllocSize);
    CALL_CL_EXT_RVRC(DeviceId, Base, clDeviceMemAllocINTEL, RC, Context,
                     DeviceInfo->Devices[DeviceId], AllocProp->data(),
                     AllocSize, Align);
    if (RC != CL_SUCCESS)
      return nullptr;
  }
  if (!Base) {
    DP("Error: Failed to allocate base buffer\n");
    return nullptr;
  }
  DP("Created base buffer " DPxMOD " during data alloc\n", DPxPTR(Base));

  void *Ret = (void *)((intptr_t)Base + MeaningfulOffset);

  if (ImplicitArg) {
    DP("Stashing an implicit argument " DPxMOD " for next kernel\n",
       DPxPTR(Ret));
  }
  DeviceInfo->MemAllocInfo[DeviceId]->add(
      Ret, Base, Size, AllocKind, false, ImplicitArg);

  DataAllocTimer.stop();

  return Ret;
}

static void *dataAllocExplicit(
    int32_t DeviceId, int64_t Size, int32_t Kind, cl_uint Align = 0) {
  auto Device = DeviceInfo->Devices[DeviceId];
  auto Context = DeviceInfo->getContext(DeviceId);
  cl_int RC;
  void *Mem = nullptr;
  ProfileIntervalTy DataAllocTimer("DataAlloc", DeviceId);
  DataAllocTimer.start();
  auto ID = DeviceId;
  auto AllocProp = DeviceInfo->getAllocMemProperties(DeviceId, Size);

  switch (Kind) {
  case TARGET_ALLOC_DEVICE:
    Mem = dataAlloc(DeviceId, Size, nullptr, nullptr, true /* ImplicitArg */,
                    Align);
    break;
  case TARGET_ALLOC_HOST:
    if (DeviceInfo->Option.Flags.UseSingleContext)
      ID = DeviceInfo->NumDevices;
    if (!DeviceInfo->isExtensionFunctionEnabled(DeviceId,
                                                clHostMemAllocINTELId)) {
      DP("Host memory allocator is not available\n");
      return nullptr;
    }
    CALL_CL_EXT_RVRC(DeviceId, Mem, clHostMemAllocINTEL, RC, Context,
                     AllocProp->data(), Size, Align);
    if (Mem) {
      DeviceInfo->MemAllocInfo[ID]->add(
          Mem, Mem, Size, Kind, false /* InPool */, true /* IsImplicitArg */);
      DP("Allocated a host memory object " DPxMOD "\n", DPxPTR(Mem));
    }
    break;
  case TARGET_ALLOC_SHARED:
    if (!DeviceInfo->isExtensionFunctionEnabled(
          DeviceId, clSharedMemAllocINTELId)) {
      DP("Shared memory allocator is not available\n");
      return nullptr;
    }
    CALL_CL_EXT_RVRC(DeviceId, Mem, clSharedMemAllocINTEL, RC, Context, Device,
                     AllocProp->data(), Size, Align);
    if (Mem) {
      DeviceInfo->MemAllocInfo[ID]->add(
          Mem, Mem, Size, Kind, false /* InPool */, true /* IsImplicitArg */);
      DP("Allocated a shared memory object " DPxMOD "\n", DPxPTR(Mem));
    }
    break;
  default:
    FATAL_ERROR("Invalid target data allocation kind");
  }

  DataAllocTimer.stop();

  return Mem;
}

static int32_t submitData(int32_t device_id, void *tgt_ptr, void *hst_ptr,
                          int64_t size) {
  if (size == 0)
    // All other plugins seem to be handling 0 size gracefully,
    // so we should do as well.
    return OFFLOAD_SUCCESS;

  cl_command_queue queue = DeviceInfo->Queues[device_id];

  // Add synthetic delay for experiments
  addDataTransferLatency();

  const char *ProfileKey = "DataWrite (Host to Device)";

  if (DeviceInfo->Option.Flags.UseBuffer) {
    std::unique_lock<std::mutex> lock(DeviceInfo->Mutexes[device_id]);
    if (DeviceInfo->ClMemBuffers[device_id].count(tgt_ptr) > 0) {
      cl_event event;
      CALL_CL_RET_FAIL(clEnqueueWriteBuffer, queue, (cl_mem)tgt_ptr, CL_FALSE,
                       0, size, hst_ptr, 0, nullptr, &event);
      CALL_CL_RET_FAIL(clWaitForEvents, 1, &event);
      if (DeviceInfo->Option.Flags.EnableProfile)
        DeviceInfo->getProfiles(device_id).update(ProfileKey, event);

      return OFFLOAD_SUCCESS;
    }
  }

  if (!DeviceInfo->Option.Flags.UseSVM) {
    if (!DeviceInfo->isExtensionFunctionEnabled(device_id,
                                                clEnqueueMemcpyINTELId)) {
      DP("Error: Extension %s is not supported\n",
         DeviceInfo->getExtensionFunctionName(device_id,
                                              clEnqueueMemcpyINTELId));
      return OFFLOAD_FAIL;
    }
    cl_event event;
    CALL_CL_EXT_RET_FAIL(device_id, clEnqueueMemcpyINTEL, queue, CL_FALSE,
                         tgt_ptr, hst_ptr, size, 0, nullptr, &event);
    CALL_CL_RET_FAIL(clWaitForEvents, 1, &event);
    if (DeviceInfo->Option.Flags.EnableProfile)
      DeviceInfo->getProfiles(device_id).update(ProfileKey, event);

    return OFFLOAD_SUCCESS;
  }

  switch (DeviceInfo->Option.DataTransferMethod) {
  case DATA_TRANSFER_METHOD_SVMMAP: {
    cl_event event;
    ProfileIntervalTy SubmitTime(ProfileKey, device_id);
    SubmitTime.start();

    CALL_CL_RET_FAIL(clEnqueueSVMMap, queue, CL_TRUE, CL_MAP_WRITE, tgt_ptr,
                     size, 0, nullptr, nullptr);
    memcpy(tgt_ptr, hst_ptr, size);
    CALL_CL_RET_FAIL(clEnqueueSVMUnmap, queue, tgt_ptr, 0, nullptr, &event);
    CALL_CL_RET_FAIL(clWaitForEvents, 1, &event);

    SubmitTime.stop();
  } break;
  case DATA_TRANSFER_METHOD_SVMMEMCPY: {
    cl_event event;
    CALL_CL_RET_FAIL(clEnqueueSVMMemcpy, queue, CL_TRUE, tgt_ptr, hst_ptr,
                     size, 0, nullptr, &event);
    if (DeviceInfo->Option.Flags.EnableProfile)
      DeviceInfo->getProfiles(device_id).update(ProfileKey, event);
  } break;
  case DATA_TRANSFER_METHOD_CLMEM:
  default: {
    cl_event event;
    cl_int rc;
    cl_mem mem = nullptr;
    CALL_CL_RVRC(mem, clCreateBuffer, rc, DeviceInfo->getContext(device_id),
                 CL_MEM_USE_HOST_PTR, size, tgt_ptr);
    if (rc != CL_SUCCESS) {
      DP("Error: Failed to create a buffer from a SVM pointer " DPxMOD "\n",
         DPxPTR(tgt_ptr));
      return OFFLOAD_FAIL;
    }
    CALL_CL_RET_FAIL(clEnqueueWriteBuffer, queue, mem, CL_FALSE, 0, size,
                     hst_ptr, 0, nullptr, &event);
    CALL_CL_RET_FAIL(clWaitForEvents, 1, &event);
    CALL_CL_RET_FAIL(clReleaseMemObject, mem);
    if (DeviceInfo->Option.Flags.EnableProfile)
      DeviceInfo->getProfiles(device_id).update(ProfileKey, event);
  }
  }
  return OFFLOAD_SUCCESS;
}

static int32_t retrieveData(int32_t device_id, void *hst_ptr, void *tgt_ptr,
                            int64_t size) {
  if (size == 0)
    // All other plugins seem to be handling 0 size gracefully,
    // so we should do as well.
    return OFFLOAD_SUCCESS;

  cl_command_queue queue = DeviceInfo->Queues[device_id];

  // Add synthetic delay for experiments
  addDataTransferLatency();

  const char *ProfileKey = "DataRead (Device to Host)";

  if (DeviceInfo->Option.Flags.UseBuffer) {
    std::unique_lock<std::mutex> lock(DeviceInfo->Mutexes[device_id]);
    if (DeviceInfo->ClMemBuffers[device_id].count(tgt_ptr) > 0) {
      cl_event event;
      CALL_CL_RET_FAIL(clEnqueueReadBuffer, queue, (cl_mem)tgt_ptr, CL_FALSE,
                       0, size, hst_ptr, 0, nullptr, &event);
      CALL_CL_RET_FAIL(clWaitForEvents, 1, &event);
      if (DeviceInfo->Option.Flags.EnableProfile)
        DeviceInfo->getProfiles(device_id).update(ProfileKey, event);

      return OFFLOAD_SUCCESS;
    }
  }

  if (!DeviceInfo->Option.Flags.UseSVM) {
    if (!DeviceInfo->isExtensionFunctionEnabled(device_id,
                                                clEnqueueMemcpyINTELId)) {
      DP("Error: Extension %s is not supported\n",
         DeviceInfo->getExtensionFunctionName(device_id,
                                              clEnqueueMemcpyINTELId));
      return OFFLOAD_FAIL;
    }
    cl_event event;
    CALL_CL_EXT_RET_FAIL(device_id, clEnqueueMemcpyINTEL, queue, CL_FALSE,
                         hst_ptr, tgt_ptr, size, 0, nullptr, &event);
    CALL_CL_RET_FAIL(clWaitForEvents, 1, &event);
    if (DeviceInfo->Option.Flags.EnableProfile)
      DeviceInfo->getProfiles(device_id).update(ProfileKey, event);

    return OFFLOAD_SUCCESS;
  }

  switch (DeviceInfo->Option.DataTransferMethod) {
  case DATA_TRANSFER_METHOD_SVMMAP: {
    cl_event event;
    ProfileIntervalTy RetrieveTime(ProfileKey, device_id);
    RetrieveTime.start();

    CALL_CL_RET_FAIL(clEnqueueSVMMap, queue, CL_TRUE, CL_MAP_READ, tgt_ptr,
                     size, 0, nullptr, nullptr);
    memcpy(hst_ptr, tgt_ptr, size);
    CALL_CL_RET_FAIL(clEnqueueSVMUnmap, queue, tgt_ptr, 0, nullptr, &event);
    CALL_CL_RET_FAIL(clWaitForEvents, 1, &event);

    RetrieveTime.stop();
  } break;
  case DATA_TRANSFER_METHOD_SVMMEMCPY: {
    cl_event event;
    CALL_CL_RET_FAIL(clEnqueueSVMMemcpy, queue, CL_TRUE, hst_ptr, tgt_ptr,
                     size, 0, nullptr, &event);
    if (DeviceInfo->Option.Flags.EnableProfile)
      DeviceInfo->getProfiles(device_id).update(ProfileKey, event);
  } break;
  case DATA_TRANSFER_METHOD_CLMEM:
  default: {
    cl_int rc;
    cl_event event;
    cl_mem mem = nullptr;
    CALL_CL_RVRC(mem, clCreateBuffer, rc, DeviceInfo->getContext(device_id),
                 CL_MEM_USE_HOST_PTR, size, tgt_ptr);
    if (rc != CL_SUCCESS) {
      DP("Error: Failed to create a buffer from a SVM pointer " DPxMOD "\n",
         DPxPTR(tgt_ptr));
      return OFFLOAD_FAIL;
    }
    CALL_CL_RET_FAIL(clEnqueueReadBuffer, queue, mem, CL_FALSE, 0, size,
                     hst_ptr, 0, nullptr, &event);
    CALL_CL_RET_FAIL(clWaitForEvents, 1, &event);
    CALL_CL_RET_FAIL(clReleaseMemObject, mem);
    if (DeviceInfo->Option.Flags.EnableProfile)
      DeviceInfo->getProfiles(device_id).update(ProfileKey, event);
  }
  }
  return OFFLOAD_SUCCESS;
}

// Return the number of total HW threads required to execute
// a loop kernel compiled with the given SIMDWidth, and the given
// loop(s) trip counts and group sizes.
// Returns UINT64_MAX, if computations overflow.
static uint64_t computeThreadsNeeded(
    const size_t (&TripCounts)[3], const size_t (&GroupSizes)[3],
    uint32_t SIMDWidth) {
  uint64_t GroupCount[3];
  for (int I = 0; I < 3; ++I) {
    if (TripCounts[I] == 0 || GroupSizes[I] == 0)
      return (std::numeric_limits<uint64_t>::max)();
    GroupCount[I] =
        (uint64_t(TripCounts[I]) + GroupSizes[I] - 1) / GroupSizes[I];
    if (GroupCount[I] > (std::numeric_limits<uint32_t>::max)())
      return (std::numeric_limits<uint64_t>::max)();
  }
  for (int I = 1; I < 3; ++I) {
    if ((std::numeric_limits<uint64_t>::max)() / GroupCount[0] < GroupCount[I])
      return (std::numeric_limits<uint64_t>::max)();
    GroupCount[0] *= GroupCount[I];
  }
  // Multiplication of the group sizes must never overflow uint64_t
  // for any existing device.
  uint64_t LocalWorkSize =
      uint64_t(GroupSizes[0]) * GroupSizes[1] * GroupSizes[2];
  uint64_t ThreadsPerWG = ((LocalWorkSize + SIMDWidth - 1) / SIMDWidth);

  // Check that the total number of threads fits uint64_t.
  if ((std::numeric_limits<uint64_t>::max)() / GroupCount[0] < ThreadsPerWG)
    return (std::numeric_limits<uint64_t>::max)();

  return GroupCount[0] * ThreadsPerWG;
}

static void decideLoopKernelGroupArguments(
    int32_t DeviceId, int32_t ThreadLimit, TgtNDRangeDescTy *LoopLevels,
    cl_kernel Kernel, size_t *GroupSizes, size_t *GroupCounts) {

  size_t MaxGroupSize = DeviceInfo->maxWorkGroupSize[DeviceId];
  auto &KernelPR = DeviceInfo->KernelProperties[DeviceId][Kernel];
  size_t KernelWidth = KernelPR.Width;
  DP("Assumed kernel SIMD width is %zu\n", KernelPR.SIMDWidth);
  DP("Preferred team size is multiple of %zu\n", KernelWidth);

  // Set correct max group size if the kernel was compiled with explicit SIMD
  auto &DevicePR = DeviceInfo->DeviceProperties[DeviceId];
  if (KernelPR.SIMDWidth == 1) {
    MaxGroupSize = DevicePR.NumEUsPerSubslice * DevicePR.NumThreadsPerEU;
  }

  size_t KernelMaxThreadGroupSize = KernelPR.MaxThreadGroupSize;
  if (KernelMaxThreadGroupSize < MaxGroupSize) {
    MaxGroupSize = KernelMaxThreadGroupSize;
    DP("Capping maximum team size to %zu due to kernel constraints.\n",
       MaxGroupSize);
  }

  bool MaxGroupSizeForced = false;

  if (ThreadLimit > 0) {
    MaxGroupSizeForced = true;

    if ((uint32_t)ThreadLimit <= MaxGroupSize) {
      MaxGroupSize = ThreadLimit;
      DP("Max team size is set to %zu (thread_limit clause)\n", MaxGroupSize);
    } else {
      DP("thread_limit(%" PRIu32 ") exceeds current maximum %zu\n",
         ThreadLimit, MaxGroupSize);
    }
  }

  if (DeviceInfo->Option.ThreadLimit > 0) {
    MaxGroupSizeForced = true;

    if (DeviceInfo->Option.ThreadLimit <= MaxGroupSize) {
      MaxGroupSize = DeviceInfo->Option.ThreadLimit;
      DP("Max team size is set to %zu (OMP_THREAD_LIMIT)\n", MaxGroupSize);
    } else {
      DP("OMP_THREAD_LIMIT(%" PRIu32 ") exceeds current maximum %zu\n",
         DeviceInfo->Option.ThreadLimit, MaxGroupSize);
    }
  }

  if (DeviceInfo->Option.NumTeams > 0)
    DP("OMP_NUM_TEAMS(%" PRIu32 ") is ignored\n", DeviceInfo->Option.NumTeams);

  GroupCounts[0] = GroupCounts[1] = GroupCounts[2] = 1;
  size_t GRPSizes[3] = {MaxGroupSize, 1, 1};
  TgtLoopDescTy *Levels = LoopLevels->Levels;
  int32_t DistributeDim = LoopLevels->DistributeDim;
  assert(DistributeDim >= 0 && DistributeDim <= 2 &&
         "Invalid distribute dimension.");
  int32_t NumLoops = LoopLevels->NumLoops;
  assert((NumLoops > 0 && NumLoops <= 3) &&
         "Invalid loop nest description for ND partitioning");

  // Compute global widths for X/Y/Z dimensions.
  size_t TripCounts[3] = {1, 1, 1};

  for (int32_t I = 0; I < NumLoops; I++) {
    assert(Levels[I].Stride > 0 && "Invalid loop stride for ND partitioning");
    DP("Loop %" PRIu32 ": lower bound = %" PRId64 ", upper bound = %" PRId64
       ", Stride = %" PRId64 "\n",
       I, Levels[I].Lb, Levels[I].Ub, Levels[I].Stride);
    if (Levels[I].Ub < Levels[I].Lb)
      TripCounts[I] = 0;
    else
      TripCounts[I] =
          (Levels[I].Ub - Levels[I].Lb + Levels[I].Stride) / Levels[I].Stride;
  }

  // Check if any of the loop has zero iterations.
  if (TripCounts[0] == 0 || TripCounts[1] == 0 || TripCounts[2] == 0) {
    std::fill(GroupSizes, GroupSizes + 3, 1);
    std::fill(GroupCounts, GroupCounts + 3, 1);
    if (DistributeDim > 0 && TripCounts[DistributeDim] != 0) {
      // There is a distribute dimension, and the distribute loop
      // has non-zero iterations, but some inner parallel loop
      // has zero iterations. We still want to split the distribute
      // loop's iterations between many WGs (of size 1), but the inner/lower
      // dimensions should be 1x1.
      // Note that this code is currently dead, because we are not
      // hoisting the inner loops' bounds outside of the target regions.
      // The code is here just for completeness.
      size_t DistributeTripCount = TripCounts[DistributeDim];
      GroupCounts[DistributeDim] = DistributeTripCount;
    }
    return;
  }

  if (!MaxGroupSizeForced) {
    // Use clGetKernelSuggestedLocalWorkSizeINTEL to compute group sizes,
    // or fallback to setting dimension 0 width to SIMDWidth.
    // Note that in case of user-specified LWS GRPSizes[0]
    // is already set according to the specified value.
    size_t GlobalSizes[3] = {TripCounts[0], TripCounts[1], TripCounts[2]};
    if (DistributeDim > 0) {
      // There is a distribute dimension.
      GlobalSizes[DistributeDim - 1] *= GlobalSizes[DistributeDim];
      GlobalSizes[DistributeDim] = 1;
    }

    cl_int RC = CL_DEVICE_NOT_FOUND;
    size_t SuggestedGroupSizes[3] = {1, 1, 1};
    if (DeviceInfo->Option.Flags.UseDriverGroupSizes &&
        DeviceInfo->isExtensionFunctionEnabled(
            DeviceId, clGetKernelSuggestedLocalWorkSizeINTELId)) {
      CALL_CL_EXT(DeviceId, RC, clGetKernelSuggestedLocalWorkSizeINTEL,
                  DeviceInfo->Queues[DeviceId], Kernel, 3, nullptr, GlobalSizes,
                  SuggestedGroupSizes);
    }
    if (RC == CL_SUCCESS) {
      GRPSizes[0] = SuggestedGroupSizes[0];
      GRPSizes[1] = SuggestedGroupSizes[1];
      GRPSizes[2] = SuggestedGroupSizes[2];
    } else {
      if (MaxGroupSize > KernelWidth) {
        GRPSizes[0] = KernelWidth;
      }
      if (DistributeDim == 0 &&
          // We need to know exact number of HW threads available
          // on the device, so we need cl_intel_device_attribute_query
          // extension to be supported.
          DeviceInfo->Extensions[DeviceId].DeviceAttributeQuery ==
          ExtensionStatusEnabled) {
        // If there is a distribute dimension, then we do not use
        // thin HW threads, since we do not know anything about
        // the iteration space of the inner parallel loop regions.
        //
        // If there is no distribute dimension, then try to use thiner
        // HW threads to get more independent HW threads executing
        // the kernel - this may allow more parallelism due to
        // the stalls being distributed across multiple HW threads rather
        // than across SIMD lanes within one HW thread.
        assert(GRPSizes[1] == 1 && GRPSizes[2] == 1 &&
               "Unexpected team sizes for dimensions 1 or/and 2.");
        uint32_t SIMDWidth = KernelPR.SIMDWidth;
        uint32_t NumEUsPerSubslice = DevicePR.NumEUsPerSubslice;
        uint32_t NumSubslices = DevicePR.NumSlices *
            DevicePR.NumSubslicesPerSlice;
        uint32_t NumThreadsPerEU = DevicePR.NumThreadsPerEU;
        uint64_t TotalThreads =
            uint64_t(NumThreadsPerEU) * NumEUsPerSubslice * NumSubslices;
        TotalThreads *= DeviceInfo->Option.ThinThreadsThreshold;

        uint64_t GRPSizePrev = GRPSizes[0];
        uint64_t ThreadsNeeded =
            computeThreadsNeeded(TripCounts, GRPSizes, SIMDWidth);
        while (ThreadsNeeded < TotalThreads) {
          GRPSizePrev = GRPSizes[0];
          // Try to half the local work size (if possible) and see
          // how many HW threads the kernel will require with this
          // new local work size.
          // In most implementations the initial GRPSizes[0]
          // will be a power-of-two.
          if (GRPSizes[0] <= 1)
            break;
          GRPSizes[0] >>= 1;
          ThreadsNeeded = computeThreadsNeeded(TripCounts, GRPSizes, SIMDWidth);
        }
        GRPSizes[0] = GRPSizePrev;
      }
    }
  }

  for (int32_t I = 0; I < NumLoops; I++) {
    if (I < DistributeDim) {
      GroupCounts[I] = 1;
      continue;
    }
    size_t Trip = TripCounts[I];
    if (GRPSizes[I] >= Trip)
      GRPSizes[I] = Trip;
    GroupCounts[I] = (Trip + GRPSizes[I] - 1) / GRPSizes[I];
  }
  std::copy(GRPSizes, GRPSizes + 3, GroupSizes);
}

static void decideKernelGroupArguments(
    int32_t DeviceId, int32_t NumTeams, int32_t ThreadLimit,
    cl_kernel Kernel, size_t *GroupSizes, size_t *GroupCounts) {
#if INTEL_CUSTOMIZATION
  // Default to best GEN9 GT4 configuration initially.
  size_t NumSubslices = 9;
  size_t NumEUsPerSubslice= 8;
  size_t NumThreadsPerEU = 7;
  size_t NumEUs = DeviceInfo->maxExecutionUnits[DeviceId];
  if (DeviceInfo->Option.DeviceType == CL_DEVICE_TYPE_GPU) {
    // Use cl_intel_device_attribute_query if available.
    if (DeviceInfo->Extensions[DeviceId].DeviceAttributeQuery ==
        ExtensionStatusEnabled) {
      auto &P = DeviceInfo->DeviceProperties[DeviceId];
      NumEUsPerSubslice = P.NumEUsPerSubslice;
      NumThreadsPerEU = P.NumThreadsPerEU;
      NumSubslices = P.NumSlices * P.NumSubslicesPerSlice;
    } else if (NumEUs >= 256) {
      // Newer GPUs.
      NumEUsPerSubslice = 16;
      NumSubslices = NumEUs / NumEUsPerSubslice;
      NumThreadsPerEU = 8;
    } else if (DeviceInfo->DeviceArchs[DeviceId] == DeviceArch_XeLP) {
      NumEUsPerSubslice = 16;
      NumSubslices = NumEUs / NumEUsPerSubslice;
      NumThreadsPerEU = 7;
    } else if (NumEUs >= 72) {
      // Default GEN9 GT4 configuration.
    } else if (NumEUs >= 48) {
      // GT3
      NumSubslices = 6;
    } else if (NumEUs >= 24) {
      // GT2
      NumSubslices = 3;
    } else if (NumEUs >= 18) {
      // GT1.5
      NumSubslices = 3;
      NumEUsPerSubslice = 6;
    } else {
      // GT1
      NumSubslices = 2;
      NumEUsPerSubslice = 6;
    }

    DPI("NumEUsPerSubslice: %zu\n", NumEUsPerSubslice);
    DPI("NumSubslices: %zu\n", NumSubslices);
    DPI("NumThreadsPerEU: %zu\n", NumThreadsPerEU);
    DPI("TotalEUs: %zu\n", NumEUs);
  }
#endif // INTEL_CUSTOMIZATION
  const KernelInfoTy *KInfo = DeviceInfo->getKernelInfo(DeviceId, Kernel);
  if (!KInfo) {
    DP("Warning: Cannot find kernel information for kernel " DPxMOD ".\n",
       DPxPTR(Kernel));
  }
  size_t MaxGroupSize = DeviceInfo->maxWorkGroupSize[DeviceId];
  bool MaxGroupSizeForced = false;
  bool MaxGroupCountForced = false;

  auto &KernelPR = DeviceInfo->KernelProperties[DeviceId][Kernel];
  size_t KernelWidth = KernelPR.Width;
#if INTEL_CUSTOMIZATION
  size_t SIMDWidth = KernelPR.SIMDWidth;
  DP("Assumed kernel SIMD width is %zu\n", SIMDWidth);
#endif // INTEL_CUSTOMIZATION
  DP("Preferred team size is multiple of %zu\n", KernelWidth);

  // Set correct max group size if the kernel was compiled with explicit SIMD
  auto &DevicePR = DeviceInfo->DeviceProperties[DeviceId];
  if (KernelPR.SIMDWidth == 1) {
    MaxGroupSize = DevicePR.NumEUsPerSubslice * DevicePR.NumThreadsPerEU;
  }

  size_t KernelMaxThreadGroupSize = KernelPR.MaxThreadGroupSize;
  if (KernelMaxThreadGroupSize < MaxGroupSize) {
    MaxGroupSize = KernelMaxThreadGroupSize;
    DP("Capping maximum team size to %zu due to kernel constraints.\n",
       MaxGroupSize);
  }

  if (ThreadLimit > 0) {
    MaxGroupSizeForced = true;

    if ((uint32_t)ThreadLimit <= MaxGroupSize) {
      MaxGroupSize = ThreadLimit;
      DP("Max team size is set to %zu (thread_limit clause)\n",
         MaxGroupSize);
    } else {
      DP("thread_limit(%" PRIu32 ") exceeds current maximum %zu\n",
         ThreadLimit, MaxGroupSize);
    }
  }

  if (DeviceInfo->Option.ThreadLimit > 0) {
    MaxGroupSizeForced = true;

    if (DeviceInfo->Option.ThreadLimit <= MaxGroupSize) {
      MaxGroupSize = DeviceInfo->Option.ThreadLimit;
      DP("Max team size is set to %zu (OMP_THREAD_LIMIT)\n", MaxGroupSize);
    } else {
      DP("OMP_THREAD_LIMIT(%" PRIu32 ") exceeds current maximum %zu\n",
         DeviceInfo->Option.ThreadLimit, MaxGroupSize);
    }
  }

  size_t MaxGroupCount = 0;

  if (NumTeams > 0) {
    MaxGroupCount = NumTeams;
    MaxGroupCountForced = true;
    DP("Max number of teams is set to %zu "
       "(num_teams clause or no teams construct)\n", MaxGroupCount);
  } else if (DeviceInfo->Option.NumTeams > 0) {
    // OMP_NUM_TEAMS only matters, if num_teams() clause is absent.
    MaxGroupCount = DeviceInfo->Option.NumTeams;
    MaxGroupCountForced = true;
    DP("Max number of teams is set to %zu (OMP_NUM_TEAMS)\n", MaxGroupCount);
  }

  if (MaxGroupCountForced) {
    // If number of teams is specified by the user, then use KernelWidth
    // WIs per WG by default, so that it matches
    // decideLoopKernelGroupArguments() behavior.
    if (!MaxGroupSizeForced) {
      MaxGroupSize = KernelWidth;
    }
  } else {
    MaxGroupCount = DeviceInfo->maxExecutionUnits[DeviceId];
#if INTEL_CUSTOMIZATION
    if (DeviceInfo->Option.DeviceType == CL_DEVICE_TYPE_GPU) {
      // A work group is partitioned into EU threads,
      // and then scheduled onto a sub slice. A sub slice must have all the
      // resources available to start a work group, otherwise it will wait
      // for resources. This means that uneven partitioning may result
      // in waiting work groups, and also unused EUs.
      // See slides 25-27 here:
      //   https://software.intel.com/sites/default/files/      \
      //   Faster-Better-Pixels-on-the-Go-and-in-the-Cloud-     \
      //   with-OpenCL-on-Intel-Architecture.pdf
      size_t NumThreadsPerSubslice = NumEUsPerSubslice * NumThreadsPerEU;
      MaxGroupCount = NumSubslices * NumThreadsPerSubslice;
      if (MaxGroupSizeForced) {
        // Set group size for the HW capacity
        size_t NumThreadsPerGroup = (MaxGroupSize + SIMDWidth - 1) / SIMDWidth;
        size_t numGroupsPerSubslice =
            (NumThreadsPerSubslice + NumThreadsPerGroup - 1) /
            NumThreadsPerGroup;
        MaxGroupCount = numGroupsPerSubslice * NumSubslices;
      } else {
        // For kernels with cross-WG reductions use LWS equal
        // to KernelWidth. This is just a performance heuristic.
        if (KInfo && KInfo->getHasTeamsReduction() &&
            // Only do this for discrete devices.
            DeviceInfo->isDiscreteDevice(DeviceId) &&
            DeviceInfo->Option.ReductionSubscriptionRate &&
            // Do not use this heuristic for kernels that use
            // atomic-free reductions. We want to maximize
            // LWS for such kernels.
            !KInfo->isAtomicFreeReduction())
          MaxGroupSize = KernelWidth;

        assert(!MaxGroupSizeForced && !MaxGroupCountForced);
        assert((MaxGroupSize <= KernelWidth ||
                MaxGroupSize % KernelWidth == 0) && "Invalid MaxGroupSize");
        // Maximize group size
        while (MaxGroupSize >= KernelWidth) {
          size_t NumThreadsPerGroup =
              (MaxGroupSize + SIMDWidth - 1) / SIMDWidth;
          if (NumThreadsPerSubslice % NumThreadsPerGroup == 0) {
            size_t numGroupsPerSubslice =
                NumThreadsPerSubslice / NumThreadsPerGroup;
            MaxGroupCount = numGroupsPerSubslice * NumSubslices;
            break;
          }
          MaxGroupSize -= KernelWidth;
        }
      }
    }
#endif  // INTEL_CUSTOMIZATION
  }

  GroupSizes[0] = MaxGroupSize;
  GroupSizes[1] = GroupSizes[2] = 1;

  if (KInfo && KInfo->getWINum()) {
    GroupSizes[0] =
        (std::min)(KInfo->getWINum(), static_cast<uint64_t>(GroupSizes[0]));
    DP("Capping maximum team size to %" PRIu64
       " due to kernel constraints (reduction).\n", KInfo->getWINum());
  }

  GroupCounts[0] = MaxGroupCount;
  GroupCounts[1] = GroupCounts[2] = 1;
  if (!MaxGroupCountForced) {
    if (KInfo && KInfo->getHasTeamsReduction() &&
        DeviceInfo->Option.ReductionSubscriptionRate) {
#if INTEL_CUSTOMIZATION
      if (DeviceInfo->isDiscreteDevice(DeviceId)) {
        // Do not apply ReductionSubscriptionRate for non-discrete devices.
        // But we have to avoid the regular SubscriptionRate in the else
        // clause. Basically, for non-discrete devices, the reduction
        // subscription rate is 1.
#endif // INTEL_CUSTOMIZATION
      if (!KInfo->isAtomicFreeReduction() ||
          !DeviceInfo->Option.ReductionSubscriptionRateIsDefault) {
        // Use reduction subscription rate 1 for kernels using
        // atomic-free reductions, unless user forced reduction subscription
        // rate via environment.
        GroupCounts[0] /= DeviceInfo->Option.ReductionSubscriptionRate;
        GroupCounts[0] = (std::max)(GroupCounts[0], size_t(1));
      }
#if INTEL_CUSTOMIZATION
      }
#endif // INTEL_CUSTOMIZATION
    } else {
      GroupCounts[0] *= DeviceInfo->Option.SubscriptionRate;
    }
  }

  if (KInfo && KInfo->getWGNum()) {
    GroupCounts[0] =
        (std::min)(KInfo->getWGNum(), static_cast<uint64_t>(GroupCounts[0]));
    DP("Capping maximum thread groups count to %" PRIu64
       " due to kernel constraints (reduction).\n", KInfo->getWGNum());
  }
}

static inline int32_t runTargetTeamNDRegion(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs,
    ptrdiff_t *TgtOffsets, int32_t NumArgs, int32_t NumTeams,
    int32_t ThreadLimit, void *LoopDesc) {

  cl_kernel Kernel = *static_cast<cl_kernel *>(TgtEntryPtr);
  if (!Kernel) {
    REPORT("Failed to invoke deleted kernel.\n");
    return OFFLOAD_FAIL;
  }

  // Libomptarget can pass negative NumTeams and ThreadLimit now after
  // introducing __tgt_target_kernel. This happens only when we have valid
  // LoopDesc and the region is not a teams region.
  if (NumTeams < 0)
    NumTeams = 0;
  if (ThreadLimit < 0)
    ThreadLimit = 0;

#if INTEL_INTERNAL_BUILD
  // TODO: kernels using to much SLM may limit the number of
  //       work groups running simultaneously on a sub slice.
  //       We may take this into account for computing the work partitioning.
  size_t DeviceLocalMemSize = (size_t)DeviceInfo->SLMSize[DeviceId];
  DP("Device local mem size: %zu\n", DeviceLocalMemSize);
  cl_ulong LocalMemSizeTmp = 0;
  CALL_CL_RET_FAIL(clGetKernelWorkGroupInfo, Kernel,
                   DeviceInfo->Devices[DeviceId], CL_KERNEL_LOCAL_MEM_SIZE,
                   sizeof(LocalMemSizeTmp), &LocalMemSizeTmp, nullptr);
  size_t KernelLocalMemSize = (size_t)LocalMemSizeTmp;
  DP("Kernel local mem size: %zu\n", KernelLocalMemSize);
#endif // INTEL_INTERNAL_BUILD

  // Decide group sizes and counts
  size_t LocalWorkSize[3] = {1, 1, 1};
  size_t NumWorkGroups[3] = {1, 1, 1};
  if (LoopDesc) {
    decideLoopKernelGroupArguments(DeviceId, ThreadLimit,
                                   (TgtNDRangeDescTy *)LoopDesc, Kernel,
                                   LocalWorkSize, NumWorkGroups);
  } else {
    decideKernelGroupArguments(DeviceId, NumTeams, ThreadLimit, Kernel,
                               LocalWorkSize, NumWorkGroups);
  }

  size_t GlobalWorkSize[3];
  for (int32_t I = 0; I < 3; ++I)
    GlobalWorkSize[I] = LocalWorkSize[I] * NumWorkGroups[I];

#if INTEL_INTERNAL_BUILD
  // Use forced group sizes. This is only for internal experiments, and we
  // don't want to plug these numbers into the decision logic.
  auto UserLWS = DeviceInfo->Option.ForcedLocalSizes;
  auto UserGWS = DeviceInfo->Option.ForcedGlobalSizes;
  if (UserLWS[0] > 0) {
    std::copy(UserLWS, UserLWS + 3, LocalWorkSize);
    DP("Forced LWS = {%zu, %zu, %zu}\n", UserLWS[0], UserLWS[1], UserLWS[2]);
  }
  if (UserGWS[0] > 0) {
    std::copy(UserGWS, UserGWS + 3, GlobalWorkSize);
    DP("Forced GWS = {%zu, %zu, %zu}\n", UserGWS[0], UserGWS[1], UserGWS[2]);
  }
#endif // INTEL_INTERNAL_BUILD

  DP("Team sizes = {%zu, %zu, %zu}\n", LocalWorkSize[0], LocalWorkSize[1],
     LocalWorkSize[2]);
  DP("Number of teams = {%zu, %zu, %zu}\n",
     GlobalWorkSize[0] / LocalWorkSize[0], GlobalWorkSize[1] / LocalWorkSize[1],
     GlobalWorkSize[2] / LocalWorkSize[2]);

  // Protect thread-unsafe OpenCL API calls
  DeviceInfo->Mutexes[DeviceId].lock();

  // Set kernel args
  for (int32_t I = 0; I < NumArgs; ++I) {
    ptrdiff_t Offset = TgtOffsets[I];
    const char *ArgType = "Unknown";
    auto *KernelInfo = DeviceInfo->getKernelInfo(DeviceId, Kernel);
    if (KernelInfo && KernelInfo->isArgLiteral(I)) {
      uint32_t Size = KernelInfo->getArgSize(I);
      CALL_CL_RET_FAIL(clSetKernelArg, Kernel, I, Size, TgtArgs[I]);
      ArgType = "ByVal";
    } else if (Offset == (std::numeric_limits<ptrdiff_t>::max)()) {
      // Offset equal to MAX(ptrdiff_t) means that the argument
      // must be passed as literal, and the offset should be ignored.
      intptr_t Arg = (intptr_t)TgtArgs[I];
      CALL_CL_RET_FAIL(clSetKernelArg, Kernel, I, sizeof(Arg), &Arg);
      ArgType = "Scalar";
    } else {
      ArgType = "Pointer";
      void *Ptr = (void *)((intptr_t)TgtArgs[I] + Offset);
      if (DeviceInfo->Option.Flags.UseBuffer &&
          DeviceInfo->ClMemBuffers[DeviceId].count(Ptr) > 0) {
        CALL_CL_RET_FAIL(clSetKernelArg, Kernel, I, sizeof(cl_mem), &Ptr);
        ArgType = "ClMem";
      } else if (DeviceInfo->Option.Flags.UseSVM) {
        CALL_CL_RET_FAIL(clSetKernelArgSVMPointer, Kernel, I, Ptr);
      } else {
        if (!DeviceInfo->isExtensionFunctionEnabled(
                DeviceId, clSetKernelArgMemPointerINTELId)) {
          DP("Error: Extension %s is not supported\n",
             DeviceInfo->getExtensionFunctionName(
                 DeviceId, clSetKernelArgMemPointerINTELId));
          return OFFLOAD_FAIL;
        }
        CALL_CL_EXT_RET_FAIL(
            DeviceId, clSetKernelArgMemPointerINTEL, Kernel, I, Ptr);
      }
    }
    DP("Kernel %s Arg %d set successfully\n", ArgType, I);
    (void)ArgType;
  }

  auto &KernelProperty = DeviceInfo->KernelProperties[DeviceId][Kernel];
  std::vector<void *> ImplicitSVMArgs;
  std::vector<void *> ImplicitUSMArgs;
  std::map<int32_t, bool> HasUSMArgs{
      {TARGET_ALLOC_DEVICE, false},
      {TARGET_ALLOC_HOST, false},
      {TARGET_ALLOC_SHARED, false}
  };
  auto &AllocInfos = DeviceInfo->MemAllocInfo;

#if INTEL_CUSTOMIZATION
  // Kernel dynamic memory is indirect access
  // FIXME: handle cases with multiple OpenCLProgramTy objects.
  auto KernelDynamicMem =
      DeviceInfo->Programs[DeviceId].back().getPGMData().DynamicMemoryLB;
  if (KernelDynamicMem) {
    ImplicitUSMArgs.push_back((void *)KernelDynamicMem);
    HasUSMArgs[TARGET_ALLOC_DEVICE] = true;
  }
  auto DynamicMemPool =
      DeviceInfo->Programs[DeviceId].back().getPGMData().DynamicMemPool;
  if (DynamicMemPool) {
    ImplicitUSMArgs.push_back((void *)DynamicMemPool);
    HasUSMArgs[TARGET_ALLOC_DEVICE] = true;
  }
#endif // INTEL_CUSTOMIZATION

  /// Kernel-dependent implicit arguments
  for (auto Ptr : KernelProperty.ImplicitArgs) {
    if (!Ptr)
      continue;
    // "Ptr" is not always the allocation information known to libomptarget, so
    // use "search" instead of "find".
    auto *Info = AllocInfos[DeviceId]->search(Ptr);
    if (Info) {
      if ((int32_t)Info->Kind == TARGET_ALLOC_SVM) {
        ImplicitSVMArgs.push_back(Ptr);
      } else {
        ImplicitUSMArgs.push_back(Ptr);
        HasUSMArgs[Info->Kind] = true;
      }
    }
    if (DeviceInfo->Option.Flags.UseSingleContext) {
      Info = AllocInfos[DeviceInfo->NumDevices]->search(Ptr);
      if (Info) {
        ImplicitUSMArgs.push_back(Ptr);
        HasUSMArgs[TARGET_ALLOC_HOST] = true;
      }
    }
  }

  /// Kernel-independent implicit arguments
  AllocInfos[DeviceId]->getImplicitArgs(ImplicitSVMArgs, ImplicitUSMArgs);
  for (auto &ArgKind : HasUSMArgs)
    if (AllocInfos[DeviceId]->hasImplicitUSMArg(ArgKind.first))
      ArgKind.second = true;
  if (DeviceInfo->Option.Flags.UseSingleContext) {
    auto ID = DeviceInfo->NumDevices;
    AllocInfos[ID]->getImplicitArgs(ImplicitSVMArgs, ImplicitUSMArgs);
    if (AllocInfos[ID]->hasImplicitUSMArg(TARGET_ALLOC_HOST))
      HasUSMArgs[TARGET_ALLOC_HOST] = true;
  }

  if (ImplicitSVMArgs.size() > 0) {
    DP("Calling clSetKernelExecInfo to pass %zu implicit SVM arguments "
       "to kernel " DPxMOD "\n", ImplicitSVMArgs.size(), DPxPTR(Kernel));
    CALL_CL_RET_FAIL(clSetKernelExecInfo, Kernel, CL_KERNEL_EXEC_INFO_SVM_PTRS,
                     sizeof(void *) * ImplicitSVMArgs.size(),
                     ImplicitSVMArgs.data());
  }

  if (ImplicitUSMArgs.size() > 0) {
    // Report non-argument USM pointers to the runtime.
    DP("Calling clSetKernelExecInfo to pass %zu implicit USM arguments "
       "to kernel " DPxMOD "\n", ImplicitUSMArgs.size(), DPxPTR(Kernel));
    CALL_CL_RET_FAIL(clSetKernelExecInfo, Kernel,
                     CL_KERNEL_EXEC_INFO_USM_PTRS_INTEL,
                     sizeof(void *) * ImplicitUSMArgs.size(),
                     ImplicitUSMArgs.data());
    // Mark the kernel as supporting indirect USM accesses, otherwise,
    // clEnqueueNDRangeKernel call below will fail.
    cl_bool KernelSupportsUSM = CL_TRUE;
    if (HasUSMArgs[TARGET_ALLOC_HOST])
      CALL_CL_RET_FAIL(clSetKernelExecInfo, Kernel,
                       CL_KERNEL_EXEC_INFO_INDIRECT_HOST_ACCESS_INTEL,
                       sizeof(cl_bool), &KernelSupportsUSM);
    if (HasUSMArgs[TARGET_ALLOC_DEVICE])
      CALL_CL_RET_FAIL(clSetKernelExecInfo, Kernel,
                       CL_KERNEL_EXEC_INFO_INDIRECT_DEVICE_ACCESS_INTEL,
                       sizeof(cl_bool), &KernelSupportsUSM);
    if (HasUSMArgs[TARGET_ALLOC_SHARED])
      CALL_CL_RET_FAIL(clSetKernelExecInfo, Kernel,
                       CL_KERNEL_EXEC_INFO_INDIRECT_SHARED_ACCESS_INTEL,
                       sizeof(cl_bool), &KernelSupportsUSM);
  }

#if INTEL_CUSTOMIZATION
  if (OMPT_ENABLED) {
    // Push current work size
    size_t FinalNumTeams =
        GlobalWorkSize[0] * GlobalWorkSize[1] * GlobalWorkSize[2];
    size_t FinalThreadLimit =
        LocalWorkSize[0] * LocalWorkSize[1] * LocalWorkSize[2];
    FinalNumTeams /= FinalThreadLimit;
    OmptGlobal->getTrace().pushWorkSize(FinalNumTeams, FinalThreadLimit);
  }
#endif // INTEL_CUSTOMIZATION

  cl_event Event;
#if INTEL_CUSTOMIZATION
  OCL_KERNEL_BEGIN(DeviceId);
#endif // INTEL_CUSTOMIZATION
  CALL_CL_RET_FAIL(clEnqueueNDRangeKernel, DeviceInfo->Queues[DeviceId],
                   Kernel, 3, nullptr, GlobalWorkSize,
                   LocalWorkSize, 0, nullptr, &Event);

  DeviceInfo->Mutexes[DeviceId].unlock();

  DP("Started executing kernel.\n");

  CALL_CL_RET_FAIL(clWaitForEvents, 1, &Event);
#if INTEL_CUSTOMIZATION
  OCL_KERNEL_END(DeviceId);
#endif // INTEL_CUSTOMIZATION
  if (DeviceInfo->Option.Flags.EnableProfile) {
    std::vector<char> Buf;
    size_t BufSize;
    CALL_CL_RET_FAIL(clGetKernelInfo, Kernel, CL_KERNEL_FUNCTION_NAME, 0,
                     nullptr, &BufSize);
    std::string KernelName("Kernel ");
    if (BufSize > 0) {
      Buf.resize(BufSize);
      CALL_CL_RET_FAIL(clGetKernelInfo, Kernel, CL_KERNEL_FUNCTION_NAME,
                       Buf.size(), Buf.data(), nullptr);
      KernelName += Buf.data();
    }
    DeviceInfo->getProfiles(DeviceId).update(KernelName.c_str(), Event);
  }
  DP("Successfully finished kernel execution.\n");

  return OFFLOAD_SUCCESS;
}

#if INTEL_CUSTOMIZATION
int32_t RTLDeviceInfoTy::resetProgramData(int32_t DeviceId) {
  for (auto &PGM : Programs[DeviceId])
    if (PGM.resetProgramData() != OFFLOAD_SUCCESS)
      return OFFLOAD_FAIL;

  return OFFLOAD_SUCCESS;
}
#endif // INTEL_CUSTOMIZATION

void *RTLDeviceInfoTy::allocDataClMem(int32_t DeviceId, size_t Size) {
  cl_mem ret = nullptr;
  cl_int rc;

  CALL_CL_RVRC(ret, clCreateBuffer, rc, getContext(DeviceId),
               CL_MEM_READ_WRITE, Size, nullptr);
  if (rc != CL_SUCCESS)
    return nullptr;

  std::unique_lock<std::mutex> lock(Mutexes[DeviceId]);
  ClMemBuffers[DeviceId].insert((void *)ret);

  DP("Allocated cl_mem data " DPxMOD "\n", DPxPTR(ret));
  return (void *)ret;
}

uint32_t RTLDeviceInfoTy::getPCIDeviceId(int32_t DeviceId) {
  if (Extensions[DeviceId].DeviceAttributeQuery == ExtensionStatusEnabled)
    return DeviceProperties[DeviceId].DeviceId;

  uint32_t Id = 0;
#ifndef _WIN32
  // Linux: Device name contains "[0xABCD]" device identifier.
  if (Option.DeviceType == CL_DEVICE_TYPE_GPU) {
    std::string DeviceName(Names[DeviceId].data());
    auto P = DeviceName.rfind("[");
    if (P != std::string::npos && DeviceName.size() - P >= 8)
      Id = std::strtol(DeviceName.substr(P + 1, 6).c_str(), nullptr, 16);
  }
#endif
  return Id;
}

uint64_t RTLDeviceInfoTy::getDeviceArch(int32_t DeviceId) {
  if (Option.DeviceType == CL_DEVICE_TYPE_CPU)
    return DeviceArch_x86_64;

  uint32_t PCIDeviceId = getPCIDeviceId(DeviceId);
  if (PCIDeviceId != 0) {
    for (auto &Arch : DeviceArchMap)
      for (auto Id : Arch.second)
        if (PCIDeviceId == Id || (PCIDeviceId & 0xFF00) == Id)
          return Arch.first;  // Exact match or prefix match
  }

  std::string DeviceName(Names[DeviceId].data());
#ifdef _WIN32
  // Windows: Device name contains published product name.
  for (auto &Arch : DeviceArchStrMap)
    for (auto Str : Arch.second)
      if (DeviceName.find(Str) != std::string::npos)
        return Arch.first;
#endif

  DP("Warning: Cannot decide device arch for %s.\n", DeviceName.c_str());
  return DeviceArch_None;
}

cl_unified_shared_memory_type_intel RTLDeviceInfoTy::getMemAllocType(
    int32_t DeviceId, const void *Ptr) {
  cl_unified_shared_memory_type_intel MemType = CL_MEM_TYPE_UNKNOWN_INTEL;
  CALL_CL_EXT_RET(DeviceId, MemType, clGetMemAllocInfoINTEL,
                  getContext(DeviceId), Ptr, CL_MEM_ALLOC_TYPE_INTEL,
                  sizeof(MemType), &MemType, nullptr);
  return MemType;
}

const KernelInfoTy *RTLDeviceInfoTy::getKernelInfo(
    int32_t DeviceId, const cl_kernel &Kernel) const {
  for (auto &Program : Programs[DeviceId]) {
    auto *KernelInfo = Program.getKernelInfo(Kernel);
    if (KernelInfo)
      return KernelInfo;
  }

  return nullptr;
}

// Get memory attributes for the given allocation size.
std::unique_ptr<std::vector<cl_mem_properties_intel>>
RTLDeviceInfoTy::getAllocMemProperties(int32_t DeviceId, size_t Size) {
  std::vector<cl_mem_properties_intel> Properties;
#if INTEL_CUSTOMIZATION
  size_t MaxSize = MaxMemAllocSize[DeviceId];
  if (Option.DeviceType == CL_DEVICE_TYPE_GPU && Size > MaxSize) {
    Properties.push_back(CL_MEM_FLAGS_INTEL);
    Properties.push_back(CL_MEM_ALLOW_UNRESTRICTED_SIZE_INTEL);
  }
#endif // INTEL_CUSTOMIZATION
  Properties.push_back(0);

  return std::make_unique<std::vector<cl_mem_properties_intel>>(
      std::move(Properties));
}

#if INTEL_CUSTOMIZATION
bool RTLDeviceInfoTy::isDiscreteDevice(int32_t DeviceId) const {
  switch (DeviceArchs[DeviceId]) {
  case DeviceArch_XeHP:
    return true;

  case DeviceArch_Gen9:
  // FIXME: if needed, we should handle discrete cards from XeLP family
  //        properly. XeLP is currently a mix.
  case DeviceArch_XeLP:
  default:
    return false;
  }
}
#endif // INTEL_CUSTOMIZATION

void SpecConstantsTy::setProgramConstants(
    int32_t DeviceId, cl_program Program) const {
  cl_int Rc;

  if (!DeviceInfo->isExtensionFunctionEnabled(
          DeviceId, clSetProgramSpecializationConstantId)) {
    DP("Error: Extension %s is not supported.\n",
       DeviceInfo->getExtensionFunctionName(
           DeviceId, clSetProgramSpecializationConstantId));
    return;
  }

  for (int I = ConstantValues.size(); I > 0; --I) {
    cl_uint Id = static_cast<cl_uint>(ConstantIds[I - 1]);
    size_t Size = ConstantValueSizes[I - 1];
    const void *Val = ConstantValues[I - 1];
    CALL_CL_EXT_SILENT(DeviceId, Rc, clSetProgramSpecializationConstant,
                       Program, Id, Size, Val);
    if (Rc == CL_SUCCESS)
      DP("Set specialization constant '0x%X'\n", static_cast<int32_t>(Id));
  }
}

int32_t ExtensionsTy::getExtensionsInfoForDevice(int32_t DeviceNum) {
  // Identify the size of OpenCL extensions string.
  size_t RetSize = 0;
  // If the below call fails, some extensions's status may be
  // left ExtensionStatusUnknown, so only ExtensionStatusEnabled
  // actually means that the extension is enabled.
  DP("Getting extensions for device %d\n", DeviceNum);

  cl_device_id DeviceId = DeviceInfo->Devices[DeviceNum];
  CALL_CL_RET_FAIL(clGetDeviceInfo, DeviceId, CL_DEVICE_EXTENSIONS, 0, nullptr,
                   &RetSize);

  std::unique_ptr<char []> Data(new char[RetSize]);
  CALL_CL_RET_FAIL(clGetDeviceInfo, DeviceId, CL_DEVICE_EXTENSIONS, RetSize,
                   Data.get(), &RetSize);

  std::string Extensions(Data.get());
  DP("Device extensions: %s\n", Extensions.c_str());

  if (UnifiedSharedMemory == ExtensionStatusUnknown &&
      Extensions.find("cl_intel_unified_shared_memory") != std::string::npos) {
    UnifiedSharedMemory = ExtensionStatusEnabled;
    DP("Extension UnifiedSharedMemory enabled.\n");
  }

  if (DeviceAttributeQuery == ExtensionStatusUnknown &&
      Extensions.find("cl_intel_device_attribute_query") != std::string::npos) {
    DeviceAttributeQuery = ExtensionStatusEnabled;
    DP("Extension DeviceAttributeQuery enabled.\n");
  }

  // Check if the extension was not explicitly disabled, i.e.
  // that its current status is unknown.
  if (GetDeviceGlobalVariablePointer == ExtensionStatusUnknown)
    // FIXME: use the right extension name.
    if (Extensions.find("") != std::string::npos) {
      GetDeviceGlobalVariablePointer = ExtensionStatusEnabled;
      DP("Extension clGetDeviceGlobalVariablePointerINTEL enabled.\n");
    }

  if (SuggestedGroupSize == ExtensionStatusUnknown)
    // FIXME: use the right extension name.
    if (Extensions.find("") != std::string::npos) {
      SuggestedGroupSize = ExtensionStatusEnabled;
      DP("Extension clGetKernelSuggestedLocalWorkSizeINTEL enabled.\n");
    }

  std::for_each(LibdeviceExtensions.begin(), LibdeviceExtensions.end(),
                [&Extensions](LibdeviceExtDescTy &Desc) {
                  if (Desc.Status == ExtensionStatusUnknown)
                    if (Extensions.find(Desc.Name) != std::string::npos) {
                      Desc.Status = ExtensionStatusEnabled;
                      DP("Extension %s enabled.\n", Desc.Name);
                    }
                });

  return CL_SUCCESS;
}

int32_t DevicePropertiesTy::getDeviceProperties(cl_device_id ID) {
  CALL_CL_RET_FAIL(clGetDeviceInfo, ID, CL_DEVICE_ID_INTEL, sizeof(cl_uint),
                   &DeviceId, nullptr);
  CALL_CL_RET_FAIL(clGetDeviceInfo, ID, CL_DEVICE_NUM_SLICES_INTEL,
                   sizeof(cl_uint), &NumSlices, nullptr);
  CALL_CL_RET_FAIL(clGetDeviceInfo, ID,
                   CL_DEVICE_NUM_SUB_SLICES_PER_SLICE_INTEL, sizeof(cl_uint),
                   &NumSubslicesPerSlice, nullptr);
  CALL_CL_RET_FAIL(clGetDeviceInfo, ID, CL_DEVICE_NUM_EUS_PER_SUB_SLICE_INTEL,
                   sizeof(cl_uint), &NumEUsPerSubslice, nullptr);
  CALL_CL_RET_FAIL(clGetDeviceInfo, ID, CL_DEVICE_NUM_THREADS_PER_EU_INTEL,
                   sizeof(cl_uint), &NumThreadsPerEU, nullptr);

  NumHWThreads =
      NumSlices * NumSubslicesPerSlice * NumEUsPerSubslice * NumThreadsPerEU;

  return OFFLOAD_SUCCESS;
}

OpenCLProgramTy::~OpenCLProgramTy() {
  for (auto Kernel : Kernels) {
    if (Kernel)
      CALL_CL_RET_VOID(clReleaseKernel, Kernel);
  }
  for (auto PGM : Programs) {
    CALL_CL_RET_VOID(clReleaseProgram, PGM);
  }
  if (RequiresProgramLink) {
    CALL_CL_RET_VOID(clReleaseProgram, FinalProgram);
  }
  // Unload offload entries
  for (auto &Entry : OffloadEntries)
    delete[] Entry.Base.name;
}

/// Add program read from a single section
int32_t OpenCLProgramTy::addProgramIL(const size_t Size,
                                      const unsigned char *Image) {
  cl_program PGM;
  cl_int RC;
  CALL_CL_RVRC(PGM, clCreateProgramWithIL, RC, Context, Image, Size);

  auto Flags = DeviceInfo->Option.Flags;

  if (RC != CL_SUCCESS || Flags.ShowBuildLog)
    debugPrintBuildLog(PGM, Device);

  if (RC != CL_SUCCESS) {
    DP("Error: Failed to create program from SPIR-V: %d\n", RC);
    return OFFLOAD_FAIL;
  }

  DeviceInfo->Option.CommonSpecConstants.setProgramConstants(DeviceId, PGM);
  Programs.push_back(PGM);
  IsBinary = false;

  // First SPIR-V image is expected to be the only image or the first image
  // that contains global information. We also add fallback libdevice image
  // here if required.
  if (Programs.size() == 1 && Flags.LinkLibDevice) {
    auto &Extensions = DeviceInfo->Extensions[DeviceId].LibdeviceExtensions;
    for (auto &ExtDesc : Extensions) {
      if (ExtDesc.Status == ExtensionStatusEnabled) {
        DP("Fallback libdevice RTL %s is not required.\n",
           ExtDesc.FallbackLibName);
        continue;
      }
      // Device runtime does not support this libdevice extension,
      // so we have to link in the fallback implementation.
      //
      // TODO: the device image must specify which libdevice extensions
      //       are actually required. We should link only the required
      //       fallback implementations.
      PGM = createProgramFromFile(ExtDesc.FallbackLibName, DeviceId);
      if (PGM) {
        DP("Added fallback libdevice RTL %s.\n", ExtDesc.FallbackLibName);
        Programs.push_back(PGM);
      } else {
        DP("Cannot add fallback libdeice RTL %s.\n", ExtDesc.FallbackLibName);
      }
    }
  }
  return OFFLOAD_SUCCESS;
}

int32_t OpenCLProgramTy::addProgramBIN(const size_t Size,
                                       const unsigned char *Image) {
  cl_program PGM;
  cl_int RC;
  CALL_CL_RVRC(PGM, clCreateProgramWithBinary, RC, Context, 1, &Device,
               &Size, &Image, nullptr);

  if (RC != CL_SUCCESS || DeviceInfo->Option.Flags.ShowBuildLog)
    debugPrintBuildLog(PGM, Device);

  if (RC != CL_SUCCESS) {
    DP("Error: Failed to create program from binary: %d\n", RC);
    return OFFLOAD_FAIL;
  }

  DeviceInfo->Option.CommonSpecConstants.setProgramConstants(DeviceId, PGM);
  Programs.push_back(PGM);
  IsBinary = true;
  return OFFLOAD_SUCCESS;
}

int32_t OpenCLProgramTy::buildPrograms(std::string &CompilationOptions,
                                       std::string &LinkingOptions) {
  int32_t RC;

  uint64_t MajorVer, MinorVer;
  if (!isValidOneOmpImage(Image, MajorVer, MinorVer)) {
    // Handle legacy plain SPIR-V image.
    char *ImgBegin = reinterpret_cast<char *>(Image->ImageStart);
    char *ImgEnd = reinterpret_cast<char *>(Image->ImageEnd);
    size_t ImgSize = ImgEnd - ImgBegin;
    dumpImageToFile(ImgBegin, ImgSize, "OpenMP");
    if (addProgramIL(ImgSize, (unsigned char *)ImgBegin) != OFFLOAD_SUCCESS)
      return OFFLOAD_FAIL;

    return OFFLOAD_SUCCESS;
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

    if (It->second.Format == 0) {
      // Native format.
      RC = addProgramBIN(ImgSize, ImgBegin);
    } else if (It->second.Format == 1) {
      // SPIR-V format.
      RC = addProgramIL(ImgSize, ImgBegin);
    } else {
      DP("Warning: image %" PRIu64 "is ignored due to unknown format.\n", Idx);
      continue;
    }

    if (RC != OFFLOAD_SUCCESS)
      continue;

    DP("Created offload program from image #%" PRIu64 ".\n", Idx);
    if (DeviceInfo->Option.Flags.UseImageOptions) {
      CompilationOptions += " " + It->second.CompileOpts;
      LinkingOptions += " " + It->second.LinkOpts;
    }

    return OFFLOAD_SUCCESS;
  }

  return OFFLOAD_FAIL;
}

int32_t OpenCLProgramTy::compilePrograms(std::string &CompOptions,
                                         std::string &LinkOptions) {
  if (IsBinary && Programs.size() > 1) {
    // Nothing to be done for split-kernel binaries as later linking step
    // falls back to SPIR-V recompilation.
    DP("Skipping compilation for multiple binary images.\n");
    RequiresProgramLink = true;
    return OFFLOAD_SUCCESS;
  }

  cl_int RC;
  auto &Flags = DeviceInfo->Option.Flags;

  if (Programs.size() == 1 &&
      (IsBinary || Flags.EnableSimd ||
      // Work around GPU API issue: clCompileProgram/clLinkProgram
      // does not work with -vc-codegen, so we have to use clBuildProgram.
      CompOptions.find(" -vc-codegen ") != std::string::npos)) {
    auto BuildOptions = CompOptions + " " + LinkOptions;
    CALL_CL(RC, clBuildProgram, Programs[0], 0, nullptr, BuildOptions.c_str(),
            nullptr, nullptr);
    if (RC != CL_SUCCESS || Flags.ShowBuildLog) {
      debugPrintBuildLog(Programs[0], Device);
      if (RC != CL_SUCCESS) {
        DP("Error: Failed to build program: %d\n", RC);
        return OFFLOAD_FAIL;
      }
    }
    RequiresProgramLink = false;
    return OFFLOAD_SUCCESS;
  }

  // Single or multiple SPIR-V programs are compiled
  for (auto &PGM : Programs) {
    CALL_CL(RC, clCompileProgram, PGM, 0, nullptr, CompOptions.c_str(), 0,
            nullptr, nullptr, nullptr, nullptr);
    if (RC != CL_SUCCESS || Flags.ShowBuildLog) {
      debugPrintBuildLog(PGM, Device);
      if (RC != CL_SUCCESS) {
        DP("Error: Failed to compile program: %d\n", RC);
        return OFFLOAD_FAIL;
      }
    }
  }

  RequiresProgramLink = true;
  return OFFLOAD_SUCCESS;
}

int32_t OpenCLProgramTy::linkPrograms(std::string &LinkOptions) {
  if (!RequiresProgramLink) {
    FinalProgram = Programs[0];
    DP("Program linking is not required.\n");
    return OFFLOAD_SUCCESS;
  }

  cl_int RC;
  CALL_CL_RVRC(FinalProgram, clLinkProgram, RC, Context, 1, &Device,
               LinkOptions.c_str(), Programs.size(), Programs.data(), nullptr,
               nullptr);
  if (RC != CL_SUCCESS || DeviceInfo->Option.Flags.ShowBuildLog) {
    debugPrintBuildLog(FinalProgram, Device);
    if (RC != CL_SUCCESS) {
      DP("Error: Failed to link program: %d\n", RC);
      return OFFLOAD_FAIL;
    }
  }

  DP("Successfully linked %zu programs.\n", Programs.size());

  return OFFLOAD_SUCCESS;
}

int32_t OpenCLProgramTy::buildKernels() {
  size_t NumEntries = (size_t)(Image->EntriesEnd - Image->EntriesBegin);

  Entries.resize(NumEntries);
  Kernels.resize(NumEntries);

  ProfileIntervalTy EntriesTimer("OffloadEntriesInit", DeviceId);
  EntriesTimer.start();
  if (!loadOffloadTable(NumEntries))
    DP("Warning: could not load offload table.\n");
  EntriesTimer.stop();

  // We are supposed to have a single final program at this point
  for (size_t I = 0; I < NumEntries; I++) {
    // Size is 0 means that it is kernel function.
    auto Size = Image->EntriesBegin[I].size;
    char *Name = Image->EntriesBegin[I].name;

    if (Size != 0) {
      EntriesTimer.start();
      void *HostAddr = Image->EntriesBegin[I].addr;
      void *TgtAddr = getOffloadVarDeviceAddr(Name, Size);

      if (!TgtAddr) {
        TgtAddr = __tgt_rtl_data_alloc(DeviceId, Size, HostAddr,
                                       TARGET_ALLOC_DEFAULT);
        __tgt_rtl_data_submit(DeviceId, TgtAddr, HostAddr, Size);
        DP("Warning: global variable '%s' allocated. "
           "Direct references will not work properly.\n", Name);
      }

      DP("Global variable mapped: Name = %s, Size = %zu, "
         "HostPtr = " DPxMOD ", TgtPtr = " DPxMOD "\n",
         Name, Size, DPxPTR(HostAddr), DPxPTR(TgtAddr));
      Entries[I].addr = TgtAddr;
      Entries[I].name = Name;
      Entries[I].size = Size;
      Kernels[I] = nullptr;
      EntriesTimer.stop();
      continue;
    }

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
#endif  // _WIN32
    cl_int RC;
    CALL_CL_RVRC(Kernels[I], clCreateKernel, RC, FinalProgram, Name);
    if (RC != CL_SUCCESS) {
      // If a kernel was deleted by optimizations (e.g. DCE), then
      // clCreateKernel will fail. We expect that such a kernel
      // will never be actually invoked.
      DP("Warning: Failed to create kernel %s, %d\n", Name, RC);
      Kernels[I] = nullptr;
    }
    Entries[I].addr = &Kernels[I];
    Entries[I].name = Name;

    if (!Kernels[I]) {
      if (Image->EntriesBegin[I].flags & OMP_DECLARE_TARGET_FPTR) {
        // Return device function ptr for entires marked as
        // OMP_DECLARE_TARGET_FPTR and inherit flags from the host entry.
        Entries[I].flags = Image->EntriesBegin[I].flags;
        Entries[I].addr = getOffloadVarDeviceAddr(Name, Size);
        DP("Returning device function pointer " DPxMOD
           " for host function pointer " DPxMOD "\n",
           DPxPTR(Entries[I].addr), DPxPTR(Image->EntriesBegin[I].addr));
      } else {
        // Do not try to query information for deleted kernels.
        DP("Warning: cannot find kernel %s\n", Name);
      }
      continue;
    }

    if (!readKernelInfo(Entries[I])) {
      DP("Error: failed to read kernel info for kernel %s\n", Name);
      return OFFLOAD_FAIL;
    }

    // Retrieve kernel group size info.
    auto Kernel = Kernels[I];
    auto &KernelProperty = DeviceInfo->KernelProperties[DeviceId][Kernel];
    CALL_CL_RET_FAIL(clGetKernelWorkGroupInfo, Kernel, Device,
                     CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                     sizeof(size_t), &KernelProperty.Width, nullptr);
    CALL_CL_RET_FAIL(clGetKernelSubGroupInfo, Kernel, Device,
                     CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE, sizeof(size_t),
                     &KernelProperty.SIMDWidth, sizeof(size_t),
                     &KernelProperty.SIMDWidth, nullptr);
    if (KernelProperty.SIMDWidth == 0) {
      // clGetKernelSubGroupInfo is not supported on Windows with CPU device, so
      // assign default value to avoid any issues when using this variable.
      KernelProperty.SIMDWidth = KernelProperty.Width / 2;
    }

#if INTEL_CUSTOMIZATION
    if (DeviceInfo->DeviceArchs[DeviceId] != DeviceArch_Gen9) {
      // Adjust kernel width to match level_zero plugin.
      KernelProperty.Width =
          (std::max)(KernelProperty.Width, 2 * KernelProperty.SIMDWidth);
    }
#endif // INTEL_CUSTOMIZATION

    assert(KernelProperty.SIMDWidth <= KernelProperty.Width &&
           "Invalid preferred team size multiple.");
    CALL_CL_RET_FAIL(clGetKernelWorkGroupInfo, Kernel, Device,
                     CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t),
                     &KernelProperty.MaxThreadGroupSize, nullptr);

    if (DebugLevel > 0) {
      // Show kernel information
      std::vector<char> Buf;
      size_t BufSize;
      cl_uint NumArgs = 0;
      CALL_CL_RET_FAIL(clGetKernelInfo, Kernel, CL_KERNEL_NUM_ARGS,
                       sizeof(cl_uint), &NumArgs, nullptr);
      DP("Kernel %zu: Name = %s, NumArgs = %" PRIu32 "\n", I, Name, NumArgs);
      for (cl_uint J = 0; J < NumArgs; J++) {
        // clGetKernelArgInfo is not supposed to work unless the program is
        // built with clCreateProgramWithSource according to the specification.
        // We still allow this if the backend RT is capable of returning the
        // argument information without using clCreateProgramWithSource.
        CALL_CL_SILENT(RC, clGetKernelArgInfo, Kernel, J,
                       CL_KERNEL_ARG_TYPE_NAME, 0, nullptr, &BufSize);
        if (RC != CL_SUCCESS)
          break; // Kernel argument info won't be available
        Buf.resize(BufSize);
        CALL_CL_RET_FAIL(clGetKernelArgInfo, Kernel, J,
                         CL_KERNEL_ARG_TYPE_NAME, BufSize, Buf.data(), nullptr);
        std::string TypeName = Buf.data();
        CALL_CL_RET_FAIL(clGetKernelArgInfo, Kernel, J, CL_KERNEL_ARG_NAME, 0,
                         nullptr, &BufSize);
        Buf.resize(BufSize);
        CALL_CL_RET_FAIL(clGetKernelArgInfo, Kernel, J, CL_KERNEL_ARG_NAME,
                         BufSize, Buf.data(), nullptr);
        DP("  Arg %2" PRIu32 ": %s %s\n", J, TypeName.c_str(),
           Buf.data() ? Buf.data() : "undefined");
      }
    }
  }

  Table.EntriesBegin = &(Entries.data()[0]);
  Table.EntriesEnd = &(Entries.data()[Entries.size()]);

  return OFFLOAD_SUCCESS;
}

#if INTEL_CUSTOMIZATION
void *OpenCLProgramTy::initDynamicMemPool() {
  size_t MemSize = DeviceInfo->Option.KernelDynamicMemorySize;
  if (MemSize == 0)
    return nullptr;

  constexpr size_t BlockSizeMin = 64;
  constexpr uint32_t NumBlocksPerDesc = 32;
  constexpr uint32_t NumDescsPerCounter = 32;

  DynamicMemPoolTy Pool;
  Pool.HeapSize = MemSize;
  Pool.NumHeaps = 1;
  size_t SupportedSize = BlockSizeMin * NumBlocksPerDesc;
  while (SupportedSize < Pool.HeapSize) {
    SupportedSize *= (2 * NumBlocksPerDesc);
    Pool.NumHeaps++;
  }
  cl_int RC;
  size_t BaseSize = Pool.NumHeaps * Pool.HeapSize;
  auto BaseProp = DeviceInfo->getAllocMemProperties(DeviceId, BaseSize);
  CALL_CL_EXT_RVRC(DeviceId, Pool.PoolBase, clDeviceMemAllocINTEL, RC, Context,
                   Device, BaseProp->data(), BaseSize, 0);
  if (RC != CL_SUCCESS || !Pool.PoolBase)
    return nullptr;
  DeviceInfo->OwnedMemory[DeviceId].push_back(Pool.PoolBase);

  // Initialize each heap
  for (uint32_t I = 0; I < Pool.NumHeaps; I++) {
    size_t BlockSize = BlockSizeMin << (6 * I);
    auto &Heap = Pool.HeapDesc[I];
    Heap.NumBlocks = Pool.HeapSize / BlockSize;
    Heap.AllocBase = (uintptr_t)Pool.PoolBase + I * Pool.HeapSize;
    Heap.BlockSize = BlockSize;
    Heap.MaxSize = BlockSize * Heap.NumBlocks;
    size_t SupportedSize = BlockSize * NumBlocksPerDesc;
    if (Heap.MaxSize > SupportedSize)
      Heap.MaxSize = SupportedSize;
    // Prepare device memory for block descriptors
    Heap.NumBlockDesc =
        (Heap.NumBlocks + NumBlocksPerDesc - 1) / NumBlocksPerDesc;
    size_t DescSize = Heap.NumBlockDesc * sizeof(uint64_t);
    auto DescProp = DeviceInfo->getAllocMemProperties(DeviceId, DescSize);
    void *BlockDesc = nullptr;
    CALL_CL_EXT_RVRC(DeviceId, BlockDesc, clDeviceMemAllocINTEL, RC, Context,
                     Device, DescProp->data(), DescSize, 0);
    if (RC != CL_SUCCESS || !BlockDesc)
      return nullptr;
    DeviceInfo->OwnedMemory[DeviceId].push_back(BlockDesc);
    Heap.BlockDesc = (uint64_t *)BlockDesc;
    std::vector<uint64_t> BlockDescInit(Heap.NumBlockDesc, 0);
    CALL_CL_EXT_RET_NULL(DeviceId, clEnqueueMemcpyINTEL,
                         DeviceInfo->Queues[DeviceId], CL_TRUE, Heap.BlockDesc,
                         BlockDescInit.data(), DescSize, 0, nullptr, nullptr);
    // Prepare device memory for block counters
    Heap.NumBlockCounter =
        (Heap.NumBlockDesc + NumDescsPerCounter - 1) / NumDescsPerCounter;
    size_t CounterSize = Heap.NumBlockCounter * sizeof(uint32_t);
    auto CounterProp = DeviceInfo->getAllocMemProperties(DeviceId, CounterSize);
    void *BlockCounter = nullptr;
    CALL_CL_EXT_RVRC(DeviceId, BlockCounter, clDeviceMemAllocINTEL, RC, Context,
                     Device, CounterProp->data(), CounterSize, 0);
    if (RC != CL_SUCCESS || !BlockCounter)
      return nullptr;
    DeviceInfo->OwnedMemory[DeviceId].push_back(BlockCounter);
    Heap.BlockCounter = (uint32_t *)BlockCounter;
    std::vector<uint32_t> BlockCounterInit(Heap.NumBlockCounter, 0);
    CALL_CL_EXT_RET_NULL(DeviceId, clEnqueueMemcpyINTEL,
                         DeviceInfo->Queues[DeviceId], CL_TRUE,
                         Heap.BlockCounter, BlockCounterInit.data(),
                         CounterSize, 0, nullptr, nullptr);
  }

  // Prepare device copy of the pool
  void *PoolDevice = nullptr;
  auto PoolProp = DeviceInfo->getAllocMemProperties(DeviceId, sizeof(Pool));
  CALL_CL_EXT_RVRC(DeviceId, PoolDevice, clDeviceMemAllocINTEL, RC, Context,
                   Device, PoolProp->data(), sizeof(Pool), 0);
  if (RC != CL_SUCCESS || !PoolDevice)
    return nullptr;
  DeviceInfo->OwnedMemory[DeviceId].push_back(PoolDevice);
  CALL_CL_EXT_RET_NULL(DeviceId, clEnqueueMemcpyINTEL,
                       DeviceInfo->Queues[DeviceId], CL_TRUE, PoolDevice, &Pool,
                       sizeof(Pool), 0, nullptr, nullptr);

  return PoolDevice;
}

int32_t OpenCLProgramTy::initProgramData() {
  // Look up program data location on device
  PGMDataPtr = getVarDeviceAddr("__omp_spirv_program_data", sizeof(PGMData));
  if (!PGMDataPtr) {
    DP("Warning: cannot find program data location on device.\n");
    return OFFLOAD_SUCCESS;
  }

  // Prepare host data to copy
  auto &P = DeviceInfo->DeviceProperties[DeviceId];
  uint32_t TotalEUs =
      P.NumSlices * P.NumSubslicesPerSlice * P.NumEUsPerSubslice;

  // Allocate dynamic memory for in-kernel allocation
  void *MemLB = 0;
  uintptr_t MemUB = 0;
  void *MemPool = nullptr;
  size_t MemSize = DeviceInfo->Option.KernelDynamicMemorySize;
  if (DeviceInfo->Option.KernelDynamicMemoryMethod == 0) {
    if (MemSize > 0) {
      cl_int RC;
      auto AllocProp = DeviceInfo->getAllocMemProperties(DeviceId, MemSize);
      CALL_CL_EXT_RVRC(DeviceId, MemLB, clDeviceMemAllocINTEL, RC, Context,
                       Device, AllocProp->data(), MemSize, 0);
    }

    if (MemLB) {
      DeviceInfo->OwnedMemory[DeviceId].push_back(MemLB);
      MemUB = (uintptr_t)MemLB + MemSize;
    }
  } else {
    MemPool = initDynamicMemPool();
  }

  int DeviceType =
      (DeviceInfo->Option.DeviceType == CL_DEVICE_TYPE_GPU) ? 0 : 1;

  PGMData = {
    1,                   // Initialized
    (int32_t)DeviceInfo->NumDevices,
                         // Number of OpenMP devices
    DeviceId,            // Device ID
    TotalEUs,            // Total EUs
    P.NumThreadsPerEU,   // HW threads per EU
    (uintptr_t)MemLB,    // Dynamic memory LB
    MemUB,               // Dynamic memory UB
    DeviceType,          // Device type (0 for GPU, 1 for CPU)
    MemPool,             // Dynamic memory pool
    (int32_t)DeviceInfo->maxWorkGroupSize[DeviceId]
                         // Teams thread limit
  };

  CALL_CL_EXT_RET_FAIL(DeviceId, clEnqueueMemcpyINTEL,
                       DeviceInfo->Queues[DeviceId], CL_TRUE, PGMDataPtr,
                       &PGMData, sizeof(PGMData), 0, nullptr, nullptr);
  return OFFLOAD_SUCCESS;
}

int32_t OpenCLProgramTy::resetProgramData() {
  if (!PGMDataPtr)
    return OFFLOAD_SUCCESS;

  CALL_CL_EXT_RET_FAIL(DeviceId, clEnqueueMemcpyINTEL,
                       DeviceInfo->Queues[DeviceId], CL_TRUE, PGMDataPtr,
                       &PGMData, sizeof(PGMData), 0, nullptr, nullptr);
  return OFFLOAD_SUCCESS;
}
#endif // INTEL_CUSTOMIZATION

bool OpenCLProgramTy::readKernelInfo(const __tgt_offload_entry &KernelEntry) {
  const cl_kernel *KernelPtr =
      reinterpret_cast<const cl_kernel *>(KernelEntry.addr);
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
  CALL_CL_EXT_RET(DeviceId, false, clEnqueueMemcpyINTEL,
                  DeviceInfo->Queues[DeviceId],
                  /*blocking=*/CL_TRUE, InfoBuffer.data(),
                  InfoVarAddr, InfoVarSize,
                  /*num_events_in_wait_list=*/0,
                  /*event_wait_list=*/nullptr,
                  /*event=*/nullptr);
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
  // Support WINum since version 4.
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

bool OpenCLProgramTy::loadOffloadTable(size_t NumEntries) {
  const char *OffloadTableSizeVarName = "__omp_offloading_entries_table_size";
  void *OffloadTableSizeVarAddr =
      getVarDeviceAddr(OffloadTableSizeVarName, sizeof(int64_t));

  if (!OffloadTableSizeVarAddr) {
    DP("Warning: cannot get device value for global variable '%s'.\n",
       OffloadTableSizeVarName);
    return false;
  }

  int64_t TableSizeVal = 0;
  CALL_CL_EXT_RET(DeviceId, false, clEnqueueMemcpyINTEL,
                  DeviceInfo->Queues[DeviceId],
                  CL_TRUE, &TableSizeVal, OffloadTableSizeVarAddr,
                  sizeof(int64_t), 0, nullptr, nullptr);
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
  void *OffloadTableVarAddr = getVarDeviceAddr(OffloadTableVarName, TableSize);
  if (!OffloadTableVarAddr) {
    DP("Warning: cannot get device value for global variable '%s'.\n",
       OffloadTableVarName);
    return false;
  }

  OffloadEntries.resize(DeviceNumEntries);
  CALL_CL_EXT_RET(DeviceId, false, clEnqueueMemcpyINTEL,
                  DeviceInfo->Queues[DeviceId],
                  CL_TRUE, OffloadEntries.data(), OffloadTableVarAddr,
                  TableSize, 0, nullptr, nullptr);

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
    CALL_CL_EXT_RET(DeviceId, false, clEnqueueMemcpyINTEL,
                    DeviceInfo->Queues[DeviceId],
                    CL_TRUE, Entry.Base.name, NameTgtAddr, NameSize, 0, nullptr,
                    nullptr);
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

void *OpenCLProgramTy::getOffloadVarDeviceAddr(const char *Name, size_t Size) {
  DP("Looking up OpenMP global variable '%s' of size %zu bytes on device %d.\n",
     Name, Size, DeviceId);

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
  } else
    DP("Warning: offload table is not loaded for device %d.\n", DeviceId);

  // Fallback to the lookup by name.
  return getVarDeviceAddr(Name, Size);
}

void *OpenCLProgramTy::getVarDeviceAddr(const char *Name, size_t *SizePtr) {
  size_t DeviceSize = 0;
  void *TgtAddr = nullptr;
  size_t Size = *SizePtr;
  bool SizeIsKnown = (Size != 0);
  if (SizeIsKnown) {
    DP("Looking up device global variable '%s' of size %zu bytes "
       "on device %d.\n", Name, Size, DeviceId);
  } else {
    DP("Looking up device global variable '%s' of unknown size "
       "on device %d.\n", Name, DeviceId);
  }

  if (!DeviceInfo->isExtensionFunctionEnabled(
          DeviceId, clGetDeviceGlobalVariablePointerINTELId))
    return nullptr;

  cl_int RC;
  auto clGetDeviceGlobalVariablePointerINTELFn =
      reinterpret_cast<clGetDeviceGlobalVariablePointerINTEL_fn>(
          DeviceInfo->getExtensionFunctionPtr(
              DeviceId, clGetDeviceGlobalVariablePointerINTELId));
  RC = clGetDeviceGlobalVariablePointerINTELFn(
      Device, FinalProgram, Name, &DeviceSize, &TgtAddr);

  if (RC != CL_SUCCESS) {
    DP("Warning: clGetDeviceGlobalVariablePointerINTEL API returned "
       "nullptr for global variable '%s'.\n", Name);
    DeviceSize = 0;
  } else if (Size != DeviceSize && SizeIsKnown) {
    DP("Warning: size mismatch for host (%zu) and device (%zu) versions "
       "of global variable: %s\n.  Direct references "
       "to this variable will not work properly.\n",
       Size, DeviceSize, Name);
    DeviceSize = 0;
  }

  if (DeviceSize == 0) {
    DP("Warning: global variable lookup failed.\n");
    return nullptr;
  }

  DP("Global variable lookup succeeded (size: %zu bytes).\n", DeviceSize);
  *SizePtr = DeviceSize;
  return TgtAddr;
}

void *OpenCLProgramTy::getVarDeviceAddr(const char *Name, size_t Size) {
  return getVarDeviceAddr(Name, &Size);
}

const KernelInfoTy *OpenCLProgramTy::getKernelInfo(
    const cl_kernel Kernel) const {
  auto I = KernelInfo.find(Kernel);
  if (I != KernelInfo.end())
    return &(I->second);
  else
    return nullptr;
}

///
/// Common plugin interface
///

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

int32_t __tgt_rtl_number_of_devices() {
  // Assume it is thread safe, since it is called once.

  DP("Start initializing OpenCL\n");
  // get available platforms
  cl_uint Count = 0;
  CALL_CL_RET_ZERO(clGetPlatformIDs, 0, nullptr, &Count);
  std::vector<cl_platform_id> PlatformIDs(Count);
  CALL_CL_RET_ZERO(clGetPlatformIDs, Count, PlatformIDs.data(), nullptr);

  // All eligible OpenCL device IDs from the platforms are stored in a list
  // in the order they are probed by clGetPlatformIDs/clGetDeviceIDs.
  for (cl_platform_id ID : PlatformIDs) {
    std::vector<char> Buf;
    size_t BufSize;
    cl_int RC;
    CALL_CL(RC, clGetPlatformInfo, ID, CL_PLATFORM_VERSION, 0, nullptr,
            &BufSize);
    if (RC != CL_SUCCESS || BufSize == 0)
      continue;
    Buf.resize(BufSize);
    CALL_CL(RC, clGetPlatformInfo, ID, CL_PLATFORM_VERSION, BufSize,
            Buf.data(), nullptr);
    // clCreateProgramWithIL() requires OpenCL 2.1.
    if (RC != CL_SUCCESS || std::stof(std::string(Buf.data() + 6)) <= 2.0) {
      continue;
    }
    cl_uint NumDevices = 0;
    CALL_CL_SILENT(RC, clGetDeviceIDs, ID, DeviceInfo->Option.DeviceType, 0,
                   nullptr, &NumDevices);
    if (RC != CL_SUCCESS || NumDevices == 0)
      continue;

    DP("Platform %s has %" PRIu32 " Devices\n",
       Buf.data() ? Buf.data() : "undefined", NumDevices);
    std::vector<cl_device_id> Devices(NumDevices);
    CALL_CL_RET_ZERO(clGetDeviceIDs, ID, DeviceInfo->Option.DeviceType,
                     NumDevices, Devices.data(), nullptr);

    cl_context Context = nullptr;
    if (DeviceInfo->Option.Flags.UseSingleContext) {
      cl_context_properties ContextProperties[] = {
          CL_CONTEXT_PLATFORM, (cl_context_properties)ID, 0
      };
      CALL_CL_RVRC(Context, clCreateContext, RC, ContextProperties,
                   Devices.size(), Devices.data(), nullptr, nullptr);
      if (RC != CL_SUCCESS)
        continue;
    }

    DeviceInfo->PlatformInfos.emplace(ID, PlatformInfoTy(ID, Context));
    for (auto Device : Devices) {
      DeviceInfo->Devices.push_back(Device);
      DeviceInfo->Platforms.push_back(ID);
    }
    DeviceInfo->NumDevices += NumDevices;
  }

  if (!DeviceInfo->Option.Flags.UseSingleContext)
    DeviceInfo->Contexts.resize(DeviceInfo->NumDevices);
  DeviceInfo->Programs.resize(DeviceInfo->NumDevices);
  DeviceInfo->maxExecutionUnits.resize(DeviceInfo->NumDevices);
  DeviceInfo->maxWorkGroupSize.resize(DeviceInfo->NumDevices);
  DeviceInfo->MaxMemAllocSize.resize(DeviceInfo->NumDevices);
  DeviceInfo->DeviceProperties.resize(DeviceInfo->NumDevices);
  DeviceInfo->Extensions.resize(DeviceInfo->NumDevices);
  DeviceInfo->Queues.resize(DeviceInfo->NumDevices);
  DeviceInfo->QueuesInOrder.resize(DeviceInfo->NumDevices, nullptr);
  DeviceInfo->KernelProperties.resize(DeviceInfo->NumDevices);
  DeviceInfo->ClMemBuffers.resize(DeviceInfo->NumDevices);
  DeviceInfo->ImplicitArgs.resize(DeviceInfo->NumDevices);
  DeviceInfo->Profiles.resize(DeviceInfo->NumDevices);
  DeviceInfo->Names.resize(DeviceInfo->NumDevices);
  DeviceInfo->DeviceArchs.resize(DeviceInfo->NumDevices);
  DeviceInfo->Initialized.resize(DeviceInfo->NumDevices);
  DeviceInfo->SLMSize.resize(DeviceInfo->NumDevices);
  DeviceInfo->Mutexes = new std::mutex[DeviceInfo->NumDevices];
  DeviceInfo->ProfileLocks = new std::mutex[DeviceInfo->NumDevices];
  DeviceInfo->OwnedMemory.resize(DeviceInfo->NumDevices);
  DeviceInfo->NumActiveKernels.resize(DeviceInfo->NumDevices, 0);

  // Host allocation information needs one additional slot
  for (uint32_t I = 0; I < DeviceInfo->NumDevices + 1; I++)
    DeviceInfo->MemAllocInfo.emplace_back(new MemAllocInfoMapTy());

  // get device specific information
  for (unsigned I = 0; I < DeviceInfo->NumDevices; I++) {
    size_t BufSize;
    cl_int RC;
    cl_device_id DeviceId = DeviceInfo->Devices[I];
    CALL_CL(RC, clGetDeviceInfo, DeviceId, CL_DEVICE_NAME, 0, nullptr,
            &BufSize);
    if (RC != CL_SUCCESS || BufSize == 0)
      continue;
    DeviceInfo->Names[I].resize(BufSize);
    CALL_CL(RC, clGetDeviceInfo, DeviceId, CL_DEVICE_NAME, BufSize,
            DeviceInfo->Names[I].data(), nullptr);
    if (RC != CL_SUCCESS)
      continue;
    DP("Device %d: %s\n", I, DeviceInfo->Names[I].data());
    CALL_CL_RET_ZERO(clGetDeviceInfo, DeviceId, CL_DEVICE_MAX_COMPUTE_UNITS, 4,
                     &DeviceInfo->maxExecutionUnits[I], nullptr);
    DP("Number of execution units on the device is %d\n",
       DeviceInfo->maxExecutionUnits[I]);
    CALL_CL_RET_ZERO(clGetDeviceInfo, DeviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE,
                     sizeof(size_t), &DeviceInfo->maxWorkGroupSize[I], nullptr);
    DP("Maximum work group size for the device is %d\n",
       static_cast<int32_t>(DeviceInfo->maxWorkGroupSize[I]));
    CALL_CL_RET_ZERO(clGetDeviceInfo, DeviceId, CL_DEVICE_MAX_MEM_ALLOC_SIZE,
                     sizeof(cl_ulong), &DeviceInfo->MaxMemAllocSize[I],
                     nullptr);
    DP("Maximum memory allocation size is %" PRIu64 "\n",
       DeviceInfo->MaxMemAllocSize[I]);
    CALL_CL_RET_ZERO(clGetDeviceInfo, DeviceId, CL_DEVICE_LOCAL_MEM_SIZE,
                     sizeof(cl_ulong), &DeviceInfo->SLMSize[I], nullptr);
    DP("Device local mem size: %zu\n", (size_t)DeviceInfo->SLMSize[I]);
    DeviceInfo->Initialized[I] = false;
  }
  if (DeviceInfo->NumDevices == 0) {
    DP("WARNING: No OpenCL devices found.\n");
  }

#ifndef _WIN32
  // Make sure it is registered after OCL handlers are registered.
  // Registerization is done in DLLmain for Windows
  if (std::atexit(closeRTL)) {
    FATAL_ERROR("Registration of clean-up function");
  }
#endif //WIN32

  return DeviceInfo->NumDevices;
}

int32_t __tgt_rtl_init_device(int32_t DeviceId) {
  cl_int RC;
  DP("Initialize OpenCL device\n");
  assert(DeviceId >= 0 && (cl_uint)DeviceId < DeviceInfo->NumDevices &&
         "bad device id");

  // Use out-of-order queue by default.
  std::vector<cl_queue_properties> QProperties {
      CL_QUEUE_PROPERTIES,
      CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE
  };
  if (DeviceInfo->Option.Flags.EnableProfile)
    QProperties.back() |= CL_QUEUE_PROFILING_ENABLE;
  QProperties.push_back(0);

  if (!DeviceInfo->Option.Flags.UseSingleContext) {
    auto Platform = DeviceInfo->Platforms[DeviceId];
    auto Device = DeviceInfo->Devices[DeviceId];
    cl_context_properties ContextProperties[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)Platform, 0
    };
    CALL_CL_RVRC(DeviceInfo->Contexts[DeviceId], clCreateContext, RC,
                 ContextProperties, 1, &Device, nullptr, nullptr);
    if (RC != CL_SUCCESS)
      return OFFLOAD_FAIL;
  }

  auto CLDeviceId = DeviceInfo->Devices[DeviceId];
  auto Context = DeviceInfo->getContext(DeviceId);
  CALL_CL_RVRC(DeviceInfo->Queues[DeviceId], clCreateCommandQueueWithProperties,
               RC, Context, CLDeviceId, QProperties.data());
  if (RC != CL_SUCCESS) {
    DP("Error: Failed to create CommandQueue: %d\n", RC);
    return OFFLOAD_FAIL;
  }

  auto &Extension = DeviceInfo->Extensions[DeviceId];
  Extension.getExtensionsInfoForDevice(DeviceId);

  if (Extension.DeviceAttributeQuery == ExtensionStatusEnabled) {
    if (OFFLOAD_SUCCESS !=
        DeviceInfo->DeviceProperties[DeviceId].getDeviceProperties(CLDeviceId))
      return OFFLOAD_FAIL;
  }

  DeviceInfo->DeviceArchs[DeviceId] = DeviceInfo->getDeviceArch(DeviceId);

#if INTEL_CUSTOMIZATION
  OMPT_CALLBACK(ompt_callback_device_initialize, DeviceId,
                DeviceInfo->Names[DeviceId].data(),
                DeviceInfo->Devices[DeviceId],
                omptLookupEntries, OmptDocument);
#endif // INTEL_CUSTOMIZATION

  DeviceInfo->Initialized[DeviceId] = true;

  return OFFLOAD_SUCCESS;
}

int64_t __tgt_rtl_init_requires(int64_t RequiresFlags) {
  DP("Initialize requires flags to %" PRId64 "\n", RequiresFlags);
  DeviceInfo->RequiresFlags = RequiresFlags;
  return RequiresFlags;
}

__tgt_target_table *__tgt_rtl_load_binary(
    int32_t DeviceId, __tgt_device_image *Image) {
  DP("Device %" PRId32 ": Loading binary from " DPxMOD "\n", DeviceId,
     DPxPTR(Image->ImageStart));

  size_t ImageSize = (size_t)Image->ImageEnd - (size_t)Image->ImageStart;
  size_t NumEntries = (size_t)(Image->EntriesEnd - Image->EntriesBegin);
  (void)NumEntries;

  DP("Expecting to have %zu entries defined\n", NumEntries);

  auto &Option = DeviceInfo->Option;
  std::string CompilationOptions(Option.CompilationOptions + " " +
                                 Option.UserCompilationOptions);
  std::string LinkingOptions(Option.UserLinkingOptions);

  DP("Base OpenCL compilation options: %s\n", CompilationOptions.c_str());
  DP("Base OpenCL linking options: %s\n", LinkingOptions.c_str());

  dumpImageToFile(Image->ImageStart, ImageSize, "OpenMP");

  auto Context = DeviceInfo->getContext(DeviceId);
  auto Device = DeviceInfo->Devices[DeviceId];
  DeviceInfo->Programs[DeviceId].emplace_back(Image, Context, Device, DeviceId);
  auto &Program = DeviceInfo->Programs[DeviceId].back();

  ProfileIntervalTy CompilationTimer("Compiling", DeviceId);
  ProfileIntervalTy LinkingTimer("Linking", DeviceId);

  CompilationTimer.start();
  int32_t RC = Program.buildPrograms(CompilationOptions, LinkingOptions);
  if (RC != OFFLOAD_SUCCESS)
    return nullptr;

  // Decide final compilation/linking options
#if INTEL_CUSTOMIZATION
  CompilationOptions += " " + DeviceInfo->Option.InternalCompilationOptions;
  LinkingOptions += " " + DeviceInfo->Option.InternalLinkingOptions;
  if (DeviceInfo->Option.DeviceType == CL_DEVICE_TYPE_GPU) {
    // For some reason GPU RT ignores -g and -cl-opt-disable passed
    // to clCompileProgram. At the same time CPU RT only accepts
    // these options for clCompileProgram - it will fail, if we pass
    // them to clLinkProgram. So here we try to look for these
    // options in the CompilationOptions and copy them to the LinkingOptions
    // only for GPU RT.

    // Add spaces around to simplify matching.
    CompilationOptions = " " + CompilationOptions + " ";
    if (CompilationOptions.find(" -g ") != std::string::npos)
      LinkingOptions += " -g ";
    if (CompilationOptions.find(" -cl-opt-disable ") != std::string::npos)
      LinkingOptions += " -cl-opt-disable ";
  }
  DPI("Final OpenCL compilation options: %s\n", CompilationOptions.c_str());
  DPI("Final OpenCL linking options: %s\n", LinkingOptions.c_str());
#else // INTEL_CUSTOMIZATION
  DP("Final OpenCL compilation options: %s\n", CompilationOptions.c_str());
  DP("Final OpenCL linking options: %s\n", LinkingOptions.c_str());
#endif // INTEL_CUSTOMIZATION
  // clLinkProgram drops the last symbol. Work this around temporarily.
  LinkingOptions += " ";

  RC = Program.compilePrograms(CompilationOptions, LinkingOptions);
  CompilationTimer.stop();
  if (RC != OFFLOAD_SUCCESS)
    return nullptr;

  LinkingTimer.start();
  RC = Program.linkPrograms(LinkingOptions);
  LinkingTimer.stop();
  if (RC != OFFLOAD_SUCCESS)
    return nullptr;

  RC = Program.buildKernels();
  if (RC != OFFLOAD_SUCCESS)
    return nullptr;

#if INTEL_CUSTOMIZATION
  if (Program.initProgramData() != OFFLOAD_SUCCESS)
    return nullptr;
#endif // INTEL_CUSTOMIZATION

  auto *Table = Program.getTablePtr();

#if INTEL_CUSTOMIZATION
  OMPT_CALLBACK(ompt_callback_device_load, DeviceId,
              "" /* filename */,
              -1 /* offset_in_file */,
              nullptr /* vma_in_file */,
              Table->EntriesEnd - Table->EntriesBegin /* bytes */,
              Table->EntriesBegin /* host_addr */,
              nullptr /* device_addr */,
              0 /* module_id */);
#endif // INTEL_CUSTOMIZATION

  return Table;
}

void *__tgt_rtl_data_alloc(int32_t DeviceId, int64_t Size, void *HstPtr,
                           int32_t Kind) {
  bool ImplicitArg = false;

  if (!HstPtr) {
    ImplicitArg = true;
    // User allocation
    if (Kind != TARGET_ALLOC_DEFAULT) {
      // Explicit allocation
      return dataAllocExplicit(DeviceId, Size, Kind);
    }
    if (DeviceInfo->Option.Flags.UseBuffer) {
      // Experimental CL buffer allocation
      return DeviceInfo->allocDataClMem(DeviceId, Size);
    }
  }
  return dataAlloc(DeviceId, Size, HstPtr, HstPtr, ImplicitArg);
}

int32_t __tgt_rtl_data_submit(int32_t DeviceId, void *TgtPtr, void *HstPtr,
                              int64_t Size) {
  return submitData(DeviceId, TgtPtr, HstPtr, Size);
}

int32_t __tgt_rtl_data_submit_async(
    int32_t DeviceId, void *TgtPtr, void *HstPtr, int64_t Size,
    __tgt_async_info *AsyncInfo /*not used*/) {
  return submitData(DeviceId, TgtPtr, HstPtr, Size);
}

int32_t __tgt_rtl_data_retrieve(int32_t DeviceId, void *HstPtr, void *TgtPtr,
                                int64_t Size) {
  return retrieveData(DeviceId, HstPtr, TgtPtr, Size);
}

int32_t __tgt_rtl_data_retrieve_async(
    int32_t DeviceId, void *HstPtr, void *TgtPtr, int64_t Size,
    __tgt_async_info *AsyncInfo /*not used*/) {
  return retrieveData(DeviceId, HstPtr, TgtPtr, Size);
}

int32_t __tgt_rtl_is_data_exchangable(int32_t SrcId, int32_t DstId) {
  // Only support this case. We don't have any documented OpenCL behavior for
  // cross-device data transfer.
  if (SrcId == DstId)
    return 1;

  return 0;
}

int32_t __tgt_rtl_data_exchange(int32_t SrcId, void *SrcPtr, int32_t DstId,
                                void *DstPtr, int64_t Size) {
  if (SrcId != DstId)
    return OFFLOAD_FAIL;

  // This is OK for same-device copy with SVM or USM extension.
  return __tgt_rtl_data_submit(DstId, DstPtr, SrcPtr, Size);
}

int32_t __tgt_rtl_data_delete(int32_t DeviceId, void *TgtPtr) {
  DeviceInfo->Mutexes[DeviceId].lock();

  // Deallocate cl_mem data
  if (DeviceInfo->Option.Flags.UseBuffer) {
    auto &ClMemBuffers = DeviceInfo->ClMemBuffers[DeviceId];
    if (ClMemBuffers.count(TgtPtr) > 0) {
      ClMemBuffers.erase(TgtPtr);
      CALL_CL_RET_FAIL(clReleaseMemObject, (cl_mem)TgtPtr);
      return OFFLOAD_SUCCESS;
    }
  }

  DeviceInfo->Mutexes[DeviceId].unlock();

  MemAllocInfoTy Info;
  auto &AllocInfos = DeviceInfo->MemAllocInfo;
  auto Removed = AllocInfos[DeviceId]->remove(TgtPtr, &Info);
  // Try again with device-independent allocation information (host USM)
  if (!Removed && DeviceInfo->Option.Flags.UseSingleContext)
    Removed = AllocInfos[DeviceInfo->NumDevices]->remove(TgtPtr, &Info);
  if (!Removed) {
    DP("Error: Cannot find memory allocation information for " DPxMOD "\n",
       DPxPTR(TgtPtr));
    return OFFLOAD_FAIL;
  }

  auto Context = DeviceInfo->getContext(DeviceId);
  if (DeviceInfo->Option.Flags.UseSVM) {
    CALL_CL_VOID(clSVMFree, Context, Info.Base);
  } else {
    CALL_CL_EXT_VOID(DeviceId, clMemFreeINTEL, Context, Info.Base);
  }

  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_run_target_team_region(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit,
    uint64_t LoopTripCount /*not used*/) {
  return runTargetTeamNDRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                               NumArgs, NumTeams, ThreadLimit, nullptr);
}

int32_t __tgt_rtl_run_target_team_region_async(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit,
    uint64_t LoopTripCount /*not used*/,
    __tgt_async_info *AsyncInfo /*not used*/) {
  return runTargetTeamNDRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                               NumArgs, NumTeams, ThreadLimit, nullptr);
}

int32_t __tgt_rtl_run_target_region(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs) {
  // use one team!
  return __tgt_rtl_run_target_team_region(DeviceId, TgtEntryPtr, TgtArgs,
                                          TgtOffsets, NumArgs, 1, 0, 0);
}

int32_t __tgt_rtl_run_target_region_async(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, __tgt_async_info *AsyncInfo /*not used*/) {
  // use one team!
  return __tgt_rtl_run_target_team_region(DeviceId, TgtEntryPtr, TgtArgs,
                                          TgtOffsets, NumArgs, 1, 0, 0);
}

int32_t __tgt_rtl_synchronize(int32_t device_id, __tgt_async_info *AsyncInfo) {
  return OFFLOAD_SUCCESS;
}

///
/// Extended plugin interface
///

// Notify the kernel about target pointers that are not explicitly
// passed as arguments, but which are pointing to mapped objects
// that may potentially be accessed in the kernel code (e.g. PTR_AND_OBJ
// objects).
int32_t __tgt_rtl_manifest_data_for_region(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtPtrs, size_t NumPtrs) {
  cl_kernel Kernel = *static_cast<cl_kernel *>(TgtEntryPtr);
  DP("Stashing %zu implicit arguments for kernel " DPxMOD "\n", NumPtrs,
     DPxPTR(Kernel));
  auto &KernelProperty = DeviceInfo->KernelProperties[DeviceId][Kernel];
  std::lock_guard<std::mutex> Lock(DeviceInfo->Mutexes[DeviceId]);
  KernelProperty.ImplicitArgs.clear();
  KernelProperty.ImplicitArgs.insert(TgtPtrs, TgtPtrs + NumPtrs);

  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_requires_mapping(int32_t DeviceId, void *Ptr, int64_t Size) {
  // Force mapping for host memory with positive size
  int32_t Ret;
  cl_unified_shared_memory_type_intel MemType = 0;
  CALL_CL_EXT_RET(DeviceId, false, clGetMemAllocInfoINTEL,
                  DeviceInfo->getContext(DeviceId), Ptr,
                  CL_MEM_ALLOC_TYPE_INTEL, sizeof(MemType), &MemType,
                  nullptr);
  if (MemType == CL_MEM_TYPE_UNKNOWN_INTEL ||
      (MemType == CL_MEM_TYPE_HOST_INTEL && Size > 0))
    Ret = 1;
  else
    Ret = 0;

  DP("Ptr " DPxMOD " %s mapping.\n", DPxPTR(Ptr),
     Ret ? "requires" : "does not require");
  return Ret;
}

// Allocate a base buffer with the given information.
void *__tgt_rtl_data_alloc_base(int32_t DeviceId, int64_t Size, void *HstPtr,
                                void *HstBase) {
  return dataAlloc(DeviceId, Size, HstPtr, HstBase, false);
}

// Allocate a managed memory object.
void *__tgt_rtl_data_alloc_managed(int32_t DeviceId, int64_t Size) {
  int32_t Kind = DeviceInfo->Option.Flags.UseHostMemForUSM
                    ? TARGET_ALLOC_HOST : TARGET_ALLOC_SHARED;
  return dataAllocExplicit(DeviceId, Size, Kind);
}

void *__tgt_rtl_data_realloc(
    int32_t DeviceId, void *Ptr, size_t Size, int32_t Kind) {
  const MemAllocInfoTy *Info = nullptr;

  if (Ptr) {
    Info = DeviceInfo->MemAllocInfo[DeviceId]->find(Ptr);
    if (!Info && DeviceInfo->Option.Flags.UseSingleContext)
      Info = DeviceInfo->MemAllocInfo[DeviceInfo->NumDevices]->find(Ptr);
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

  int32_t AllocKind =
      (Kind == TARGET_ALLOC_DEFAULT) ? TARGET_ALLOC_DEVICE : Kind;

  void *Mem = dataAllocExplicit(DeviceId, Size, AllocKind);

  if (Mem && Info) {
    if (AllocKind == TARGET_ALLOC_DEVICE || Info->Kind == TARGET_ALLOC_DEVICE ||
        Info->Kind == TARGET_ALLOC_SVM) {
      // TARGET_ALLOC_SVM is for "Device" memory type when SVM is enabled
      auto Queue = DeviceInfo->Queues[DeviceId];
      if (DeviceInfo->Option.Flags.UseSVM) {
        CALL_CL_RET_NULL(clEnqueueSVMMemcpy, Queue, CL_TRUE, Mem, Ptr,
                         Info->Size, 0, nullptr, nullptr);
      } else {
        CALL_CL_EXT_RET_NULL(DeviceId, clEnqueueMemcpyINTEL, Queue, CL_TRUE,
                             Mem, Ptr, Info->Size, 0, nullptr, nullptr);
      }
    } else {
      std::copy_n((char *)Ptr, Info->Size, (char *)Mem);
    }
    auto Rc = __tgt_rtl_data_delete(DeviceId, Ptr);
    if (Rc != OFFLOAD_SUCCESS)
      return nullptr;
  }

  return Mem;
}

void *__tgt_rtl_get_context_handle(int32_t DeviceId) {
  return (void *)DeviceInfo->getContext(DeviceId);
}

void *__tgt_rtl_data_aligned_alloc(int32_t DeviceId, size_t Align, size_t Size,
                                   int32_t Kind) {
  if (Align != 0 && (Align & (Align - 1)) != 0) {
    DP("Error: Alignment %zu is not power of two.\n", Align);
    return nullptr;
  }

  int32_t AllocKind =
      (Kind == TARGET_ALLOC_DEFAULT) ? TARGET_ALLOC_DEVICE : Kind;

  return dataAllocExplicit(DeviceId, Size, AllocKind, Align);
}

int32_t __tgt_rtl_run_target_team_nd_region(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit, void *LoopDesc) {
  return runTargetTeamNDRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                               NumArgs, NumTeams, ThreadLimit, LoopDesc);
}

char *__tgt_rtl_get_device_name(int32_t DeviceId, char *Buf, size_t BufMax) {
  assert(Buf && "Buf cannot be nullptr.");
  assert(BufMax > 0 && "BufMax cannot be zero.");
  CALL_CL_RET_NULL(clGetDeviceInfo, DeviceInfo->Devices[DeviceId],
                   CL_DEVICE_NAME, BufMax, Buf, nullptr);
  return Buf;
}

#if INTEL_CUSTOMIZATION
int32_t __tgt_rtl_get_data_alloc_info(
    int32_t DeviceId, int32_t NumPtrs, void *TgtPtrs, void *AllocInfo) {
  void **Ptrs = static_cast<void **>(TgtPtrs);
  __tgt_memory_info *Info = static_cast<__tgt_memory_info *>(AllocInfo);
  for (int32_t I = 0; I < NumPtrs; I++) {
    auto *MemInfo = DeviceInfo->MemAllocInfo[DeviceId]->find(Ptrs[I]);
    if (!MemInfo) {
      DP("%s cannot find allocation information for " DPxMOD "\n", __func__,
         DPxPTR(Ptrs[I]));
      return OFFLOAD_FAIL;
    }
    Info[I].Base = MemInfo->Base;
    Info[I].Offset = (uintptr_t)Ptrs[I] - (uintptr_t)Info[I].Base;
    Info[I].Size = MemInfo->Size + Info[I].Offset;
  }
  return OFFLOAD_SUCCESS;
}
#endif // INTEL_CUSTOMIZATION

void __tgt_rtl_add_build_options(
    const char *CompileOptions, const char *LinkOptions) {
  if (CompileOptions) {
    auto &compileOptions = DeviceInfo->Option.UserCompilationOptions;
    if (compileOptions.empty()) {
      compileOptions = std::string(CompileOptions) + " ";
    } else {
      DP("Respecting LIBOMPTARGET_OPENCL_COMPILATION_OPTIONS=%s\n",
         compileOptions.c_str());
    }
  }
  if (LinkOptions) {
    auto &linkOptions = DeviceInfo->Option.UserLinkingOptions;
    if (linkOptions.empty()) {
      linkOptions = std::string(LinkOptions) + " ";
    } else {
      DP("Respecting LIBOMPTARGET_OPENCL_LINKING_OPTIONS=%s\n",
         linkOptions.c_str());
    }
  }
}

int32_t __tgt_rtl_is_supported_device(int32_t DeviceId, void *DeviceType) {
  if (!DeviceType)
    return true;

  uint64_t DeviceArch = DeviceInfo->DeviceArchs[DeviceId];
  int32_t Ret = (uint64_t)(DeviceArch & (uint64_t)DeviceType) == DeviceArch;
  DP("Device %" PRIu32 " does%s match the requested device types " DPxMOD "\n",
     DeviceId, Ret ? "" : " not", DPxPTR(DeviceType));
  return Ret;
}

void __tgt_rtl_deinit(void) {
  // No-op on Linux
#ifdef _WIN32
  if (DeviceInfo) {
    closeRTL();
    deinit();
  }
#endif // _WIN32
}

#if INTEL_CUSTOMIZATION
__tgt_interop *__tgt_rtl_create_interop(
    int32_t DeviceId, int32_t InteropContext, int32_t NumPrefers,
    int32_t *PreferIDs) {
  // Preference-list is ignored since we cannot have multiple runtimes.
  auto Ret = new __tgt_interop();
  Ret->FrId = OCLInterop::FrId;
  Ret->FrName = OCLInterop::FrName;
  Ret->Vendor = OCLInterop::Vendor;
  Ret->VendorName = OCLInterop::VendorName;
  Ret->DeviceNum = DeviceId;

  auto Platform = DeviceInfo->Platforms[DeviceId];
  auto Context = DeviceInfo->getContext(DeviceId);
  auto Device = DeviceInfo->Devices[DeviceId];

  if (InteropContext == OMP_INTEROP_CONTEXT_TARGET ||
      InteropContext == OMP_INTEROP_CONTEXT_TARGETSYNC) {
    Ret->Platform = Platform;
    Ret->Device = Device;
    Ret->DeviceContext = Context;
  }
  if (InteropContext == OMP_INTEROP_CONTEXT_TARGETSYNC) {
    // Create a new out-of-order queue with profiling enabled.
    cl_queue_properties QProperties[] = {
      CL_QUEUE_PROPERTIES,
      CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE,
      0
    };
    cl_command_queue CmdQueue = nullptr;
    cl_int RC;
    CALL_CL_RVRC(CmdQueue, clCreateCommandQueueWithProperties, RC, Context,
                 Device, QProperties);
    if (RC != CL_SUCCESS) {
      DP("Error: Failed to create targetsync for interop\n");
      delete Ret;
      return nullptr;
    }
    Ret->TargetSync = CmdQueue;
  }

  Ret->RTLProperty = new OCLInterop::Property();

  return Ret;
}

int32_t __tgt_rtl_release_interop(int32_t DeviceId, __tgt_interop *Interop) {
  if (!Interop || Interop->DeviceNum != (intptr_t)DeviceId ||
      Interop->FrId != OCLInterop::FrId) {
    DP("Invalid/inconsistent OpenMP interop " DPxMOD "\n", DPxPTR(Interop));
    return OFFLOAD_FAIL;
  }

  if (Interop->TargetSync) {
    auto CmdQueue = static_cast<cl_command_queue>(Interop->TargetSync);
    CALL_CL_RET_FAIL(clFinish, CmdQueue);
    CALL_CL_RET_FAIL(clReleaseCommandQueue, CmdQueue);
  }

  auto OCL = static_cast<OCLInterop::Property *>(Interop->RTLProperty);
  delete OCL;
  delete Interop;

  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_use_interop(int32_t DeviceId, __tgt_interop *Interop) {
  if (!Interop || Interop->DeviceNum != (intptr_t)DeviceId ||
      Interop->FrId != OCLInterop::FrId) {
    DP("Invalid/inconsistent OpenMP interop " DPxMOD "\n", DPxPTR(Interop));
    return OFFLOAD_FAIL;
  }

  if (Interop->TargetSync) {
    auto CmdQueue = static_cast<cl_command_queue>(Interop->TargetSync);
    CALL_CL_RET_FAIL(clFinish, CmdQueue);
  }

  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_get_num_interop_properties(int32_t DeviceId) {
  // TODO: decide implementation-defined properties
  return 0;
}

/// Return the value of the requested property
int32_t __tgt_rtl_get_interop_property_value(
    int32_t DeviceId, __tgt_interop *Interop, int32_t Ipr, int32_t ValueType,
    size_t Size, void *Value) {

  if (Interop->RTLProperty == nullptr)
    return omp_irc_out_of_range;

  int32_t RC = omp_irc_success;

  // TODO
  //  switch (Ipr) {
  //default:
  //RC = omp_irc_out_of_range;
  //}

  return RC;
}

const char *__tgt_rtl_get_interop_property_info(
    int32_t DeviceId, int32_t Ipr, int32_t InfoType) {
  int32_t Offset = Ipr - omp_ipr_first;
  if (Offset < 0 || (size_t)Offset >= OCLInterop::IprNames.size())
    return nullptr;

  if (InfoType == OMP_IPR_INFO_NAME)
    return OCLInterop::IprNames[Offset];
  else if (InfoType == OMP_IPR_INFO_TYPE_DESC)
    return OCLInterop::IprTypeDescs[Offset];

  return nullptr;
}

const char *__tgt_rtl_get_interop_rc_desc(int32_t DeviceId, int32_t RetCode) {
  // TODO: decide implementation-defined return code.
  return nullptr;
}
#endif // INTEL_CUSTOMIZATION

int32_t __tgt_rtl_is_accessible_addr_range(
    int32_t DeviceId, const void *Ptr, size_t Size) {
  if (!Ptr || Size == 0)
    return 0;

  auto MemType = DeviceInfo->getMemAllocType(DeviceId, Ptr);
  if (MemType != CL_MEM_TYPE_HOST_INTEL && MemType != CL_MEM_TYPE_SHARED_INTEL)
    return 0;

  if (MemType == CL_MEM_TYPE_HOST_INTEL &&
      DeviceInfo->Option.Flags.UseSingleContext)
    DeviceId = DeviceInfo->NumDevices;

  if (DeviceInfo->MemAllocInfo[DeviceId]->contains(Ptr, Size))
    return 1;
  else
    return 0;
}

#if INTEL_CUSTOMIZATION
int32_t __tgt_rtl_notify_indirect_access(
    int32_t DeviceId, const void *Ptr, size_t Offset) {
  auto Fn = reinterpret_cast<clGitsIndirectAllocationOffsets_fn>(
      DeviceInfo->getExtensionFunctionPtr(DeviceId,
                                          clGitsIndirectAllocationOffsetsId));
  void *PtrBase = (void *)((uintptr_t)Ptr - Offset);
  // This DP is only for testability
  DP("Notifying indirect access: " DPxMOD " + %zu\n", DPxPTR(PtrBase), Offset);
  if (Fn) {
    Fn(PtrBase, 1, &Offset);
  }
  return OFFLOAD_SUCCESS;
}
#endif // INTEL_CUSTOMIZATION

int32_t __tgt_rtl_is_private_arg_on_host(
    int32_t DeviceId, const void *TgtEntryPtr, uint32_t Idx) {
  const cl_kernel *Kernel = static_cast<const cl_kernel *>(TgtEntryPtr);
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

#if INTEL_CUSTOMIZATION
int32_t __tgt_rtl_set_function_ptr_map(
    int32_t DeviceId, uint64_t Size,
    const __omp_offloading_fptr_map_t *FnPtrs) {
  cl_int RC;
  if (Size == 0)
    return OFFLOAD_SUCCESS;

  ProfileIntervalTy Timer("Function pointers init", DeviceId);
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
  void *FnPtrMapMem = nullptr;
  auto AllocProp =
      DeviceInfo->getAllocMemProperties(DeviceId, FnPtrMapSizeInBytes);
  CALL_CL_EXT_RVRC(DeviceId, FnPtrMapMem, clDeviceMemAllocINTEL, RC,
                   DeviceInfo->getContext(DeviceId),
                   DeviceInfo->Devices[DeviceId], AllocProp->data(),
                   FnPtrMapSizeInBytes, 0);
  if (RC != CL_SUCCESS || !FnPtrMapMem)
    return OFFLOAD_FAIL;

  DeviceInfo->Mutexes[DeviceId].lock();
  DeviceInfo->OwnedMemory[DeviceId].push_back(FnPtrMapMem);
  DeviceInfo->Mutexes[DeviceId].unlock();
  // Track this memory as being implicitly accessed by kernels.
  DeviceInfo->MemAllocInfo[DeviceId]->add(
      FnPtrMapMem, FnPtrMapMem, FnPtrMapSizeInBytes,
      TARGET_ALLOC_DEVICE, /*_InPool=*/false, /*_ImplicitArg=*/true);

  if (DebugLevel >= 2) {
    DP("Transferring function pointers table (%" PRIu64
       " entries) to the device: {\n", Size);
    // Limit the number of printed entries with (DebugLevel * 5).
    uint64_t PrintEntriesNum =
        std::min<uint64_t>(Size, static_cast<uint64_t>(DebugLevel) * 5);
    for (uint64_t I = 0; I < PrintEntriesNum; ++I) {
      DP("\t{ " DPxMOD ", " DPxMOD " }\n",
         DPxPTR(FnPtrs[I].HostPtr), DPxPTR(FnPtrs[I].TargetPtr));
    }
    if (PrintEntriesNum < Size)
      DP("\t... increase LIBOMPTARGET_DEBUG to see more entries ...\n");
    DP("}\n");
  }

  CALL_CL_EXT_RET_FAIL(DeviceId, clEnqueueMemcpyINTEL,
                       DeviceInfo->Queues[DeviceId],
                       /*blocking=*/CL_TRUE,
                       FnPtrMapMem, FnPtrs, FnPtrMapSizeInBytes,
                       /*num_events_in_wait_list=*/0,
                       /*num_events_in_wait_list*/nullptr,
                       /*event=*/nullptr);

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

  CALL_CL_EXT_RET_FAIL(DeviceId, clEnqueueMemcpyINTEL,
                       DeviceInfo->Queues[DeviceId],
                       /*blocking=*/CL_TRUE,
                       DeviceMapPtrVarAddr, &FnPtrMapMem, PtrTransferSize,
                       /*num_events_in_wait_list=*/0,
                       /*num_events_in_wait_list*/nullptr,
                       /*event=*/nullptr);

  // Initialize __omp_offloading_fptr_map_size with the table size.
  CALL_CL_EXT_RET_FAIL(DeviceId, clEnqueueMemcpyINTEL,
                       DeviceInfo->Queues[DeviceId],
                       /*blocking=*/CL_TRUE,
                       DeviceMapSizeVarAddr, &Size, sizeof(uint64_t),
                       /*num_events_in_wait_list=*/0,
                       /*num_events_in_wait_list*/nullptr,
                       /*event=*/nullptr);

  return OFFLOAD_SUCCESS;
}
#endif // INTEL_CUSTOMIZATION

void *__tgt_rtl_alloc_per_hw_thread_scratch(
    int32_t DeviceId, size_t ObjSize, int32_t AllocKind) {
  void *Mem = nullptr;
  cl_uint NumHWThreads = DeviceInfo->DeviceProperties[DeviceId].NumHWThreads;

  if (NumHWThreads == 0)
    return Mem;

  // Only support USM
  cl_int RC;
  auto Context = DeviceInfo->getContext(DeviceId);
  auto Device = DeviceInfo->Devices[DeviceId];
  size_t AllocSize = ObjSize * NumHWThreads;
  auto AllocProp = DeviceInfo->getAllocMemProperties(DeviceId, AllocSize);

  switch (AllocKind) {
  case TARGET_ALLOC_HOST:
    CALL_CL_EXT_RVRC(DeviceId, Mem, clHostMemAllocINTEL, RC, Context,
                     AllocProp->data(), AllocSize, 0 /* Align */);
    break;
  case TARGET_ALLOC_SHARED:
    CALL_CL_EXT_RVRC(DeviceId, Mem, clSharedMemAllocINTEL, RC, Context, Device,
                     AllocProp->data(), AllocSize, 0 /* Align */);
    break;
  case TARGET_ALLOC_DEVICE:
  default:
    CALL_CL_EXT_RVRC(DeviceId, Mem, clDeviceMemAllocINTEL, RC, Context, Device,
                     AllocProp->data(), AllocSize, 0 /* Align */);
  }

  if (RC != CL_SUCCESS) {
    DP("Failed to allocate per-hw-thread scratch space.\n");
    return nullptr;
  }

  DP("Allocated %zu byte per-hw-thread scratch space at " DPxMOD "\n",
     AllocSize, DPxPTR(Mem));

  return Mem;
}

void __tgt_rtl_free_per_hw_thread_scratch(int32_t DeviceId, void *Ptr) {
  auto Context = DeviceInfo->getContext(DeviceId);
  CALL_CL_EXT_VOID(DeviceId, clMemFreeINTEL, Context, Ptr);
}

#endif // INTEL_COLLAB

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
#include <CL/cl_ext_intel.h>
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
#include "omptarget-tools.h"
#include "rtl-trace.h"

#include "llvm/Support/Endian.h"

#if INTEL_CUSTOMIZATION
// FIXME: when this is upstreamed for OpenCL.
#define CL_MEM_FLAGS_INTEL                                               0x10001
#define CL_MEM_ALLOW_UNRESTRICTED_SIZE_INTEL                           (1 << 23)
#endif // INTEL_CUSTOMIZATION

#ifdef _WIN32
// TODO: enable again if XDEPS-3027 is resolved
#define OCL_KERNEL_BEGIN(ID)
#define OCL_KERNEL_END(ID)
#else // _WIN32
#define OCL_KERNEL_BEGIN(ID)                                                   \
  do {                                                                         \
    if (DeviceInfo->Option.KernelDynamicMemorySize > 0) {                      \
      /* Already in critical section */                                        \
      DeviceInfo->NumActiveKernels[ID]++;                                      \
    }                                                                          \
  } while (0)

#define OCL_KERNEL_END(ID)                                                     \
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
std::map<uint64_t, std::vector<const char *>> DeviceArchMap {
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
#else // !defined(_WIN32)
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
#endif // !defined(_WIN32)

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

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define OFFLOADSECTIONNAME "omp_offloading_entries"

//#pragma OPENCL EXTENSION cl_khr_spir : enable

// Get memory attributes for the given allocation size.
static std::unique_ptr<std::vector<cl_mem_properties_intel>>
getAllocMemProperties(size_t Size, cl_ulong MaxSize) {
  std::vector<cl_mem_properties_intel> Properties;
#if INTEL_CUSTOMIZATION
  if (Size > MaxSize) {
    Properties.push_back(CL_MEM_FLAGS_INTEL);
    Properties.push_back(CL_MEM_ALLOW_UNRESTRICTED_SIZE_INTEL);
  }
#endif // INTEL_CUSTOMIZATION
  Properties.push_back(0);

  return std::make_unique<std::vector<cl_mem_properties_intel>>(
      std::move(Properties));
}

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
};

/// Keep entries table per device.
struct FuncOrGblEntryTy {
  __tgt_target_table Table;
  std::vector<__tgt_offload_entry> Entries;
  std::vector<cl_kernel> Kernels;
  cl_program Program;
  std::unordered_map<cl_kernel, KernelInfoTy> KernelInfo;
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
      double deviceTime = 1e-9 * d.second.device * resolution;
      std::string key(d.first);

      if (d.first.substr(0, kernelPrefix.size()) == kernelPrefix)
        key = kernelPrefix + std::to_string(kernelId++);

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
      if (ExtensionFunctionPointers[i])
        DP("Extension %s is found.\n", ExtensionFunctionNames[i]);
      else
        DP("Warning: Extension %s is not found.\n", ExtensionFunctionNames[i]);
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
#if INTEL_CUSTOMIZATION
  ExtensionStatusTy GetDeviceGlobalVariablePointer = ExtensionStatusUnknown;
  ExtensionStatusTy SuggestedGroupSize = ExtensionStatusUnknown;
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

/// Handler and argument for an asynchronous event.
/// Libomptarget is expected to provide this data.
struct AsyncEventTy {
  void (*handler)(void *); // Handler for the event
  void *arg;               // Argument to the handler
};

/// Data type used within this plugin for OCL event.
struct AsyncDataTy {
  AsyncEventTy *Event; // Data from Libomptarget
  // Add plugin data below
  int32_t DeviceId; // OMP device ID
  cl_mem MemToRelease; // Memory object to be released

  AsyncDataTy(AsyncEventTy *event, int32_t deviceId) : Event(event),
      DeviceId(deviceId), MemToRelease(nullptr) {}
};

/// Data transfer method
enum DataTransferMethodTy {
  DATA_TRANSFER_METHOD_INVALID = -1,   // Invalid
  DATA_TRANSFER_METHOD_CLMEM = 0,      // Use Buffer on SVM
  DATA_TRANSFER_METHOD_SVMMAP,         // Use SVMMap/Unmap
  DATA_TRANSFER_METHOD_SVMMEMCPY,      // Use SVMMemcpy
  DATA_TRANSFER_METHOD_LAST,
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
  std::string UserCompilationOptions;
  std::string UserLinkingOptions;

#if INTEL_CUSTOMIZATION
  std::string InternalCompilationOptions;
  std::string InternalLinkingOptions;
#endif  // INTEL_CUSTOMIZATION

  /// Limit for the number of WIs in a WG.
  uint32_t ThreadLimit = 0;

  /// Limit for the number of WGs.
  uint32_t NumTeams = 0;

  /// Dynamic kernel memory size
  size_t KernelDynamicMemorySize = 0; // Turned off by default

  // This is a factor applied to the number of WGs computed
  // for the execution, based on the HW characteristics.
  size_t SubscriptionRate = 1;

  // For kernels that compute cross-WG reductions the number of computed WGs
  // is reduced by this factor.
  size_t ReductionSubscriptionRate = 1;

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
      if (Value >= 0 && Value <= 0xFFFF)
        ReductionSubscriptionRate = Value;
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

    // Read LIBOMPTARGET_DYNAMIC_MEMORY_SIZE=<SizeInMB>
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

/// Class containing all the device information.
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

  /// Looks up an external global variable with the given \p Name
  /// in the device environment for device \p DeviceId.
  /// \p Size must not be null. If (*Size) is not zero, then
  /// the lookup verifies that the found variable's size matches
  /// (*Size), otherwise, the found variable's size is returned
  /// via \p Size.
  void *getVarDeviceAddr(int32_t DeviceId, const char *Name, size_t *Size);
public:
  cl_uint NumDevices = 0;

  // Contains context and extension API
  std::map<cl_platform_id, PlatformInfoTy> PlatformInfos;

  // per device information
  std::vector<cl_platform_id> Platforms;
  std::vector<cl_context> Contexts;
  std::vector<cl_device_id> Devices;
  // Internal device type ID
  std::vector<uint64_t> DeviceArchs;
  /// Device properties
  std::vector<int32_t> maxExecutionUnits;
  std::vector<size_t> maxWorkGroupSize;
  std::vector<cl_ulong> MaxMemAllocSize;

  // A vector of descriptors of OpenCL extensions for each device.
  std::vector<ExtensionsTy> Extensions;
  std::vector<cl_command_queue> Queues;
  std::vector<cl_command_queue> QueuesInOrder;
  std::vector<std::list<FuncOrGblEntryTy>> FuncGblEntries;
  std::vector<std::map<cl_kernel, KernelPropertiesTy>>
      KernelProperties;
  std::vector<std::map<cl_kernel, std::set<void *>>> ImplicitArgs;
  std::vector<std::map<int32_t, ProfileDataTy>> Profiles;
  std::vector<std::vector<char>> Names;
  std::vector<bool> Initialized;
  std::vector<cl_ulong> SLMSize;
  std::mutex *Mutexes;
  std::mutex *ProfileLocks;
  std::vector<std::vector<std::vector<DeviceOffloadEntryTy>>> OffloadTables;
  std::vector<std::set<void *>> ClMemBuffers;
  // Memory owned by the plugin
  std::vector<std::vector<void *>> OwnedMemory;

  /// Internal allocation information
  std::vector<std::unique_ptr<MemAllocInfoMapTy>> MemAllocInfo;

  // Requires flags
  int64_t RequiresFlags = OMP_REQ_UNDEFINED;

  /// Number of active kernel launches for each device
  std::vector<uint32_t> NumActiveKernels;

  /// Cached target program data
  std::vector<ProgramDataTy> ProgramData;

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
#if INTEL_CUSTOMIZATION
    case clGetDeviceGlobalVariablePointerINTELId:
      return Extensions[DeviceId].GetDeviceGlobalVariablePointer ==
          ExtensionStatusEnabled;
    case clGetKernelSuggestedLocalWorkSizeINTELId:
      return Extensions[DeviceId].SuggestedGroupSize == ExtensionStatusEnabled;
    case clGitsIndirectAllocationOffsetsId:
      return Extensions[DeviceId].GitsIndirectAllocationOffsets ==
          ExtensionStatusEnabled;
#endif // INTEL_CUSTOMIZATION
    default:
      return true;
    }
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

  /// Read KernelInfo auxiliary information for the specified kernel.
  /// The information is stored in FuncGblEntries array.
  /// The function is called during the binary loading.
  bool readKernelInfo(int32_t DeviceId, const __tgt_offload_entry &KernelEntry);

  /// For the given kernel return its KernelInfo auxiliary information
  /// that was previously read by readKernelInfo().
  const KernelInfoTy *
      getKernelInfo(int32_t DeviceId, const cl_kernel &Kernel) const;
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
    if (OMPT_ENABLED) {
      OMPT_CALLBACK(ompt_callback_device_unload, i, 0 /* module ID */);
      OMPT_CALLBACK(ompt_callback_device_finalize, i);
    }

    for (auto &Entries : DeviceInfo->FuncGblEntries[i]) {
      for (auto Kernel : Entries.Kernels)
        if (Kernel)
          CALL_CL_EXIT_FAIL(clReleaseKernel, Kernel);
      // No entries may exist if offloading was done through MKL
      if (Entries.Program)
        CALL_CL_EXIT_FAIL(clReleaseProgram, Entries.Program);
    }

    CALL_CL_EXIT_FAIL(clReleaseCommandQueue, DeviceInfo->Queues[i]);

    if (DeviceInfo->QueuesInOrder[i])
      CALL_CL_EXIT_FAIL(clReleaseCommandQueue, DeviceInfo->QueuesInOrder[i]);

    DeviceInfo->unloadOffloadTable(i);

    for (auto mem : DeviceInfo->OwnedMemory[i])
      CALL_CL_EXT_VOID(i, clMemFreeINTEL, DeviceInfo->getContext(i), mem);

    if (!DeviceInfo->Option.Flags.UseSingleContext)
      CALL_CL_EXIT_FAIL(clReleaseContext, DeviceInfo->Contexts[i]);
  }

  if (DeviceInfo->Option.Flags.UseSingleContext)
    for (auto platformInfo : DeviceInfo->PlatformInfos)
      CALL_CL_EXIT_FAIL(clReleaseContext, platformInfo.second.Context);

  delete[] DeviceInfo->Mutexes;
  delete[] DeviceInfo->ProfileLocks;
  DP("Closed RTL successfully\n");
}

static std::string getDeviceRTLPath(const char *basename) {
  std::string rtl_path;
#ifdef _WIN32
  char path[_MAX_PATH];
  HMODULE module = nullptr;
  if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                          GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR) &DeviceInfo, &module))
    return rtl_path;
  if (!GetModuleFileNameA(module, path, sizeof(path)))
    return rtl_path;
  rtl_path = path;
#else
  Dl_info rtl_info;
  if (!dladdr(&DeviceInfo, &rtl_info))
    return rtl_path;
  rtl_path = rtl_info.dli_fname;
#endif
  size_t split = rtl_path.find_last_of("/\\");
  rtl_path.replace(split + 1, std::string::npos, basename);
  return rtl_path;
}

/// Initialize program data.
/// TODO: consider moving allocation of static buffers in device RTL to here
///       as it requires device information.
int32_t RTLDeviceInfoTy::initProgramData(int32_t deviceId) {
  uint32_t totalEUs = maxExecutionUnits[deviceId];
  uint32_t numThreadsPerEU;

  switch (DeviceArchs[deviceId]) {
  case DeviceArch_Gen9:
  case DeviceArch_XeLP:
    numThreadsPerEU = 7;
    break;
  case DeviceArch_XeHP:
    numThreadsPerEU = 8;
    break;
  default:
    numThreadsPerEU = 1;
  }

  // Allocate dynamic memory for in-kernel allocation
  void *memLB = 0;
  uintptr_t memUB = 0;
  if (Option.KernelDynamicMemorySize > 0) {
    cl_int rc;
    CALL_CL_EXT_RVRC(deviceId, memLB, clDeviceMemAllocINTEL, rc,
                     getContext(deviceId), Devices[deviceId],
                     getAllocMemProperties(Option.KernelDynamicMemorySize,
                                           MaxMemAllocSize[deviceId])->data(),
                     Option.KernelDynamicMemorySize, 0);
  }
  if (memLB) {
    OwnedMemory[deviceId].push_back(memLB);
    memUB = (uintptr_t)memLB + Option.KernelDynamicMemorySize;
  }

  int DType = (Option.DeviceType == CL_DEVICE_TYPE_GPU) ? 0 : 1;

  ProgramDataTy hostData = {
    1,                   // Initialized
    (int32_t)NumDevices, // Number of devices
    deviceId,            // Device ID
    totalEUs,            // Total EUs
    numThreadsPerEU,     // HW threads per EU
    (uintptr_t)memLB,    // Dynamic memory LB
    memUB,               // Dynamic memory UB
    DType                // Device type (0 for GPU, 1 for CPU)
  };

  ProgramData[deviceId] = hostData;

  return resetProgramData(deviceId);
}

int32_t RTLDeviceInfoTy::resetProgramData(int32_t DeviceId) {
#if INTEL_CUSTOMIZATION
  if (!isExtensionFunctionEnabled(
        DeviceId, clGetDeviceGlobalVariablePointerINTELId)) {
    DP("Warning: cannot initialize program data on device.\n");
    return OFFLOAD_SUCCESS;
  }

  void *DataPtr = getVarDeviceAddr(DeviceId, "__omp_spirv_program_data",
                                   sizeof(ProgramDataTy));
  if (!DataPtr) {
    DP("Warning: cannot find program data location on device.\n");
    return OFFLOAD_SUCCESS;
  }

  CALL_CL_EXT_RET_FAIL(DeviceId, clEnqueueMemcpyINTEL, Queues[DeviceId],
                       CL_TRUE, DataPtr, &ProgramData[DeviceId],
                       sizeof(ProgramDataTy), 0, nullptr, nullptr);
#endif // INTEL_CUSTOMIZATION

  return OFFLOAD_SUCCESS;
}

void *RTLDeviceInfoTy::getOffloadVarDeviceAddr(
    int32_t DeviceId, const char *Name, size_t Size) {
  DP("Looking up OpenMP global variable '%s' of size %zu bytes on device %d.\n",
     Name, Size, DeviceId);

  const std::vector<DeviceOffloadEntryTy> *OffloadTable = nullptr;

  if (!OffloadTables[DeviceId].empty())
    OffloadTable = &OffloadTables[DeviceId].back();

  if (OffloadTable && !OffloadTable->empty()) {
    size_t NameSize = strlen(Name) + 1;
    auto I = std::lower_bound(
        OffloadTable->begin(), OffloadTable->end(), Name,
        [NameSize](const DeviceOffloadEntryTy &E, const char *Name) {
          return strncmp(E.Base.name, Name, NameSize) < 0;
        });

    if (I != OffloadTable->end() &&
        strncmp(I->Base.name, Name, NameSize) == 0) {
      DP("Global variable '%s' found in the offload table at position %zu.\n",
         Name, std::distance(OffloadTable->begin(), I));
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
  size_t DeviceSize = 0;
  void *TgtAddr = nullptr;
  size_t Size = *SizePtr;
  bool SizeIsKnown = (Size != 0);
  if (SizeIsKnown)
    DP("Looking up device global variable '%s' of size %zu bytes "
       "on device %d.\n", Name, Size, DeviceId);
  else
    DP("Looking up device global variable '%s' of unknown size "
       "on device %d.\n", Name, DeviceId);

#if INTEL_CUSTOMIZATION
  if (!isExtensionFunctionEnabled(DeviceId,
                                  clGetDeviceGlobalVariablePointerINTELId))
    return nullptr;

  cl_int rc;
  auto clGetDeviceGlobalVariablePointerINTELFn =
      reinterpret_cast<clGetDeviceGlobalVariablePointerINTEL_fn>(
          getExtensionFunctionPtr(
              DeviceId, clGetDeviceGlobalVariablePointerINTELId));
  rc = clGetDeviceGlobalVariablePointerINTELFn(Devices[DeviceId],
      FuncGblEntries[DeviceId].back().Program, Name, &DeviceSize, &TgtAddr);

  if (rc != CL_SUCCESS) {
    DPI("Warning: clGetDeviceGlobalVariablePointerINTEL API returned "
        "nullptr for global variable '%s'.\n", Name);
    DeviceSize = 0;
  } else if (Size != DeviceSize && SizeIsKnown) {
    DPI("Warning: size mismatch for host (%zu) and device (%zu) versions "
        "of global variable: %s\n.  Direct references "
        "to this variable will not work properly.\n",
        Size, DeviceSize, Name);
    DeviceSize = 0;
  }
#else  // INTEL_CUSTOMIZATION
  // TODO: use device API to get variable address by name.
#endif // INTEL_CUSTOMIZATION

  if (DeviceSize == 0) {
    DP("Warning: global variable lookup failed.\n");
    return nullptr;
  }

  DP("Global variable lookup succeeded (size: %zu bytes).\n", DeviceSize);
  *SizePtr = DeviceSize;
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
  CALL_CL_EXT_RET(DeviceId, false, clEnqueueMemcpyINTEL, Queues[DeviceId],
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
  CALL_CL_EXT_RET(DeviceId, false, clEnqueueMemcpyINTEL, Queues[DeviceId],
                  CL_TRUE, DeviceTable.data(), OffloadTableVarAddr,
                  TableSize, 0, nullptr, nullptr);

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
    CALL_CL_EXT_RET(DeviceId, false, clEnqueueMemcpyINTEL, Queues[DeviceId],
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

  std::string DeviceName(Names[DeviceId].data());
#ifdef _WIN32
  // Windows: Device name contains published product name.
  for (auto &Arch : DeviceArchMap)
    for (auto Str : Arch.second)
      if (DeviceName.find(Str) != std::string::npos)
        return Arch.first;
#else
  uint32_t PCIDeviceId = getPCIDeviceId(DeviceId);
  if (PCIDeviceId != 0) {
    for (auto &Arch : DeviceArchMap)
      for (auto Id : Arch.second)
        if (PCIDeviceId == Id || (PCIDeviceId & 0xFF00) == Id)
          return Arch.first;  // Exact match or prefix match
  }
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

bool RTLDeviceInfoTy::readKernelInfo(
    int32_t DeviceId, const __tgt_offload_entry &KernelEntry) {
  const cl_kernel *KernelPtr =
      reinterpret_cast<const cl_kernel *>(KernelEntry.addr);
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
  CALL_CL_EXT_RET(DeviceId, false, clEnqueueMemcpyINTEL, Queues[DeviceId],
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

  FuncGblEntries[DeviceId].back().KernelInfo.emplace(
      std::make_pair(*KernelPtr, std::move(Info)));
  return true;
}

const KernelInfoTy *RTLDeviceInfoTy::getKernelInfo(
    int32_t DeviceId, const cl_kernel &Kernel) const {
  for (auto &Entry : FuncGblEntries[DeviceId]) {
    auto &KernelInfo = Entry.KernelInfo;
    auto It = KernelInfo.find(Kernel);
    if (It != KernelInfo.end())
      return &(It->second);
  }

  return nullptr;
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

#ifdef __cplusplus
extern "C" {
#endif

static inline void addDataTransferLatency() {
  if (!DeviceInfo->Option.Flags.CollectDataTransferLatency)
    return;
  double goal = omp_get_wtime() + 1e-6 * DeviceInfo->Option.DataTransferLatency;
  // Naive spinning should be enough
  while (omp_get_wtime() < goal)
    ;
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
#if INTEL_CUSTOMIZATION
  // Check if the extension was not explicitly disabled, i.e.
  // that its current status is unknown.
  if (GetDeviceGlobalVariablePointer == ExtensionStatusUnknown)
    // FIXME: use the right extension name.
    if (Extensions.find("") != std::string::npos) {
      GetDeviceGlobalVariablePointer = ExtensionStatusEnabled;
      DPI("Extension clGetDeviceGlobalVariablePointerINTEL enabled.\n");
    }

  if (SuggestedGroupSize == ExtensionStatusUnknown)
    // FIXME: use the right extension name.
    if (Extensions.find("") != std::string::npos) {
      SuggestedGroupSize = ExtensionStatusEnabled;
      DPI("Extension clGetKernelSuggestedLocalWorkSizeINTEL enabled.\n");
    }
#endif // INTEL_CUSTOMIZATION

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

EXTERN
int32_t __tgt_rtl_number_of_devices() {
  // Assume it is thread safe, since it is called once.

  DP("Start initializing OpenCL\n");
  // get available platforms
  cl_uint platformIdCount = 0;
  CALL_CL_RET_ZERO(clGetPlatformIDs, 0, nullptr, &platformIdCount);
  std::vector<cl_platform_id> platformIds(platformIdCount);
  CALL_CL_RET_ZERO(clGetPlatformIDs, platformIdCount, platformIds.data(),
                   nullptr);

  // All eligible OpenCL device IDs from the platforms are stored in a list
  // in the order they are probed by clGetPlatformIDs/clGetDeviceIDs.
  for (cl_platform_id id : platformIds) {
    std::vector<char> buf;
    size_t buf_size;
    cl_int rc;
    CALL_CL(rc, clGetPlatformInfo, id, CL_PLATFORM_VERSION, 0, nullptr,
            &buf_size);
    if (rc != CL_SUCCESS || buf_size == 0)
      continue;
    buf.resize(buf_size);
    CALL_CL(rc, clGetPlatformInfo, id, CL_PLATFORM_VERSION, buf_size,
            buf.data(), nullptr);
    // clCreateProgramWithIL() requires OpenCL 2.1.
    if (rc != CL_SUCCESS || std::stof(std::string(buf.data() + 6)) <= 2.0) {
      continue;
    }
    cl_uint numDevices = 0;
    CALL_CL_SILENT(rc, clGetDeviceIDs, id, DeviceInfo->Option.DeviceType, 0,
                   nullptr, &numDevices);
    if (rc != CL_SUCCESS || numDevices == 0)
      continue;

    const char *platformName = buf.data() ? buf.data() : "undefined";
    DP("Platform %s has %" PRIu32 " Devices\n", platformName, numDevices);
    std::vector<cl_device_id> devices(numDevices);
    CALL_CL_RET_ZERO(clGetDeviceIDs, id, DeviceInfo->Option.DeviceType,
                     numDevices, devices.data(), nullptr);

    cl_context context = nullptr;
    if (DeviceInfo->Option.Flags.UseSingleContext) {
      cl_context_properties contextProperties[] = {
          CL_CONTEXT_PLATFORM, (cl_context_properties)id, 0
      };
      CALL_CL_RVRC(context, clCreateContext, rc, contextProperties,
                   devices.size(), devices.data(), nullptr, nullptr);
      if (rc != CL_SUCCESS)
        continue;
    }

    DeviceInfo->PlatformInfos.emplace(id, PlatformInfoTy(id, context));
    for (auto device : devices) {
      DeviceInfo->Devices.push_back(device);
      DeviceInfo->Platforms.push_back(id);
    }
    DeviceInfo->NumDevices += numDevices;
  }

  if (!DeviceInfo->Option.Flags.UseSingleContext)
    DeviceInfo->Contexts.resize(DeviceInfo->NumDevices);
  DeviceInfo->maxExecutionUnits.resize(DeviceInfo->NumDevices);
  DeviceInfo->maxWorkGroupSize.resize(DeviceInfo->NumDevices);
  DeviceInfo->MaxMemAllocSize.resize(DeviceInfo->NumDevices);
  DeviceInfo->Extensions.resize(DeviceInfo->NumDevices);
  DeviceInfo->Queues.resize(DeviceInfo->NumDevices);
  DeviceInfo->QueuesInOrder.resize(DeviceInfo->NumDevices, nullptr);
  DeviceInfo->FuncGblEntries.resize(DeviceInfo->NumDevices);
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
  DeviceInfo->OffloadTables.resize(DeviceInfo->NumDevices);
  DeviceInfo->OwnedMemory.resize(DeviceInfo->NumDevices);
  DeviceInfo->NumActiveKernels.resize(DeviceInfo->NumDevices, 0);
  DeviceInfo->ProgramData.resize(DeviceInfo->NumDevices);

  // Host allocation information needs one additional slot
  for (uint32_t I = 0; I < DeviceInfo->NumDevices + 1; I++)
    DeviceInfo->MemAllocInfo.emplace_back(new MemAllocInfoMapTy());

  // get device specific information
  for (unsigned i = 0; i < DeviceInfo->NumDevices; i++) {
    size_t buf_size;
    cl_int rc;
    cl_device_id deviceId = DeviceInfo->Devices[i];
    CALL_CL(rc, clGetDeviceInfo, deviceId, CL_DEVICE_NAME, 0, nullptr,
            &buf_size);
    if (rc != CL_SUCCESS || buf_size == 0)
      continue;
    DeviceInfo->Names[i].resize(buf_size);
    CALL_CL(rc, clGetDeviceInfo, deviceId, CL_DEVICE_NAME, buf_size,
            DeviceInfo->Names[i].data(), nullptr);
    if (rc != CL_SUCCESS)
      continue;
    DeviceInfo->DeviceArchs[i] = DeviceInfo->getDeviceArch(i);
    DP("Device %d: %s\n", i, DeviceInfo->Names[i].data());
    CALL_CL_RET_ZERO(clGetDeviceInfo, deviceId, CL_DEVICE_MAX_COMPUTE_UNITS, 4,
                     &DeviceInfo->maxExecutionUnits[i], nullptr);
    DP("Number of execution units on the device is %d\n",
       DeviceInfo->maxExecutionUnits[i]);
    CALL_CL_RET_ZERO(clGetDeviceInfo, deviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE,
                     sizeof(size_t), &DeviceInfo->maxWorkGroupSize[i], nullptr);
    DP("Maximum work group size for the device is %d\n",
       static_cast<int32_t>(DeviceInfo->maxWorkGroupSize[i]));
    CALL_CL_RET_ZERO(clGetDeviceInfo, deviceId, CL_DEVICE_MAX_MEM_ALLOC_SIZE,
                     sizeof(cl_ulong), &DeviceInfo->MaxMemAllocSize[i],
                     nullptr);
    DP("Maximum memory allocation size is %" PRIu64 "\n",
       DeviceInfo->MaxMemAllocSize[i]);
    cl_uint addressmode;
    CALL_CL_RET_ZERO(clGetDeviceInfo, deviceId, CL_DEVICE_ADDRESS_BITS, 4,
                     &addressmode, nullptr);
    DP("Addressing mode is %d bit\n", addressmode);
    CALL_CL_RET_ZERO(clGetDeviceInfo, deviceId, CL_DEVICE_LOCAL_MEM_SIZE,
                     sizeof(cl_ulong), &DeviceInfo->SLMSize[i], nullptr);
    DP("Device local mem size: %zu\n", (size_t)DeviceInfo->SLMSize[i]);
    DeviceInfo->Initialized[i] = false;
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

EXTERN
int32_t __tgt_rtl_init_device(int32_t device_id) {
  cl_int status;
  DP("Initialize OpenCL device\n");
  assert(device_id >= 0 && (cl_uint)device_id < DeviceInfo->NumDevices &&
         "bad device id");

  // Use out-of-order queue by default.
  std::vector<cl_queue_properties> qProperties {
      CL_QUEUE_PROPERTIES,
      CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE
  };
  if (DeviceInfo->Option.Flags.EnableProfile)
    qProperties.back() |= CL_QUEUE_PROFILING_ENABLE;
  qProperties.push_back(0);

  if (!DeviceInfo->Option.Flags.UseSingleContext) {
    auto platform = DeviceInfo->Platforms[device_id];
    auto device = DeviceInfo->Devices[device_id];
    cl_context_properties contextProperties[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0
    };
    cl_int rc;
    CALL_CL_RVRC(DeviceInfo->Contexts[device_id], clCreateContext, rc,
                 contextProperties, 1, &device, nullptr, nullptr);
    if (rc != CL_SUCCESS)
      return OFFLOAD_FAIL;
  }

  auto deviceID = DeviceInfo->Devices[device_id];
  auto context = DeviceInfo->getContext(device_id);
  CALL_CL_RVRC(DeviceInfo->Queues[device_id],
               clCreateCommandQueueWithProperties, status, context, deviceID,
               qProperties.data());
  if (status != CL_SUCCESS) {
    DP("Error: Failed to create CommandQueue: %d\n", status);
    return OFFLOAD_FAIL;
  }

  DeviceInfo->Extensions[device_id].getExtensionsInfoForDevice(device_id);

  OMPT_CALLBACK(ompt_callback_device_initialize, device_id,
                DeviceInfo->Names[device_id].data(),
                DeviceInfo->Devices[device_id],
                omptLookupEntries, OmptDocument);

  DeviceInfo->Initialized[device_id] = true;

  return OFFLOAD_SUCCESS;
}

EXTERN int64_t __tgt_rtl_init_requires(int64_t RequiresFlags) {
  DP("Initialize requires flags to %" PRId64 "\n", RequiresFlags);
  DeviceInfo->RequiresFlags = RequiresFlags;
  return RequiresFlags;
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

static cl_program createProgramFromFile(
    const char *basename, int32_t device_id, std::string &options) {
  std::string device_rtl_path = getDeviceRTLPath(basename);
  std::ifstream device_rtl(device_rtl_path, std::ios::binary);

  if (device_rtl.is_open()) {
    DP("Found device RTL: %s\n", device_rtl_path.c_str());
    device_rtl.seekg(0, device_rtl.end);
    int device_rtl_len = device_rtl.tellg();
    std::string device_rtl_bin(device_rtl_len, '\0');
    device_rtl.seekg(0);
    if (!device_rtl.read(&device_rtl_bin[0], device_rtl_len)) {
      DP("I/O Error: Failed to read device RTL.\n");
      return nullptr;
    }

    dumpImageToFile(device_rtl_bin.c_str(), device_rtl_len, basename);

    cl_int status;
    cl_program program;
    CALL_CL_RVRC(program, clCreateProgramWithIL, status,
                 DeviceInfo->getContext(device_id), device_rtl_bin.c_str(),
                 device_rtl_len);
    if (status != CL_SUCCESS) {
      DP("Error: Failed to create device RTL from IL: %d\n", status);
      return nullptr;
    }

    DeviceInfo->Option.CommonSpecConstants.setProgramConstants(device_id,
                                                               program);

    CALL_CL(status, clCompileProgram, program, 0, nullptr, options.c_str(), 0,
            nullptr, nullptr, nullptr, nullptr);
    if (status != CL_SUCCESS || DeviceInfo->Option.Flags.ShowBuildLog) {
      debugPrintBuildLog(program, DeviceInfo->Devices[device_id]);
      if (status != CL_SUCCESS) {
        DP("Error: Failed to compile program: %d\n", status);
        return nullptr;
      }
    }

    return program;
  }

  DP("Cannot find device RTL: %s\n", device_rtl_path.c_str());
  return nullptr;
}

static cl_program getOpenCLProgramForImage(int32_t DeviceId,
                                           __tgt_device_image *Image,
                                           std::string &CompilationOptions,
                                           std::string &LinkingOptions,
                                           bool &IsBinary) {
  cl_int Status;

  uint64_t MajorVer, MinorVer;
  if (!isValidOneOmpImage(Image, MajorVer, MinorVer)) {
    // Handle legacy plain SPIR-V image.
    char *ImgBegin = reinterpret_cast<char *>(Image->ImageStart);
    char *ImgEnd = reinterpret_cast<char *>(Image->ImageEnd);
    size_t ImgSize = ImgEnd - ImgBegin;
    dumpImageToFile(ImgBegin, ImgSize, "OpenMP");
    cl_program Program;
    CALL_CL_RVRC(Program, clCreateProgramWithIL, Status,
                 DeviceInfo->getContext(DeviceId),
                 ImgBegin, ImgSize);
    if (Status != CL_SUCCESS || DeviceInfo->Option.Flags.ShowBuildLog) {
      debugPrintBuildLog(Program, DeviceInfo->Devices[DeviceId]);
      if (Status != CL_SUCCESS) {
        DP("Error: Failed to create program: %d\n", Status);
        return nullptr;
      }
    }

    return Program;
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
    cl_program Program;

    if (It->second.Format == 0) {
      // Native format.
      IsBinary = true;
      CALL_CL_RVRC(Program, clCreateProgramWithBinary, Status,
                   DeviceInfo->getContext(DeviceId),
                   1, &DeviceInfo->Devices[DeviceId],
                   &ImgSize, &ImgBegin, nullptr);
    } else if (It->second.Format == 1) {
      // SPIR-V format.
      IsBinary = false;
      CALL_CL_RVRC(Program, clCreateProgramWithIL, Status,
                   DeviceInfo->getContext(DeviceId),
                   ImgBegin, ImgSize);
    } else {
      DP("Warning: image %" PRIu64 "is ignored due to unknown format.\n", Idx);
      continue;
    }

    if (Status != CL_SUCCESS || DeviceInfo->Option.Flags.ShowBuildLog) {
      debugPrintBuildLog(Program, DeviceInfo->Devices[DeviceId]);
      if (Status != CL_SUCCESS) {
        DP("Warning: failed to create program from %s (%" PRIu64 "): %d\n",
           IsBinary ? "binary" : "SPIR-V", Idx, Status);
        continue;
      }
    }

    DP("Created offload program from image #%" PRIu64 ".\n", Idx);
    if (DeviceInfo->Option.Flags.UseImageOptions) {
      CompilationOptions += " " + It->second.CompileOpts;
      LinkingOptions += " " + It->second.LinkOpts;
    }
    return Program;
  }

  return nullptr;
}

EXTERN
__tgt_target_table *__tgt_rtl_load_binary(int32_t device_id,
                                          __tgt_device_image *image) {

  DP("Device %d: load binary from " DPxMOD " image\n", device_id,
     DPxPTR(image->ImageStart));

  size_t NumEntries = (size_t)(image->EntriesEnd - image->EntriesBegin);
  DP("Expecting to have %zu entries defined.\n", NumEntries);

  ProfileIntervalTy CompilationTimer("Compiling", device_id);
  ProfileIntervalTy LinkingTimer("Linking", device_id);

  DeviceInfo->FuncGblEntries[device_id].emplace_back();
  auto &FuncGblEntries = DeviceInfo->FuncGblEntries[device_id].back();

  // create Program
  cl_int status;
  std::vector<cl_program> programs;
  cl_program linked_program;
  std::string CompilationOptions(
      DeviceInfo->Option.CompilationOptions + " " +
      DeviceInfo->Option.UserCompilationOptions);
  std::string LinkingOptions(DeviceInfo->Option.UserLinkingOptions);
  DP("Basic OpenCL compilation options: %s\n", CompilationOptions.c_str());
  DP("Basic OpenCL linking options: %s\n", LinkingOptions.c_str());

  bool IsBinary = false;
  // Create program for the target regions.
  // User program must be first in the link order.
  CompilationTimer.start();
  cl_program program = getOpenCLProgramForImage(device_id, image,
                                                CompilationOptions,
                                                LinkingOptions,
                                                IsBinary);
  if (!program) {
    // This must never happen. The compiler must make sure that
    // at least one generic SPIR-V image fits all devices.
    DP("Error: failed to create a program from the offload image.\n");
    return NULL;
  }

  DeviceInfo->Option.CommonSpecConstants.setProgramConstants(device_id,
                                                             program);

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

  if (IsBinary || DeviceInfo->Option.Flags.EnableSimd ||
      // Work around GPU API issue: clCompileProgram/clLinkProgram
      // does not work with -vc-codegen, so we have to use clBuildProgram.
      CompilationOptions.find(" -vc-codegen ") != std::string::npos) {
    // Programs created from binary must still be built.
    CALL_CL(status, clBuildProgram, program, 0, nullptr,
      (CompilationOptions + " " + LinkingOptions).c_str(), nullptr, nullptr);
    if (status != CL_SUCCESS || DeviceInfo->Option.Flags.ShowBuildLog) {
      debugPrintBuildLog(program, DeviceInfo->Devices[device_id]);
      if (status != CL_SUCCESS) {
        DP("Error: Failed to build program: %d\n", status);
        return NULL;
      }
    }
    linked_program = program;
    FuncGblEntries.Program = linked_program;
    CompilationTimer.stop();
  } else {
    CALL_CL(status, clCompileProgram, program, 0, nullptr,
      CompilationOptions.c_str(), 0, nullptr, nullptr, nullptr, nullptr);
    if (status != CL_SUCCESS || DeviceInfo->Option.Flags.ShowBuildLog) {
      debugPrintBuildLog(program, DeviceInfo->Devices[device_id]);
      if (status != CL_SUCCESS) {
        DP("Error: Failed to compile program: %d\n", status);
        return NULL;
      }
    }
    programs.push_back(program);

    if (DeviceInfo->Option.Flags.LinkLibDevice) {
      // Link libdevice fallback implementations, if needed.
      auto &libdevice_extensions =
          DeviceInfo->Extensions[device_id].LibdeviceExtensions;

      for (unsigned i = 0; i < libdevice_extensions.size(); ++i) {
        auto &desc = libdevice_extensions[i];
        if (desc.Status != ExtensionStatusEnabled) {
          // Device runtime does not support this libdevice extension,
          // so we have to link in the fallback implementation.
          //
          // TODO: the device image must specify which libdevice extensions
          //       are actually required. We should link only the required
          //       fallback implementations.
          cl_program program =
              createProgramFromFile(desc.FallbackLibName, device_id,
                                    CompilationOptions);
          if (program) {
            DP("Successfully compiled fallback libdevice %s\n",
               desc.FallbackLibName);
            programs.push_back(program);
          } else {
            DP("Error: Failed to compile fallback libdevice %s\n",
               desc.FallbackLibName);
            return NULL;
          }
        } else {
          DP("Skipped device RTL: %s\n", desc.FallbackLibName);
        }
      }
    }
    CompilationTimer.stop();

    LinkingTimer.start();

    CALL_CL_RVRC(linked_program, clLinkProgram, status,
        DeviceInfo->getContext(device_id), 1, &DeviceInfo->Devices[device_id],
        LinkingOptions.c_str(), programs.size(), programs.data(), nullptr,
        nullptr);
    if (status != CL_SUCCESS || DeviceInfo->Option.Flags.ShowBuildLog) {
      debugPrintBuildLog(linked_program, DeviceInfo->Devices[device_id]);
      if (status != CL_SUCCESS) {
        DP("Error: Failed to link program: %d\n", status);
        return NULL;
      }
    } else {
      DP("Successfully linked program.\n");
    }
    FuncGblEntries.Program = linked_program;
    LinkingTimer.stop();
  }

  // create kernel and target entries
  FuncGblEntries.Entries.resize(NumEntries);
  FuncGblEntries.Kernels.resize(NumEntries);
  auto &entries = FuncGblEntries.Entries;
  auto &kernels = FuncGblEntries.Kernels;

  ProfileIntervalTy EntriesTimer("OffloadEntriesInit", device_id);
  EntriesTimer.start();
  // FIXME: table loading does not work at all on XeLP.
  // Enable it after CMPLRLIBS-33285 is fixed.
  if (DeviceInfo->DeviceArchs[device_id] != DeviceArch_XeLP &&
      !DeviceInfo->loadOffloadTable(device_id, NumEntries))
    DP("Warning: offload table loading failed.\n");
  EntriesTimer.stop();

  for (unsigned i = 0; i < NumEntries; i++) {
    // Size is 0 means that it is kernel function.
    auto Size = image->EntriesBegin[i].size;

    if (Size != 0) {
      EntriesTimer.start();

      void *HostAddr = image->EntriesBegin[i].addr;
      char *Name = image->EntriesBegin[i].name;

      void *TgtAddr =
          DeviceInfo->getOffloadVarDeviceAddr(device_id, Name, Size);
      if (!TgtAddr) {
        TgtAddr = __tgt_rtl_data_alloc(device_id, Size, HostAddr,
                                       TARGET_ALLOC_DEFAULT);
        __tgt_rtl_data_submit(device_id, TgtAddr, HostAddr, Size);
        DP("Warning: global variable '%s' allocated. "
           "Direct references will not work properly.\n", Name);
      }

      DP("Global variable mapped: Name = %s, Size = %zu, "
         "HostPtr = " DPxMOD ", TgtPtr = " DPxMOD "\n",
         Name, Size, DPxPTR(HostAddr), DPxPTR(TgtAddr));
      entries[i].addr = TgtAddr;
      entries[i].name = Name;
      entries[i].size = Size;

      EntriesTimer.stop();
      continue;
    }

    char *name = image->EntriesBegin[i].name;
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
#endif  // _WIN32
    CALL_CL_RVRC(kernels[i], clCreateKernel, status, linked_program, name);
    if (status != CL_SUCCESS) {
      // If a kernel was deleted by optimizations (e.g. DCE), then
      // clCreateKernel will fail. We expect that such a kernel
      // will never be actually invoked.
      DP("Warning: Failed to create kernel %s, %d\n", name, status);
      kernels[i] = nullptr;
    }
    entries[i].addr = &kernels[i];
    entries[i].name = name;

    // Do not try to query information for deleted kernels.
    if (!kernels[i])
      continue;

    if (!DeviceInfo->readKernelInfo(device_id, entries[i])) {
      DP("Error: failed to read kernel info for kernel %s\n", name);
      return nullptr;
    }

    // Retrieve kernel group size info.
    auto &KernelProperty = DeviceInfo->KernelProperties[device_id][kernels[i]];
    auto Device = DeviceInfo->Devices[device_id];
    CALL_CL_RET_NULL(clGetKernelWorkGroupInfo, kernels[i], Device,
                     CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                     sizeof(size_t), &KernelProperty.Width, nullptr);
    CALL_CL_RET_NULL(clGetKernelSubGroupInfo, kernels[i], Device,
                     CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE, sizeof(size_t),
                     &KernelProperty.SIMDWidth, sizeof(size_t),
                     &KernelProperty.SIMDWidth, nullptr);
    assert(KernelProperty.SIMDWidth <= KernelProperty.Width &&
           "Invalid preferred group size multiple.");

    CALL_CL_RET_NULL(clGetKernelWorkGroupInfo, kernels[i], Device,
                     CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t),
                     &KernelProperty.MaxThreadGroupSize, nullptr);

    if (DebugLevel > 0) {
      // Show kernel information
      std::vector<char> buf;
      size_t buf_size;
      cl_uint kernel_num_args = 0;
      CALL_CL_RET_NULL(clGetKernelInfo, kernels[i], CL_KERNEL_FUNCTION_NAME, 0,
                       nullptr, &buf_size);
      buf.resize(buf_size);
      CALL_CL_RET_NULL(clGetKernelInfo, kernels[i], CL_KERNEL_FUNCTION_NAME,
                       buf_size, buf.data(), nullptr);
      CALL_CL_RET_NULL(clGetKernelInfo, kernels[i], CL_KERNEL_NUM_ARGS,
                       sizeof(cl_uint), &kernel_num_args, nullptr);
      const char *kernelName = buf.data() ? buf.data() : "undefined";
      DP("Kernel %d: Name = %s, NumArgs = %d\n", i, kernelName,
         kernel_num_args);
      for (unsigned idx = 0; idx < kernel_num_args; idx++) {
        CALL_CL_RET_NULL(clGetKernelArgInfo, kernels[i], idx,
                         CL_KERNEL_ARG_TYPE_NAME, 0, nullptr, &buf_size);
        buf.resize(buf_size);
        CALL_CL_RET_NULL(clGetKernelArgInfo, kernels[i], idx,
                         CL_KERNEL_ARG_TYPE_NAME, buf_size, buf.data(),
                         nullptr);
        std::string type_name = buf.data();
        CALL_CL_RET_NULL(clGetKernelArgInfo, kernels[i], idx,
                         CL_KERNEL_ARG_NAME, 0, nullptr, &buf_size);
        buf.resize(buf_size);
        CALL_CL_RET_NULL(clGetKernelArgInfo, kernels[i], idx,
                         CL_KERNEL_ARG_NAME, buf_size, buf.data(), nullptr);
        const char *argName = buf.data() ? buf.data() : "undefined";
        DP("  Arg %2d: %s %s\n", idx, type_name.c_str(), argName);
      }
    }
  }

  // Release intermediate programs and store the final program.
  if (!DeviceInfo->Option.Flags.EnableSimd) {
    for (uint32_t i = 0; i < programs.size(); i++) {
      CALL_CL_EXIT_FAIL(clReleaseProgram, programs[i]);
    }
  }

  if (DeviceInfo->initProgramData(device_id) != OFFLOAD_SUCCESS)
    return nullptr;
  __tgt_target_table &table = FuncGblEntries.Table;
  table.EntriesBegin = &(entries[0]);
  table.EntriesEnd = &(entries.data()[entries.size()]);

  OMPT_CALLBACK(ompt_callback_device_load, device_id,
                "" /* filename */,
                -1 /* offset_in_file */,
                nullptr /* vma_in_file */,
                table.EntriesEnd - table.EntriesBegin /* bytes */,
                table.EntriesBegin /* host_addr */,
                nullptr /* device_addr */,
                0 /* module_id */);

  return &table;
}

void event_callback_completed(cl_event event, cl_int status, void *data) {
  if (status == CL_SUCCESS) {
    if (!data) {
      FATAL_ERROR("Invalid asynchronous offloading event");
    }
    AsyncDataTy *async_data = (AsyncDataTy *)data;
    AsyncEventTy *async_event = async_data->Event;
    if (!async_event || !async_event->handler || !async_event->arg) {
      FATAL_ERROR("Invalid asynchronous offloading event");
    }

    cl_command_type cmd;
    CALL_CL_EXIT_FAIL(clGetEventInfo, event, CL_EVENT_COMMAND_TYPE, sizeof(cmd),
                      &cmd, nullptr);

    // Release the temporary cl_mem object used in the data operation.
    if (async_data->MemToRelease &&
        (cmd == CL_COMMAND_READ_BUFFER || cmd == CL_COMMAND_WRITE_BUFFER)) {
      CALL_CL_EXIT_FAIL(clReleaseMemObject, async_data->MemToRelease);
    }

    if (DeviceInfo->Option.Flags.EnableProfile) {
      const char *event_name;
      switch (cmd) {
      case CL_COMMAND_NDRANGE_KERNEL:
        event_name = "KernelAsync";
        break;
      case CL_COMMAND_SVM_MEMCPY:
        event_name = "DataAsync";
        break;
      case CL_COMMAND_READ_BUFFER:
        event_name = "DataReadAsync";
        break;
      case CL_COMMAND_WRITE_BUFFER:
        event_name = "DataWriteAsync";
        break;
      default:
        event_name = "OthersAsync";
      }
      DeviceInfo->getProfiles(async_data->DeviceId).update(event_name, event);
    }

    // Libomptarget is responsible for defining the handler and argument.
    DP("Calling asynchronous offloading event handler " DPxMOD
       " with argument " DPxMOD "\n", DPxPTR(async_event->handler),
       DPxPTR(async_event->arg));
    async_event->handler(async_event->arg);
    delete async_data;
  } else {
    FATAL_ERROR("Failed to complete asynchronous offloading");
  }
}

// Notify the kernel about target pointers that are not explicitly
// passed as arguments, but which are pointing to mapped objects
// that may potentially be accessed in the kernel code (e.g. PTR_AND_OBJ
// objects).
EXTERN int32_t __tgt_rtl_manifest_data_for_region(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtPtrs, size_t NumPtrs) {
  cl_kernel Kernel = *static_cast<cl_kernel *>(TgtEntryPtr);
  DP("Stashing %zu implicit arguments for kernel " DPxMOD "\n", NumPtrs,
     DPxPTR(Kernel));
  auto &KernelProperty = DeviceInfo->KernelProperties[DeviceId][Kernel];
  std::lock_guard<std::mutex>(DeviceInfo->Mutexes[DeviceId]);
  KernelProperty.ImplicitArgs.clear();
  KernelProperty.ImplicitArgs.insert(TgtPtrs, TgtPtrs + NumPtrs);

  return OFFLOAD_SUCCESS;
}

EXTERN void __tgt_rtl_get_offload_queue(int32_t DeviceId, void *Interop,
       bool CreateNew) {

  if (Interop == nullptr) {
    DP("Invalid interop object in %s\n", __func__);
    return;
  }

  __tgt_interop_obj *obj = static_cast<__tgt_interop_obj *>(Interop);
  cl_int status;
  cl_command_queue queue = nullptr;
  auto device = DeviceInfo->Devices[DeviceId];
  auto context = DeviceInfo->getContext(DeviceId);

  // Queue properties for profiling
  cl_queue_properties qProperties[] = {
    CL_QUEUE_PROPERTIES,
    CL_QUEUE_PROFILING_ENABLE,
    0
  };
  auto enableProfile = DeviceInfo->Option.Flags.EnableProfile;

  // Return a shared in-order queue for synchronous case
  if (!CreateNew) {
    std::unique_lock<std::mutex> lock(DeviceInfo->Mutexes[DeviceId]);
    queue = DeviceInfo->QueuesInOrder[DeviceId];
    if (!queue) {
      CALL_CL_RVRC(queue, clCreateCommandQueueWithProperties, status, context,
                   device, enableProfile ? qProperties : nullptr);
      if (status != CL_SUCCESS) {
        DP("Error: Failed to create interop command queue: %d\n", status);
        obj->queue = nullptr;
        return;
      }
      DeviceInfo->QueuesInOrder[DeviceId] = queue;
    }
    DP("%s returns a shared in-order queue " DPxMOD "\n", __func__,
       DPxPTR(queue));
    obj->queue = queue;
    return;
  }

  // Return a new in-order queue for asynchronous case
  CALL_CL_RVRC(queue, clCreateCommandQueueWithProperties, status, context,
               device, enableProfile ? qProperties : nullptr);
  if (status != CL_SUCCESS) {
    DP("Error: Failed to create interop command queue\n");
    obj->queue = nullptr;
    return;
  }
  DP("%s creates and returns a new in-order queue " DPxMOD "\n", __func__,
     DPxPTR(queue));
  obj->queue = queue;
  return;
}

// Release the command queue if it is a new in-order command queue.
EXTERN int32_t __tgt_rtl_release_offload_queue(int32_t device_id, void *queue) {
  cl_command_queue cmdQueue = (cl_command_queue)queue;
  std::unique_lock<std::mutex> lock(DeviceInfo->Mutexes[device_id]);
  if (cmdQueue != DeviceInfo->QueuesInOrder[device_id] &&
      cmdQueue != DeviceInfo->Queues[device_id]) {
    CALL_CL_RET_FAIL(clReleaseCommandQueue, cmdQueue);
    DP("%s releases an in-order queue " DPxMOD "\n", __func__, DPxPTR(queue));
  }
  return OFFLOAD_SUCCESS;
}

EXTERN void *__tgt_rtl_get_platform_handle(int32_t device_id) {
  auto context = DeviceInfo->getContext(device_id);
  return (void *) context;
}

EXTERN void __tgt_rtl_set_device_handle(int32_t DeviceId, void *Interop) {
  if (Interop == nullptr) {
    DP("Invalid interop object in %s\n", __func__);
    return;
  }
  __tgt_interop_obj *obj = static_cast<__tgt_interop_obj *>(Interop);
  obj->device_handle = DeviceInfo->Devices[DeviceId];
}

EXTERN void *__tgt_rtl_get_context_handle(int32_t DeviceId) {
  auto context = DeviceInfo->getContext(DeviceId);
  return (void *)context;
}

// Check if the pointer belongs to a device-accessible pointer ranges
EXTERN int32_t __tgt_rtl_is_device_accessible_ptr(
    int32_t DeviceId, void *Ptr) {
  // What we want here is to check if Ptr is a SVM/USM pointer.
  // For GPU device, use the USM API to allow use of external memory allocation.
  // For CPU device, use the existing internal data when SVM is enabled since
  // USM API does not return consistent result.
  int32_t Ret = 0;
  if (DeviceInfo->Option.Flags.UseSVM &&
      DeviceInfo->Option.DeviceType == CL_DEVICE_TYPE_CPU) {
    if (DeviceInfo->MemAllocInfo[DeviceId]->contains(Ptr, 1))
      Ret = 1;
  } else {
    cl_unified_shared_memory_type_intel memType = 0;
    CALL_CL_EXT_RET(DeviceId, false, clGetMemAllocInfoINTEL,
                    DeviceInfo->getContext(DeviceId), Ptr,
                    CL_MEM_ALLOC_TYPE_INTEL, sizeof(memType), &memType,
                    nullptr);
    switch (memType) {
    case CL_MEM_TYPE_HOST_INTEL:
    case CL_MEM_TYPE_DEVICE_INTEL:
    case CL_MEM_TYPE_SHARED_INTEL:    // Includes SVM on GPU
      Ret = 1;
      break;
    case CL_MEM_TYPE_UNKNOWN_INTEL:   // Normal host memory
    default:
      Ret = 0;
    }
  }
  DP("Ptr " DPxMOD " is %sa device-accessible pointer.\n", DPxPTR(Ptr),
     Ret ? "" : "not ");
  return Ret;
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
  auto MaxSize = DeviceInfo->MaxMemAllocSize[DeviceId];
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
    CALL_CL_EXT_RVRC(DeviceId, Base, clDeviceMemAllocINTEL, RC, Context,
                     DeviceInfo->Devices[DeviceId],
                     getAllocMemProperties(AllocSize, MaxSize)->data(),
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
  auto MaxSize = DeviceInfo->MaxMemAllocSize[DeviceId];
  ProfileIntervalTy DataAllocTimer("DataAlloc", DeviceId);
  DataAllocTimer.start();
  auto ID = DeviceId;

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
                     getAllocMemProperties(Size, MaxSize)->data(), Size, Align);
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
                     getAllocMemProperties(Size, MaxSize)->data(), Size, Align);
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

EXTERN
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

// Allocate a base buffer with the given information.
EXTERN
void *__tgt_rtl_data_alloc_base(int32_t DeviceId, int64_t Size, void *HstPtr,
                                void *HstBase) {
  return dataAlloc(DeviceId, Size, HstPtr, HstBase, false);
}

// Allocate a managed memory object.
EXTERN void *__tgt_rtl_data_alloc_managed(int32_t DeviceId, int64_t Size) {
  int32_t Kind = DeviceInfo->Option.Flags.UseHostMemForUSM
                    ? TARGET_ALLOC_HOST : TARGET_ALLOC_SHARED;
  return dataAllocExplicit(DeviceId, Size, Kind);
}

EXTERN void *__tgt_rtl_data_realloc(
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

EXTERN void *__tgt_rtl_data_aligned_alloc(
    int32_t DeviceId, size_t Align, size_t Size, int32_t Kind) {
  if (Align != 0 && (Align & (Align - 1)) != 0) {
    DP("Error: Alignment %zu is not power of two.\n", Align);
    return nullptr;
  }

  int32_t AllocKind =
      (Kind == TARGET_ALLOC_DEFAULT) ? TARGET_ALLOC_DEVICE : Kind;

  return dataAllocExplicit(DeviceId, Size, AllocKind, Align);
}

EXTERN
int32_t __tgt_rtl_data_submit_nowait(int32_t device_id, void *tgt_ptr,
                                     void *hst_ptr, int64_t size,
                                     void *async_event) {
  if (size == 0)
    // All other plugins seem to be handling 0 size gracefully,
    // so we should do as well.
    return OFFLOAD_SUCCESS;

  cl_command_queue queue = DeviceInfo->Queues[device_id];

  // Add synthetic delay for experiments
  addDataTransferLatency();

  const char *ProfileKey = "DataWrite (Host to Device)";

  AsyncDataTy *async_data = nullptr;
  if (async_event && ((AsyncEventTy *)async_event)->handler) {
    async_data = new AsyncDataTy((AsyncEventTy *)async_event, device_id);
  }

  if (DeviceInfo->Option.Flags.UseBuffer) {
    std::unique_lock<std::mutex> lock(DeviceInfo->Mutexes[device_id]);
    if (DeviceInfo->ClMemBuffers[device_id].count(tgt_ptr) > 0) {
      cl_event event;
      CALL_CL_RET_FAIL(clEnqueueWriteBuffer, queue, (cl_mem)tgt_ptr, CL_FALSE,
                       0, size, hst_ptr, 0, nullptr, &event);
      if (async_data) {
        CALL_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                         &event_callback_completed, async_data);
      } else {
        CALL_CL_RET_FAIL(clWaitForEvents, 1, &event);
        if (DeviceInfo->Option.Flags.EnableProfile)
          DeviceInfo->getProfiles(device_id).update(ProfileKey, event);
      }

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
    if (async_data) {
      CALL_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                       &event_callback_completed, async_data);
    } else {
      CALL_CL_RET_FAIL(clWaitForEvents, 1, &event);
      if (DeviceInfo->Option.Flags.EnableProfile)
        DeviceInfo->getProfiles(device_id).update(ProfileKey, event);
    }
    return OFFLOAD_SUCCESS;
  }

  switch (DeviceInfo->Option.DataTransferMethod) {
  case DATA_TRANSFER_METHOD_SVMMAP: {
    cl_event event;
    if (async_data) {
      // Use SVMMemcpy for asynchronous transfer
      CALL_CL_RET_FAIL(clEnqueueSVMMemcpy, queue, CL_FALSE, tgt_ptr, hst_ptr,
                       size, 0, nullptr, &event);
      CALL_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                       &event_callback_completed, async_data);
    } else {
      ProfileIntervalTy SubmitTime(ProfileKey, device_id);
      SubmitTime.start();

      CALL_CL_RET_FAIL(clEnqueueSVMMap, queue, CL_TRUE, CL_MAP_WRITE, tgt_ptr,
                       size, 0, nullptr, nullptr);
      memcpy(tgt_ptr, hst_ptr, size);
      CALL_CL_RET_FAIL(clEnqueueSVMUnmap, queue, tgt_ptr, 0, nullptr, &event);
      CALL_CL_RET_FAIL(clWaitForEvents, 1, &event);

      SubmitTime.stop();
    }
  } break;
  case DATA_TRANSFER_METHOD_SVMMEMCPY: {
    cl_event event;
    if (async_data) {
      CALL_CL_RET_FAIL(clEnqueueSVMMemcpy, queue, CL_FALSE, tgt_ptr, hst_ptr,
                       size, 0, nullptr, &event);
      CALL_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                       &event_callback_completed, async_data);
    } else {
      CALL_CL_RET_FAIL(clEnqueueSVMMemcpy, queue, CL_TRUE, tgt_ptr, hst_ptr,
                       size, 0, nullptr, &event);
      if (DeviceInfo->Option.Flags.EnableProfile)
        DeviceInfo->getProfiles(device_id).update(ProfileKey, event);
    }
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
    if (async_data) {
      async_data->MemToRelease = mem;
      CALL_CL_RET_FAIL(clEnqueueWriteBuffer, queue, mem, CL_FALSE, 0, size,
                       hst_ptr, 0, nullptr, &event);
      CALL_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                       &event_callback_completed, async_data);
    } else {
      CALL_CL_RET_FAIL(clEnqueueWriteBuffer, queue, mem, CL_FALSE, 0, size,
                       hst_ptr, 0, nullptr, &event);
      CALL_CL_RET_FAIL(clWaitForEvents, 1, &event);
      CALL_CL_RET_FAIL(clReleaseMemObject, mem);
      if (DeviceInfo->Option.Flags.EnableProfile)
        DeviceInfo->getProfiles(device_id).update(ProfileKey, event);
    }
  }
  }
  return OFFLOAD_SUCCESS;
}

EXTERN
int32_t __tgt_rtl_data_submit(int32_t device_id, void *tgt_ptr, void *hst_ptr,
                              int64_t size) {
  return __tgt_rtl_data_submit_nowait(device_id, tgt_ptr, hst_ptr, size,
                                      nullptr);
}

EXTERN
int32_t __tgt_rtl_data_submit_async(int32_t device_id, void *tgt_ptr, void *hst_ptr,
                              int64_t size,
                              __tgt_async_info *AsyncInfoPtr /*not used*/) {
  return __tgt_rtl_data_submit_nowait(device_id, tgt_ptr, hst_ptr, size,
                                      nullptr);
}

EXTERN
int32_t __tgt_rtl_data_retrieve_nowait(int32_t device_id, void *hst_ptr,
                                       void *tgt_ptr, int64_t size,
                                       void *async_event) {
  if (size == 0)
    // All other plugins seem to be handling 0 size gracefully,
    // so we should do as well.
    return OFFLOAD_SUCCESS;

  cl_command_queue queue = DeviceInfo->Queues[device_id];

  // Add synthetic delay for experiments
  addDataTransferLatency();

  const char *ProfileKey = "DataRead (Device to Host)";

  AsyncDataTy *async_data = nullptr;
  if (async_event && ((AsyncEventTy *)async_event)->handler) {
    async_data = new AsyncDataTy((AsyncEventTy *)async_event, device_id);
  }

  if (DeviceInfo->Option.Flags.UseBuffer) {
    std::unique_lock<std::mutex> lock(DeviceInfo->Mutexes[device_id]);
    if (DeviceInfo->ClMemBuffers[device_id].count(tgt_ptr) > 0) {
      cl_event event;
      CALL_CL_RET_FAIL(clEnqueueReadBuffer, queue, (cl_mem)tgt_ptr, CL_FALSE,
                       0, size, hst_ptr, 0, nullptr, &event);
      if (async_data) {
        CALL_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                         &event_callback_completed, async_data);
      } else {
        CALL_CL_RET_FAIL(clWaitForEvents, 1, &event);
        if (DeviceInfo->Option.Flags.EnableProfile)
          DeviceInfo->getProfiles(device_id).update(ProfileKey, event);
      }

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
    if (async_data) {
      CALL_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                       &event_callback_completed, async_data);
    } else {
      CALL_CL_RET_FAIL(clWaitForEvents, 1, &event);
      if (DeviceInfo->Option.Flags.EnableProfile)
        DeviceInfo->getProfiles(device_id).update(ProfileKey, event);
    }
    return OFFLOAD_SUCCESS;
  }

  switch (DeviceInfo->Option.DataTransferMethod) {
  case DATA_TRANSFER_METHOD_SVMMAP: {
    cl_event event;
    if (async_data) {
      // Use SVMMemcpy for asynchronous transfer
      CALL_CL_RET_FAIL(clEnqueueSVMMemcpy, queue, CL_FALSE, hst_ptr, tgt_ptr,
                       size, 0, nullptr, &event);
      CALL_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                       &event_callback_completed, async_data);
    } else {
      ProfileIntervalTy RetrieveTime(ProfileKey, device_id);
      RetrieveTime.start();

      CALL_CL_RET_FAIL(clEnqueueSVMMap, queue, CL_TRUE, CL_MAP_READ, tgt_ptr,
                       size, 0, nullptr, nullptr);
      memcpy(hst_ptr, tgt_ptr, size);
      CALL_CL_RET_FAIL(clEnqueueSVMUnmap, queue, tgt_ptr, 0, nullptr, &event);
      CALL_CL_RET_FAIL(clWaitForEvents, 1, &event);

      RetrieveTime.stop();
    }
  } break;
  case DATA_TRANSFER_METHOD_SVMMEMCPY: {
    cl_event event;
    if (async_data) {
      CALL_CL_RET_FAIL(clEnqueueSVMMemcpy, queue, CL_FALSE, hst_ptr, tgt_ptr,
                       size, 0, nullptr, &event);
      CALL_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                       &event_callback_completed, async_data);
    } else {
      CALL_CL_RET_FAIL(clEnqueueSVMMemcpy, queue, CL_TRUE, hst_ptr, tgt_ptr,
                       size, 0, nullptr, &event);
      if (DeviceInfo->Option.Flags.EnableProfile)
        DeviceInfo->getProfiles(device_id).update(ProfileKey, event);
    }
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
    if (async_data) {
      CALL_CL_RET_FAIL(clEnqueueReadBuffer, queue, mem, CL_FALSE, 0, size,
                       hst_ptr, 0, nullptr, &event);
      async_data->MemToRelease = mem;
      CALL_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                       &event_callback_completed, async_data);
    } else {
      CALL_CL_RET_FAIL(clEnqueueReadBuffer, queue, mem, CL_FALSE, 0, size,
                       hst_ptr, 0, nullptr, &event);
      CALL_CL_RET_FAIL(clWaitForEvents, 1, &event);
      CALL_CL_RET_FAIL(clReleaseMemObject, mem);
      if (DeviceInfo->Option.Flags.EnableProfile)
        DeviceInfo->getProfiles(device_id).update(ProfileKey, event);
    }
  }
  }
  return OFFLOAD_SUCCESS;
}

EXTERN
int32_t __tgt_rtl_data_retrieve(int32_t device_id, void *hst_ptr, void *tgt_ptr,
                                int64_t size) {
  return __tgt_rtl_data_retrieve_nowait(device_id, hst_ptr, tgt_ptr, size,
                                        nullptr);
}

EXTERN
int32_t
__tgt_rtl_data_retrieve_async(int32_t device_id, void *hst_ptr, void *tgt_ptr,
                              int64_t size,
                              __tgt_async_info *AsyncInfoPtr /*not used*/) {
  return __tgt_rtl_data_retrieve_nowait(device_id, hst_ptr, tgt_ptr, size,
                                        nullptr);
}

EXTERN int32_t __tgt_rtl_is_data_exchangable(int32_t SrcId, int32_t DstId) {
  // Only support this case. We don't have any documented OpenCL behavior for
  // cross-device data transfer.
  if (SrcId == DstId)
    return 1;

  return 0;
}

EXTERN int32_t __tgt_rtl_data_exchange(
    int32_t SrcId, void *SrcPtr, int32_t DstId, void *DstPtr, int64_t Size) {
  if (SrcId != DstId)
    return OFFLOAD_FAIL;

  // This is OK for same-device copy with SVM or USM extension.
  return __tgt_rtl_data_submit(DstId, DstPtr, SrcPtr, Size);
}

EXTERN int32_t __tgt_rtl_data_delete(int32_t DeviceId, void *TgtPtr) {
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

static void decideLoopKernelGroupArguments(
    int32_t DeviceId, int32_t ThreadLimit, TgtNDRangeDescTy *LoopLevels,
    cl_kernel Kernel, size_t *GroupSizes, size_t *GroupCounts) {

  size_t maxGroupSize = DeviceInfo->maxWorkGroupSize[DeviceId];
  auto &KernelProperty = DeviceInfo->KernelProperties[DeviceId][Kernel];
  size_t kernelWidth = KernelProperty.Width;
  DP("Assumed kernel SIMD width is %zu\n", KernelProperty.SIMDWidth);
  DP("Preferred group size is multiple of %zu\n", kernelWidth);

  size_t kernelMaxThreadGroupSize = KernelProperty.MaxThreadGroupSize;
  if (kernelMaxThreadGroupSize < maxGroupSize) {
    maxGroupSize = kernelMaxThreadGroupSize;
    DP("Capping maximum thread group size to %zu due to kernel constraints.\n",
       maxGroupSize);
  }

  bool maxGroupSizeForced = false;

  if (ThreadLimit > 0) {
    maxGroupSizeForced = true;

    if ((uint32_t)ThreadLimit <= maxGroupSize) {
      maxGroupSize = ThreadLimit;
      DP("Max group size is set to %zu (thread_limit clause)\n", maxGroupSize);
    } else {
      DP("thread_limit(%" PRIu32 ") exceeds current maximum %zu\n",
         ThreadLimit, maxGroupSize);
    }
  }

  if (DeviceInfo->Option.ThreadLimit > 0) {
    maxGroupSizeForced = true;

    if (DeviceInfo->Option.ThreadLimit <= maxGroupSize) {
      maxGroupSize = DeviceInfo->Option.ThreadLimit;
      DP("Max group size is set to %zu (OMP_THREAD_LIMIT)\n", maxGroupSize);
    } else {
      DP("OMP_THREAD_LIMIT(%" PRIu32 ") exceeds current maximum %zu\n",
         DeviceInfo->Option.ThreadLimit, maxGroupSize);
    }
  }

  if (DeviceInfo->Option.NumTeams > 0)
    DP("OMP_NUM_TEAMS(%" PRIu32 ") is ignored\n", DeviceInfo->Option.NumTeams);

  GroupCounts[0] = GroupCounts[1] = GroupCounts[2] = 1;
  size_t groupSizes[3] = {maxGroupSize, 1, 1};
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
    if (level[i].Ub < level[i].Lb) {
      std::fill(GroupCounts, GroupCounts + 3, 1);
      std::fill(GroupSizes, GroupSizes + 3, 1);
      return;
    }
    tripCounts[i] =
        (level[i].Ub - level[i].Lb + level[i].Stride) / level[i].Stride;
  }

  if (!maxGroupSizeForced) {
    // Use clGetKernelSuggestedLocalWorkSizeINTEL to compute group sizes,
    // or fallback to setting dimension 0 width to SIMDWidth.
    // Note that in case of user-specified LWS groupSizes[0]
    // is already set according to the specified value.
    size_t globalSizes[3] = { tripCounts[0], tripCounts[1], tripCounts[2] };
    if (distributeDim > 0) {
      // There is a distribute dimension.
      globalSizes[distributeDim - 1] *= globalSizes[distributeDim];
      globalSizes[distributeDim] = 1;
    }

    cl_int rc = CL_DEVICE_NOT_FOUND;
    size_t suggestedGroupSizes[3] = {1, 1, 1};
#if INTEL_CUSTOMIZATION
    if (DeviceInfo->Option.Flags.UseDriverGroupSizes &&
        DeviceInfo->isExtensionFunctionEnabled(
            DeviceId, clGetKernelSuggestedLocalWorkSizeINTELId)) {
      CALL_CL_EXT(DeviceId, rc, clGetKernelSuggestedLocalWorkSizeINTEL,
                  DeviceInfo->Queues[DeviceId], Kernel, 3, nullptr, globalSizes,
                  suggestedGroupSizes);
    }
#endif // INTEL_CUSTOMIZATION
    if (rc == CL_SUCCESS) {
      groupSizes[0] = suggestedGroupSizes[0];
      groupSizes[1] = suggestedGroupSizes[1];
      groupSizes[2] = suggestedGroupSizes[2];
    } else if (maxGroupSize > kernelWidth) {
      groupSizes[0] = kernelWidth;
    }
  }

  for (int32_t i = 0; i < numLoopLevels; i++) {
    if (i < distributeDim) {
      GroupCounts[i] = 1;
      continue;
    }
    size_t trip = tripCounts[i];
    if (groupSizes[i] >= trip)
      groupSizes[i] = trip;
    GroupCounts[i] = (trip + groupSizes[i] - 1) / groupSizes[i];
  }
  std::copy(groupSizes, groupSizes + 3, GroupSizes);
}

static void decideKernelGroupArguments(
    int32_t DeviceId, int32_t NumTeams, int32_t ThreadLimit,
    cl_kernel Kernel, size_t *GroupSizes, size_t *GroupCounts) {
#if INTEL_CUSTOMIZATION
  // Default to best GEN9 GT4 configuration initially.
  size_t numSubslices = 9;
  size_t numEUsPerSubslice= 8;
  size_t numThreadsPerEU = 7;
  size_t numEUs = DeviceInfo->maxExecutionUnits[DeviceId];
  if (DeviceInfo->Option.DeviceType == CL_DEVICE_TYPE_GPU) {
    // TODO: we need to find a way to compute the number of sub slices
    //       and number of EUs per sub slice for the particular device.
    if (numEUs >= 256) {
      // Newer GPUs.
      numEUsPerSubslice = 16;
      numSubslices = numEUs / numEUsPerSubslice;
      numThreadsPerEU = 8;
    } else if (numEUs >= 72) {
      // Default GEN9 GT4 configuration.
    } else if (numEUs >= 48) {
      // GT3
      numSubslices = 6;
    } else if (numEUs >= 24) {
      // GT2
      numSubslices = 3;
    } else if (numEUs >= 18) {
      // GT1.5
      numSubslices = 3;
      numEUsPerSubslice = 6;
    } else {
      // GT1
      numSubslices = 2;
      numEUsPerSubslice = 6;
    }

    DPI("numEUsPerSubslice: %zu\n", numEUsPerSubslice);
    DPI("numSubslices: %zu\n", numSubslices);
    DPI("numThreadsPerEU: %zu\n", numThreadsPerEU);
    DPI("totalEUs: %zu\n", numEUs);
  }
#endif // INTEL_CUSTOMIZATION
  const KernelInfoTy *KInfo = DeviceInfo->getKernelInfo(DeviceId, Kernel);
  if (!KInfo) {
    DP("Warning: Cannot find kernel information for kernel " DPxMOD ".\n",
       DPxPTR(Kernel));
  }
  size_t maxGroupSize = DeviceInfo->maxWorkGroupSize[DeviceId];
  bool maxGroupSizeForced = false;
  bool maxGroupCountForced = false;

  auto &KernelProperty = DeviceInfo->KernelProperties[DeviceId][Kernel];
  size_t kernelWidth = KernelProperty.Width;
  size_t simdWidth = KernelProperty.SIMDWidth;
  DP("Assumed kernel SIMD width is %zu\n", simdWidth);
  DP("Preferred group size is multiple of %zu\n", kernelWidth);

  size_t kernelMaxThreadGroupSize = KernelProperty.MaxThreadGroupSize;
  if (kernelMaxThreadGroupSize < maxGroupSize) {
    maxGroupSize = kernelMaxThreadGroupSize;
    DP("Capping maximum thread group size to %zu due to kernel constraints.\n",
       maxGroupSize);
  }

  if (ThreadLimit > 0) {
    maxGroupSizeForced = true;

    if ((uint32_t)ThreadLimit <= maxGroupSize) {
      maxGroupSize = ThreadLimit;
      DP("Max group size is set to %zu (thread_limit clause)\n",
         maxGroupSize);
    } else {
      DP("thread_limit(%" PRIu32 ") exceeds current maximum %zu\n",
         ThreadLimit, maxGroupSize);
    }
  }

  if (DeviceInfo->Option.ThreadLimit > 0) {
    maxGroupSizeForced = true;

    if (DeviceInfo->Option.ThreadLimit <= maxGroupSize) {
      maxGroupSize = DeviceInfo->Option.ThreadLimit;
      DP("Max group size is set to %zu (OMP_THREAD_LIMIT)\n", maxGroupSize);
    } else {
      DP("OMP_THREAD_LIMIT(%" PRIu32 ") exceeds current maximum %zu\n",
         DeviceInfo->Option.ThreadLimit, maxGroupSize);
    }
  }

  size_t maxGroupCount = 0;

  if (NumTeams > 0) {
    maxGroupCount = NumTeams;
    maxGroupCountForced = true;
    DP("Max group count is set to %zu "
       "(num_teams clause or no teams construct)\n", maxGroupCount);
  } else if (DeviceInfo->Option.NumTeams > 0) {
    // OMP_NUM_TEAMS only matters, if num_teams() clause is absent.
    maxGroupCount = DeviceInfo->Option.NumTeams;
    maxGroupCountForced = true;
    DP("Max group count is set to %zu (OMP_NUM_TEAMS)\n", maxGroupCount);
  }

  if (maxGroupCountForced) {
    // If number of teams is specified by the user, then use kernelWidth
    // WIs per WG by default, so that it matches
    // decideLoopKernelGroupArguments() behavior.
    if (!maxGroupSizeForced) {
      maxGroupSize = kernelWidth;
    }
  } else {
    maxGroupCount = DeviceInfo->maxExecutionUnits[DeviceId];
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
      size_t numThreadsPerSubslice = numEUsPerSubslice * numThreadsPerEU;
      maxGroupCount = numSubslices * numThreadsPerSubslice;
      if (maxGroupSizeForced) {
        // Set group size for the HW capacity
        size_t numThreadsPerGroup =
            (maxGroupSize + simdWidth - 1) / simdWidth;
        size_t numGroupsPerSubslice =
            (numThreadsPerSubslice + numThreadsPerGroup - 1) /
            numThreadsPerGroup;
        maxGroupCount = numGroupsPerSubslice * numSubslices;
      } else {
        // For kernels with cross-WG reductions use LWS equal
        // to kernelWidth. This is just a performance heuristic.
        if (KInfo && KInfo->getHasTeamsReduction() &&
#if INTEL_CUSTOMIZATION
            // Only do this for discrete devices.
            DeviceInfo->isDiscreteDevice(DeviceId) &&
#endif // INTEL_CUSTOMIZATION
            DeviceInfo->Option.ReductionSubscriptionRate)
          maxGroupSize = kernelWidth;

        assert(!maxGroupSizeForced && !maxGroupCountForced);
        assert((maxGroupSize <= kernelWidth ||
                maxGroupSize % kernelWidth == 0) && "Invalid maxGroupSize");
        // Maximize group size
        while (maxGroupSize >= kernelWidth) {
          size_t numThreadsPerGroup =
              (maxGroupSize + simdWidth - 1) / simdWidth;
          if (numThreadsPerSubslice % numThreadsPerGroup == 0) {
            size_t numGroupsPerSubslice =
                numThreadsPerSubslice / numThreadsPerGroup;
            maxGroupCount = numGroupsPerSubslice * numSubslices;
            break;
          }
          maxGroupSize -= kernelWidth;
        }
      }
    }
#endif  // INTEL_CUSTOMIZATION
  }

  GroupSizes[0] = maxGroupSize;
  GroupSizes[1] = GroupSizes[2] = 1;

  if (KInfo && KInfo->getWINum()) {
    GroupSizes[0] =
        (std::min)(KInfo->getWINum(), static_cast<uint64_t>(GroupSizes[0]));
    DP("Capping maximum thread group size to %" PRIu64
       " due to kernel constraints (reduction).\n", KInfo->getWINum());
  }

  GroupCounts[0] = maxGroupCount;
  GroupCounts[1] = GroupCounts[2] = 1;
  if (!maxGroupCountForced) {
    if (KInfo && KInfo->getHasTeamsReduction() &&
        DeviceInfo->Option.ReductionSubscriptionRate) {
#if INTEL_CUSTOMIZATION
      if (DeviceInfo->isDiscreteDevice(DeviceId)) {
#endif // INTEL_CUSTOMIZATION
      GroupCounts[0] /= DeviceInfo->Option.ReductionSubscriptionRate;
      GroupCounts[0] = (std::max)(GroupCounts[0], size_t(1));
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
    int32_t ThreadLimit, void *LoopDesc, void *AsyncEvent) {

  cl_kernel Kernel = *static_cast<cl_kernel *>(TgtEntryPtr);
  if (!Kernel) {
    REPORT("Failed to invoke deleted kernel.\n");
    return OFFLOAD_FAIL;
  }

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

  DP("Group sizes = {%zu, %zu, %zu}\n", LocalWorkSize[0], LocalWorkSize[1],
     LocalWorkSize[2]);
  DP("Group counts = {%zu, %zu, %zu}\n", GlobalWorkSize[0] / LocalWorkSize[0],
     GlobalWorkSize[1] / LocalWorkSize[1],
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

  if (OMPT_ENABLED) {
    // Push current work size
    size_t FinalNumTeams =
        GlobalWorkSize[0] * GlobalWorkSize[1] * GlobalWorkSize[2];
    size_t FinalThreadLimit =
        LocalWorkSize[0] * LocalWorkSize[1] * LocalWorkSize[2];
    FinalNumTeams /= FinalThreadLimit;
    OmptGlobal->getTrace().pushWorkSize(FinalNumTeams, FinalThreadLimit);
  }

  cl_event Event;
  OCL_KERNEL_BEGIN(DeviceId);
  CALL_CL_RET_FAIL(clEnqueueNDRangeKernel, DeviceInfo->Queues[DeviceId],
                   Kernel, 3, nullptr, GlobalWorkSize,
                   LocalWorkSize, 0, nullptr, &Event);

  DeviceInfo->Mutexes[DeviceId].unlock();

  DP("Started executing kernel.\n");

  if (AsyncEvent) {
    if (((AsyncEventTy *)AsyncEvent)->handler) {
      // Add event handler if necessary.
      CALL_CL_RET_FAIL(clSetEventCallback, Event, CL_COMPLETE,
          &event_callback_completed,
          new AsyncDataTy((AsyncEventTy *)AsyncEvent, DeviceId));
    } else {
      // Make sure all queued commands finish before the next one starts.
      CALL_CL_RET_FAIL(clEnqueueBarrierWithWaitList,
                       DeviceInfo->Queues[DeviceId], 0, nullptr, nullptr);
    }
  } else {
    CALL_CL_RET_FAIL(clWaitForEvents, 1, &Event);
    OCL_KERNEL_END(DeviceId);
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
  }

  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_run_target_team_nd_region(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit, void *LoopDesc) {
  return runTargetTeamNDRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                               NumArgs, NumTeams, ThreadLimit, LoopDesc,
                               nullptr);
}

EXTERN int32_t __tgt_rtl_run_target_team_nd_region_nowait(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit, void *LoopDesc,
    void *AsyncEvent) {
  return runTargetTeamNDRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                               NumArgs, NumTeams, ThreadLimit, LoopDesc,
                               AsyncEvent);
}

EXTERN int32_t __tgt_rtl_run_target_team_region_nowait(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit,
    uint64_t LoopTripCount, void *AsyncEvent) {
  return runTargetTeamNDRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                               NumArgs, NumTeams, ThreadLimit, nullptr,
                               AsyncEvent);
}

EXTERN int32_t __tgt_rtl_run_target_region_nowait(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, void *AsyncEvent) {
  return runTargetTeamNDRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                               NumArgs, 1, 0, nullptr, AsyncEvent);
}

EXTERN int32_t __tgt_rtl_run_target_team_region(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit,
    uint64_t LoopTripCount /*not used*/) {
  return runTargetTeamNDRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                               NumArgs, NumTeams, ThreadLimit, nullptr,
                               nullptr);
}

EXTERN int32_t __tgt_rtl_run_target_team_region_async(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, int32_t NumTeams, int32_t ThreadLimit,
    uint64_t LoopTripCount /*not used*/,
    __tgt_async_info *AsyncInfoPtr /*not used*/) {
  return runTargetTeamNDRegion(DeviceId, TgtEntryPtr, TgtArgs, TgtOffsets,
                               NumArgs, NumTeams, ThreadLimit, nullptr,
                               nullptr);
}

EXTERN int32_t __tgt_rtl_run_target_region(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs) {
  // use one team!
  return __tgt_rtl_run_target_team_region(DeviceId, TgtEntryPtr, TgtArgs,
                                          TgtOffsets, NumArgs, 1, 0, 0);
}

EXTERN int32_t __tgt_rtl_run_target_region_async(
    int32_t DeviceId, void *TgtEntryPtr, void **TgtArgs, ptrdiff_t *TgtOffsets,
    int32_t NumArgs, __tgt_async_info *AsyncInfoPtr /*not used*/) {
  // use one team!
  return __tgt_rtl_run_target_team_region(DeviceId, TgtEntryPtr, TgtArgs,
                                          TgtOffsets, NumArgs, 1, 0, 0);
}

EXTERN char *__tgt_rtl_get_device_name(
    int32_t device_id, char *buffer, size_t buffer_max_size) {
  assert(buffer && "buffer cannot be nullptr.");
  assert(buffer_max_size > 0 && "buffer_max_size cannot be zero.");
  CALL_CL_RET_NULL(clGetDeviceInfo, DeviceInfo->Devices[device_id],
                   CL_DEVICE_NAME, buffer_max_size, buffer, nullptr);
  return buffer;
}

EXTERN int32_t __tgt_rtl_synchronize(int32_t device_id,
                                     __tgt_async_info *async_info_ptr) {
  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_get_data_alloc_info(
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

EXTERN void __tgt_rtl_add_build_options(
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
  ret->FrId = OCLInterop::FrId;
  ret->FrName = OCLInterop::FrName;
  ret->Vendor = OCLInterop::Vendor;
  ret->VendorName = OCLInterop::VendorName;
  ret->DeviceNum = DeviceId;

  auto platform = DeviceInfo->Platforms[DeviceId];
  auto context = DeviceInfo->getContext(DeviceId);
  auto device = DeviceInfo->Devices[DeviceId];

  if (InteropContext == OMP_INTEROP_CONTEXT_TARGET ||
      InteropContext == OMP_INTEROP_CONTEXT_TARGETSYNC) {
    ret->Platform = platform;
    ret->Device = device;
    ret->DeviceContext = context;
  }
  if (InteropContext == OMP_INTEROP_CONTEXT_TARGETSYNC) {
    // Create a new out-of-order queue with profiling enabled.
    cl_queue_properties properties[] = {
      CL_QUEUE_PROPERTIES,
      CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE,
      0
    };
    cl_command_queue cmdQueue = nullptr;
    cl_int rc;
    CALL_CL_RVRC(cmdQueue, clCreateCommandQueueWithProperties, rc, context,
                 device, properties);
    if (rc != CL_SUCCESS) {
      DP("Error: Failed to create targetsync for interop\n");
      delete ret;
      return nullptr;
    }
    ret->TargetSync = cmdQueue;
  }

  ret->RTLProperty = new OCLInterop::Property();

  return ret;
}

EXTERN int32_t __tgt_rtl_release_interop(
    int32_t DeviceId, __tgt_interop *Interop) {
  if (!Interop || Interop->DeviceNum != (intptr_t)DeviceId ||
      Interop->FrId != OCLInterop::FrId) {
    DP("Invalid/inconsistent OpenMP interop " DPxMOD "\n", DPxPTR(Interop));
    return OFFLOAD_FAIL;
  }

  if (Interop->TargetSync) {
    auto cmdQueue = static_cast<cl_command_queue>(Interop->TargetSync);
    CALL_CL_RET_FAIL(clFinish, cmdQueue);
    CALL_CL_RET_FAIL(clReleaseCommandQueue, cmdQueue);
  }

  auto OCL = static_cast<OCLInterop::Property *>(Interop->RTLProperty);
  delete OCL;
  delete Interop;

  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_use_interop(int32_t DeviceId, __tgt_interop *Interop) {
  if (!Interop || Interop->DeviceNum != (intptr_t)DeviceId ||
      Interop->FrId != OCLInterop::FrId) {
    DP("Invalid/inconsistent OpenMP interop " DPxMOD "\n", DPxPTR(Interop));
    return OFFLOAD_FAIL;
  }

  if (Interop->TargetSync) {
    auto cmdQueue = static_cast<cl_command_queue>(Interop->TargetSync);
    CALL_CL_RET_FAIL(clFinish, cmdQueue);
  }

  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_get_num_interop_properties(int32_t DeviceId) {
  // TODO: decide implementation-defined properties
  return 0;
}

/// Return the value of the requested property
EXTERN int32_t __tgt_rtl_get_interop_property_value(
    int32_t DeviceId, __tgt_interop *Interop, int32_t Ipr, int32_t ValueType,
    size_t Size, void *Value) {

  if (Interop->RTLProperty == nullptr)
    return omp_irc_out_of_range;

  int32_t retCode = omp_irc_success;

  // TODO
  //  switch (Ipr) {
  //default:
  //retCode = omp_irc_out_of_range;
  //}

  return retCode;
}

EXTERN const char *__tgt_rtl_get_interop_property_info(
    int32_t DeviceId, int32_t Ipr, int32_t InfoType) {
  int32_t offset = Ipr - omp_ipr_first;
  if (offset < 0 || (size_t)offset >= OCLInterop::IprNames.size())
    return nullptr;

  if (InfoType == OMP_IPR_INFO_NAME)
    return OCLInterop::IprNames[offset];
  else if (InfoType == OMP_IPR_INFO_TYPE_DESC)
    return OCLInterop::IprTypeDescs[offset];

  return nullptr;
}

EXTERN const char *__tgt_rtl_get_interop_rc_desc(int32_t DeviceId,
                                                 int32_t RetCode) {
  // TODO: decide implementation-defined return code.
  return nullptr;
}

EXTERN int32_t __tgt_rtl_is_accessible_addr_range(
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

EXTERN int32_t __tgt_rtl_notify_indirect_access(
    int32_t DeviceId, const void *Ptr, size_t Offset) {
#if INTEL_CUSTOMIZATION
  auto Fn = reinterpret_cast<clGitsIndirectAllocationOffsets_fn>(
      DeviceInfo->getExtensionFunctionPtr(DeviceId,
                                          clGitsIndirectAllocationOffsetsId));
  void *PtrBase = (void *)((uintptr_t)Ptr - Offset);
  // This DP is only for testability
  DP("Notifying indirect access: " DPxMOD " + %zu\n", DPxPTR(PtrBase), Offset);
  if (Fn) {
    Fn(PtrBase, 1, &Offset);
  }
#endif // INTEL_CUSTOMIZATION
  return OFFLOAD_SUCCESS;
}

EXTERN int32_t __tgt_rtl_is_private_arg_on_host(
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
#ifdef __cplusplus
}
#endif

#endif // INTEL_COLLAB

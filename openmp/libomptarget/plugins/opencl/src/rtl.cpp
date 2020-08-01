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
#include <vector>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <Windows.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

#include "omptargetplugin.h"
#include "omptarget-tools.h"
#include "rtl-trace.h"

int DebugLevel = 0;

#if INTEL_CUSTOMIZATION
// FIXME: find a way to include cl_usm_ext.h to get these definitions
//        from there.
#define CL_MEM_ALLOC_TYPE_INTEL         0x419A
#define CL_MEM_TYPE_UNKNOWN_INTEL       0x4196
#define CL_MEM_TYPE_HOST_INTEL          0x4197
#define CL_MEM_TYPE_DEVICE_INTEL        0x4198
#define CL_MEM_TYPE_SHARED_INTEL        0x4199

#define CL_MEM_ALLOC_FLAGS_INTEL        0x4195

#define CL_KERNEL_EXEC_INFO_INDIRECT_DEVICE_ACCESS_INTEL    0x4201
#define CL_KERNEL_EXEC_INFO_USM_PTRS_INTEL                  0x4203

typedef cl_uint cl_mem_info_intel;
typedef cl_uint cl_unified_shared_memory_type_intel;
typedef cl_bitfield cl_mem_properties_intel;
typedef cl_int  (CL_API_CALL *clGetMemAllocInfoINTELTy)(
    cl_context context,
    const void* ptr,
    cl_mem_info_intel param_name,
    size_t param_value_size,
    void* param_value,
    size_t* param_value_size_ret);
typedef void * (CL_API_CALL *clHostMemAllocINTELTy)(
    cl_context context,
    cl_mem_properties_intel *properties,
    size_t size,
    cl_uint alignment,
    cl_int *errcodeRet);
typedef void * (CL_API_CALL *clSharedMemAllocINTELTy)(
    cl_context context,
    cl_device_id device,
    const cl_mem_properties_intel *properties,
    size_t size,
    cl_uint alignment,
    cl_int *errcode_ret);
typedef cl_int (CL_API_CALL *clMemFreeINTELTy)(
    cl_context context,
    const void *ptr);
typedef cl_int (CL_API_CALL *clGetDeviceGlobalVariablePointerINTELTy)(
    cl_device_id,
    cl_program,
    const char *,
    size_t *,
    void **);
#endif // INTEL_CUSTOMIZATION

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// FIXME: we should actually include omp.h instead of declaring
//        these ourselves.
#if _WIN32
int __cdecl omp_get_max_teams(void);
int __cdecl omp_get_thread_limit(void);
double __cdecl omp_get_wtime(void);
#else   // !_WIN32
int omp_get_max_teams(void) __attribute__((weak));
int omp_get_thread_limit(void) __attribute__((weak));
double omp_get_wtime(void) __attribute__((weak));
#endif  // !_WIN32

#ifdef __cplusplus
}
#endif  // __cplusplus

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define OFFLOADSECTIONNAME "omp_offloading_entries"

//#pragma OPENCL EXTENSION cl_khr_spir : enable

/// Keep entries table per device.
struct FuncOrGblEntryTy {
  __tgt_target_table Table;
  std::vector<__tgt_offload_entry> Entries;
  std::vector<cl_kernel> Kernels;
  cl_program Program;
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

  void printData(int32_t deviceId, const char *deviceName, int64_t resolution) {
    fprintf(stderr, "LIBOMPTARGET_PROFILE for OMP DEVICE(%" PRId32 ") %s\n",
            deviceId, deviceName);
    const char *units = resolution == 1000 ? "msec" : "usec";
    fprintf(stderr, "-- Name:\tHost Time (%s)\tDevice Time (%s)\n",
            units, units);
    double host_total = 0.0;
    double device_total = 0.0;
    for (const auto &d : data) {
      double host_time = 1e-9 * d.second.host * resolution;
      double device_time = 1e-9 * d.second.device * resolution;
      fprintf(stderr, "-- %s:\t%.3f\t%.3f\n", d.first.c_str(),
              host_time, device_time);
      host_total += host_time;
      device_total += device_time;
    }
    fprintf(stderr, "-- Total:\t%.3f\t%.3f\n", host_total, device_total);
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
#if INTEL_CUSTOMIZATION
  // clGetDeviceGlobalVariablePointerINTEL API:
  ExtensionStatusTy GetDeviceGlobalVariablePointer = ExtensionStatusUnknown;
  ExtensionStatusTy GetMemAllocInfoINTELPointer = ExtensionStatusUnknown;
  ExtensionStatusTy HostMemAllocINTELPointer = ExtensionStatusUnknown;
  ExtensionStatusTy SharedMemAllocINTELPointer = ExtensionStatusUnknown;
  ExtensionStatusTy MemFreeINTELPointer = ExtensionStatusUnknown;
  ExtensionStatusTy SuggestedGroupSize = ExtensionStatusUnknown;
#endif  // INTEL_CUSTOMIZATION

  // Libdevice extensions that may be supported by device runtime.
  struct LibdeviceExtDescTy {
    const char *Name;
    const char *FallbackLibName;
    ExtensionStatusTy Status;
  };

  std::vector<LibdeviceExtDescTy> LibdeviceExtensions = {
    {
      "cl_intel_assert",
      "libomp-fallback-cassert.spv",
      ExtensionStatusUnknown
    },
    {
      "cl_intel_math",
      "libomp-fallback-cmath.spv",
      ExtensionStatusUnknown
    },
    {
      "cl_intel_math_fp64",
      "libomp-fallback-cmath-fp64.spv",
      ExtensionStatusUnknown
    },
    {
      "cl_intel_complex",
      "libomp-fallback-complex.spv",
      ExtensionStatusUnknown
    },
    {
      "cl_intel_complex_fp64",
      "libomp-fallback-complex-fp64.spv",
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

/// Buffer allocation information.
struct BufferInfoTy {
  void *Base;   // Base address
  int64_t Size; // Allocation size
};

/// Program data to be initialized.
/// TODO: include other runtime parameters if necessary.
struct ProgramData {
  int Initialized = 0;
  int NumDevices = 0;
  int DeviceNum = -1;
};

/// RTL flags
struct RTLFlagsTy {
  uint64_t CollectDataTransferLatency : 1;
  uint64_t EnableProfile : 1;
  uint64_t UseInteropQueueInorderAsync : 1;
  uint64_t UseInteropQueueInorderSharedSync : 1;
  uint64_t UseHostMemForUSM : 1;
  uint64_t UseDriverGroupSizes : 1;
  // Add new flags here
  uint64_t Reserved : 58;
  RTLFlagsTy() :
      CollectDataTransferLatency(0),
      EnableProfile(0),
      UseInteropQueueInorderAsync(0),
      UseInteropQueueInorderSharedSync(0),
      UseHostMemForUSM(0),
      UseDriverGroupSizes(0),
      Reserved(0) {}
};

/// Kernel properties.
struct KernelPropertiesTy {
  size_t Width = 0;
  size_t MaxThreadGroupSize = 0;
};

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
public:
  cl_uint numDevices;

  // per device information
  std::vector<cl_platform_id> platformIDs;
  std::vector<cl_device_id> deviceIDs;
  std::vector<int32_t> maxExecutionUnits;
  std::vector<size_t> maxWorkGroupSize;

  // A vector of descriptors of OpenCL extensions for each device.
  std::vector<ExtensionsTy> Extensions;
  std::vector<cl_context> CTX;
  std::vector<cl_command_queue> Queues;
  std::vector<cl_command_queue> QueuesOOO; // out-of-order queues
  std::vector<FuncOrGblEntryTy> FuncGblEntries;
  std::vector<std::map<cl_kernel, KernelPropertiesTy>>
      KernelProperties;
  std::vector<std::map<void *, BufferInfoTy> > Buffers;
  std::vector<std::map<cl_kernel, std::set<void *> > > ImplicitArgs;
  std::vector<ProfileDataTy> Profiles;
  std::vector<std::vector<char>> Names;
  std::vector<bool> Initialized;
  std::vector<cl_ulong> SLMSize;
  std::vector<std::map<void *, int64_t>> ManagedData;
  std::mutex *Mutexes;
  std::vector<std::vector<DeviceOffloadEntryTy>> OffloadTables;

  // Requires flags
  int64_t RequiresFlags = OMP_REQ_UNDEFINED;

  RTLFlagsTy Flags;
  int32_t DataTransferLatency;
  int32_t DataTransferMethod;
  int64_t ProfileResolution;
  cl_device_type DeviceType;

  // OpenCL 2.0 builtins (like atomic_load_explicit and etc.) are used by
  // runtime, so we have to explicitly specify the "-cl-std=CL2.0" compilation
  // option. With it, the SPIR-V will be converted to LLVM IR with OpenCL 2.0
  // builtins. Otherwise, SPIR-V will be converted to LLVM IR with OpenCL 1.2
  // builtins.
  std::string CompilationOptions = "-cl-std=CL2.0 ";
  std::string LinkingOptions;

#if INTEL_CUSTOMIZATION
  std::string InternalCompilationOptions;
  std::string InternalLinkingOptions;

  // A pointer to clGetMemAllocInfoINTEL extension API.
  // It can be used to distinguish SVM and USM pointers.
  // It is available on the whole platform, so it is not
  // device-specific within the same platform.
  clGetMemAllocInfoINTELTy clGetMemAllocInfoINTELFn = nullptr;
  clHostMemAllocINTELTy clHostMemAllocINTELFn = nullptr;
  clSharedMemAllocINTELTy clSharedMemAllocINTELFn = nullptr;
  clMemFreeINTELTy clMemFreeINTELFn = nullptr;
  clGetDeviceGlobalVariablePointerINTELTy
      clGetDeviceGlobalVariablePointerINTELFn = nullptr;
  clGetKernelSuggestedLocalWorkSizeINTELTy
      clGetKernelSuggestedLocalWorkSizeINTELFn = nullptr;
#endif  // INTEL_CUSTOMIZATION

  // Limit for the number of WIs in a WG.
  uint32_t ThreadLimit = 0;
  // Limit for the number of WGs.
  uint32_t NumTeams = 0;

  // This is a factor applied to the number of WGs computed
  // for the execution, based on the HW characteristics.
  size_t SubscriptionRate = 1;
#if INTEL_INTERNAL_BUILD
  size_t ForcedLocalSizes[3] = {0, 0, 0};
  size_t ForcedGlobalSizes[3] = {0, 0, 0};
#endif // INTEL_INTERNAL_BUILD

  RTLDeviceInfoTy() : numDevices(0), DataTransferLatency(0),
      DataTransferMethod(DATA_TRANSFER_METHOD_CLMEM) {
    char *env;
    if (env = readEnvVar("LIBOMPTARGET_DEBUG")) {
      DebugLevel = std::stoi(env);
    }
    // set misc. flags

    // Get global OMP_THREAD_LIMIT for SPMD parallelization.
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

    // Read LIBOMPTARGET_DATA_TRANSFER_LATENCY (experimental input)
    if (env = readEnvVar("LIBOMPTARGET_DATA_TRANSFER_LATENCY")) {
      std::string value(env);
      if (value.substr(0, 2) == "T,") {
        Flags.CollectDataTransferLatency = 1;
        int32_t usec = std::stoi(value.substr(2).c_str());
        DataTransferLatency = (usec > 0) ? usec : 0;
      }
    }

    // Read LIBOMPTARGET_DATA_TRANSFER_METHOD
    // Read LIBOMPTARGET_OPENCL_DATA_TRANSFER_METHOD
    if (env = readEnvVar("LIBOMPTARGET_OPENCL_DATA_TRANSFER_METHOD",
                         "LIBOMPTARGET_DATA_TRANSFER_METHOD")) {
      std::string value(env);
      DataTransferMethod = DATA_TRANSFER_METHOD_INVALID;
      if (value.size() == 1 && std::isdigit(value.c_str()[0])) {
        int method = std::stoi(env);
        if (method < DATA_TRANSFER_METHOD_LAST)
          DataTransferMethod = method;
      }
      if (DataTransferMethod == DATA_TRANSFER_METHOD_INVALID) {
        WARNING("Invalid data transfer method (%s) selected"
                " -- using default method.\n", env);
        DataTransferMethod = DATA_TRANSFER_METHOD_CLMEM;
      }
    }
    // Read LIBOMPTARGET_DEVICETYPE
    DeviceType = CL_DEVICE_TYPE_GPU;
    if (env = readEnvVar("LIBOMPTARGET_DEVICETYPE")) {
      std::string value(env);
      if (value == "GPU" || value == "gpu")
        DeviceType = CL_DEVICE_TYPE_GPU;
      else if (value == "CPU" || value == "cpu")
        DeviceType = CL_DEVICE_TYPE_CPU;
      else
        WARNING("Invalid or unsupported LIBOMPTARGET_DEVICETYPE=%s\n", env);
    }
    DP("Target device type is set to %s\n",
       (DeviceType == CL_DEVICE_TYPE_CPU) ? "CPU" : "GPU");

#if INTEL_CUSTOMIZATION
    if (DeviceType == CL_DEVICE_TYPE_GPU) {
      // Default subscription rate is heuristically set to 4 for GPU.
      // It only matters for the default ND-range parallelization,
      // i.e. when the global size is unknown on the host.
      SubscriptionRate = 4;
    }
#endif  // INTEL_CUSTOMIZATION

    if (env = readEnvVar("LIBOMPTARGET_OPENCL_SUBSCRIPTION_RATE")) {
      int32_t value = std::stoi(env);

      // Set some reasonable limits.
      if (value > 0 || value <= 0xFFFF)
        SubscriptionRate = value;
    }

    // Read LIBOMPTARGET_PROFILE
    ProfileResolution = 1000;
    if (env = readEnvVar("LIBOMPTARGET_PROFILE")) {
      std::istringstream value(env);
      std::string token;
      while (std::getline(value, token, ',')) {
        if (token == "T" || token == "1")
          Flags.EnableProfile = 1;
        else if (token == "unit_usec" || token == "usec")
          ProfileResolution = 1000000;
      }
    }

    // Read LIBOMPTARGET_OPENCL_INTEROP_QUEUE
    // Two independent options can be specified as follows.
    // -- inorder_async: use a new in-order queue for asynchronous case
    //    (default: shared out-of-order queue)
    // -- inorder_shared_sync: use the existing shared in-order queue for
    //    synchronous case (default: new in-order queue).
    if (env = readEnvVar("LIBOMPTARGET_OPENCL_INTEROP_QUEUE",
                         "LIBOMPTARGET_INTEROP_PIPE")) {
      std::istringstream value(env);
      std::string token;
      while (std::getline(value, token, ',')) {
        if (token == "inorder_async") {
          Flags.UseInteropQueueInorderAsync = 1;
          DP("    enabled in-order asynchronous separate queue\n");
        } else if (token == "inorder_shared_sync") {
          Flags.UseInteropQueueInorderSharedSync = 1;
          DP("    enabled in-order synchronous shared queue\n");
        }
      }
    }

    if (env = readEnvVar("LIBOMPTARGET_OPENCL_COMPILATION_OPTIONS")) {
      CompilationOptions += env;
    }
    if (env = readEnvVar("LIBOMPTARGET_OPENCL_LINKING_OPTIONS")) {
      LinkingOptions += env;
    }
#if INTEL_CUSTOMIZATION
    // OpenCL CPU compiler complains about unsupported option.
    // Intel Graphics compilers that do not support that option
    // silently ignore it.
    if (DeviceType == CL_DEVICE_TYPE_GPU) {
      if (!(env = readEnvVar("LIBOMPTARGET_OPENCL_TARGET_GLOBALS")) ||
          (env[0] != 'F' && env[0] != 'f' && env[0] != '0'))
        InternalLinkingOptions += " -cl-take-global-address ";
      if (!(env = readEnvVar("LIBOMPTARGET_OPENCL_MATCH_SINCOSPI")) ||
          (env[0] != 'F' && env[0] != 'f' && env[0] != '0'))
        InternalLinkingOptions += " -cl-match-sincospi ";
      if (env = readEnvVar("LIBOMPTARGET_OPENCL_USE_DRIVER_GROUP_SIZES"))
        if (env[0] == 'T' || env[0] == 't' || env[0] == '1')
        Flags.UseDriverGroupSizes = 1;
    }
#endif  // INTEL_CUSTOMIZATION

    // Read LIBOMPTARGET_USM_HOST_MEM
    if (env = readEnvVar("LIBOMPTARGET_USM_HOST_MEM")) {
      if (env[0] == 'T' || env[0] == 't' || env[0] == '1')
        Flags.UseHostMemForUSM = 1;
    }

#if INTEL_INTERNAL_BUILD
    // Force work group sizes -- for internal experiments
    if (env = readEnvVar("LIBOMPTARGET_LOCAL_WG_SIZE")) {
      parseGroupSizes("LIBOMPTARGET_LOCAL_WG_SIZE", env, ForcedLocalSizes);
    }
    if (env = readEnvVar("LIBOMPTARGET_GLOBAL_WG_SIZE")) {
      parseGroupSizes("LIBOMPTARGET_GLOBAL_WG_SIZE", env, ForcedGlobalSizes);
    }
#endif // INTEL_INTERNAL_BUILD
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
  void parseGroupSizes(const char *Name, const char *Value, size_t *Sizes) {
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

#ifdef _WIN32
#define __ATTRIBUTE__(X)
#else
#define __ATTRIBUTE__(X)  __attribute__((X))
#endif // _WIN32

static RTLDeviceInfoTy *DeviceInfo;

__ATTRIBUTE__(constructor(101)) void init() {
  DP("Init OpenCL plugin!\n");
  DeviceInfo = new RTLDeviceInfoTy();
}

__ATTRIBUTE__(destructor(101)) void deinit() {
  DP("Deinit OpenCL plugin!\n");
  delete DeviceInfo;
}

#if _WIN32
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
      ClDeviceId(DeviceInfo->deviceIDs[DeviceId]) {
    if (DeviceInfo->Flags.EnableProfile)
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

    DeviceInfo->Profiles[DeviceId].update(
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
  for (uint32_t i = 0; i < DeviceInfo->numDevices; i++) {
    if (!DeviceInfo->Initialized[i])
      continue;
    if (DeviceInfo->Flags.EnableProfile)
      DeviceInfo->Profiles[i].printData(i, DeviceInfo->Names[i].data(),
                                       DeviceInfo->ProfileResolution);
#ifndef _WIN32
    if (OMPT_ENABLED) {
      // Disabled for Windows to alleviate dll finalization issue.
      OMPT_CALLBACK(ompt_callback_device_unload, i, 0 /* module ID */);
      OMPT_CALLBACK(ompt_callback_device_finalize, i);
    }
    // Making OpenCL calls during process exit on Windows is unsafe.
    for (auto kernel : DeviceInfo->FuncGblEntries[i].Kernels) {
      if (kernel)
        CALL_CL_EXIT_FAIL(clReleaseKernel, kernel);
    }
    // No entries may exist if offloading was done through MKL
    if (DeviceInfo->FuncGblEntries[i].Program)
       CALL_CL_EXIT_FAIL(clReleaseProgram, DeviceInfo->FuncGblEntries[i].Program);
    CALL_CL_EXIT_FAIL(clReleaseCommandQueue, DeviceInfo->Queues[i]);
    if (DeviceInfo->QueuesOOO[i]) {
      CALL_CL_EXIT_FAIL(clReleaseCommandQueue, DeviceInfo->QueuesOOO[i]);
    }
    CALL_CL_EXIT_FAIL(clReleaseContext, DeviceInfo->CTX[i]);
#endif // !defined(_WIN32)
    DeviceInfo->unloadOffloadTable(i);
  }
  delete[] DeviceInfo->Mutexes;
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

/// Invoke kernel to initialize program data.
/// TODO: consider moving allocation of static buffers in device RTL to here
///       as it requires device information.
static int32_t initProgram(int32_t deviceId) {
  int32_t rc;
  ProgramData hostData = {
    1,                              // Initialized
    (int32_t)DeviceInfo->numDevices, // Number of devices
    deviceId                        // Device ID
  };
  auto context = DeviceInfo->CTX[deviceId];
  auto queue = DeviceInfo->Queues[deviceId];
  auto program = DeviceInfo->FuncGblEntries[deviceId].Program;
  cl_mem devData;
  CALL_CL_RVRC(devData, clCreateBuffer, rc, context, CL_MEM_READ_ONLY,
               sizeof(hostData), nullptr);
  if (rc != CL_SUCCESS) {
    DP("Failed to initialize program\n");
    return OFFLOAD_FAIL;
  }
  CALL_CL_RET_FAIL(clEnqueueWriteBuffer, queue, devData, true, 0,
                   sizeof(hostData), &hostData, 0, nullptr, nullptr);
  cl_kernel initPgm;
  CALL_CL_RVRC(initPgm, clCreateKernel, rc, program, "__kmpc_init_program");
  if (rc != CL_SUCCESS) {
    DP("Failed to initialize program\n");
    return OFFLOAD_FAIL;
  }
  size_t globalWork = 1;
  size_t localWork = 1;
  CALL_CL_RET_FAIL(clSetKernelArg, initPgm, 0, sizeof(devData), &devData);
  CALL_CL_RET_FAIL(clEnqueueNDRangeKernel, queue, initPgm, 1, nullptr,
                   &globalWork, &localWork, 0, nullptr, nullptr);
  CALL_CL_RET_FAIL(clFinish, queue);
  CALL_CL_RET_FAIL(clReleaseMemObject, devData);
  CALL_CL_RET_FAIL(clReleaseKernel, initPgm);
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
  size_t DeviceSize = 0;
  void *TgtAddr = nullptr;
  DP("Looking up device global variable '%s' of size %zu bytes on device %d.\n",
     Name, Size, DeviceId);
#if INTEL_CUSTOMIZATION
  if (!clGetDeviceGlobalVariablePointerINTELFn)
    return nullptr;

  if (clGetDeviceGlobalVariablePointerINTELFn(
          deviceIDs[DeviceId], FuncGblEntries[DeviceId].Program,
          Name, &DeviceSize, &TgtAddr) != CL_SUCCESS) {
    DPI("Error: clGetDeviceGlobalVariablePointerINTEL API returned "
        "nullptr for global variable '%s'.\n", Name);
    DeviceSize = 0;
  } else if (Size != DeviceSize) {
    DPI("Error: size mismatch for host (%zu) and device (%zu) versions "
        "of global variable: %s\n.  Direct references "
        "to this variable will not work properly.\n",
        Size, DeviceSize, Name);
    DeviceSize = 0;
  }
#else  // INTEL_CUSTOMIZATION
  // TODO: use device API to get variable address by name.
#endif // INTEL_CUSTOMIZATION

  if (DeviceSize == 0) {
    DP("Error: global variable lookup failed.\n");
    return nullptr;
  }

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

#ifdef __cplusplus
extern "C" {
#endif

static inline void addDataTransferLatency() {
  if (!DeviceInfo->Flags.CollectDataTransferLatency)
    return;
  double goal = omp_get_wtime() + 1e-6 * DeviceInfo->DataTransferLatency;
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

  cl_device_id DeviceId = DeviceInfo->deviceIDs[DeviceNum];
  CALL_CL_RET_FAIL(clGetDeviceInfo, DeviceId, CL_DEVICE_EXTENSIONS, 0, nullptr,
                   &RetSize);

  std::unique_ptr<char []> Data(new char[RetSize]);
  CALL_CL_RET_FAIL(clGetDeviceInfo, DeviceId, CL_DEVICE_EXTENSIONS, RetSize,
                   Data.get(), &RetSize);

  std::string Extensions(Data.get());
  DP("Device extensions: %s\n", Extensions.c_str());

#if INTEL_CUSTOMIZATION
  // Check if the extension was not explicitly disabled, i.e.
  // that its current status is unknown.
  if (GetDeviceGlobalVariablePointer == ExtensionStatusUnknown)
    // FIXME: use the right extension name.
    if (Extensions.find("") != std::string::npos) {
      GetDeviceGlobalVariablePointer = ExtensionStatusEnabled;
      DPI("Extension clGetDeviceGlobalVariablePointerINTEL enabled.\n");
    }

  if (GetMemAllocInfoINTELPointer == ExtensionStatusUnknown)
    if (Extensions.find("cl_intel_unified_shared_memory") !=
        std::string::npos) {
      GetMemAllocInfoINTELPointer = ExtensionStatusEnabled;
      DPI("Extension clGetMemAllocInfoINTEL enabled.\n");
    }

  if (Extensions.find("cl_intel_unified_shared_memory_preview") !=
      std::string::npos) {
    HostMemAllocINTELPointer = ExtensionStatusEnabled;
    SharedMemAllocINTELPointer = ExtensionStatusEnabled;
    MemFreeINTELPointer = ExtensionStatusEnabled;
  }

  if (SuggestedGroupSize == ExtensionStatusUnknown)
    // FIXME: use the right extension name.
    if (Extensions.find("") != std::string::npos) {
      SuggestedGroupSize = ExtensionStatusEnabled;
      DPI("Extension clGetKernelSuggestedLocalWorkSizeINTEL enabled.\n");
    }
#endif  // INTEL_CUSTOMIZATION

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

EXTERN
int32_t __tgt_rtl_is_valid_binary(__tgt_device_image *image) {
  uint32_t magicWord = *(uint32_t *)image->ImageStart;
  // compare magic word in little endian and big endian:
  int32_t ret = (magicWord == 0x07230203 || magicWord == 0x03022307);
  DP("Target binary is %s\n", ret ? "VALID" : "INVALID");
  return ret;
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
    CALL_CL_SILENT(rc, clGetDeviceIDs, id, DeviceInfo->DeviceType, 0, nullptr,
                   &numDevices);
    if (rc != CL_SUCCESS || numDevices == 0)
      continue;

    DP("Platform %s has %" PRIu32 " Devices\n", buf.data(), numDevices);
    std::vector<cl_device_id> devices(numDevices);
    CALL_CL_RET_ZERO(clGetDeviceIDs, id, DeviceInfo->DeviceType, numDevices,
                     devices.data(), nullptr);
    for (auto device : devices) {
      DeviceInfo->deviceIDs.push_back(device);
      DeviceInfo->platformIDs.push_back(id);
    }
    DeviceInfo->numDevices += numDevices;
  }

  DeviceInfo->maxExecutionUnits.resize(DeviceInfo->numDevices);
  DeviceInfo->maxWorkGroupSize.resize(DeviceInfo->numDevices);
  DeviceInfo->Extensions.resize(DeviceInfo->numDevices);
  DeviceInfo->CTX.resize(DeviceInfo->numDevices);
  DeviceInfo->Queues.resize(DeviceInfo->numDevices);
  DeviceInfo->QueuesOOO.resize(DeviceInfo->numDevices);
  DeviceInfo->FuncGblEntries.resize(DeviceInfo->numDevices);
  DeviceInfo->KernelProperties.resize(DeviceInfo->numDevices);
  DeviceInfo->Buffers.resize(DeviceInfo->numDevices);
  DeviceInfo->ImplicitArgs.resize(DeviceInfo->numDevices);
  DeviceInfo->Profiles.resize(DeviceInfo->numDevices);
  DeviceInfo->Names.resize(DeviceInfo->numDevices);
  DeviceInfo->Initialized.resize(DeviceInfo->numDevices);
  DeviceInfo->SLMSize.resize(DeviceInfo->numDevices);
  DeviceInfo->ManagedData.resize(DeviceInfo->numDevices);
  DeviceInfo->Mutexes = new std::mutex[DeviceInfo->numDevices];
  DeviceInfo->OffloadTables.resize(DeviceInfo->numDevices);

  // get device specific information
  for (unsigned i = 0; i < DeviceInfo->numDevices; i++) {
    size_t buf_size;
    cl_int rc;
    cl_device_id deviceId = DeviceInfo->deviceIDs[i];
    CALL_CL(rc, clGetDeviceInfo, deviceId, CL_DEVICE_NAME, 0, nullptr,
            &buf_size);
    if (rc != CL_SUCCESS || buf_size == 0)
      continue;
    DeviceInfo->Names[i].resize(buf_size);
    CALL_CL(rc, clGetDeviceInfo, deviceId, CL_DEVICE_NAME, buf_size,
            DeviceInfo->Names[i].data(), nullptr);
    if (rc != CL_SUCCESS)
      continue;
    DP("Device %d: %s\n", i, DeviceInfo->Names[i].data());
    CALL_CL_RET_ZERO(clGetDeviceInfo, deviceId, CL_DEVICE_MAX_COMPUTE_UNITS, 4,
                     &DeviceInfo->maxExecutionUnits[i], nullptr);
    DP("Number of execution units on the device is %d\n",
       DeviceInfo->maxExecutionUnits[i]);
    CALL_CL_RET_ZERO(clGetDeviceInfo, deviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE,
                     sizeof(size_t), &DeviceInfo->maxWorkGroupSize[i], nullptr);
    DP("Maximum work group size for the device is %d\n",
       static_cast<int32_t>(DeviceInfo->maxWorkGroupSize[i]));
    cl_uint addressmode;
    CALL_CL_RET_ZERO(clGetDeviceInfo, deviceId, CL_DEVICE_ADDRESS_BITS, 4,
                     &addressmode, nullptr);
    DP("Addressing mode is %d bit\n", addressmode);
    CALL_CL_RET_ZERO(clGetDeviceInfo, deviceId, CL_DEVICE_LOCAL_MEM_SIZE,
                     sizeof(cl_ulong), &DeviceInfo->SLMSize[i], nullptr);
    DP("Device local mem size: %zu\n", (size_t)DeviceInfo->SLMSize[i]);
    DeviceInfo->Initialized[i] = false;
  }
  if (DeviceInfo->numDevices == 0) {
    DP("WARNING: No OpenCL devices found.\n");
  }

#ifndef _WIN32
  // Make sure it is registered after OCL handlers are registered.
  // Registerization is done in DLLmain for Windows
  if (std::atexit(closeRTL)) {
    FATAL_ERROR("Registration of clean-up function");
  }
#endif //WIN32

  return DeviceInfo->numDevices;
}

EXTERN
int32_t __tgt_rtl_init_device(int32_t device_id) {
  cl_int status;
  DP("Initialize OpenCL device\n");
  assert(device_id >= 0 && (cl_uint)device_id < DeviceInfo->numDevices &&
         "bad device id");

  // create context
  auto PlatformID = DeviceInfo->platformIDs[device_id];
  cl_context_properties props[] = {
      CL_CONTEXT_PLATFORM,
      (cl_context_properties)PlatformID, 0};
  CALL_CL_RVRC(DeviceInfo->CTX[device_id], clCreateContext, status, props, 1,
               &DeviceInfo->deviceIDs[device_id], nullptr, nullptr);
  if (status != CL_SUCCESS) {
    DP("Error: Failed to create context: %d\n", status);
    return OFFLOAD_FAIL;
  }

  cl_queue_properties qprops[3] = {CL_QUEUE_PROPERTIES, 0, 0};
  if (DeviceInfo->Flags.EnableProfile)
    qprops[1] = CL_QUEUE_PROFILING_ENABLE;

  auto deviceID = DeviceInfo->deviceIDs[device_id];
  auto context = DeviceInfo->CTX[device_id];
  CALL_CL_RVRC(DeviceInfo->Queues[device_id], clCreateCommandQueueWithProperties,
               status, context, deviceID, qprops);
  if (status != 0) {
    DP("Error: Failed to create CommandQueue: %d\n", status);
    return OFFLOAD_FAIL;
  }

  // Out-of-order queue will be created on demand.
  DeviceInfo->QueuesOOO[device_id] = nullptr;

  DeviceInfo->Extensions[device_id].getExtensionsInfoForDevice(device_id);

#if INTEL_CUSTOMIZATION
  // Find extension function pointers
  auto &ext = DeviceInfo->Extensions[device_id];
  auto platformID = DeviceInfo->platformIDs[device_id];
  if (ext.HostMemAllocINTELPointer == ExtensionStatusEnabled) {
    void *fn = nullptr;
    CALL_CL_RV(fn, clGetExtensionFunctionAddressForPlatform, platformID,
               "clHostMemAllocINTEL");
    DeviceInfo->clHostMemAllocINTELFn =
        reinterpret_cast<clHostMemAllocINTELTy>(fn);
    if (DeviceInfo->clHostMemAllocINTELFn)
      DP("Extension clHostMemAllocINTEL enabled.\n");
  }
  if (ext.SharedMemAllocINTELPointer == ExtensionStatusEnabled) {
    void *fn = nullptr;
    CALL_CL_RV(fn, clGetExtensionFunctionAddressForPlatform, platformID,
               "clSharedMemAllocINTEL");
    DeviceInfo->clSharedMemAllocINTELFn =
        reinterpret_cast<clSharedMemAllocINTELTy>(fn);
    if (DeviceInfo->clSharedMemAllocINTELFn)
      DP("Extension clSharedMemAllocINTEL enabled.\n");
  }
  if (ext.MemFreeINTELPointer == ExtensionStatusEnabled) {
    void *fn = nullptr;
    CALL_CL_RV(fn, clGetExtensionFunctionAddressForPlatform, platformID,
               "clMemFreeINTEL");
    DeviceInfo->clMemFreeINTELFn = reinterpret_cast<clMemFreeINTELTy>(fn);
    if (DeviceInfo->clMemFreeINTELFn)
      DP("Extension clMemFreeINTEL enabled.\n");
  }
#endif // INTEL_CUSTOMIZATION

  OMPT_CALLBACK(ompt_callback_device_initialize, device_id,
                DeviceInfo->Names[device_id].data(),
                DeviceInfo->deviceIDs[device_id],
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
#if INTEL_CUSTOMIZATION
  if (DebugLevel <= 0)
    return;

  size_t len = 0;
  CALL_CL_RET_VOID(clGetProgramBuildInfo, program, did, CL_PROGRAM_BUILD_LOG, 0,
                   nullptr, &len);
  if (len == 0)
    return;
  std::vector<char> buffer(len);
  CALL_CL_RET_VOID(clGetProgramBuildInfo, program, did, CL_PROGRAM_BUILD_LOG,
                   len, buffer.data(), nullptr);
  DP("%s\n", buffer.data());
#endif // INTEL_CUSTOMIZATION
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
      DeviceInfo->CTX[device_id], device_rtl_bin.c_str(), device_rtl_len);
    if (status != CL_SUCCESS) {
      DP("Error: Failed to create device RTL from IL: %d\n", status);
      return nullptr;
    }

    CALL_CL(status, clCompileProgram, program, 0, nullptr, options.c_str(), 0,
            nullptr, nullptr, nullptr, nullptr);
    if (status != CL_SUCCESS) {
      debugPrintBuildLog(program, DeviceInfo->deviceIDs[device_id]);
      DP("Error: Failed to compile program: %d\n", status);
      return nullptr;
    }

    return program;
  }

  DP("Cannot find device RTL: %s\n", device_rtl_path.c_str());
  return nullptr;
}

EXTERN
__tgt_target_table *__tgt_rtl_load_binary(int32_t device_id,
                                          __tgt_device_image *image) {

  DP("Device %d: load binary from " DPxMOD " image\n", device_id,
     DPxPTR(image->ImageStart));

  size_t ImageSize = (size_t)image->ImageEnd - (size_t)image->ImageStart;
  size_t NumEntries = (size_t)(image->EntriesEnd - image->EntriesBegin);
  DP("Expecting to have %zu entries defined.\n", NumEntries);

  ProfileIntervalTy CompilationTimer("Compiling", device_id);
  ProfileIntervalTy LinkingTimer("Linking", device_id);

  // create Program
  cl_int status;
  std::vector<cl_program> programs;
  cl_program linked_program;
  std::string compilation_options(DeviceInfo->CompilationOptions);
  std::string linking_options(DeviceInfo->LinkingOptions);

  DP("OpenCL compilation options: %s\n", compilation_options.c_str());
  DP("OpenCL linking options: %s\n", linking_options.c_str());
#if INTEL_CUSTOMIZATION
  compilation_options += " " + DeviceInfo->InternalCompilationOptions;
  linking_options += " " + DeviceInfo->InternalLinkingOptions;
  DPI("Final OpenCL compilation options: %s\n", compilation_options.c_str());
  DPI("Final OpenCL linking options: %s\n", linking_options.c_str());
#endif // INTEL_CUSTOMIZATION
  // clLinkProgram drops the last symbol. Work this around temporarily.
  linking_options += " ";

  // Create program for the target regions.
  // User program must be first in the link order.
  CompilationTimer.start();
  dumpImageToFile(image->ImageStart, ImageSize, "OpenMP");
  cl_program program;
  CALL_CL_RVRC(program, clCreateProgramWithIL, status,
               DeviceInfo->CTX[device_id], image->ImageStart, ImageSize);
  if (status != CL_SUCCESS) {
    debugPrintBuildLog(program, DeviceInfo->deviceIDs[device_id]);
    DP("Error: Failed to create program: %d\n", status);
    return NULL;
  }
  CALL_CL(status, clCompileProgram, program, 0, nullptr,
          compilation_options.c_str(), 0, nullptr, nullptr, nullptr, nullptr);
  if (status != CL_SUCCESS) {
    debugPrintBuildLog(program, DeviceInfo->deviceIDs[device_id]);
    DP("Error: Failed to compile program: %d\n", status);
    return NULL;
  }
  programs.push_back(program);

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
                                compilation_options);
      if (program)
        programs.push_back(program);
    } else
      DP("Skipped device RTL: %s\n", desc.FallbackLibName);
  }
  CompilationTimer.stop();

  LinkingTimer.start();

  CALL_CL_RVRC(linked_program, clLinkProgram, status,
      DeviceInfo->CTX[device_id], 1, &DeviceInfo->deviceIDs[device_id],
      linking_options.c_str(), programs.size(), programs.data(), nullptr,
      nullptr);
  if (status != CL_SUCCESS) {
    debugPrintBuildLog(linked_program, DeviceInfo->deviceIDs[device_id]);
    DP("Error: Failed to link program: %d\n", status);
    return NULL;
  } else {
    DP("Successfully linked program.\n");
  }
  DeviceInfo->FuncGblEntries[device_id].Program = linked_program;

  LinkingTimer.stop();

  // create kernel and target entries
  DeviceInfo->FuncGblEntries[device_id].Entries.resize(NumEntries);
  DeviceInfo->FuncGblEntries[device_id].Kernels.resize(NumEntries);
  std::vector<__tgt_offload_entry> &entries =
      DeviceInfo->FuncGblEntries[device_id].Entries;
  std::vector<cl_kernel> &kernels =
      DeviceInfo->FuncGblEntries[device_id].Kernels;

#if INTEL_CUSTOMIZATION
  auto platformID = DeviceInfo->platformIDs[device_id];
  if (!DeviceInfo->clGetDeviceGlobalVariablePointerINTELFn &&
      DeviceInfo->Extensions[device_id].GetDeviceGlobalVariablePointer ==
      ExtensionStatusEnabled) {
    void *fn = nullptr;
    CALL_CL_RV(fn, clGetExtensionFunctionAddressForPlatform, platformID,
               "clGetDeviceGlobalVariablePointerINTEL");
    DeviceInfo->clGetDeviceGlobalVariablePointerINTELFn =
        reinterpret_cast<clGetDeviceGlobalVariablePointerINTELTy>(fn);

    if (!DeviceInfo->clGetDeviceGlobalVariablePointerINTELFn) {
      DPI("Error: clGetDeviceGlobalVariablePointerINTEL API "
          "is nullptr.  Direct references to declare target variables "
          "will not work properly.\n");
    }
  }

  if (!DeviceInfo->clGetMemAllocInfoINTELFn &&
      DeviceInfo->Extensions[device_id].GetMemAllocInfoINTELPointer ==
      ExtensionStatusEnabled && DeviceInfo->DeviceType == CL_DEVICE_TYPE_CPU) {
    // TODO: limit this to CPU devices for the time being.
    void *fn = nullptr;
    CALL_CL_RV(fn, clGetExtensionFunctionAddressForPlatform, platformID,
               "clGetMemAllocInfoINTEL");
    DeviceInfo->clGetMemAllocInfoINTELFn =
        reinterpret_cast<clGetMemAllocInfoINTELTy>(fn);

    if (!DeviceInfo->clGetMemAllocInfoINTELFn) {
      DPI("Error: clGetMemAllocInfoINTEL API is nullptr.  Direct references "
          "to declare target variables will not work properly.\n");
    }
  }

  if (!DeviceInfo->clGetKernelSuggestedLocalWorkSizeINTELFn &&
      DeviceInfo->Extensions[device_id].SuggestedGroupSize ==
      ExtensionStatusEnabled) {
    void *fn = nullptr;
    CALL_CL_RV(fn, clGetExtensionFunctionAddressForPlatform, platformID,
               "clGetKernelSuggestedLocalWorkSizeINTEL");
    DeviceInfo->clGetKernelSuggestedLocalWorkSizeINTELFn =
        reinterpret_cast<clGetKernelSuggestedLocalWorkSizeINTELTy>(fn);

    if (!DeviceInfo->clGetKernelSuggestedLocalWorkSizeINTELFn &&
        DeviceInfo->Flags.UseDriverGroupSizes)
      DPI("Warning: clGetKernelSuggestedLocalWorkSizeINTEL API is nullptr.\n");
  }
#endif  // INTEL_CUSTOMIZATION

  ProfileIntervalTy EntriesTimer("Offload entries init", device_id);
  EntriesTimer.start();
  if (!DeviceInfo->loadOffloadTable(device_id, NumEntries))
    DP("Error: offload table loading failed.\n");
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
        TgtAddr = __tgt_rtl_data_alloc(device_id, Size, HostAddr);
        __tgt_rtl_data_submit(device_id, TgtAddr, HostAddr, Size);
        DP("Error: global variable '%s' allocated. "
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
    if (status != 0) {
      DP("Error: Failed to create kernel %s, %d\n", name, status);
      return NULL;
    }
    entries[i].addr = &kernels[i];
    entries[i].name = name;

    // Retrieve kernel group size info.
    size_t kernel_simd_width = 1;
    CALL_CL_RET_NULL(clGetKernelWorkGroupInfo, kernels[i],
                     DeviceInfo->deviceIDs[device_id],
                     CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                     sizeof(size_t), &kernel_simd_width, nullptr);
    DeviceInfo->KernelProperties[device_id][kernels[i]].Width =
        kernel_simd_width;

    size_t kernel_wg_size = 1;
    CALL_CL_RET_NULL(clGetKernelWorkGroupInfo, kernels[i],
                     DeviceInfo->deviceIDs[device_id],
                     CL_KERNEL_WORK_GROUP_SIZE,
                     sizeof(size_t), &kernel_wg_size, nullptr);
    DeviceInfo->KernelProperties[device_id][kernels[i]].MaxThreadGroupSize =
        kernel_wg_size;

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
      DP("Kernel %d: Name = %s, NumArgs = %d\n", i, buf.data(), kernel_num_args);
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
        DP("  Arg %2d: %s %s\n", idx, type_name.c_str(), buf.data());
      }
    }
  }

  // Release intermediate programs and store the final program.
  for (uint32_t i = 0; i < programs.size(); i++) {
    CALL_CL_EXIT_FAIL(clReleaseProgram, programs[i]);
  }
  if (initProgram(device_id) != OFFLOAD_SUCCESS)
    return nullptr;
  __tgt_target_table &table = DeviceInfo->FuncGblEntries[device_id].Table;
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

    if (DeviceInfo->Flags.EnableProfile) {
      const char *event_name;
      switch (cmd) {
      case CL_COMMAND_NDRANGE_KERNEL:
        event_name = "EXEC-ASYNC";
        break;
      case CL_COMMAND_SVM_MEMCPY:
        event_name = "DATA-ASYNC";
        break;
      case CL_COMMAND_READ_BUFFER:
        event_name = "DATA-ASYNC-READ";
        break;
      case CL_COMMAND_WRITE_BUFFER:
        event_name = "DATA-ASYNC-WRITE";
        break;
      default:
        event_name = "OTHERS-ASYNC";
      }
      DeviceInfo->Profiles[async_data->DeviceId].update(event_name, event);
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
EXTERN
int32_t __tgt_rtl_manifest_data_for_region(
    int32_t device_id,
    void *tgt_entry_ptr,
    void **tgt_ptrs,
    size_t num_ptrs) {

  cl_kernel *kernel = static_cast<cl_kernel *>(tgt_entry_ptr);
  DP("Stashing %" PRIu64 " implicit arguments for kernel " DPxMOD "\n",
     static_cast<uint64_t>(num_ptrs), DPxPTR(kernel));
  DeviceInfo->Mutexes[device_id].lock();
  DeviceInfo->ImplicitArgs[device_id][*kernel] =
      std::set<void *>(tgt_ptrs, tgt_ptrs + num_ptrs);
  DeviceInfo->Mutexes[device_id].unlock();

  return OFFLOAD_SUCCESS;
}

EXTERN void *__tgt_rtl_create_offload_queue(int32_t device_id, bool is_async) {
  // Return a shared in-order queue for synchronous case if requested
  if (!is_async && DeviceInfo->Flags.UseInteropQueueInorderSharedSync) {
    DP("%s returns the shared in-order queue " DPxMOD "\n", __func__,
       DPxPTR(DeviceInfo->Queues[device_id]));
    return (void *)DeviceInfo->Queues[device_id];
  }

  cl_int status;
  cl_command_queue queue;
  auto deviceId = DeviceInfo->deviceIDs[device_id];
  auto context = DeviceInfo->CTX[device_id];

  // Return a shared out-of-order queue for asynchronous case by default
  if (is_async && !DeviceInfo->Flags.UseInteropQueueInorderAsync) {
    queue = DeviceInfo->QueuesOOO[device_id];
    if (!queue) {
      cl_queue_properties qprops[3] =
          {CL_QUEUE_PROPERTIES, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, 0};
      CALL_CL_RVRC(queue, clCreateCommandQueueWithProperties, status, context,
                   deviceId, qprops);
      if (status != CL_SUCCESS) {
        DP("Error: Failed to create interop command queue: %d\n", status);
        return nullptr;
      }
      DP("%s creates a shared out-of-order queue " DPxMOD "\n", __func__,
         DPxPTR(queue));
      DeviceInfo->QueuesOOO[device_id] = queue;
    }
    DP("%s returns a shared out-of-order queue " DPxMOD "\n", __func__,
       DPxPTR(queue));
    return (void *)queue;
  }

  // Return a new in-order queue for other cases
  CALL_CL_RVRC(queue, clCreateCommandQueueWithProperties, status, context,
               deviceId, nullptr);
  if (status != CL_SUCCESS) {
    DP("Error: Failed to create interop command queue\n");
    return nullptr;
  }
  DP("%s creates and returns a separate in-order queue " DPxMOD "\n", __func__,
     DPxPTR(queue));
  return (void *)queue;
}

// Release the command queue if it is a new in-order command queue.
EXTERN int32_t __tgt_rtl_release_offload_queue(int32_t device_id, void *queue) {
  cl_command_queue tqueue = (cl_command_queue)queue;
  if (tqueue != DeviceInfo->QueuesOOO[device_id] &&
      tqueue != DeviceInfo->Queues[device_id]) {
    CALL_CL_RET_FAIL(clReleaseCommandQueue, tqueue);
    DP("%s releases a separate in-order queue " DPxMOD "\n",__func__,
       DPxPTR(queue));
  }
  return OFFLOAD_SUCCESS;
}


EXTERN void *__tgt_rtl_get_platform_handle(int32_t device_id) {
  auto context = DeviceInfo->CTX[device_id];
  return (void *) context;
}

#if INTEL_CUSTOMIZATION
// Allocate a managed memory object.
EXTERN void *__tgt_rtl_data_alloc_managed(int32_t device_id, int64_t size) {
  int32_t kind = DeviceInfo->Flags.UseHostMemForUSM ? TARGET_ALLOC_HOST
                                                    : TARGET_ALLOC_SHARED;
  return __tgt_rtl_data_alloc_explicit(device_id, size, kind);
}

// Delete a managed memory object.
EXTERN int32_t __tgt_rtl_data_delete_managed(int32_t device_id, void *ptr) {
  if (!DeviceInfo->clMemFreeINTELFn) {
    DP("clMemFreeINTEL is not available\n");
    return OFFLOAD_FAIL;
  }
  auto &mutex = DeviceInfo->Mutexes[device_id];
  mutex.lock();
  CALL_CL_EXT_RET_FAIL(clMemFreeINTEL, DeviceInfo->clMemFreeINTELFn,
                       DeviceInfo->CTX[device_id], ptr);
  DeviceInfo->ManagedData[device_id].erase(ptr);
  mutex.unlock();
  DP("Deleted a managed memory object " DPxMOD "\n", DPxPTR(ptr));
  return OFFLOAD_SUCCESS;
}
#endif // INTEL_CUSTOMIZATION

// Check if the pointer belongs to a managed memory addres range.
EXTERN int32_t __tgt_rtl_is_managed_ptr(int32_t device_id, void *ptr) {
  int32_t ret = false;
  auto &mutex = DeviceInfo->Mutexes[device_id];
  mutex.lock();
  for (auto &range : DeviceInfo->ManagedData[device_id]) {
    intptr_t base = (intptr_t)range.first;
    if (base <= (intptr_t)ptr && (intptr_t)ptr < base + range.second) {
      ret = true;
      break;
    }
  }
  mutex.unlock();
  DP("Ptr " DPxMOD " is %sa managed memory pointer.\n", DPxPTR(ptr),
     ret ? "" : "not ");
  return ret;
}

static inline
void *tgt_rtl_data_alloc_template(int32_t device_id, int64_t size,
                                  void *hst_ptr, void *hst_base,
                                  int32_t is_implicit_arg) {
  intptr_t offset = (intptr_t)hst_ptr - (intptr_t)hst_base;
  // If the offset is negative, then for our practical purposes it can be
  // considered 0 because the base address of an array will be contained
  // within or after the allocated memory.
  intptr_t meaningful_offset = offset >= 0 ? offset : 0;
  // If the offset is negative and the size we map is not large enough to reach
  // the base, then we must allocate extra memory up to the base (+1 to include
  // at least the first byte the base is pointing to).
  int64_t meaningful_size =
      offset < 0 && abs(offset) >= size ? abs(offset) + 1 : size;

  void *base = nullptr;
  CALL_CL_RV(base, clSVMAlloc, DeviceInfo->CTX[device_id], CL_MEM_READ_WRITE,
             meaningful_size + meaningful_offset, 0);
  if (!base) {
    DP("Error: Failed to allocate base buffer\n");
    return nullptr;
  }
  DP("Created base buffer " DPxMOD " during data alloc\n", DPxPTR(base));

  void *ret = (void *)((intptr_t)base + meaningful_offset);

  // Store allocation information
  DeviceInfo->Buffers[device_id][ret] = {base, meaningful_size};

  // Store list of pointers to be passed to kernel implicitly
  if (is_implicit_arg) {
    DP("Stashing an implicit argument " DPxMOD " for next kernel\n",
       DPxPTR(ret));
    DeviceInfo->Mutexes[device_id].lock();
    // key "0" for kernel-independent implicit arguments
    DeviceInfo->ImplicitArgs[device_id][0].insert(ret);
    DeviceInfo->Mutexes[device_id].unlock();
  }

  return ret;
}

#if INTEL_CUSTOMIZATION
EXTERN void *__tgt_rtl_data_alloc_explicit(
    int32_t device_id, int64_t size, int32_t kind) {
  auto device = DeviceInfo->deviceIDs[device_id];
  auto context = DeviceInfo->CTX[device_id];
  cl_int rc;
  void *mem = nullptr;
  auto &mutex = DeviceInfo->Mutexes[device_id];

  switch (kind) {
  case TARGET_ALLOC_DEVICE:
    mem = tgt_rtl_data_alloc_template(device_id, size, nullptr, nullptr, 0);
    break;
  case TARGET_ALLOC_HOST:
    if (!DeviceInfo->clHostMemAllocINTELFn) {
      DP("Host memory allocator is not available\n");
      return nullptr;
    }
    CALL_CL_EXT_RVRC(mem, clHostMemAllocINTEL,
                     DeviceInfo->clHostMemAllocINTELFn, rc, context, nullptr,
                     size, 0);
    if (mem) {
      std::unique_lock<std::mutex> dataLock(mutex);
      DeviceInfo->ManagedData[device_id].emplace(std::make_pair(mem, size));
      DP("Allocated a host memory object " DPxMOD "\n", DPxPTR(mem));
    }
    break;
  case TARGET_ALLOC_SHARED:
    if (!DeviceInfo->clSharedMemAllocINTELFn) {
      DP("Shared memory allocator is not available\n");
      return nullptr;
    }
    CALL_CL_EXT_RVRC(mem, clSharedMemAllocINTEL,
                     DeviceInfo->clSharedMemAllocINTELFn, rc, context, device,
                     nullptr, size, 0);
    if (mem) {
      std::unique_lock<std::mutex> dataLock(mutex);
      DeviceInfo->ManagedData[device_id].emplace(std::make_pair(mem, size));
      DP("Allocated a shared memory object " DPxMOD "\n", DPxPTR(mem));
    }
    break;
  default:
    FATAL_ERROR("Invalid target data allocation kind");
  }

  return mem;
}
#endif // INTEL_CUSTOMIZATION

EXTERN
void *__tgt_rtl_data_alloc(int32_t device_id, int64_t size, void *hst_ptr) {
  return tgt_rtl_data_alloc_template(device_id, size, hst_ptr, hst_ptr, 0);
}

// Allocate a base buffer with the given information.
EXTERN
void *__tgt_rtl_data_alloc_base(int32_t device_id, int64_t size, void *hst_ptr,
                                void *hst_base) {
  return tgt_rtl_data_alloc_template(device_id, size, hst_ptr, hst_base, 0);
}

// Create a buffer from the given SVM pointer.
EXTERN
void *__tgt_rtl_create_buffer(int32_t device_id, void *tgt_ptr) {
  if (DeviceInfo->Buffers[device_id].count(tgt_ptr) == 0) {
    DP("Error: Cannot create buffer from unknown device pointer " DPxMOD "\n",
       DPxPTR(tgt_ptr));
    return nullptr;
  }
  cl_int rc;
  int64_t size = DeviceInfo->Buffers[device_id][tgt_ptr].Size;
  cl_mem ret = nullptr;
  CALL_CL_RVRC(ret, clCreateBuffer, rc, DeviceInfo->CTX[device_id],
               CL_MEM_USE_HOST_PTR, size, tgt_ptr);
  if (rc != CL_SUCCESS) {
    DP("Error: Failed to create a buffer from a SVM pointer " DPxMOD "\n",
       DPxPTR(tgt_ptr));
    return nullptr;
  }
  DP("Created a buffer " DPxMOD " from a SVM pointer " DPxMOD "\n",
     DPxPTR(ret), DPxPTR(tgt_ptr));
  return ret;
}

// Release the buffer
EXTERN
int32_t __tgt_rtl_release_buffer(void *tgt_buffer) {
  CALL_CL_RET_FAIL(clReleaseMemObject, (cl_mem)tgt_buffer);
  return OFFLOAD_SUCCESS;
}

// Allocation was initiated by user (omp_target_alloc)
EXTERN
void *__tgt_rtl_data_alloc_user(int32_t device_id, int64_t size,
                                void *hst_ptr) {
  return tgt_rtl_data_alloc_template(device_id, size, hst_ptr, hst_ptr, 1);
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

  AsyncDataTy *async_data = nullptr;
  if (async_event && ((AsyncEventTy *)async_event)->handler) {
    async_data = new AsyncDataTy((AsyncEventTy *)async_event, device_id);
  }

  switch (DeviceInfo->DataTransferMethod) {
  case DATA_TRANSFER_METHOD_SVMMAP: {
    // No asynchronous data copy here since we use map/unmap as explicit
    // synchronization points.
    ProfileIntervalTy SubmitTime("DATA-WRITE", device_id);
    SubmitTime.start();

    CALL_CL_RET_FAIL(clEnqueueSVMMap, queue, CL_TRUE, CL_MAP_WRITE, tgt_ptr,
                     size, 0, nullptr, nullptr);
    memcpy(tgt_ptr, hst_ptr, size);
    CALL_CL_RET_FAIL(clEnqueueSVMUnmap, queue, tgt_ptr, 0, nullptr, nullptr);

    SubmitTime.stop();
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
      if (DeviceInfo->Flags.EnableProfile)
        DeviceInfo->Profiles[device_id].update("DATA-WRITE", event);
    }
  } break;
  case DATA_TRANSFER_METHOD_CLMEM:
  default: {
    cl_event event;
    cl_int rc;
    cl_mem mem = nullptr;
    CALL_CL_RVRC(mem, clCreateBuffer, rc, DeviceInfo->CTX[device_id],
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
      CALL_CL_RET_FAIL(clEnqueueWriteBuffer, queue, mem, CL_TRUE, 0, size,
                       hst_ptr, 0, nullptr, &event);
      CALL_CL_RET_FAIL(clReleaseMemObject, mem);
      if (DeviceInfo->Flags.EnableProfile)
        DeviceInfo->Profiles[device_id].update("DATA-WRITE", event);
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

  AsyncDataTy *async_data = nullptr;
  if (async_event && ((AsyncEventTy *)async_event)->handler) {
    async_data = new AsyncDataTy((AsyncEventTy *)async_event, device_id);
  }

  switch (DeviceInfo->DataTransferMethod) {
  case DATA_TRANSFER_METHOD_SVMMAP: {
    // No asynchronous data copy here since we use map/unmap as explicit
    // synchronization points.
    ProfileIntervalTy RetrieveTime("DATA-READ", device_id);
    RetrieveTime.start();

    CALL_CL_RET_FAIL(clEnqueueSVMMap, queue, CL_TRUE, CL_MAP_READ, tgt_ptr,
                     size, 0, nullptr, nullptr);
    memcpy(hst_ptr, tgt_ptr, size);
    CALL_CL_RET_FAIL(clEnqueueSVMUnmap, queue, tgt_ptr, 0, nullptr, nullptr);

    RetrieveTime.stop();
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
      if (DeviceInfo->Flags.EnableProfile)
        DeviceInfo->Profiles[device_id].update("DATA-READ", event);
    }
  } break;
  case DATA_TRANSFER_METHOD_CLMEM:
  default: {
    cl_int rc;
    cl_event event;
    cl_mem mem = nullptr;
    CALL_CL_RVRC(mem, clCreateBuffer, rc, DeviceInfo->CTX[device_id],
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
      CALL_CL_RET_FAIL(clEnqueueReadBuffer, queue, mem, CL_TRUE, 0, size,
                       hst_ptr, 0, nullptr, &event);
      CALL_CL_RET_FAIL(clReleaseMemObject, mem);
      if (DeviceInfo->Flags.EnableProfile)
        DeviceInfo->Profiles[device_id].update("DATA-READ", event);
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

EXTERN
int32_t __tgt_rtl_data_delete(int32_t device_id, void *tgt_ptr) {
  if (DeviceInfo->Buffers[device_id].count(tgt_ptr) == 0) {
    DP("Cannot find allocation information for " DPxMOD "\n", DPxPTR(tgt_ptr));
    return OFFLOAD_FAIL;
  }
  void *base = DeviceInfo->Buffers[device_id][tgt_ptr].Base;
  DeviceInfo->Buffers[device_id].erase(tgt_ptr);

  DeviceInfo->Mutexes[device_id].lock();
  // Erase from the internal list
  for (auto &J : DeviceInfo->ImplicitArgs[device_id])
    J.second.erase(tgt_ptr);
  DeviceInfo->Mutexes[device_id].unlock();

  CALL_CL_VOID(clSVMFree, DeviceInfo->CTX[device_id], base);
  return OFFLOAD_SUCCESS;
}

static void decideLoopKernelGroupArguments(
    int32_t DeviceId, int32_t ThreadLimit, TgtNDRangeDescTy *LoopLevels,
    cl_kernel Kernel, size_t *GroupSizes, size_t *GroupCounts) {

  size_t maxGroupSize = DeviceInfo->maxWorkGroupSize[DeviceId];
  size_t kernelWidth = DeviceInfo->KernelProperties[DeviceId][Kernel].Width;
  DP("Assumed kernel SIMD width is %zu\n", kernelWidth);

  size_t kernelMaxThreadGroupSize =
      DeviceInfo->KernelProperties[DeviceId][Kernel].MaxThreadGroupSize;
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
      DP("Max group size is set to %zu (thread_limit clause)\n",
         maxGroupSize);
    } else {
      DP("thread_limit(%" PRIu32 ") exceeds current maximum %zu\n",
         ThreadLimit, maxGroupSize);
    }
  }

  if (DeviceInfo->ThreadLimit > 0) {
    maxGroupSizeForced = true;

    if (DeviceInfo->ThreadLimit <= maxGroupSize) {
      maxGroupSize = DeviceInfo->ThreadLimit;
      DP("Max group size is set to %zu (OMP_THREAD_LIMIT)\n",
         maxGroupSize);
    } else {
      DP("OMP_THREAD_LIMIT(%" PRIu32 ") exceeds current maximum %zu\n",
         DeviceInfo->ThreadLimit, maxGroupSize);
    }
  }

  if (DeviceInfo->NumTeams > 0)
    DP("OMP_NUM_TEAMS(%" PRIu32 ") is ignored\n", DeviceInfo->NumTeams);

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
    assert((level[i].Ub >= level[i].Lb && level[i].Stride > 0) &&
           "Invalid loop nest description for ND partitioning");
    DP("Level %" PRIu32 ": Lb = %" PRId64 ", Ub = %" PRId64 ", Stride = %"
       PRId64 "\n", i, level[i].Lb, level[i].Ub, level[i].Stride);
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
    if (DeviceInfo->Flags.UseDriverGroupSizes &&
        DeviceInfo->clGetKernelSuggestedLocalWorkSizeINTELFn) {
      CALL_CL_EXT(rc, clGetKernelSuggestedLocalWorkSizeINTEL,
                  DeviceInfo->clGetKernelSuggestedLocalWorkSizeINTELFn,
                  DeviceInfo->Queues[DeviceId], Kernel,
                  3, nullptr, globalSizes, suggestedGroupSizes);
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

  size_t maxGroupSize = DeviceInfo->maxWorkGroupSize[DeviceId];
  bool maxGroupSizeForced = false;
  bool maxGroupCountForced = false;

  size_t kernelWidth = DeviceInfo->KernelProperties[DeviceId][Kernel].Width;
  DP("Assumed kernel SIMD width is %zu\n", kernelWidth);

  size_t kernelMaxThreadGroupSize =
      DeviceInfo->KernelProperties[DeviceId][Kernel].MaxThreadGroupSize;
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

  if (DeviceInfo->ThreadLimit > 0) {
    maxGroupSizeForced = true;

    if (DeviceInfo->ThreadLimit <= maxGroupSize) {
      maxGroupSize = DeviceInfo->ThreadLimit;
      DP("Max group size is set to %zu (OMP_THREAD_LIMIT)\n",
         maxGroupSize);
    } else {
      DP("OMP_THREAD_LIMIT(%" PRIu32 ") exceeds current maximum %zu\n",
         DeviceInfo->ThreadLimit, maxGroupSize);
    }
  }

  size_t maxGroupCount = 0;

  if (NumTeams > 0) {
    maxGroupCount = NumTeams;
    maxGroupCountForced = true;
    DP("Max group count is set to %zu "
       "(num_teams clause or no teams construct)\n", maxGroupCount);
  } else if (DeviceInfo->NumTeams > 0) {
    // OMP_NUM_TEAMS only matters, if num_teams() clause is absent.
    maxGroupCount = DeviceInfo->NumTeams;
    maxGroupCountForced = true;
    DP("Max group count is set to %zu (OMP_NUM_TEAMS)\n",
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
    maxGroupCount = DeviceInfo->maxExecutionUnits[DeviceId];
#if INTEL_CUSTOMIZATION
    if (DeviceInfo->DeviceType == CL_DEVICE_TYPE_GPU) {
      // We are currently handling only GEN9/GEN9.5 here.
      // TODO: we need to find a way to compute the number of sub slices
      //       and number of EUs per sub slice for the particular device.
      size_t numSubslices = 9;
      size_t numEUsPerSubslice= 8;
      size_t numThreadsPerEU = 7;
      size_t numEUs = DeviceInfo->maxExecutionUnits[DeviceId];

      // Each EU has 7 threads. A work group is partitioned into EU threads,
      // and then scheduled onto a sub slice. A sub slice must have all the
      // resources available to start a work group, otherwise it will wait
      // for resources. This means that uneven partitioning may result
      // in waiting work groups, and also unused EUs.
      // See slides 25-27 here:
      //   https://software.intel.com/sites/default/files/      \
      //   Faster-Better-Pixels-on-the-Go-and-in-the-Cloud-     \
      //   with-OpenCL-on-Intel-Architecture.pdf
      if (numEUs >= 72) {
        // Default best Halo (GT4) configuration.
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

      size_t numThreadsPerSubslice = numEUsPerSubslice * numThreadsPerEU;
      maxGroupCount = numSubslices * numThreadsPerSubslice;
      if (maxGroupSizeForced) {
        // Set group size for the HW capacity
        size_t numThreadsPerGroup =
            (maxGroupSize + kernelWidth - 1) / kernelWidth;
        size_t numGroupsPerSubslice =
            (numThreadsPerSubslice + numThreadsPerGroup - 1) /
            numThreadsPerGroup;
        maxGroupCount = numGroupsPerSubslice * numSubslices;
      } else {
        assert(!maxGroupSizeForced && !maxGroupCountForced);
        assert((maxGroupSize <= kernelWidth ||
                maxGroupSize % kernelWidth == 0) && "Invalid maxGroupSize");
        // Maximize group size
        while (maxGroupSize > kernelWidth) {
          size_t numThreadsPerGroup = maxGroupSize / kernelWidth;
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

  GroupCounts[0] = maxGroupCount;
  GroupCounts[1] = GroupCounts[2] = 1;
  if (!maxGroupCountForced)
    GroupCounts[0] *= DeviceInfo->SubscriptionRate;
}

static inline int32_t run_target_team_nd_region(
    int32_t device_id, void *tgt_entry_ptr, void **tgt_args,
    ptrdiff_t *tgt_offsets, int32_t num_args, int32_t num_teams,
    int32_t thread_limit, void *loop_desc, void *async_event) {

  cl_kernel *kernel = static_cast<cl_kernel *>(tgt_entry_ptr);

#if INTEL_INTERNAL_BUILD
  // TODO: kernels using to much SLM may limit the number of
  //       work groups running simultaneously on a sub slice.
  //       We may take this into account for computing the work partitioning.
  size_t device_local_mem_size = (size_t)DeviceInfo->SLMSize[device_id];
  DP("Device local mem size: %zu\n", device_local_mem_size);
  cl_ulong local_mem_size_tmp = 0;
  CALL_CL_RET_FAIL(clGetKernelWorkGroupInfo, *kernel,
                   DeviceInfo->deviceIDs[device_id], CL_KERNEL_LOCAL_MEM_SIZE,
                   sizeof(local_mem_size_tmp), &local_mem_size_tmp, nullptr);
  size_t kernel_local_mem_size = (size_t)local_mem_size_tmp;
  DP("Kernel local mem size: %zu\n", kernel_local_mem_size);
#endif // INTEL_INTERNAL_BUILD

  // Decide group sizes and counts
  size_t local_work_size[3] = {1, 1, 1};
  size_t num_work_groups[3] = {1, 1, 1};
  if (loop_desc) {
    decideLoopKernelGroupArguments(device_id, thread_limit,
                                   (TgtNDRangeDescTy *)loop_desc, *kernel,
                                   local_work_size, num_work_groups);
  } else {
    decideKernelGroupArguments(device_id, num_teams, thread_limit,
                               *kernel, local_work_size, num_work_groups);
  }

  size_t global_work_size[3];
  for (int32_t i = 0; i < 3; ++i)
    global_work_size[i] = local_work_size[i] * num_work_groups[i];

#if INTEL_INTERNAL_BUILD
  // Use forced group sizes. This is only for internal experiments, and we
  // don't want to plug these numbers into the decision logic.
  auto userLWS = DeviceInfo->ForcedLocalSizes;
  auto userGWS = DeviceInfo->ForcedGlobalSizes;
  if (userLWS[0] > 0) {
    std::copy(userLWS, userLWS + 3, local_work_size);
    DP("Forced LWS = {%zu, %zu, %zu}\n", userLWS[0], userLWS[1], userLWS[2]);
  }
  if (userGWS[0] > 0) {
    std::copy(userGWS, userGWS + 3, global_work_size);
    DP("Forced GWS = {%zu, %zu, %zu}\n", userGWS[0], userGWS[1], userGWS[2]);
  }
#endif // INTEL_INTERNAL_BUILD

  DP("Global work size = (%zu, %zu, %zu)\n", global_work_size[0],
     global_work_size[1], global_work_size[2]);
  DP("Local work size = (%zu, %zu, %zu)\n", local_work_size[0],
     local_work_size[1], local_work_size[2]);

  // Protect thread-unsafe OpenCL API calls
  DeviceInfo->Mutexes[device_id].lock();

  // Set implicit kernel args
  std::vector<void *> implicit_args;
#if INTEL_CUSTOMIZATION
  // Device pointers to global variables returned by
  // clGetDeviceGlobalVariablePointerINTEL are USM pointers
  // and they have to be reported to the runtime in a special way.
  std::vector<void *> implicit_usm_args;
#endif  // INTEL_CUSTOMIZATION

  // Array sections of zero size may result in nullptr target pointer,
  // which will not be accepted by clSetKernelExecInfo, so we should
  // avoid manifesting them.

  // Reserve space in implicit_args to speed up the back_inserter.
  size_t num_implicit_args = 0;
  if (DeviceInfo->ImplicitArgs[device_id].count(*kernel) > 0) {
    num_implicit_args += DeviceInfo->ImplicitArgs[device_id][*kernel].size();
  }

  implicit_args.reserve(num_implicit_args);

  if (DeviceInfo->ImplicitArgs[device_id].count(*kernel) > 0) {
    // kernel-dependent arguments
    std::copy_if(DeviceInfo->ImplicitArgs[device_id][*kernel].begin(),
                 DeviceInfo->ImplicitArgs[device_id][*kernel].end(),
                 std::back_inserter(implicit_args),
                 [] (void *ptr) {
                   return ptr != nullptr;
                 });
  }
  if (DeviceInfo->ImplicitArgs[device_id].count(0) > 0) {
    // kernel-independent arguments
    // Note that these pointers may not be nullptr.
    implicit_args.insert(implicit_args.end(),
        DeviceInfo->ImplicitArgs[device_id][0].begin(),
        DeviceInfo->ImplicitArgs[device_id][0].end());
  }

  // set kernel args
  std::vector<void *> ptrs(num_args);
  for (int32_t i = 0; i < num_args; ++i) {
    ptrs[i] = (void *)((intptr_t)tgt_args[i] + tgt_offsets[i]);
    CALL_CL_RET_FAIL(clSetKernelArgSVMPointer, *kernel, i, ptrs[i]);
    DP("Kernel Arg %d set successfully\n", i);
  }

#if INTEL_CUSTOMIZATION
  if (DeviceInfo->clGetMemAllocInfoINTELFn) {
    // Reserve space for USM pointers.
    implicit_usm_args.reserve(num_implicit_args);
    // Move USM pointers into a separate list, since they need to be
    // reported to the runtime using a separate clSetKernelExecInfo call.
    implicit_args.erase(
        std::remove_if(implicit_args.begin(),
                       implicit_args.end(),
                       [&](void *ptr) {
                         cl_unified_shared_memory_type_intel type = 0;
                         CALL_CL_EXT_RET(false,
                             clGetMemAllocInfoINTEL,
                             DeviceInfo->clGetMemAllocInfoINTELFn,
                             DeviceInfo->CTX[device_id],
                             ptr, CL_MEM_ALLOC_TYPE_INTEL,
                             sizeof(cl_unified_shared_memory_type_intel),
                             &type, nullptr);
                         DPI("clGetMemAllocInfoINTEL API returned %d "
                             "for pointer " DPxMOD "\n", type, DPxPTR(ptr));
                         // USM pointers are classified as
                         // CL_MEM_TYPE_DEVICE_INTEL.
                         // SVM pointers (e.g. returned by clSVMAlloc)
                         // are classified as CL_MEM_TYPE_UNKNOWN_INTEL.
                         // We cannot allocate any other pointer type now.
                         if (type == CL_MEM_TYPE_DEVICE_INTEL) {
                           implicit_usm_args.push_back(ptr);
                           return true;
                         }
                         return false;
                       }),
        implicit_args.end());
  }
#endif  // INTEL_CUSTOMIZATION

  if (implicit_args.size() > 0) {
    DP("Calling clSetKernelExecInfo to pass %zu implicit SVM arguments "
       "to kernel " DPxMOD "\n", implicit_args.size(), DPxPTR(kernel));
    CALL_CL_RET_FAIL(clSetKernelExecInfo, *kernel, CL_KERNEL_EXEC_INFO_SVM_PTRS,
                     sizeof(void *) * implicit_args.size(),
                     implicit_args.data());
  }

#if INTEL_CUSTOMIZATION
  if (implicit_usm_args.size() > 0) {
    // Report non-argument USM pointers to the runtime.
    DP("Calling clSetKernelExecInfo to pass %zu implicit USM arguments "
       "to kernel " DPxMOD "\n", implicit_usm_args.size(), DPxPTR(kernel));
    CALL_CL_RET_FAIL(clSetKernelExecInfo, *kernel,
                     CL_KERNEL_EXEC_INFO_USM_PTRS_INTEL,
                     sizeof(void *) * implicit_usm_args.size(),
                     implicit_usm_args.data());
    // Mark the kernel as supporting indirect USM accesses, otherwise,
    // clEnqueueNDRangeKernel call below will fail.
    cl_bool KernelSupportsUSM = CL_TRUE;
    CALL_CL_RET_FAIL(clSetKernelExecInfo, *kernel,
                     CL_KERNEL_EXEC_INFO_INDIRECT_DEVICE_ACCESS_INTEL,
                     sizeof(cl_bool), &KernelSupportsUSM);
  }
#endif  // INTEL_CUSTOMIZATION

  if (OMPT_ENABLED) {
    // Push current work size
    size_t finalNumTeams =
        global_work_size[0] * global_work_size[1] * global_work_size[2];
    size_t finalThreadLimit =
        local_work_size[0] * local_work_size[1] * local_work_size[2];
    finalNumTeams /= finalThreadLimit;
    OmptGlobal->getTrace().pushWorkSize(finalNumTeams, finalThreadLimit);
  }

  cl_event event;
  CALL_CL_RET_FAIL(clEnqueueNDRangeKernel, DeviceInfo->Queues[device_id],
                   *kernel, 3, nullptr, global_work_size,
                   local_work_size, 0, nullptr, &event);

  DeviceInfo->Mutexes[device_id].unlock();

  DP("Started executing kernel.\n");

  if (async_event) {
    if (((AsyncEventTy *)async_event)->handler) {
      // Add event handler if necessary.
      CALL_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
          &event_callback_completed,
          new AsyncDataTy((AsyncEventTy *)async_event, device_id));
    } else {
      // Make sure all queued commands finish before the next one starts.
      CALL_CL_RET_FAIL(clEnqueueBarrierWithWaitList,
                       DeviceInfo->Queues[device_id], 0, nullptr, nullptr);
    }
  } else {
    if (DeviceInfo->Flags.EnableProfile) {
      std::vector<char> buf;
      size_t buf_size;
      CALL_CL_RET_FAIL(clWaitForEvents, 1, &event);
      CALL_CL_RET_FAIL(clGetKernelInfo, *kernel, CL_KERNEL_FUNCTION_NAME, 0,
                       nullptr, &buf_size);
      std::string kernel_name("EXEC-");
      if (buf_size > 0) {
        buf.resize(buf_size);
        CALL_CL_RET_FAIL(clGetKernelInfo, *kernel, CL_KERNEL_FUNCTION_NAME,
                         buf.size(), buf.data(), nullptr);
        kernel_name += buf.data();
      }
      DeviceInfo->Profiles[device_id].update(kernel_name.c_str(), event);
    }
    CALL_CL_RET_FAIL(clFinish, DeviceInfo->Queues[device_id]);
    DP("Successfully finished kernel execution.\n");
  }

  return OFFLOAD_SUCCESS;
}

EXTERN int32_t
__tgt_rtl_run_target_team_nd_region(int32_t device_id, void *tgt_entry_ptr,
                                    void **tgt_args, ptrdiff_t *tgt_offsets,
                                    int32_t num_args, int32_t num_teams,
                                    int32_t thread_limit, void *loop_desc) {
  return run_target_team_nd_region(device_id, tgt_entry_ptr, tgt_args,
                                   tgt_offsets, num_args, num_teams,
                                   thread_limit, loop_desc, nullptr);
}

EXTERN
int32_t __tgt_rtl_run_target_team_nd_region_nowait(
    int32_t device_id, void *tgt_entry_ptr, void **tgt_args,
    ptrdiff_t *tgt_offsets, int32_t num_args, int32_t num_teams,
    int32_t thread_limit, void *loop_desc, void *async_event) {
  return run_target_team_nd_region(device_id, tgt_entry_ptr, tgt_args,
                                   tgt_offsets, num_args, num_teams,
                                   thread_limit, loop_desc, async_event);
}

EXTERN
int32_t __tgt_rtl_run_target_team_region_nowait(
    int32_t device_id, void *tgt_entry_ptr, void **tgt_args,
    ptrdiff_t *tgt_offsets, int32_t arg_num, int32_t team_num,
    int32_t thread_limit, uint64_t loop_tripcount, void *async_event) {
  return run_target_team_nd_region(device_id, tgt_entry_ptr, tgt_args,
                                   tgt_offsets, arg_num, team_num, thread_limit,
                                   nullptr, async_event);
}

EXTERN
int32_t __tgt_rtl_run_target_region_nowait(int32_t device_id,
                                           void *tgt_entry_ptr, void **tgt_args,
                                           ptrdiff_t *tgt_offsets,
                                           int32_t arg_num, void *async_event) {
  return run_target_team_nd_region(device_id, tgt_entry_ptr, tgt_args,
                                   tgt_offsets, arg_num, 1, 0, nullptr,
                                   async_event);
}

EXTERN
int32_t __tgt_rtl_run_target_team_region(int32_t device_id, void *tgt_entry_ptr,
                                         void **tgt_args,
                                         ptrdiff_t *tgt_offsets,
                                         int32_t arg_num, int32_t team_num,
                                         int32_t thread_limit,
                                         uint64_t loop_tripcount /*not used*/) {
  return run_target_team_nd_region(device_id, tgt_entry_ptr, tgt_args,
                                   tgt_offsets, arg_num, team_num, thread_limit,
                                   nullptr, nullptr);
}

EXTERN
int32_t __tgt_rtl_run_target_team_region_async(
    int32_t device_id, void *tgt_entry_ptr, void **tgt_args,
    ptrdiff_t *tgt_offsets, int32_t arg_num, int32_t team_num,
    int32_t thread_limit, uint64_t loop_tripcount /*not used*/,
    __tgt_async_info *AsyncInfoPtr /*not used*/) {
  return run_target_team_nd_region(device_id, tgt_entry_ptr, tgt_args,
                                   tgt_offsets, arg_num, team_num, thread_limit,
                                   nullptr, nullptr);
}

EXTERN
int32_t __tgt_rtl_run_target_region(int32_t device_id, void *tgt_entry_ptr,
                                    void **tgt_args, ptrdiff_t *tgt_offsets,
                                    int32_t arg_num) {
  // use one team!
  return __tgt_rtl_run_target_team_region(device_id, tgt_entry_ptr, tgt_args,
                                          tgt_offsets, arg_num, 1, 0, 0);
}

EXTERN
int32_t
__tgt_rtl_run_target_region_async(int32_t device_id, void *tgt_entry_ptr,
                                  void **tgt_args, ptrdiff_t *tgt_offsets,
                                  int32_t arg_num,
                                  __tgt_async_info *AsyncInfoPtr /*not used*/) {
  // use one team!
  return __tgt_rtl_run_target_team_region(device_id, tgt_entry_ptr, tgt_args,
                                          tgt_offsets, arg_num, 1, 0, 0);
}

EXTERN char *__tgt_rtl_get_device_name(
    int32_t device_id, char *buffer, size_t buffer_max_size) {
  assert(buffer && "buffer cannot be nullptr.");
  assert(buffer_max_size > 0 && "buffer_max_size cannot be zero.");
  CALL_CL_RET_NULL(clGetDeviceInfo, DeviceInfo->deviceIDs[device_id],
                   CL_DEVICE_NAME, buffer_max_size, buffer, nullptr);
  return buffer;
}

EXTERN int32_t __tgt_rtl_synchronize(int32_t device_id,
                                     __tgt_async_info *async_info_ptr) {
  return OFFLOAD_SUCCESS;
}

#ifdef __cplusplus
}
#endif

#endif // INTEL_COLLAB

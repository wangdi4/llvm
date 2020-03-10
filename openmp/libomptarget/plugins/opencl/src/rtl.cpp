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

extern thread_local OmptTraceTy *omptTracePtr;
extern void omptInitPlugin();
extern const char *omptDocument;
extern ompt_interface_fn_t omptLookupEntries(const char *);

#if INTEL_CUSTOMIZATION
// FIXME: find a way to include cl_usm_ext.h to get these definitions
//        from there.
#define CL_MEM_ALLOC_TYPE_INTEL         0x419A

#define CL_MEM_TYPE_UNKNOWN_INTEL       0x4196
#define CL_MEM_TYPE_HOST_INTEL          0x4197
#define CL_MEM_TYPE_DEVICE_INTEL        0x4198
#define CL_MEM_TYPE_SHARED_INTEL        0x4199

#define CL_MEM_ALLOC_FLAGS_INTEL        0x4195
#define CL_MEM_ALLOC_DEFAULT_INTEL      0

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
typedef cl_int (CL_API_CALL *clMemFreeINTELTy)(
    cl_context context,
    const void *ptr);
#endif  // INTEL_CUSTOMIZATION
#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// FIXME: we should actually include omp.h instead of declaring
//        these ourselves.
#if _WIN32
int __cdecl omp_get_thread_limit(void);
double __cdecl omp_get_wtime(void);
#else   // !_WIN32
int omp_get_thread_limit(void) __attribute__((weak));
double omp_get_wtime(void) __attribute__((weak));
#endif  // !_WIN32

#ifdef __cplusplus
}
#endif  // __cplusplus

#ifndef TARGET_NAME
#define TARGET_NAME OPENCL
#endif

#define GETNAME2(name) #name
#define GETNAME(name) GETNAME2(name)
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#ifdef OMPTARGET_OPENCL_DEBUG
int DebugLevel = 0;
#define DP(...)                                                                \
  do {                                                                         \
    if (DebugLevel > 0) {                                                      \
      DEBUGP("Target " GETNAME(TARGET_NAME) " RTL", __VA_ARGS__);              \
    }                                                                          \
  } while (false)
#else
#define DP(...)                                                                \
  {}
#endif // OMPTARGET_OPENCL_DEBUG

#if INTEL_CUSTOMIZATION
// DPI() is for printing sensitive information in the debug output.
// It will only print anything in non-release builds.
#if INTEL_INTERNAL_BUILD
#define DPI(...) DP(__VA_ARGS__)
#else  // !INTEL_INTERNAL_BUILD
#define DPI(...)
#endif // !INTEL_INTERNAL_BUILD
#else  // INTEL_CUSTOMIZATION
#define DPI(...)
#endif // INTEL_CUSTOMIZATION

#define FOREACH_CL_ERROR_CODE(FN)                                              \
  FN(CL_SUCCESS)                                                               \
  FN(CL_DEVICE_NOT_FOUND)                                                      \
  FN(CL_DEVICE_NOT_AVAILABLE)                                                  \
  FN(CL_COMPILER_NOT_AVAILABLE)                                                \
  FN(CL_MEM_OBJECT_ALLOCATION_FAILURE)                                         \
  FN(CL_OUT_OF_RESOURCES)                                                      \
  FN(CL_OUT_OF_HOST_MEMORY)                                                    \
  FN(CL_PROFILING_INFO_NOT_AVAILABLE)                                          \
  FN(CL_MEM_COPY_OVERLAP)                                                      \
  FN(CL_IMAGE_FORMAT_MISMATCH)                                                 \
  FN(CL_IMAGE_FORMAT_NOT_SUPPORTED)                                            \
  FN(CL_BUILD_PROGRAM_FAILURE)                                                 \
  FN(CL_MAP_FAILURE)                                                           \
  FN(CL_MISALIGNED_SUB_BUFFER_OFFSET)                                          \
  FN(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)                             \
  FN(CL_COMPILE_PROGRAM_FAILURE)                                               \
  FN(CL_LINKER_NOT_AVAILABLE)                                                  \
  FN(CL_LINK_PROGRAM_FAILURE)                                                  \
  FN(CL_DEVICE_PARTITION_FAILED)                                               \
  FN(CL_KERNEL_ARG_INFO_NOT_AVAILABLE)                                         \
  FN(CL_INVALID_VALUE)                                                         \
  FN(CL_INVALID_DEVICE_TYPE)                                                   \
  FN(CL_INVALID_PLATFORM)                                                      \
  FN(CL_INVALID_DEVICE)                                                        \
  FN(CL_INVALID_CONTEXT)                                                       \
  FN(CL_INVALID_QUEUE_PROPERTIES)                                              \
  FN(CL_INVALID_COMMAND_QUEUE)                                                 \
  FN(CL_INVALID_HOST_PTR)                                                      \
  FN(CL_INVALID_MEM_OBJECT)                                                    \
  FN(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)                                       \
  FN(CL_INVALID_IMAGE_SIZE)                                                    \
  FN(CL_INVALID_SAMPLER)                                                       \
  FN(CL_INVALID_BINARY)                                                        \
  FN(CL_INVALID_BUILD_OPTIONS)                                                 \
  FN(CL_INVALID_PROGRAM)                                                       \
  FN(CL_INVALID_PROGRAM_EXECUTABLE)                                            \
  FN(CL_INVALID_KERNEL_NAME)                                                   \
  FN(CL_INVALID_KERNEL_DEFINITION)                                             \
  FN(CL_INVALID_KERNEL)                                                        \
  FN(CL_INVALID_ARG_INDEX)                                                     \
  FN(CL_INVALID_ARG_VALUE)                                                     \
  FN(CL_INVALID_ARG_SIZE)                                                      \
  FN(CL_INVALID_KERNEL_ARGS)                                                   \
  FN(CL_INVALID_WORK_DIMENSION)                                                \
  FN(CL_INVALID_WORK_GROUP_SIZE)                                               \
  FN(CL_INVALID_WORK_ITEM_SIZE)                                                \
  FN(CL_INVALID_GLOBAL_OFFSET)                                                 \
  FN(CL_INVALID_EVENT_WAIT_LIST)                                               \
  FN(CL_INVALID_EVENT)                                                         \
  FN(CL_INVALID_OPERATION)                                                     \
  FN(CL_INVALID_GL_OBJECT)                                                     \
  FN(CL_INVALID_BUFFER_SIZE)                                                   \
  FN(CL_INVALID_MIP_LEVEL)                                                     \
  FN(CL_INVALID_GLOBAL_WORK_SIZE)                                              \
  FN(CL_INVALID_PROPERTY)                                                      \
  FN(CL_INVALID_IMAGE_DESCRIPTOR)                                              \
  FN(CL_INVALID_COMPILER_OPTIONS)                                              \
  FN(CL_INVALID_LINKER_OPTIONS)                                                \
  FN(CL_INVALID_DEVICE_PARTITION_COUNT)                                        \
  FN(CL_INVALID_PIPE_SIZE)                                                     \
  FN(CL_INVALID_DEVICE_QUEUE)

#define TO_STR(s) case s: return #s;

#ifdef OMPTARGET_OPENCL_DEBUG
static const char *getCLErrorName(int error) {
  switch (error) {
    FOREACH_CL_ERROR_CODE(TO_STR)
  default:
    return "Unknown Error";
  }
}
#endif // OMPTARGET_OPENCL_DEBUG

#define FATAL_ERROR(msg)                                                       \
  do {                                                                         \
    fprintf(stderr, "Error: %s failed (%s) -- exiting...\n", __func__, msg);   \
    exit(EXIT_FAILURE);                                                        \
  } while (false)

#define WARNING(fmt, ...) fprintf(stderr, "Warning: " fmt "\n", __VA_ARGS__)

#define INVOKE_CL_RET(ret, fn, ...)                                            \
  do {                                                                         \
    cl_int rc = fn(__VA_ARGS__);                                               \
    if (rc != CL_SUCCESS) {                                                    \
      DP("Error: %s:%s failed with error code %d, %s\n", __func__, #fn, rc,    \
         getCLErrorName(rc));                                                  \
      return ret;                                                              \
    }                                                                          \
  } while (false)

#define INVOKE_CL_EXIT_FAIL(fn, ...)                                           \
  do {                                                                         \
    cl_int rc = fn(__VA_ARGS__);                                               \
    if (rc != CL_SUCCESS) {                                                    \
      DP("Error: %s:%s failed with error code %d, %s\n", __func__, #fn, rc,    \
         getCLErrorName(rc));                                                  \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (false)

#define INVOKE_CL_RET_FAIL(fn, ...) INVOKE_CL_RET(OFFLOAD_FAIL, fn, __VA_ARGS__)
#define INVOKE_CL_RET_NULL(fn, ...) INVOKE_CL_RET(NULL, fn, __VA_ARGS__)

#define OFFLOADSECTIONNAME "omp_offloading_entries"
#ifdef _WIN32
#define DEVICE_RTL_NAME "..\\lib\\libomptarget-opencl.spv"
#else
#define DEVICE_RTL_NAME "libomptarget-opencl.spv"
#endif

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
  int64_t lb;     // The lower bound of the i-th loop
  int64_t ub;     // The upper bound of the i-th loop
  int64_t stride; // The stride of the i-th loop
} TgtLoopDescTy;

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
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_QUEUED,
                            sizeof(cl_ulong), &host_begin, nullptr);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_COMPLETE,
                            sizeof(cl_ulong), &host_end, nullptr);
    cl_ulong device_begin = 0, device_end = 0;
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START,
                            sizeof(cl_ulong), &device_begin, nullptr);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,
                            sizeof(cl_ulong), &device_end, nullptr);
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
  ExtensionStatusTy MemFreeINTELPointer = ExtensionStatusUnknown;
#endif  // INTEL_CUSTOMIZATION

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
  int DeviceType = CL_DEVICE_TYPE_GPU;
};

/// Class containing all the device information.
class RTLDeviceInfoTy {

public:
  cl_uint numDevices;
  cl_platform_id platformID;
  // per device information
  std::vector<cl_device_id> deviceIDs;
  std::vector<int32_t> maxExecutionUnits;
  std::vector<size_t> maxWorkGroupSize;

  // A vector of descriptors of OpenCL extensions for each device.
  std::vector<ExtensionsTy> Extensions;
  std::vector<cl_context> CTX;
  std::vector<cl_command_queue> Queues;
  std::vector<cl_command_queue> QueuesOOO; // out-of-order queues
  std::vector<FuncOrGblEntryTy> FuncGblEntries;
  std::vector<std::map<void *, BufferInfoTy> > Buffers;
  std::vector<std::map<cl_kernel, std::set<void *> > > ImplicitArgs;
  std::vector<ProfileDataTy> Profiles;
  std::vector<std::vector<char>> Names;
  std::vector<bool> Initialized;
  std::vector<cl_ulong> SLMSize;
  std::mutex *Mutexes;

  // Requires flags
  int64_t RequiresFlags = OMP_REQ_UNDEFINED;

  uint64_t Flags;
  int32_t DataTransferLatency;
  int32_t DataTransferMethod;
  int64_t ProfileResolution;
  cl_device_type DeviceType;
  std::string CompilationOptions;
  std::string LinkingOptions;

#if INTEL_CUSTOMIZATION
  // A pointer to clGetMemAllocInfoINTEL extension API.
  // It can be used to distinguish SVM and USM pointers.
  // It is available on the whole platform, so it is not
  // device-specific within the same platform.
  clGetMemAllocInfoINTELTy clGetMemAllocInfoINTELFn = nullptr;
  clHostMemAllocINTELTy clHostMemAllocINTELFn = nullptr;
  clMemFreeINTELTy clMemFreeINTELFn = nullptr;
#endif  // INTEL_CUSTOMIZATION

  // Limit for the number of WIs in a WG.
  int32_t OMPThreadLimit = -1;
#if INTEL_CUSTOMIZATION
  // Default subscription rate is heuristically set to 4.
  // It only matters for the default ND-range parallelization,
  // i.e. when the global size is unknown on the host.
  // This is a factor applied to the number of WGs computed
  // for the execution, based on the HW characteristics.
  size_t SubscriptionRate = 4;
#endif  // INTEL_CUSTOMIZATION
  static const uint64_t LinkDeviceRTLFlag                    = 1ULL << 0;
  static const uint64_t CollectDataTransferLatencyFlag       = 1ULL << 1;
  static const uint64_t EnableProfileFlag                    = 1ULL << 2;
  static const uint64_t UseInteropQueueInorderAsyncFlag      = 1ULL << 3;
  static const uint64_t UseInteropQueueInorderSharedSyncFlag = 1ULL << 4;

  RTLDeviceInfoTy() : numDevices(0), Flags(0), DataTransferLatency(0),
      DataTransferMethod(DATA_TRANSFER_METHOD_CLMEM) {
#ifdef OMPTARGET_OPENCL_DEBUG
    if (char *envStr = getenv("LIBOMPTARGET_DEBUG")) {
      DebugLevel = std::stoi(envStr);
    }
#endif // OMPTARGET_OPENCL_DEBUG
    // set misc. flags

    // Get global OMP_THREAD_LIMIT for SPMD parallelization.
    OMPThreadLimit = omp_get_thread_limit();
    DP("omp_get_thread_limit() returned %d\n", OMPThreadLimit);

    const char *env;

#if INTEL_CUSTOMIZATION
    if (env = std::getenv("LIBOMPTARGET_OPENCL_SUBSCRIPTION_RATE")) {
      int32_t value = std::stoi(env);

      // Set some reasonable limits.
      if (value > 0 || value <= 0xFFFF)
        SubscriptionRate = value;
    }
#endif  // INTEL_CUSTOMIZATION

    // Read LIBOMPTARGET_DATA_TRANSFER_LATENCY (experimental input)
    if (env = std::getenv("LIBOMPTARGET_DATA_TRANSFER_LATENCY")) {
      std::string value(env);
      if (value.substr(0, 2) == "T,") {
        Flags |= CollectDataTransferLatencyFlag;
        int32_t usec = std::stoi(value.substr(2).c_str());
        DataTransferLatency = (usec > 0) ? usec : 0;
      }
    }
    // Read LIBOMPTARGET_DATA_TRANSFER_METHOD
    if (env = std::getenv("LIBOMPTARGET_DATA_TRANSFER_METHOD")) {
      std::string value(env);
      DataTransferMethod = DATA_TRANSFER_METHOD_INVALID;
      if (value.size() == 1 && std::isdigit(value.c_str()[0])) {
        int method = std::stoi(env);
        if (method < DATA_TRANSFER_METHOD_LAST)
          DataTransferMethod = method;
      }
      if (DataTransferMethod == DATA_TRANSFER_METHOD_INVALID) {
        DP("Warning: Invalid data transfer method (%s) selected"
           " -- using default method.\n", env);
        DataTransferMethod = DATA_TRANSFER_METHOD_CLMEM;
      }
    }
    // Read LIBOMPTARGET_DEVICETYPE
    DeviceType = CL_DEVICE_TYPE_GPU;
    if (env = std::getenv("LIBOMPTARGET_DEVICETYPE")) {
      std::string value(env);
      if (value == "GPU" || value == "gpu")
        DeviceType = CL_DEVICE_TYPE_GPU;
      else if (value == "ACCELERATOR" || value == "accelerator")
        DeviceType = CL_DEVICE_TYPE_ACCELERATOR;
      else if (value == "CPU" || value == "cpu")
        DeviceType = CL_DEVICE_TYPE_CPU;
      else
        DP("Warning: Invalid LIBOMPTARGET_DEVICETYPE=%s\n", env);
    }
    DP("Target device type is set to %s\n",
       (DeviceType == CL_DEVICE_TYPE_GPU) ? "GPU" : (
       (DeviceType == CL_DEVICE_TYPE_ACCELERATOR) ? "ACCELERATOR" : (
       (DeviceType == CL_DEVICE_TYPE_CPU) ? "CPU" : "INVALID")));

    if (env = std::getenv("LIBOMPTARGET_LINK_OPENCL_DEVICE_RTL"))
      if (std::stoi(env) != 0)
        Flags |= LinkDeviceRTLFlag;

    // Read LIBOMPTARGET_PROFILE
    ProfileResolution = 1000;
    if (env = std::getenv("LIBOMPTARGET_PROFILE")) {
      std::istringstream value(env);
      std::string token;
      while (std::getline(value, token, ',')) {
        if (token == "T" || token == "1")
          Flags |= EnableProfileFlag;
        else if (token == "unit_usec" || token == "usec")
          ProfileResolution = 1000000;
      }
    }

    // Read LIBOMPTARGET_INTEROP_PIPE
    // Two independent options can be specified as follows.
    // -- inorder_async: use a new in-order queue for asynchronous case
    //    (default: shared out-of-order queue)
    // -- inorder_shared_sync: use the existing shared in-order queue for
    //    synchronous case (default: new in-order queue).
    if (env = std::getenv("LIBOMPTARGET_INTEROP_PIPE")) {
      std::istringstream value(env);
      std::string token;
      DP("LIBOMPTARGET_INTEROP_PIPE=%s was set\n", env);
      while (std::getline(value, token, ',')) {
        if (token == "inorder_async") {
          Flags |= UseInteropQueueInorderAsyncFlag;
          DP("    enabled in-order asynchronous separate queue\n");
        } else if (token == "inorder_shared_sync") {
          Flags |= UseInteropQueueInorderSharedSyncFlag;
          DP("    enabled in-order synchronous shared queue\n");
        }
      }
    }

    if (env = std::getenv("LIBOMPTARGET_OPENCL_COMPILATION_OPTIONS")) {
      CompilationOptions += env;
    }
    if (env = std::getenv("LIBOMPTARGET_OPENCL_LINKING_OPTIONS")) {
      LinkingOptions += env;
    }
#if INTEL_CUSTOMIZATION
    // OpenCL CPU compiler complains about unsupported option.
    // Intel Graphics compilers that do not support that option
    // silently ignore it.
    if (DeviceType == CL_DEVICE_TYPE_GPU &&
        (env = std::getenv("LIBOMPTARGET_OPENCL_TARGET_GLOBALS")) &&
        (env[0] == 'T' || env[0] == 't' || env[0] == '1'))
        LinkingOptions += " -cl-take-global-address ";
#endif  // INTEL_CUSTOMIZATION
  }
};

static RTLDeviceInfoTy DeviceInfo;

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
      ClDeviceId(DeviceInfo.deviceIDs[DeviceId]) {
    if (DeviceInfo.Flags & RTLDeviceInfoTy::EnableProfileFlag)
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
              "is disabled due to start/stop mismatch.",
              Name.c_str(), DeviceId, DeviceInfo.Names[DeviceId].data());
      return;
    }

    DeviceInfo.Profiles[DeviceId].update(
        Name.c_str(), HostElapsed, DeviceElapsed);
  }

  // Trigger interval start.
  void start() {
    if (Status == TimerStatusTy::Disabled)
      return;
    if (Status == TimerStatusTy::Running) {
      Status = TimerStatusTy::Disabled;
      WARNING("profiling timer '%s' for OpenMP device (%" PRId32 ") %s "
              "is disabled due to start/stop mismatch.",
              Name.c_str(), DeviceId, DeviceInfo.Names[DeviceId].data());
      return;
    }
    if (clGetDeviceAndHostTimer(ClDeviceId, &DeviceTimeTemp, &HostTimeTemp) !=
            CL_SUCCESS) {
      Status = TimerStatusTy::Disabled;
      WARNING("profiling timer '%s' for OpenMP device (%" PRId32 ") %s "
              "is disabled due to invalid OpenCL timer.",
              Name.c_str(), DeviceId, DeviceInfo.Names[DeviceId].data());
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
              "is disabled due to start/stop mismatch.",
              Name.c_str(), DeviceId, DeviceInfo.Names[DeviceId].data());
      return;
    }

    cl_ulong DeviceTime, HostTime;
    if (clGetDeviceAndHostTimer(ClDeviceId, &DeviceTime, &HostTime) !=
            CL_SUCCESS) {
      Status = TimerStatusTy::Disabled;
      WARNING("profiling timer '%s' for OpenMP device (%" PRId32 ") %s "
              "is disabled due to invalid OpenCL timer.",
              Name.c_str(), DeviceId, DeviceInfo.Names[DeviceId].data());
      return;
    }

    if (DeviceTime < DeviceTimeTemp || HostTime < HostTimeTemp) {
      Status = TimerStatusTy::Disabled;
      WARNING("profiling timer '%s' for OpenMP device (%" PRId32 ") %s "
              "is disabled due to timer overflow.",
              Name.c_str(), DeviceId, DeviceInfo.Names[DeviceId].data());
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
  for (uint32_t i = 0; i < DeviceInfo.numDevices; i++) {
    if (!DeviceInfo.Initialized[i])
      continue;
    // Invoke OMPT callbacks
    if (omptEnabled.enabled) {
      OMPT_CALLBACK(ompt_callback_device_unload, i, 0 /* module ID */);
      OMPT_CALLBACK(ompt_callback_device_finalize, i);
    }
    if (DeviceInfo.Flags & RTLDeviceInfoTy::EnableProfileFlag)
      DeviceInfo.Profiles[i].printData(i, DeviceInfo.Names[i].data(),
                                       DeviceInfo.ProfileResolution);
#ifndef _WIN32
    // Making OpenCL calls during process exit on Windows is unsafe.
    for (auto kernel : DeviceInfo.FuncGblEntries[i].Kernels) {
      if (kernel)
        INVOKE_CL_EXIT_FAIL(clReleaseKernel, kernel);
    }
    INVOKE_CL_EXIT_FAIL(clReleaseProgram, DeviceInfo.FuncGblEntries[i].Program);
    INVOKE_CL_EXIT_FAIL(clReleaseCommandQueue, DeviceInfo.Queues[i]);
    if (DeviceInfo.QueuesOOO[i]) {
      INVOKE_CL_EXIT_FAIL(clReleaseCommandQueue, DeviceInfo.QueuesOOO[i]);
    }
    INVOKE_CL_EXIT_FAIL(clReleaseContext, DeviceInfo.CTX[i]);
#endif // !defined(_WIN32)
  }
  delete[] DeviceInfo.Mutexes;
  DP("Closed RTL successfully\n");
}

static std::string getDeviceRTLPath() {
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
  rtl_path.replace(split + 1, std::string::npos, DEVICE_RTL_NAME);
  return rtl_path;
}

/// Invoke kernel to initialize program data.
/// TODO: consider moving allocation of static buffers in device RTL to here
///       as it requires device information.
static int32_t initProgram(int32_t deviceId) {
  int32_t rc;
  ProgramData hostData = {
    1,                              // Initialized
    (int32_t)DeviceInfo.numDevices, // Number of devices
    deviceId,                       // Device ID
    (int32_t)DeviceInfo.DeviceType  // Device type
  };
  auto context = DeviceInfo.CTX[deviceId];
  auto queue = DeviceInfo.Queues[deviceId];
  auto program = DeviceInfo.FuncGblEntries[deviceId].Program;
  cl_mem devData = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(hostData),
                                  nullptr, &rc);
  if (rc != CL_SUCCESS) {
    DP("Failed to initialize program\n");
    return OFFLOAD_FAIL;
  }
  INVOKE_CL_RET_FAIL(clEnqueueWriteBuffer, queue, devData, true, 0,
                     sizeof(hostData), &hostData, 0, nullptr, nullptr);
  cl_kernel initPgm = clCreateKernel(program, "__kmpc_init_program", &rc);
  if (rc != CL_SUCCESS) {
    DP("Failed to initialize program\n");
    return OFFLOAD_FAIL;
  }
  size_t globalWork = 1;
  size_t localWork = 1;
  INVOKE_CL_RET_FAIL(clSetKernelArg, initPgm, 0, sizeof(devData), &devData);
  INVOKE_CL_RET_FAIL(clEnqueueNDRangeKernel, queue, initPgm, 1, nullptr,
                     &globalWork, &localWork, 0, nullptr, nullptr);
  INVOKE_CL_RET_FAIL(clFinish, queue);
  INVOKE_CL_RET_FAIL(clReleaseMemObject, devData);
  INVOKE_CL_RET_FAIL(clReleaseKernel, initPgm);
  return OFFLOAD_SUCCESS;
}

#ifdef __cplusplus
extern "C" {
#endif

static inline void addDataTransferLatency() {
  if ((DeviceInfo.Flags & RTLDeviceInfoTy::CollectDataTransferLatencyFlag) != 0)
    return;
  double goal = omp_get_wtime() + 1e-6 * DeviceInfo.DataTransferLatency;
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

  cl_device_id DeviceId = DeviceInfo.deviceIDs[DeviceNum];
  INVOKE_CL_RET_FAIL(clGetDeviceInfo, DeviceId, CL_DEVICE_EXTENSIONS,
                     0, nullptr, &RetSize);

  std::unique_ptr<char []> Data(new char[RetSize]);
  INVOKE_CL_RET_FAIL(clGetDeviceInfo, DeviceId, CL_DEVICE_EXTENSIONS,
                     RetSize, Data.get(), &RetSize);

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
    MemFreeINTELPointer = ExtensionStatusEnabled;
  }
#endif  // INTEL_CUSTOMIZATION

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
  clGetPlatformIDs(0, nullptr, &platformIdCount);
  std::vector<cl_platform_id> platformIds(platformIdCount);
  clGetPlatformIDs(platformIdCount, platformIds.data(), nullptr);

  // OpenCL device IDs are stored in a list so that
  // 1. All device IDs from a single platform are stored consecutively.
  // 2. Device IDs from a platform having at least one GPU device appear
  //    before any device IDs from a platform having no GPU devices.
  for (cl_platform_id id : platformIds) {
    std::vector<char> buf;
    size_t buf_size;
    cl_int rc;
    rc = clGetPlatformInfo(id, CL_PLATFORM_VERSION, 0, nullptr, &buf_size);
    if (rc != CL_SUCCESS || buf_size == 0)
      continue;
    buf.resize(buf_size);
    rc = clGetPlatformInfo(id, CL_PLATFORM_VERSION, buf_size, buf.data(),
                           nullptr);
    // clCreateProgramWithIL() requires OpenCL 2.1.
    if (rc != CL_SUCCESS || strncmp("OpenCL 2.1", buf.data(), 8)) {
      continue;
    }
    cl_uint numDevices = 0;
    clGetDeviceIDs(id, DeviceInfo.DeviceType, 0, nullptr, &numDevices);
    if (numDevices == 0)
      continue;

    DP("Platform %s has %" PRIu32 " Devices\n", buf.data(), numDevices);
    DeviceInfo.deviceIDs.resize(numDevices);
    clGetDeviceIDs(id, DeviceInfo.DeviceType, numDevices,
                   DeviceInfo.deviceIDs.data(), nullptr);
    DeviceInfo.numDevices = numDevices;
    DeviceInfo.platformID = id;
    break;
    // It is unrealistic to have multiple platforms that support the same
    // device type, so breaking here should be fine.
  }

  DeviceInfo.maxExecutionUnits.resize(DeviceInfo.numDevices);
  DeviceInfo.maxWorkGroupSize.resize(DeviceInfo.numDevices);
  DeviceInfo.Extensions.resize(DeviceInfo.numDevices);
  DeviceInfo.CTX.resize(DeviceInfo.numDevices);
  DeviceInfo.Queues.resize(DeviceInfo.numDevices);
  DeviceInfo.QueuesOOO.resize(DeviceInfo.numDevices);
  DeviceInfo.FuncGblEntries.resize(DeviceInfo.numDevices);
  DeviceInfo.Buffers.resize(DeviceInfo.numDevices);
  DeviceInfo.ImplicitArgs.resize(DeviceInfo.numDevices);
  DeviceInfo.Profiles.resize(DeviceInfo.numDevices);
  DeviceInfo.Names.resize(DeviceInfo.numDevices);
  DeviceInfo.Initialized.resize(DeviceInfo.numDevices);
  DeviceInfo.SLMSize.resize(DeviceInfo.numDevices);
  DeviceInfo.Mutexes = new std::mutex[DeviceInfo.numDevices];

  // get device specific information
  for (unsigned i = 0; i < DeviceInfo.numDevices; i++) {
    size_t buf_size;
    cl_int rc;
    cl_device_id deviceId = DeviceInfo.deviceIDs[i];
    rc = clGetDeviceInfo(deviceId, CL_DEVICE_NAME, 0, nullptr, &buf_size);
    if (rc != CL_SUCCESS || buf_size == 0)
      continue;
    DeviceInfo.Names[i].resize(buf_size);
    rc = clGetDeviceInfo(deviceId, CL_DEVICE_NAME, buf_size,
                         DeviceInfo.Names[i].data(), nullptr);
    if (rc != CL_SUCCESS)
      continue;
    DP("Device %d: %s\n", i, DeviceInfo.Names[i].data());
    clGetDeviceInfo(deviceId, CL_DEVICE_MAX_COMPUTE_UNITS, 4,
                    &DeviceInfo.maxExecutionUnits[i], nullptr);
    DP("Number of execution units on the device is %d\n",
       DeviceInfo.maxExecutionUnits[i]);
    clGetDeviceInfo(deviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t),
                    &DeviceInfo.maxWorkGroupSize[i], nullptr);
    DP("Maximum work group size for the device is %d\n",
       static_cast<int32_t>(DeviceInfo.maxWorkGroupSize[i]));
#ifdef OMPTARGET_OPENCL_DEBUG
    cl_uint addressmode;
    clGetDeviceInfo(deviceId, CL_DEVICE_ADDRESS_BITS, 4, &addressmode,
                    nullptr);
    DP("Addressing mode is %d bit\n", addressmode);
#endif
    clGetDeviceInfo(deviceId, CL_DEVICE_LOCAL_MEM_SIZE,
                    sizeof(cl_ulong), &DeviceInfo.SLMSize[i], nullptr);
    DP("Device local mem size: %zu\n", (size_t)DeviceInfo.SLMSize[i]);
    DeviceInfo.Initialized[i] = false;
  }
  if (DeviceInfo.numDevices > 0) {
    omptInitPlugin();
  } else {
    DP("WARNING: No OpenCL devices found.\n");
  }

  return DeviceInfo.numDevices;
}

EXTERN
int32_t __tgt_rtl_init_device(int32_t device_id) {
  cl_int status;
  DP("Initialize OpenCL device\n");
  assert(device_id >= 0 && (cl_uint)device_id < DeviceInfo.numDevices &&
         "bad device id");

  // create context
  auto PlatformID = DeviceInfo.platformID;
  cl_context_properties props[] = {
      CL_CONTEXT_PLATFORM,
      (cl_context_properties)PlatformID, 0};
  DeviceInfo.CTX[device_id] = clCreateContext(
      props, 1, &DeviceInfo.deviceIDs[device_id], nullptr, nullptr, &status);
  if (status != CL_SUCCESS) {
    DP("Error: Failed to create context: %d\n", status);
    return OFFLOAD_FAIL;
  }

  cl_queue_properties qprops[3] = {CL_QUEUE_PROPERTIES, 0, 0};
  if (DeviceInfo.Flags & RTLDeviceInfoTy::EnableProfileFlag)
    qprops[1] = CL_QUEUE_PROFILING_ENABLE;

  auto deviceID = DeviceInfo.deviceIDs[device_id];
  auto context = DeviceInfo.CTX[device_id];
  DeviceInfo.Queues[device_id] =
      clCreateCommandQueueWithProperties(context, deviceID, qprops, &status);
  if (status != 0) {
    DP("Error: Failed to create CommandQueue: %d\n", status);
    return OFFLOAD_FAIL;
  }

  // Out-of-order queue will be created on demand.
  DeviceInfo.QueuesOOO[device_id] = nullptr;

  DeviceInfo.Extensions[device_id].getExtensionsInfoForDevice(device_id);

#if INTEL_CUSTOMIZATION
  // Find extension function pointers
  auto &ext = DeviceInfo.Extensions[device_id];
  if (ext.HostMemAllocINTELPointer == ExtensionStatusEnabled) {
    DeviceInfo.clHostMemAllocINTELFn =
        reinterpret_cast<clHostMemAllocINTELTy>(
        clGetExtensionFunctionAddressForPlatform(DeviceInfo.platformID,
        "clHostMemAllocINTEL"));
    if (DeviceInfo.clHostMemAllocINTELFn)
      DP("Extension clHostMemAllocINTEL enabled.\n");
  }
  if (ext.MemFreeINTELPointer == ExtensionStatusEnabled) {
    DeviceInfo.clMemFreeINTELFn =
        reinterpret_cast<clMemFreeINTELTy>(
        clGetExtensionFunctionAddressForPlatform(DeviceInfo.platformID,
        "clMemFreeINTEL"));
    if (DeviceInfo.clMemFreeINTELFn)
      DP("Extension clMemFreeINTEL enabled.\n");
  }
#endif // INTEL_CUSTOMIZATION

  OMPT_CALLBACK(ompt_callback_device_initialize, device_id,
                DeviceInfo.Names[device_id].data(),
                DeviceInfo.deviceIDs[device_id],
                omptLookupEntries, omptDocument);

  // Make sure it is registered after OCL handlers are registered.
  if (std::atexit(closeRTL)) {
    FATAL_ERROR("Registration of clean-up function");
  }
  DeviceInfo.Initialized[device_id] = true;

  return OFFLOAD_SUCCESS;
}

EXTERN int64_t __tgt_rtl_init_requires(int64_t RequiresFlags) {
  DP("Initialize requires flags to %" PRId64 "\n", RequiresFlags);
  DeviceInfo.RequiresFlags = RequiresFlags;
  return RequiresFlags;
}

static void dumpImageToFile(
    const void *Image, size_t ImageSize, const char *Type) {
#if INTEL_CUSTOMIZATION
#if INTEL_INTERNAL_BUILD
#ifdef OMPTARGET_OPENCL_DEBUG
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
#endif  // OMPTARGET_OPENCL_DEBUG
#endif  // INTEL_INTERNAL_BUILD
#endif  // INTEL_CUSTOMIZATION
}

static void debugPrintBuildLog(cl_program program, cl_device_id did) {
#if INTEL_CUSTOMIZATION
#if OMPTARGET_OPENCL_DEBUG
  if (DebugLevel <= 0)
    return;

  size_t len = 0;
  int ret =
      clGetProgramBuildInfo(program, did, CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
  if (ret != CL_SUCCESS || len == 0)
    return;
  std::vector<char> buffer(len);
  ret = clGetProgramBuildInfo(program, did, CL_PROGRAM_BUILD_LOG, len,
                              buffer.data(), NULL);
  if (ret != CL_SUCCESS)
    return;
  DPI("%s\n", buffer.data());
#endif // OMPTARGET_OPENCL_DEBUG
#endif // INTEL_CUSTOMIZATION
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
  cl_program program[3];
  cl_uint num_programs = 0;
  std::string compilation_options(DeviceInfo.CompilationOptions);
  std::string linking_options(DeviceInfo.LinkingOptions);

  DP("OpenCL compilation options: %s\n", compilation_options.c_str());

  if ((DeviceInfo.Flags & RTLDeviceInfoTy::LinkDeviceRTLFlag) != 0) {
    std::string device_rtl_path = getDeviceRTLPath();
    std::ifstream device_rtl(device_rtl_path, std::ios::binary);

    if (device_rtl.is_open()) {
      DP("Found device RTL: %s\n", device_rtl_path.c_str());
      device_rtl.seekg(0, device_rtl.end);
      int device_rtl_len = device_rtl.tellg();
      std::string device_rtl_bin(device_rtl_len, '\0');
      device_rtl.seekg(0);
      if (!device_rtl.read(&device_rtl_bin[0], device_rtl_len)) {
        DP("I/O Error: Failed to read device RTL.\n");
        return NULL;
      }

      dumpImageToFile(device_rtl_bin.c_str(), device_rtl_len, "RTL");

      program[1] = clCreateProgramWithIL(DeviceInfo.CTX[device_id],
                                         device_rtl_bin.c_str(), device_rtl_len,
                                         &status);
      if (status != CL_SUCCESS) {
        DP("Error: Failed to create device RTL from IL: %d\n", status);
        return NULL;
      }

      status = clCompileProgram(program[1], 0, nullptr,
                                compilation_options.c_str(), 0,
                                nullptr, nullptr, nullptr, nullptr);
      if (status != CL_SUCCESS) {
        debugPrintBuildLog(program[1], DeviceInfo.deviceIDs[device_id]);
        DP("Error: Failed to compile program: %d\n", status);
        return NULL;
      }
      num_programs++;
    } else {
      DP("Cannot find device RTL: %s\n", device_rtl_path.c_str());
    }
  }

  // Create program for the target regions.
  dumpImageToFile(image->ImageStart, ImageSize, "OpenMP");
  CompilationTimer.start();
  program[0] = clCreateProgramWithIL(DeviceInfo.CTX[device_id],
                                     image->ImageStart, ImageSize, &status);
  if (status != CL_SUCCESS) {
    debugPrintBuildLog(program[0], DeviceInfo.deviceIDs[device_id]);
    DP("Error: Failed to create program: %d\n", status);
    return NULL;
  }
  status = clCompileProgram(program[0], 0, nullptr,
                            compilation_options.c_str(), 0,
                            nullptr, nullptr, nullptr, nullptr);
  if (status != CL_SUCCESS) {
    debugPrintBuildLog(program[0], DeviceInfo.deviceIDs[device_id]);
    DP("Error: Failed to compile program: %d\n", status);
    return NULL;
  }

  CompilationTimer.stop();

  num_programs++;

  if (num_programs < 2)
    DP("Skipped device RTL.\n");

  DP("OpenCL linking options: %s\n", linking_options.c_str());
  // clLinkProgram drops the last symbol. Work this around temporarily.
  linking_options += " ";

  LinkingTimer.start();

  program[2] = clLinkProgram(
      DeviceInfo.CTX[device_id], 1, &DeviceInfo.deviceIDs[device_id],
      linking_options.c_str(), num_programs, &program[0], nullptr, nullptr,
      &status);
  if (status != CL_SUCCESS) {
    debugPrintBuildLog(program[2], DeviceInfo.deviceIDs[device_id]);
    DP("Error: Failed to link program: %d\n", status);
    return NULL;
  } else {
    DP("Successfully linked program.\n");
  }

  LinkingTimer.stop();

  // create kernel and target entries
  DeviceInfo.FuncGblEntries[device_id].Entries.resize(NumEntries);
  DeviceInfo.FuncGblEntries[device_id].Kernels.resize(NumEntries);
  std::vector<__tgt_offload_entry> &entries =
      DeviceInfo.FuncGblEntries[device_id].Entries;
  std::vector<cl_kernel> &kernels =
      DeviceInfo.FuncGblEntries[device_id].Kernels;

#if INTEL_CUSTOMIZATION
  cl_int (CL_API_CALL *ExtCall)(cl_device_id, cl_program, const char *,
                                size_t *, void **) = nullptr;

  if (DeviceInfo.Extensions[device_id].GetDeviceGlobalVariablePointer ==
      ExtensionStatusEnabled)
    ExtCall = reinterpret_cast<decltype(ExtCall)>(
        clGetExtensionFunctionAddressForPlatform(
            DeviceInfo.platformID,
            "clGetDeviceGlobalVariablePointerINTEL"));

  if (!ExtCall) {
    DPI("Error: clGetDeviceGlobalVariablePointerINTEL API "
        "is nullptr.  Direct references to declare target variables "
        "will not work properly.\n");
  }

  if (!DeviceInfo.clGetMemAllocInfoINTELFn &&
      DeviceInfo.Extensions[device_id].GetMemAllocInfoINTELPointer ==
      ExtensionStatusEnabled && DeviceInfo.DeviceType == CL_DEVICE_TYPE_CPU) {
    // TODO: limit this to CPU devices for the time being.
    DeviceInfo.clGetMemAllocInfoINTELFn =
        reinterpret_cast<clGetMemAllocInfoINTELTy>(
            clGetExtensionFunctionAddressForPlatform(DeviceInfo.platformID,
                                                     "clGetMemAllocInfoINTEL"));

    if (!DeviceInfo.clGetMemAllocInfoINTELFn)
      DPI("Error: clGetMemAllocInfoINTEL API is nullptr.  Direct references "
          "to declare target variables will not work properly.\n");
  }

  ProfileIntervalTy EntriesTimer("Offload entries init", device_id);
#endif  // INTEL_CUSTOMIZATION

  for (unsigned i = 0; i < NumEntries; i++) {
    // Size is 0 means that it is kernel function.
    auto Size = image->EntriesBegin[i].size;

    if (Size != 0) {
#if INTEL_CUSTOMIZATION
      EntriesTimer.start();

      void *TgtAddr = nullptr;
      auto HostAddr = image->EntriesBegin[i].addr;
      auto Name = image->EntriesBegin[i].name;
      size_t DeviceSize = 0;

      if (ExtCall) {
        DP("Looking up global variable '%s' of size %zu bytes\n",
           Name, Size);

        if (ExtCall(DeviceInfo.deviceIDs[device_id], program[2],
                    Name, &DeviceSize, &TgtAddr) != CL_SUCCESS) {
          // FIXME: this may happen for static global variables,
          //        since they are not declared as Extern, thus,
          //        the driver cannot find them. We have to be able
          //        to externalize static variables, if we name
          //        them uniquely.
          DPI("Error: clGetDeviceGlobalVariablePointerINTEL API returned "
              "nullptr for global variable '%s'.\n", Name);
          DP("Error: direct references to declare target variable '%s' "
             "will not work properly.\n", Name);
          DeviceSize = 0;
        } else if (Size != DeviceSize) {
          DP("Error: size mismatch for host (%zu) and device (%zu) versions "
             "of global variable: %s\n.  Direct references "
             "to this variable will not work properly.\n",
             Size, DeviceSize, Name);
          DeviceSize = 0;
        }
      }

      // DeviceSize equal to zero means that the symbol lookup failed.
      // Allocate the device buffer dynamically for this host object.
      // Note that the direct references to the global object in the device
      // code will refer to completely different memory, so programs
      // may produce incorrect results, e.g.:
      //          #pragma omp declare target
      //          static int a[100];
      //          void foo() {
      //            a[7] = 7;
      //          }
      //          #pragma omp end declare target
      //
      //          void bar() {
      //          #pragma omp target
      //            { foo(); }
      //          }
      //
      //        foo() will have a reference to global 'a', and there
      //        is currently no way to associate this access with the buffer
      //        that we allocate here.
      if (DeviceSize == 0) {
        TgtAddr = __tgt_rtl_data_alloc(device_id, Size, HostAddr);
        __tgt_rtl_data_submit(device_id, TgtAddr, HostAddr, Size);
      }

      DP("Global variable allocated: Name = %s, Size = %zu"
         ", HostPtr = " DPxMOD ", TgtPtr = " DPxMOD "\n",
         Name, Size, DPxPTR(HostAddr), DPxPTR(TgtAddr));
      entries[i].addr = TgtAddr;
      entries[i].name = Name;
      entries[i].size = Size;

      EntriesTimer.stop();
#endif  // INTEL_CUSTOMIZATION
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
    kernels[i] = clCreateKernel(program[2], name, &status);
    if (status != 0) {
      DP("Error: Failed to create kernel %s, %d\n", name, status);
      return NULL;
    }
    entries[i].addr = &kernels[i];
    entries[i].name = name;
#ifdef OMPTARGET_OPENCL_DEBUG
    // Show kernel information
    std::vector<char> buf;
    size_t buf_size;
    cl_uint kernel_num_args = 0;
    INVOKE_CL_RET_NULL(clGetKernelInfo, kernels[i], CL_KERNEL_FUNCTION_NAME, 0,
                       nullptr, &buf_size);
    buf.resize(buf_size);
    INVOKE_CL_RET_NULL(clGetKernelInfo, kernels[i], CL_KERNEL_FUNCTION_NAME,
                       buf_size, buf.data(), nullptr);
    INVOKE_CL_RET_NULL(clGetKernelInfo, kernels[i], CL_KERNEL_NUM_ARGS,
                       sizeof(cl_uint), &kernel_num_args, nullptr);
    DP("Kernel %d: Name = %s, NumArgs = %d\n", i, buf.data(), kernel_num_args);
    for (unsigned idx = 0; idx < kernel_num_args; idx++) {
      INVOKE_CL_RET_NULL(clGetKernelArgInfo, kernels[i], idx,
                         CL_KERNEL_ARG_TYPE_NAME, 0, nullptr, &buf_size);
      buf.resize(buf_size);
      INVOKE_CL_RET_NULL(clGetKernelArgInfo, kernels[i], idx,
                         CL_KERNEL_ARG_TYPE_NAME, buf_size, buf.data(),
                         nullptr);
      std::string type_name = buf.data();
      INVOKE_CL_RET_NULL(clGetKernelArgInfo, kernels[i], idx,
                         CL_KERNEL_ARG_NAME, 0, nullptr, &buf_size);
      buf.resize(buf_size);
      INVOKE_CL_RET_NULL(clGetKernelArgInfo, kernels[i], idx,
                         CL_KERNEL_ARG_NAME, buf_size, buf.data(), nullptr);
      DP("  Arg %2d: %s %s\n", idx, type_name.c_str(), buf.data());
    }
#endif // OMPTARGET_OPENCL_DEBUG
  }

  // Release intermediate programs and store the final program.
  for (uint32_t i = 0; i < num_programs; i++) {
    INVOKE_CL_EXIT_FAIL(clReleaseProgram, program[i]);
  }
  DeviceInfo.FuncGblEntries[device_id].Program = program[2];
  if (initProgram(device_id) != OFFLOAD_SUCCESS)
    return nullptr;
  __tgt_target_table &table = DeviceInfo.FuncGblEntries[device_id].Table;
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
    clGetEventInfo(event, CL_EVENT_COMMAND_TYPE, sizeof(cmd), &cmd, nullptr);

    // Release the temporary cl_mem object used in the data operation.
    if (async_data->MemToRelease &&
        (cmd == CL_COMMAND_READ_BUFFER || cmd == CL_COMMAND_WRITE_BUFFER)) {
      if (clReleaseMemObject(async_data->MemToRelease) != CL_SUCCESS) {
        FATAL_ERROR("Failed to handle asynchronous data operation");
      }
    }

    if (DeviceInfo.Flags & RTLDeviceInfoTy::EnableProfileFlag) {
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
      DeviceInfo.Profiles[async_data->DeviceId].update(event_name, event);
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
  DeviceInfo.Mutexes[device_id].lock();
  DeviceInfo.ImplicitArgs[device_id][*kernel] =
      std::set<void *>(tgt_ptrs, tgt_ptrs + num_ptrs);
  DeviceInfo.Mutexes[device_id].unlock();

  return OFFLOAD_SUCCESS;
}

EXTERN void *__tgt_rtl_create_offload_pipe(int32_t device_id, bool is_async) {
  // Return a shared in-order queue for synchronous case if requested
  if (!is_async && DeviceInfo.Flags &
                   RTLDeviceInfoTy::UseInteropQueueInorderSharedSyncFlag) {
    DP("%s returns the shared in-order queue " DPxMOD "\n", __func__,
       DPxPTR(DeviceInfo.Queues[device_id]));
    return (void *)DeviceInfo.Queues[device_id];
  }

  cl_int status;
  cl_command_queue queue;
  auto deviceId = DeviceInfo.deviceIDs[device_id];
  auto context = DeviceInfo.CTX[device_id];

  // Return a shared out-of-order queue for asynchronous case by default
  if (is_async &&
      !(DeviceInfo.Flags & RTLDeviceInfoTy::UseInteropQueueInorderAsyncFlag)) {
    queue = DeviceInfo.QueuesOOO[device_id];
    if (!queue) {
      cl_queue_properties qprops[3] =
          {CL_QUEUE_PROPERTIES, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, 0};
      queue = clCreateCommandQueueWithProperties(context, deviceId, qprops,
                                                 &status);
      DP("%s creates a shared out-of-order queue " DPxMOD "\n", __func__,
         DPxPTR(queue));
      if (status != CL_SUCCESS) {
        DP("Error: Failed to create interop command queue: %d\n", status);
        return nullptr;
      }
      DeviceInfo.QueuesOOO[device_id] = queue;
    }
    DP("%s returns a shared out-of-order queue " DPxMOD "\n", __func__,
       DPxPTR(queue));
    return (void *)queue;
  }

  // Return a new in-order queue for other cases
  queue = clCreateCommandQueueWithProperties(context, deviceId, nullptr,
                                             &status);
  if (status != CL_SUCCESS) {
    DP("Error: Failed to create interop command queue\n");
    return nullptr;
  }
  DP("%s creates and returns a separate in-order queue " DPxMOD "\n", __func__,
     DPxPTR(queue));
  return (void *)queue;
}

// Release the command queue if it is a new in-order command queue.
EXTERN int32_t __tgt_rtl_release_offload_pipe(int32_t device_id, void *pipe) {
  cl_command_queue queue = (cl_command_queue)pipe;
  if (queue != DeviceInfo.QueuesOOO[device_id] &&
      queue != DeviceInfo.Queues[device_id]) {
    INVOKE_CL_RET_FAIL(clReleaseCommandQueue, queue);
    DP("%s releases a separate in-order queue " DPxMOD "\n",__func__,
       DPxPTR(queue));
  }
  return OFFLOAD_SUCCESS;
}

#if INTEL_CUSTOMIZATION
// Allocate a managed memory object.
EXTERN void *__tgt_rtl_data_alloc_managed(int32_t device_id, int64_t size) {
  if (!DeviceInfo.clHostMemAllocINTELFn) {
    DP("clHostMemAllocINTEL is not available\n");
    return nullptr;
  }
  cl_mem_properties_intel properties[] = {
      CL_MEM_ALLOC_FLAGS_INTEL, CL_MEM_ALLOC_DEFAULT_INTEL, 0};
  cl_int rc;
  void *mem = DeviceInfo.clHostMemAllocINTELFn(
      DeviceInfo.CTX[device_id], properties, size, 0, &rc);
  if (rc != CL_SUCCESS) {
    DP("clHostMemAllocINTEL failed with error code %d, %s\n", rc,
       getCLErrorName(rc));
    return nullptr;
  }
  DP("Allocated a managed memory object " DPxMOD "\n", DPxPTR(mem));
  return mem;
}

// Delete a managed memory object.
EXTERN int32_t __tgt_rtl_data_delete_managed(int32_t device_id, void *ptr) {
  if (!DeviceInfo.clMemFreeINTELFn) {
    DP("clMemFreeINTEL is not available\n");
    return OFFLOAD_FAIL;
  }
  INVOKE_CL_RET_FAIL(DeviceInfo.clMemFreeINTELFn, DeviceInfo.CTX[device_id],
                     ptr);
  DP("Deleted a managed memory object " DPxMOD "\n", DPxPTR(ptr));
  return OFFLOAD_SUCCESS;
}
#endif // INTEL_CUSTOMIZATION

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

  void *base = clSVMAlloc(DeviceInfo.CTX[device_id], CL_MEM_READ_WRITE,
      meaningful_size + meaningful_offset, 0);
  if (!base) {
    DP("Error: Failed to allocate base buffer\n");
    return nullptr;
  }
  DP("Created base buffer " DPxMOD " during data alloc\n", DPxPTR(base));

  void *ret = (void *)((intptr_t)base + meaningful_offset);

  // Store allocation information
  DeviceInfo.Buffers[device_id][ret] = {base, meaningful_size};

  // Store list of pointers to be passed to kernel implicitly
  if (is_implicit_arg) {
    DP("Stashing an implicit argument " DPxMOD " for next kernel\n",
       DPxPTR(ret));
    DeviceInfo.Mutexes[device_id].lock();
    // key "0" for kernel-independent implicit arguments
    DeviceInfo.ImplicitArgs[device_id][0].insert(ret);
    DeviceInfo.Mutexes[device_id].unlock();
  }

  return ret;
}

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
  if (DeviceInfo.DeviceType != CL_DEVICE_TYPE_GPU) {
    DP("Attemping to create buffer for cpu offloading.\n");
    return nullptr;
  }
  if (DeviceInfo.Buffers[device_id].count(tgt_ptr) == 0) {
    DP("Error: Cannot create buffer from unknown device pointer " DPxMOD "\n",
       DPxPTR(tgt_ptr));
    return nullptr;
  }
  cl_int rc;
  int64_t size = DeviceInfo.Buffers[device_id][tgt_ptr].Size;
  cl_mem ret = clCreateBuffer(DeviceInfo.CTX[device_id], CL_MEM_USE_HOST_PTR,
                              size, tgt_ptr, &rc);
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
  INVOKE_CL_RET_FAIL(clReleaseMemObject, (cl_mem)tgt_buffer);
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

  cl_command_queue queue = DeviceInfo.Queues[device_id];

  // Add synthetic delay for experiments
  addDataTransferLatency();

  uint64_t profile_enabled =
      DeviceInfo.Flags & RTLDeviceInfoTy::EnableProfileFlag;

  AsyncDataTy *async_data = nullptr;
  if (async_event && ((AsyncEventTy *)async_event)->handler) {
    async_data = new AsyncDataTy((AsyncEventTy *)async_event, device_id);
  }

  switch (DeviceInfo.DataTransferMethod) {
  case DATA_TRANSFER_METHOD_SVMMAP: {
    // No asynchronous data copy here since we use map/unmap as explicit
    // synchronization points.
    ProfileIntervalTy SubmitTime("DATA-WRITE", device_id);
    SubmitTime.start();

    INVOKE_CL_RET_FAIL(clEnqueueSVMMap, queue, CL_TRUE, CL_MAP_WRITE, tgt_ptr,
                       size, 0, nullptr, nullptr);
    memcpy(tgt_ptr, hst_ptr, size);
    INVOKE_CL_RET_FAIL(clEnqueueSVMUnmap, queue, tgt_ptr, 0, nullptr, nullptr);

    SubmitTime.stop();
  } break;
  case DATA_TRANSFER_METHOD_SVMMEMCPY: {
    cl_event event;
    if (async_data) {
      INVOKE_CL_RET_FAIL(clEnqueueSVMMemcpy, queue, CL_FALSE, tgt_ptr, hst_ptr,
                         size, 0, nullptr, &event);
      INVOKE_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                         &event_callback_completed, async_data);
    } else {
      INVOKE_CL_RET_FAIL(clEnqueueSVMMemcpy, queue, CL_TRUE, tgt_ptr, hst_ptr,
                         size, 0, nullptr, &event);
      if (profile_enabled)
        DeviceInfo.Profiles[device_id].update("DATA-WRITE", event);
    }
  } break;
  case DATA_TRANSFER_METHOD_CLMEM:
  default: {
    cl_event event;
    cl_int rc;
    cl_mem mem = clCreateBuffer(DeviceInfo.CTX[device_id], CL_MEM_USE_HOST_PTR,
                                size, tgt_ptr, &rc);
    if (rc != CL_SUCCESS) {
      DP("Error: Failed to create a buffer from a SVM pointer " DPxMOD "\n",
         DPxPTR(tgt_ptr));
      return OFFLOAD_FAIL;
    }
    if (async_data) {
      async_data->MemToRelease = mem;
      INVOKE_CL_RET_FAIL(clEnqueueWriteBuffer, queue, mem, CL_FALSE, 0, size,
                         hst_ptr, 0, nullptr, &event);
      INVOKE_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                         &event_callback_completed, async_data);
    } else {
      INVOKE_CL_RET_FAIL(clEnqueueWriteBuffer, queue, mem, CL_TRUE, 0, size,
                         hst_ptr, 0, nullptr, &event);
      INVOKE_CL_RET_FAIL(clReleaseMemObject, mem);
      if (profile_enabled)
        DeviceInfo.Profiles[device_id].update("DATA-WRITE", event);
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
int32_t __tgt_rtl_data_retrieve_nowait(int32_t device_id, void *hst_ptr,
                                       void *tgt_ptr, int64_t size,
                                       void *async_event) {
  if (size == 0)
    // All other plugins seem to be handling 0 size gracefully,
    // so we should do as well.
    return OFFLOAD_SUCCESS;

  cl_command_queue queue = DeviceInfo.Queues[device_id];

  // Add synthetic delay for experiments
  addDataTransferLatency();

  uint64_t profile_enabled =
      DeviceInfo.Flags & RTLDeviceInfoTy::EnableProfileFlag;

  AsyncDataTy *async_data = nullptr;
  if (async_event && ((AsyncEventTy *)async_event)->handler) {
    async_data = new AsyncDataTy((AsyncEventTy *)async_event, device_id);
  }

  switch (DeviceInfo.DataTransferMethod) {
  case DATA_TRANSFER_METHOD_SVMMAP: {
    // No asynchronous data copy here since we use map/unmap as explicit
    // synchronization points.
    ProfileIntervalTy RetrieveTime("DATA-READ", device_id);
    RetrieveTime.start();

    INVOKE_CL_RET_FAIL(clEnqueueSVMMap, queue, CL_TRUE, CL_MAP_READ, tgt_ptr,
                       size, 0, nullptr, nullptr);
    memcpy(hst_ptr, tgt_ptr, size);
    INVOKE_CL_RET_FAIL(clEnqueueSVMUnmap, queue, tgt_ptr, 0, nullptr, nullptr);

    RetrieveTime.stop();
  } break;
  case DATA_TRANSFER_METHOD_SVMMEMCPY: {
    cl_event event;
    if (async_data) {
      INVOKE_CL_RET_FAIL(clEnqueueSVMMemcpy, queue, CL_FALSE, hst_ptr, tgt_ptr,
                         size, 0, nullptr, &event);
      INVOKE_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                         &event_callback_completed, async_data);
    } else {
      INVOKE_CL_RET_FAIL(clEnqueueSVMMemcpy, queue, CL_TRUE, hst_ptr, tgt_ptr,
                         size, 0, nullptr, &event);
      if (profile_enabled)
        DeviceInfo.Profiles[device_id].update("DATA-READ", event);
    }
  } break;
  case DATA_TRANSFER_METHOD_CLMEM:
  default: {
    cl_int rc;
    cl_event event;
    cl_mem mem = clCreateBuffer(DeviceInfo.CTX[device_id], CL_MEM_USE_HOST_PTR,
                                size, tgt_ptr, &rc);
    if (rc != CL_SUCCESS) {
      DP("Error: Failed to create a buffer from a SVM pointer " DPxMOD "\n",
         DPxPTR(tgt_ptr));
      return OFFLOAD_FAIL;
    }
    if (async_data) {
      INVOKE_CL_RET_FAIL(clEnqueueReadBuffer, queue, mem, CL_FALSE, 0, size,
                         hst_ptr, 0, nullptr, &event);
      async_data->MemToRelease = mem;
      INVOKE_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                         &event_callback_completed, async_data);
    } else {
      INVOKE_CL_RET_FAIL(clEnqueueReadBuffer, queue, mem, CL_TRUE, 0, size,
                         hst_ptr, 0, nullptr, &event);
      INVOKE_CL_RET_FAIL(clReleaseMemObject, mem);
      if (profile_enabled)
        DeviceInfo.Profiles[device_id].update("DATA-READ", event);
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
int32_t __tgt_rtl_data_delete(int32_t device_id, void *tgt_ptr) {
  if (DeviceInfo.Buffers[device_id].count(tgt_ptr) == 0) {
    DP("Cannot find allocation information for " DPxMOD "\n", DPxPTR(tgt_ptr));
    return OFFLOAD_FAIL;
  }
  void *base = DeviceInfo.Buffers[device_id][tgt_ptr].Base;
  DeviceInfo.Buffers[device_id].erase(tgt_ptr);

  DeviceInfo.Mutexes[device_id].lock();
  // Erase from the internal list
  for (auto &J : DeviceInfo.ImplicitArgs[device_id])
    J.second.erase(tgt_ptr);
  DeviceInfo.Mutexes[device_id].unlock();

  clSVMFree(DeviceInfo.CTX[device_id], base);
  return OFFLOAD_SUCCESS;
}

static inline int32_t run_target_team_nd_region(
    int32_t device_id, void *tgt_entry_ptr, void **tgt_args,
    ptrdiff_t *tgt_offsets, int32_t num_args, int32_t num_teams,
    int32_t thread_limit, void *loop_desc, void *async_event) {

  cl_kernel *kernel = static_cast<cl_kernel *>(tgt_entry_ptr);

  // compute local/global work size

  // TODO: this looks valid only for ATS
  // TODO: do whatever changes for ATS later once we have access to the device.
  // size_t simd_len = 16;
  // size_t local_work_size_max = 64 * simd_len;

  // For portability, we also need to set max local_work_size.
  size_t local_work_size_max = DeviceInfo.maxWorkGroupSize[device_id];
  DP("Maximum work-group size on the device is %zu.\n", local_work_size_max);
  size_t num_execution_units_max = DeviceInfo.maxExecutionUnits[device_id];
  DP("Number of execution units on the device is %zu.\n",
     num_execution_units_max);

#ifdef OMPTARGET_OPENCL_DEBUG
  // TODO: kernels using to much SLM may limit the number of
  //       work groups running simultaneously on a sub slice.
  //       We may take this into account for computing the work partitioning.
  size_t device_local_mem_size = (size_t)DeviceInfo.SLMSize[device_id];
  DP("Device local mem size: %zu\n", device_local_mem_size);
  cl_ulong local_mem_size_tmp = 0;
  INVOKE_CL_RET_FAIL(clGetKernelWorkGroupInfo, *kernel,
                     DeviceInfo.deviceIDs[device_id], CL_KERNEL_LOCAL_MEM_SIZE,
                     sizeof(local_mem_size_tmp), &local_mem_size_tmp, nullptr);
  size_t kernel_local_mem_size = (size_t)local_mem_size_tmp;
  DP("Kernel local mem size: %zu\n", kernel_local_mem_size);
#endif  // OMPTARGET_OPENCL_DEBUG

  size_t kernel_simd_width = 1;
  INVOKE_CL_RET_FAIL(clGetKernelWorkGroupInfo, *kernel,
                     DeviceInfo.deviceIDs[device_id],
                     CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                     sizeof(size_t), &kernel_simd_width, nullptr);
  DP("Preferred work-group size multiple: %zu\n", kernel_simd_width);

  // Account for kernel-specific maximum work group size.
  size_t kernel_wg_size = 1;

  INVOKE_CL_RET_FAIL(clGetKernelWorkGroupInfo, *kernel,
                     DeviceInfo.deviceIDs[device_id], CL_KERNEL_WORK_GROUP_SIZE,
                     sizeof(size_t), &kernel_wg_size, nullptr);
  if (kernel_wg_size < local_work_size_max) {
    local_work_size_max = kernel_wg_size;
    DP("Capping maximum work-group size to %zu due to kernel constraints.\n",
       local_work_size_max);
  }

  bool local_size_forced_by_user = false;

  if (thread_limit > 0 &&
      (size_t)thread_limit <= local_work_size_max) {
    local_work_size_max = (size_t)thread_limit;
    local_size_forced_by_user = true;
    DP("Setting maximum work-group size to %zu (due to thread_limit clause).\n",
       local_work_size_max);
  }

  if (DeviceInfo.OMPThreadLimit > 0 &&
      (size_t)DeviceInfo.OMPThreadLimit <= local_work_size_max) {
    local_work_size_max = (size_t)DeviceInfo.OMPThreadLimit;
    local_size_forced_by_user = true;
    DP("Setting maximum work-group size to %zu (due to OMP_THREAD_LIMIT).\n",
       local_work_size_max);
  }

#if INTEL_CUSTOMIZATION
  bool num_teams_forced_by_user = false;

  // We are currently handling only GEN9/GEN9.5 here.
  // TODO: we need to find a way to compute the number of sub slices
  //       and number of EUs per sub slice for the particular device.
  size_t number_of_subslices = 9;
  size_t eus_per_ss = 8;
  size_t threads_per_eu = 7;


  // Each EU has 7 threads. A work group is partitioned into EU threads,
  // and then scheduled onto a sub slice. A sub slice must have all the
  // resources available to start a work group, otherwise it will wait
  // for resources. This means that uneven partitioning may result
  // in waiting work groups, and also unused EUs.
  // See slides 25-27 here:
  //   https://software.intel.com/sites/default/files/\
  //   Faster-Better-Pixels-on-the-Go-and-in-the-Cloud-\
  //   with-OpenCL-on-Intel-Architecture.pdf
  if (num_execution_units_max >= 72) {
    // Default best Halo (GT4) configuration.
  } else if (num_execution_units_max >= 48) {
    // GT3
    number_of_subslices = 6;
  } else if (num_execution_units_max >= 24) {
    // GT2
    number_of_subslices = 3;
  } else if (num_execution_units_max >= 18) {
    // GT1.5
    number_of_subslices = 3;
    eus_per_ss = 6;
  } else {
    // GT1
    number_of_subslices = 2;
    eus_per_ss = 6;
  }

  size_t num_work_groups_max =
      number_of_subslices * eus_per_ss * threads_per_eu;

  // For open-source keep the previous default, i.e. maximum number
  // of work groups is equal to the number of execution units.
#else  // INTEL_CUSTOMIZATION
  size_t num_work_groups_max = num_execution_units_max;
#endif // INTEL_CUSTOMIZATION

  if (num_teams > 0) {
    num_work_groups_max = (size_t)num_teams;
#if INTEL_CUSTOMIZATION
    num_teams_forced_by_user = true;
#endif  // INTEL_CUSTOMIZATION
    DP("Setting maximum number of work groups to %zu "
       "(due to num_teams clause).\n", num_work_groups_max);
  }

  int64_t *loop_levels = loop_desc ? (int64_t *)loop_desc : nullptr;

#if INTEL_CUSTOMIZATION
  // With specific ND-range parallelization we use 8/16/32 WIs per WG.
  // Each WG can be run by one EU thread, so all work groups evenly
  // fit the sub slices.
  if (!loop_levels &&
      // If user specifies both number of work groups and the local size,
      // the we must honor that.
      (!local_size_forced_by_user || !num_teams_forced_by_user)) {
    // For default ND-range parallelization, i.e. when we do not know
    // the number of iterations, we want to create work groups with
    // maximum local size and also make sure that we can start all
    // work groups at once. We want to maximize the local size
    // to reduce the number of work groups run on the same sub slice
    // simultaneously, because this number is limited by the amount
    // of local memory used by the kernel, and the number of barriers,
    // which is currently unknown.

    size_t threads_per_ss = eus_per_ss * threads_per_eu;

    if (num_teams_forced_by_user) {
      if (num_work_groups_max >= number_of_subslices * threads_per_ss) {
        // If the requested number of teams is bigger than the number
        // of work groups that the HW can run simultaneously, then
        // just use the kernel's SIMD width for the local size.
        // This means every EU thread will run one work group,
        // but some work groups may be waiting for vacant threads.
        local_work_size_max = kernel_simd_width;
      } else {
        // Try to maximize the local size, but still fit all work groups
        // into the sub slices at once.
        assert((local_work_size_max <= kernel_simd_width ||
                (local_work_size_max % kernel_simd_width) == 0) &&
               "invalid local_work_size_max.");

        while (local_work_size_max > kernel_simd_width) {
          size_t threads_per_wg = local_work_size_max / kernel_simd_width;
          size_t wgs_per_ss = threads_per_ss / threads_per_wg;
          size_t number_of_simultaneous_wgs = wgs_per_ss * number_of_subslices;
          if (number_of_simultaneous_wgs >= num_work_groups_max)
            break;

          local_work_size_max -= kernel_simd_width;
        }
        // The minimum value for local_work_size_max is kernel_simd_width
        // after this loop, which means one work group is run by one
        // EU thread.
      }
    } else if (local_size_forced_by_user) {
      // Local size is fixed by the user, so we need to compute
      // the maximum number of simultaneously running work groups,
      // which will be used later to set the global size.
      size_t threads_per_wg =
          (local_work_size_max + kernel_simd_width - 1) / kernel_simd_width;
      size_t wgs_per_ss = threads_per_ss / threads_per_wg;
      num_work_groups_max = wgs_per_ss * number_of_subslices;
    } else {
      assert(!local_size_forced_by_user && !num_teams_forced_by_user &&
             "mismatched conditions.");
      // Make sure we use all sub slices without loss of any EU thread,
      // maximize the work group size, and also fit all work groups
      // at once.
      //
      // Even use of sub slices means:
      //   (eus_per_ss * threads_per_eu) % (local_size / SIMDWidth) == 0
      assert((local_work_size_max <= kernel_simd_width ||
              (local_work_size_max % kernel_simd_width) == 0) &&
             "invalid local_work_size_max.");

      while (local_work_size_max > kernel_simd_width) {
        size_t threads_per_wg = local_work_size_max / kernel_simd_width;
        if ((threads_per_ss % threads_per_wg) == 0) {
          size_t wgs_per_ss = threads_per_ss / threads_per_wg;
          size_t max_wg_num = wgs_per_ss * number_of_subslices;
          num_work_groups_max = max_wg_num;
          break;
        }

        local_work_size_max -= kernel_simd_width;
      }
    }
  }
#endif  // INTEL_CUSTOMIZATION

  size_t optimal_work_size = local_work_size_max;

  if (loop_levels && !local_size_forced_by_user &&
      optimal_work_size > kernel_simd_width)
    // Default to 8/16/32 WIs per WG for ND-range paritioning depending
    // on the SIMD width the kernel was compiled for.
    // This size seems to provide the best results for steam and nbody
    // benchmarks. Users may use more WIs/WG by using thread_limit clause
    // and OMP_THREAD_LIMIT, but the number may not exceed OpenCL limits.
    optimal_work_size = kernel_simd_width;

  // TODO: we may want to reshape local work if necessary.
  size_t local_work_size[3] = { 1, 1, 1 };
  size_t num_work_groups[3] = { 1, 1, 1 };
  int32_t work_dim = 1;

  if (loop_levels) {
    // ND-range dimension 0 is the fastest changing one.
    // It corresponds to the innermost OpenMP loop in a loop nest.
    work_dim = (int32_t)*loop_levels;
    assert(work_dim > 0 && work_dim <= 3 &&
           "ND-range parallelization requested "
           "with invalid number of dimensions.");
    if (work_dim == 1)
      // Keep the current local_size default for 1D cases.
      local_work_size[0] = optimal_work_size;
    else
      // TODO: we should take into account the global size,
      //       e.g. if the 1st dimension is small, e.g. 8, it may make
      //       sense to use (8, 2, 1) instead of (8, 1, 1),
      //       assuming that optimal_work_size is 16.
      local_work_size[0] = optimal_work_size;
  }
  else {
    local_work_size[0] = optimal_work_size;
    num_work_groups[0] = num_work_groups_max;
#if INTEL_CUSTOMIZATION
    if (!num_teams_forced_by_user)
      num_work_groups[0] *= DeviceInfo.SubscriptionRate;
#endif  // INTEL_CUSTOMIZATION
  }

  // Compute num_work_groups using the loop descriptor.
  if (loop_levels) {
    assert(num_teams <= 0 &&
           "ND-range parallelization requested with num_teams.");
    TgtLoopDescTy *level = (TgtLoopDescTy *)(loop_levels + 1);

    for (int32_t i = 0; i < work_dim; ++i) {
      assert(level[i].ub >= level[i].lb && level[i].stride > 0);
      DP("NDrange[dim=%d]: (lb=%" PRId64 ", ub=%" PRId64
         ", stride=%" PRId64 ")\n",
         i, level[i].lb, level[i].ub, level[i].stride);
      size_t trip =
          (level[i].ub - level[i].lb + level[i].stride) / level[i].stride;
      if (local_work_size[i] >= trip)
        local_work_size[i] = trip;

      num_work_groups[i] = (trip + local_work_size[i] - 1) / local_work_size[i];
    }
  }

  size_t global_work_size[3];
  for (int32_t i = 0; i < 3; ++i)
    global_work_size[i] = local_work_size[i] * num_work_groups[i];

  DP("THREAD_LIMIT = %d, NUM_TEAMS = %d\n", thread_limit, num_teams);
  if (loop_levels) {
    DP("Collapsed %" PRId64 " loops.\n", *loop_levels);
  }
  DP("Global work size = (%zu, %zu, %zu)\n", global_work_size[0],
     global_work_size[1], global_work_size[2]);
  DP("Local work size = (%zu, %zu, %zu)\n", local_work_size[0],
     local_work_size[1], local_work_size[2]);
  DP("Work dimension = %d\n", work_dim);

  // Protect thread-unsafe OpenCL API calls
  DeviceInfo.Mutexes[device_id].lock();

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
  if (DeviceInfo.ImplicitArgs[device_id].count(*kernel) > 0) {
    num_implicit_args += DeviceInfo.ImplicitArgs[device_id][*kernel].size();
  }

  implicit_args.reserve(num_implicit_args);

  if (DeviceInfo.ImplicitArgs[device_id].count(*kernel) > 0) {
    // kernel-dependent arguments
    std::copy_if(DeviceInfo.ImplicitArgs[device_id][*kernel].begin(),
                 DeviceInfo.ImplicitArgs[device_id][*kernel].end(),
                 std::back_inserter(implicit_args),
                 [] (void *ptr) {
                   return ptr != nullptr;
                 });
  }
  if (DeviceInfo.ImplicitArgs[device_id].count(0) > 0) {
    // kernel-independent arguments
    // Note that these pointers may not be nullptr.
    implicit_args.insert(implicit_args.end(),
        DeviceInfo.ImplicitArgs[device_id][0].begin(),
        DeviceInfo.ImplicitArgs[device_id][0].end());
  }

  // set kernel args
  std::vector<void *> ptrs(num_args);
  for (int32_t i = 0; i < num_args; ++i) {
    ptrs[i] = (void *)((intptr_t)tgt_args[i] + tgt_offsets[i]);
    INVOKE_CL_RET_FAIL(clSetKernelArgSVMPointer, *kernel, i, ptrs[i]);
    DP("Kernel Arg %d set successfully\n", i);
  }

#if INTEL_CUSTOMIZATION
  if (DeviceInfo.clGetMemAllocInfoINTELFn) {
    // Reserve space for USM pointers.
    implicit_usm_args.reserve(num_implicit_args);
    // Move USM pointers into a separate list, since they need to be
    // reported to the runtime using a separate clSetKernelExecInfo call.
    implicit_args.erase(
        std::remove_if(implicit_args.begin(),
                       implicit_args.end(),
                       [&](void *ptr) {
                         cl_unified_shared_memory_type_intel type = 0;
                         INVOKE_CL_RET(false,
                             DeviceInfo.clGetMemAllocInfoINTELFn,
                             DeviceInfo.CTX[device_id],
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
    INVOKE_CL_RET_FAIL(clSetKernelExecInfo, *kernel,
                       CL_KERNEL_EXEC_INFO_SVM_PTRS,
                       sizeof(void *) * implicit_args.size(),
                       implicit_args.data());
  }

#if INTEL_CUSTOMIZATION
  if (implicit_usm_args.size() > 0) {
    // Report non-argument USM pointers to the runtime.
    DP("Calling clSetKernelExecInfo to pass %zu implicit USM arguments "
       "to kernel " DPxMOD "\n", implicit_usm_args.size(), DPxPTR(kernel));
    INVOKE_CL_RET_FAIL(clSetKernelExecInfo, *kernel,
                       CL_KERNEL_EXEC_INFO_USM_PTRS_INTEL,
                       sizeof(void *) * implicit_usm_args.size(),
                       implicit_usm_args.data());
    // Mark the kernel as supporting indirect USM accesses, otherwise,
    // clEnqueueNDRangeKernel call below will fail.
    cl_bool KernelSupportsUSM = CL_TRUE;
    INVOKE_CL_RET_FAIL(clSetKernelExecInfo, *kernel,
                       CL_KERNEL_EXEC_INFO_INDIRECT_DEVICE_ACCESS_INTEL,
                       sizeof(cl_bool), &KernelSupportsUSM);
  }
#endif  // INTEL_CUSTOMIZATION

  if (omptEnabled.enabled) {
    // Push current work size
    size_t finalNumTeams =
        global_work_size[0] * global_work_size[1] * global_work_size[2];
    size_t finalThreadLimit =
        local_work_size[0] * local_work_size[1] * local_work_size[2];
    finalNumTeams /= finalThreadLimit;
    omptTracePtr->pushWorkSize(finalNumTeams, finalThreadLimit);
  }

  cl_event event;
  INVOKE_CL_RET_FAIL(clEnqueueNDRangeKernel, DeviceInfo.Queues[device_id],
                     *kernel, (cl_uint)work_dim, nullptr, global_work_size,
                     local_work_size, 0, nullptr, &event);

  DeviceInfo.Mutexes[device_id].unlock();

  DP("Started executing kernel.\n");

  if (async_event) {
    if (((AsyncEventTy *)async_event)->handler) {
      // Add event handler if necessary.
      INVOKE_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
          &event_callback_completed,
          new AsyncDataTy((AsyncEventTy *)async_event, device_id));
    } else {
      // Make sure all queued commands finish before the next one starts.
      INVOKE_CL_RET_FAIL(clEnqueueBarrierWithWaitList,
                         DeviceInfo.Queues[device_id], 0, nullptr, nullptr);
    }
  } else {
    if (DeviceInfo.Flags & RTLDeviceInfoTy::EnableProfileFlag) {
      std::vector<char> buf;
      size_t buf_size;
      INVOKE_CL_RET_FAIL(clWaitForEvents, 1, &event);
      INVOKE_CL_RET_FAIL(clGetKernelInfo, *kernel, CL_KERNEL_FUNCTION_NAME, 0,
                         nullptr, &buf_size);
      std::string kernel_name("EXEC-");
      if (buf_size > 0) {
        buf.resize(buf_size);
        INVOKE_CL_RET_FAIL(clGetKernelInfo, *kernel, CL_KERNEL_FUNCTION_NAME,
                           buf.size(), buf.data(), nullptr);
        kernel_name += buf.data();
      }
      DeviceInfo.Profiles[device_id].update(kernel_name.c_str(), event);
    }
    INVOKE_CL_RET_FAIL(clFinish, DeviceInfo.Queues[device_id]);
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
int32_t __tgt_rtl_run_target_region(int32_t device_id, void *tgt_entry_ptr,
                                    void **tgt_args, ptrdiff_t *tgt_offsets,
                                    int32_t arg_num) {
  // use one team!
  return __tgt_rtl_run_target_team_region(device_id, tgt_entry_ptr, tgt_args,
                                          tgt_offsets, arg_num, 1, 0, 0);
}

EXTERN char *__tgt_rtl_get_device_name(
    int32_t device_id, char *buffer, size_t buffer_max_size) {
  assert(buffer && "buffer cannot be nullptr.");
  assert(buffer_max_size > 0 && "buffer_max_size cannot be zero.");
  INVOKE_CL_RET_NULL(clGetDeviceInfo, DeviceInfo.deviceIDs[device_id],
                     CL_DEVICE_NAME, buffer_max_size, buffer, nullptr);
  return buffer;
}

#ifdef __cplusplus
}
#endif

#endif // INTEL_COLLAB

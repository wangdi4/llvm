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
static int DebugLevel = 0;
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

#define INVOKE_CL_RET(ret, fn, ...)                                            \
  do {                                                                         \
    cl_int rc = fn(__VA_ARGS__);                                               \
    if (rc != CL_SUCCESS) {                                                    \
      DP("Error: %s:%s failed with error code %d, %s\n", __func__, #fn, rc,    \
         getCLErrorName(rc));                                                  \
      return ret;                                                              \
    }                                                                          \
  } while (false)

#define INVOKE_CL_RET_FAIL(fn, ...) INVOKE_CL_RET(OFFLOAD_FAIL, fn, __VA_ARGS__)
#define INVOKE_CL_RET_NULL(fn, ...) INVOKE_CL_RET(NULL, fn, __VA_ARGS__)

#define OFFLOADSECTIONNAME ".omp_offloading.entries"
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
};

/// Loop descriptor
typedef struct {
  int64_t lb;     // The lower bound of the i-th loop
  int64_t ub;     // The upper bound of the i-th loop
  int64_t stride; // The stride of the i-th loop
} TgtLoopDescTy;

typedef enum {
  PROFILE_ENABLED = 0x1,
  PROFILE_UNIT_USEC = 0x2,
  // more option can be added;
} ProfileFlagTy;

struct RTLProfileTy {
  int64_t flags;
  std::map<std::string, double> data;

  RTLProfileTy() : flags(0), data() {
    // parse user input
    const char *env = std::getenv("LIBOMPTARGET_PROFILE");
    if (env) {
      std::istringstream env_str(env);
      std::string token;
      while (std::getline(env_str, token, ',')) {
        if (token == "T")
          flags |= PROFILE_ENABLED;
        else if (token == "unit_usec")
          flags |= PROFILE_UNIT_USEC;
      }
    }
  }

  ~RTLProfileTy() {
    if (flags & PROFILE_ENABLED) {
      fprintf(stderr, "LIBOMPTARGET_PROFILE:\n");
      for (const auto &d : data) {
        const char *unit = "msec"; // msec by default
        double value = d.second * 1e-6;
        if (flags & PROFILE_UNIT_USEC) {
          unit = "usec";
          value = d.second * 1e-3;
        }
        fprintf(stderr, "-- %s: %.3f %s\n", d.first.c_str(), value, unit);
      }
    }
  }

  // for non-event profile
  void update(const char *name, cl_ulong elapsed) {
    std::string key(name);
    data[key] += elapsed;
  }

  // for event profile
  void update(const char *name, cl_event event) {
    cl_ulong begin = 0, end = 0;
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_QUEUED,
                            sizeof(cl_ulong), &begin, nullptr);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_COMPLETE,
                            sizeof(cl_ulong), &end, nullptr);
    update(name, end - begin);
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
#if INTEL_CUSTOMIZATION
  // clGetDeviceGlobalVariablePointerINTEL API:
  ExtensionStatusTy GetDeviceGlobalVariablePointer = ExtensionStatusUnknown;
#endif  // INTEL_CUSTOMIZATION

  // Initialize extensions' statuses for the given device.
  int32_t getExtensionsInfoForDevice(int32_t DeviceId);
};

/// Data for handling asynchronous calls.
struct AsyncEventTy {
  void (*handler)(void *); // Handler for the event
  void *arg;               // Argument to the handler
  void *data;              // Internal data
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

/// Class containing all the device information.
class RTLDeviceInfoTy {

public:
  cl_uint numDevices;
  cl_platform_id platformID;
  // per device information
  std::vector<cl_device_id> deviceIDs;
  std::vector<int32_t> maxWorkGroups;
  std::vector<size_t> maxWorkGroupSize;

  // A vector of descriptors of OpenCL extensions for each device.
  std::vector<ExtensionsTy> Extensions;
  std::vector<cl_context> CTX;
  std::vector<cl_command_queue> Queues;
  std::vector<FuncOrGblEntryTy> FuncGblEntries;
  std::vector<std::map<void *, BufferInfoTy> > Buffers;
  std::vector<std::map<cl_kernel, std::set<void *> > > ImplicitArgs;
  std::mutex *Mutexes;

  int64_t flag;
  int32_t DataTransferLatency;
  int32_t DataTransferMethod;
  cl_device_type DeviceType;

  // Limit for the number of WIs in a WG.
  int32_t OMPThreadLimit = -1;
  const int64_t DATA_TRANSFER_LATENCY = 0x2;

  RTLDeviceInfoTy() : numDevices(0), flag(0), DataTransferLatency(0),
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

    // Read LIBOMPTARGET_DATA_TRANSFER_LATENCY (experimental input)
    if (env = std::getenv("LIBOMPTARGET_DATA_TRANSFER_LATENCY")) {
      std::string value(env);
      if (value.substr(0, 2) == "T,") {
        flag |= DATA_TRANSFER_LATENCY;
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
  }

  ~RTLDeviceInfoTy() {
    delete[] Mutexes;
  }
};

static RTLDeviceInfoTy DeviceInfo;
static RTLProfileTy profile;

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

#ifdef __cplusplus
extern "C" {
#endif

static inline void addDataTransferLatency() {
  if (!(DeviceInfo.flag & DeviceInfo.DATA_TRANSFER_LATENCY))
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

  DeviceInfo.maxWorkGroups.resize(DeviceInfo.numDevices);
  DeviceInfo.maxWorkGroupSize.resize(DeviceInfo.numDevices);
  DeviceInfo.Extensions.resize(DeviceInfo.numDevices);
  DeviceInfo.CTX.resize(DeviceInfo.numDevices);
  DeviceInfo.Queues.resize(DeviceInfo.numDevices);
  DeviceInfo.FuncGblEntries.resize(DeviceInfo.numDevices);
  DeviceInfo.Buffers.resize(DeviceInfo.numDevices);
  DeviceInfo.ImplicitArgs.resize(DeviceInfo.numDevices);
  DeviceInfo.Mutexes = new std::mutex[DeviceInfo.numDevices];

  // get device specific information
  for (unsigned i = 0; i < DeviceInfo.numDevices; i++) {
    std::vector<char> buf;
    size_t buf_size;
    cl_int rc;
    cl_device_id deviceId = DeviceInfo.deviceIDs[i];
    rc = clGetDeviceInfo(deviceId, CL_DEVICE_NAME, 0, nullptr, &buf_size);
    if (rc != CL_SUCCESS || buf_size == 0)
      continue;
    buf.resize(buf_size);
    rc = clGetDeviceInfo(deviceId, CL_DEVICE_NAME, buf_size, buf.data(),
                         nullptr);
    if (rc != CL_SUCCESS)
      continue;
    DP("Device %d: %s\n", i, buf.data());
    clGetDeviceInfo(deviceId, CL_DEVICE_MAX_COMPUTE_UNITS, 4,
                    &DeviceInfo.maxWorkGroups[i], nullptr);
    DP("Maximum number of work groups (compute units) is %d\n",
       DeviceInfo.maxWorkGroups[i]);
    clGetDeviceInfo(deviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t),
                    &DeviceInfo.maxWorkGroupSize[i], nullptr);
    DP("Maximum work group size is %d\n",
       static_cast<int32_t>(DeviceInfo.maxWorkGroupSize[i]));
#ifdef OMPTARGET_OPENCL_DEBUG
    cl_uint addressmode;
    clGetDeviceInfo(deviceId, CL_DEVICE_ADDRESS_BITS, 4, &addressmode,
                    nullptr);
    DP("Addressing mode is %d bit\n", addressmode);
#endif
  }
  if (DeviceInfo.numDevices == 0)
    DP("WARNING: No OpenCL devices found.\n");

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

  cl_queue_properties qprops[3] = {0};
  if (profile.flags & PROFILE_ENABLED) {
    qprops[0] = CL_QUEUE_PROPERTIES;
    qprops[1] = CL_QUEUE_PROFILING_ENABLE;
  }

  DeviceInfo.Queues[device_id] = clCreateCommandQueueWithProperties(
      DeviceInfo.CTX[device_id], DeviceInfo.deviceIDs[device_id], qprops,
      &status);
  if (status != 0) {
    DP("Error: Failed to create CommandQueue: %d\n", status);
    return OFFLOAD_FAIL;
  }

  DeviceInfo.Extensions[device_id].getExtensionsInfoForDevice(device_id);

  return OFFLOAD_SUCCESS;
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

EXTERN
__tgt_target_table *__tgt_rtl_load_binary(int32_t device_id,
                                          __tgt_device_image *image) {

  DP("Device %d: load binary from " DPxMOD " image\n", device_id,
     DPxPTR(image->ImageStart));

  size_t ImageSize = (size_t)image->ImageEnd - (size_t)image->ImageStart;
  size_t NumEntries = (size_t)(image->EntriesEnd - image->EntriesBegin);
  DP("Expecting to have %zu entries defined.\n", NumEntries);

  // create Program
  cl_int status;
  cl_program program[3];
  cl_uint num_programs = 0;
  const char *compilation_options = "";
#if INTEL_CUSTOMIZATION
  cl_device_type device_type;

  if (clGetDeviceInfo(DeviceInfo.deviceIDs[device_id],
                      CL_DEVICE_TYPE, sizeof(device_type), &device_type,
                      nullptr) == CL_SUCCESS &&
      device_type == CL_DEVICE_TYPE_GPU)
    // OpenCL CPU compiler complains about unsupported option.
    // Intel Graphics compilers that do not support that option
    // silently ignore it.
    compilation_options = "-cl-intel-enable-global-relocation";
#endif // INTEL_CUSTOMIZATION

  DP("OpenCL compilation options: %s\n", compilation_options);

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

    INVOKE_CL_RET_NULL(clCompileProgram, program[1], 0, nullptr,
                       compilation_options, 0, nullptr, nullptr,
                       nullptr, nullptr);
    num_programs++;
  } else {
    DP("Cannot find device RTL: %s\n", device_rtl_path.c_str());
  }

  // Create program for the target regions.
  dumpImageToFile(image->ImageStart, ImageSize, "OpenMP");
  program[0] = clCreateProgramWithIL(DeviceInfo.CTX[device_id],
                                     image->ImageStart, ImageSize, &status);
  if (status != 0) {
    DP("Error: Failed to create program: %d\n", status);
    return NULL;
  }

  INVOKE_CL_RET_NULL(clCompileProgram, program[0], 0, nullptr,
                     compilation_options, 0, nullptr, nullptr,
                     nullptr, nullptr);

  num_programs++;

  if (num_programs < 2)
    DP("Skipped device RTL.\n");

  program[2] = clLinkProgram(
      DeviceInfo.CTX[device_id], 1, &DeviceInfo.deviceIDs[device_id],
      compilation_options, num_programs, &program[0], nullptr, nullptr,
      &status);
  if (status != CL_SUCCESS) {
    DP("Error: Failed to link program: %d\n", status);
    return NULL;
  } else {
    DP("Successfully linked program.\n");
  }

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
#endif  // INTEL_CUSTOMIZATION

  for (unsigned i = 0; i < NumEntries; i++) {
    // Size is 0 means that it is kernel function.
    auto Size = image->EntriesBegin[i].size;

    if (Size != 0) {
#if INTEL_CUSTOMIZATION
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

  __tgt_target_table &table = DeviceInfo.FuncGblEntries[device_id].Table;
  table.EntriesBegin = &(entries[0]);
  table.EntriesEnd = &(entries.data()[entries.size()]);
  return &table;
}

void event_callback_completed(cl_event event, cl_int status, void *data) {
  if (status == CL_SUCCESS) {

    AsyncEventTy *async_event = (AsyncEventTy *)data;
    if (!async_event->handler || !async_event->arg) {
      DP("Error: Invalid asynchronous offloading event\n");
      return;
    }

    cl_command_type cmd;
    clGetEventInfo(event, CL_EVENT_COMMAND_TYPE, sizeof(cmd), &cmd, nullptr);
    if (cmd == CL_COMMAND_READ_BUFFER || cmd == CL_COMMAND_WRITE_BUFFER) {
      if (!async_event->data ||
          clReleaseMemObject((cl_mem)async_event->data) != CL_SUCCESS) {
        DP("Error: Failed to handle asynchronous data operation.\n");
        return;
      }
    }

    if (profile.flags & PROFILE_ENABLED) {
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
      profile.update(event_name, event);
    }

    // Libomptarget is responsible for defining the handler and argument.
    DP("Calling asynchronous offloading event handler " DPxMOD
       " with argument " DPxMOD "\n", DPxPTR(async_event->handler),
       DPxPTR(async_event->arg));
    async_event->handler(async_event->arg);

  } else {
    DP("Error: Failed to complete asynchronous offloading.\n");
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
  cl_device_id id = DeviceInfo.deviceIDs[device_id];

  // Add synthetic delay for experiments
  addDataTransferLatency();

  switch (DeviceInfo.DataTransferMethod) {
  case DATA_TRANSFER_METHOD_SVMMAP: {
    // No asynchronous data copy here since we use map/unmap as explicit
    // synchronization points.
    cl_ulong begin, end, dummy;
    if (profile.flags & PROFILE_ENABLED)
      INVOKE_CL_RET_FAIL(clGetDeviceAndHostTimer, id, &begin, &dummy);

    INVOKE_CL_RET_FAIL(clEnqueueSVMMap, queue, CL_TRUE, CL_MAP_WRITE, tgt_ptr,
                       size, 0, nullptr, nullptr);
    memcpy(tgt_ptr, hst_ptr, size);
    INVOKE_CL_RET_FAIL(clEnqueueSVMUnmap, queue, tgt_ptr, 0, nullptr, nullptr);

    if (profile.flags & PROFILE_ENABLED) {
      INVOKE_CL_RET_FAIL(clGetDeviceAndHostTimer, id, &end, &dummy);
      profile.update("DATA-WRITE", end - begin);
    }
  } break;
  case DATA_TRANSFER_METHOD_SVMMEMCPY: {
    cl_event event;
    if (async_event && ((AsyncEventTy *)async_event)->handler) {
      INVOKE_CL_RET_FAIL(clEnqueueSVMMemcpy, queue, CL_FALSE, tgt_ptr, hst_ptr,
                         size, 0, nullptr, &event);
      INVOKE_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                         &event_callback_completed, async_event);
    } else {
      INVOKE_CL_RET_FAIL(clEnqueueSVMMemcpy, queue, CL_TRUE, tgt_ptr, hst_ptr,
                         size, 0, nullptr, &event);
      if (profile.flags & PROFILE_ENABLED)
        profile.update("DATA-WRITE", event);
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
    if (async_event && ((AsyncEventTy *)async_event)->handler) {
      INVOKE_CL_RET_FAIL(clEnqueueWriteBuffer, queue, mem, CL_FALSE, 0, size,
                         hst_ptr, 0, nullptr, &event);
      ((AsyncEventTy *)async_event)->data = (void *)mem;
      INVOKE_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                         &event_callback_completed, async_event);
    } else {
      INVOKE_CL_RET_FAIL(clEnqueueWriteBuffer, queue, mem, CL_TRUE, 0, size,
                         hst_ptr, 0, nullptr, &event);
      INVOKE_CL_RET_FAIL(clReleaseMemObject, mem);
      if (profile.flags & PROFILE_ENABLED)
        profile.update("DATA-WRITE", event);
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
  cl_device_id id = DeviceInfo.deviceIDs[device_id];

  // Add synthetic delay for experiments
  addDataTransferLatency();

  switch (DeviceInfo.DataTransferMethod) {
  case DATA_TRANSFER_METHOD_SVMMAP: {
    // No asynchronous data copy here since we use map/unmap as explicit
    // synchronization points.
    cl_ulong begin, end, dummy;
    if (profile.flags & PROFILE_ENABLED)
      INVOKE_CL_RET_FAIL(clGetDeviceAndHostTimer, id, &begin, &dummy);

    INVOKE_CL_RET_FAIL(clEnqueueSVMMap, queue, CL_TRUE, CL_MAP_READ, tgt_ptr,
                       size, 0, nullptr, nullptr);
    memcpy(hst_ptr, tgt_ptr, size);
    INVOKE_CL_RET_FAIL(clEnqueueSVMUnmap, queue, tgt_ptr, 0, nullptr, nullptr);

    if (profile.flags & PROFILE_ENABLED) {
      INVOKE_CL_RET_FAIL(clGetDeviceAndHostTimer, id, &end, &dummy);
      profile.update("DATA-READ", end - begin);
    }
  } break;
  case DATA_TRANSFER_METHOD_SVMMEMCPY: {
    cl_event event;
    if (async_event && ((AsyncEventTy *)async_event)->handler) {
      INVOKE_CL_RET_FAIL(clEnqueueSVMMemcpy, queue, CL_FALSE, hst_ptr, tgt_ptr,
                         size, 0, nullptr, &event);
      INVOKE_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                         &event_callback_completed, async_event);
    } else {
      INVOKE_CL_RET_FAIL(clEnqueueSVMMemcpy, queue, CL_TRUE, hst_ptr, tgt_ptr,
                         size, 0, nullptr, &event);
      if (profile.flags & PROFILE_ENABLED)
        profile.update("DATA-READ", event);
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
    if (async_event && ((AsyncEventTy *)async_event)->handler) {
      INVOKE_CL_RET_FAIL(clEnqueueReadBuffer, queue, mem, CL_FALSE, 0, size,
                         hst_ptr, 0, nullptr, &event);
      ((AsyncEventTy *)async_event)->data = (void *)mem;
      INVOKE_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                         &event_callback_completed, async_event);
    } else {
      INVOKE_CL_RET_FAIL(clEnqueueReadBuffer, queue, mem, CL_TRUE, 0, size,
                         hst_ptr, 0, nullptr, &event);
      INVOKE_CL_RET_FAIL(clReleaseMemObject, mem);
      if (profile.flags & PROFILE_ENABLED)
        profile.update("DATA-READ", event);
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
  DP("OpenCL maximum work-group size is %zu.\n", local_work_size_max);
  size_t num_work_groups_max = DeviceInfo.maxWorkGroups[device_id];
  DP("OpenCL maximum number of work-groups is %zu.\n", num_work_groups_max);

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

  if (thread_limit > 0 &&
      (size_t)thread_limit < local_work_size_max) {
    local_work_size_max = (size_t)thread_limit;
    DP("Setting maximum work-group size to %zu (due to thread_limit clause).\n",
       local_work_size_max);
  }

  if (DeviceInfo.OMPThreadLimit > 0 &&
      (size_t)DeviceInfo.OMPThreadLimit < local_work_size_max) {
    local_work_size_max = (size_t)DeviceInfo.OMPThreadLimit;
    DP("Setting maximum work-group size to %zu (due to OMP_THREAD_LIMIT).\n",
       local_work_size_max);
  }

  if (num_teams > 0 &&
      (size_t)num_teams < num_work_groups_max) {
    num_work_groups_max = (size_t)num_teams;
    DP("Setting maximum number of work groups to %zu "
       "(due to num_teams clause).\n", num_work_groups_max);
  }

  int64_t *loop_levels = loop_desc ? (int64_t *)loop_desc : nullptr;
  if (loop_levels && thread_limit <= 0 &&
      (DeviceInfo.OMPThreadLimit <= 0 ||
       // omp_get_thread_limit() would return INT_MAX by default.
       // NOTE: Windows.h defines max() macro, so we have to guard
       //       the call with parentheses.
       DeviceInfo.OMPThreadLimit == (std::numeric_limits<int32_t>::max)()) &&
      local_work_size_max > 16)
    // Default to 16 WIs per WG for ND-range paritioning.
    // This size seems to provide the best results for steam and nbody
    // benchmarks. Users may use more WIs/WG by using thread_limit clause
    // and OMP_THREAD_LIMIT, but the number may not exceed OpenCL limits.
    local_work_size_max = 16;

  // TODO: we may want to reshape local work if necessary.
  size_t local_work_size[3] = {local_work_size_max, 1, 1};
  size_t num_work_groups[3] = {num_work_groups_max, 1, 1};
  cl_uint work_dim = 1;

  // Compute num_work_groups using the loop descriptor.
  if (loop_levels) {
    assert(*loop_levels > 0 && *loop_levels <= 3 &&
           "ND-range parallelization requested "
           "with invalid number of dimensions.");
    assert(num_teams <= 0 &&
           "ND-range parallelization requested with num_teams.");
    TgtLoopDescTy *level = (TgtLoopDescTy *)(loop_levels + 1);

    for (int32_t i = 0; i < *loop_levels; ++i) {
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
    work_dim = *loop_levels;
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
  DP("Work dimension = %u\n", work_dim);

  // Protect thread-unsafe OpenCL API calls
  DeviceInfo.Mutexes[device_id].lock();

  // Set implicit kernel args
  std::vector<void *> implicit_args;
  if (DeviceInfo.ImplicitArgs[device_id].count(*kernel) > 0) {
    // kernel-dependent arguments
    implicit_args.insert(implicit_args.end(),
        DeviceInfo.ImplicitArgs[device_id][*kernel].begin(),
        DeviceInfo.ImplicitArgs[device_id][*kernel].end());
  }
  if (DeviceInfo.ImplicitArgs[device_id].count(0) > 0) {
    // kernel-independent arguments
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

  if (implicit_args.size() > 0) {
    DP("Calling clSetKernelExecInfo to pass %zu implicit arguments to kernel "
       DPxMOD "\n", implicit_args.size(), DPxPTR(kernel));
    INVOKE_CL_RET_FAIL(clSetKernelExecInfo, *kernel,
                       CL_KERNEL_EXEC_INFO_SVM_PTRS,
                       sizeof(void *) * implicit_args.size(),
                       implicit_args.data());
  }

  cl_event event;
  INVOKE_CL_RET_FAIL(clEnqueueNDRangeKernel, DeviceInfo.Queues[device_id],
                     *kernel, work_dim, nullptr, global_work_size,
                     local_work_size, 0, nullptr, &event);

  DeviceInfo.Mutexes[device_id].unlock();

  DP("Started executing kernel.\n");

  if (async_event) {
    if (((AsyncEventTy *)async_event)->handler) {
      // Add event handler if necessary.
      INVOKE_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                         &event_callback_completed, async_event);
    } else {
      // Make sure all queued commands finish before the next one starts.
      INVOKE_CL_RET_FAIL(clEnqueueBarrierWithWaitList,
                         DeviceInfo.Queues[device_id], 0, nullptr, nullptr);
    }
  } else {
    if (profile.flags & PROFILE_ENABLED) {
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
      profile.update(kernel_name.c_str(), event);
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

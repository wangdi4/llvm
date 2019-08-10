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
#include <Windows.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

#include "omptargetplugin.h"

#ifndef TARGET_NAME
#define TARGET_NAME OPENCL
#endif

#define GETNAME2(name) #name
#define GETNAME(name) GETNAME2(name)
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#ifndef USE_SVM_MEMCPY
#define USE_SVM_MEMCPY 0
#endif

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
};

/// Class containing all the device information.
class RTLDeviceInfoTy {

public:
  cl_uint numDevices;
  // per device information
  std::vector<cl_platform_id> platformIDs;
  std::vector<cl_device_id> deviceIDs;
  std::vector<int32_t> maxWorkGroups;
  std::vector<int32_t> maxWorkGroupSize;

  // A vector of descriptors of OpenCL extensions for each device.
  std::vector<ExtensionsTy> Extensions;
  std::vector<cl_context> CTX;
  std::vector<cl_command_queue> Queues;
  std::vector<FuncOrGblEntryTy> FuncGblEntries;
  std::vector<std::map<void *, void *> > BaseBuffers;
  std::vector<std::map<cl_kernel, std::set<void *> > > ImplicitArgs;
  std::mutex *Mutexes;

  int64_t flag;
  int32_t DataTransferLatency;
  const int64_t DEVICE_LIMIT_NUM_WORK_GROUPS = 0x1;
  const int64_t DATA_TRANSFER_LATENCY = 0x2;

  RTLDeviceInfoTy() : numDevices(0), flag(0), DataTransferLatency(0) {
#ifdef OMPTARGET_OPENCL_DEBUG
    if (char *envStr = getenv("LIBOMPTARGET_DEBUG")) {
      DebugLevel = std::stoi(envStr);
    }
#endif // OMPTARGET_OPENCL_DEBUG
    // set misc. flags
    const char *env = std::getenv("SIMT");
    if (!env || std::string(env) != "on") {
      flag |= DEVICE_LIMIT_NUM_WORK_GROUPS;
    }
    // Read LIBOMPTARGET_DATA_TRANSFER_LATENCY (experimental input)
    if ((env = std::getenv("LIBOMPTARGET_DATA_TRANSFER_LATENCY"))) {
      std::string value(env);
      if (value.substr(0, 2) == "T,") {
        flag |= DATA_TRANSFER_LATENCY;
        int32_t usec = std::stoi(value.substr(2).c_str());
        DataTransferLatency = (usec > 0) ? usec : 0;
      }
    }
#if INTEL_CUSTOMIZATION
    bool CheckExtensionGlobalRelocation = false;
    // Check environment variable that enables
    // clGetDeviceGlobalVariablePointerINTEL extension in OpenCL graphics
    // compiler rather than using a separate environment variable.
    // FIXME: remove this, when it is enabled by default in
    //        OpenCL graphics compiler.
    if (const auto *Val = std::getenv("IGC_EnableGlobalRelocation"))
      if (std::stoi(Val) != 0)
        CheckExtensionGlobalRelocation = true;
#endif  // INTEL_CUSTOMIZATION

    DP("Start initializing OpenCL\n");
    // get available platforms
    cl_uint platformIdCount = 0;
    clGetPlatformIDs(0, nullptr, &platformIdCount);
    std::vector<cl_platform_id> platformIds(platformIdCount);
    clGetPlatformIDs(platformIdCount, platformIds.data(), nullptr);

    std::vector<cl_device_id> deviceIDTail;
    std::vector<cl_platform_id> platformIDTail;

    // OpenCL device IDs are stored in a list so that
    // 1. All device IDs from a single platfrom are stored consecutively.
    // 2. Device IDs from a platform having at least one GPU device appears
    //    before any device IDs from a platform having no GPU devices.
    for (cl_platform_id id : platformIds) {
      char buffer[128];
      clGetPlatformInfo(id, CL_PLATFORM_VERSION, 128, buffer, NULL);
      if (strncmp("OpenCL 2", buffer, 8)) {
        continue;
      }

      cl_uint numGPU = 0, numACC = 0, numCPU = 0;
      char *envStr = getenv("LIBOMPTARGET_DEVICETYPE");
      if (!envStr || strncmp(envStr, "GPU", 3) == 0)
        clGetDeviceIDs(id, CL_DEVICE_TYPE_GPU, 0, nullptr, &numGPU);
      if (!envStr || strncmp(envStr, "ACCELERATOR", 11) == 0)
        clGetDeviceIDs(id, CL_DEVICE_TYPE_ACCELERATOR, 0, nullptr, &numACC);
      if (!envStr || strncmp(envStr, "CPU", 3) == 0)
        clGetDeviceIDs(id, CL_DEVICE_TYPE_CPU, 0, nullptr, &numCPU);
      cl_uint numCurrDevices = numGPU + numACC + numCPU;
      if (numCurrDevices == 0)
        continue;

      DP("Platform %s has %d GPUs, %d ACCELERATORS, %d CPUs\n", buffer, numGPU,
         numACC, numCPU);
      std::vector<cl_device_id> currDeviceIDs(numCurrDevices);
      std::vector<cl_platform_id> currPlatformIDs(numCurrDevices, id);
      clGetDeviceIDs(id, CL_DEVICE_TYPE_GPU, numGPU, &currDeviceIDs[0],
                     nullptr);
      clGetDeviceIDs(id, CL_DEVICE_TYPE_ACCELERATOR, numACC,
                     &currDeviceIDs[numGPU], nullptr);
      clGetDeviceIDs(id, CL_DEVICE_TYPE_CPU, numCPU,
                     &currDeviceIDs[numGPU + numACC], nullptr);

      std::vector<cl_device_id> *dID = &deviceIDs;
      std::vector<cl_platform_id> *pID = &platformIDs;
      if (numGPU == 0) {
        dID = &deviceIDTail;
        pID = &platformIDTail;
      }
      dID->insert(dID->end(), currDeviceIDs.begin(), currDeviceIDs.end());
      pID->insert(pID->end(), currPlatformIDs.begin(), currPlatformIDs.end());
      numDevices += numCurrDevices;
    }
    deviceIDs.insert(deviceIDs.end(), deviceIDTail.begin(), deviceIDTail.end());
    platformIDs.insert(platformIDs.end(), platformIDTail.begin(),
                       platformIDTail.end());

    maxWorkGroups.resize(numDevices);
    maxWorkGroupSize.resize(numDevices);
    Extensions.resize(numDevices);
    CTX.resize(numDevices);
    Queues.resize(numDevices);
    FuncGblEntries.resize(numDevices);
    BaseBuffers.resize(numDevices);
    ImplicitArgs.resize(numDevices);
    Mutexes = new std::mutex[numDevices];

    // get device specific information
    for (unsigned i = 0; i < numDevices; i++) {
      char buffer[128];
      cl_device_id deviceId = deviceIDs[i];
      clGetDeviceInfo(deviceId, CL_DEVICE_NAME, 128, buffer, nullptr);
      DP("Device %d: %s\n", i, buffer);
      clGetDeviceInfo(deviceId, CL_DEVICE_MAX_COMPUTE_UNITS, 4,
                      &maxWorkGroups[i], nullptr);
      DP("Maximum number of work groups (compute units) is %d\n",
         maxWorkGroups[i]);
      clGetDeviceInfo(deviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t),
                      &maxWorkGroupSize[i], nullptr);
      DP("Maximum work group size is %d\n", maxWorkGroupSize[i]);
#ifdef OMPTARGET_OPENCL_DEBUG
      cl_uint addressmode;
      clGetDeviceInfo(deviceId, CL_DEVICE_ADDRESS_BITS, 4, &addressmode,
                      nullptr);
      DP("Addressing mode is %d bit\n", addressmode);
#endif
#if INTEL_CUSTOMIZATION
      // Disable GetDeviceGlobalVariablePointer extension, if
      // IGC_EnableGlobalRelocation environment variable is not set.
      // If the extension status is left ExtensionStatusUnknown,
      // then we will query OpenCL in __tgt_rtl_init_device().
      // FIXME: this is a temporary solution until clGetDeviceInfo
      //        supports querying this extension.
      if (!CheckExtensionGlobalRelocation)
        Extensions[i].GetDeviceGlobalVariablePointer = ExtensionStatusDisabled;
#endif  // INTEL_CUSTOMIZATION
    }
    if (numDevices == 0)
      DP("WARNING: No OpenCL devices found.\n");
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

#if _WIN32
__declspec(dllimport) double omp_get_wtime(void);
#else   // !_WIN32
double omp_get_wtime(void) __attribute__((weak));
#endif  // !_WIN32

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
int32_t __tgt_rtl_number_of_devices() { return DeviceInfo.numDevices; } // fixme

EXTERN
int32_t __tgt_rtl_init_device(int32_t device_id) {

  cl_int status;
  DP("Initialize OpenCL device\n");
  assert(device_id >= 0 && (cl_uint)device_id < DeviceInfo.numDevices &&
         "bad device id");

  // create context
  auto PlatformID = DeviceInfo.platformIDs[device_id];
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
#ifndef _WIN32
  if (DebugLevel <= 0)
    return;

  if (!std::getenv("LIBOMPTARGET_SAVE_TEMPS"))
    return;

  char TmpFileName[] = "omptarget_opencl_image_XXXXXX";
  int TmpFileFd = mkstemp(TmpFileName);
  DPI("Dumping %s image of size %d from address " DPxMOD " to file %s\n",
      Type, static_cast<int32_t>(ImageSize), DPxPTR(Image), TmpFileName);

  if (TmpFileFd < 0) {
    DPI("Error creating temporary file: %s\n", strerror(errno));
    return;
  }

  if (write(TmpFileFd, Image, ImageSize) < 0)
    DPI("Error writing temporary file %s: %s\n", TmpFileName, strerror(errno));

  if (close(TmpFileFd) < 0)
    DPI("Error closing temporary file %s: %s\n", TmpFileName, strerror(errno));
#endif  // !_WIN32
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
  DP("Expecting to have %zd entries defined.\n", NumEntries);

  // create Program
  cl_int status;
  cl_program program[3];
  cl_uint num_programs = 0;

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

    program[0] = clCreateProgramWithIL(DeviceInfo.CTX[device_id],
                                       device_rtl_bin.c_str(), device_rtl_len,
                                       &status);
    if (status != CL_SUCCESS) {
      DP("Error: Failed to create device RTL from IL: %d\n", status);
      return NULL;
    }

    INVOKE_CL_RET_NULL(clCompileProgram, program[0], 0, nullptr, nullptr, 0,
                       nullptr, nullptr, nullptr, nullptr);
    num_programs++;
  } else {
    DP("Cannot find device RTL: %s\n", device_rtl_path.c_str());
  }

  // Create program for the target regions.
  dumpImageToFile(image->ImageStart, ImageSize, "OpenMP");
  program[1] = clCreateProgramWithIL(DeviceInfo.CTX[device_id],
                                     image->ImageStart, ImageSize, &status);
  if (status != 0) {
    DP("Error: Failed to create program: %d\n", status);
    return NULL;
  }

  INVOKE_CL_RET_NULL(clCompileProgram, program[1], 0, nullptr, nullptr, 0,
                     nullptr, nullptr, nullptr, nullptr);

  num_programs++;

  if (num_programs < 2)
    DP("Skipped device RTL.\n");

  program[2] = clLinkProgram(
      DeviceInfo.CTX[device_id], 1, &DeviceInfo.deviceIDs[device_id], nullptr,
      num_programs, &program[0], nullptr, nullptr, &status);
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
  for (unsigned i = 0; i < NumEntries; i++) {
    // Size is 0 means that it is kernel function.
    auto Size = image->EntriesBegin[i].size;

    if (Size != 0) {
#if INTEL_CUSTOMIZATION
      void *TgtAddr = nullptr;
      auto HostAddr = image->EntriesBegin[i].addr;
      auto Name = image->EntriesBegin[i].name;

      DP("Looking up global variable '%s' of size %" PRIu64 " bytes\n",
         Name, (uint64_t)Size);

      if (DeviceInfo.Extensions[device_id].GetDeviceGlobalVariablePointer ==
          ExtensionStatusEnabled) {
        // TODO: we should probably query the API pointer once
        //       and keep it in some platform/device descriptor
        //       (e.g. DeviceInfo.Extensions).
        cl_int (CL_API_CALL *ExtCall)(cl_device_id, cl_program, const char *,
                                      size_t *, void **);

        ExtCall = reinterpret_cast<decltype(ExtCall)>(
            clGetExtensionFunctionAddressForPlatform(
                DeviceInfo.platformIDs[device_id],
                "clGetDeviceGlobalVariablePointerINTEL"));
        if (!ExtCall) {
          DPI("Error: clGetDeviceGlobalVariablePointerINTEL API "
              "is nullptr.\n");
          return nullptr;
        }

        size_t DeviceSize = 0;
        INVOKE_CL_RET_NULL(ExtCall,
                           DeviceInfo.deviceIDs[device_id], program[2],
                           Name, &DeviceSize, &TgtAddr);
        if (Size != DeviceSize) {
          DP("Error: size mismatch for host and device versions "
             "of global variable: %s\n", Name);
          return nullptr;
        }
      } else {
        // Allocate buffers for global data and copy data from host to device.
        // FIXME: this is a temporary solution for global declare target data,
        //        until we have support from OpenCL side.
        //        This will not solve issues for the following case:
        //          #pragma omp declare target
        //          int a[100];
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
        TgtAddr = __tgt_rtl_data_alloc(device_id, Size, HostAddr);
        __tgt_rtl_data_submit(device_id, TgtAddr, HostAddr, Size);
      }

      DP("Global variable allocated: Name = %s, Size = %" PRIu64
         ", HostPtr = " DPxMOD ", TgtPtr = " DPxMOD "\n",
         Name, (uint64_t)Size, DPxPTR(HostAddr), DPxPTR(TgtAddr));
      entries[i].addr = TgtAddr;
      entries[i].name = Name;
      entries[i].size = Size;
#endif  // INTEL_CUSTOMIZATION
      continue;
    }

    char *name = image->EntriesBegin[i].name;
    kernels[i] = clCreateKernel(program[2], name, &status);
    if (status != 0) {
      DP("Error: Failed to create kernel %s, %d\n", name, status);
      return NULL;
    }
    entries[i].addr = &kernels[i];
    entries[i].name = name;
#ifdef OMPTARGET_OPENCL_DEBUG
    // Show kernel information
    char kernel_info[80];
    cl_uint kernel_num_args = 0;
    cl_int rc;
    rc = clGetKernelInfo(kernels[i], CL_KERNEL_FUNCTION_NAME,
                         sizeof(kernel_info), kernel_info, nullptr);
    if (rc != CL_SUCCESS)
      continue;
    rc = clGetKernelInfo(kernels[i], CL_KERNEL_NUM_ARGS,
                         sizeof(cl_uint), &kernel_num_args, nullptr);
    if (rc != CL_SUCCESS)
      continue;
    DP("Kernel %d: Name = %s, NumArgs = %d\n", i, kernel_info, kernel_num_args);
    for (unsigned idx = 0; idx < kernel_num_args; idx++) {
      clGetKernelArgInfo(kernels[i], idx, CL_KERNEL_ARG_TYPE_NAME, 40,
                         kernel_info, nullptr);
      clGetKernelArgInfo(kernels[i], idx, CL_KERNEL_ARG_NAME, 40,
                         &kernel_info[40], nullptr);
      DP("  Arg %2d: %s %s\n", idx, kernel_info, &kernel_info[40]);
    }
#endif // OMPTARGET_OPENCL_DEBUG
  }

  __tgt_target_table &table = DeviceInfo.FuncGblEntries[device_id].Table;
  table.EntriesBegin = &(entries[0]);
  table.EntriesEnd = &(entries[entries.size()]);
  return &table;
}

void event_callback_completed(cl_event event, cl_int status, void *data) {
  if (status == CL_SUCCESS) {

    AsyncEventTy *async_event = (AsyncEventTy *)data;
    if (!async_event->handler || !async_event->arg) {
      DP("Error: Invalid asynchronous offloading event\n");
      return;
    }

    if (profile.flags & PROFILE_ENABLED) {
      cl_command_type cmd;
      const char *event_name;
      clGetEventInfo(event, CL_EVENT_COMMAND_TYPE, sizeof(cmd), &cmd, nullptr);
      switch (cmd) {
      case CL_COMMAND_NDRANGE_KERNEL:
        event_name = "EXEC-ASYNC";
        break;
      case CL_COMMAND_READ_BUFFER:
        event_name = "DATA-READ-ASYNC";
        break;
      case CL_COMMAND_WRITE_BUFFER:
        event_name = "DATA-WRITE-ASYNC";
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
  if (offset < 0) {
    DP("Error: Failed to create base buffer due to invalid array section\n");
    return nullptr;
  }
  void *base = clSVMAlloc(DeviceInfo.CTX[device_id], CL_MEM_READ_WRITE,
                          size + offset, 0);
  if (!base) {
    DP("Error: Failed to allocate base buffer\n");
    return nullptr;
  }
  DP("Created base buffer " DPxMOD " during data alloc\n", DPxPTR(base));

  void *ret = (void *)((intptr_t)base + offset);

  // Store base pointer if returning something else
  if (offset != 0)
    DeviceInfo.BaseBuffers[device_id][ret] = base;

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

#if USE_SVM_MEMCPY
  cl_event event;
  if (async_event) {
    INVOKE_CL_RET_FAIL(clEnqueueSVMMemcpy, queue, CL_FALSE, tgt_ptr, hst_ptr,
                       size, 0, nullptr, &event);
    if (((AsyncEventTy *)async_event)->handler) {
      // Add event handler if necessary.
      INVOKE_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                         &event_callback_completed, async_event);
    } else {
      // Make sure all queued commands finish before the next one starts.
      INVOKE_CL_RET_FAIL(clEnqueueBarrierWithWaitList, queue, 0, nullptr,
                         nullptr);
    }
  } else {
    INVOKE_CL_RET_FAIL(clEnqueueSVMMemcpy, queue, CL_TRUE, tgt_ptr, hst_ptr,
                       size, 0, nullptr, &event);
    if (profile.flags & PROFILE_ENABLED)
      profile.update("DATA-WRITE", event);
  }
#else
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
#endif
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

#if USE_SVM_MEMCPY
  cl_event event;
  if (async_event) {
    INVOKE_CL_RET_FAIL(clEnqueueSVMMemcpy, queue, CL_FALSE, hst_ptr, tgt_ptr,
                       size, 0, nullptr, &event);
    if (((AsyncEventTy *)async_event)->handler) {
      // Add event handler if necessary.
      INVOKE_CL_RET_FAIL(clSetEventCallback, event, CL_COMPLETE,
                         &event_callback_completed, async_event);
    } else {
      // Make sure all queued commands finish before the next one starts.
      INVOKE_CL_RET_FAIL(clEnqueueBarrierWithWaitList, queue, 0, nullptr,
                         nullptr);
    }
  } else {
    INVOKE_CL_RET_FAIL(clEnqueueSVMMemcpy, queue, CL_TRUE, hst_ptr, tgt_ptr,
                       size, 0, nullptr, &event);
    if (profile.flags & PROFILE_ENABLED)
      profile.update("DATA-READ", event);
  }
#else
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
#endif
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
  std::map<void *, void *> &bases = DeviceInfo.BaseBuffers[device_id];
  void *base = tgt_ptr;
  auto I = bases.find(tgt_ptr);
  if (I != bases.end())
    bases.erase(I);

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
  size_t num_work_groups_max = DeviceInfo.maxWorkGroups[device_id];

  assert(num_teams >= 0 && "negative num_teams!");
  assert(thread_limit >= 0 && "negative thread_limit!");

  if (thread_limit)
    local_work_size_max = MIN((size_t)thread_limit, local_work_size_max);

  if (num_teams)
    num_work_groups_max = MIN((size_t)num_teams, num_work_groups_max);

  // TODO: we may want to reshape local work if necessary.
  size_t local_work_size[3] = {local_work_size_max, 1, 1};
  size_t num_work_groups[3] = {num_work_groups_max, 1, 1};
  cl_uint work_dim = 1;

  int64_t *loop_levels = (int64_t *)loop_desc;
  // Compute num_work_groups using the loop info
  if (!num_teams && loop_levels) {
    TgtLoopDescTy *level = (TgtLoopDescTy *)(loop_levels + 1);
    size_t num_work_groups_total = 1;
    // TODO: check if we need to reverse this loop.
    for (int32_t i = 0; i < *loop_levels; ++i) {
      assert(level[i].ub > level[i].lb && level[i].stride > 0);
      int64_t trip = (level[i].ub - level[i].lb) / level[i].stride + 1;
      num_work_groups[i] = (trip - 1) / local_work_size[i] + 1;
      num_work_groups_total *= num_work_groups[i];
    }
    if ((DeviceInfo.flag & DeviceInfo.DEVICE_LIMIT_NUM_WORK_GROUPS) &&
        num_work_groups_total > num_work_groups_max) {
      num_work_groups[0] = num_work_groups_max;
      num_work_groups[1] = 1;
      num_work_groups[2] = 1;
    }
    work_dim = *loop_levels;
  }

  size_t global_work_size[3];
  for (int32_t i = 0; i < 3; ++i)
    global_work_size[i] = local_work_size[i] * num_work_groups[i];

  DP("THREAD_LIMIT = %d, NUM_TEAMS = %d\n", thread_limit, num_teams);
  if (loop_levels) {
    DP("Collapsed %ld loops.\n", *loop_levels);
  }
  DP("Global work size = (%zd, %zd, %zd)\n", global_work_size[0],
     global_work_size[1], global_work_size[2]);
  DP("Local work size = (%zd, %zd, %zd)\n", local_work_size[0],
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
    DP("Calling clSetKernelExecInfo to pass %zd implicit arguments to kernel "
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
      char buf[80];
      INVOKE_CL_RET_FAIL(clWaitForEvents, 1, &event);
      INVOKE_CL_RET_FAIL(clGetKernelInfo, *kernel, CL_KERNEL_FUNCTION_NAME,
                         sizeof(buf), buf, nullptr);
      std::string kernel_name("EXEC-");
      kernel_name += buf;
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
  // TODO: convert loop_tripcount to loop descriptor
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

#ifdef __cplusplus
}
#endif
#endif // INTEL_COLLAB

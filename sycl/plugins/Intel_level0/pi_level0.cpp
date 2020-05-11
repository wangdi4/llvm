//
// TODO: Refactor this code for upstream, taking these general guidelines
//       into consideration:
//
// 1. Use more C++ where it makes sense, e.g. constructors/destructors or
//    encapsulating some functionalities of PI objects.
// 2. Settle/follow a convention for naming L0/PI handles.
// 3. Address TODO comments in the code.
// 4. Cover PI API with unittests
//
#include "pi_level0.hpp"
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <cstdarg>
#include <cstdio>

#include <level_zero/zet_api.h>

// Controls L0 calls serialization to w/a of L0 driver being not MT ready.
// Recognized values (can be used as a bit mask):
enum {
  ZE_SERIALIZE_NONE  = 0, // no locking or blocking (except when SYCL RT requested blocking)
  ZE_SERIALIZE_LOCK  = 1, // locking around each ZE_CALL
  ZE_SERIALIZE_BLOCK = 2, // blocking ZE calls, where supported (usually in enqueue commands)
};
pi_uint32 ZE_SERIALIZE = 0;

// This class encapsulates actions taken along with a call to L0 API.
class ZeCall {
private:
  // The global mutex that is used for total serialization of L0 calls.
  static std::mutex globalLock;

public:

  ZeCall() {
    if ((ZE_SERIALIZE & ZE_SERIALIZE_LOCK) != 0) {
      globalLock.lock();
    }
  }
  ~ZeCall() {
    if ((ZE_SERIALIZE & ZE_SERIALIZE_LOCK) != 0) {
      globalLock.unlock();
    }
  }

  static ze_result_t check(ze_result_t ze_result, const char *call_str, bool trace_error = true);

  // The non-static version just calls static one.
  ze_result_t checkthis(ze_result_t ze_result, const char *call_str, bool trace_error = true) {
    return ZeCall::check(ze_result, call_str, trace_error);
  }
};
std::mutex ZeCall::globalLock;

// Controls L0 calls tracing in zePrint.
bool ZE_DEBUG = false;

static void zePrint(const char *format, ... ) {
  if (ZE_DEBUG) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
  }
}

// TODO:: In the following 4 methods we may want to distinguish read access vs.
// write (as it is OK for multiple threads to read the map without locking it).

pi_result _pi_mem::addMapping(void *MappedTo, size_t offset, size_t size) {
  std::lock_guard<std::mutex> lock(MappingsMutex);
  auto it = Mappings.find(MappedTo);
  if (it != Mappings.end()) {
    zePrint("piEnqueueMemBufferMap: duplicate mapping detected\n");
    return PI_INVALID_OPERATION;
  } else {
    Mappings.insert({MappedTo, {offset, size}});
  }
  return PI_SUCCESS;
}

pi_result _pi_mem::removeMapping(void *MappedTo, mapping& map_info) {
  std::lock_guard<std::mutex> lock(MappingsMutex);
  auto it = Mappings.find(MappedTo);
  if (it == Mappings.end()) {
    zePrint("piEnqueueMemUnmap: unknown memory mapping\n");
    return PI_INVALID_VALUE;
  }
  map_info = it->second;
  Mappings.erase(it);
  return PI_SUCCESS;
}

ze_result_t
_pi_context::getFreeSlotInExistingOrNewPool(ze_event_pool_handle_t &pool,
                                            size_t &index) {
  // Maximum number of events that can be present in an event pool is captured
  // here Setting it to 256 gave best possible performance for several
  // benchmarks
  static const char *getEnv =
      std::getenv("MAX_NUMBER_OF_EVENTS_PER_EVENT_POOL");
  static const pi_uint32 MaxNumEventsPerPool =
      (getEnv) ? std::atoi(getEnv) : 256;

  index = 0;
  // Create one event pool per MaxNumEventsPerPool events
  if ((L0EventPool == nullptr) ||
      (NumEventsAvailableInEventPool[L0EventPool] == 0)) {
    // Creation of the new pool with record in NumEventsAvailableInEventPool and
    // initialization of the record in NumEventsLiveInEventPool must be done
    // atomically. Otherwise it is possible that decrementAliveEventsInPool will
    // be called for the record in NumEventsLiveInEventPool before its
    // initialization.
    std::lock(NumEventsAvailableInEventPoolMutex,
              NumEventsLiveInEventPoolMutex);
    std::lock_guard<std::mutex> NumEventsAvailableInEventPoolGuard(
        NumEventsAvailableInEventPoolMutex, std::adopt_lock);
    std::lock_guard<std::mutex> NumEventsLiveInEventPoolGuard(
        NumEventsLiveInEventPoolMutex, std::adopt_lock);

    ze_event_pool_desc_t ze_event_pool_desc;
    ze_event_pool_desc.count = MaxNumEventsPerPool;
    ze_event_pool_desc.flags = ZE_EVENT_POOL_FLAG_TIMESTAMP;
    ze_event_pool_desc.version = ZE_EVENT_POOL_DESC_VERSION_CURRENT;

    ze_device_handle_t ze_device = Device->L0Device;
    if (ze_result_t res =
            zeEventPoolCreate(Device->Platform->L0Driver, &ze_event_pool_desc,
                              1, &ze_device, &L0EventPool))
      return res;
    NumEventsAvailableInEventPool[L0EventPool] = MaxNumEventsPerPool - 1;
    NumEventsLiveInEventPool[L0EventPool] = MaxNumEventsPerPool;
  } else {
    std::lock_guard<std::mutex> NumEventsAvailableInEventPoolGuard(
        NumEventsAvailableInEventPoolMutex);
    index = MaxNumEventsPerPool - NumEventsAvailableInEventPool[L0EventPool];
    --NumEventsAvailableInEventPool[L0EventPool];
  }
  pool = L0EventPool;
  return ZE_RESULT_SUCCESS;
}

ze_result_t _pi_context::decrementAliveEventsInPool(ze_event_pool_handle_t pool) {
  std::lock_guard<std::mutex> lock(NumEventsLiveInEventPoolMutex);
  --NumEventsLiveInEventPool[pool];
  if (NumEventsLiveInEventPool[pool] == 0) {
    return zeEventPoolDestroy(pool);
  }
  return ZE_RESULT_SUCCESS;
}

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// Some opencl extensions we know are supported by all Level0 devices.
#define L0_SUPPORTED_EXTENSIONS                                                \
  "cl_khr_il_program cl_khr_subgroups cl_intel_subgroups "                     \
  "cl_intel_subgroups_short cl_intel_required_subgroup_size "

// Map L0 runtime error code to PI error code
static pi_result mapError(ze_result_t result) {
  // TODO: these mapping need to be clarified and synced with the PI API return
  // values, which is TBD.
  switch (result) {
  case ZE_RESULT_SUCCESS:
    return PI_SUCCESS;
  case ZE_RESULT_ERROR_DEVICE_LOST:
    return PI_DEVICE_NOT_FOUND;
  case ZE_RESULT_ERROR_INSUFFICIENT_PERMISSIONS:
    return PI_INVALID_OPERATION;
  case ZE_RESULT_ERROR_NOT_AVAILABLE:
    return PI_INVALID_OPERATION;
  case ZE_RESULT_ERROR_UNINITIALIZED:
    return PI_INVALID_PLATFORM;
  case ZE_RESULT_ERROR_INVALID_ARGUMENT:
    return PI_INVALID_VALUE;
  case ZE_RESULT_ERROR_INVALID_NULL_POINTER:
    return PI_INVALID_VALUE;
  case ZE_RESULT_ERROR_INVALID_SIZE:
    return PI_INVALID_VALUE;
  case ZE_RESULT_ERROR_UNSUPPORTED_SIZE:
    return PI_INVALID_VALUE;
  case ZE_RESULT_ERROR_UNSUPPORTED_ALIGNMENT:
    return PI_INVALID_VALUE;
  case ZE_RESULT_ERROR_INVALID_SYNCHRONIZATION_OBJECT:
    return PI_INVALID_EVENT;
  case ZE_RESULT_ERROR_INVALID_ENUMERATION:
    return PI_INVALID_VALUE;
  case ZE_RESULT_ERROR_UNSUPPORTED_ENUMERATION:
    return PI_INVALID_VALUE;
  case ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT:
    return PI_INVALID_VALUE;
  case ZE_RESULT_ERROR_INVALID_NATIVE_BINARY:
    return PI_INVALID_BINARY;
  case ZE_RESULT_ERROR_INVALID_KERNEL_NAME:
    return PI_INVALID_KERNEL_NAME;
  case ZE_RESULT_ERROR_INVALID_FUNCTION_NAME:
    return PI_BUILD_PROGRAM_FAILURE;
  case ZE_RESULT_ERROR_OVERLAPPING_REGIONS:
    return PI_INVALID_OPERATION;
  default:
    return PI_ERROR_UNKNOWN;
  }
}

// Forward declarations
static pi_result enqueueMemCopyHelper(
  pi_command_type    command_type,
  pi_queue           queue,
  void *             dst,
  pi_bool            blocking_write,
  size_t             size,
  const void *       src,
  pi_uint32          num_events_in_wait_list,
  const pi_event *   event_wait_list,
  pi_event *         event);

static pi_result enqueueMemCopyRectHelper(
  pi_command_type    command_type,
  pi_queue           queue,
  void *             src_buffer,
  void *             dst_buffer,
  const size_t *     src_origin,
  const size_t *     dst_origin,
  const size_t *     region,
  size_t             src_row_pitch,
  size_t             src_slice_pitch,
  size_t             dst_row_pitch,
  size_t             dst_slice_pitch,
  pi_bool            blocking,
  pi_uint32          num_events_in_wait_list,
  const pi_event *   event_wait_list,
  pi_event *         event);

inline void zeParseError(ze_result_t error, std::string &errorString)
{
  switch (error) {
  case ZE_RESULT_SUCCESS:
    errorString = "ZE_RESULT_SUCCESS";
    break;
  case ZE_RESULT_NOT_READY:
    errorString = "ZE_RESULT_NOT_READY";
    break;
  case ZE_RESULT_ERROR_DEVICE_LOST:
    errorString = "ZE_RESULT_ERROR_DEVICE_LOST";
    break;
  case ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY:
    errorString = "ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY";
    break;
  case ZE_RESULT_ERROR_OUT_OF_DEVICE_MEMORY:
    errorString = "ZE_RESULT_ERROR_OUT_OF_DEVICE_MEMORY";
    break;
  case ZE_RESULT_ERROR_MODULE_BUILD_FAILURE:
    errorString = "ZE_RESULT_ERROR_MODULE_BUILD_FAILURE";
    break;
  case ZE_RESULT_ERROR_INSUFFICIENT_PERMISSIONS:
    errorString = "ZE_RESULT_ERROR_INSUFFICIENT_PERMISSIONS";
    break;
  case ZE_RESULT_ERROR_NOT_AVAILABLE:
    errorString = "ZE_RESULT_ERROR_NOT_AVAILABLE";
    break;
  case ZE_RESULT_ERROR_UNINITIALIZED:
    errorString = "ZE_RESULT_ERROR_UNINITIALIZED";
    break;
  case ZE_RESULT_ERROR_UNSUPPORTED_VERSION:
    errorString = "ZE_RESULT_ERROR_UNSUPPORTED_VERSION";
    break;
  case ZE_RESULT_ERROR_UNSUPPORTED_FEATURE:
    errorString = "ZE_RESULT_ERROR_UNSUPPORTED_FEATURE";
    break;
  case ZE_RESULT_ERROR_INVALID_ARGUMENT:
    errorString = "ZE_RESULT_ERROR_INVALID_ARGUMENT";
    break;
  case ZE_RESULT_ERROR_INVALID_NULL_HANDLE:
    errorString = "ZE_RESULT_ERROR_INVALID_NULL_HANDLE";
    break;
  case ZE_RESULT_ERROR_HANDLE_OBJECT_IN_USE:
    errorString = "ZE_RESULT_ERROR_HANDLE_OBJECT_IN_USE";
    break;
  case ZE_RESULT_ERROR_INVALID_NULL_POINTER:
    errorString = "ZE_RESULT_ERROR_INVALID_NULL_POINTER";
    break;
  case ZE_RESULT_ERROR_INVALID_SIZE:
    errorString = "ZE_RESULT_ERROR_INVALID_SIZE";
    break;
  case ZE_RESULT_ERROR_UNSUPPORTED_SIZE:
    errorString = "ZE_RESULT_ERROR_UNSUPPORTED_SIZE";
    break;
  case ZE_RESULT_ERROR_UNSUPPORTED_ALIGNMENT:
    errorString = "ZE_RESULT_ERROR_UNSUPPORTED_ALIGNMENT";
    break;
  case ZE_RESULT_ERROR_INVALID_SYNCHRONIZATION_OBJECT:
    errorString = "ZE_RESULT_ERROR_INVALID_SYNCHRONIZATION_OBJECT";
    break;
  case ZE_RESULT_ERROR_INVALID_ENUMERATION:
    errorString = "ZE_RESULT_ERROR_INVALID_ENUMERATION";
    break;
  case ZE_RESULT_ERROR_UNSUPPORTED_ENUMERATION:
    errorString = "ZE_RESULT_ERROR_UNSUPPORTED_ENUMERATION";
    break;
  case ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT:
    errorString = "ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT";
    break;
  case ZE_RESULT_ERROR_INVALID_NATIVE_BINARY:
    errorString = "ZE_RESULT_ERROR_INVALID_NATIVE_BINARY";
    break;
  case ZE_RESULT_ERROR_INVALID_GLOBAL_NAME:
    errorString = "ZE_RESULT_ERROR_INVALID_GLOBAL_NAME";
    break;
  case ZE_RESULT_ERROR_INVALID_KERNEL_NAME:
    errorString = "ZE_RESULT_ERROR_INVALID_KERNEL_NAME";
    break;
  case ZE_RESULT_ERROR_INVALID_FUNCTION_NAME:
    errorString = "ZE_RESULT_ERROR_INVALID_FUNCTION_NAME";
    break;
  case ZE_RESULT_ERROR_INVALID_GROUP_SIZE_DIMENSION:
    errorString = "ZE_RESULT_ERROR_INVALID_GROUP_SIZE_DIMENSION";
    break;
  case ZE_RESULT_ERROR_INVALID_GLOBAL_WIDTH_DIMENSION:
    errorString = "ZE_RESULT_ERROR_INVALID_GLOBAL_WIDTH_DIMENSION";
    break;
  case ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX:
    errorString = "ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX";
    break;
  case ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE:
    errorString = "ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE";
    break;
  case ZE_RESULT_ERROR_INVALID_KERNEL_ATTRIBUTE_VALUE:
    errorString = "ZE_RESULT_ERROR_INVALID_KERNEL_ATTRIBUTE_VALUE";
    break;
  case ZE_RESULT_ERROR_INVALID_COMMAND_LIST_TYPE:
    errorString = "ZE_RESULT_ERROR_INVALID_COMMAND_LIST_TYPE";
    break;
  case ZE_RESULT_ERROR_OVERLAPPING_REGIONS:
    errorString = "ZE_RESULT_ERROR_OVERLAPPING_REGIONS";
    break;
  case ZE_RESULT_ERROR_UNKNOWN:
    errorString = "ZE_RESULT_ERROR_UNKNOWN";
    break;
  default:
    assert("Unexpected error code");
  }
}

ze_result_t ZeCall::check(ze_result_t ze_result, const char *call_str, bool trace_error) {
  zePrint("ZE ---> %s\n", call_str);

  if (ze_result && trace_error) {
    std::string errorString;
    zeParseError(ze_result, errorString);
    zePrint("Error (%s) in %s\n", errorString.c_str(), call_str);
  }
  return ze_result;
}

#define ZE_CALL(call)                                                          \
  if (auto result = ZeCall().checkthis(call, #call, true))                     \
    return mapError(result);
#define ZE_CALL_NOCHECK(call) ZeCall().checkthis(call, #call, false)

// Crate a new command list to be used in a PI call
pi_result
_pi_device::createCommandList(ze_command_list_handle_t *ze_command_list) {
  // Create the command list, because in L0 commands are added to
  // the command lists, and later are then added to the command queue.
  //
  // TODO: Fugire out how to lower the overhead of creating a new list
  // for each PI command, if that appears to be important.
  //
  ze_command_list_desc_t ze_command_list_desc = {};
  ze_command_list_desc.version = ZE_COMMAND_LIST_DESC_VERSION_CURRENT;

  // TODO: can we just reset the command-list created when an earlier
  // command was submitted to the queue?
  //
  ZE_CALL(zeCommandListCreate(
    L0Device,
    &ze_command_list_desc,
    ze_command_list));

  return PI_SUCCESS;
}

pi_result
_pi_queue::executeCommandList(ze_command_list_handle_t ze_command_list,
                              bool is_blocking) {
  // Close the command list and have it ready for dispatch.
  ZE_CALL(zeCommandListClose(ze_command_list));
  // Offload command list to the GPU for asynchronous execution
  ZE_CALL(zeCommandQueueExecuteCommandLists(
    L0CommandQueue, 1, &ze_command_list, nullptr));

  // Check global control to make every command blocking for debugging.
  if (is_blocking || (ZE_SERIALIZE & ZE_SERIALIZE_BLOCK) != 0) {
    // Wait until command lists attached to the command queue are executed.
    ZE_CALL(zeCommandQueueSynchronize(L0CommandQueue, UINT32_MAX));
  }
  return PI_SUCCESS;
}

ze_event_handle_t * _pi_event::createL0EventList(
  pi_uint32         event_list_length,
  const pi_event *  event_list)
{
  ze_event_handle_t * ze_event_list =
    new ze_event_handle_t[event_list_length];

  for (pi_uint32 i = 0; i < event_list_length; i++) {
    ze_event_list[i] = event_list[i]->L0Event;
  }
  return ze_event_list;
}

void _pi_event::deleteL0EventList(
  ze_event_handle_t *ze_event_list)
{
  delete[] ze_event_list;
}

// TODO: remove this
#define L0(pi_api) pi_api

// Forward declararitons
decltype(piEventCreate) L0(piEventCreate);

// No generic lambdas in C++11, so use this convinence macro.
// NOTE: to be used in API returning "param_value".
// NOTE: memset is used to clear all bytes in the memory allocated by SYCL RT
// for value. This is a workaround for the problem when return type of the
// parameter is incorrect in L0 plugin which can result in bad value. This
// memset can be removed if it is necessary.
#define SET_PARAM_VALUE(value)                                                 \
  {                                                                            \
    typedef decltype(value) T;                                                 \
    if (param_value) {                                                         \
      memset(param_value, 0, param_value_size);                                \
      *(T *)param_value = value;                                               \
    }                                                                          \
    if (param_value_size_ret)                                                  \
      *param_value_size_ret = sizeof(T);                                       \
  }
#define SET_PARAM_VALUE_STR(value)                                             \
  {                                                                            \
    if (param_value)                                                           \
      memcpy(param_value, value, param_value_size);                            \
    if (param_value_size_ret)                                                  \
      *param_value_size_ret = strlen(value) + 1;                               \
  }

#define SET_PARAM_VALUE_VLA(value, num_values, ret_type)                       \
  {                                                                            \
    if (param_value) {                                                         \
      memset(param_value, 0, param_value_size);                                \
      for (uint32_t i = 0; i < num_values; i++)                                \
        ((ret_type *)param_value)[i] = (ret_type)value[i];                     \
    }                                                                          \
    if (param_value_size_ret)                                                  \
      *param_value_size_ret = num_values * sizeof(ret_type);                   \
  }


#ifndef _WIN32
// Recover from Linux SIGSEGV signal.
// We can't reliably catch C++ exceptions thrown from signal
// handler so use setjmp/longjmp.
//
#include <signal.h>
#include <setjmp.h>
jmp_buf return_here;
static void piSignalHandler(int signum) {
  // We are somewhere the signall was raised, so go back to
  // where we started tracking.
  longjmp (return_here, 0);
}
// Only handle segfault now, but can be extended.
#define __TRY()                       \
  signal(SIGSEGV, &piSignalHandler);  \
  if (!setjmp(return_here)) {
#define __CATCH()                     \
  } else {
#define __FINALLY()                   \
  } signal(SIGSEGV, SIG_DFL);

#else // _WIN32
  // TODO: on Windows we could use structured exception handling.
  // Just dummy implementation now (meaning no error handling).
#define __TRY()     if (true) {
#define __CATCH()   } else {
#define __FINALLY() }
#endif // _WIN32


pi_result L0(piPlatformsGet)(pi_uint32       num_entries,
                             pi_platform *   platforms,
                             pi_uint32 *     num_platforms) {

  static const char *getEnv1 = std::getenv("ZE_DEBUG");
  if (getEnv1)
    ZE_DEBUG = true;

  static const char *getEnv2 = std::getenv("ZE_SERIALIZE");
  static const pi_uint32 valEnv2 = getEnv2 ? std::atoi(getEnv2) : 0;
  ZE_SERIALIZE = valEnv2;

  if (num_entries == 0 && platforms != nullptr) {
    return PI_INVALID_VALUE;
  }
  if (platforms == nullptr && num_platforms == nullptr) {
    return PI_INVALID_VALUE;
  }

  ze_result_t ze_result;
  // This is a good time to initialize L0.
  // We can still safely recover if something goes wrong during the init.
  //
  // NOTE: for some reason only first segfault is reliably handled,
  // so remember it, and avoid calling zeInit again.
  //
  // TODO: we should not call zeInit multiples times ever, so
  // this code should be changed.
  //
  static bool segfault = false;
  __TRY() {
    ze_result = segfault ? ZE_RESULT_ERROR_UNINITIALIZED :
        ZE_CALL_NOCHECK(zeInit(ZE_INIT_FLAG_NONE));
  }
  __CATCH() {
    segfault = true;
    zePrint("L0 raised segfault: assume no platforms\n");
    ze_result = ZE_RESULT_ERROR_UNINITIALIZED;
  }
  __FINALLY()

  // Absorb the ZE_RESULT_ERROR_UNINITIALIZED and just return 0 platforms.
  if (ze_result == ZE_RESULT_ERROR_UNINITIALIZED) {
    assert(num_platforms != 0);
    *num_platforms = 0;
    return PI_SUCCESS;
  } else if (auto res = ZeCall::check(ze_result, "zeInit")) {
    return mapError(res);
  }

  // L0 does not have concept of platforms, but L0 driver is the
  // closest match.
  //
  if (platforms && num_entries > 0) {
    uint32_t ze_driver_count = 0;
    ZE_CALL(zeDriverGet(&ze_driver_count, nullptr));
    if (ze_driver_count == 0) {
      assert(num_platforms != 0);
      *num_platforms = 0;
      return PI_SUCCESS;
    }

    ze_driver_handle_t ze_driver;
    assert(ze_driver_count == 1);
    ZE_CALL(zeDriverGet(&ze_driver_count, &ze_driver));

    // TODO: figure out how/when to release this memory
    *platforms = new _pi_platform(ze_driver);

    // Cache driver properties
    ze_driver_properties_t ze_driver_properties;
    ZE_CALL(zeDriverGetProperties(ze_driver, &ze_driver_properties));
    uint32_t ze_driver_version = ze_driver_properties.driverVersion;
    // Intel Level-Zero GPU driver stores version as:
    // | 31 - 24 | 23 - 16 | 15 - 0 |
    // |  Major  |  Minor  | Build  |
    std::string versionMajor = std::to_string((ze_driver_version & 0xFF000000) >> 24);
    std::string versionMinor = std::to_string((ze_driver_version & 0x00FF0000) >> 16);
    std::string versionBuild = std::to_string(ze_driver_version & 0x0000FFFF);
    platforms[0]->L0DriverVersion = versionMajor +
                                 std::string(".") +
                                 versionMinor +
                                 std::string(".") +
                                 versionBuild;

    ze_api_version_t ze_api_version;
    ZE_CALL(zeDriverGetApiVersion(ze_driver, &ze_api_version));
    platforms[0]->L0DriverApiVersion = std::to_string(ZE_MAJOR_VERSION(ze_api_version)) +
                                    std::string(".") +
                                    std::to_string(ZE_MINOR_VERSION(ze_api_version));
  }

  if (num_platforms)
    *num_platforms = 1;

  return PI_SUCCESS;
}

pi_result L0(piPlatformGetInfo)(
  pi_platform       platform,
  pi_platform_info  param_name,
  size_t            param_value_size,
  void *            param_value,
  size_t *          param_value_size_ret) {

  assert(platform);
  zePrint("==========================\n");
  zePrint("SYCL over Level-Zero %s\n", platform->L0DriverVersion.c_str());
  zePrint("==========================\n");

  switch (param_name) {
  case PI_PLATFORM_INFO_NAME:
    // TODO: Query L0 driver when relevant info is added there.
    SET_PARAM_VALUE_STR("Intel(R) Level-Zero");
    break;
  case PI_PLATFORM_INFO_VENDOR:
    // TODO: Query L0 driver when relevant info is added there.
    SET_PARAM_VALUE_STR("Intel(R) Corporation");
    break;
  case PI_PLATFORM_INFO_EXTENSIONS:
    // Convention adopted from OpenCL:
    //     "Returns a space-separated list of extension names (the extension
    // names themselves do not contain any spaces) supported by the platform.
    // Extensions defined here must be supported by all devices associated
    // with this platform."
    //
    // TODO: Check the common extensions supported by all connected devices and
    // return them. For now, hardcoding some extensions we know are supported by
    // all Level0 devices.
    SET_PARAM_VALUE_STR(L0_SUPPORTED_EXTENSIONS);
    break;
  case PI_PLATFORM_INFO_PROFILE:
    // TODO: figure out what this means and how is this used
    SET_PARAM_VALUE_STR("FULL_PROFILE");
    break;
  case PI_PLATFORM_INFO_VERSION:
    // TODO: this should query to zeDriverGetDriverVersion
    // but we don't yet have the driver handle here.
    //
    // From OpenCL 2.1: "This version string has the following format:
    // OpenCL<space><major_version.minor_version><space><platform-specific
    // information>. Follow the same notation here.
    //
    SET_PARAM_VALUE_STR(platform->L0DriverApiVersion.c_str());
    break;
  default:
    // TODO: implement other parameters
    die("Unsupported param_name in piPlatformGetInfo");
  }

  return PI_SUCCESS;
}

pi_result L0(piDevicesGet)(pi_platform      platform,
                           pi_device_type   device_type,
                           pi_uint32        num_entries,
                           pi_device *      devices,
                           pi_uint32 *      num_devices) {

  assert(platform);
  ze_driver_handle_t ze_driver = platform->L0Driver;

  // Get number of devices supporting L0
  uint32_t ze_device_count = 0;
  const bool askingForGPU = (device_type & PI_DEVICE_TYPE_GPU);
  ZE_CALL(zeDeviceGet(ze_driver, &ze_device_count, nullptr));
  if (ze_device_count == 0 || !askingForGPU) {
    if (num_devices)
      *num_devices = 0;
    return PI_SUCCESS;
  }

  if (num_devices)
    *num_devices = ze_device_count;

  // TODO: Delete array at teardown
  ze_device_handle_t *ze_devices = new ze_device_handle_t[ze_device_count];
  ZE_CALL(zeDeviceGet(ze_driver, &ze_device_count, ze_devices));

  for (uint32_t i = 0; i < ze_device_count; ++i) {
    // TODO: add check for device type
    if (i < num_entries) {
      auto L0PiDevice = new _pi_device(ze_devices[i], platform);

      // Create the immediate command list to be used for initializations
      // Created as synchronous so level-zero performs implicit synchronization and
      // there is no need to query for completion in the plugin
      ze_device_handle_t ze_device = L0PiDevice->L0Device;
      ze_command_queue_desc_t ze_command_queue_desc = {};
      ze_command_queue_desc.version = ZE_COMMAND_QUEUE_DESC_VERSION_CURRENT;
      ze_command_queue_desc.ordinal = 0;
      ze_command_queue_desc.mode = ZE_COMMAND_QUEUE_MODE_SYNCHRONOUS;
      ZE_CALL(zeCommandListCreateImmediate(ze_device, &ze_command_queue_desc,
                                           &L0PiDevice->L0CommandListInit));

      // Cache device properties
      L0PiDevice->L0DeviceProperties.version = ZE_DEVICE_PROPERTIES_VERSION_CURRENT;
      ZE_CALL(zeDeviceGetProperties(
        ze_device,
        &L0PiDevice->L0DeviceProperties));

      L0PiDevice->L0DeviceComputeProperties.version = ZE_DEVICE_COMPUTE_PROPERTIES_VERSION_CURRENT;
      ZE_CALL(zeDeviceGetComputeProperties(
        ze_device,
        &L0PiDevice->L0DeviceComputeProperties));

      devices[i] = L0PiDevice;
    }
  }
  return PI_SUCCESS;
}

pi_result L0(piDeviceRetain)(pi_device device)
{
  assert(device);

  // The root-device ref-count remains unchanged (always 1).
  if (device->IsSubDevice) {
    ++(device->RefCount);
  }
  return PI_SUCCESS;
}

pi_result L0(piDeviceRelease)(pi_device device)
{
  assert(device);

  // TODO: OpenCL says root-device ref-count remains unchanged (1),
  // but when would we free the device's data?
  //
  if (--(device->RefCount) == 0) {
    // Destroy the command list used for initializations
    ZE_CALL(zeCommandListDestroy(device->L0CommandListInit));
    delete device;
  }

  return PI_SUCCESS;
}

pi_result L0(piDeviceGetInfo)(pi_device       device,
                              pi_device_info  param_name,
                              size_t          param_value_size,
                              void *          param_value,
                              size_t *        param_value_size_ret) {

  assert(device != nullptr);

  ze_device_handle_t ze_device = device->L0Device;

  uint32_t ze_avail_mem_count = 0;
  ZE_CALL(zeDeviceGetMemoryProperties(
    ze_device,
    &ze_avail_mem_count,
    nullptr
  ));
  // Confirm at least one memory is available in the device
  assert(ze_avail_mem_count > 0);
  ze_device_memory_properties_t *ze_device_memory_properties =
    new ze_device_memory_properties_t[ze_avail_mem_count]();
  for (uint32_t i = 0; i < ze_avail_mem_count; i++) {
    ze_device_memory_properties[i].version = ZE_DEVICE_MEMORY_PROPERTIES_VERSION_CURRENT;
  }
  // TODO: cache various device properties in the PI device object,
  // and initialize them only upon they are first requested.
  //
  ZE_CALL(zeDeviceGetMemoryProperties(
    ze_device,
    &ze_avail_mem_count,
    ze_device_memory_properties
  ));

  ze_device_image_properties_t ze_device_image_properties;
  ze_device_image_properties.version = ZE_DEVICE_IMAGE_PROPERTIES_VERSION_CURRENT;
  ZE_CALL(zeDeviceGetImageProperties(
    ze_device,
    &ze_device_image_properties
  ));

  ze_device_kernel_properties_t ze_device_kernel_properties;
  ze_device_kernel_properties.version = ZE_DEVICE_KERNEL_PROPERTIES_VERSION_CURRENT;
  ZE_CALL(zeDeviceGetKernelProperties(
    ze_device,
    &ze_device_kernel_properties
  ));

  ze_device_cache_properties_t ze_device_cache_properties;
    ze_device_cache_properties.version = ZE_DEVICE_CACHE_PROPERTIES_VERSION_CURRENT;
    ZE_CALL(zeDeviceGetCacheProperties(
      ze_device,
      &ze_device_cache_properties
    ));

  switch (param_name) {
  case PI_DEVICE_INFO_TYPE: {
    if (device->L0DeviceProperties.type == ZE_DEVICE_TYPE_GPU) {
      SET_PARAM_VALUE(PI_DEVICE_TYPE_GPU);
    } else { // ZE_DEVICE_TYPE_FPGA
      zePrint("FPGA not supported\n");
      return PI_INVALID_VALUE;
    }
    break;
  }
  case PI_DEVICE_INFO_PARENT_DEVICE:
    // TODO: all L0 devices are parent ?
    SET_PARAM_VALUE(pi_device{0});
    break;
  case PI_DEVICE_INFO_PLATFORM:
    SET_PARAM_VALUE(device->Platform);
    break;
  case PI_DEVICE_INFO_VENDOR_ID:
    SET_PARAM_VALUE(pi_uint32{device->L0DeviceProperties.vendorId});
    break;
  case PI_DEVICE_INFO_EXTENSIONS: {
    // Convention adopted from OpenCL:
    //     "Returns a space separated list of extension names (the extension
    // names themselves do not contain any spaces) supported by the device."
    //
    // TODO: Use proper mechanism to get this information from Level0 after
    // it is added to Level0.
    // Hardcoding the few we know are supported by the current hardware.
    //
    //
    std::string SupportedExtensions;

    // cl_khr_il_program - OpenCL 2.0 KHR extension for SPIRV support. Core
    //   feature in >OpenCL 2.1
    // cl_khr_subgroups - Extension adds support for implementation-controlled
    //   subgroups.
    // cl_intel_subgroups - Extension adds subgroup features, defined by
    // Intel. cl_intel_subgroups_short - Extension adds subgroup functions
    // described in
    //   the cl_intel_subgroups extension to support 16-bit integer data types
    //   for performance.
    // cl_intel_required_subgroup_size - Extension to allow programmers to
    //   optionally specify the required subgroup size for a kernel function.
    // cl_khr_fp16 - Optional half floating-point support.
    // cl_khr_fp64 - Support for double floating-point precision.
    // cl_khr_int64_base_atomics, cl_khr_int64_extended_atomics - Optional
    //   extensions that implement atomic operations on 64-bit signed and
    //   unsigned integers to locations in __global and __local memory.
    // cl_khr_3d_image_writes - Extension to enable writes to 3D image memory
    //   objects.
    //
    // Hardcoding some extensions we know are supported by all Level0 devices.
    SupportedExtensions += (L0_SUPPORTED_EXTENSIONS);
    if (ze_device_kernel_properties.fp16Supported)
      SupportedExtensions += ("cl_khr_fp16 ");
    if (ze_device_kernel_properties.fp64Supported)
      SupportedExtensions += ("cl_khr_fp64 ");
    if (ze_device_kernel_properties.int64AtomicsSupported)
      // int64AtomicsSupported indicates support for both.
      SupportedExtensions +=
          ("cl_khr_int64_base_atomics cl_khr_int64_extended_atomics ");
    if (ze_device_image_properties.supported)
      // Supports reading and writing of images.
      SupportedExtensions += ("cl_khr_3d_image_writes ");

    SET_PARAM_VALUE_STR(SupportedExtensions.c_str());
    break;
  }
  case PI_DEVICE_INFO_NAME:
    SET_PARAM_VALUE_STR(device->L0DeviceProperties.name);
    break;
  case PI_DEVICE_INFO_COMPILER_AVAILABLE:
    SET_PARAM_VALUE(pi_bool{1});
    break;
  case PI_DEVICE_INFO_LINKER_AVAILABLE:
    SET_PARAM_VALUE(pi_bool{1});
    break;
  case PI_DEVICE_INFO_MAX_COMPUTE_UNITS: {
    pi_uint32 max_compute_units =
        device->L0DeviceProperties.numEUsPerSubslice *
        device->L0DeviceProperties.numSubslicesPerSlice *
        device->L0DeviceProperties.numSlices;
    SET_PARAM_VALUE(pi_uint32{max_compute_units});
    break;
  }
  case PI_DEVICE_INFO_MAX_WORK_ITEM_DIMENSIONS:
    // L0 spec defines only three dimensions
    SET_PARAM_VALUE(pi_uint32{3});
    break;
  case PI_DEVICE_INFO_MAX_WORK_GROUP_SIZE:
    SET_PARAM_VALUE(
        pi_uint64{device->L0DeviceComputeProperties.maxTotalGroupSize});
    break;
  case PI_DEVICE_INFO_MAX_WORK_ITEM_SIZES: {
    struct {
      size_t arr[3];
    } max_group_size = {{device->L0DeviceComputeProperties.maxGroupSizeX,
                         device->L0DeviceComputeProperties.maxGroupSizeY,
                         device->L0DeviceComputeProperties.maxGroupSizeZ}};
    SET_PARAM_VALUE(max_group_size);
    break;
  }
  case PI_DEVICE_INFO_MAX_CLOCK_FREQUENCY:
    SET_PARAM_VALUE(pi_uint32{device->L0DeviceProperties.coreClockRate});
    break;
  case PI_DEVICE_INFO_ADDRESS_BITS: {
    // TODO: To confirm with spec.
    SET_PARAM_VALUE(pi_uint32{64});
    break;
  }
  case PI_DEVICE_INFO_MAX_MEM_ALLOC_SIZE: {
    // TODO: To confirm with spec.
    uint32_t max_mem_alloc_size = 0;
    for (uint32_t i = 0; i < ze_avail_mem_count; i++) {
      max_mem_alloc_size += ze_device_memory_properties[i].totalSize;
    }
    SET_PARAM_VALUE(pi_uint64{max_mem_alloc_size});
    break;
  }
  case PI_DEVICE_INFO_GLOBAL_MEM_SIZE: {
    uint32_t global_mem_size = 0;
    for (uint32_t i = 0; i < ze_avail_mem_count; i++) {
      global_mem_size += ze_device_memory_properties[i].totalSize;
    }
    SET_PARAM_VALUE(pi_uint64{global_mem_size});
    break;
  }
  case PI_DEVICE_INFO_LOCAL_MEM_SIZE:
    SET_PARAM_VALUE(
        pi_uint64{device->L0DeviceComputeProperties.maxSharedLocalMemory});
    break;
  case PI_DEVICE_INFO_IMAGE_SUPPORT:
    SET_PARAM_VALUE(pi_bool{ze_device_image_properties.supported});
    break;
  case PI_DEVICE_INFO_HOST_UNIFIED_MEMORY:
    SET_PARAM_VALUE(
        pi_bool{device->L0DeviceProperties.unifiedMemorySupported});
    break;
  case PI_DEVICE_INFO_AVAILABLE:
    SET_PARAM_VALUE(pi_bool{ze_device ? true : false});
    break;
  case PI_DEVICE_INFO_VENDOR:
    // TODO: Level-Zero does not return vendor's name at the moment
    // only the ID.
    SET_PARAM_VALUE_STR("Intel(R) Corporation");
    break;
  case PI_DEVICE_INFO_DRIVER_VERSION:
    SET_PARAM_VALUE_STR(device->Platform->L0DriverVersion.c_str());
        break;
  case PI_DEVICE_INFO_VERSION:
    SET_PARAM_VALUE_STR(device->Platform->L0DriverApiVersion.c_str());
    break;
  case PI_DEVICE_INFO_PARTITION_MAX_SUB_DEVICES: {
    uint32_t ze_sub_device_count = 0;
    ZE_CALL(zeDeviceGetSubDevices(ze_device, &ze_sub_device_count, nullptr));
    SET_PARAM_VALUE(pi_uint32{ze_sub_device_count});
    break;
  }
  case PI_DEVICE_INFO_REFERENCE_COUNT:
    SET_PARAM_VALUE(pi_uint32{device->RefCount});
    break;
  case PI_DEVICE_INFO_PARTITION_PROPERTIES: {
    //
    // It is debatable if SYCL sub-device and partitioning APIs sufficient to
    // expose Level0 sub-devices / tiles?  We start with support of
    // "partition_by_affinity_domain" and "numa" but if that doesn't seem to
    // be a good fit we could look at adding a more descriptive partitioning
    // type.
    //
    // See https://gitlab.devtools.intel.com/one-api/level_zero/issues/239.
    //
    struct {
      pi_device_partition_property arr[2];
    } partition_properties = {{PI_DEVICE_PARTITION_BY_AFFINITY_DOMAIN, 0}};
    SET_PARAM_VALUE(partition_properties);
    break;
  }
  case PI_DEVICE_INFO_PARTITION_AFFINITY_DOMAIN:
    SET_PARAM_VALUE(pi_device_affinity_domain{
        PI_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE});
    break;
  case PI_DEVICE_INFO_PARTITION_TYPE: {
    if (device->IsSubDevice) {
      struct {
        pi_device_partition_property arr[3];
      } partition_properties = {{PI_DEVICE_PARTITION_BY_AFFINITY_DOMAIN,
                                 PI_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE,
                                 0}};
      SET_PARAM_VALUE(partition_properties);
    } else {
      // For root-device there is no partitioning to report.
      SET_PARAM_VALUE(pi_device_partition_property{0});
    }
    break;
  }

    // Everything under here is not supported yet

  case PI_DEVICE_INFO_OPENCL_C_VERSION:
    SET_PARAM_VALUE_STR("");
    break;
  case PI_DEVICE_INFO_PREFERRED_INTEROP_USER_SYNC:
    SET_PARAM_VALUE(pi_bool{true});
    break;
  case PI_DEVICE_INFO_PRINTF_BUFFER_SIZE:
    SET_PARAM_VALUE(size_t{ze_device_kernel_properties.printfBufferSize});
    break;
  case PI_DEVICE_INFO_PROFILE:
    SET_PARAM_VALUE_STR("FULL_PROFILE");
    break;
  case PI_DEVICE_INFO_BUILT_IN_KERNELS:
    // TODO: To find out correct value
    SET_PARAM_VALUE_STR("");
    break;
  case PI_DEVICE_INFO_QUEUE_PROPERTIES:
    SET_PARAM_VALUE(pi_queue_properties{
        PI_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | PI_QUEUE_PROFILING_ENABLE});
    break;
  case PI_DEVICE_INFO_EXECUTION_CAPABILITIES:
    SET_PARAM_VALUE(pi_device_exec_capabilities{
        PI_DEVICE_EXEC_CAPABILITIES_NATIVE_KERNEL});
    break;
  case PI_DEVICE_INFO_ENDIAN_LITTLE:
    SET_PARAM_VALUE(pi_bool{true});
    break;
  case PI_DEVICE_INFO_ERROR_CORRECTION_SUPPORT:
    SET_PARAM_VALUE(pi_bool{device->L0DeviceProperties.eccMemorySupported});
    break;
  case PI_DEVICE_INFO_PROFILING_TIMER_RESOLUTION:
    SET_PARAM_VALUE(size_t{device->L0DeviceProperties.timerResolution});
    break;
  case PI_DEVICE_INFO_LOCAL_MEM_TYPE:
    SET_PARAM_VALUE(PI_DEVICE_LOCAL_MEM_TYPE_LOCAL);
    break;
  case PI_DEVICE_INFO_MAX_CONSTANT_ARGS:
    SET_PARAM_VALUE(pi_uint32{64});
    break;
  case PI_DEVICE_INFO_MAX_CONSTANT_BUFFER_SIZE:
    SET_PARAM_VALUE(pi_uint64{ze_device_image_properties.maxImageBufferSize});
    break;
  case PI_DEVICE_INFO_GLOBAL_MEM_CACHE_TYPE:
    SET_PARAM_VALUE(PI_DEVICE_MEM_CACHE_TYPE_READ_WRITE_CACHE);
    break;
  case PI_DEVICE_INFO_GLOBAL_MEM_CACHELINE_SIZE:
    SET_PARAM_VALUE(
        pi_uint32{ze_device_cache_properties.lastLevelCachelineSize});
    break;
  case PI_DEVICE_INFO_GLOBAL_MEM_CACHE_SIZE:
    SET_PARAM_VALUE(pi_uint64{ze_device_cache_properties.lastLevelCacheSize});
    break;
  case PI_DEVICE_INFO_MAX_PARAMETER_SIZE:
    SET_PARAM_VALUE(size_t{ze_device_kernel_properties.maxArgumentsSize});
    break;
  case PI_DEVICE_INFO_MEM_BASE_ADDR_ALIGN:
    // SYCL/OpenCL spec is vague on what this means exactly, but seems to
    // be for "alignment requirement (in bits) for sub-buffer offsets."
    // An OpenCL implementation returns 8*128, but L0 can do just 8,
    // meaning unaligned access for values of types larger than 8 bits.
    //
    SET_PARAM_VALUE(pi_uint32{8});
    break;
  case PI_DEVICE_INFO_MAX_SAMPLERS:
    SET_PARAM_VALUE(pi_uint32{ze_device_image_properties.maxSamplers});
    break;
  case PI_DEVICE_INFO_MAX_READ_IMAGE_ARGS:
    SET_PARAM_VALUE(pi_uint32{ze_device_image_properties.maxReadImageArgs});
    break;
  case PI_DEVICE_INFO_MAX_WRITE_IMAGE_ARGS:
    SET_PARAM_VALUE(pi_uint32{ze_device_image_properties.maxWriteImageArgs});
    break;
  case PI_DEVICE_INFO_SINGLE_FP_CONFIG: {
    uint64_t singleFPValue = 0;
    ze_fp_capabilities_t singleFpCapabilities =
        ze_device_kernel_properties.singleFpCapabilities;
    if (ZE_FP_CAPS_DENORM & singleFpCapabilities) {
      singleFPValue |= PI_FP_DENORM;
    }
    if (ZE_FP_CAPS_INF_NAN & singleFpCapabilities) {
      singleFPValue |= PI_FP_INF_NAN;
    }
    if (ZE_FP_CAPS_ROUND_TO_NEAREST & singleFpCapabilities) {
      singleFPValue |= PI_FP_ROUND_TO_NEAREST;
    }
    if (ZE_FP_CAPS_ROUND_TO_ZERO & singleFpCapabilities) {
      singleFPValue |= PI_FP_ROUND_TO_ZERO;
    }
    if (ZE_FP_CAPS_ROUND_TO_INF & singleFpCapabilities) {
      singleFPValue |= PI_FP_ROUND_TO_INF;
    }
    if (ZE_FP_CAPS_FMA & singleFpCapabilities) {
      singleFPValue |= PI_FP_FMA;
    }
    SET_PARAM_VALUE(pi_uint64{singleFPValue});
    break;
  }
  case PI_DEVICE_INFO_HALF_FP_CONFIG: {
    uint64_t halfFPValue = 0;
    ze_fp_capabilities_t halfFpCapabilities =
        ze_device_kernel_properties.halfFpCapabilities;
    if (ZE_FP_CAPS_DENORM & halfFpCapabilities) {
      halfFPValue |= PI_FP_DENORM;
    }
    if (ZE_FP_CAPS_INF_NAN & halfFpCapabilities) {
      halfFPValue |= PI_FP_INF_NAN;
    }
    if (ZE_FP_CAPS_ROUND_TO_NEAREST & halfFpCapabilities) {
      halfFPValue |= PI_FP_ROUND_TO_NEAREST;
    }
    if (ZE_FP_CAPS_ROUND_TO_ZERO & halfFpCapabilities) {
      halfFPValue |= PI_FP_ROUND_TO_ZERO;
    }
    if (ZE_FP_CAPS_ROUND_TO_INF & halfFpCapabilities) {
      halfFPValue |= PI_FP_ROUND_TO_INF;
    }
    if (ZE_FP_CAPS_FMA & halfFpCapabilities) {
      halfFPValue |= PI_FP_FMA;
    }
    SET_PARAM_VALUE(pi_uint64{halfFPValue});
    break;
  }
  case PI_DEVICE_INFO_DOUBLE_FP_CONFIG: {
    uint64_t doubleFPValue = 0;
    ze_fp_capabilities_t doubleFpCapabilities =
        ze_device_kernel_properties.doubleFpCapabilities;
    if (ZE_FP_CAPS_DENORM & doubleFpCapabilities) {
      doubleFPValue |= PI_FP_DENORM;
    }
    if (ZE_FP_CAPS_INF_NAN & doubleFpCapabilities) {
      doubleFPValue |= PI_FP_INF_NAN;
    }
    if (ZE_FP_CAPS_ROUND_TO_NEAREST & doubleFpCapabilities) {
      doubleFPValue |= PI_FP_ROUND_TO_NEAREST;
    }
    if (ZE_FP_CAPS_ROUND_TO_ZERO & doubleFpCapabilities) {
      doubleFPValue |= PI_FP_ROUND_TO_ZERO;
    }
    if (ZE_FP_CAPS_ROUND_TO_INF & doubleFpCapabilities) {
      doubleFPValue |= PI_FP_ROUND_TO_INF;
    }
    if (ZE_FP_CAPS_FMA & doubleFpCapabilities) {
      doubleFPValue |= PI_FP_FMA;
    }
    SET_PARAM_VALUE(pi_uint64{doubleFPValue});
    break;
  }
  case PI_DEVICE_INFO_IMAGE2D_MAX_WIDTH:
    // TODO: https://gitlab.devtools.intel.com/one-api/level_zero/issues/288
    // Until L0 provides needed info, hardcode default minimum values required
    // by the SYCL specification.
    //
    SET_PARAM_VALUE(size_t{8192});
    break;
  case PI_DEVICE_INFO_IMAGE2D_MAX_HEIGHT:
    // TODO: https://gitlab.devtools.intel.com/one-api/level_zero/issues/288
    // Until L0 provides needed info, hardcode default minimum values required
    // by the SYCL specification.
    //
    SET_PARAM_VALUE(size_t{8192});
    break;
  case PI_DEVICE_INFO_IMAGE3D_MAX_WIDTH:
    // TODO: https://gitlab.devtools.intel.com/one-api/level_zero/issues/288
    // Until L0 provides needed info, hardcode default minimum values required
    // by the SYCL specification.
    //
    SET_PARAM_VALUE(size_t{2048});
    break;
  case PI_DEVICE_INFO_IMAGE3D_MAX_HEIGHT:
    // TODO: https://gitlab.devtools.intel.com/one-api/level_zero/issues/288
    // Until L0 provides needed info, hardcode default minimum values required
    // by the SYCL specification.
    //
    SET_PARAM_VALUE(size_t{2048});
    break;
  case PI_DEVICE_INFO_IMAGE3D_MAX_DEPTH:
    // TODO: https://gitlab.devtools.intel.com/one-api/level_zero/issues/288
    // Until L0 provides needed info, hardcode default minimum values required
    // by the SYCL specification.
    //
    SET_PARAM_VALUE(size_t{2048});
    break;
  case PI_DEVICE_INFO_IMAGE_MAX_BUFFER_SIZE:
    SET_PARAM_VALUE(size_t{ze_device_image_properties.maxImageBufferSize});
    break;
  case PI_DEVICE_INFO_IMAGE_MAX_ARRAY_SIZE:
    SET_PARAM_VALUE(size_t{ze_device_image_properties.maxImageArraySlices});
    break;
  //
  // Handle SIMD widths.
  // TODO: can we do better than this?
  // See https://gitlab.devtools.intel.com/one-api/level_zero/issues/239.
  //
  case PI_DEVICE_INFO_NATIVE_VECTOR_WIDTH_CHAR:
  case PI_DEVICE_INFO_PREFERRED_VECTOR_WIDTH_CHAR:
    SET_PARAM_VALUE(device->L0DeviceProperties.physicalEUSimdWidth / 1);
    break;
  case PI_DEVICE_INFO_NATIVE_VECTOR_WIDTH_SHORT:
  case PI_DEVICE_INFO_PREFERRED_VECTOR_WIDTH_SHORT:
    SET_PARAM_VALUE(device->L0DeviceProperties.physicalEUSimdWidth / 2);
    break;
  case PI_DEVICE_INFO_NATIVE_VECTOR_WIDTH_INT:
  case PI_DEVICE_INFO_PREFERRED_VECTOR_WIDTH_INT:
    SET_PARAM_VALUE(device->L0DeviceProperties.physicalEUSimdWidth / 4);
    break;
  case PI_DEVICE_INFO_NATIVE_VECTOR_WIDTH_LONG:
  case PI_DEVICE_INFO_PREFERRED_VECTOR_WIDTH_LONG:
    SET_PARAM_VALUE(device->L0DeviceProperties.physicalEUSimdWidth / 8);
    break;
  case PI_DEVICE_INFO_NATIVE_VECTOR_WIDTH_FLOAT:
  case PI_DEVICE_INFO_PREFERRED_VECTOR_WIDTH_FLOAT:
    SET_PARAM_VALUE(device->L0DeviceProperties.physicalEUSimdWidth / 4);
    break;
  case PI_DEVICE_INFO_NATIVE_VECTOR_WIDTH_DOUBLE:
  case PI_DEVICE_INFO_PREFERRED_VECTOR_WIDTH_DOUBLE:
    SET_PARAM_VALUE(device->L0DeviceProperties.physicalEUSimdWidth / 8);
    break;
  case PI_DEVICE_INFO_NATIVE_VECTOR_WIDTH_HALF:
  case PI_DEVICE_INFO_PREFERRED_VECTOR_WIDTH_HALF:
    SET_PARAM_VALUE(device->L0DeviceProperties.physicalEUSimdWidth / 2);
    break;
  case PI_DEVICE_INFO_MAX_NUM_SUB_GROUPS: {
    // Max_num_sub_Groups =
    // maxTotalGroupSize/min(set
    // of subGroupSizes);
    uint32_t MinSubGroupSize =
        device->L0DeviceComputeProperties.subGroupSizes[0];
    for (uint32_t i = 1;
         i < device->L0DeviceComputeProperties.numSubGroupSizes; i++) {
      if (MinSubGroupSize >
          device->L0DeviceComputeProperties.subGroupSizes[i])
        MinSubGroupSize = device->L0DeviceComputeProperties.subGroupSizes[i];
    }
    SET_PARAM_VALUE(device->L0DeviceComputeProperties.maxTotalGroupSize /
                    MinSubGroupSize);
    break;
  }
  case PI_DEVICE_INFO_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS: {
    // TODO: Not supported yet. Needs to be updated after support is added.
    // https://gitlab.devtools.intel.com/one-api/level_zero/issues/338
    SET_PARAM_VALUE(pi_bool{false});
    break;
  }
  case PI_DEVICE_INFO_SUB_GROUP_SIZES_INTEL: {
    // ze_device_compute_properties.subGroupSizes is in uint32_t whereas the
    // expected return is size_t datatype. size_t can be 8 bytes of data.
    SET_PARAM_VALUE_VLA(device->L0DeviceComputeProperties.subGroupSizes,
                        device->L0DeviceComputeProperties.numSubGroupSizes,
                        size_t);
    break;
  }
  case PI_DEVICE_INFO_IL_VERSION: {
    // Set to a space separated list of IL version strings of the form
    // <IL_Prefix>_<Major_version>.<Minor_version>.
    // "SPIR-V" is a required IL prefix when cl_khr_il_progam extension is
    // reported.
    uint32_t spirv_version =
        ze_device_kernel_properties.spirvVersionSupported;
    uint32_t spirv_version_major = ZE_MAJOR_VERSION(spirv_version);
    uint32_t spirv_version_minor = ZE_MINOR_VERSION(spirv_version);

    char spirv_version_string[50];
    int len = sprintf(spirv_version_string, "SPIR-V_%d.%d ",
                      spirv_version_major, spirv_version_minor);
    // returned string to contain only len number of characters.
    std::string IL_version(spirv_version_string, len);
    SET_PARAM_VALUE_STR(IL_version.c_str());
    break;
  }
  case PI_DEVICE_INFO_USM_HOST_SUPPORT:
  case PI_DEVICE_INFO_USM_DEVICE_SUPPORT:
  case PI_DEVICE_INFO_USM_SINGLE_SHARED_SUPPORT:
  case PI_DEVICE_INFO_USM_CROSS_SHARED_SUPPORT:
  case PI_DEVICE_INFO_USM_SYSTEM_SHARED_SUPPORT: {
    pi_uint64 supported = 0;
    if (device->L0DeviceProperties.unifiedMemorySupported) {
      // TODO: Use
      // ze_memory_access_capabilities_t
      supported = PI_USM_ACCESS | PI_USM_ATOMIC_ACCESS |
                  PI_USM_CONCURRENT_ACCESS | PI_USM_CONCURRENT_ATOMIC_ACCESS;
    }
    SET_PARAM_VALUE(supported);
    break;
  }
  default:
    zePrint("Unsupported param_name in piGetDeviceInfo\n");
    zePrint("param_name=%d(0x%x)\n", param_name, param_name);
    return PI_INVALID_VALUE;
  }

  return PI_SUCCESS;
}

pi_result L0(piDevicePartition)(
  pi_device     device,
  const pi_device_partition_property * properties,
  pi_uint32     num_devices,
  pi_device *   out_devices,
  pi_uint32 *   out_num_devices)
{
  // Other partitioning ways are not supported by L0
  if (properties[0] != PI_DEVICE_PARTITION_BY_AFFINITY_DOMAIN ||
      properties[1] != PI_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE) {
    return PI_INVALID_VALUE;
  }

  assert(device);
  // Get the number of subdevices/tiles available.
  // TODO: maybe add interface to create the specified # of subdevices.
  uint32_t count = 0;
  ZE_CALL(zeDeviceGetSubDevices(device->L0Device, &count, nullptr));

  // Check that the requested/allocated # of sub-devices is the same
  // as was reported by the above call.
  // TODO: we may want to support smaller/larger # devices too.
  if (count != num_devices) {
    zePrint("piDevicePartition: unsupported # of sub-devices requested\n");
    return PI_INVALID_OPERATION;
  }

  if (out_num_devices) {
    *out_num_devices = count;
  }

  if (!out_devices) {
    // If we are not given the buffer, we are done.
    return PI_SUCCESS;
  }

  auto ze_subdevices = new ze_device_handle_t[count];
  ZE_CALL(zeDeviceGetSubDevices(device->L0Device, &count, ze_subdevices));

  // Wrap the L0 sub-devices into PI sub-devices, and write them out.
  for (uint32_t i = 0; i < count; ++i) {
    auto L0PiDevice = new _pi_device(ze_subdevices[i], device->Platform,
                                     true /* isSubDevice */);
    out_devices[i] = L0PiDevice;

    // Cache device properties
    L0PiDevice->L0DeviceProperties.version = ZE_DEVICE_PROPERTIES_VERSION_CURRENT;
    ZE_CALL(zeDeviceGetProperties(ze_subdevices[i], &L0PiDevice->L0DeviceProperties));

    L0PiDevice->L0DeviceComputeProperties.version = ZE_DEVICE_COMPUTE_PROPERTIES_VERSION_CURRENT;
    ZE_CALL(zeDeviceGetComputeProperties(ze_subdevices[i], &L0PiDevice->L0DeviceComputeProperties));
  }
  delete[] ze_subdevices;

  return PI_SUCCESS;
}

pi_result L0(piextDeviceSelectBinary)(
  pi_device           device, // TODO: does this need to be context?
  pi_device_binary *  binaries,
  pi_uint32           num_binaries,
  pi_uint32        *  selected_binary_ind) {

  // TODO dummy implementation.
  // Real implementaion will use the same mechanism OpenCL ICD dispatcher
  // uses. Somthing like:
  //   PI_VALIDATE_HANDLE_RETURN_HANDLE(ctx, PI_INVALID_CONTEXT);
  //     return context->dispatch->piextDeviceSelectIR(
  //       ctx, images, num_images, selected_image);
  // where context->dispatch is set to the dispatch table provided by PI
  // plugin for platform/device the ctx was created for.

  constexpr pi_uint32 invalid_ind = std::numeric_limits<pi_uint32>::max();
  *selected_binary_ind = num_binaries > 0 ? 0 : invalid_ind;
  return PI_SUCCESS;
}

pi_result piextDeviceGetNativeHandle(pi_device device,
                                     pi_native_handle *nativeHandle) {
  assert(device);

  auto ze_device = pi_cast<ze_device_handle_t*>(nativeHandle);
  // Extract the L0 module handle from the given PI device
  *ze_device = device->L0Device;
  return PI_SUCCESS;
}

pi_result piextDeviceCreateWithNativeHandle(pi_native_handle nativeHandle,
                                            pi_device *device) {
  // Create PI device from the given L0 device handle.
  die("piextDeviceCreateWithNativeHandle: not supported");
  return PI_SUCCESS;
}

pi_result L0(piContextCreate)(
  const pi_context_properties * properties,
  pi_uint32         num_devices,
  const pi_device * devices,
  void (*           pfn_notify)(
    const char * errinfo,
    const void * private_info,
    size_t       cb,
    void *       user_data),
  void *            user_data,
  pi_context *      ret_context) {

  // L0 does not have notion of contexts.
  // Return the device handle (only single device is allowed) as a context handle.
  //
  if (num_devices != 1) {
    zePrint("piCreateContext: context should have exactly one device\n");
    return PI_INVALID_VALUE;
  }

  assert(devices);
  assert(ret_context);

  *ret_context =  new _pi_context(*devices);
  return PI_SUCCESS;
}

pi_result L0(piContextGetInfo)(
  pi_context         context,
  pi_context_info    param_name,
  size_t             param_value_size,
  void *             param_value,
  size_t *           param_value_size_ret) {

  assert(context);

  if (param_name == PI_CONTEXT_INFO_DEVICES) {
    SET_PARAM_VALUE(context->Device);
  }
  else if (param_name == PI_CONTEXT_INFO_NUM_DEVICES) {
    SET_PARAM_VALUE(pi_uint32{1});
  }
  else if (param_name == PI_CONTEXT_INFO_REFERENCE_COUNT) {
    SET_PARAM_VALUE(pi_uint32{context->RefCount});
  }
  else {
    // TODO: implement other parameters
    die("piGetContextInfo: unsuppported param_name.");
  }

  return PI_SUCCESS;
}

// FIXME: Dummy implementation to prevent link fail
pi_result piextContextSetExtendedDeleter(pi_context context,
                                         pi_context_extended_deleter function,
                                         void *user_data) {
  die("piextContextSetExtendedDeleter: not supported");
  return PI_SUCCESS;
}

pi_result piextContextGetNativeHandle(pi_context context,
                                      pi_native_handle *nativeHandle) {
  die("piextContextGetNativeHandle: not supported");
  return PI_SUCCESS;
}

pi_result piextContextCreateWithNativeHandle(pi_native_handle nativeHandle,
    pi_context *context) {
  die("piextContextCreateWithNativeHandle: not supported");
  return PI_SUCCESS;
}

pi_result L0(piContextRetain)(
  pi_context context) {

  assert(context);
  ++(context->RefCount);
  return PI_SUCCESS;
}

pi_result L0(piContextRelease)(
  pi_context context) {

  assert(context);
  if (--(context->RefCount) == 0) {
    delete context;
  }
  return PI_SUCCESS;
}

pi_result L0(piQueueCreate)(
  pi_context                    context,
  pi_device                     device,
  pi_queue_properties           properties,
  pi_queue *                    queue) {

  // Check that unexpected bits are not set.
  assert(!(properties & ~(PI_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
                          PI_QUEUE_PROFILING_ENABLE | PI_QUEUE_ON_DEVICE |
                          PI_QUEUE_ON_DEVICE_DEFAULT)));

  ze_device_handle_t        ze_device;
  ze_command_queue_handle_t ze_command_queue;

  if (!context) {
    return PI_INVALID_CONTEXT;
  }
  if (context->Device != device) {
    return PI_INVALID_DEVICE;
  }

  assert(device);
  ze_device = device->L0Device;
  ze_command_queue_desc_t ze_command_queue_desc = {};
  ze_command_queue_desc.version = ZE_COMMAND_QUEUE_DESC_VERSION_CURRENT;
  ze_command_queue_desc.ordinal = 0;
  ze_command_queue_desc.mode = ZE_COMMAND_QUEUE_MODE_ASYNCHRONOUS;

  ZE_CALL(zeCommandQueueCreate(
    ze_device,
    &ze_command_queue_desc,  // TODO: translate properties
    &ze_command_queue));

  assert(queue);
  *queue = new _pi_queue(ze_command_queue, context);

  return PI_SUCCESS;
}

pi_result L0(piQueueGetInfo)(
  pi_queue            queue,
  pi_queue_info       param_name,
  size_t              param_value_size,
  void *              param_value,
  size_t *            param_value_size_ret) {

  assert(queue);

  // TODO: consider support for queue properties and size
  switch (param_name) {
  case PI_QUEUE_INFO_CONTEXT:
    SET_PARAM_VALUE(queue->Context);
    break;
  case PI_QUEUE_INFO_DEVICE:
    SET_PARAM_VALUE(queue->Context->Device);
    break;
  case PI_QUEUE_INFO_REFERENCE_COUNT:
    SET_PARAM_VALUE(pi_uint32{queue->RefCount});
    break;
  case PI_QUEUE_INFO_PROPERTIES:
    die("PI_QUEUE_INFO_PROPERTIES in piQueueGetInfo not implemented\n");
    break;
  case PI_QUEUE_INFO_SIZE:
    die("PI_QUEUE_INFO_SIZE in piQueueGetInfo not implemented\n");
    break;
  case PI_QUEUE_INFO_DEVICE_DEFAULT:
    die("PI_QUEUE_INFO_DEVICE_DEFAULT in piQueueGetInfo not implemented\n");
    break;
  default:
    zePrint("Unsupported param_name in piQueueGetInfo: param_name=%d(0x%x)\n",
            param_name, param_name);
    return PI_INVALID_VALUE;
  }

  return PI_SUCCESS;
}


pi_result L0(piQueueRetain)(pi_queue queue) {
  return PI_SUCCESS;
}

pi_result L0(piQueueRelease)(pi_queue queue) {
  assert(queue);
  ZE_CALL(zeCommandQueueDestroy(queue->L0CommandQueue));
  return PI_SUCCESS;
}

pi_result L0(piQueueFinish)(pi_queue queue)
{
  // Wait until command lists attached to the command queue are executed.
  assert(queue);
  ZE_CALL(zeCommandQueueSynchronize(queue->L0CommandQueue, UINT32_MAX));
  return PI_SUCCESS;
}

pi_result piextQueueGetNativeHandle(pi_queue queue,
                                    pi_native_handle *nativeHandle) {
  die("piextQueueGetNativeHandle: not supported");
  return PI_SUCCESS;
}

pi_result piextQueueCreateWithNativeHandle(pi_native_handle nativeHandle,
                                           pi_queue *queue) {
  die("piextQueueCreateWithNativeHandle: not supported");
  return PI_SUCCESS;
}

pi_result piMemBufferCreate(
  pi_context   context,
  pi_mem_flags flags,
  size_t       size,
  void *       host_ptr,
  pi_mem *     ret_mem) {

 // TODO: implement read-only, write-only
  assert((flags & PI_MEM_FLAGS_ACCESS_RW) != 0);
  assert(context);
  assert(ret_mem);

  void *ptr;
  ze_device_handle_t ze_device = context->Device->L0Device;

  ze_device_mem_alloc_desc_t ze_desc = {};
  ze_desc.flags = ZE_DEVICE_MEM_ALLOC_FLAG_DEFAULT;
  ze_desc.ordinal = 0;
  ZE_CALL(zeDriverAllocDeviceMem(
    context->Device->Platform->L0Driver,
    &ze_desc,
    size,
    1, // TODO: alignment
    ze_device,
    &ptr));

  if ((flags & PI_MEM_FLAGS_HOST_PTR_USE)  != 0 ||
      (flags & PI_MEM_FLAGS_HOST_PTR_COPY) != 0) {
    // Initialize the buffer synchronously with immediate offload
    ZE_CALL(zeCommandListAppendMemoryCopy(context->Device->L0CommandListInit,
                                          ptr, host_ptr, size, nullptr));
  }
  else if (flags == 0 ||
           (flags == PI_MEM_FLAGS_ACCESS_RW)) {
    // Nothing more to do.
  }
  else {
    die("piMemBufferCreate: not implemented");
  }

  auto HostPtr =
      (flags & PI_MEM_FLAGS_HOST_PTR_USE) ? pi_cast<char *>(host_ptr) : nullptr;
  *ret_mem =
      new _pi_buffer(context->Device->Platform,
                     pi_cast<char *>(ptr) /* L0 Memory Handle */, HostPtr);

  return PI_SUCCESS;
}

pi_result L0(piMemGetInfo)(
  pi_mem           mem,
  cl_mem_info      param_name, // TODO: untie from OpenCL
  size_t           param_value_size,
  void *           param_value,
  size_t *         param_value_size_ret) {
  die("piMemGetInfo: not implemented");
  return {};
}


pi_result L0(piMemRetain)(pi_mem mem) {
  assert(mem);
  ++(mem->RefCount);
  return PI_SUCCESS;
}

pi_result L0(piMemRelease)(pi_mem mem) {
  assert(mem);
  if (--(mem->RefCount) == 0) {
    if (mem->isImage()) {
      ZE_CALL(zeImageDestroy(pi_cast<ze_image_handle_t>(mem->getL0Handle())));
    }
    else {
      auto buf = static_cast<_pi_buffer *>(mem);
      if (!buf->isSubBuffer()) {
        ZE_CALL(zeDriverFreeMem(mem->Platform->L0Driver, mem->getL0Handle()));
      }
    }
    delete mem;
  }
  return PI_SUCCESS;
}

pi_result L0(piMemImageCreate)(
  pi_context              context,
  pi_mem_flags            flags,
  const pi_image_format * image_format,
  const pi_image_desc *   image_desc,
  void *                  host_ptr,
  pi_mem *                ret_image) {

  // TODO: implement read-only, write-only
  assert((flags & PI_MEM_FLAGS_ACCESS_RW) != 0);
  assert(image_format);
  assert(context);
  assert(ret_image);

  ze_image_format_type_t ze_image_format_type;
  size_t ze_image_format_type_size;
  switch (image_format->image_channel_data_type) {
  case CL_FLOAT:
    ze_image_format_type = ZE_IMAGE_FORMAT_TYPE_FLOAT;
    ze_image_format_type_size = 32;
    break;
  case CL_HALF_FLOAT:
    ze_image_format_type = ZE_IMAGE_FORMAT_TYPE_FLOAT;
    ze_image_format_type_size = 16;
    break;
  case CL_UNSIGNED_INT32:
    ze_image_format_type = ZE_IMAGE_FORMAT_TYPE_UINT;
    ze_image_format_type_size = 32;
    break;
  case CL_UNSIGNED_INT16:
    ze_image_format_type = ZE_IMAGE_FORMAT_TYPE_UINT;
    ze_image_format_type_size = 16;
    break;
  case CL_UNSIGNED_INT8:
    ze_image_format_type = ZE_IMAGE_FORMAT_TYPE_UINT;
    ze_image_format_type_size = 8;
    break;
  case CL_UNORM_INT16:
    ze_image_format_type = ZE_IMAGE_FORMAT_TYPE_UNORM;
    ze_image_format_type_size = 16;
    break;
  case CL_UNORM_INT8:
    ze_image_format_type = ZE_IMAGE_FORMAT_TYPE_UNORM;
    ze_image_format_type_size = 8;
    break;
  case CL_SIGNED_INT32:
    ze_image_format_type = ZE_IMAGE_FORMAT_TYPE_SINT;
    ze_image_format_type_size = 32;
    break;
  case CL_SIGNED_INT16:
    ze_image_format_type = ZE_IMAGE_FORMAT_TYPE_SINT;
    ze_image_format_type_size = 16;
    break;
  case CL_SIGNED_INT8:
    ze_image_format_type = ZE_IMAGE_FORMAT_TYPE_SINT;
    ze_image_format_type_size = 8;
    break;
  case CL_SNORM_INT16:
    ze_image_format_type = ZE_IMAGE_FORMAT_TYPE_SNORM;
    ze_image_format_type_size = 16;
    break;
  case CL_SNORM_INT8:
    ze_image_format_type = ZE_IMAGE_FORMAT_TYPE_SNORM;
    ze_image_format_type_size = 8;
    break;
  default:
    zePrint("piMemImageCreate: unsupported image data type: data type = %d\n",
            image_format->image_channel_data_type);
    return PI_INVALID_VALUE;
  }

  // TODO: populate the layout mapping
  ze_image_format_layout_t ze_image_format_layout;
  switch (image_format->image_channel_order) {
  case CL_RGBA:
    switch (ze_image_format_type_size) {
    case 8:  ze_image_format_layout = ZE_IMAGE_FORMAT_LAYOUT_8_8_8_8; break;
    case 16: ze_image_format_layout = ZE_IMAGE_FORMAT_LAYOUT_16_16_16_16; break;
    case 32: ze_image_format_layout = ZE_IMAGE_FORMAT_LAYOUT_32_32_32_32; break;
    default:
      zePrint("piMemImageCreate: unexpected data type size\n");
      return PI_INVALID_VALUE;
    }
    break;
  default:
    zePrint("format layout = %d\n", image_format->image_channel_order);
    die("piMemImageCreate: unsupported image format layout\n");
    break;
  }

  ze_image_format_desc_t formatDesc = {
    ze_image_format_layout,
    ze_image_format_type,
    // TODO: are swizzles deducted from image_format->image_channel_order?
    ZE_IMAGE_FORMAT_SWIZZLE_R,
    ZE_IMAGE_FORMAT_SWIZZLE_G,
    ZE_IMAGE_FORMAT_SWIZZLE_B,
    ZE_IMAGE_FORMAT_SWIZZLE_A
  };

  ze_image_type_t ze_image_type;
  switch (image_desc->image_type) {
  case PI_MEM_TYPE_IMAGE1D: ze_image_type = ZE_IMAGE_TYPE_1D; break;
  case PI_MEM_TYPE_IMAGE2D: ze_image_type = ZE_IMAGE_TYPE_2D; break;
  case PI_MEM_TYPE_IMAGE3D: ze_image_type = ZE_IMAGE_TYPE_3D; break;
  case PI_MEM_TYPE_IMAGE1D_ARRAY: ze_image_type = ZE_IMAGE_TYPE_1DARRAY; break;
  case PI_MEM_TYPE_IMAGE2D_ARRAY: ze_image_type = ZE_IMAGE_TYPE_2DARRAY; break;
  default:
    zePrint("piMemImageCreate: unsupported image type\n");
    return PI_INVALID_VALUE;
  }

  ze_image_desc_t imageDesc = {
    ZE_IMAGE_DESC_VERSION_CURRENT,
    pi_cast<ze_image_flag_t>(ZE_IMAGE_FLAG_PROGRAM_READ |
                             ZE_IMAGE_FLAG_PROGRAM_WRITE),
    ze_image_type,
    formatDesc,
    pi_cast<uint32_t>(image_desc->image_width),
    pi_cast<uint32_t>(image_desc->image_height),
    pi_cast<uint32_t>(image_desc->image_depth),
    pi_cast<uint32_t>(image_desc->image_array_size),
    image_desc->num_mip_levels
  };

  ze_image_handle_t hImage;
  ZE_CALL(zeImageCreate(context->Device->L0Device, &imageDesc, &hImage));

  auto HostPtr =
      (flags & PI_MEM_FLAGS_HOST_PTR_USE) ? pi_cast<char *>(host_ptr) : nullptr;
  auto L0PiImage = new _pi_image(context->Device->Platform, hImage, HostPtr);

#ifndef NDEBUG
  L0PiImage->L0ImageDesc = imageDesc;
#endif // !NDEBUG

  if ((flags & PI_MEM_FLAGS_HOST_PTR_USE)  != 0 ||
      (flags & PI_MEM_FLAGS_HOST_PTR_COPY) != 0) {
    // Initialize image synchronously with immediate offload
    ZE_CALL(zeCommandListAppendImageCopyFromMemory(context->Device->L0CommandListInit,
                                                   hImage, host_ptr, nullptr, nullptr));
  }

  *ret_image = L0PiImage;
  return PI_SUCCESS;
}

pi_result piextMemGetNativeHandle(pi_mem mem, pi_native_handle *nativeHandle) {
  die("piextMemGetNativeHandle: not supported");
  return PI_SUCCESS;
}

pi_result piextMemCreateWithNativeHandle(pi_native_handle nativeHandle,
                                         pi_mem *mem) {
  die("piextMemCreateWithNativeHandle: not supported");
  return PI_SUCCESS;
}

pi_result L0(piProgramCreate)(
  pi_context    context,
  const void *  il,
  size_t        length,
  pi_program *  program) {

  assert(context);
  assert(program);
  ze_device_handle_t ze_device = context->Device->L0Device;

  ze_module_desc_t ze_module_desc = {};
  ze_module_desc.version = ZE_MODULE_DESC_VERSION_CURRENT;
  ze_module_desc.format = ZE_MODULE_FORMAT_IL_SPIRV;
  ze_module_desc.inputSize = length;
  ze_module_desc.pInputModule = pi_cast<const uint8_t*>(il);
  ze_module_desc.pBuildFlags = nullptr;

  ze_module_handle_t ze_module;
  ZE_CALL(zeModuleCreate(
    ze_device,
    &ze_module_desc,
    &ze_module,
    0)); // TODO: handle build log

  auto L0PiProgram = new _pi_program(ze_module, context);
  *program = pi_cast<pi_program>(L0PiProgram);
  return PI_SUCCESS;
}

pi_result L0(piclProgramCreateWithBinary)(
  pi_context                     context,
  pi_uint32                      num_devices,
  const pi_device *              device_list,
  const size_t *                 lengths,
  const unsigned char **         binaries,
  pi_int32 *                     binary_status,
  pi_program *                   ret_program) {

  // This must be for the single device in this context.
  assert(num_devices == 1);
  assert(context);
  assert(ret_program);
  assert(device_list && device_list[0] == context->Device);
  ze_device_handle_t ze_device = context->Device->L0Device;

  // Check the binary too.
  assert(lengths && lengths[0] != 0);
  assert(binaries && binaries[0] != nullptr);
  size_t length = lengths[0];
  auto binary = pi_cast<const uint8_t*>(binaries[0]);

  ze_module_desc_t ze_module_desc = {};
  ze_module_desc.version = ZE_MODULE_DESC_VERSION_CURRENT;
  ze_module_desc.format = ZE_MODULE_FORMAT_NATIVE;
  ze_module_desc.inputSize = length;
  ze_module_desc.pInputModule = binary;
  ze_module_desc.pBuildFlags = nullptr;

  ze_module_handle_t ze_module;
  ZE_CALL(zeModuleCreate(
    ze_device,
    &ze_module_desc,
    &ze_module,
    0));

  auto L0PiProgram = new _pi_program(ze_module, context);
  *ret_program = pi_cast<pi_program>(L0PiProgram);

  if (binary_status) {
    *binary_status = PI_SUCCESS;
  }
  return PI_SUCCESS;
}

pi_result L0(piclProgramCreateWithSource)(
  pi_context        context,
  pi_uint32         count,
  const char **     strings,
  const size_t *    lengths,
  pi_program *      ret_program) {

  zePrint("piclProgramCreateWithSource: not supported in L0\n");
  return PI_INVALID_OPERATION;
}

pi_result L0(piProgramGetInfo)(
  pi_program          program,
  pi_program_info     param_name,
  size_t              param_value_size,
  void *              param_value,
  size_t *            param_value_size_ret) {

  assert(program);
  switch (param_name) {
  case PI_PROGRAM_INFO_REFERENCE_COUNT:
    SET_PARAM_VALUE(pi_uint32{program->RefCount});
    break;
  case PI_PROGRAM_INFO_NUM_DEVICES:
    // L0 Module is always for a single device.
    SET_PARAM_VALUE(pi_uint32{1});
    break;
  case PI_PROGRAM_INFO_DEVICES:
    SET_PARAM_VALUE(program->Context->Device);
    break;
  case PI_PROGRAM_INFO_BINARY_SIZES: {
    size_t szBinary = 0;
    ZE_CALL(zeModuleGetNativeBinary(program->L0Module, &szBinary, nullptr));
    // This is an array of 1 element, initialize if it were scalar.
    SET_PARAM_VALUE(size_t{szBinary});
    break;
  }
  case PI_PROGRAM_INFO_BINARIES: {
    size_t szBinary = 0;
    uint8_t **pBinary = pi_cast<uint8_t **>(param_value);
    ZE_CALL(zeModuleGetNativeBinary(program->L0Module, &szBinary, pBinary[0]));
    break;
  }
  case PI_PROGRAM_INFO_NUM_KERNELS: {
    uint32_t num_kernels = 0;
    ZE_CALL(zeModuleGetKernelNames(program->L0Module, &num_kernels, nullptr));
    SET_PARAM_VALUE(size_t{num_kernels});
    break;
  }
  case PI_PROGRAM_INFO_KERNEL_NAMES: {
    // There are extra allocations/copying here dictated by the difference
    // in L0 and PI interfaces. Also see discussions at
    // https://gitlab.devtools.intel.com/one-api/level_zero/issues/305.
    //
    uint32_t count = 0;
    ZE_CALL(zeModuleGetKernelNames(program->L0Module, &count, nullptr));
    char **pNames = new char *[count];
    ZE_CALL(zeModuleGetKernelNames(program->L0Module, &count,
                                   const_cast<const char **>(pNames)));
    std::string piNames{""};
    for (uint32_t i = 0; i < count; ++i) {
      piNames += (i > 0 ? ";" : "");
      piNames += pNames[i];
    }
    delete[] pNames;
    SET_PARAM_VALUE_STR(piNames.c_str());
    break;
  }
  default:
    die("piProgramGetInfo: not implemented");
  }

  return PI_SUCCESS;
}

pi_result L0(piProgramLink)(
  pi_context          context,
  pi_uint32           num_devices,
  const pi_device *   device_list,
  const char *        options,
  pi_uint32           num_input_programs,
  const pi_program *  input_programs,
  void (*  pfn_notify)(pi_program program,
                       void * user_data),
  void *              user_data,
  pi_program *        ret_program) {

  // TODO: L0 builds the program at the time of piProgramCreate.
  // But build options are not available at that time, so we must
  // stop building it there, but move it here. The problem though
  // is that this would mean moving zeModuleCreate here entirely,
  // and so L0 module creation would be deferred until
  // piProgramCompile/piProgramLink/piProgramBuild.
  //
  // See https://gitlab.devtools.intel.com/one-api/level_zero/issues/172
  //
  assert(num_input_programs == 1 && input_programs);
  assert(ret_program);
  *ret_program = input_programs[0];
  return PI_SUCCESS;
}

pi_result L0(piProgramCompile)(
  pi_program           program,
  pi_uint32            num_devices,
  const pi_device *    device_list,
  const char *         options,
  pi_uint32            num_input_headers,
  const pi_program *   input_headers,
  const char **        header_include_names,
  void (*  pfn_notify)(pi_program program, void * user_data),
  void *               user_data) {

  // TODO: L0 builds the program at the time of piProgramCreate.
  // But build options are not available at that time, so we must
  // stop building it there, but move it here. The problem though
  // is that this would mean moving zeModuleCreate here entirely,
  // and so L0 module creation would be deferred until
  // piProgramCompile/piProgramLink/piProgramBuild.
  //
  // See https://gitlab.devtools.intel.com/one-api/level_zero/issues/172
  //
  return PI_SUCCESS;
}

pi_result L0(piProgramBuild)(
  pi_program           program,
  pi_uint32            num_devices,
  const pi_device *    device_list,
  const char *         options,
  void (*  pfn_notify)(pi_program program, void * user_data),
  void *               user_data) {

  // TODO: L0 builds the program at the time of piProgramCreate.
  // But build options are not available at that time, so we must
  // stop building it there, but move it here. The problem though
  // is that this would mean moving zeModuleCreate here entirely,
  // and so L0 module creation would be deferred until
  // piProgramCompile/piProgramLink/piProgramBuild.
  //
  // See https://gitlab.devtools.intel.com/one-api/level_zero/issues/172
  //
  return PI_SUCCESS;
}

pi_result L0(piProgramGetBuildInfo)(
  pi_program              program,
  pi_device               device,
  cl_program_build_info   param_name,
  size_t                  param_value_size,
  void *                  param_value,
  size_t *                param_value_size_ret) {

  if (param_name == CL_PROGRAM_BINARY_TYPE) {
    // TODO: is this the only supported binary type in L0?
    // We should probably return CL_PROGRAM_BINARY_TYPE_NONE if asked
    // before the program was compiled.
    //
    SET_PARAM_VALUE(cl_program_binary_type{CL_PROGRAM_BINARY_TYPE_EXECUTABLE});
  }
  else if (param_name == CL_PROGRAM_BUILD_OPTIONS) {
    // TODO: how to get module build options out of L0?
    // For the programs that we compiled we can remember the options
    // passed with piProgramCompile/piProgramBuild, but what can we
    // return for programs that were built outside and registered
    // with piProgramRegister?
    //
    SET_PARAM_VALUE_STR("");
  }
  else {
    zePrint("piProgramGetBuildInfo: unsupported param_name\n");
    return PI_INVALID_VALUE;
  }
  return PI_SUCCESS;
}

pi_result L0(piProgramRetain)(pi_program program) {
  assert(program);
  ++(program->RefCount);
  return PI_SUCCESS;
}

pi_result L0(piProgramRelease)(pi_program program) {
  assert(program);
  if (--(program->RefCount) == 0) {
    // TODO: call zeModuleDestroy for non-interop L0 modules
    delete program;
  }
  return PI_SUCCESS;
}

pi_result piextProgramGetNativeHandle(pi_program program,
                                      pi_native_handle *nativeHandle) {
  assert(program);
  assert(nativeHandle);

  auto ze_module = pi_cast<ze_module_handle_t*>(nativeHandle);
  // Extract the L0 module handle from the given PI program
  *ze_module = program->L0Module;
  return PI_SUCCESS;
}

pi_result piextProgramCreateWithNativeHandle(pi_native_handle nativeHandle,
                                             pi_context context,
                                             pi_program *program) {
  assert(nativeHandle);
  assert(context);
  assert(program);

  auto ze_module = pi_cast<ze_module_handle_t*>(nativeHandle);
  assert(*ze_module);
  // Create PI program from the given L0 module handle
  auto L0PiProgram = new _pi_program(*ze_module, context);

  *program = pi_cast<pi_program>(L0PiProgram);
  return PI_SUCCESS;
}

pi_result L0(piKernelCreate)(
  pi_program      program,
  const char *    kernel_name,
  pi_kernel *     ret_kernel) {

  assert(program);
  assert(ret_kernel);
  assert(kernel_name);
  ze_kernel_desc_t ze_kernel_desc = {};
  ze_kernel_desc.version = ZE_KERNEL_DESC_VERSION_CURRENT;
  ze_kernel_desc.flags = ZE_KERNEL_FLAG_NONE;
  ze_kernel_desc.pKernelName = kernel_name;

  ze_kernel_handle_t ze_kernel;
  ZE_CALL(zeKernelCreate(
    pi_cast<ze_module_handle_t>(program->L0Module),
    &ze_kernel_desc,
    &ze_kernel));

  auto L0PiKernel = new _pi_kernel(ze_kernel, program);
  *ret_kernel = pi_cast<pi_kernel>(L0PiKernel);
  return PI_SUCCESS;
}

pi_result L0(piKernelSetArg)(
  pi_kernel    kernel,
  pi_uint32    arg_index,
  size_t       arg_size,
  const void * arg_value) {

  // OpenCL: "the arg_value pointer can be NULL or point to a NULL value
  // in which case a NULL value will be used as the value for the argument
  // declared as a pointer to global or constant memory in the kernel"
  //
  // We don't know the type of the argument but it seems that the only time
  // SYCL RT would send a pointer to NULL in 'arg_value' is when the argument
  // is a NULL pointer. Treat a pointer to NULL in 'arg_value' as a NULL.
  //
  if (arg_size == sizeof(void*) && arg_value &&
      *(void**)(const_cast<void*>(arg_value)) == nullptr) {
    arg_value = nullptr;
  }

  assert(kernel);
  ZE_CALL(zeKernelSetArgumentValue(
    pi_cast<ze_kernel_handle_t>(kernel->L0Kernel),
    pi_cast<uint32_t>(arg_index),
    pi_cast<size_t>(arg_size),
    pi_cast<const void*>(arg_value)));

  return PI_SUCCESS;
}

// Special version of piKernelSetArg to accept pi_mem and pi_sampler.
pi_result L0(piextKernelSetArgMemObj)(
  pi_kernel         kernel,
  pi_uint32         arg_index,
  const pi_mem *    arg_value)
{
  // TODO: the better way would probably be to add a new PI API for
  // extracting native PI object from PI handle, and have SYCL
  // RT pass that directly to the regular piKernelSetArg (and
  // then remove this piextKernelSetArgMemObj).
  //

  assert(kernel);
  ZE_CALL(zeKernelSetArgumentValue(
    pi_cast<ze_kernel_handle_t>(kernel->L0Kernel),
    pi_cast<uint32_t>(arg_index),
    sizeof(void *),
    (*arg_value)->getL0HandlePtr()));

  return PI_SUCCESS;
}

pi_result L0(piKernelGetInfo)(
  pi_kernel       kernel,
  pi_kernel_info  param_name,
  size_t          param_value_size,
  void *          param_value,
  size_t *        param_value_size_ret)
{
  assert(kernel);
  ze_kernel_properties_t ze_kernel_properties;
  ze_kernel_properties.version = ZE_KERNEL_PROPERTIES_VERSION_CURRENT;
  ZE_CALL(zeKernelGetProperties(kernel->L0Kernel, &ze_kernel_properties));

  switch (param_name) {
  case PI_KERNEL_INFO_CONTEXT:
    SET_PARAM_VALUE(pi_context{kernel->Program->Context});
    break;
  case PI_KERNEL_INFO_PROGRAM:
    SET_PARAM_VALUE(pi_program{kernel->Program});
    break;
  case PI_KERNEL_INFO_FUNCTION_NAME:
    SET_PARAM_VALUE_STR(ze_kernel_properties.name);
    break;
  case PI_KERNEL_INFO_NUM_ARGS:
    SET_PARAM_VALUE(pi_uint32{ze_kernel_properties.numKernelArgs});
    break;
  case PI_KERNEL_INFO_REFERENCE_COUNT:
    SET_PARAM_VALUE(pi_uint32{kernel->RefCount});
    break;
  case PI_KERNEL_INFO_ATTRIBUTES: {
    uint32_t size;
    ZE_CALL(zeKernelGetAttribute(
        kernel->L0Kernel, ZE_KERNEL_ATTR_SOURCE_ATTRIBUTE, &size, nullptr));
    char *attributes = new char[size];
    ZE_CALL(zeKernelGetAttribute(
        kernel->L0Kernel, ZE_KERNEL_ATTR_SOURCE_ATTRIBUTE, &size, attributes));
    SET_PARAM_VALUE_STR(attributes);
    delete[] attributes;
    break;
  }
  default:
    zePrint("Unsupported param_name in piKernelGetInfo: param_name=%d(0x%x)\n",
            param_name, param_name);
    return PI_INVALID_VALUE;
  }

  return PI_SUCCESS;
}

pi_result L0(piKernelGetGroupInfo)(
  pi_kernel                  kernel,
  pi_device                  device,
  pi_kernel_group_info       param_name,
  size_t                     param_value_size,
  void *                     param_value,
  size_t *                   param_value_size_ret)
{
  assert(kernel);
  assert(device);
  ze_device_handle_t ze_device = device->L0Device;
  ze_device_compute_properties_t ze_device_compute_properties;
  ze_device_compute_properties.version = ZE_DEVICE_COMPUTE_PROPERTIES_VERSION_CURRENT;
  ZE_CALL(zeDeviceGetComputeProperties(
    ze_device,
    &ze_device_compute_properties));

  ze_kernel_properties_t ze_kernel_properties;
  ze_kernel_properties.version = ZE_KERNEL_PROPERTIES_VERSION_CURRENT;
  ZE_CALL(zeKernelGetProperties(kernel->L0Kernel, &ze_kernel_properties));

  switch (param_name) {
  case PI_KERNEL_GROUP_INFO_GLOBAL_WORK_SIZE: {
    // TODO: To revisit after level_zero/issues/262 is resolved
    struct {
      size_t arr[3];
    } work_size = {{ze_device_compute_properties.maxGroupSizeX,
                    ze_device_compute_properties.maxGroupSizeY,
                    ze_device_compute_properties.maxGroupSizeZ}};
    SET_PARAM_VALUE(work_size);
    break;
  }
  case PI_KERNEL_GROUP_INFO_WORK_GROUP_SIZE: {
    uint32_t X, Y, Z;
    ZE_CALL(zeKernelSuggestGroupSize(kernel->L0Kernel, 10000, 10000, 10000, &X,
                                     &Y, &Z));
    SET_PARAM_VALUE(size_t{X * Y * Z});
    break;
  }
  case PI_KERNEL_GROUP_INFO_COMPILE_WORK_GROUP_SIZE: {
    struct {
      size_t arr[3];
    } wg_size = {{ze_kernel_properties.requiredGroupSizeX,
                  ze_kernel_properties.requiredGroupSizeY,
                  ze_kernel_properties.requiredGroupSizeZ}};
    SET_PARAM_VALUE(wg_size);
    break;
  }
  case PI_KERNEL_GROUP_INFO_LOCAL_MEM_SIZE: {
    // TODO: Assume 0 for now, replace with ze_kernel_properties_t::localMemSize
    // once released in RT. spec issue:
    // https://gitlab.devtools.intel.com/one-api/level_zero/issues/285
    SET_PARAM_VALUE(pi_uint32{0});
    break;
  }
  case PI_KERNEL_GROUP_INFO_PREFERRED_WORK_GROUP_SIZE_MULTIPLE: {
    ze_device_properties_t ze_device_properties;
    ze_device_properties.version = ZE_DEVICE_PROPERTIES_VERSION_CURRENT;
    ZE_CALL(zeDeviceGetProperties(ze_device, &ze_device_properties));

    SET_PARAM_VALUE(size_t{ze_device_properties.physicalEUSimdWidth});
    break;
  }
  case PI_KERNEL_GROUP_INFO_PRIVATE_MEM_SIZE:
    // TODO: Assume 0 for now, replace with
    // ze_kernel_properties_t::privateMemSize once released in RT. spec issue:
    // https://gitlab.devtools.intel.com/one-api/level_zero/issues/285
    SET_PARAM_VALUE(pi_uint32{0});
    break;
  default:
    zePrint("Unknown param_name in piKernelGetGroupInfo: param_name=%d(0x%x)\n",
            param_name, param_name);
    return PI_INVALID_VALUE;
  }
  return PI_SUCCESS;
}

pi_result L0(piKernelGetSubGroupInfo)(
  pi_kernel                   kernel,
  pi_device                   device,
  cl_kernel_sub_group_info    param_name, // TODO: untie from OpenCL
  size_t                      input_value_size,
  const void*                 input_value,
  size_t                      param_value_size,
  void*                       param_value,
  size_t*                     param_value_size_ret) {

  die("piKernelGetSubGroupInfo: not implemented");
  return {};
}

pi_result L0(piKernelRetain)(pi_kernel    kernel) {

  assert(kernel);
  ++(kernel->RefCount);
  return PI_SUCCESS;
}

pi_result L0(piKernelRelease)(pi_kernel    kernel) {

  assert(kernel);
  if (--(kernel->RefCount) == 0) {
    delete kernel;
  }
  return PI_SUCCESS;
}

pi_result L0(piEnqueueKernelLaunch)(
  pi_queue          queue,
  pi_kernel         kernel,
  pi_uint32         work_dim,
  const size_t *    global_work_offset,
  const size_t *    global_work_size,
  const size_t *    local_work_size,
  pi_uint32         num_events_in_wait_list,
  const pi_event *  event_wait_list,
  pi_event *        event)
{
  assert(kernel);
  assert(queue);
  assert(work_dim > 0);
  assert(work_dim < 4);

  ze_group_count_t thread_group_dimensions {1, 1, 1};
  uint32_t wg[3];

  // global_work_size of unused dimensions must be set to 1
  if (work_dim < 3) {
    assert(global_work_size[2] == 1);
  }
  if (work_dim < 2) {
    assert(global_work_size[1] == 1);
  }
  if (local_work_size) {
    wg[0]= pi_cast<uint32_t>(local_work_size[0]);
    wg[1]= pi_cast<uint32_t>(local_work_size[1]);
    wg[2]= pi_cast<uint32_t>(local_work_size[2]);
  } else {
    ZE_CALL(zeKernelSuggestGroupSize(kernel->L0Kernel, global_work_size[0],
             global_work_size[1], global_work_size[2], &wg[0], &wg[1], &wg[2]));
  }

  // TODO: assert if sizes do not fit into 32-bit?
  switch (work_dim) {
  case 3:
    thread_group_dimensions.groupCountX =
                                pi_cast<uint32_t>(global_work_size[0] / wg[0]);
    thread_group_dimensions.groupCountY =
                                pi_cast<uint32_t>(global_work_size[1] / wg[1]);
    thread_group_dimensions.groupCountZ =
                                pi_cast<uint32_t>(global_work_size[2] / wg[2]);
    break;
  case 2:
    thread_group_dimensions.groupCountX =
                                pi_cast<uint32_t>(global_work_size[0] / wg[0]);
    thread_group_dimensions.groupCountY =
                                pi_cast<uint32_t>(global_work_size[1] / wg[1]);
    wg[2] = 1;
    break;
  case 1:
    thread_group_dimensions.groupCountX =
                                pi_cast<uint32_t>(global_work_size[0] / wg[0]);
    wg[1] = wg[2] = 1;
    break;

  default:
    zePrint("piEnqueueKernelLaunch: unsupported work_dim\n");
    return PI_INVALID_VALUE;
  }

  assert(global_work_size[0] == (thread_group_dimensions.groupCountX * wg[0]));
  assert(global_work_size[1] == (thread_group_dimensions.groupCountY * wg[1]));
  assert(global_work_size[2] == (thread_group_dimensions.groupCountZ * wg[2]));

  ZE_CALL(zeKernelSetGroupSize(kernel->L0Kernel, wg[0], wg[1], wg[2]));

  // Get a new command list to be used on this call
  ze_command_list_handle_t ze_command_list = nullptr;
  if (auto res = queue->Context->Device->createCommandList(&ze_command_list))
    return res;

  auto res = L0(piEventCreate)(kernel->Program->Context, event);
  if (res != PI_SUCCESS)
    return res;

  (*event)->Queue = queue;
  (*event)->CommandType = PI_COMMAND_TYPE_NDRANGE_KERNEL;
  (*event)->L0CommandList = ze_command_list;

  ze_event_handle_t ze_event = (*event)->L0Event;

  ze_event_handle_t *ze_event_wait_list =
    _pi_event::createL0EventList(num_events_in_wait_list, event_wait_list);

  // Add the command to the command list
  ZE_CALL(zeCommandListAppendLaunchKernel(
    ze_command_list,
    kernel->L0Kernel,
    &thread_group_dimensions,
    ze_event,
    num_events_in_wait_list,
    ze_event_wait_list));

  zePrint("calling zeCommandListAppendLaunchKernel() with"
                  "  ze_event %lx\n"
                  "  num_events_in_wait_list %d:",
          pi_cast<std::uintptr_t>(ze_event), num_events_in_wait_list);
  for (pi_uint32 i = 0; i < num_events_in_wait_list; i++) {
    zePrint(" %lx", pi_cast<std::uintptr_t>(ze_event_wait_list[i]));
  }
  zePrint("\n");

  // Execute command list asynchronously, as the event will be used
  // to track down its completion.
  if (auto res = queue->executeCommandList(ze_command_list))
    return res;

  _pi_event::deleteL0EventList(ze_event_wait_list);

  return PI_SUCCESS;
}

//
// Events
//
pi_result L0(piEventCreate)(
  pi_context    context,
  pi_event *    ret_event)
{
  size_t index = 0;
  ze_event_pool_handle_t ze_event_pool = {};
  ZE_CALL(context->getFreeSlotInExistingOrNewPool(ze_event_pool, index));
  ze_event_handle_t ze_event;
  ze_event_desc_t ze_event_desc = {};
  ze_event_desc.signal = ZE_EVENT_SCOPE_FLAG_NONE;
  ze_event_desc.wait = ZE_EVENT_SCOPE_FLAG_NONE;
  ze_event_desc.version = ZE_EVENT_DESC_VERSION_CURRENT;
  ze_event_desc.index = index;

  ZE_CALL(zeEventCreate(
    ze_event_pool,
    &ze_event_desc,
    &ze_event));

  *ret_event =
      new _pi_event(ze_event, ze_event_pool, context, PI_COMMAND_TYPE_USER);
  return PI_SUCCESS;
}

pi_result L0(piEventGetInfo)(
  pi_event         event,
  pi_event_info    param_name,
  size_t           param_value_size,
  void *           param_value,
  size_t *         param_value_size_ret) {

  assert(event);
  switch (param_name) {
  case PI_EVENT_INFO_COMMAND_QUEUE:
    SET_PARAM_VALUE(pi_queue{event->Queue});
    break;
  case PI_EVENT_INFO_CONTEXT:
    SET_PARAM_VALUE(pi_context{event->Queue->Context});
    break;
  case PI_EVENT_INFO_COMMAND_TYPE:
    SET_PARAM_VALUE(pi_cast<pi_uint64>(event->CommandType));
    break;
  case PI_EVENT_INFO_COMMAND_EXECUTION_STATUS: {
    ze_result_t ze_result;
    ze_result = ZE_CALL_NOCHECK(zeEventQueryStatus(event->L0Event));
    if (ze_result == ZE_RESULT_SUCCESS) {
      SET_PARAM_VALUE(pi_int32{CL_COMPLETE}); // Untie from OpenCL
    } else {
      // TODO: We don't know if the status is queueed, submitted or running.
      //       See
      //       https://gitlab.devtools.intel.com/one-api/level_zero/issues/243
      //       For now return "running", as others are unlikely to be of
      //       interest.
      SET_PARAM_VALUE(pi_int32{CL_RUNNING});
    }
    break;
  }
  case PI_EVENT_INFO_REFERENCE_COUNT:
    SET_PARAM_VALUE(pi_uint32{event->RefCount});
    break;
  default:
    zePrint("Unsupported param_name in piEventGetInfo: param_name=%d(%x)\n",
            param_name, param_name);
    return PI_INVALID_VALUE;
  }

  return PI_SUCCESS;
}

pi_result L0(piEventGetProfilingInfo)(
  pi_event            event,
  pi_profiling_info   param_name,
  size_t              param_value_size,
  void *              param_value,
  size_t *            param_value_size_ret) {

  assert(event);
  uint64_t L0TimerResolution =
      event->Queue->Context->Device->L0DeviceProperties.timerResolution;

  if (param_name == PI_PROFILING_INFO_COMMAND_START) {
    uint64_t context_start;
    ZE_CALL(zeEventGetTimestamp(
        event->L0Event, ZE_EVENT_TIMESTAMP_CONTEXT_START, &context_start));
    context_start *= L0TimerResolution;
    SET_PARAM_VALUE(uint64_t{context_start});
  }
  else if (param_name == PI_PROFILING_INFO_COMMAND_END) {
    uint64_t context_end;
    ZE_CALL(zeEventGetTimestamp(
        event->L0Event, ZE_EVENT_TIMESTAMP_CONTEXT_END, &context_end));
    context_end *= L0TimerResolution;
    SET_PARAM_VALUE(uint64_t{context_end});
  }
  else if (param_name == PI_PROFILING_INFO_COMMAND_QUEUED ||
           param_name == PI_PROFILING_INFO_COMMAND_SUBMIT) {
    // TODO: Support these when L0 supported is added.
    // https://gitlab.devtools.intel.com/one-api/level_zero/issues/373
    SET_PARAM_VALUE(uint64_t{0});
  }
  else {
    zePrint("piEventGetProfilingInfo: not supported param_name\n");
    return PI_INVALID_VALUE;
  }

  return PI_SUCCESS;
}

pi_result L0(piEventsWait)(
  pi_uint32           num_events,
  const pi_event *    event_list)
{
  ze_result_t ze_result;

  if (num_events && !event_list) {
    return PI_INVALID_EVENT;
  }

  for (uint32_t i = 0; i < num_events; i++) {

    ze_event_handle_t ze_event = event_list[i]->L0Event;
    zePrint("ze_event = %lx\n", pi_cast<std::uintptr_t>(ze_event));
    // TODO: Using UINT32_MAX for timeout should have the desired
    // effect of waiting until the event is trigerred, but it seems that
    // it is causing an OS crash, so use an interruptable loop for now.
    //
    do {
      ze_result = ZE_CALL_NOCHECK(zeEventHostSynchronize(ze_event, 100000));
    } while (ze_result == ZE_RESULT_NOT_READY);

    // Check the result to be success.
    ZE_CALL(ze_result);

    // NOTE: we are destroying associated command lists here to free
    // resources sooner in case RT is not calling piEventRelease soon enough.
    //
    if (event_list[i]->L0CommandList) {
      // Event has been signaled: Destroy the command list associated with the
      // call that generated the event.
      ZE_CALL(zeCommandListDestroy(event_list[i]->L0CommandList));
      event_list[i]->L0CommandList = nullptr;
    }
  }
  return PI_SUCCESS;
}

pi_result L0(piEventSetCallback)(
  pi_event    event,
  pi_int32    command_exec_callback_type,
  void (*     pfn_notify)(pi_event event,
                          pi_int32 event_command_status,
                          void *   user_data),
  void *      user_data) {

  // Increment the pi_event's reference counter to avoid destroying the event before all
  // callbacks are executed.
  piEventRetain(event);

  // TODO: Can we support CL_SUBMITTED and CL_RUNNING?
  // See https://gitlab.devtools.intel.com/one-api/level_zero/issues/243
  //
  if (command_exec_callback_type != CL_COMPLETE) {
    zePrint("piEventSetCallback: unsupported callback type\n");
    return PI_INVALID_VALUE;
  }

  // Execute the wait and callback trigger in a side thread to not
  // block the main host thread.
  // TODO: We should use a single thread to serve all callbacks.
  //
  std::thread wait_thread([](pi_event    event,
                             pi_int32    command_exec_callback_type,
                             void (*     pfn_notify)(pi_event event,
                                                     pi_int32 event_command_status,
                                                     void *   user_data),
                             void *      user_data) {

    // Implements the wait for the event to complete.
    assert(command_exec_callback_type == CL_COMPLETE);
    assert(event);
    ze_result_t ze_result;
    do {
      ze_result =
        ZE_CALL_NOCHECK(zeEventHostSynchronize(event->L0Event, 10000));
    } while (ze_result == ZE_RESULT_NOT_READY);

    // Call the callback.
    pfn_notify(event, command_exec_callback_type, user_data);
    piEventRelease(event);

  }, event, command_exec_callback_type, pfn_notify, user_data);

  wait_thread.detach();
  return PI_SUCCESS;
}

pi_result L0(piEventSetStatus)(
  pi_event   event,
  pi_int32   execution_status)
{
  if (execution_status != CL_COMPLETE) {
    // For everything else see https://gitlab.devtools.intel.com/
    // one-api/level_zero/issues/291
    die("piEventSetStatus: not implemented");
  }

  assert(event);
  ze_result_t ze_result;
  ze_event_handle_t ze_event = event->L0Event;

  ze_result = ZE_CALL_NOCHECK(zeEventQueryStatus(ze_event));
  // It can be that the status is already what we need it to be.
  if (ze_result != ZE_RESULT_SUCCESS) {
    ZE_CALL(zeEventHostSignal(ze_event));
    ZE_CALL(zeEventQueryStatus(ze_event)); // double check
  }
  return PI_SUCCESS;
}

pi_result L0(piEventRetain)(pi_event event) {
  ++(event->RefCount);
  return PI_SUCCESS;
}

pi_result L0(piEventRelease)(pi_event event) {
  assert(event);
  if (--(event->RefCount) == 0) {
    if (event->L0CommandList) {
      // Destroy the command list associated with the call that generated
      // the event.
      //
      ZE_CALL(zeCommandListDestroy(event->L0CommandList));
      event->L0CommandList = nullptr;
    }
    if (event->CommandType == PI_COMMAND_TYPE_MEM_BUFFER_UNMAP &&
        event->CommandData) {
      // Free the memory allocated in the piEnqueueMemBufferMap.
      ZE_CALL(zeDriverFreeMem(
          event->Queue->Context->Device->Platform->L0Driver, event->CommandData));
      event->CommandData = nullptr;
    }
    ZE_CALL(zeEventDestroy(event->L0Event));

    auto context = event->Context;
    ZE_CALL(context->decrementAliveEventsInPool(event->L0EventPool));

    delete event;
  }
  return PI_SUCCESS;
}

pi_result piextEventGetNativeHandle(pi_event event,
                                    pi_native_handle *nativeHandle) {
  die("piextEventGetNativeHandle: not supported");
  return PI_SUCCESS;
}

pi_result piextEventCreateWithNativeHandle(pi_native_handle nativeHandle,
                                           pi_event *event) {
  die("piextEventCreateWithNativeHandle: not supported");
  return PI_SUCCESS;
}

//
// Sampler
//
pi_result L0(piSamplerCreate)(
  pi_context                     context,
  const pi_sampler_properties *  sampler_properties,
  pi_sampler *                   ret_sampler) {

  assert(context);
  assert(ret_sampler);

  ze_device_handle_t ze_device = context->Device->L0Device;

  ze_sampler_handle_t ze_sampler;
  ze_sampler_desc_t ze_sampler_desc = {};
  ze_sampler_desc.version = ZE_SAMPLER_DESC_VERSION_CURRENT;

  // Set the default values for the ze_sampler_desc.
  ze_sampler_desc.isNormalized = PI_TRUE;
  ze_sampler_desc.addressMode = ZE_SAMPLER_ADDRESS_MODE_CLAMP;
  ze_sampler_desc.filterMode = ZE_SAMPLER_FILTER_MODE_NEAREST;

  // Update the values of the ze_sampler_desc from the pi_sampler_properties list.
  // Default values will be used if any of the following is true:
  //   a) sampler_properties list is NULL
  //   b) sampler_properties list is missing any properties

  if (sampler_properties) {
    const pi_sampler_properties * cur_property = sampler_properties;

    while (*cur_property != 0) {
      switch (*cur_property) {
        case PI_SAMPLER_PROPERTIES_NORMALIZED_COORDS:
          {
            pi_bool cur_value_bool = pi_cast<pi_bool>(*(++cur_property));

            if (cur_value_bool == PI_TRUE)
              ze_sampler_desc.isNormalized = PI_TRUE;
            else if (cur_value_bool == PI_FALSE)
              ze_sampler_desc.isNormalized = PI_FALSE;
            else {
              zePrint("piSamplerCreate: unsupported "
                      "PI_SAMPLER_NORMALIZED_COORDS value\n");
              return PI_INVALID_VALUE;
            }
          }
          break;

        case PI_SAMPLER_PROPERTIES_ADDRESSING_MODE:
          {
            pi_sampler_addressing_mode cur_value_addressing_mode =
              pi_cast<pi_sampler_addressing_mode>(
                pi_cast<pi_uint32>(*(++cur_property)));

            // TODO: add support for PI_SAMPLER_ADDRESSING_MODE_CLAMP_TO_EDGE
            switch (cur_value_addressing_mode) {
            case PI_SAMPLER_ADDRESSING_MODE_NONE:
              ze_sampler_desc.addressMode = ZE_SAMPLER_ADDRESS_MODE_NONE;
              break;
            case PI_SAMPLER_ADDRESSING_MODE_REPEAT:
              ze_sampler_desc.addressMode = ZE_SAMPLER_ADDRESS_MODE_REPEAT;
              break;
            case PI_SAMPLER_ADDRESSING_MODE_CLAMP:
              ze_sampler_desc.addressMode = ZE_SAMPLER_ADDRESS_MODE_CLAMP;
              break;
            case PI_SAMPLER_ADDRESSING_MODE_CLAMP_TO_EDGE:
              ze_sampler_desc.addressMode =
                  ZE_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
              break;
            case PI_SAMPLER_ADDRESSING_MODE_MIRRORED_REPEAT:
              ze_sampler_desc.addressMode = ZE_SAMPLER_ADDRESS_MODE_MIRROR;
              break;
            default:
              zePrint("piSamplerCreate: unsupported PI_SAMPLER_ADDRESSING_MODE "
                      "value\n");
              zePrint("PI_SAMPLER_ADDRESSING_MODE=%d\n",
                      cur_value_addressing_mode);
              return PI_INVALID_VALUE;
            }
          }
          break;

        case PI_SAMPLER_PROPERTIES_FILTER_MODE:
          {
            pi_sampler_filter_mode cur_value_filter_mode =
              pi_cast<pi_sampler_filter_mode>(
                pi_cast<pi_uint32>(*(++cur_property)));

            if (cur_value_filter_mode == PI_SAMPLER_FILTER_MODE_NEAREST)
              ze_sampler_desc.filterMode = ZE_SAMPLER_FILTER_MODE_NEAREST;
            else if (cur_value_filter_mode == PI_SAMPLER_FILTER_MODE_LINEAR)
              ze_sampler_desc.filterMode = ZE_SAMPLER_FILTER_MODE_LINEAR;
            else {
              zePrint("PI_SAMPLER_FILTER_MODE=%d\n", cur_value_filter_mode);
              zePrint("piSamplerCreate: unsupported PI_SAMPLER_FILTER_MODE value\n");
              return PI_INVALID_VALUE;
            }
          }
          break;

        default:
          break;
      }
      cur_property++;
    }
  }

  ZE_CALL(zeSamplerCreate(
    ze_device,
    &ze_sampler_desc,  // TODO: translate properties
    &ze_sampler));

  *ret_sampler = new _pi_sampler(ze_sampler);
  return PI_SUCCESS;
}

pi_result L0(piSamplerGetInfo)(
  pi_sampler         sampler,
  pi_sampler_info    param_name,
  size_t             param_value_size,
  void *             param_value,
  size_t *           param_value_size_ret) {

  die("piSamplerGetInfo: not implemented");
  return {};
}

pi_result L0(piSamplerRetain)(pi_sampler sampler) {
  assert(sampler);
  ++(sampler->RefCount);
  return PI_SUCCESS;
}

pi_result L0(piSamplerRelease)(pi_sampler sampler) {
  assert(sampler);
  if (--(sampler->RefCount) == 0) {
    ZE_CALL(zeSamplerDestroy(sampler->L0Sampler));
    delete sampler;
  }
  return PI_SUCCESS;
}

//
// Queue Commands
//
pi_result L0(piEnqueueEventsWait)(
  pi_queue          queue,
  pi_uint32         num_events_in_wait_list,
  const pi_event *  event_wait_list,
  pi_event *        event) {

  die("piEnqueueEventsWait: not implemented");
  return {};
}

pi_result L0(piEnqueueMemBufferRead)(
  pi_queue            queue,
  pi_mem              src,
  pi_bool             blocking_read,
  size_t              offset,
  size_t              size,
  void *              dst,
  pi_uint32           num_events_in_wait_list,
  const pi_event *    event_wait_list,
  pi_event *          event)
{
  assert(src);
  return enqueueMemCopyHelper(
    PI_COMMAND_TYPE_MEM_BUFFER_READ,
    queue,
    dst,
    blocking_read,
    size,
    pi_cast<char *>(src->getL0Handle()) + offset,
    num_events_in_wait_list,
    event_wait_list,
    event);
}

pi_result L0(piEnqueueMemBufferReadRect)(
  pi_queue            queue,
  pi_mem              buffer,
  pi_bool             blocking_read,
  const size_t *      buffer_offset,
  const size_t *      host_offset,
  const size_t *      region,
  size_t              buffer_row_pitch,
  size_t              buffer_slice_pitch,
  size_t              host_row_pitch,
  size_t              host_slice_pitch,
  void *              ptr,
  pi_uint32           num_events_in_wait_list,
  const pi_event *    event_wait_list,
  pi_event *          event) {

  assert(buffer);
  return enqueueMemCopyRectHelper(
    PI_COMMAND_TYPE_MEM_BUFFER_READ_RECT,
    queue,
    buffer->getL0Handle(),
    static_cast<char *>(ptr),
    buffer_offset,
    host_offset,
    region,
    buffer_row_pitch,
    host_row_pitch,
    buffer_slice_pitch,
    host_slice_pitch,
    blocking_read,
    num_events_in_wait_list,
    event_wait_list,
    event);
}

// Shared by all memory read/write/copy PI interfaces.
static pi_result enqueueMemCopyHelper(
  pi_command_type    command_type,
  pi_queue           queue,
  void *             dst,
  pi_bool            blocking_write,
  size_t             size,
  const void *       src,
  pi_uint32          num_events_in_wait_list,
  const pi_event *   event_wait_list,
  pi_event *         event) {

  assert(queue);
  // Get a new command list to be used on this call
  ze_command_list_handle_t ze_command_list = nullptr;
  if (auto res = queue->Context->Device->createCommandList(&ze_command_list))
    return res;

  auto res = L0(piEventCreate)(queue->Context, event);
  if (res != PI_SUCCESS)
    return res;

  (*event)->Queue = queue;
  (*event)->CommandType = command_type;
  (*event)->L0CommandList = ze_command_list;

  ze_event_handle_t ze_event = (*event)->L0Event;

  ze_event_handle_t *ze_event_wait_list =
    _pi_event::createL0EventList(num_events_in_wait_list, event_wait_list);

  ZE_CALL(zeCommandListAppendWaitOnEvents(
    ze_command_list,
    num_events_in_wait_list,
    ze_event_wait_list
  ));

  ZE_CALL(zeCommandListAppendMemoryCopy(
    ze_command_list,
    dst,
    src,
    size,
    ze_event
  ));

  if (auto res = queue->executeCommandList(ze_command_list, blocking_write))
    return res;

  zePrint("calling zeCommandListAppendMemoryCopy() with\n"
                  "  xe_event %lx\n"
                  "  num_events_in_wait_list %d:",
          pi_cast<std::uintptr_t>(ze_event), num_events_in_wait_list);
  for (pi_uint32 i = 0; i < num_events_in_wait_list; i++) {
    zePrint(" %lx", pi_cast<std::uintptr_t>(ze_event_wait_list[i]));
  }
  zePrint("\n");

  _pi_event::deleteL0EventList(ze_event_wait_list);

  return PI_SUCCESS;
}

// Shared by all memory read/write/copy rect PI interfaces.
static pi_result enqueueMemCopyRectHelper(
  pi_command_type    command_type,
  pi_queue           queue,
  void *             src_buffer,
  void *             dst_buffer,
  const size_t *     src_origin,
  const size_t *     dst_origin,
  const size_t *     region,
  size_t             src_row_pitch,
  size_t             dst_row_pitch,
  size_t             src_slice_pitch,
  size_t             dst_slice_pitch,
  pi_bool            blocking,
  pi_uint32          num_events_in_wait_list,
  const pi_event *   event_wait_list,
  pi_event *         event) {

  assert(region);
  assert(src_origin);
  assert(dst_origin);
  assert(queue);

  // Get a new command list to be used on this call
  ze_command_list_handle_t ze_command_list = nullptr;
  if (auto res = queue->Context->Device->createCommandList(&ze_command_list))
    return res;

  auto res = L0(piEventCreate)(queue->Context, event);
  if (res != PI_SUCCESS)
    return res;

  (*event)->Queue = queue;
  (*event)->CommandType = command_type;
  (*event)->L0CommandList = ze_command_list;

  ze_event_handle_t ze_event = (*event)->L0Event;

  ze_event_handle_t *ze_event_wait_list =
    _pi_event::createL0EventList(num_events_in_wait_list, event_wait_list);

  ZE_CALL(zeCommandListAppendWaitOnEvents(
    ze_command_list,
    num_events_in_wait_list,
    ze_event_wait_list
  ));

  zePrint("calling zeCommandListAppendWaitOnEvents() with\n"
                "  num_events_in_wait_list %d:",
                pi_cast<std::uintptr_t>(ze_event), num_events_in_wait_list);
  for (pi_uint32 i = 0; i < num_events_in_wait_list; i++) {
    zePrint(" %lx", pi_cast<std::uintptr_t>(ze_event_wait_list[i]));
  }
  zePrint("\n");

  uint32_t srcOriginX = pi_cast<uint32_t>(src_origin[0]);
  uint32_t srcOriginY = pi_cast<uint32_t>(src_origin[1]);
  uint32_t srcOriginZ = pi_cast<uint32_t>(src_origin[2]);

  uint32_t srcPitch = src_row_pitch;
  if (srcPitch == 0)
    srcPitch = pi_cast<uint32_t>(region[0]);

  if (src_slice_pitch == 0)
    src_slice_pitch = pi_cast<uint32_t>(region[1]) * srcPitch;

  uint32_t dstOriginX = pi_cast<uint32_t>(dst_origin[0]);
  uint32_t dstOriginY = pi_cast<uint32_t>(dst_origin[1]);
  uint32_t dstOriginZ = pi_cast<uint32_t>(dst_origin[2]);

  uint32_t dstPitch = dst_row_pitch;
  if (dstPitch == 0)
    dstPitch = pi_cast<uint32_t>(region[0]);

  if (dst_slice_pitch == 0)
    dst_slice_pitch = pi_cast<uint32_t>(region[1]) * dstPitch;

  uint32_t width = pi_cast<uint32_t>(region[0]);
  uint32_t height = pi_cast<uint32_t>(region[1]);
  uint32_t depth = pi_cast<uint32_t>(region[2]);

  const ze_copy_region_t srcRegion = {srcOriginX, srcOriginY, srcOriginZ, width, height, depth};
  const ze_copy_region_t dstRegion = {dstOriginX, dstOriginY, dstOriginZ, width, height, depth};

  ZE_CALL(zeCommandListAppendMemoryCopyRegion(
    ze_command_list,
    dst_buffer,
    &dstRegion,
    dstPitch,
    dst_slice_pitch,
    src_buffer,
    &srcRegion,
    srcPitch,
    src_slice_pitch,
    nullptr
  ));

  zePrint("calling zeCommandListAppendMemoryCopyRegion()\n");

  ZE_CALL(zeCommandListAppendBarrier(
    ze_command_list,
    ze_event,
    0,
    nullptr
  ));

  zePrint("calling zeCommandListAppendBarrier() with event %lx\n",
    pi_cast<std::uintptr_t>(ze_event));

  if (auto res = queue->executeCommandList(ze_command_list, blocking))
    return res;

  _pi_event::deleteL0EventList(ze_event_wait_list);

  return PI_SUCCESS;
}

pi_result L0(piEnqueueMemBufferWrite)(
  pi_queue           queue,
  pi_mem             buffer,
  pi_bool            blocking_write,
  size_t             offset,
  size_t             size,
  const void *       ptr,
  pi_uint32          num_events_in_wait_list,
  const pi_event *   event_wait_list,
  pi_event *         event) {

  assert(buffer);
  return enqueueMemCopyHelper(
    PI_COMMAND_TYPE_MEM_BUFFER_WRITE,
    queue,
    pi_cast<char *>(buffer->getL0Handle()) + offset, // dst
    blocking_write,
    size,
    ptr, // src
    num_events_in_wait_list,
    event_wait_list,
    event);
}

pi_result L0(piEnqueueMemBufferWriteRect)(
  pi_queue            queue,
  pi_mem              buffer,
  pi_bool             blocking_write,
  const size_t *      buffer_offset,
  const size_t *      host_offset,
  const size_t *      region,
  size_t              buffer_row_pitch,
  size_t              buffer_slice_pitch,
  size_t              host_row_pitch,
  size_t              host_slice_pitch,
  const void *        ptr,
  pi_uint32           num_events_in_wait_list,
  const pi_event *    event_wait_list,
  pi_event *          event) {

  assert(buffer);
  return enqueueMemCopyRectHelper(
    PI_COMMAND_TYPE_MEM_BUFFER_WRITE_RECT,
    queue,
    const_cast<char *>(static_cast<const char *>(ptr)),
    buffer->getL0Handle(),
    host_offset,
    buffer_offset,
    region,
    host_row_pitch,
    buffer_row_pitch,
    host_slice_pitch,
    buffer_slice_pitch,
    blocking_write,
    num_events_in_wait_list,
    event_wait_list,
    event);
}

pi_result L0(piEnqueueMemBufferCopy)(
  pi_queue            queue,
  pi_mem              src_buffer,
  pi_mem              dst_buffer,
  size_t              src_offset,
  size_t              dst_offset,
  size_t              size,
  pi_uint32           num_events_in_wait_list,
  const pi_event *    event_wait_list,
  pi_event *          event) {

  assert(src_buffer);
  assert(dst_buffer);
  return enqueueMemCopyHelper(
    PI_COMMAND_TYPE_MEM_BUFFER_COPY,
    queue,
    pi_cast<char *>(dst_buffer->getL0Handle()) + dst_offset,
    false, // blocking
    size,
    pi_cast<char *>(src_buffer->getL0Handle()) + src_offset,
    num_events_in_wait_list,
    event_wait_list,
    event);
}

pi_result L0(piEnqueueMemBufferCopyRect)(
  pi_queue            queue,
  pi_mem              src_buffer,
  pi_mem              dst_buffer,
  const size_t *      src_origin,
  const size_t *      dst_origin,
  const size_t *      region,
  size_t              src_row_pitch,
  size_t              src_slice_pitch,
  size_t              dst_row_pitch,
  size_t              dst_slice_pitch,
  pi_uint32           num_events_in_wait_list,
  const pi_event *    event_wait_list,
  pi_event *          event) {

  assert(src_buffer);
  assert(dst_buffer);
  return enqueueMemCopyRectHelper(
    PI_COMMAND_TYPE_MEM_BUFFER_COPY_RECT,
    queue,
    src_buffer->getL0Handle(),
    dst_buffer->getL0Handle(),
    src_origin,
    dst_origin,
    region,
    src_row_pitch,
    dst_row_pitch,
    src_slice_pitch,
    dst_slice_pitch,
    false, // blocking
    num_events_in_wait_list,
    event_wait_list,
    event);
}

static pi_result enqueueMemFillHelper(
  pi_command_type    command_type,
  pi_queue           queue,
  void *             ptr,
  const void *       pattern,
  size_t             pattern_size,
  size_t             size,
  pi_uint32          num_events_in_wait_list,
  const pi_event *   event_wait_list,
  pi_event *         event) {

  assert(queue);
  // Get a new command list to be used on this call
  ze_command_list_handle_t ze_command_list = nullptr;
  if (auto res = queue->Context->Device->createCommandList(&ze_command_list))
    return res;

  auto res = L0(piEventCreate)(queue->Context, event);
  if (res != PI_SUCCESS)
    return res;

  (*event)->Queue = queue;
  (*event)->CommandType = command_type;
  (*event)->L0CommandList = ze_command_list;

  ze_event_handle_t ze_event = (*event)->L0Event;

  ze_event_handle_t *ze_event_wait_list =
    _pi_event::createL0EventList(num_events_in_wait_list, event_wait_list);

  ZE_CALL(zeCommandListAppendWaitOnEvents(
    ze_command_list,
    num_events_in_wait_list,
    ze_event_wait_list
  ));

  // pattern size must be a power of two
  assert((pattern_size > 0) && ((pattern_size & (pattern_size - 1)) == 0));

  ZE_CALL(zeCommandListAppendMemoryFill(
    ze_command_list,
    ptr,
    pattern,
    pattern_size,
    size,
    ze_event
  ));

  zePrint("calling zeCommandListAppendMemoryFill() with\n"
                  "  xe_event %lx\n"
                  "  num_events_in_wait_list %d:",
      pi_cast<pi_uint64>(ze_event), num_events_in_wait_list);
  for (pi_uint32 i = 0; i < num_events_in_wait_list; i++) {
    zePrint(" %lx", pi_cast<pi_uint64>(ze_event_wait_list[i]));
  }
  zePrint("\n");

  // Execute command list asynchronously, as the event will be used
  // to track down its completion.
  if (auto res = queue->executeCommandList(ze_command_list))
    return res;

  _pi_event::deleteL0EventList(ze_event_wait_list);

  return PI_SUCCESS;
}

pi_result L0(piEnqueueMemBufferFill)(
  pi_queue           queue,
  pi_mem             buffer,
  const void *       pattern,
  size_t             pattern_size,
  size_t             offset,
  size_t             size,
  pi_uint32          num_events_in_wait_list,
  const pi_event *   event_wait_list,
  pi_event *         event) {

  assert(buffer);
  return enqueueMemFillHelper(
    PI_COMMAND_TYPE_MEM_BUFFER_FILL,
    queue,
    pi_cast<char *>(buffer->getL0Handle()) + offset,
    pattern,
    pattern_size,
    size,
    num_events_in_wait_list,
    event_wait_list,
    event);
}

pi_result L0(piEnqueueMemBufferMap)(
  pi_queue          queue,
  pi_mem            buffer,
  pi_bool           blocking_map,
  cl_map_flags      map_flags,  // TODO: untie from OpenCL
  size_t            offset,
  size_t            size,
  pi_uint32         num_events_in_wait_list,
  const pi_event *  event_wait_list,
  pi_event *        event,
  void **           ret_map) {

  // TODO: we don't implement read-only or write-only, always read-write.
  // assert((map_flags & CL_MAP_READ) != 0);
  // assert((map_flags & CL_MAP_WRITE) != 0);
  assert(queue);
  assert(buffer);

  // Get a new command list to be used on this call
  ze_command_list_handle_t ze_command_list = nullptr;
  if (auto res = queue->Context->Device->createCommandList(&ze_command_list))
    return res;

  auto res = L0(piEventCreate)(queue->Context, event);
  if (res != PI_SUCCESS)
    return res;

  (*event)->Queue = queue;
  (*event)->CommandType = PI_COMMAND_TYPE_MEM_BUFFER_MAP;
  (*event)->L0CommandList = ze_command_list;

  ze_event_handle_t *ze_event_wait_list =
    _pi_event::createL0EventList(num_events_in_wait_list, event_wait_list);

  ZE_CALL(zeCommandListAppendWaitOnEvents(
    ze_command_list,
    num_events_in_wait_list,
    ze_event_wait_list
  ));

  // TODO: L0 is missing the memory "mapping" capabilities, so we are left
  // to doing new memory allocation and a copy (read).
  // See https://gitlab.devtools.intel.com/one-api/level_zero/issues/293.
  //
  // TODO: check if the input buffer is already allocated in shared
  // memory and thus is accessible from the host as is. Can we get SYCL RT
  // to predict/allocate in shared memory from the beginning?
  //
  if (buffer->MapHostPtr) {
    // NOTE: borrowing below semantics from OpenCL as SYCL RT relies on it.
    // It is also better for performance.
    //
    // "If the buffer object is created with CL_MEM_USE_HOST_PTR set in
    // mem_flags, the following will be true:
    // - The host_ptr specified in clCreateBuffer is guaranteed to contain the
    //   latest bits in the region being mapped when the clEnqueueMapBuffer
    //   command has completed.
    // - The pointer value returned by clEnqueueMapBuffer will be derived from
    //   the host_ptr specified when the buffer object is created."
    //
    *ret_map = buffer->MapHostPtr + offset;
  }
  else {
    ze_host_mem_alloc_desc_t ze_desc = {};
    ze_desc.flags = ZE_HOST_MEM_ALLOC_FLAG_DEFAULT;
    ZE_CALL(zeDriverAllocHostMem(
      queue->Context->Device->Platform->L0Driver,
      &ze_desc,
      size,
      1, // TODO: alignment
      ret_map));
  }

  ze_event_handle_t ze_event = (*event)->L0Event;
  ZE_CALL(zeCommandListAppendMemoryCopy(
    ze_command_list,
    *ret_map,
    pi_cast<char *>(buffer->getL0Handle()) + offset,
    size,
    ze_event
  ));

  if (auto res = queue->executeCommandList(ze_command_list, blocking_map))
    return res;

  _pi_event::deleteL0EventList(ze_event_wait_list);

  return buffer->addMapping(*ret_map, offset, size);
}

pi_result L0(piEnqueueMemUnmap)(
  pi_queue         queue,
  pi_mem           memobj,
  void *           mapped_ptr,
  pi_uint32        num_events_in_wait_list,
  const pi_event * event_wait_list,
  pi_event *       event) {

  // Get a new command list to be used on this call
  ze_command_list_handle_t ze_command_list = nullptr;
  if (auto res = queue->Context->Device->createCommandList(&ze_command_list))
    return res;

  // TODO: handle the case when user does not care to follow the event
  // of unmap completion.
  //
  assert(event);

  auto res = L0(piEventCreate)(queue->Context, event);
  if (res != PI_SUCCESS)
    return res;

  (*event)->Queue = queue;
  (*event)->CommandType = PI_COMMAND_TYPE_MEM_BUFFER_UNMAP;
  (*event)->L0CommandList = ze_command_list;

  ze_event_handle_t *ze_event_wait_list =
    _pi_event::createL0EventList(num_events_in_wait_list, event_wait_list);

  ZE_CALL(zeCommandListAppendWaitOnEvents(
    ze_command_list,
    num_events_in_wait_list,
    ze_event_wait_list
  ));

  // TODO: L0 is missing the memory "mapping" capabilities, so we are left
  // to doing copy (write back to the device).
  // See https://gitlab.devtools.intel.com/one-api/level_zero/issues/293.
  //
  // NOTE: Keep this in sync with the implementation of
  // piEnqueueMemBufferMap/piEnqueueMemImageMap.
  //
  _pi_mem::mapping map_info = {};
  if (pi_result res = memobj->removeMapping(mapped_ptr, map_info))
    return res;

  ze_event_handle_t ze_event = (*event)->L0Event;
  ZE_CALL(zeCommandListAppendMemoryCopy(
    ze_command_list,
    pi_cast<char *>(memobj->getL0Handle()) + map_info.offset,
    mapped_ptr,
    map_info.size,
    ze_event
  ));

  // NOTE: we still have to free the host memory allocated/returned by
  // piEnqueueMemBufferMap, but can only do so after the above copy
  // is completed. Instead of waiting for it here (blocking), we shall
  // do so in piEventRelease called for the pi_event tracking the unmap.
  (*event)->CommandData = memobj->MapHostPtr ? nullptr : mapped_ptr;

  // Execute command list asynchronously, as the event will be used
  // to track down its completion.
  if (auto res = queue->executeCommandList(ze_command_list))
    return res;

  _pi_event::deleteL0EventList(ze_event_wait_list);

  return PI_SUCCESS;
}

pi_result L0(piMemImageGetInfo) (
  pi_mem          image,
  pi_image_info   param_name,
  size_t          param_value_size,
  void *          param_value ,
  size_t *        param_value_size_ret) {

  die("piMemImageGetInfo: not implemented");
  return {};
}

static ze_image_region_t getImageRegionHelper(
  pi_mem            mem,
  const size_t *    origin,
  const size_t *    region) {

  assert(mem && origin);
#ifndef NDEBUG
  assert(mem->isImage());
  auto image = static_cast<_pi_image *>(mem);
  ze_image_desc_t imageDesc = image->L0ImageDesc;
#endif // !NDEBUG

  assert(imageDesc.type == ZE_IMAGE_TYPE_1D && origin[1] == 0 && origin[2] == 0 ||
         imageDesc.type == ZE_IMAGE_TYPE_1DARRAY && origin[2] == 0 ||
         imageDesc.type == ZE_IMAGE_TYPE_2D && origin[2] == 0 ||
         imageDesc.type == ZE_IMAGE_TYPE_3D);

  uint32_t originX = pi_cast<uint32_t>(origin[0]);
  uint32_t originY = pi_cast<uint32_t>(origin[1]);
  uint32_t originZ = pi_cast<uint32_t>(origin[2]);

  assert (region[0] && region[1] && region[2]);
  assert (imageDesc.type == ZE_IMAGE_TYPE_1D && region[1] == 1 && region[2] == 1 ||
          imageDesc.type == ZE_IMAGE_TYPE_1DARRAY && region[2] == 1 ||
          imageDesc.type == ZE_IMAGE_TYPE_2D && region[2] == 1 ||
          imageDesc.type == ZE_IMAGE_TYPE_3D);

  uint32_t width = pi_cast<uint32_t>(region[0]);
  uint32_t height = pi_cast<uint32_t>(region[1]);
  uint32_t depth = pi_cast<uint32_t>(region[2]);

  const ze_image_region_t zeRegion = {originX, originY, originZ, width, height, depth};
  return zeRegion;
}

// Helper function to implement image read/write/copy.
static pi_result enqueueMemImageCommandHelper(
  pi_command_type   command_type,
  pi_queue          queue,
  const void *      src, // image or ptr
  void *            dst, // image or ptr
  pi_bool           is_blocking,
  const size_t *    src_origin,
  const size_t *    dst_origin,
  const size_t *    region,
  size_t            row_pitch,
  size_t            slice_pitch,
  pi_uint32         num_events_in_wait_list,
  const pi_event *  event_wait_list,
  pi_event *        event) {

  assert(queue);
  // Get a new command list to be used on this call
  ze_command_list_handle_t ze_command_list = nullptr;
  if (auto res = queue->Context->Device->createCommandList(&ze_command_list))
    return res;

  auto res = L0(piEventCreate)(queue->Context, event);
  if (res != PI_SUCCESS)
    return res;

  (*event)->Queue = queue;
  (*event)->CommandType = command_type;
  (*event)->L0CommandList = ze_command_list;

  ze_event_handle_t ze_event = (*event)->L0Event;

  ze_event_handle_t *ze_event_wait_list =
    _pi_event::createL0EventList(num_events_in_wait_list, event_wait_list);

  ZE_CALL(zeCommandListAppendWaitOnEvents(
    ze_command_list,
    num_events_in_wait_list,
    ze_event_wait_list
  ));

  if (command_type == PI_COMMAND_TYPE_IMAGE_READ) {
    pi_mem src_mem = pi_cast<pi_mem>(const_cast<void *>(src));

    const ze_image_region_t srcRegion =
        getImageRegionHelper(src_mem, src_origin, region);

    // TODO: L0 does not support row_pitch/slice_pitch for images yet.
    // https://gitlab.devtools.intel.com/one-api/level_zero/issues/303
    // Check that SYCL RT did not want pitch larger than default.
    //
#ifndef NDEBUG
    assert(src_mem->isImage());
    auto src_image = static_cast<_pi_image *>(src_mem);
    const ze_image_desc_t &ImageDesc = src_image->L0ImageDesc;
    assert(row_pitch == 0 ||
           // special case RGBA image pitch equal to region's width
           (ImageDesc.format.layout == ZE_IMAGE_FORMAT_LAYOUT_32_32_32_32 &&
            row_pitch == 4 * 4 * srcRegion.width) ||
           (ImageDesc.format.layout == ZE_IMAGE_FORMAT_LAYOUT_16_16_16_16 &&
            row_pitch == 4 * 2 * srcRegion.width) ||
           (ImageDesc.format.layout == ZE_IMAGE_FORMAT_LAYOUT_8_8_8_8 &&
            row_pitch == 4 * srcRegion.width));
    assert(slice_pitch == 0 || slice_pitch == row_pitch * srcRegion.height);
#endif // !NDEBUG

    ZE_CALL(zeCommandListAppendImageCopyToMemory(
      ze_command_list,
      dst,
      pi_cast<ze_image_handle_t>(src_mem->getL0Handle()),
      &srcRegion,
      ze_event
    ));
  } else if (command_type == PI_COMMAND_TYPE_IMAGE_WRITE) {
    pi_mem dst_mem = pi_cast<pi_mem>(dst);
    const ze_image_region_t dstRegion =
      getImageRegionHelper(dst_mem, dst_origin, region);

    // TODO: L0 does not support row_pitch/slice_pitch for images yet.
    // https://gitlab.devtools.intel.com/one-api/level_zero/issues/303
    // Check that SYCL RT did not want pitch larger than default.
    //
#ifndef NDEBUG
    assert(dst_mem->isImage());
    auto dst_image = static_cast<_pi_image *>(dst_mem);
    const ze_image_desc_t &ImageDesc = dst_image->L0ImageDesc;
    assert(row_pitch == 0 ||
           // special case RGBA image pitch equal to region's width
           (ImageDesc.format.layout == ZE_IMAGE_FORMAT_LAYOUT_32_32_32_32 &&
            row_pitch == 4 * 4 * dstRegion.width) ||
           (ImageDesc.format.layout == ZE_IMAGE_FORMAT_LAYOUT_16_16_16_16 &&
            row_pitch == 4 * 2 * dstRegion.width) ||
           (ImageDesc.format.layout == ZE_IMAGE_FORMAT_LAYOUT_8_8_8_8 &&
            row_pitch == 4 * dstRegion.width));
    assert(slice_pitch == 0 || slice_pitch == row_pitch * dstRegion.height);
#endif // !NDEBUG

    ZE_CALL(zeCommandListAppendImageCopyFromMemory(
      ze_command_list,
      pi_cast<ze_image_handle_t>(dst_mem->getL0Handle()),
      src,
      &dstRegion,
      ze_event
    ));
  } else if (command_type == PI_COMMAND_TYPE_IMAGE_COPY) {
    pi_mem src_image = pi_cast<pi_mem>(const_cast<void*>(src));
    pi_mem dst_image = pi_cast<pi_mem>(dst);

    const ze_image_region_t srcRegion =
      getImageRegionHelper(src_image, src_origin, region);
    const ze_image_region_t dstRegion =
      getImageRegionHelper(dst_image, dst_origin, region);

    ZE_CALL(zeCommandListAppendImageCopyRegion(
      ze_command_list,
      pi_cast<ze_image_handle_t>(dst_image->getL0Handle()),
      pi_cast<ze_image_handle_t>(src_image->getL0Handle()),
      &dstRegion,
      &srcRegion,
      ze_event
    ));
  } else {
    zePrint("enqueueMemImageUpdate: unsupported image command type\n");
    return PI_INVALID_OPERATION;
  }

  if (auto res = queue->executeCommandList(ze_command_list, is_blocking))
    return res;

  _pi_event::deleteL0EventList(ze_event_wait_list);

  return PI_SUCCESS;
}

pi_result L0(piEnqueueMemImageRead)(
  pi_queue          queue,
  pi_mem            image,
  pi_bool           blocking_read,
  const size_t *    origin,
  const size_t *    region,
  size_t            row_pitch,
  size_t            slice_pitch,
  void *            ptr,
  pi_uint32         num_events_in_wait_list,
  const pi_event *  event_wait_list,
  pi_event *        event) {

  return enqueueMemImageCommandHelper(
    PI_COMMAND_TYPE_IMAGE_READ,
    queue,
    image,      // src
    ptr,        // dst
    blocking_read,
    origin,     // src_origin
    nullptr,    // dst_origin
    region,
    row_pitch,
    slice_pitch,
    num_events_in_wait_list,
    event_wait_list, event);
}

pi_result L0(piEnqueueMemImageWrite)(
  pi_queue          queue,
  pi_mem            image,
  pi_bool           blocking_write,
  const size_t *    origin,
  const size_t *    region,
  size_t            input_row_pitch,
  size_t            input_slice_pitch,
  const void *      ptr,
  pi_uint32         num_events_in_wait_list,
  const pi_event *  event_wait_list,
  pi_event *        event) {

  return enqueueMemImageCommandHelper(
    PI_COMMAND_TYPE_IMAGE_WRITE,
    queue,
    ptr,        // src
    image,      // dst
    blocking_write,
    nullptr,    // src_origin
    origin,     // dst_origin
    region,
    input_row_pitch,
    input_slice_pitch,
    num_events_in_wait_list,
    event_wait_list,
    event);
}

pi_result L0(piEnqueueMemImageCopy)(
  pi_queue          queue,
  pi_mem            src_image,
  pi_mem            dst_image,
  const size_t *    src_origin,
  const size_t *    dst_origin,
  const size_t *    region,
  pi_uint32         num_events_in_wait_list,
  const pi_event *  event_wait_list,
  pi_event *        event) {

  return enqueueMemImageCommandHelper(
    PI_COMMAND_TYPE_IMAGE_COPY,
    queue,
    src_image,
    dst_image,
    false,      // is_blocking
    src_origin,
    dst_origin,
    region,
    0,          // row pitch
    0,          // slice pitch
    num_events_in_wait_list,
    event_wait_list,
    event);
}

pi_result L0(piEnqueueMemImageFill)(
  pi_queue          queue,
  pi_mem            image,
  const void *      fill_color,
  const size_t *    origin,
  const size_t *    region,
  pi_uint32         num_events_in_wait_list,
  const pi_event *  event_wait_list,
  pi_event *        event) {

  die("piEnqueueMemImageFill: not implemented");
  return {};
}

pi_result L0(piMemBufferPartition)(
    pi_mem                    buffer,
    pi_mem_flags              flags,
    pi_buffer_create_type     buffer_create_type,
    void *                    buffer_create_info,
    pi_mem *                  ret_mem) {

  assert(buffer && !buffer->isImage());
  assert(flags == PI_MEM_FLAGS_ACCESS_RW);
  assert(buffer_create_type == PI_BUFFER_CREATE_TYPE_REGION);
  assert(!(static_cast<_pi_buffer *>(buffer))->isSubBuffer() &&
         "Sub-buffer cannot be partitioned");
  assert(buffer_create_info);
  assert(ret_mem);

  auto region = (pi_buffer_region)buffer_create_info;
  assert(region->size != 0u && "Invalid size");
  assert(region->origin <= (region->origin + region->size) && "Overflow");
  *ret_mem = new _pi_buffer(
      buffer->Platform,
      pi_cast<char *>(buffer->getL0Handle()) +
          region->origin /* L0 memory handle */,
      nullptr /* Host pointer */, buffer /* Parent buffer */,
      region->origin /* Sub-buffer origin */, region->size /*Sub-buffer size*/);

  return PI_SUCCESS;
}

pi_result L0(piEnqueueNativeKernel)(
  pi_queue         queue,
  void             (*user_func)(void *),
  void *           args,
  size_t           cb_args,
  pi_uint32        num_mem_objects,
  const pi_mem *   mem_list,
  const void **    args_mem_loc,
  pi_uint32        num_events_in_wait_list,
  const pi_event * event_wait_list,
  pi_event *       event) {

  die("piEnqueueNativeKernel: not implemented");
  return {};
}

// TODO: Check if the function_pointer_ret type can be converted to void**.
pi_result L0(piextGetDeviceFunctionPointer)(
  pi_device        device,
  pi_program       program,
  const char *     function_name,
  pi_uint64 *      function_pointer_ret) {
  assert(program != nullptr);
  ZE_CALL(zeModuleGetFunctionPointer(
      program->L0Module, function_name,
      reinterpret_cast<void **>(function_pointer_ret)));
  return PI_SUCCESS;
}

pi_result L0(piextUSMHostAlloc)(void **result_ptr, pi_context context,
                                pi_usm_mem_properties *properties, size_t size,
                                pi_uint32 alignment) {

  assert(context);
  // Check that incorrect bits are not set in the properties.
  assert(!properties || (properties && !(*properties & ~PI_MEM_ALLOC_FLAGS)));

  ze_host_mem_alloc_desc_t ze_desc = {};
  ze_desc.flags = ZE_HOST_MEM_ALLOC_FLAG_DEFAULT;
  // TODO: translate PI properties to L0 flags
  ZE_CALL(zeDriverAllocHostMem(
    context->Device->Platform->L0Driver,
    &ze_desc,
    size,
    alignment,
    result_ptr));

  return PI_SUCCESS;
}

pi_result L0(piextUSMDeviceAlloc)(void **result_ptr, pi_context context,
                                  pi_device device,
                                  pi_usm_mem_properties *properties,
                                  size_t size, pi_uint32 alignment) {

  assert(context);
  assert(device);
  // Check that incorrect bits are not set in the properties.
  assert(!properties || (properties && !(*properties & ~PI_MEM_ALLOC_FLAGS)));

  // TODO: translate PI properties to L0 flags
  ze_device_mem_alloc_desc_t ze_desc = {};
  ze_desc.flags = ZE_DEVICE_MEM_ALLOC_FLAG_DEFAULT;
  ze_desc.ordinal = 0;
  ZE_CALL(zeDriverAllocDeviceMem(
    context->Device->Platform->L0Driver,
    &ze_desc,
    size,
    alignment,
    device->L0Device,
    result_ptr));

  return PI_SUCCESS;
}

pi_result L0(piextUSMSharedAlloc)(
  void **                 result_ptr,
  pi_context              context,
  pi_device               device,
  pi_usm_mem_properties * properties,
  size_t                  size,
  pi_uint32               alignment) {

  assert(context);
  assert(device);
  // Check that incorrect bits are not set in the properties.
  assert(!properties || (properties && !(*properties & ~PI_MEM_ALLOC_FLAGS)));

  // TODO: translate PI properties to L0 flags
  ze_host_mem_alloc_desc_t ze_host_desc = {};
  ze_host_desc.flags = ZE_HOST_MEM_ALLOC_FLAG_DEFAULT;
  ze_device_mem_alloc_desc_t ze_dev_desc = {};
  ze_dev_desc.flags = ZE_DEVICE_MEM_ALLOC_FLAG_DEFAULT;
  ze_dev_desc.ordinal = 0;
  ZE_CALL(zeDriverAllocSharedMem(
    context->Device->Platform->L0Driver,
    &ze_dev_desc,
    &ze_host_desc,
    size,
    alignment,
    device->L0Device,
    result_ptr));

  return PI_SUCCESS;
}

pi_result L0(piextUSMFree)(pi_context context, void *ptr)
{
  ZE_CALL(zeDriverFreeMem(context->Device->Platform->L0Driver, ptr));
  return PI_SUCCESS;
}

pi_result L0(piextKernelSetArgPointer)(
  pi_kernel kernel,
  pi_uint32 arg_index,
  size_t arg_size,
  const void *arg_value) {

  return piKernelSetArg(kernel, arg_index, arg_size, arg_value);
}

/// USM Memset API
///
/// @param queue is the queue to submit to
/// @param ptr is the ptr to memset
/// @param value is value to set.  It is interpreted as an 8-bit value and the upper
///        24 bits are ignored
/// @param count is the size in bytes to memset
/// @param num_events_in_waitlist is the number of events to wait on
/// @param events_waitlist is an array of events to wait on
/// @param event is the event that represents this operation
pi_result L0(piextUSMEnqueueMemset)(
  pi_queue queue,
  void *ptr,
  pi_int32 value,
  size_t count,
  pi_uint32 num_events_in_waitlist,
  const pi_event *events_waitlist,
  pi_event *event)
{
  if (!ptr) {
    return PI_INVALID_VALUE;
  }

  return enqueueMemFillHelper(
    // TODO: do we need a new command type for USM memset?
    PI_COMMAND_TYPE_MEM_BUFFER_FILL,
    queue,
    ptr,
    &value, // It will be interpreted as an 8-bit value,
    1,      // which is indicated with this pattern_size==1
    count,
    num_events_in_waitlist,
    events_waitlist,
    event);
}

pi_result L0(piextUSMEnqueueMemcpy)(
  pi_queue        queue,
  pi_bool         blocking,
  void *          dst_ptr,
  const void *    src_ptr,
  size_t          size,
  pi_uint32       num_events_in_waitlist,
  const pi_event *events_waitlist,
  pi_event *      event) {

  if (!dst_ptr) {
    return PI_INVALID_VALUE;
  }

  return enqueueMemCopyHelper(
    // TODO: do we need a new command type for this?
    PI_COMMAND_TYPE_MEM_BUFFER_COPY,
    queue,
    dst_ptr,
    blocking,
    size,
    src_ptr,
    num_events_in_waitlist,
    events_waitlist,
    event);
}

/// Hint to migrate memory to the device
///
/// @param queue is the queue to submit to
/// @param ptr points to the memory to migrate
/// @param size is the number of bytes to migrate
/// @param flags is a bitfield used to specify memory migration options
/// @param num_events_in_waitlist is the number of events to wait on
/// @param events_waitlist is an array of events to wait on
/// @param event is the event that represents this operation
pi_result L0(piextUSMEnqueuePrefetch)(
  pi_queue queue,
  const void *ptr,
  size_t size,
  pi_usm_migration_flags flags,
  pi_uint32 num_events_in_waitlist,
  const pi_event *events_waitlist,
  pi_event *event)
{
  assert(queue);
  assert(!(flags & ~PI_USM_MIGRATION_TBD0));

  // Get a new command list to be used on this call
  ze_command_list_handle_t ze_command_list = nullptr;
  if (auto res = queue->Context->Device->createCommandList(&ze_command_list))
    return res;

  // TODO: do we need to create a unique command type for this?
  auto res = L0(piEventCreate)(queue->Context, event);
  if (res != PI_SUCCESS)
    return res;

  (*event)->Queue = queue;
  (*event)->CommandType = PI_COMMAND_TYPE_USER;
  (*event)->L0CommandList = ze_command_list;

  ze_event_handle_t *ze_event_wait_list =
    _pi_event::createL0EventList(num_events_in_waitlist, events_waitlist);

  ZE_CALL(zeCommandListAppendWaitOnEvents(
    ze_command_list,
    num_events_in_waitlist,
    ze_event_wait_list
  ));

  // TODO: figure out how to translate "flags"
  ZE_CALL(zeCommandListAppendMemoryPrefetch(
    ze_command_list,
    ptr,
    size
  ));

  // TODO: L0 does not have a completion "event" with the prefetch API,
  // so manually add command to signal our event.
  //
  ZE_CALL(zeCommandListAppendSignalEvent(ze_command_list, (*event)->L0Event));

  if (auto res = queue->executeCommandList(ze_command_list, false))
    return res;

  _pi_event::deleteL0EventList(ze_event_wait_list);

  return PI_SUCCESS;
}

/// USM memadvise API to govern behavior of automatic migration mechanisms
///
/// @param queue is the queue to submit to
/// @param ptr is the data to be advised
/// @param length is the size in bytes of the meory to advise
/// @param advice is device specific advice
/// @param event is the event that represents this operation
///
pi_result L0(piextUSMEnqueueMemAdvise)(
  pi_queue queue,
  const void *ptr,
  size_t length,
  pi_mem_advice advice,
  pi_event *event)
{
  assert(queue);
  ze_memory_advice_t ze_advice = {};
  switch (advice) {
  case PI_MEM_ADVICE_SET_READ_MOSTLY:
    ze_advice = ZE_MEMORY_ADVICE_SET_READ_MOSTLY;
    break;
  case PI_MEM_ADVICE_CLEAR_READ_MOSTLY:
    ze_advice = ZE_MEMORY_ADVICE_CLEAR_READ_MOSTLY;
    break;
  case PI_MEM_ADVICE_SET_PREFERRED_LOCATION:
    ze_advice = ZE_MEMORY_ADVICE_SET_PREFERRED_LOCATION;
    break;
  case PI_MEM_ADVICE_CLEAR_PREFERRED_LOCATION:
    ze_advice = ZE_MEMORY_ADVICE_CLEAR_PREFERRED_LOCATION;
    break;
  case PI_MEM_ADVICE_SET_ACCESSED_BY:
    ze_advice = ZE_MEMORY_ADVICE_SET_ACCESSED_BY;
    break;
  case PI_MEM_ADVICE_CLEAR_ACCESSED_BY:
    ze_advice = ZE_MEMORY_ADVICE_CLEAR_ACCESSED_BY;
    break;
  case PI_MEM_ADVICE_SET_NON_ATOMIC_MOSTLY:
    ze_advice = ZE_MEMORY_ADVICE_SET_NON_ATOMIC_MOSTLY;
    break;
  case PI_MEM_ADVICE_CLEAR_NON_ATOMIC_MOSTLY:
    ze_advice = ZE_MEMORY_ADVICE_CLEAR_NON_ATOMIC_MOSTLY;
    break;
  case PI_MEM_ADVICE_BIAS_CACHED:
    ze_advice = ZE_MEMORY_ADVICE_BIAS_CACHED;
    break;
  case PI_MEM_ADVICE_BIAS_UNCACHED:
    ze_advice = ZE_MEMORY_ADVICE_BIAS_UNCACHED;
    break;
  default:
    zePrint("piextUSMEnqueueMemAdvise: unexpected memory advise\n");
    return PI_INVALID_VALUE;
  }

  // Get a new command list to be used on this call
  ze_command_list_handle_t ze_command_list = nullptr;
  if (auto res = queue->Context->Device->createCommandList(&ze_command_list))
    return res;

  // TODO: do we need to create a unique command type for this?
  auto res = L0(piEventCreate)(queue->Context, event);
  if (res != PI_SUCCESS)
    return res;

  (*event)->Queue = queue;
  (*event)->CommandType = PI_COMMAND_TYPE_USER;
  (*event)->L0CommandList = ze_command_list;

  ZE_CALL(zeCommandListAppendMemAdvise(
    ze_command_list,
    queue->Context->Device->L0Device,
    ptr,
    length,
    ze_advice
  ));

  // TODO: L0 does not have a completion "event" with the advise API,
  // so manually add command to signal our event.
  //
  ZE_CALL(zeCommandListAppendSignalEvent(ze_command_list, (*event)->L0Event));

  queue->executeCommandList(ze_command_list, false);
  return PI_SUCCESS;
}

/// API to query information about USM allocated pointers
/// Valid Queries:
///   PI_MEM_ALLOC_TYPE returns host/device/shared pi_usm_type value
///   PI_MEM_ALLOC_BASE_PTR returns the base ptr of an allocation if
///                         the queried pointer fell inside an allocation.
///                         Result must fit in void *
///   PI_MEM_ALLOC_SIZE returns how big the queried pointer's
///                     allocation is in bytes. Result is a size_t.
///   PI_MEM_ALLOC_DEVICE returns the pi_device this was allocated against
///
/// @param context is the pi_context
/// @param ptr is the pointer to query
/// @param param_name is the type of query to perform
/// @param param_value_size is the size of the result in bytes
/// @param param_value is the result
/// @param param_value_ret is how many bytes were written
pi_result L0(piextUSMGetMemAllocInfo)(
  pi_context context,
  const void *ptr,
  pi_mem_info param_name,
  size_t param_value_size,
  void *param_value,
  size_t *param_value_size_ret)
{
  assert(context);
  ze_device_handle_t ze_device_handle;
  ze_memory_allocation_properties_t ze_memory_allocation_properties = {
    ZE_MEMORY_ALLOCATION_PROPERTIES_VERSION_CURRENT
  };

  ZE_CALL(zeDriverGetMemAllocProperties(
    context->Device->Platform->L0Driver,
    ptr,
    &ze_memory_allocation_properties,
    &ze_device_handle));

  switch (param_name) {
  case PI_MEM_ALLOC_TYPE: {
    pi_usm_type mem_alloc_type;
    switch (ze_memory_allocation_properties.type) {
    case ZE_MEMORY_TYPE_UNKNOWN:
      mem_alloc_type = PI_MEM_TYPE_UNKNOWN;
      break;
    case ZE_MEMORY_TYPE_HOST:
      mem_alloc_type = PI_MEM_TYPE_HOST;
      break;
    case ZE_MEMORY_TYPE_DEVICE:
      mem_alloc_type = PI_MEM_TYPE_DEVICE;
      break;
    case ZE_MEMORY_TYPE_SHARED:
      mem_alloc_type = PI_MEM_TYPE_SHARED;
      break;
    default:
      zePrint("piextUSMGetMemAllocInfo: unexpected usm memory type\n");
      return PI_INVALID_VALUE;
    }
    SET_PARAM_VALUE(mem_alloc_type);
    break;
  }
  case PI_MEM_ALLOC_DEVICE: {
    // TODO: this wants pi_device, but we didn't remember it, and cannot
    // deduct from the L0 device.
    //
    die("piextUSMGetMemAllocInfo: PI_MEM_ALLOC_DEVICE not implemented");
    break;
  }
  case PI_MEM_ALLOC_BASE_PTR: {
    void *base;
    ZE_CALL(zeDriverGetMemAddressRange(context->Device->Platform->L0Driver, ptr,
                                       &base, nullptr));
    SET_PARAM_VALUE(base);
    break;
  }
  case PI_MEM_ALLOC_SIZE: {
    size_t size;
    ZE_CALL(zeDriverGetMemAddressRange(context->Device->Platform->L0Driver, ptr,
                                       nullptr, &size));
    SET_PARAM_VALUE(size);
    break;
  }
  default:
    zePrint("piextUSMGetMemAllocInfo: unsupported param_name\n");
    return PI_INVALID_VALUE;
  }
  return PI_SUCCESS;
}

pi_result L0(piKernelSetExecInfo)(pi_kernel kernel,
                                  pi_kernel_exec_info param_name,
                                  size_t param_value_size,
                                  const void *param_value) {
  assert(kernel);
  assert(param_value);
  if (param_name == PI_USM_INDIRECT_ACCESS &&
      *(static_cast<const pi_bool *>(param_value)) == PI_TRUE) {
    // The whole point for users really was to not need to know anything
    // about the types of allocations kernel uses. So in DPC++ we always
    // just set all 3 modes for each kernel.
    //
    bool ze_indirect_value = true;
    ZE_CALL(zeKernelSetAttribute(
      kernel->L0Kernel, ZE_KERNEL_ATTR_INDIRECT_SHARED_ACCESS,
        sizeof(bool), &ze_indirect_value));
    ZE_CALL(zeKernelSetAttribute(
      kernel->L0Kernel, ZE_KERNEL_ATTR_INDIRECT_DEVICE_ACCESS,
        sizeof(bool), &ze_indirect_value));
    ZE_CALL(zeKernelSetAttribute(
      kernel->L0Kernel, ZE_KERNEL_ATTR_INDIRECT_HOST_ACCESS,
        sizeof(bool), &ze_indirect_value));
  }
  else {
   zePrint("piKernelSetExecInfo: unsupported param_name\n");
   return PI_INVALID_VALUE;
  }

  return PI_SUCCESS;
}

pi_result piextProgramSetSpecializationConstant(pi_program prog,
                                                pi_uint32 spec_id,
                                                size_t spec_size,
                                                const void *spec_value) {
  // TODO: implement
  die("piextProgramSetSpecializationConstant: not implemented");
  return {};
}

pi_result L0(piPluginInit)(pi_plugin *PluginInit)
{
  assert(PluginInit);
  // TODO: handle versioning/targets properly.
  size_t PluginVersionSize = sizeof(PluginInit->PluginVersion);
  assert(strlen(_PI_H_VERSION_STRING) < PluginVersionSize);
  strncpy(PluginInit->PluginVersion, _PI_H_VERSION_STRING, PluginVersionSize);

#define _PI_API(api)                                                \
  (PluginInit->PiFunctionTable).api = (decltype(&::api))(&L0(api));
#include <CL/sycl/detail/pi.def>

  return PI_SUCCESS;
}

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

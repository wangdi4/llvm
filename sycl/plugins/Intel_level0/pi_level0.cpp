//
// TODO: Refactor this code for upstream, taking these general guidelines
//       into consideration:
//
// 1. Use more C++ where it makes sense, e.g. constructors/destructors or
//    encapsulating some functionalities of PI objects.
// 2. Settle/follow a convention for naming L0/PI handles.
// 3. Make this code not throwing exception, but return any errors in result.
// 4. Make this code more robust, assert of assumptions and supported features.
// 5. Make this code thread-safe.
// 6. Address TODO comments in the code.
// 7. Cover PI API with unittests
//
#include "pi_level0.hpp"
#include <map>
#include <memory>
#include <string>
#include <thread>

#include <level_zero/zet_api.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <stdarg.h>

bool ZE_DEBUG = false;

// Forward declarations
static pi_result enqueueMemCopyHelper(
  pi_command_type    command_type,
  pi_queue           command_queue,
  void *             dst,
  pi_bool            blocking_write,
  size_t             size,
  const void *       src,
  pi_uint32          num_events_in_wait_list,
  const pi_event *   event_wait_list,
  pi_event *         event);


static void zePrint(const char *format, ... ) {
  if (ZE_DEBUG) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
  }
}

inline void zeParseError(ze_result_t error, std::string &errorString)
{
    if (ZE_RESULT_SUCCESS == error) {
      errorString = "ZE_RESULT_SUCCESS";
    }
    else if (ZE_RESULT_NOT_READY == error) {
      errorString = "ZE_RESULT_NOT_READY";
    }
    else if (ZE_RESULT_ERROR_DEVICE_LOST == error) {
      errorString = "ZE_RESULT_ERROR_DEVICE_LOST";
    }
    else if (ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY == error) {
      errorString = "ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY";
    }
    else if (ZE_RESULT_ERROR_OUT_OF_DEVICE_MEMORY == error) {
      errorString = "ZE_RESULT_ERROR_OUT_OF_DEVICE_MEMORY";
    }
    else if (ZE_RESULT_ERROR_MODULE_BUILD_FAILURE == error) {
      errorString = "ZE_RESULT_ERROR_MODULE_BUILD_FAILURE";
    }
    else if (ZE_RESULT_ERROR_INSUFFICIENT_PERMISSIONS == error) {
      errorString = "ZE_RESULT_ERROR_INSUFFICIENT_PERMISSIONS";
    }
    else if (ZE_RESULT_ERROR_NOT_AVAILABLE == error) {
      errorString = "ZE_RESULT_ERROR_NOT_AVAILABLE";
    }
    else if (ZE_RESULT_ERROR_UNINITIALIZED == error) {
      errorString = "ZE_RESULT_ERROR_UNINITIALIZED";
    }
    else if (ZE_RESULT_ERROR_UNSUPPORTED_VERSION == error) {
      errorString = "ZE_RESULT_ERROR_UNSUPPORTED_VERSION";
    }
    else if (ZE_RESULT_ERROR_UNSUPPORTED_FEATURE == error) {
      errorString = "ZE_RESULT_ERROR_UNSUPPORTED_FEATURE";
    }
    else if (ZE_RESULT_ERROR_INVALID_ARGUMENT == error) {
      errorString = "ZE_RESULT_ERROR_INVALID_ARGUMENT";
    }
    else if (ZE_RESULT_ERROR_INVALID_NULL_HANDLE == error) {
      errorString = "ZE_RESULT_ERROR_INVALID_NULL_HANDLE";
    }
    else if (ZE_RESULT_ERROR_HANDLE_OBJECT_IN_USE == error) {
      errorString = "ZE_RESULT_ERROR_HANDLE_OBJECT_IN_USE";
    }
    else if (ZE_RESULT_ERROR_INVALID_NULL_POINTER == error) {
      errorString = "ZE_RESULT_ERROR_INVALID_NULL_POINTER";
    }
    else if (ZE_RESULT_ERROR_INVALID_SIZE == error) {
      errorString = "ZE_RESULT_ERROR_INVALID_SIZE";
    }
    else if (ZE_RESULT_ERROR_UNSUPPORTED_SIZE == error) {
      errorString = "ZE_RESULT_ERROR_UNSUPPORTED_SIZE";
    }
    else if (ZE_RESULT_ERROR_UNSUPPORTED_ALIGNMENT == error) {
      errorString = "ZE_RESULT_ERROR_UNSUPPORTED_ALIGNMENT";
    }
    else if (ZE_RESULT_ERROR_INVALID_SYNCHRONIZATION_OBJECT == error) {
      errorString = "ZE_RESULT_ERROR_INVALID_SYNCHRONIZATION_OBJECT";
    }
    else if (ZE_RESULT_ERROR_INVALID_ENUMERATION == error) {
      errorString = "ZE_RESULT_ERROR_INVALID_ENUMERATION";
    }
    else if (ZE_RESULT_ERROR_UNSUPPORTED_ENUMERATION == error) {
      errorString = "ZE_RESULT_ERROR_UNSUPPORTED_ENUMERATION";
    }
    else if (ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT == error) {
      errorString = "ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT";
    }
    else if (ZE_RESULT_ERROR_INVALID_NATIVE_BINARY == error) {
      errorString = "ZE_RESULT_ERROR_INVALID_NATIVE_BINARY";
    }
    else if (ZE_RESULT_ERROR_INVALID_GLOBAL_NAME == error) {
      errorString = "ZE_RESULT_ERROR_INVALID_GLOBAL_NAME";
    }
    else if (ZE_RESULT_ERROR_INVALID_KERNEL_NAME == error) {
      errorString = "ZE_RESULT_ERROR_INVALID_KERNEL_NAME";
    }
    else if (ZE_RESULT_ERROR_INVALID_FUNCTION_NAME == error) {
      errorString = "ZE_RESULT_ERROR_INVALID_FUNCTION_NAME";
    }
    else if (ZE_RESULT_ERROR_INVALID_GROUP_SIZE_DIMENSION == error) {
      errorString = "ZE_RESULT_ERROR_INVALID_GROUP_SIZE_DIMENSION";
    }
    else if (ZE_RESULT_ERROR_INVALID_GLOBAL_WIDTH_DIMENSION == error) {
      errorString = "ZE_RESULT_ERROR_INVALID_GLOBAL_WIDTH_DIMENSION";
    }
    else if (ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX == error) {
      errorString = "ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX";
    }
    else if (ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE == error) {
      errorString = "ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE";
    }
    else if (ZE_RESULT_ERROR_INVALID_KERNEL_ATTRIBUTE_VALUE == error) {
      errorString = "ZE_RESULT_ERROR_INVALID_KERNEL_ATTRIBUTE_VALUE";
    }
    else if (ZE_RESULT_ERROR_INVALID_COMMAND_LIST_TYPE == error) {
      errorString = "ZE_RESULT_ERROR_INVALID_COMMAND_LIST_TYPE";
    }
    else if (ZE_RESULT_ERROR_OVERLAPPING_REGIONS == error) {
      errorString = "ZE_RESULT_ERROR_OVERLAPPING_REGIONS";
    }
    else if (ZE_RESULT_ERROR_UNKNOWN == error) {
      errorString = "ZE_RESULT_ERROR_UNKNOWN";
    }
}

static ze_result_t zeCallCheck(ze_result_t ze_result, const char *call_str, bool nothrow = false)
{
  zePrint("ZE ---> %s\n", call_str);

  // TODO: handle errors
  if (ze_result) {
    if (!nothrow) {
      std::string errorString;
      zeParseError(ze_result, errorString);
      fprintf(stderr, "Error (%s) in %s\n", errorString.c_str(), call_str);
      pi_throw("L0 Error");
    }
  }
  return ze_result;
}

#define ZE_CALL(call)         zeCallCheck(call, #call, false)
#define ZE_CALL_NOTHROW(call) zeCallCheck(call, #call, true)

// Crate a new command list to be used in a PI call
ze_command_list_handle_t _pi_device::createCommandList()
{
  // Create the command list, because in L0 commands are added to
  // the command lists, and later are then added to the command queue.
  //
  // TODO: Fugire out how to lower the overhead of creating a new list
  // for each PI command, if that appears to be important.
  //
  ze_command_list_handle_t ze_command_list = nullptr;
  ze_command_list_desc_t ze_command_list_desc;
  ze_command_list_desc.version = ZE_COMMAND_LIST_DESC_VERSION_CURRENT;

  // TODO: can we just reset the command-list created when an earlier
  // command was submitted to the queue?
  //
  ZE_CALL(zeCommandListCreate(
    L0Device,
    &ze_command_list_desc,
    &ze_command_list));

  return ze_command_list;
}

void _pi_queue::executeCommandList(ze_command_list_handle_t ze_command_list,
                                   bool is_blocking)
{
  // Close the command list and have it ready for dispatch.
  ZE_CALL(zeCommandListClose(ze_command_list));
  // Offload command list to the GPU for asynchronous execution
  ZE_CALL(zeCommandQueueExecuteCommandLists(
    L0CommandQueue, 1, &ze_command_list, nullptr));

  // TODO: add a global control to make every command blocking for debugging.
  if (is_blocking) {
    // Wait until command lists attached to the command queue are executed.
    ZE_CALL(zeCommandQueueSynchronize(L0CommandQueue, UINT32_MAX));
  }
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

// TODO: Figure out how to pass these objects and eliminated these globals.
ze_driver_handle_t ze_driver_global = {0};

pi_result L0(piPlatformsGet)(pi_uint32       num_entries,
                             pi_platform *   platforms,
                             pi_uint32 *     num_platforms) {

  static const char *getEnv = std::getenv("ZE_DEBUG");
  if (getEnv)
    ZE_DEBUG = true;

  // This is a good time to initialize L0.
  ze_result_t result =
    ZE_CALL(zeInit(ZE_INIT_FLAG_NONE));

  // L0 does not have concept of platforms, return a fake one.
  if (platforms && num_entries >= 1) {
    *platforms = pi_cast<pi_platform>(&ze_driver_global);
  }
  if (num_platforms)
    *num_platforms = 1;

  return pi_cast<pi_result>(result);
}

pi_result L0(piPlatformGetInfo)(
  pi_platform       platform,
  pi_platform_info  param_name,
  size_t            param_value_size,
  void *            param_value,
  size_t *          param_value_size_ret) {

  pi_assert(ze_driver_global != nullptr);
  ze_driver_properties_t ze_driver_properties;
  ZE_CALL(zeDriverGetProperties(ze_driver_global, &ze_driver_properties));
  uint32_t ze_driver_version = ze_driver_properties.driverVersion;
  uint32_t ze_driver_version_major = ZE_MAJOR_VERSION(ze_driver_version);
  uint32_t ze_driver_version_minor = ZE_MINOR_VERSION(ze_driver_version);

  char ze_driver_version_string[255];
  sprintf(ze_driver_version_string, "Level-Zero %d.%d\n",
      ze_driver_version_major,
      ze_driver_version_minor);
  zePrint("==========================\n");
  zePrint("SYCL over %s\n", ze_driver_version_string);
  zePrint("==========================\n");

  if (param_name == PI_PLATFORM_INFO_NAME) {
    SET_PARAM_VALUE_STR("Intel Level-Zero Platform");
  }
  else if (param_name == PI_PLATFORM_INFO_VENDOR) {
    SET_PARAM_VALUE_STR("Intel");
  }
  else if (param_name == PI_PLATFORM_INFO_EXTENSIONS) {
    // Convention adopted from OpenCL:
    //     "Returns a space-separated list of extension names (the extension
    // names themselves do not contain any spaces) supported by the platform.
    // Extensions defined here must be supported by all devices associated
    // with this platform."
    //
    // TODO: any DPC++ features need this?
    //
    SET_PARAM_VALUE_STR("");
  }
  else if (param_name == PI_PLATFORM_INFO_PROFILE) {
    // TODO: figure out what this means and how is this used
    SET_PARAM_VALUE_STR("FULL_PROFILE");
  }
  else if (param_name == PI_PLATFORM_INFO_VERSION) {
    // TODO: this should query to zeDriverGetDriverVersion
    // but we don't yet have the driver handle here.
    //
    // From OpenCL 2.1: "This version string has the following format:
    // OpenCL<space><major_version.minor_version><space><platform-specific information>.
    // Follow the same notation here.
    //
    SET_PARAM_VALUE_STR(ze_driver_version_string);
  }
  else {
    // TODO: implement other parameters
    pi_throw("Unsupported param_name in piPlatformGetInfo");
  }

  return PI_SUCCESS;
}

pi_result L0(piDevicesGet)(pi_platform      platform,
                           pi_device_type   device_type,
                           pi_uint32        num_entries,
                           pi_device *      devices,
                           pi_uint32 *      num_devices) {

  ze_result_t ze_res;
  uint32_t ze_driver_count = 0;
  ze_res = ZE_CALL(zeDriverGet(&ze_driver_count, nullptr));
  if (ze_res || (ze_driver_count == 0)) {
    return pi_cast<pi_result>(ze_res);
  }

  ze_driver_count = 1;
  ze_driver_handle_t ze_driver;
  ze_res = ZE_CALL(zeDriverGet(&ze_driver_count, &ze_driver));
  if (ze_res) {
    return pi_cast<pi_result>(ze_res);
  }
  ze_driver_global = ze_driver;

  // L0 does not have platforms, expect fake pointer here
  pi_assert(platform == pi_cast<pi_platform>(&ze_driver_global));

  // Get number of devices supporting L0
  uint32_t ze_device_count = 0;
  ze_res = ZE_CALL(zeDeviceGet(ze_driver, &ze_device_count, nullptr));
  if (ze_res || (ze_device_count == 0)) {
    return pi_cast<pi_result>(ze_res);
  }

  if (num_devices)
    *num_devices = ze_device_count;

  // TODO: Delete array at teardown
  ze_device_handle_t *ze_devices = new ze_device_handle_t[ze_device_count];
  ze_res = ZE_CALL(zeDeviceGet(ze_driver, &ze_device_count, ze_devices));
  if (ze_res) {
    return pi_cast<pi_result>(ze_res);
  }

  for (uint32_t i = 0; i < ze_device_count; ++i) {
    // TODO: add check for device type
    if (i < num_entries) {
      auto L0PiDevice = new _pi_device();
      L0PiDevice->L0Device = ze_devices[i];
      L0PiDevice->IsSubDevice = false;
      L0PiDevice->RefCount = 1;

      // Create the immediate command list to be used for initializations
      // Created as synchronous so level-zero performs implicit synchronization and
      // there is no need to query for completion in the plugin
      ze_device_handle_t ze_device = L0PiDevice->L0Device;
      ze_command_queue_desc_t ze_command_queue_desc =
          {ZE_COMMAND_QUEUE_DESC_VERSION_CURRENT};
      ze_command_queue_desc.ordinal = 0;
      ze_command_queue_desc.mode = ZE_COMMAND_QUEUE_MODE_SYNCHRONOUS;
      ZE_CALL(zeCommandListCreateImmediate(ze_device, &ze_command_queue_desc,
                                           &L0PiDevice->L0CommandListInit));

      devices[i] = L0PiDevice;
    }
  }
  return PI_SUCCESS;
}

pi_result L0(piDeviceRetain)(pi_device device)
{
  // The root-device ref-count remains unchanged (always 1).
  if (device->IsSubDevice) {
    ++(device->RefCount);
  }
  return PI_SUCCESS;
}

pi_result L0(piDeviceRelease)(pi_device device)
{
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

  // TODO: cache device properties instead of querying L0 each time
  ze_device_handle_t ze_device = device->L0Device;
  ze_device_properties_t ze_device_properties;
  ze_device_properties.version = ZE_DEVICE_PROPERTIES_VERSION_CURRENT;
  ZE_CALL(zeDeviceGetProperties(
    ze_device,
    &ze_device_properties));

  ze_device_compute_properties_t ze_device_compute_properties;
  ze_device_compute_properties.version = ZE_DEVICE_COMPUTE_PROPERTIES_VERSION_CURRENT;
  ZE_CALL(zeDeviceGetComputeProperties(
    ze_device,
    &ze_device_compute_properties));

  uint32_t ze_avail_mem_count = 0;
  ZE_CALL(zeDeviceGetMemoryProperties(
    ze_device,
    &ze_avail_mem_count,
    nullptr
  ));
  // Confirm at least one memory is available in the device
  pi_assert(ze_avail_mem_count > 0);
  ze_device_memory_properties_t *ze_device_memory_properties =
    new ze_device_memory_properties_t[ze_avail_mem_count]();
  for (uint32_t i = 0; i < ze_avail_mem_count; i++) {
    ze_device_memory_properties[i].version = ZE_DEVICE_MEMORY_PROPERTIES_VERSION_CURRENT;
  }
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

  if (param_name == PI_DEVICE_INFO_TYPE) {
    if (ze_device_properties.type == ZE_DEVICE_TYPE_GPU) {
      SET_PARAM_VALUE(PI_DEVICE_TYPE_GPU);
    }
    else { // ZE_DEVICE_TYPE_FPGA
      pi_throw("FPGA not supported");
    }
  }
  else if (param_name == PI_DEVICE_INFO_PARENT) {
    // TODO: all L0 devices are parent ?
    SET_PARAM_VALUE(pi_device{0});
  }
  else if (param_name == PI_DEVICE_INFO_PLATFORM) {
    // This is our fake platform.
    SET_PARAM_VALUE(pi_cast<pi_platform>(&ze_driver_global));
  }
  else if (param_name == PI_DEVICE_INFO_VENDOR_ID) {
    SET_PARAM_VALUE(pi_uint32{ze_device_properties.vendorId});
  }
  else if (param_name == PI_DEVICE_INFO_EXTENSIONS) {
    // Convention adopted from OpenCL:
    //     "Returns a space separated list of extension names (the extension
    // names themselves do not contain any spaces) supported by the device."
    //
    // TODO: any DPC++ features need this?
    //
    SET_PARAM_VALUE_STR("");
  }
  else if (param_name == PI_DEVICE_INFO_NAME) {
    SET_PARAM_VALUE_STR(ze_device_properties.name);
  }
  else if (param_name == PI_DEVICE_INFO_COMPILER_AVAILABLE) {
    SET_PARAM_VALUE(pi_bool{1});
  }
  else if (param_name == PI_DEVICE_INFO_LINKER_AVAILABLE) {
    SET_PARAM_VALUE(pi_bool{1});
  }
  else if (param_name == PI_DEVICE_INFO_MAX_COMPUTE_UNITS) {
    pi_uint32 max_compute_units =
        ze_device_properties.numEUsPerSubslice *
        ze_device_properties.numSubslicesPerSlice *
        ze_device_properties.numSlicesPerTile *
        (ze_device_properties.numTiles > 0 ? ze_device_properties.numTiles : 1);
    SET_PARAM_VALUE(pi_uint32{max_compute_units});
  }
  else if (param_name == PI_DEVICE_INFO_MAX_WORK_ITEM_DIMENSIONS) {
    // L0 spec defines only three dimensions
    SET_PARAM_VALUE(pi_uint32{3});
  }
  else if (param_name == PI_DEVICE_INFO_MAX_WORK_GROUP_SIZE) {
    SET_PARAM_VALUE(pi_uint64{ze_device_compute_properties.maxTotalGroupSize});
  }
  else if (param_name == PI_DEVICE_INFO_MAX_WORK_ITEM_SIZES) {
    struct {
      size_t arr[3];
    }  max_group_size = {
      { ze_device_compute_properties.maxGroupSizeX,
        ze_device_compute_properties.maxGroupSizeY,
        ze_device_compute_properties.maxGroupSizeZ
      }
    };
    SET_PARAM_VALUE(max_group_size);
  }
  else if (param_name == PI_DEVICE_INFO_MAX_CLOCK_FREQUENCY) {
    SET_PARAM_VALUE(pi_uint32{ze_device_properties.coreClockRate});
  }
  else if (param_name == PI_DEVICE_INFO_ADDRESS_BITS) {
    // TODO: To confirm with spec.
    SET_PARAM_VALUE(pi_uint32{64});
  }
  else if (param_name == PI_DEVICE_INFO_MAX_MEM_ALLOC_SIZE) {
    // TODO: To confirm with spec.
    uint32_t max_mem_alloc_size = 0;
    for (uint32_t i = 0; i < ze_avail_mem_count; i++) {
      max_mem_alloc_size += ze_device_memory_properties[i].totalSize;
    }
    SET_PARAM_VALUE(pi_uint64{max_mem_alloc_size});
  }
  else if (param_name == PI_DEVICE_INFO_GLOBAL_MEM_SIZE) {
    uint32_t global_mem_size = 0;
    for (uint32_t i = 0; i < ze_avail_mem_count; i++) {
      global_mem_size += ze_device_memory_properties[i].totalSize;
    }
    SET_PARAM_VALUE(pi_uint64{global_mem_size});
  }
  else if (param_name == PI_DEVICE_INFO_LOCAL_MEM_SIZE) {
    SET_PARAM_VALUE(pi_uint64{ze_device_compute_properties.maxSharedLocalMemory});
  }
  else if (param_name == PI_DEVICE_INFO_IMAGE_SUPPORT) {
    SET_PARAM_VALUE(pi_bool{ze_device_image_properties.supported});
  }
  else if (param_name == PI_DEVICE_INFO_HOST_UNIFIED_MEMORY) {
    SET_PARAM_VALUE(pi_bool{ze_device_properties.unifiedMemorySupported});
  }
  else if (param_name == PI_DEVICE_INFO_AVAILABLE) {
    SET_PARAM_VALUE(pi_bool{ze_device ? true : false});
  }
  else if (param_name == PI_DEVICE_INFO_VENDOR) {
    // TODO: Level-Zero does not return vendor's name at the moment
    // only the ID.
    SET_PARAM_VALUE_STR("Intel");
  }
  else if (param_name == PI_DEVICE_INFO_DRIVER_VERSION) {
    pi_assert(ze_driver_global != nullptr);
    ze_driver_properties_t ze_driver_properties;
    ZE_CALL(zeDriverGetProperties(ze_driver_global, &ze_driver_properties));
    uint32_t ze_driver_version = ze_driver_properties.driverVersion;
    std::string driver_version =
        std::to_string(ZE_MAJOR_VERSION(ze_driver_version)) +
        std::string(".") +
        std::to_string(ZE_MINOR_VERSION(ze_driver_version));
    SET_PARAM_VALUE_STR(driver_version.c_str());
  }
  else if (param_name == PI_DEVICE_INFO_VERSION) {
    std::string version = std::to_string(pi_cast<pi_uint32>(ze_device_properties.version));
    SET_PARAM_VALUE_STR(version.c_str());
  }
  else if (param_name == PI_DEVICE_INFO_PARTITION_MAX_SUB_DEVICES) {
    SET_PARAM_VALUE(pi_uint32{ze_device_properties.numTiles});
  }
  else if (param_name == PI_DEVICE_INFO_REFERENCE_COUNT) {
    SET_PARAM_VALUE(pi_uint32{device->RefCount});
  }
  else if (param_name == PI_DEVICE_INFO_PARTITION_PROPERTIES) {
    //
    // It is debatable if SYCL sub-device and partitioning APIs sufficient to
    // expose Level0 sub-devices / tiles?  We start with support of
    // "partition_by_affinity_domain" and "numa" but if that doesn't seem to be
    // a good fit we could look at adding a more descriptive partitioning type.
    //
    // See https://gitlab.devtools.intel.com/one-api/level_zero/issues/239.
    //
    struct {
      pi_device_partition_property arr[2];
    }  partition_properties = {
      { PI_DEVICE_PARTITION_BY_AFFINITY_DOMAIN, 0 }
    };
    SET_PARAM_VALUE(partition_properties);
  }
  else if (param_name == PI_DEVICE_INFO_PARTITION_AFFINITY_DOMAIN) {
    SET_PARAM_VALUE(pi_device_affinity_domain{
      PI_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE});
  }
  else if (param_name == PI_DEVICE_INFO_PARTITION_TYPE) {
    if (device->IsSubDevice) {
      struct {
        pi_device_partition_property arr[3];
      }  partition_properties = {
        { PI_DEVICE_PARTITION_BY_AFFINITY_DOMAIN,
          PI_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE, 0 }
      };
      SET_PARAM_VALUE(partition_properties);
    }
    else {
      // For root-device there is no partitioning to report.
      SET_PARAM_VALUE(pi_device_partition_property{0});
    }
  }

  // Everything under here is not supported yet

  else if (param_name == PI_DEVICE_INFO_OPENCL_C_VERSION) {
    SET_PARAM_VALUE_STR("");
  }
  else if (param_name == PI_DEVICE_INFO_PREFERRED_INTEROP_USER_SYNC) {
    SET_PARAM_VALUE(pi_bool{true});
  }
  else if (param_name == PI_DEVICE_INFO_PRINTF_BUFFER_SIZE) {
    SET_PARAM_VALUE(size_t{ze_device_kernel_properties.printfBufferSize});
  }
  else if (param_name == PI_DEVICE_INFO_PROFILE) {
    SET_PARAM_VALUE_STR("FULL_PROFILE");
  }
  else if (param_name == PI_DEVICE_INFO_BUILT_IN_KERNELS) {
    // TODO: To find out correct value
    SET_PARAM_VALUE_STR("");
  }
  else if (param_name == PI_DEVICE_INFO_QUEUE_PROPERTIES) {
    SET_PARAM_VALUE(pi_queue_properties{PI_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
                    PI_QUEUE_PROFILING_ENABLE});
  }
  else if (param_name == PI_DEVICE_INFO_EXECUTION_CAPABILITIES) {
    SET_PARAM_VALUE(pi_device_exec_capabilities{PI_DEVICE_EXEC_CAPABILITIES_NATIVE_KERNEL});
  }
  else if (param_name == PI_DEVICE_INFO_ENDIAN_LITTLE) {
    SET_PARAM_VALUE(pi_bool{true});
  }
  else if (param_name == PI_DEVICE_INFO_ERROR_CORRECTION_SUPPORT) {
    SET_PARAM_VALUE(pi_bool{ze_device_properties.eccMemorySupported});
  }
  else if (param_name == PI_DEVICE_INFO_PROFILING_TIMER_RESOLUTION) {
    SET_PARAM_VALUE(size_t{ze_device_properties.timerResolution });
  }
  else if (param_name == PI_DEVICE_INFO_LOCAL_MEM_TYPE) {
    SET_PARAM_VALUE(PI_DEVICE_LOCAL_MEM_TYPE_LOCAL);
  }
  else if (param_name == PI_DEVICE_INFO_MAX_CONSTANT_ARGS) {
    SET_PARAM_VALUE(pi_uint32{64});
  }
  else if (param_name == PI_DEVICE_INFO_MAX_CONSTANT_BUFFER_SIZE) {
    SET_PARAM_VALUE(pi_uint64{ze_device_image_properties.maxImageBufferSize});
  }
  else if (param_name == PI_DEVICE_INFO_GLOBAL_MEM_CACHE_TYPE) {
    SET_PARAM_VALUE(PI_DEVICE_MEM_CACHE_TYPE_READ_WRITE_CACHE);
  }
  else if (param_name == PI_DEVICE_INFO_GLOBAL_MEM_CACHELINE_SIZE) {
    SET_PARAM_VALUE(pi_uint32{ze_device_cache_properties.lastLevelCachelineSize});
  }
  else if (param_name == PI_DEVICE_INFO_GLOBAL_MEM_CACHE_SIZE) {
    SET_PARAM_VALUE(pi_uint64{ze_device_cache_properties.lastLevelCacheSize});
  }
  else if (param_name == PI_DEVICE_INFO_MAX_PARAMETER_SIZE) {
      SET_PARAM_VALUE(size_t{ze_device_kernel_properties.maxArgumentsSize});
  }
  else if (param_name == PI_DEVICE_INFO_MEM_BASE_ADDR_ALIGN) {
    SET_PARAM_VALUE(pi_uint32{8});
  }
  else if (param_name == PI_DEVICE_INFO_MAX_SAMPLERS) {
    SET_PARAM_VALUE(pi_uint32{ze_device_image_properties.maxSamplers});
  }
  else if (param_name == PI_DEVICE_INFO_MAX_READ_IMAGE_ARGS) {
    SET_PARAM_VALUE(pi_uint32{ze_device_image_properties.maxReadImageArgs});
  }
  else if (param_name == PI_DEVICE_INFO_MAX_WRITE_IMAGE_ARGS) {
    SET_PARAM_VALUE(pi_uint32{ze_device_image_properties.maxWriteImageArgs});
  }
  else if (param_name == PI_DEVICE_INFO_SINGLE_FP_CONFIG) {
    uint32_t singleFPValue = 0;
    ze_floating_point_capabilities_t singleFpCapabilities = ze_device_kernel_properties.singleFpCapabilities;
    if (ZE_FP_CAPS_DENORM & singleFpCapabilities) {
      singleFPValue |= CL_FP_DENORM;
    }
    if (ZE_FP_CAPS_INF_NAN & singleFpCapabilities) {
      singleFPValue |= CL_FP_INF_NAN;
    }
    if (ZE_FP_CAPS_ROUND_TO_NEAREST  & singleFpCapabilities) {
      singleFPValue |= CL_FP_ROUND_TO_NEAREST;
    }
    if (ZE_FP_CAPS_ROUND_TO_ZERO & singleFpCapabilities) {
      singleFPValue |= CL_FP_ROUND_TO_ZERO;
    }
    if (ZE_FP_CAPS_ROUND_TO_INF & singleFpCapabilities) {
      singleFPValue |= CL_FP_ROUND_TO_INF;
    }
    if (ZE_FP_CAPS_FMA & singleFpCapabilities) {
      singleFPValue |= CL_FP_FMA;
    }
    SET_PARAM_VALUE(pi_uint32{singleFPValue});
  }
  else if (param_name == PI_DEVICE_INFO_HALF_FP_CONFIG) {
    // TODO: To find out correct value
    printf("Unsupported PI_DEVICE_INFO_HALF_FP_CONFIG in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_INFO_DOUBLE_FP_CONFIG) {
    uint32_t doubleFPValue = 0;
    ze_floating_point_capabilities_t doubleFpCapabilities = ze_device_kernel_properties.doubleFpCapabilities;
    if (ZE_FP_CAPS_DENORM & doubleFpCapabilities) {
      doubleFPValue |= CL_FP_DENORM;
    }
    if (ZE_FP_CAPS_INF_NAN & doubleFpCapabilities) {
      doubleFPValue |= CL_FP_INF_NAN;
    }
    if (ZE_FP_CAPS_ROUND_TO_NEAREST  & doubleFpCapabilities) {
      doubleFPValue |= CL_FP_ROUND_TO_NEAREST;
    }
    if (ZE_FP_CAPS_ROUND_TO_ZERO & doubleFpCapabilities) {
      doubleFPValue |= CL_FP_ROUND_TO_ZERO;
    }
    if (ZE_FP_CAPS_ROUND_TO_INF & doubleFpCapabilities) {
      doubleFPValue |= CL_FP_ROUND_TO_INF;
    }
    if (ZE_FP_CAPS_FMA & doubleFpCapabilities) {
      doubleFPValue |= CL_FP_FMA;
    }
    SET_PARAM_VALUE(pi_uint32{doubleFPValue});
  }
  else if (param_name == PI_DEVICE_INFO_IMAGE2D_MAX_WIDTH) {
    // TODO: https://gitlab.devtools.intel.com/one-api/level_zero/issues/288
    // Until L0 provides needed info, hardcode default minimum values required
    // by the SYCL specification.
    //
    SET_PARAM_VALUE(size_t{8192});
  }
  else if (param_name == PI_DEVICE_INFO_IMAGE2D_MAX_HEIGHT) {
    // TODO: https://gitlab.devtools.intel.com/one-api/level_zero/issues/288
    // Until L0 provides needed info, hardcode default minimum values required
    // by the SYCL specification.
    //
    SET_PARAM_VALUE(size_t{8192});
  }
  else if (param_name == PI_DEVICE_INFO_IMAGE3D_MAX_WIDTH) {
    // TODO: https://gitlab.devtools.intel.com/one-api/level_zero/issues/288
    // Until L0 provides needed info, hardcode default minimum values required
    // by the SYCL specification.
    //
    SET_PARAM_VALUE(size_t{2048});
  }
  else if (param_name == PI_DEVICE_INFO_IMAGE3D_MAX_HEIGHT) {
    // TODO: https://gitlab.devtools.intel.com/one-api/level_zero/issues/288
    // Until L0 provides needed info, hardcode default minimum values required
    // by the SYCL specification.
    //
    SET_PARAM_VALUE(size_t{2048});
  }
  else if (param_name == PI_DEVICE_INFO_IMAGE3D_MAX_DEPTH) {
    // TODO: https://gitlab.devtools.intel.com/one-api/level_zero/issues/288
    // Until L0 provides needed info, hardcode default minimum values required
    // by the SYCL specification.
    //
    SET_PARAM_VALUE(size_t{2048});
  }
  else if (param_name == PI_DEVICE_INFO_IMAGE_MAX_BUFFER_SIZE) {
    SET_PARAM_VALUE(size_t{ze_device_image_properties.maxImageBufferSize});
  }
  else if (param_name == PI_DEVICE_INFO_IMAGE_MAX_ARRAY_SIZE) {
    SET_PARAM_VALUE(size_t{ze_device_image_properties.maxImageArraySlices});
  }
  //
  // Handle SIMD widths.
  // TODO: can we do better than this?
  // See https://gitlab.devtools.intel.com/one-api/level_zero/issues/239.
  //
  else if (param_name == PI_DEVICE_INFO_NATIVE_VECTOR_WIDTH_CHAR ||
           param_name == PI_DEVICE_INFO_PREFERRED_VECTOR_WIDTH_CHAR) {
    SET_PARAM_VALUE(ze_device_properties.physicalEUSimdWidth / 1);
  }
  else if (param_name == PI_DEVICE_INFO_NATIVE_VECTOR_WIDTH_SHORT ||
           param_name == PI_DEVICE_INFO_PREFERRED_VECTOR_WIDTH_SHORT) {
    SET_PARAM_VALUE(ze_device_properties.physicalEUSimdWidth / 2);
  }
  else if (param_name == PI_DEVICE_INFO_NATIVE_VECTOR_WIDTH_INT ||
           param_name == PI_DEVICE_INFO_PREFERRED_VECTOR_WIDTH_INT) {
    SET_PARAM_VALUE(ze_device_properties.physicalEUSimdWidth / 4);
  }
  else if (param_name == PI_DEVICE_INFO_NATIVE_VECTOR_WIDTH_LONG ||
           param_name == PI_DEVICE_INFO_PREFERRED_VECTOR_WIDTH_LONG) {
    SET_PARAM_VALUE(ze_device_properties.physicalEUSimdWidth / 8);
  }
  else if (param_name == PI_DEVICE_INFO_NATIVE_VECTOR_WIDTH_FLOAT ||
           param_name == PI_DEVICE_INFO_PREFERRED_VECTOR_WIDTH_FLOAT) {
    SET_PARAM_VALUE(ze_device_properties.physicalEUSimdWidth / 4);
  }
  else if (param_name == PI_DEVICE_INFO_NATIVE_VECTOR_WIDTH_DOUBLE ||
           param_name == PI_DEVICE_INFO_PREFERRED_VECTOR_WIDTH_DOUBLE) {
    SET_PARAM_VALUE(ze_device_properties.physicalEUSimdWidth / 8);
  }
  else if (param_name == PI_DEVICE_INFO_NATIVE_VECTOR_WIDTH_HALF ||
           param_name == PI_DEVICE_INFO_PREFERRED_VECTOR_WIDTH_HALF) {
    SET_PARAM_VALUE(ze_device_properties.physicalEUSimdWidth / 2);
  }
  else if (param_name == PI_DEVICE_INFO_USM_HOST_SUPPORT ||
           param_name == PI_DEVICE_INFO_USM_DEVICE_SUPPORT ||
           param_name == PI_DEVICE_INFO_USM_SINGLE_SHARED_SUPPORT ||
           param_name == PI_DEVICE_INFO_USM_CROSS_SHARED_SUPPORT ||
           param_name == PI_DEVICE_INFO_USM_SYSTEM_SHARED_SUPPORT) {

    pi_uint64 supported = 0;
    if (ze_device_properties.unifiedMemorySupported) {
      // TODO: Use ze_memory_access_capabilities_t
      supported =
          PI_USM_ACCESS |
          PI_USM_ATOMIC_ACCESS |
          PI_USM_CONCURRENT_ACCESS |
          PI_USM_CONCURRENT_ATOMIC_ACCESS;
    }
    SET_PARAM_VALUE(supported);
  }
  else {
    fprintf(stderr, "param_name=%d(0x%x)\n", param_name, param_name);
    pi_throw("Unsupported param_name in piGetDeviceInfo");
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

  // Get the number of subdevices/tiles available.
  // TODO: maybe add interface to create the specified # of subdevices.
  uint32_t count = 0;
  ZE_CALL(zeDeviceGetSubDevices(device->L0Device, &count, nullptr));

  // Check that the requested/allocated # of sub-devices is the same
  // as was reported by the above call.
  // TODO: we may want to support smaller/larger # devices too.
  if (count != num_devices) {
    pi_throw("piDevicePartition: unsupported # of sub-devices requested");
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
    auto L0PiDevice = new _pi_device();
    L0PiDevice->L0Device = ze_subdevices[i];
    L0PiDevice->IsSubDevice = true;
    L0PiDevice->RefCount = 1;
    out_devices[i] = L0PiDevice;
  }
  delete[] ze_subdevices;

  return PI_SUCCESS;
}

pi_result L0(piextDeviceSelectBinary)(
  pi_device           device, // TODO: does this need to be context?
  pi_device_binary *  binaries,
  pi_uint32           num_binaries,
  pi_device_binary *  selected_binary) {

  // TODO dummy implementation.
  // Real implementaion will use the same mechanism OpenCL ICD dispatcher
  // uses. Somthing like:
  //   PI_VALIDATE_HANDLE_RETURN_HANDLE(ctx, PI_INVALID_CONTEXT);
  //     return context->dispatch->piextDeviceSelectIR(
  //       ctx, images, num_images, selected_image);
  // where context->dispatch is set to the dispatch table provided by PI
  // plugin for platform/device the ctx was created for.

  *selected_binary = num_binaries > 0 ? binaries[0] : nullptr;
  return PI_SUCCESS;
}

pi_result L0(piContextCreate)(
  const cl_context_properties * properties,
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
    pi_throw("piCreateContext: context should have exactly one device");
  }

  auto L0PiContext = new _pi_context();
  L0PiContext->Device = *devices;
  L0PiContext->RefCount = 1;

  *ret_context = L0PiContext;
  return PI_SUCCESS;
}

pi_result L0(piContextGetInfo)(
  pi_context         context,
  pi_context_info    param_name,
  size_t             param_value_size,
  void *             param_value,
  size_t *           param_value_size_ret) {

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
    pi_throw("piGetContextInfo: unsuppported param_name.");
  }

  return PI_SUCCESS;
}

pi_result L0(piContextRetain)(
  pi_context context) {

  ++(context->RefCount);
  return PI_SUCCESS;
}

pi_result L0(piContextRelease)(
  pi_context context) {

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

  ze_result_t ze_result;
  ze_device_handle_t        ze_device;
  ze_command_queue_handle_t ze_command_queue;

  pi_assert(device == context->Device);
  ze_device = device->L0Device;

  ze_command_queue_desc_t ze_command_queue_desc =
    {ZE_COMMAND_QUEUE_DESC_VERSION_CURRENT};
  ze_command_queue_desc.ordinal = 0;
  ze_command_queue_desc.mode = ZE_COMMAND_QUEUE_MODE_ASYNCHRONOUS;

  ze_result = ZE_CALL(zeCommandQueueCreate(
    ze_device,
    &ze_command_queue_desc,  // TODO: translate properties
    &ze_command_queue));

  auto L0PiQueue = new _pi_queue();
  L0PiQueue->L0CommandQueue = ze_command_queue;
  L0PiQueue->Context = context;
  *queue = L0PiQueue;

  // TODO: translate errors
  return pi_cast<pi_result>(ze_result);
}

pi_result L0(piQueueGetInfo)(
  pi_queue            command_queue,
  pi_queue_info       param_name,
  size_t              param_value_size,
  void *              param_value,
  size_t *            param_value_size_ret) {

  // TODO: consider support for queue properties and size
  if (param_name == PI_QUEUE_INFO_CONTEXT) {
    SET_PARAM_VALUE(command_queue->Context);
  }
  else if (param_name == PI_QUEUE_INFO_DEVICE) {
    SET_PARAM_VALUE(command_queue->Context->Device);
  }
  else if (param_name == PI_QUEUE_INFO_REFERENCE_COUNT) {
    SET_PARAM_VALUE(pi_uint32{command_queue->RefCount});
  }
  else if (param_name == PI_QUEUE_INFO_PROPERTIES) {
    pi_throw("Unsupported PI_QUEUE_INFO_PROPERTIES in piQueueGetInfo\n");
  }
  else if (param_name == PI_QUEUE_INFO_SIZE) {
    pi_throw("Unsupported PI_QUEUE_INFO_SIZE in piQueueGetInfo\n");
  }
  else if (param_name == PI_QUEUE_INFO_DEVICE_DEFAULT) {
    pi_throw("Unsupported PI_QUEUE_INFO_DEVICE_DEFAULT in piQueueGetInfo\n");
  }
  else {
    fprintf(stderr, "param_name=%d(0x%x)\n", param_name, param_name);
    pi_throw("Unsupported param_name in piQueueGetInfo");
  }

  return PI_SUCCESS;
}


pi_result L0(piQueueRetain)(pi_queue command_queue) {
  return PI_SUCCESS;
}

pi_result L0(piQueueRelease)(pi_queue command_queue) {
    // TODO: handle errors
  ZE_CALL(zeCommandQueueDestroy(
    command_queue->L0CommandQueue));

  return PI_SUCCESS;
}

pi_result L0(piQueueFinish)(pi_queue command_queue)
{
  // Wait until command lists attached to the command queue are executed.
  ZE_CALL(zeCommandQueueSynchronize(
    command_queue->L0CommandQueue, UINT32_MAX));
  return PI_SUCCESS;
}

pi_result piMemBufferCreate(
  pi_context   context,
  pi_mem_flags flags,
  size_t       size,
  void *       host_ptr,
  pi_mem *     ret_mem) {

 // TODO: implement read-only, write-only
  pi_assert((flags & PI_MEM_FLAGS_ACCESS_RW) != 0);

  void *ptr;
  ze_device_handle_t ze_device = context->Device->L0Device;

  // TODO: translate errors
  ze_device_mem_alloc_desc_t ze_desc;
  ze_desc.flags = ZE_DEVICE_MEM_ALLOC_FLAG_DEFAULT;
  ze_desc.ordinal = 0;
  ZE_CALL(zeDriverAllocDeviceMem(
    ze_driver_global,
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
    pi_throw("piMemBufferCreate: not implemented");
  }

  auto L0PiMem = new _pi_mem();
  L0PiMem->L0Mem = pi_cast<char*>(ptr);
  L0PiMem->RefCount = 1;
  L0PiMem->IsMemImage = false;
  L0PiMem->MapHostPtr = (flags & PI_MEM_FLAGS_HOST_PTR_USE) ?
      pi_cast<char*>(host_ptr): nullptr;
  *ret_mem = L0PiMem;

  return PI_SUCCESS;
}

pi_result L0(piMemGetInfo)(
  pi_mem           mem,
  cl_mem_info      param_name, // TODO: untie from OpenCL
  size_t           param_value_size,
  void *           param_value,
  size_t *         param_value_size_ret) {
  pi_throw("piMemGetInfo: not implemented");
}


pi_result L0(piMemRetain)(pi_mem mem) {
  ++(mem->RefCount);
  return PI_SUCCESS;
}

pi_result L0(piMemRelease)(pi_mem mem) {
  if (--(mem->RefCount) == 0) {
    if (mem->IsMemImage) {
      ZE_CALL(zeImageDestroy(mem->L0Image));
    }
    else {
      ZE_CALL(zeDriverFreeMem(ze_driver_global, mem->L0Mem));
    }
    delete mem;
  }
  // TODO: handle errors
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
  pi_assert((flags & PI_MEM_FLAGS_ACCESS_RW) != 0);

  ze_image_format_type_t ze_image_format_type;
  size_t ze_image_format_type_size;
  switch (image_format->image_channel_data_type) {
  case CL_FLOAT:
    ze_image_format_type = ZE_IMAGE_FORMAT_TYPE_FLOAT;
    ze_image_format_type_size = 32;
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
    zePrint("data type = %d\n", image_format->image_channel_data_type);
    pi_throw("piMemImageCreate: unsupported image data type");
    break;
  }

  // TODO: populate the layout mapping
  ze_image_format_layout_t ze_image_format_layout;
  switch (image_format->image_channel_order) {
  case CL_RGBA:
    switch (ze_image_format_type_size) {
    case 8:  ze_image_format_layout = ZE_IMAGE_FORMAT_LAYOUT_8_8_8_8; break;
    case 16: ze_image_format_layout = ZE_IMAGE_FORMAT_LAYOUT_16_16_16_16; break;
    case 32: ze_image_format_layout = ZE_IMAGE_FORMAT_LAYOUT_32_32_32_32; break;
    default: pi_throw("piMemImageCreate: unexpected data type size"); break;
    }
    break;
  default:
    zePrint("format layout = %d\n", image_format->image_channel_order);
    pi_throw("piMemImageCreate: unsupported image format layout");
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
  default: pi_throw("piMemImageCreate: unsupported image type");
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

  auto L0PiImage = new _pi_mem();
  L0PiImage->L0Image = hImage;
  L0PiImage->RefCount = 1;
  L0PiImage->IsMemImage = true;
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

pi_result L0(piProgramCreate)(
  pi_context    context,
  const void *  il,
  size_t        length,
  pi_program *  program) {

  ze_device_handle_t ze_device = context->Device->L0Device;

  ze_module_desc_t ze_module_desc;
  ze_module_desc.version = ZE_MODULE_DESC_VERSION_CURRENT;
  ze_module_desc.format = ZE_MODULE_FORMAT_IL_SPIRV;
  ze_module_desc.inputSize = length;
  ze_module_desc.pInputModule = pi_cast<const uint8_t*>(il);
  ze_module_desc.pBuildFlags = nullptr;

  ze_module_handle_t ze_module;
  // TODO: handle errors
  ZE_CALL(zeModuleCreate(
    ze_device,
    &ze_module_desc,
    &ze_module,
    0)); // TODO: handle build log

  auto L0PiProgram = new _pi_program();
  L0PiProgram->L0Module = ze_module;
  L0PiProgram->Context = context;
  L0PiProgram->RefCount = 1;

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
  pi_assert(num_devices == 1);
  pi_assert(device_list && device_list[0] == context->Device);
  ze_device_handle_t ze_device = context->Device->L0Device;

  // Check the binary too.
  pi_assert(lengths && lengths[0] != 0);
  pi_assert(binaries && binaries[0] != nullptr);
  size_t length = lengths[0];
  auto binary = pi_cast<const uint8_t*>(binaries[0]);

  ze_module_desc_t ze_module_desc;
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

  auto L0PiProgram = new _pi_program();
  L0PiProgram->L0Module = ze_module;
  L0PiProgram->Context = context;
  L0PiProgram->RefCount = 1;

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

  pi_throw("piclProgramCreateWithSource: not supported in L0");
}

pi_result L0(piProgramGetInfo)(
  pi_program          program,
  pi_program_info     param_name,
  size_t              param_value_size,
  void *              param_value,
  size_t *            param_value_size_ret) {

  if (param_name == PI_PROGRAM_INFO_REFERENCE_COUNT) {
    SET_PARAM_VALUE(pi_uint32{program->RefCount});
  }
  else if (param_name == PI_PROGRAM_INFO_NUM_DEVICES) {
    // L0 Module is always for a single device.
    SET_PARAM_VALUE(pi_uint32{1});
  }
  else if (param_name == PI_PROGRAM_INFO_DEVICES) {
    SET_PARAM_VALUE(program->Context->Device);
  }
  else if (param_name == PI_PROGRAM_INFO_BINARY_SIZES) {
    size_t szBinary = 0;
    ZE_CALL(zeModuleGetNativeBinary(program->L0Module, &szBinary, nullptr));
    // This is an array of 1 element, initialize if it were scalar.
    SET_PARAM_VALUE(size_t{szBinary});
  }
  else if (param_name == PI_PROGRAM_INFO_BINARIES) {
    size_t szBinary = 0;
    uint8_t **pBinary = pi_cast<uint8_t **>(param_value);
    ZE_CALL(zeModuleGetNativeBinary(program->L0Module, &szBinary, pBinary[0]));
  }
  else if (param_name == PI_PROGRAM_INFO_NUM_KERNELS) {
    uint32_t num_kernels = 0;
    ZE_CALL(zeModuleGetKernelNames(program->L0Module, &num_kernels, nullptr));
    SET_PARAM_VALUE(size_t{num_kernels});
  }
  else if (param_name == PI_PROGRAM_INFO_KERNEL_NAMES) {
    // There are extra allocations/copying here dictated by the difference
    // in L0 and PI interfaces. Also see discussions at
    // https://gitlab.devtools.intel.com/one-api/level_zero/issues/305.
    //
    uint32_t count = 0;
    ZE_CALL(zeModuleGetKernelNames(program->L0Module, &count, nullptr));
    char** pNames = new char*[count];
    ZE_CALL(zeModuleGetKernelNames(program->L0Module, &count,
                                   const_cast<const char **>(pNames)));
    std::string piNames{""};
    for (uint32_t i = 0; i < count; ++i) {
      piNames += (i > 0 ? ";" : "");
      piNames += pNames[i];
    }
    delete[] pNames;
    SET_PARAM_VALUE_STR(piNames.c_str());
  }
  else {
    pi_throw("piProgramGetInfo: not implemented");
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
  pi_assert(num_input_programs == 1 && input_programs);
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

  pi_throw("piProgramGetBuildInfo: not implemented");
}

pi_result L0(piProgramRetain)(pi_program program) {
  ++(program->RefCount);
  return PI_SUCCESS;
}

pi_result L0(piProgramRelease)(pi_program program) {
  if (--(program->RefCount) == 0) {
    delete program;
  }
  return PI_SUCCESS;
}

pi_result L0(piKernelCreate)(
  pi_program      program,
  const char *    kernel_name,
  pi_kernel *     ret_kernel) {

  ze_kernel_desc_t ze_kernel_desc;
  ze_kernel_desc.version = ZE_KERNEL_DESC_VERSION_CURRENT;
  ze_kernel_desc.flags = ZE_KERNEL_FLAG_NONE;
  ze_kernel_desc.pKernelName = kernel_name;

  ze_kernel_handle_t ze_kernel;
  // TODO: handle errors
  ZE_CALL(zeKernelCreate(
    pi_cast<ze_module_handle_t>(program->L0Module),
    &ze_kernel_desc,
    &ze_kernel));

  auto L0PiKernel = new _pi_kernel();
  L0PiKernel->L0Kernel = ze_kernel;
  L0PiKernel->Program = program;
  L0PiKernel->RefCount = 1;

  *ret_kernel = pi_cast<pi_kernel>(L0PiKernel);
  return PI_SUCCESS;
}

pi_result L0(piKernelSetArg)(
  pi_kernel    kernel,
  pi_uint32    arg_index,
  size_t       arg_size,
  const void * arg_value) {

  // TODO: handle errors
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
  // We don't care to understand here which exactly PI object we are
  // dealing with here as long as corresponding native L0 handle is
  // the first thing in the structure representing this PI object.
  //
  // TODO: the better way would probably be to add a new PI API for
  // extracting native PI object from PI handle, and have SYCL
  // RT pass that directly to the regular piKernelSetArg (and
  // then remove this piextKernelSetArgMemObj).
  //

  // TODO: handle errors
  ZE_CALL(zeKernelSetArgumentValue(
    pi_cast<ze_kernel_handle_t>(kernel->L0Kernel),
    pi_cast<uint32_t>(arg_index),
    sizeof(void *),
    pi_cast<const void*>(&(*arg_value)->L0Mem)));

  return PI_SUCCESS;
}

pi_result L0(piKernelGetInfo)(
  pi_kernel       kernel,
  pi_kernel_info  param_name,
  size_t          param_value_size,
  void *          param_value,
  size_t *        param_value_size_ret)
{
  ze_kernel_properties_t ze_kernel_properties;
  ze_kernel_properties.version = ZE_KERNEL_PROPERTIES_VERSION_CURRENT;
  ZE_CALL(zeKernelGetProperties(kernel->L0Kernel, &ze_kernel_properties));

  if (param_name == PI_KERNEL_INFO_CONTEXT) {
    SET_PARAM_VALUE(pi_context{kernel->Program->Context});
  }
  else if (param_name == PI_KERNEL_INFO_PROGRAM) {
    SET_PARAM_VALUE(pi_program{kernel->Program});
  }
  else if (param_name == PI_KERNEL_INFO_FUNCTION_NAME) {
    SET_PARAM_VALUE_STR(ze_kernel_properties.name);
  }
  else if (param_name == PI_KERNEL_INFO_NUM_ARGS) {
    SET_PARAM_VALUE(pi_uint32{ze_kernel_properties.numKernelArgs});
  }
  else if (param_name == PI_KERNEL_INFO_REFERENCE_COUNT) {
    SET_PARAM_VALUE(pi_uint32{kernel->RefCount});
  }
  else if (param_name == PI_KERNEL_INFO_ATTRIBUTES) {
    pi_throw("Unsupported PI_KERNEL_ATTRIBUTES in piKernelGetInfo\n");
  }
  else {
    fprintf(stderr, "param_name=%d(0x%x)\n", param_name, param_name);
    pi_throw("Unsupported param_name in piKernelGetInfo");
  }

  return PI_SUCCESS;
}

pi_result L0(piKernelGetGroupInfo)(
  pi_kernel                  kernel,
  pi_device                  device,
  cl_kernel_work_group_info  param_name,  // TODO: change to pi_kernel_group_info
  size_t                     param_value_size,
  void *                     param_value,
  size_t *                   param_value_size_ret)
{
  ze_device_handle_t ze_device = device->L0Device;
  ze_device_compute_properties_t ze_device_compute_properties;
  ze_device_compute_properties.version = ZE_DEVICE_COMPUTE_PROPERTIES_VERSION_CURRENT;
  ZE_CALL(zeDeviceGetComputeProperties(
    ze_device,
    &ze_device_compute_properties));

  ze_kernel_properties_t ze_kernel_properties;
  ze_kernel_properties.version = ZE_KERNEL_PROPERTIES_VERSION_CURRENT;
  ZE_CALL(zeKernelGetProperties(kernel->L0Kernel, &ze_kernel_properties));

  if (param_name == PI_KERNEL_GLOBAL_WORK_SIZE) {
    // TODO: To revisit after level_zero/issues/262 is resolved
    struct {
      size_t arr[3];
    }  work_size = {
      { ze_device_compute_properties.maxGroupSizeX,
        ze_device_compute_properties.maxGroupSizeY,
        ze_device_compute_properties.maxGroupSizeZ
      }
    };
    SET_PARAM_VALUE(work_size);
  }
  if (param_name == PI_KERNEL_WORK_GROUP_SIZE) {
    uint32_t X, Y, Z;
    ZE_CALL(zeKernelSuggestGroupSize(kernel->L0Kernel,
                                     10000, 10000, 10000, &X, &Y, &Z));
    SET_PARAM_VALUE(size_t{X * Y * Z});
  }
  else if (param_name == PI_KERNEL_COMPILE_WORK_GROUP_SIZE) {
    struct {
      size_t arr[3];
    }  wg_size = {
      { ze_kernel_properties.requiredGroupSizeX,
        ze_kernel_properties.requiredGroupSizeY,
        ze_kernel_properties.requiredGroupSizeZ
      }
    };
    SET_PARAM_VALUE(wg_size);
  }
  else if (param_name == PI_KERNEL_LOCAL_MEM_SIZE) {
    // TODO: Assume 0 for now, but follow-up on https://gitlab.devtools.intel.com/one-api/level_zero/issues/285
    SET_PARAM_VALUE(pi_uint32{0});
  }
  else if (param_name == PI_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE) {
    // TODO: Follow-up on https://gitlab.devtools.intel.com/one-api/level_zero/issues/285
    pi_throw("Unsupported PI_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE in piKernelGetInfo\n");
  }
  else if (param_name == PI_KERNEL_PRIVATE_MEM_SIZE) {
    // TODO: Follow-up on https://gitlab.devtools.intel.com/one-api/level_zero/issues/285
    pi_throw("Unsupported PI_KERNEL_PRIVATE_MEM_SIZE in piKernelGetInfo\n");
  }
  else {
    fprintf(stderr, "param_name=%d(0x%x)\n", param_name, param_name);
    pi_throw("piKernelGetGroupInfo: unknown param_name");
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

  pi_throw("piKernelGetSubGroupInfo: not implemented");
}

pi_result L0(piKernelRetain)(pi_kernel    kernel) {

  ++(kernel->RefCount);
  return PI_SUCCESS;
}

pi_result L0(piKernelRelease)(pi_kernel    kernel) {

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
  ze_group_count_t thread_group_dimensions {1, 1, 1};
  uint32_t wg[3];

  // global_work_size of unused dimensions must be set to 1
  if (work_dim < 3) {
    pi_assert(global_work_size[2] == 1);
  }
  if (work_dim < 2) {
    pi_assert(global_work_size[1] == 1);
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
    pi_throw("piEnqueueKernelLaunch: unsupported work_dim");
  }

  pi_assert(global_work_size[0] == (thread_group_dimensions.groupCountX * wg[0]));
  pi_assert(global_work_size[1] == (thread_group_dimensions.groupCountY * wg[1]));
  pi_assert(global_work_size[2] == (thread_group_dimensions.groupCountZ * wg[2]));

  ZE_CALL(zeKernelSetGroupSize(kernel->L0Kernel, wg[0], wg[1], wg[2]));

  // Get a new command list to be used on this call
  ze_command_list_handle_t ze_command_list =
    queue->Context->Device->createCommandList();

  L0(piEventCreate)(kernel->Program->Context, event);
  (*event)->Queue = queue;
  (*event)->CommandType = PI_COMMAND_TYPE_NDRANGE_KERNEL;
  (*event)->L0CommandList = ze_command_list;

  ze_event_handle_t ze_event = (*event)->L0Event;

  ze_event_handle_t *ze_event_wait_list =
    _pi_event::createL0EventList(num_events_in_wait_list, event_wait_list);

  // Add the command to the command list
  ze_result_t result = ZE_CALL(zeCommandListAppendLaunchKernel(
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
  queue->executeCommandList(ze_command_list);
  _pi_event::deleteL0EventList(ze_event_wait_list);

  // TODO: translate errors
  return pi_cast<pi_result>(result);
}

//
// Events
//
pi_result L0(piEventCreate)(
  pi_context    context,
  pi_event *    ret_event)
{
  ze_result_t ze_res;

  ze_event_pool_desc_t ze_event_pool_desc;
  ze_event_pool_desc.count = 1;
  ze_event_pool_desc.flags = ZE_EVENT_POOL_FLAG_HOST_VISIBLE;
  ze_event_pool_desc.version = ZE_EVENT_POOL_DESC_VERSION_CURRENT;

  // TODO: see if we can employ larger event pools for better efficency
  ze_event_pool_handle_t ze_event_pool;
  ze_device_handle_t ze_device = context->Device->L0Device;
  ze_res = ZE_CALL(zeEventPoolCreate(
    ze_driver_global,
    &ze_event_pool_desc,
    1,
    &ze_device,
    &ze_event_pool));

  if (ze_res) {
    return pi_cast<pi_result>(ze_res);
  }

  ze_event_handle_t ze_event;
  ze_event_desc_t ze_event_desc;
  ze_event_desc.signal = ZE_EVENT_SCOPE_FLAG_NONE;
  ze_event_desc.wait = ZE_EVENT_SCOPE_FLAG_NONE;
  ze_event_desc.version = ZE_EVENT_DESC_VERSION_CURRENT;
  ze_event_desc.index = 0;

  ze_res = ZE_CALL(zeEventCreate(
    ze_event_pool,
    &ze_event_desc,
    &ze_event));

  if (ze_res) {
    return pi_cast<pi_result>(ze_res);
  }

  auto PiEvent = new _pi_event;
  PiEvent->L0Event = ze_event;
  PiEvent->L0EventPool = ze_event_pool;
  PiEvent->Queue = nullptr;
  PiEvent->CommandType = PI_COMMAND_TYPE_USER;   // TODO: verify
  PiEvent->L0CommandList = nullptr;
  PiEvent->RefCount = 1;

  *ret_event = PiEvent;
  return PI_SUCCESS;
}

pi_result L0(piEventGetInfo)(
  pi_event         event,
  pi_event_info    param_name,
  size_t           param_value_size,
  void *           param_value,
  size_t *         param_value_size_ret) {

  if (param_name == PI_EVENT_INFO_COMMAND_QUEUE) {
    SET_PARAM_VALUE(pi_queue{event->Queue});
  }
  else if (param_name == PI_EVENT_INFO_CONTEXT) {
    SET_PARAM_VALUE(pi_context{event->Queue->Context});
  }
  else if (param_name == PI_EVENT_INFO_COMMAND_TYPE) {
    SET_PARAM_VALUE(pi_cast<pi_uint64>(event->CommandType));
  }
  else if (param_name == PI_EVENT_INFO_COMMAND_EXECUTION_STATUS) {
    ze_result_t ze_result;
    ze_result = ZE_CALL_NOTHROW(zeEventQueryStatus(event->L0Event));
    if (ze_result == ZE_RESULT_SUCCESS) {
      SET_PARAM_VALUE(pi_int32{CL_COMPLETE}); // Untie from OpenCL
    }
    else {
      // TODO: We don't know if the status is queueed, submitted or running.
      //       See https://gitlab.devtools.intel.com/one-api/level_zero/issues/243
      //       For now return "running", as others are unlikely to be of interest.
      SET_PARAM_VALUE(pi_int32{CL_RUNNING});
    }
  }
  else if (param_name == PI_EVENT_INFO_REFERENCE_COUNT) {
    SET_PARAM_VALUE(pi_uint32{event->RefCount});
  }
  else {
    fprintf(stderr, "param_name=%d(%x)\n", param_name, param_name);
    pi_throw("Unsupported param_name in piEventGetInfo");
  }

  return PI_SUCCESS;
}

pi_result L0(piEventGetProfilingInfo)(
  pi_event            event,
  cl_profiling_info   param_name, // TODO: untie from OpenCL
  size_t              param_value_size,
  void *              param_value,
  size_t *            param_value_size_ret) {

  if (param_name == CL_PROFILING_COMMAND_SUBMIT ||
      param_name == CL_PROFILING_COMMAND_START ||
      param_name == CL_PROFILING_COMMAND_END ) {

    // TODO: return dummy "0" until
    // https://gitlab.devtools.intel.com/one-api/level_zero/issues/290
    // is resolved.
    //
    SET_PARAM_VALUE(size_t{0});
  }
  else {
    pi_throw("piEventGetProfilingInfo: not supported param_name");
  }

  return PI_SUCCESS;
}

pi_result L0(piEventsWait)(
  pi_uint32           num_events,
  const pi_event *    event_list)
{
  ze_result_t ze_result;

  for (uint32_t i = 0; i < num_events; i++) {
    ze_event_handle_t ze_event = event_list[i]->L0Event;
    zePrint("ze_event = %lx\n", pi_cast<std::uintptr_t>(ze_event));
    // TODO: Using UINT32_MAX for timeout should have the desired
    // effect of waiting until the event is trigerred, but it seems that
    // it is causing an OS crash, so use an interruptable loop for now.
    //
    do {
      ze_result = ZE_CALL_NOTHROW(zeEventHostSynchronize(ze_event, 100000));
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

  // TODO: Can we support CL_SUBMITTED and CL_RUNNING?
  // See https://gitlab.devtools.intel.com/one-api/level_zero/issues/243
  //
  if (command_exec_callback_type != CL_COMPLETE) {
    pi_throw("piEventSetCallback: unsupported callback type");
  }

  // Execute the wait and callback trigger in a side thread to not
  // block the main host thread.
  //
  // TODO: we should probably remember the callback within
  //       the pi_event to not destroy the event before all
  //       are executed.
  //
  std::thread wait_thread([](pi_event    event,
                             pi_int32    command_exec_callback_type,
                             void (*     pfn_notify)(pi_event event,
                                                     pi_int32 event_command_status,
                                                     void *   user_data),
                             void *      user_data) {

    // Implements the wait for the event to complete.
    assert(command_exec_callback_type == CL_COMPLETE);
    ze_result_t ze_result;
    do {
      ze_result =
        ZE_CALL_NOTHROW(zeEventHostSynchronize(event->L0Event, 10000));
    } while (ze_result == ZE_RESULT_NOT_READY);

    // Call the callback.
    pfn_notify(event, command_exec_callback_type, user_data);

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
    pi_throw("piEventSetStatus: not implemented");
  }

  ze_result_t ze_result;
  ze_event_handle_t ze_event = event->L0Event;

  ze_result = ZE_CALL_NOTHROW(zeEventQueryStatus(ze_event));
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
      ZE_CALL(zeDriverFreeMem(ze_driver_global, event->CommandData));
      event->CommandData = nullptr;
    }

    ZE_CALL(zeEventDestroy(event->L0Event));
    ZE_CALL(zeEventPoolDestroy(event->L0EventPool));
    delete event;
  }
  return PI_SUCCESS;
}

//
// Sampler
//
pi_result L0(piSamplerCreate)(
  pi_context                     context,
  const pi_sampler_properties *  sampler_properties,
  pi_sampler *                   ret_sampler) {

  ze_device_handle_t ze_device = context->Device->L0Device;

  ze_sampler_handle_t ze_sampler;
  ze_sampler_desc_t ze_sampler_desc =
    {ZE_SAMPLER_DESC_VERSION_CURRENT};

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
            else
              pi_throw("piSamplerCreate: unsupported PI_SAMPLER_NORMALIZED_COORDS value");
          }
          break;

        case PI_SAMPLER_PROPERTIES_ADDRESSING_MODE:
          {
            pi_sampler_addressing_mode cur_value_addressing_mode =
              pi_cast<pi_sampler_addressing_mode>(
                pi_cast<pi_uint32>(*(++cur_property)));

            // TODO: add support for PI_SAMPLER_ADDRESSING_MODE_CLAMP_TO_EDGE
            if (cur_value_addressing_mode == PI_SAMPLER_ADDRESSING_MODE_NONE)
              ze_sampler_desc.addressMode = ZE_SAMPLER_ADDRESS_MODE_NONE;
            else if (cur_value_addressing_mode == PI_SAMPLER_ADDRESSING_MODE_REPEAT)
              ze_sampler_desc.addressMode = ZE_SAMPLER_ADDRESS_MODE_REPEAT;
            else if (cur_value_addressing_mode == PI_SAMPLER_ADDRESSING_MODE_CLAMP)
              ze_sampler_desc.addressMode = ZE_SAMPLER_ADDRESS_MODE_CLAMP;
            else if (cur_value_addressing_mode == PI_SAMPLER_ADDRESSING_MODE_MIRRORED_REPEAT)
              ze_sampler_desc.addressMode = ZE_SAMPLER_ADDRESS_MODE_MIRROR;
            else {
              fprintf(stderr, "PI_SAMPLER_ADDRESSING_MODE=%d\n", cur_value_addressing_mode);
              pi_throw("piSamplerCreate: unsupported PI_SAMPLER_ADDRESSING_MODE value");
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
              fprintf(stderr, "PI_SAMPLER_FILTER_MODE=%d\n", cur_value_filter_mode);
              pi_throw("piSamplerCreate: unsupported PI_SAMPLER_FILTER_MODE value");
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

  auto L0PiSampler = new _pi_sampler();
  L0PiSampler->L0Sampler = ze_sampler;
  L0PiSampler->RefCount = 1;

  *ret_sampler = L0PiSampler;
  return PI_SUCCESS;
}

pi_result L0(piSamplerGetInfo)(
  pi_sampler         sampler,
  pi_sampler_info    param_name,
  size_t             param_value_size,
  void *             param_value,
  size_t *           param_value_size_ret) {

  pi_throw("piSamplerGetInfo: not implemented");
}

pi_result L0(piSamplerRetain)(pi_sampler sampler) {
  ++(sampler->RefCount);
  return PI_SUCCESS;
}

pi_result L0(piSamplerRelease)(pi_sampler sampler) {
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
  pi_queue          command_queue,
  pi_uint32         num_events_in_wait_list,
  const pi_event *  event_wait_list,
  pi_event *        event) {

  pi_throw("piEnqueueEventsWait: not implemented");
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
  return enqueueMemCopyHelper(
    PI_COMMAND_TYPE_MEM_BUFFER_READ,
    queue,
    dst,
    blocking_read,
    size,
    src->L0Mem + offset,
    num_events_in_wait_list,
    event_wait_list,
    event);
}

pi_result L0(piEnqueueMemBufferReadRect)(
  pi_queue            command_queue,
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

  ze_result_t ze_result;
  // Get a new command list to be used on this call
  ze_command_list_handle_t ze_command_list =
    command_queue->Context->Device->createCommandList();

  L0(piEventCreate)(command_queue->Context, event);
  (*event)->Queue = command_queue;
  (*event)->CommandType = PI_COMMAND_TYPE_MEM_BUFFER_READ_RECT;
  (*event)->L0CommandList = ze_command_list;

  ze_event_handle_t ze_event = (*event)->L0Event;

  ze_event_handle_t *ze_event_wait_list =
    _pi_event::createL0EventList(num_events_in_wait_list, event_wait_list);

  ze_result = ZE_CALL(zeCommandListAppendWaitOnEvents(
    ze_command_list,
    num_events_in_wait_list,
    ze_event_wait_list
  ));
  pi_assert(ze_result == 0);

  zePrint("calling zeCommandListAppendWaitOnEvents() with\n"
                "  num_events_in_wait_list %d:",
                pi_cast<std::uintptr_t>(ze_event), num_events_in_wait_list);
  for (pi_uint32 i = 0; i < num_events_in_wait_list; i++) {
    zePrint(" %lx", pi_cast<std::uintptr_t>(ze_event_wait_list[i]));
  }
  zePrint("\n");

  uint32_t dstOriginX = pi_cast<uint32_t>(host_offset[0]);
  uint32_t dstOriginY = pi_cast<uint32_t>(host_offset[1]);
  uint32_t dstOriginZ = pi_cast<uint32_t>(host_offset[2]);

  uint32_t dstPitch = host_row_pitch;
  if (dstPitch == 0)
    dstPitch = pi_cast<uint32_t>(region[0]);

  if (host_slice_pitch == 0)
    host_slice_pitch = pi_cast<uint32_t>(region[1]) * dstPitch;

  uint32_t srcOriginX = pi_cast<uint32_t>(buffer_offset[0]);
  uint32_t srcOriginY = pi_cast<uint32_t>(buffer_offset[1]);
  uint32_t srcOriginZ = pi_cast<uint32_t>(buffer_offset[2]);

  uint32_t srcPitch = buffer_row_pitch;
  if (srcPitch == 0)
    srcPitch = pi_cast<uint32_t>(region[0]);

  if (buffer_slice_pitch == 0)
    buffer_slice_pitch = pi_cast<uint32_t>(region[1]) * srcPitch;

  uint32_t width = pi_cast<uint32_t>(region[0]);
  uint32_t height = pi_cast<uint32_t>(region[1]);
  uint32_t depth = pi_cast<uint32_t>(region[2]);

  char *destination_ptr = static_cast<char *>(ptr) + dstOriginZ;
  char *source_ptr = buffer->L0Mem + srcOriginZ;

  /*
  * Command List is Created for handling all enqueued Memory Copy Regions
  * and 1 Barrier. Once command_queue->executeCommandList() is called the
  * command list is closed & no more commands are added to this
  * command list such that the barrier only blocks the Memory Copy Regions enqueued.
  *
  * Until L0 Spec issue https://gitlab.devtools.intel.com/one-api/level_zero/issues/300
  * is resolved 3D buffer copies must be spit into multiple 2D buffer copies in the
  * sycl plugin.
  */
  const ze_copy_region_t dstRegion = {dstOriginX, dstOriginY, 0, width, height, 0};
  const ze_copy_region_t srcRegion = {srcOriginX, srcOriginY, 0, width, height, 0};

  // TODO: Remove the for loop and use the slice pitches
  for (uint32_t i = 0; i < depth; i++) {
    ze_result = ZE_CALL(zeCommandListAppendMemoryCopyRegion(
      ze_command_list,
      destination_ptr,
      &dstRegion,
      dstPitch,
      0, /* dstSlicePitc */
      source_ptr,
      &srcRegion,
      srcPitch,
      0, /* srcSlicePitch */
      nullptr
    ));

    destination_ptr += host_slice_pitch;
    source_ptr += buffer_slice_pitch;
  }
  zePrint("calling zeCommandListAppendMemoryCopyRegion()\n");

  ze_result = ZE_CALL(zeCommandListAppendBarrier(
    ze_command_list,
    ze_event,
    0,
    nullptr
  ));
  pi_assert(ze_result == 0);
  zePrint("calling zeCommandListAppendBarrier() with event %lx\n",
    pi_cast<std::uintptr_t>(ze_event));

  command_queue->executeCommandList(ze_command_list, blocking_read);
  _pi_event::deleteL0EventList(ze_event_wait_list);

  // TODO: translate errors
  return pi_cast<pi_result>(ze_result);
}

// Shared by all memory read/write/copy PI interfaces.
static pi_result enqueueMemCopyHelper(
  pi_command_type    command_type,
  pi_queue           command_queue,
  void *             dst,
  pi_bool            blocking_write,
  size_t             size,
  const void *       src,
  pi_uint32          num_events_in_wait_list,
  const pi_event *   event_wait_list,
  pi_event *         event) {

  ze_result_t               ze_result;
  // Get a new command list to be used on this call
  ze_command_list_handle_t ze_command_list =
    command_queue->Context->Device->createCommandList();

  L0(piEventCreate)(command_queue->Context, event);
  (*event)->Queue = command_queue;
  (*event)->CommandType = command_type;
  (*event)->L0CommandList = ze_command_list;

  ze_event_handle_t ze_event = (*event)->L0Event;

  ze_event_handle_t *ze_event_wait_list =
    _pi_event::createL0EventList(num_events_in_wait_list, event_wait_list);

  ze_result = ZE_CALL(zeCommandListAppendWaitOnEvents(
    ze_command_list,
    num_events_in_wait_list,
    ze_event_wait_list
  ));
  pi_assert(ze_result == 0);

  ze_result = ZE_CALL(zeCommandListAppendMemoryCopy(
    ze_command_list,
    dst,
    src,
    size,
    ze_event
  ));

  command_queue->executeCommandList(ze_command_list, blocking_write);
  zePrint("calling zeCommandListAppendMemoryCopy() with\n"
                  "  xe_event %lx\n"
                  "  num_events_in_wait_list %d:",
          pi_cast<std::uintptr_t>(ze_event), num_events_in_wait_list);
  for (pi_uint32 i = 0; i < num_events_in_wait_list; i++) {
    zePrint(" %lx", pi_cast<std::uintptr_t>(ze_event_wait_list[i]));
  }
  zePrint("\n");

  _pi_event::deleteL0EventList(ze_event_wait_list);

  // TODO: translate errors
  return pi_cast<pi_result>(ze_result);
}

pi_result L0(piEnqueueMemBufferWrite)(
  pi_queue           command_queue,
  pi_mem             buffer,
  pi_bool            blocking_write,
  size_t             offset,
  size_t             size,
  const void *       ptr,
  pi_uint32          num_events_in_wait_list,
  const pi_event *   event_wait_list,
  pi_event *         event) {

  return enqueueMemCopyHelper(
    PI_COMMAND_TYPE_MEM_BUFFER_WRITE,
    command_queue,
    buffer->L0Mem + offset, // dst
    blocking_write,
    size,
    ptr, // src
    num_events_in_wait_list,
    event_wait_list,
    event);
}

pi_result L0(piEnqueueMemBufferWriteRect)(
  pi_queue            command_queue,
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

  ze_result_t ze_result;
  // Get a new command list to be used on this call
  ze_command_list_handle_t ze_command_list =
    command_queue->Context->Device->createCommandList();

  L0(piEventCreate)(command_queue->Context, event);
  (*event)->Queue = command_queue;
  (*event)->CommandType = PI_COMMAND_TYPE_MEM_BUFFER_WRITE_RECT;
  (*event)->L0CommandList = ze_command_list;

  ze_event_handle_t ze_event = (*event)->L0Event;

  ze_event_handle_t *ze_event_wait_list =
    _pi_event::createL0EventList(num_events_in_wait_list, event_wait_list);

  ze_result = ZE_CALL(zeCommandListAppendWaitOnEvents(
    ze_command_list,
    num_events_in_wait_list,
    ze_event_wait_list
  ));
  pi_assert(ze_result == 0);

  zePrint("calling zeCommandListAppendWaitOnEvents() with\n"
                "  num_events_in_wait_list %d:",
                pi_cast<std::uintptr_t>(ze_event), num_events_in_wait_list);
  for (pi_uint32 i = 0; i < num_events_in_wait_list; i++) {
    zePrint(" %lx", pi_cast<std::uintptr_t>(ze_event_wait_list[i]));
  }
  zePrint("\n");

  uint32_t srcOriginX = pi_cast<uint32_t>(host_offset[0]);
  uint32_t srcOriginY = pi_cast<uint32_t>(host_offset[1]);
  uint32_t srcOriginZ = pi_cast<uint32_t>(host_offset[2]);

  uint32_t srcPitch = host_row_pitch;
  if (srcPitch == 0)
    srcPitch = pi_cast<uint32_t>(region[0]);

  if (host_slice_pitch == 0)
    host_slice_pitch = pi_cast<uint32_t>(region[1]) * srcPitch;

  uint32_t dstOriginX = pi_cast<uint32_t>(buffer_offset[0]);
  uint32_t dstOriginY = pi_cast<uint32_t>(buffer_offset[1]);
  uint32_t dstOriginZ = pi_cast<uint32_t>(buffer_offset[2]);

  uint32_t dstPitch = buffer_row_pitch;
  if (dstPitch == 0)
    dstPitch = pi_cast<uint32_t>(region[0]);

  if (buffer_slice_pitch == 0)
    buffer_slice_pitch = pi_cast<uint32_t>(region[1]) * dstPitch;

  uint32_t width = pi_cast<uint32_t>(region[0]);
  uint32_t height = pi_cast<uint32_t>(region[1]);
  uint32_t depth = pi_cast<uint32_t>(region[2]);

  char *source_ptr = const_cast<char *>(static_cast<const char *>(ptr)) + srcOriginZ;
  char *destination_ptr = buffer->L0Mem + dstOriginZ;

  //
  // Command List is Created for handling all enqueued Memory Copy Regions
  // and 1 Barrier. Once command_queue->executeCommandList() is called the
  // command list is closed & no more commands are added to this
  // command list such that the barrier only blocks the Memory Copy Regions enqueued.
  //
  // Until L0 Spec issue https://gitlab.devtools.intel.com/one-api/level_zero/issues/300
  // is resolved 3D buffer copies must be split into multiple 2D buffer copies in the
  // sycl plugin.
  //
  const ze_copy_region_t srcRegion = {srcOriginX, srcOriginY, 0, width, height, 0};
  const ze_copy_region_t dstRegion = {dstOriginX, dstOriginY, 0, width, height, 0};

  // TODO: Remove the for loop and use the slice pitches.
  for (uint32_t i = 0; i < depth; i++) {
    ze_result = ZE_CALL(zeCommandListAppendMemoryCopyRegion(
      ze_command_list,
      destination_ptr,
      &dstRegion,
      dstPitch,
      0, /* dstSlicePitch */
      source_ptr,
      &srcRegion,
      srcPitch,
      0, /* srcSlicePitch */
      nullptr
    ));

    destination_ptr += buffer_slice_pitch;
    source_ptr += host_slice_pitch;
  }
  zePrint("calling zeCommandListAppendMemoryCopyRegion()\n");

  ze_result = ZE_CALL(zeCommandListAppendBarrier(
    ze_command_list,
    ze_event,
    0,
    nullptr
  ));
  pi_assert(ze_result == 0);

  zePrint("calling zeCommandListAppendBarrier() with event %lx\n",
    pi_cast<std::uintptr_t>(ze_event));

  command_queue->executeCommandList(ze_command_list, blocking_write);
  _pi_event::deleteL0EventList(ze_event_wait_list);

  // TODO: translate errors
  return pi_cast<pi_result>(ze_result);
}

pi_result L0(piEnqueueMemBufferCopy)(
  pi_queue            command_queue,
  pi_mem              src_buffer,
  pi_mem              dst_buffer,
  size_t              src_offset,
  size_t              dst_offset,
  size_t              size,
  pi_uint32           num_events_in_wait_list,
  const pi_event *    event_wait_list,
  pi_event *          event) {

  return enqueueMemCopyHelper(
    PI_COMMAND_TYPE_MEM_BUFFER_COPY,
    command_queue,
    dst_buffer->L0Mem + dst_offset,
    false, // blocking
    size,
    src_buffer->L0Mem + src_offset,
    num_events_in_wait_list,
    event_wait_list,
    event);
}

pi_result L0(piEnqueueMemBufferCopyRect)(
  pi_queue            command_queue,
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

  pi_throw("piEnqueueMemBufferCopyRect: not implemented");
}

static pi_result enqueueMemFillHelper(
  pi_command_type    command_type,
  pi_queue           command_queue,
  void *             ptr,
  const void *       pattern,
  size_t             pattern_size,
  size_t             size,
  pi_uint32          num_events_in_wait_list,
  const pi_event *   event_wait_list,
  pi_event *         event) {

  ze_result_t               ze_result;
  // Get a new command list to be used on this call
  ze_command_list_handle_t ze_command_list =
    command_queue->Context->Device->createCommandList();

  L0(piEventCreate)(command_queue->Context, event);
  (*event)->Queue = command_queue;
  (*event)->CommandType = command_type;
  (*event)->L0CommandList = ze_command_list;

  ze_event_handle_t ze_event = (*event)->L0Event;

  ze_event_handle_t *ze_event_wait_list =
    _pi_event::createL0EventList(num_events_in_wait_list, event_wait_list);

  ze_result = ZE_CALL(zeCommandListAppendWaitOnEvents(
    ze_command_list,
    num_events_in_wait_list,
    ze_event_wait_list
  ));
  pi_assert(ze_result == 0);

  // pattern size must be a power of two
  assert((pattern_size > 0) && ((pattern_size & (pattern_size - 1)) == 0));

  ze_result = ZE_CALL(zeCommandListAppendMemoryFill(
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
  command_queue->executeCommandList(ze_command_list);
  _pi_event::deleteL0EventList(ze_event_wait_list);

  // TODO: translate errors
  return pi_cast<pi_result>(ze_result);
}

pi_result L0(piEnqueueMemBufferFill)(
  pi_queue           command_queue,
  pi_mem             buffer,
  const void *       pattern,
  size_t             pattern_size,
  size_t             offset,
  size_t             size,
  pi_uint32          num_events_in_wait_list,
  const pi_event *   event_wait_list,
  pi_event *         event) {

  return enqueueMemFillHelper(
    PI_COMMAND_TYPE_MEM_BUFFER_FILL,
    command_queue,
    buffer->L0Mem + offset,
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

  ze_result_t ze_result;

 // TODO: implement read-only, write-only
  pi_assert((map_flags & CL_MAP_READ) != 0);
  pi_assert((map_flags & CL_MAP_WRITE) != 0);

  // Get a new command list to be used on this call
  ze_command_list_handle_t ze_command_list =
    queue->Context->Device->createCommandList();

  L0(piEventCreate)(queue->Context, event);
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
    ze_host_mem_alloc_desc_t ze_desc;
    ze_desc.flags = ZE_HOST_MEM_ALLOC_FLAG_DEFAULT;
    ZE_CALL(zeDriverAllocHostMem(
      ze_driver_global,
      &ze_desc,
      size,
      1, // TODO: alignment
      ret_map));
  }

  ze_event_handle_t ze_event = (*event)->L0Event;
  ze_result = ZE_CALL(zeCommandListAppendMemoryCopy(
    ze_command_list,
    *ret_map,
    buffer->L0Mem + offset,
    size,
    ze_event
  ));

  queue->executeCommandList(ze_command_list, blocking_map);
  _pi_event::deleteL0EventList(ze_event_wait_list);

  // Record the created mapping to facilitate its later unmap.
  // TODO: ensure this is thread-safe.
  auto it = buffer->Mappings.find(*ret_map);
  if (it != buffer->Mappings.end()) {
    pi_throw("piEnqueueMemBufferMap: duplicate mapping detected");
  }
  else {
    buffer->Mappings.insert({*ret_map, {offset, size}});
  }

  // TODO: translate errors
  return pi_cast<pi_result>(ze_result);
}

pi_result L0(piEnqueueMemUnmap)(
  pi_queue         queue,
  pi_mem           memobj,
  void *           mapped_ptr,
  pi_uint32        num_events_in_wait_list,
  const pi_event * event_wait_list,
  pi_event *       event) {

  ze_result_t ze_result;
  // Get a new command list to be used on this call
  ze_command_list_handle_t ze_command_list =
    queue->Context->Device->createCommandList();

  // TODO: handle the case when user does not care to follow the event
  // of unmap completion.
  //
  pi_assert(event);

  L0(piEventCreate)(queue->Context, event);
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
  auto it = memobj->Mappings.find(mapped_ptr);
  if (it == memobj->Mappings.end()) {
    pi_throw("piEnqueueMemUnmap: unknown memory mapping");
  }
  auto map_info = it->second;
  memobj->Mappings.erase(it);

  ze_event_handle_t ze_event = (*event)->L0Event;
  ze_result = ZE_CALL(zeCommandListAppendMemoryCopy(
    ze_command_list,
    memobj->L0Mem + map_info.offset,
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
  queue->executeCommandList(ze_command_list);
  _pi_event::deleteL0EventList(ze_event_wait_list);

  // TODO: translate errors
  return pi_cast<pi_result>(ze_result);
}

pi_result L0(piMemImageGetInfo) (
  pi_mem          image,
  pi_image_info   param_name,
  size_t          param_value_size,
  void *          param_value ,
  size_t *        param_value_size_ret) {

  pi_throw("piMemImageGetInfo: not implemented");
}

static ze_image_region_t getImageRegionHelper(
  pi_mem            image,
  const size_t *    origin,
  const size_t *    region) {

  assert(image && origin);
#ifndef NDEBUG
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
  pi_queue          command_queue,
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

  ze_result_t ze_result;
  // Get a new command list to be used on this call
  ze_command_list_handle_t ze_command_list =
    command_queue->Context->Device->createCommandList();

  L0(piEventCreate)(command_queue->Context, event);
  (*event)->Queue = command_queue;
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
    pi_mem src_image = pi_cast<pi_mem>(const_cast<void*>(src));
    const ze_image_region_t srcRegion =
      getImageRegionHelper(src_image, src_origin, region);

    // TODO: L0 does not support row_pitch/slice_pitch for images yet.
    // https://gitlab.devtools.intel.com/one-api/level_zero/issues/303
    // Check that SYCL RT did not want pitch larger than default.
    //
#ifndef NDEBUG
    assert(row_pitch == 0 ||
           // special case RGBA image pitch equal to region's width
           (src_image->L0ImageDesc.format.layout ==
                ZE_IMAGE_FORMAT_LAYOUT_32_32_32_32 &&
            row_pitch == 4 * 4 * srcRegion.width) ||
           (src_image->L0ImageDesc.format.layout ==
                ZE_IMAGE_FORMAT_LAYOUT_16_16_16_16 &&
            row_pitch == 4 * 2 * srcRegion.width) ||
           (src_image->L0ImageDesc.format.layout ==
                ZE_IMAGE_FORMAT_LAYOUT_8_8_8_8 &&
            row_pitch == 4 * srcRegion.width));
    assert(slice_pitch == 0 ||
           slice_pitch == row_pitch * srcRegion.height);
#endif // !NDEBUG

    ze_result = ZE_CALL(zeCommandListAppendImageCopyToMemory(
      ze_command_list,
      dst,
      src_image->L0Image,
      &srcRegion,
      ze_event
    ));
  }
  else if (command_type == PI_COMMAND_TYPE_IMAGE_WRITE) {
    pi_mem dst_image = pi_cast<pi_mem>(dst);
    const ze_image_region_t dstRegion =
      getImageRegionHelper(dst_image, dst_origin, region);

    // TODO: L0 does not support row_pitch/slice_pitch for images yet.
    // https://gitlab.devtools.intel.com/one-api/level_zero/issues/303
    // Check that SYCL RT did not want pitch larger than default.
    //
#ifndef NDEBUG
    assert(row_pitch == 0 ||
           // special case RGBA image pitch equal to region's width
           (dst_image->L0ImageDesc.format.layout ==
                ZE_IMAGE_FORMAT_LAYOUT_32_32_32_32 &&
            row_pitch == 4 * 4 * dstRegion.width) ||
           (dst_image->L0ImageDesc.format.layout ==
                ZE_IMAGE_FORMAT_LAYOUT_16_16_16_16 &&
            row_pitch == 4 * 2 * dstRegion.width) ||
           (dst_image->L0ImageDesc.format.layout ==
                ZE_IMAGE_FORMAT_LAYOUT_8_8_8_8 &&
            row_pitch == 4 * dstRegion.width));
    assert(slice_pitch == 0 ||
           slice_pitch == row_pitch * dstRegion.height);
#endif // !NDEBUG

    ze_result = ZE_CALL(zeCommandListAppendImageCopyFromMemory(
      ze_command_list,
      dst_image->L0Image,
      src,
      &dstRegion,
      ze_event
    ));
  }
  else if (command_type == PI_COMMAND_TYPE_IMAGE_COPY) {
    pi_mem src_image = pi_cast<pi_mem>(const_cast<void*>(src));
    pi_mem dst_image = pi_cast<pi_mem>(dst);

    const ze_image_region_t srcRegion =
      getImageRegionHelper(src_image, src_origin, region);
    const ze_image_region_t dstRegion =
      getImageRegionHelper(dst_image, dst_origin, region);

    ze_result = ZE_CALL(zeCommandListAppendImageCopyRegion(
      ze_command_list,
      dst_image->L0Image,
      src_image->L0Image,
      &dstRegion,
      &srcRegion,
      ze_event
    ));
  }
  else {
    pi_throw("enqueueMemImageUpdate: unsupported image command type");

  }

  command_queue->executeCommandList(ze_command_list, is_blocking);
  _pi_event::deleteL0EventList(ze_event_wait_list);

  // TODO: translate errors
  return pi_cast<pi_result>(ze_result);
}

pi_result L0(piEnqueueMemImageRead)(
  pi_queue          command_queue,
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
    command_queue,
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
  pi_queue          command_queue,
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
    command_queue,
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
  pi_queue          command_queue,
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
    command_queue,
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
  pi_queue          command_queue,
  pi_mem            image,
  const void *      fill_color,
  const size_t *    origin,
  const size_t *    region,
  pi_uint32         num_events_in_wait_list,
  const pi_event *  event_wait_list,
  pi_event *        event) {

  pi_throw("piEnqueueMemImageFill: not implemented");
}

pi_result L0(piMemBufferPartition)(
    pi_mem                    buffer,
    pi_mem_flags              flags,
    pi_buffer_create_type     buffer_create_type,
    void *                    buffer_create_info,
    pi_mem *                  ret_mem) {

  pi_throw("piMemBufferPartition: not implemented");
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

  pi_throw("piEnqueueNativeKernel: not implemented");
}

// TODO: Check if the function_pointer_ret type can be converted to void**.
pi_result L0(piextGetDeviceFunctionPointer)(
  pi_device        device,
  pi_program       program,
  const char *     function_name,
  pi_uint64 *      function_pointer_ret) {
  pi_assert(program != nullptr);
  // TODO: Handle Errors.
  ze_result_t ze_res = ZE_CALL(zeModuleGetFunctionPointer(
      program->L0Module, function_name,
      reinterpret_cast<void **>(function_pointer_ret)));
  return pi_cast<pi_result>(ze_res);
}

pi_result L0(piextUSMHostAlloc)(void **result_ptr, pi_context context,
                                pi_usm_mem_properties *properties, size_t size,
                                pi_uint32 alignment) {

  ze_host_mem_alloc_desc_t ze_desc;
  ze_desc.flags = ZE_HOST_MEM_ALLOC_FLAG_DEFAULT;
  // TODO: translate PI properties to L0 flags
  ze_result_t ze_result = ZE_CALL_NOTHROW(zeDriverAllocHostMem(
    ze_driver_global,
    &ze_desc,
    size,
    alignment,
    result_ptr));

  if (ze_result == ZE_RESULT_ERROR_UNSUPPORTED_SIZE) {
    // TODO: document the erros returned by piextUSMHostAlloc
    return PI_INVALID_VALUE;
  }
  else {
    // TODO: handle other errors.
    zeCallCheck(ze_result, "zeDriverAllocHostMem");
  }

  return PI_SUCCESS;
}

pi_result L0(piextUSMDeviceAlloc)(void **result_ptr, pi_context context,
                                  pi_device device,
                                  pi_usm_mem_properties *properties,
                                  size_t size, pi_uint32 alignment) {

  // TODO: translate PI properties to L0 flags
  ze_device_mem_alloc_desc_t ze_desc;
  ze_desc.flags = ZE_DEVICE_MEM_ALLOC_FLAG_DEFAULT;
  ze_desc.ordinal = 0;
  ze_result_t ze_result = ZE_CALL_NOTHROW(zeDriverAllocDeviceMem(
    ze_driver_global,
    &ze_desc,
    size,
    alignment,
    device->L0Device,
    result_ptr));

  if (ze_result == ZE_RESULT_ERROR_UNSUPPORTED_SIZE) {
    // TODO: document the erros returned by piextUSMDeviceAlloc
    return PI_INVALID_VALUE;
  }
  else {
    // TODO: handle other errors.
    zeCallCheck(ze_result, "zeDriverAllocDeviceMem");
  }

  return PI_SUCCESS;
}

pi_result L0(piextUSMSharedAlloc)(
  void **                 result_ptr,
  pi_context              context,
  pi_device               device,
  pi_usm_mem_properties * properties,
  size_t                  size,
  pi_uint32               alignment) {

  // TODO: translate PI properties to L0 flags
  ze_host_mem_alloc_desc_t ze_host_desc;
  ze_host_desc.flags = ZE_HOST_MEM_ALLOC_FLAG_DEFAULT;
  ze_device_mem_alloc_desc_t ze_dev_desc;
  ze_dev_desc.flags = ZE_DEVICE_MEM_ALLOC_FLAG_DEFAULT;
  ze_dev_desc.ordinal = 0;
  ze_result_t ze_result = ZE_CALL_NOTHROW(zeDriverAllocSharedMem(
    ze_driver_global,
    &ze_dev_desc,
    &ze_host_desc,
    size,
    alignment,
    device->L0Device,
    result_ptr));

  if (ze_result == ZE_RESULT_ERROR_UNSUPPORTED_SIZE) {
    // TODO: document the erros returned by piextUSMSharedAlloc
    return PI_INVALID_VALUE;
  }
  else {
    // TODO: handle other errors.
    zeCallCheck(ze_result, "zeDriverAllocSharedMem");
  }

  return PI_SUCCESS;
}

pi_result L0(piextUSMFree)(pi_context context, void *ptr)
{
  ZE_CALL(zeDriverFreeMem(ze_driver_global, ptr));
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
  // TODO: this may not be needed when we translate L0 errors to PI
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

  // TODO: this may not be needed when we translate L0 errors to PI
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
  // Get a new command list to be used on this call
  ze_command_list_handle_t ze_command_list =
    queue->Context->Device->createCommandList();

  // TODO: do we need to create a unique command type for this?
  L0(piEventCreate)(queue->Context, event);
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
  
  queue->executeCommandList(ze_command_list, false);
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
  // Get a new command list to be used on this call
  ze_command_list_handle_t ze_command_list =
    queue->Context->Device->createCommandList();

  // TODO: do we need to create a unique command type for this?
  L0(piEventCreate)(queue->Context, event);
  (*event)->Queue = queue;
  (*event)->CommandType = PI_COMMAND_TYPE_USER;
  (*event)->L0CommandList = ze_command_list;

  ZE_CALL(zeCommandListAppendMemAdvise(
    ze_command_list,
    queue->Context->Device->L0Device,
    ptr,
    length,
    // TODO: we need some translation to L0 advices
    // pi_cast<ze_memory_advice_t>(advice)
    ZE_MEMORY_ADVICE_BIAS_CACHED
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
  ze_device_handle_t ze_device_handle;
  ze_memory_allocation_properties_t ze_memory_allocation_properties = {
    ZE_MEMORY_ALLOCATION_PROPERTIES_VERSION_CURRENT
  };

  ZE_CALL(zeDriverGetMemAllocProperties(
    ze_driver_global,
    ptr,
    &ze_memory_allocation_properties,
    &ze_device_handle));

  if (param_name == PI_MEM_ALLOC_TYPE) {
    pi_usm_type mem_alloc_type;
    switch (ze_memory_allocation_properties.type) {
    case ZE_MEMORY_TYPE_UNKNOWN: mem_alloc_type = PI_MEM_TYPE_UNKNOWN; break;
    case ZE_MEMORY_TYPE_HOST:    mem_alloc_type = PI_MEM_TYPE_HOST; break;
    case ZE_MEMORY_TYPE_DEVICE:  mem_alloc_type = PI_MEM_TYPE_DEVICE; break;
    case ZE_MEMORY_TYPE_SHARED:  mem_alloc_type = PI_MEM_TYPE_SHARED; break;
    default:
      pi_throw("piextUSMGetMemAllocInfo: unexpected usm memory type");
    }
    SET_PARAM_VALUE(mem_alloc_type);
  }
  else if (param_name == PI_MEM_ALLOC_DEVICE) {
    // TODO: this wants pi_device, but we didn't remember it, and cannot
    // deduct from the L0 device.
    //
    pi_throw("piextUSMGetMemAllocInfo: PI_MEM_ALLOC_DEVICE unsupported");
  }
  else if (param_name == PI_MEM_ALLOC_BASE_PTR) {
    void * base;
    ZE_CALL(zeDriverGetMemAddressRange(ze_driver_global, ptr, &base, nullptr));
    SET_PARAM_VALUE(base);
  }
  else if (param_name == PI_MEM_ALLOC_SIZE) {
    size_t size;
    ZE_CALL(zeDriverGetMemAddressRange(ze_driver_global, ptr, nullptr, &size));
    SET_PARAM_VALUE(size);
  }
  else {
    pi_throw("piextUSMGetMemAllocInfo: unsupported param_name");
  }
  return PI_SUCCESS;
}

pi_result L0(piKernelSetExecInfo)(pi_kernel kernel,
                                  pi_kernel_exec_info param_name,
                                  size_t param_value_size,
                                  const void *param_value) {

  if (param_name == PI_USM_INDIRECT_ACCESS &&
      *(static_cast<const pi_bool *>(param_value)) == PI_TRUE) {
    // TODO: enable when this is resolved:
    // https://gitlab.devtools.intel.com/one-api/level_zero_gpu_driver/
    // issues/45
    //
#if 0
    // The whole point for users really was to not need to know anything
    // about the types of allocations kernel uses. So in DPC++ we always
    // just set all 3 modes for each kernel.
    //
    ZE_CALL(zeKernelSetAttribute(
      kernel->L0Kernel, ZE_KERNEL_SET_ATTR_INDIRECT_SHARED_ACCESS, 1));
    ZE_CALL(zeKernelSetAttribute(
      kernel->L0Kernel, ZE_KERNEL_SET_ATTR_INDIRECT_DEVICE_ACCESS, 1));
    ZE_CALL(zeKernelSetAttribute(
      kernel->L0Kernel, ZE_KERNEL_SET_ATTR_INDIRECT_HOST_ACCESS, 1));
#endif // 0
    return PI_SUCCESS;
  }

  pi_throw("piKernelSetExecInfo: param not supported");
}

pi_result L0(piPluginInit)(pi_plugin *PluginInit)
{
  // TODO: handle versioning/targets properly.
  size_t PluginVersionSize = sizeof(PluginInit->PluginVersion);
  assert(strlen(_PI_H_VERSION_STRING) < PluginVersionSize);
  strncpy(PluginInit->PluginVersion, _PI_H_VERSION_STRING, PluginVersionSize);

#define _PI_API(api)                                                \
  (PluginInit->PiFunctionTable).api = (decltype(&::api))(&L0(api));
#include <CL/sycl/detail/pi.def>

  // TODO: handle errors
  return PI_SUCCESS;
}

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

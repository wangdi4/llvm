#include "pi_level0.hpp"
#include <map>
#include <memory>
#include <thread>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <stdarg.h>

bool ZE_DEBUG = false;

static void zePrint(const char *format, ... ) {
  if (ZE_DEBUG) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
  }
}

static ze_result_t zeCallCheck(ze_result_t ze_result, const char *call_str, bool nothrow = false)
{
  zePrint("ZE ---> %s\n", call_str);

  // TODO: handle errors
  if (ze_result) {
    if (!nothrow) {
      fprintf(stderr, "Error (%d) in %s\n", pi_cast<uint32_t>(ze_result), call_str);
      pi_throw("L0 Error");
    }
  }
  return ze_result;
}

#define ZE_CALL(call)         zeCallCheck(call, #call, false)
#define ZE_CALL_NOTHROW(call) zeCallCheck(call, #call, true)

ze_command_list_handle_t _pi_queue::getCommandList()
{
  // Create the command list, because in L0 commands are added to
  // the command lists, and later are then added to the command queue.
  //
  // TODO: Fugire out how to lower the overhead of creating a new list
  // for each command, if that appears to be important.
  //
  ze_command_list_desc_t ze_command_list_desc;
  ze_command_list_desc.version = ZE_COMMAND_LIST_DESC_VERSION_CURRENT;

  // TODO: can we just reset the command-list created when an earlier
  // command was submitted to the queue?
  //
  ZE_CALL(zeCommandListCreate(
    pi_cast<ze_device_handle_t>(Context->Device),
    &ze_command_list_desc,
    &L0CommandList));

  return L0CommandList;
}

void _pi_queue::executeCommandList()
{
  // Send the command list to the queue.
  ZE_CALL(zeCommandListClose(L0CommandList));
  ZE_CALL(zeCommandQueueExecuteCommandLists(
    L0CommandQueue, 1, &L0CommandList, nullptr));
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
#define SET_PARAM_VALUE(value)          \
{                                       \
  typedef decltype(value) T;            \
  if (param_value)                      \
    *(T*)param_value = value;           \
  if (param_value_size_ret)             \
    *param_value_size_ret = sizeof(T);  \
}
#define SET_PARAM_VECTOR(value, size)   \
{                                       \
  if (param_value)                      \
    memcpy(param_value, value, size);   \
  if (param_value_size_ret)             \
    *param_value_size_ret = size;       \
}
#define SET_PARAM_VALUE_STR(value)                  \
{                                                   \
  if (param_value)                                  \
    memcpy(param_value, value, param_value_size);   \
  if (param_value_size_ret)                         \
    *param_value_size_ret = strlen(value) + 1;      \
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
    *platforms = 0;
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

  uint32_t ze_driver_version;
  pi_assert(ze_driver_global != nullptr);
  ZE_CALL(zeDriverGetDriverVersion(ze_driver_global, &ze_driver_version));
  uint32_t ze_driver_version_major = ZE_DRIVER_MAJOR_VERSION(ze_driver_version);
  uint32_t ze_driver_version_minor = ZE_DRIVER_MINOR_VERSION(ze_driver_version);
  uint32_t ze_driver_version_patch = ZE_DRIVER_PATCH_VERSION(ze_driver_version);

  char ze_driver_version_string[255];
  sprintf(ze_driver_version_string, "Level-Zero %d.%d.%d\n",
      ze_driver_version_major,
      ze_driver_version_minor,
      ze_driver_version_patch);
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
    pi_throw("Unsuppported param_name in piPlatformGetInfo");
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

  // L0 does not have platforms, expect fake 0 here
  pi_assert(platform == 0);

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
      devices[i] = pi_cast<pi_device>(ze_devices[i]);
    }
  }
  return PI_SUCCESS;
}

pi_result L0(piDeviceRetain)(pi_device     device) {
  return PI_SUCCESS;
}

pi_result L0(piDeviceRelease)(pi_device     device) {
  return PI_SUCCESS;
}

pi_result L0(piDeviceGetInfo)(pi_device       device,
                              pi_device_info  param_name,
                              size_t          param_value_size,
                              void *          param_value,
                              size_t *        param_value_size_ret) {

  // TODO: cache device properties instead of querying L0 each time
  ze_device_handle_t ze_device = pi_cast<ze_device_handle_t>(device);
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
    // TODO: is there a way to query L0 device for a device-group?
    SET_PARAM_VALUE(pi_platform{0});
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
    pi_uint32 max_compute_units = ze_device_properties.numEUsPerSubslice *
      ze_device_properties.numSubslicesPerSlice *
      ze_device_properties.numSlicesPerTile *
      ze_device_properties.numTiles;
    SET_PARAM_VALUE(pi_uint32{max_compute_units});
  }
  else if (param_name == PI_DEVICE_MAX_WORK_ITEM_DIMENSIONS) {
    // L0 spec defines only three dimensions
    SET_PARAM_VALUE(pi_uint32{3});
  }
  else if (param_name == PI_DEVICE_MAX_WORK_GROUP_SIZE) {
    SET_PARAM_VALUE(pi_uint32{ze_device_compute_properties.maxTotalGroupSize});
  }
  else if (param_name == PI_DEVICE_MAX_WORK_ITEM_SIZES) {
    // Make sure there is space to store the values in three dimensions
    pi_assert(!(param_value_size <
      (sizeof(ze_device_compute_properties.maxGroupSizeX) * 3)));
    // TODO: Is there a SET_PARAM_VALUE() for vectors?
     SET_PARAM_VALUE(pi_uint32{
       (ze_device_compute_properties.maxGroupSizeX << 16) |
       (ze_device_compute_properties.maxGroupSizeY << 8)  |
       ze_device_compute_properties.maxGroupSizeZ});
  }
  else if (param_name == PI_DEVICE_MAX_CLOCK_FREQUENCY) {
    SET_PARAM_VALUE(pi_uint32{ze_device_properties.coreClockRate});
  }
  else if (param_name == PI_DEVICE_ADDRESS_BITS) {
    // TODO: To confirm with spec.
    SET_PARAM_VALUE(pi_uint32{64});
  }
  else if (param_name == PI_DEVICE_MAX_MEM_ALLOC_SIZE) {
    // TODO: To confirm with spec.
    uint32_t max_mem_alloc_size = 0;
    for (uint32_t i = 0; i < ze_avail_mem_count; i++) {
      max_mem_alloc_size += ze_device_memory_properties[i].totalSize;
    }
    SET_PARAM_VALUE(pi_uint32{max_mem_alloc_size});
  }
  else if (param_name == PI_DEVICE_GLOBAL_MEM_SIZE) {
    // TODO: To confirm with spec.
    uint32_t max_mem_alloc_size = 0;
    for (uint32_t i = 0; i < ze_avail_mem_count; i++) {
      max_mem_alloc_size += ze_device_memory_properties[i].totalSize;
    }
    SET_PARAM_VALUE(pi_uint32{max_mem_alloc_size});
  }
  else if (param_name == PI_DEVICE_LOCAL_MEM_SIZE) {
  // TODO: To confirm with spec.
    uint32_t max_mem_alloc_size = 0;
    for (uint32_t i = 0; i < ze_avail_mem_count; i++) {
      max_mem_alloc_size += ze_device_memory_properties[i].totalSize;
    }
    SET_PARAM_VALUE(pi_uint32{max_mem_alloc_size});
  }
  else if (param_name == PI_DEVICE_IMAGE_SUPPORT) {
    SET_PARAM_VALUE(pi_bool{ze_device_image_properties.supported});
  }
  else if (param_name == PI_DEVICE_HOST_UNIFIED_MEMORY) {
    SET_PARAM_VALUE(pi_bool{ze_device_properties.unifiedMemorySupported});
  }
  else if (param_name == PI_DEVICE_AVAILABLE) {
    SET_PARAM_VALUE(pi_bool{ze_device ? true : false});
  }
  else if (param_name == PI_DEVICE_VENDOR) {
    // TODO: Level-Zero does not return vendor's name at the moment
    // only the ID.
    SET_PARAM_VALUE_STR("Intel");
  }
  else if (param_name == PI_DRIVER_VERSION) {
    // TODO: Is there a SET_PARAM_VALUE() for vectors?
    uint32_t ze_driver_version;
    pi_assert(ze_driver_global != nullptr);
    ZE_CALL(zeDriverGetDriverVersion(ze_driver_global, &ze_driver_version));
    uint32_t ze_driver_version_major = ZE_DRIVER_MAJOR_VERSION(ze_driver_version);
    uint32_t ze_driver_version_minor = ZE_DRIVER_MINOR_VERSION(ze_driver_version);
     SET_PARAM_VALUE(pi_uint32{(ze_driver_version_major << 8) | ze_driver_version_minor});
  }
  else if (param_name == PI_DEVICE_VERSION) {
    SET_PARAM_VALUE(pi_cast<pi_uint32>(ze_device_properties.version));
  }
  else if (param_name == PI_DEVICE_PARTITION_MAX_SUB_DEVICES) {
    SET_PARAM_VALUE(pi_uint32{ze_device_properties.numTiles});
  }
  else if (param_name == PI_DEVICE_REFERENCE_COUNT) {
    if (ze_device_properties.isSubdevice == 0) {
      SET_PARAM_VALUE(pi_uint32{1});
    }
    else {
      SET_PARAM_VALUE(pi_uint32{ze_device_properties.subdeviceId});
    }
  }

  // Everything under here is not supported yet

  else if (param_name == PI_DEVICE_PARTITION_AFFINITY_DOMAIN) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_PARTITION_AFFINITY_DOMAIN in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_PARTITION_TYPE) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_PARTITION_TYPE in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_OPENCL_C_VERSION) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_OPENCL_C_VERSION in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_PREFERRED_INTEROP_USER_SYNC) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_PREFERRED_INTEROP_USER_SYNC in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_PRINTF_BUFFER_SIZE) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_PRINTF_BUFFER_SIZE in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_PARTITION_PROPERTIES) {
    // TODO: Whatever partitioning we decide is supported by L0 should be
    // returned here (and supported in piDevicePartition), none for now.
    // See https://gitlab.devtools.intel.com/one-api/level_zero/issues/239.
    // See https://gitlab.devtools.intel.com/one-api/level_zero/issues/295.
    //
    SET_PARAM_VALUE(intptr_t{0});
  }
  else if (param_name == PI_DEVICE_PROFILE) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_PROFILE in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_BUILT_IN_KERNELS) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_BUILT_IN_KERNELS in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_QUEUE_PROPERTIES) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_QUEUE_PROPERTIES in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_EXECUTION_CAPABILITIES) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_EXECUTION_CAPABILITIES in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_ENDIAN_LITTLE) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_ENDIAN_LITTLE in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_ERROR_CORRECTION_SUPPORT) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_ERROR_CORRECTION_SUPPORT in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_PROFILING_TIMER_RESOLUTION) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_PROFILING_TIMER_RESOLUTION in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_LOCAL_MEM_TYPE) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_LOCAL_MEM_TYPE in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_MAX_CONSTANT_ARGS) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_MAX_CONSTANT_ARGS in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_MAX_CONSTANT_BUFFER_SIZE) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_MAX_CONSTANT_BUFFER_SIZE in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_GLOBAL_MEM_CACHE_TYPE) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_GLOBAL_MEM_CACHE_TYPE in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_GLOBAL_MEM_CACHELINE_SIZE) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_GLOBAL_MEM_CACHELINE_SIZE in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_GLOBAL_MEM_CACHE_SIZE) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_GLOBAL_MEM_CACHE_SIZE in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_MAX_PARAMETER_SIZE) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_MAX_PARAMETER_SIZE in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_MEM_BASE_ADDR_ALIGN) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_MEM_BASE_ADDR_ALIGN in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_MAX_SAMPLERS) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_MAX_SAMPLERS in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_MAX_READ_IMAGE_ARGS) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_MAX_READ_IMAGE_ARGS in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_MAX_WRITE_IMAGE_ARGS) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_MAX_WRITE_IMAGE_ARGS in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_SINGLE_FP_CONFIG) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_SINGLE_FP_CONFIG in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_HALF_FP_CONFIG) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_HALF_FP_CONFIG in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_DOUBLE_FP_CONFIG) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_DOUBLE_FP_CONFIG in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_IMAGE2D_MAX_WIDTH) {
    // TODO: https://gitlab.devtools.intel.com/one-api/level_zero/issues/288
    // Until L0 provides needed info, hardcode default minimum values required
    // by the SYCL specification.
    //
    SET_PARAM_VALUE(size_t{8192});
  }
  else if (param_name == PI_DEVICE_IMAGE2D_MAX_HEIGHT) {
    // TODO: https://gitlab.devtools.intel.com/one-api/level_zero/issues/288
    // Until L0 provides needed info, hardcode default minimum values required
    // by the SYCL specification.
    //
    SET_PARAM_VALUE(size_t{8192});
  }
  else if (param_name == PI_DEVICE_IMAGE3D_MAX_WIDTH) {
    // TODO: https://gitlab.devtools.intel.com/one-api/level_zero/issues/288
    // Until L0 provides needed info, hardcode default minimum values required
    // by the SYCL specification.
    //
    SET_PARAM_VALUE(size_t{2048});
  }
  else if (param_name == PI_DEVICE_IMAGE3D_MAX_HEIGHT) {
    // TODO: https://gitlab.devtools.intel.com/one-api/level_zero/issues/288
    // Until L0 provides needed info, hardcode default minimum values required
    // by the SYCL specification.
    //
    SET_PARAM_VALUE(size_t{2048});
  }
  else if (param_name == PI_DEVICE_IMAGE3D_MAX_DEPTH) {
    // TODO: https://gitlab.devtools.intel.com/one-api/level_zero/issues/288
    // Until L0 provides needed info, hardcode default minimum values required
    // by the SYCL specification.
    //
    SET_PARAM_VALUE(size_t{2048});
  }
  else if (param_name == PI_DEVICE_IMAGE_MAX_BUFFER_SIZE) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_IMAGE_MAX_BUFFER_SIZE in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_IMAGE_MAX_ARRAY_SIZE) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_IMAGE_MAX_ARRAY_SIZE in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_PREFERRED_VECTOR_WIDTH_INT) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_PREFERRED_VECTOR_WIDTH_INT in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_PREFERRED_VECTOR_WIDTH_LONG) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_PREFERRED_VECTOR_WIDTH_LONG in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_PREFERRED_VECTOR_WIDTH_HALF) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_PREFERRED_VECTOR_WIDTH_HALF in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_NATIVE_VECTOR_WIDTH_CHAR) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_NATIVE_VECTOR_WIDTH_CHAR in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_NATIVE_VECTOR_WIDTH_SHORT) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_NATIVE_VECTOR_WIDTH_SHORT in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_NATIVE_VECTOR_WIDTH_INT) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_NATIVE_VECTOR_WIDTH_INT in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_NATIVE_VECTOR_WIDTH_LONG) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_NATIVE_VECTOR_WIDTH_LONG in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE in piGetDeviceInfo");
  }
  else if (param_name == PI_DEVICE_NATIVE_VECTOR_WIDTH_HALF) {
    // TODO: To find out correct value
    pi_throw("Unsupported PI_DEVICE_NATIVE_VECTOR_WIDTH_HALF in piGetDeviceInfo");
  }
  else {
    fprintf(stderr, "param_name=%d(%lx)\n", param_name, (unsigned long int)param_name);
    pi_throw("Unsupported param_name in piGetDeviceInfo");
  }

  return PI_SUCCESS;
}

pi_result L0(piDevicePartition)(
  pi_device     device,
  const cl_device_partition_property * properties,
  pi_uint32     num_devices,
  pi_device *   out_devices,
  pi_uint32 *   out_num_devices) {

  pi_throw("piDevicePartition not implemented");
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
    pi_throw("l0_piCreateContext: context should have exactly one device");
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
    pi_throw("l0_piGetContextInfo: unsuppported param_name.");
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

// TODO: make this thread-safe
static std::map<ze_command_queue_handle_t,
                ze_command_list_handle_t> ze_queue2list_map;

pi_result L0(piQueueCreate)(
  pi_context                    context,
  pi_device                     device,
  pi_queue_properties           properties,
  pi_queue *                    queue) {

  ze_result_t ze_result;
  ze_device_handle_t        ze_device;
  ze_command_queue_handle_t ze_command_queue;

  pi_assert(device == context->Device);
  ze_device = pi_cast<ze_device_handle_t>(device);

  ze_command_queue_desc_t ze_command_queue_desc =
    {ZE_COMMAND_QUEUE_DESC_VERSION_CURRENT};
  ze_command_queue_desc.ordinal = 0;
  ze_command_queue_desc.mode = ZE_COMMAND_QUEUE_MODE_ASYNCHRONOUS;

  ze_result = ZE_CALL(zeCommandQueueCreate(
    ze_device,
    &ze_command_queue_desc,  // TODO: translate properties
    &ze_command_queue));

  // Create the default command list for this command queue
  ze_command_list_handle_t ze_command_list = 0;
  ze_command_list_desc_t ze_command_list_desc;
  ze_command_list_desc.version = ZE_COMMAND_LIST_DESC_VERSION_CURRENT;

  ze_result = ZE_CALL(zeCommandListCreate(
    ze_device,
    &ze_command_list_desc,
    &ze_command_list));

  ze_queue2list_map[ze_command_queue] = ze_command_list;

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
    fprintf(stderr, "%d param_name=%d(%lx)\n",
        __LINE__, param_name, (unsigned long int)param_name);
    pi_throw("Unsupported param_name in piQueueGetInfo");
  }

  return PI_SUCCESS;
}


pi_result L0(piQueueRetain)(pi_queue command_queue) {
  return PI_SUCCESS;
}

pi_result L0(piQueueRelease)(pi_queue command_queue) {
  return PI_SUCCESS;
}

pi_result L0(piQueueFinish)(pi_queue command_queue) {
  pi_throw("piQueueFinish: not implemented");
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
  ze_device_handle_t ze_device = pi_cast<ze_device_handle_t>(context->Device);

  // TODO: translate errors
  ZE_CALL(zeDriverAllocDeviceMem(
    ze_driver_global,
    ze_device,
    ZE_DEVICE_MEM_ALLOC_FLAG_DEFAULT,
    0, // ordinal
    size,
    1, // TODO: alignment
    &ptr));

  *ret_mem = pi_cast<pi_mem>(ptr);

  if ((flags & PI_MEM_FLAGS_HOST_PTR_USE)  != 0 ||
      (flags & PI_MEM_FLAGS_HOST_PTR_COPY) != 0) {

    // TODO: how to PI_MEM_PROP_HOST_PTR_USE without the copy ?
    memcpy(ptr, host_ptr, size);
    return PI_SUCCESS;
  }
  else if (flags == 0 ||
           (flags == PI_MEM_FLAGS_ACCESS_RW)) {
    return PI_SUCCESS;
  }
  else {
    pi_throw("piMemBufferCreate: not implemented");
  }
}

pi_result L0(piMemGetInfo)(
  pi_mem           mem,
  cl_mem_info      param_name, // TODO: untie from OpenCL
  size_t           param_value_size,
  void *           param_value,
  size_t *         param_value_size_ret) {
  pi_throw("piMemGetInfo: not implemented");
}


pi_result L0(piMemRetain)(pi_mem) {
  pi_throw("piMemRetain: not implemented");
}

pi_result L0(piMemRelease)(pi_mem ptr) {

  // TODO: handle errors
  ZE_CALL(zeDriverFreeMem(
    ze_driver_global,
    pi_cast<void*>(ptr)));

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
  ZE_CALL(zeImageCreate(
    pi_cast<ze_device_handle_t>(context->Device),
    &imageDesc, &hImage));

  if ((flags & PI_MEM_FLAGS_HOST_PTR_USE)  != 0 ||
      (flags & PI_MEM_FLAGS_HOST_PTR_COPY) != 0) {

    // TODO: how to PI_MEM_PROP_HOST_PTR_USE without the copy?
    // TODO: how to initialize image once per context/device and not
    // once per command list?
    // https://gitlab.devtools.intel.com/one-api/level_zero/issues/289
    pi_throw("piMemImageCreate: unsupported init from host ptr");
  }

  *ret_image = pi_cast<pi_mem>(hImage);
  return PI_SUCCESS;
}

pi_result L0(piProgramCreate)(
  pi_context    context,
  const void *  il,
  size_t        length,
  pi_program *  program) {

  ze_device_handle_t ze_device =
    pi_cast<ze_device_handle_t>(context->Device);

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

  pi_throw("piclProgramCreateWithBinary: not implemented");
}

pi_result L0(piclProgramCreateWithSource)(
  pi_context        context,
  pi_uint32         count,
  const char **     strings,
  const size_t *    lengths,
  pi_program *      ret_program) {

  pi_throw("piclProgramCreateWithSource: not implemented");
}

pi_result L0(piProgramGetInfo)(
  pi_program          program,
  cl_program_info     param_name,
  size_t              param_value_size,
  void *              param_value,
  size_t *            param_value_size_ret) {

  pi_throw("piProgramGetInfo: not implemented");
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

  pi_throw("piProgramLink: not implemented");
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

  pi_throw("piProgramCompile: not implemented");
}

pi_result L0(piProgramBuild)(
  pi_program           program,
  pi_uint32            num_devices,
  const pi_device *    device_list,
  const char *         options,
  void (*  pfn_notify)(pi_program program, void * user_data),
  void *               user_data) {

  // L0 builds the program at the time of piCreateProgram.
  // TODO: check parameters?
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
  return PI_SUCCESS;
}

pi_result L0(piProgramRelease)(pi_program program) {
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
  else if (param_name == PI_KERNEL_FUNCTION_NAME) {
    SET_PARAM_VALUE_STR(ze_kernel_properties.name);
  }
  else if (param_name == PI_KERNEL_NUM_ARGS) {
    SET_PARAM_VALUE(pi_uint32{ze_kernel_properties.numKernelArgs});
  }
  else if (param_name == PI_KERNEL_REFERENCE_COUNT) {
    SET_PARAM_VALUE(pi_uint32{kernel->RefCount});
  }
  else if (param_name == PI_KERNEL_ATTRIBUTES) {
    pi_throw("Unsupported PI_KERNEL_ATTRIBUTES in piKernelGetInfo\n");
  }
  else {
    fprintf(stderr, "%d param_name=%d(%lx)\n",
        __LINE__, param_name, (unsigned long int)param_name);
    pi_throw("Unsupported param_name in piGetDeviceInfo");
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
  ze_device_handle_t ze_device = pi_cast<ze_device_handle_t>(device);
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
    fprintf(stderr, "%d param_name=%d(%lx)\n", __LINE__, param_name, (unsigned long int)param_name);
    uint32_t *global_work_size = pi_cast<uint32_t *>(param_value);
    global_work_size[0] = ze_device_compute_properties.maxGroupSizeX;
    global_work_size[1] = ze_device_compute_properties.maxGroupSizeX;
    global_work_size[2] = ze_device_compute_properties.maxGroupSizeX;
    SET_PARAM_VECTOR(global_work_size, sizeof(uint32_t) * 3);
  }
  if (param_name == PI_KERNEL_WORK_GROUP_SIZE) {
    uint32_t X, Y, Z;
    ZE_CALL(zeKernelSuggestGroupSize(
      kernel->L0Kernel,
      10000, 10000, 10000, &X, &Y, &Z));

    //fprintf(stderr, "(%d %d %d)\n", X, Y, Z);
    SET_PARAM_VALUE(size_t{X * Y * Z});
  }
  else if (param_name == PI_KERNEL_COMPILE_WORK_GROUP_SIZE) {
    uint32_t *work_group_size = pi_cast<uint32_t *>(param_value);
    work_group_size[0] = ze_kernel_properties.compileGroupSize.groupCountX;
    work_group_size[1] = ze_kernel_properties.compileGroupSize.groupCountY;
    work_group_size[2] = ze_kernel_properties.compileGroupSize.groupCountZ;
    SET_PARAM_VECTOR(work_group_size, sizeof(uint32_t) * 3);
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
    fprintf(stderr, "%d param_name=%d(%lx)\n",
        __LINE__, param_name, (unsigned long int)param_name);
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
  ze_thread_group_dimensions_t thread_group_dimensions {1, 1, 1};
  uint32_t wg[3] = {1, 1, 1};

  // TODO: is using {1,1,1} OK when local_work_size is NULL?
  // TODO: assert if sizes do not fit into 32-bit?
  switch (work_dim) {
  case 3:
    wg[2] = local_work_size ? pi_cast<uint32_t>(local_work_size[2]) : 1;
    thread_group_dimensions.groupCountZ = pi_cast<uint32_t>(global_work_size[2] / wg[2]);
    // fallthru
  case 2:
    wg[1] = local_work_size ? pi_cast<uint32_t>(local_work_size[1]) : 1;
    thread_group_dimensions.groupCountY = pi_cast<uint32_t>(global_work_size[1] / wg[1]);
    // fallthru
  case 1:
    wg[0] = local_work_size ? pi_cast<uint32_t>(local_work_size[0]) : 1;
    thread_group_dimensions.groupCountX = pi_cast<uint32_t>(global_work_size[0] / wg[0]);
    break;

  default:
    pi_throw("piEnqueueKernelLaunch: unsupported work_dim");
  }

  ZE_CALL(zeKernelSetGroupSize(kernel->L0Kernel, wg[0], wg[1], wg[2]));

  L0(piEventCreate)(kernel->Program->Context, event);
  (*event)->Queue = queue;
  (*event)->CommandType = PI_COMMAND_TYPE_NDRANGE_KERNEL;

  ze_event_handle_t ze_event = (*event)->L0Event;

  ze_event_handle_t *ze_event_wait_list =
    _pi_event::createL0EventList(num_events_in_wait_list, event_wait_list);

  // Add the command to the command list
  ze_result_t result = ZE_CALL(zeCommandListAppendLaunchKernel(
    queue->getCommandList(),
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

  queue->executeCommandList();
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
  ze_device_handle_t ze_device = pi_cast<ze_device_handle_t>(context->Device);
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
  PiEvent->Queue = NULL;
  PiEvent->CommandType = PI_COMMAND_TYPE_USER;   // TODO: verify
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
    fprintf(stderr, "%d param_name=%d(%lx)\n",
        __LINE__, param_name, (unsigned long int)param_name);
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
    ZE_CALL(zeEventDestroy(event->L0Event));
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

ze_device_handle_t ze_device = pi_cast<ze_device_handle_t>(context->Device);

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
            assert(sizeof(pi_bool) == sizeof(pi_sampler_properties));
            pi_bool cur_value_bool = pi_cast<pi_bool>(*(++cur_property));

            if (cur_value_bool == PI_TRUE)
              ze_sampler_desc.isNormalized = PI_TRUE;
            if (cur_value_bool == PI_FALSE)
              ze_sampler_desc.isNormalized = PI_FALSE;
            else
              pi_throw("piSamplerCreate: unsupported PI_SAMPLER_NORMALIZED_COORDS value");
          }
          break;

        case PI_SAMPLER_PROPERTIES_ADDRESSING_MODE:
          {
            assert(sizeof(pi_sampler_addressing_mode) == sizeof(pi_sampler_properties));
            pi_sampler_addressing_mode cur_value_addressing_mode = pi_cast<pi_sampler_addressing_mode>(*(++cur_property));

            // TODO: add support for PI_SAMPLER_ADDRESSING_MODE_CLAMP_TO_EDGE
            if (cur_value_addressing_mode == PI_SAMPLER_ADDRESSING_MODE_NONE)
              ze_sampler_desc.addressMode = ZE_SAMPLER_ADDRESS_MODE_NONE;
            if (cur_value_addressing_mode == PI_SAMPLER_ADDRESSING_MODE_REPEAT)
              ze_sampler_desc.addressMode = ZE_SAMPLER_ADDRESS_MODE_REPEAT;
            if (cur_value_addressing_mode == PI_SAMPLER_ADDRESSING_MODE_CLAMP)
              ze_sampler_desc.addressMode = ZE_SAMPLER_ADDRESS_MODE_CLAMP;
            if (cur_value_addressing_mode == PI_SAMPLER_ADDRESSING_MODE_MIRRORED_REPEAT)
              ze_sampler_desc.addressMode = ZE_SAMPLER_ADDRESS_MODE_MIRROR;
            else
              pi_throw("piSamplerCreate: unsupported PI_SAMPLER_ADDRESSING_MODE value");
          }
          break;

        case PI_SAMPLER_PROPERTIES_FILTER_MODE:
          {
            pi_assert(sizeof(pi_sampler_filter_mode) == sizeof(pi_sampler_properties));
            pi_sampler_filter_mode cur_value_filter_mode = pi_cast<pi_sampler_filter_mode>(*(++cur_property));

            if (cur_value_filter_mode == PI_SAMPLER_FILTER_MODE_NEAREST)
              ze_sampler_desc.filterMode = ZE_SAMPLER_FILTER_MODE_NEAREST;
            if (cur_value_filter_mode == PI_SAMPLER_FILTER_MODE_LINEAR)
              ze_sampler_desc.filterMode = ZE_SAMPLER_FILTER_MODE_LINEAR;
            else
              pi_throw("piSamplerCreate: unsupported PI_SAMPLER_FILTER_MODE value");
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

  *ret_sampler = pi_cast<pi_sampler>(ze_sampler);
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

  pi_throw("piSamplerRetain: not implemented");
}

pi_result L0(piSamplerRelease)(pi_sampler sampler) {

  pi_throw("piSamplerRelease: not implemented");
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
  ze_result_t               ze_result;

  L0(piEventCreate)(queue->Context, event);
  (*event)->Queue = queue;
  (*event)->CommandType = PI_COMMAND_TYPE_MEM_BUFFER_READ;

  ze_event_handle_t ze_event = (*event)->L0Event;

  ze_event_handle_t *ze_event_wait_list =
    _pi_event::createL0EventList(num_events_in_wait_list, event_wait_list);

  ze_result = ZE_CALL(zeCommandListAppendWaitOnEvents(
    queue->getCommandList(),
    num_events_in_wait_list,
    ze_event_wait_list
  ));
  pi_assert(ze_result == 0);

  ze_result = ZE_CALL(zeCommandListAppendMemoryCopy(
    queue->getCommandList(),
    dst,
    pi_cast<void*>(pi_cast<char*>(src) + offset),
    size,
    ze_event
  ));

  if (blocking_read) {
    ze_result = ZE_CALL(zeCommandListAppendWaitOnEvents(
      queue->getCommandList(),
      1,
      &ze_event
    ));
    pi_assert(ze_result == 0);
  }

  zePrint("calling zeCommandListAppendMemoryCopy() with\n"
                  "  xe_event %lx\n"
                  "  num_events_in_wait_list %d:",
          pi_cast<std::uintptr_t>(ze_event), num_events_in_wait_list);
  for (pi_uint32 i = 0; i < num_events_in_wait_list; i++) {
    zePrint(" %lx", pi_cast<std::uintptr_t>(ze_event_wait_list[i]));
  }
  zePrint("\n");

  queue->executeCommandList();
  _pi_event::deleteL0EventList(ze_event_wait_list);

  // TODO: translate errors
  return pi_cast<pi_result>(ze_result);
}

pi_result L0(piEnqueueMemBufferReadRect)(
  pi_queue            command_queue,
  pi_mem              buffer,
  pi_bool             blocking_read, // TODO: To implement
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

  L0(piEventCreate)(command_queue->Context, event);
  (*event)->Queue = command_queue;
  (*event)->CommandType = PI_COMMAND_TYPE_MEM_BUFFER_READ_RECT;

  ze_event_handle_t ze_event = (*event)->L0Event;

  ze_event_handle_t *ze_event_wait_list =
    _pi_event::createL0EventList(num_events_in_wait_list, event_wait_list);

  ze_result = ZE_CALL(zeCommandListAppendWaitOnEvents(
    command_queue->getCommandList(),
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
  // Z dim only for images
  if (host_offset[2] != 0)
    pi_throw("piEnqueueMemBufferReadRect: 3D buffers not implemented");

  uint32_t dstPitch = host_row_pitch;
  if (dstPitch == 0)
    dstPitch = pi_cast<uint32_t>(region[0]);


  uint32_t srcOriginX = pi_cast<uint32_t>(buffer_offset[0]);
  uint32_t srcOriginY = pi_cast<uint32_t>(buffer_offset[1]);
  if (buffer_offset[2] != 0)
    pi_throw("piEnqueueMemBufferReadRect: 3D buffers not implemented");
  // Z dim only for images
  uint32_t srcPitch = buffer_row_pitch;
  if (srcPitch == 0)
    srcPitch = pi_cast<uint32_t>(region[0]);

  uint32_t width = pi_cast<uint32_t>(region[0]);
  uint32_t height = pi_cast<uint32_t>(region[1]);

  const ze_copy_region_t dstRegion = {dstOriginX, dstOriginY, width, height};
  const ze_copy_region_t srcRegion = {srcOriginX, srcOriginY, width, height};

  ze_result = ZE_CALL(zeCommandListAppendMemoryCopyRegion(
    command_queue->getCommandList(),
    ptr,
    &dstRegion,
    dstPitch,
    pi_cast<void*>(buffer),
    &srcRegion,
    srcPitch,
    ze_event
  ));

  zePrint("calling zeCommandListAppendMemoryCopyRegion() with\n"
                  "  ze_event %lx\n", pi_cast<std::uintptr_t>(ze_event));

  command_queue->executeCommandList();
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

  ze_result_t               ze_result;

  L0(piEventCreate)(command_queue->Context, event);
  (*event)->Queue = command_queue;
  (*event)->CommandType = PI_COMMAND_TYPE_MEM_BUFFER_WRITE;

  ze_event_handle_t ze_event = (*event)->L0Event;

  ze_event_handle_t *ze_event_wait_list =
    _pi_event::createL0EventList(num_events_in_wait_list, event_wait_list);

  ze_result = ZE_CALL(zeCommandListAppendWaitOnEvents(
    command_queue->getCommandList(),
    num_events_in_wait_list,
    ze_event_wait_list
  ));
  pi_assert(ze_result == 0);

  ze_result = ZE_CALL(zeCommandListAppendMemoryCopy(
    command_queue->getCommandList(),
    pi_cast<void*>(pi_cast<char*>(buffer) + offset),
    ptr,
    size,
    ze_event
  ));

  if (blocking_write) {
    ze_result = ZE_CALL(zeCommandListAppendWaitOnEvents(
      command_queue->getCommandList(),
      1,
      &ze_event
    ));
    pi_assert(ze_result == 0);
  }

  zePrint("calling zeCommandListAppendMemoryCopy() with\n"
                  "  xe_event %lx\n"
                  "  num_events_in_wait_list %d:",
          pi_cast<std::uintptr_t>(ze_event), num_events_in_wait_list);
  for (pi_uint32 i = 0; i < num_events_in_wait_list; i++) {
    zePrint(" %lx", pi_cast<std::uintptr_t>(ze_event_wait_list[i]));
  }
  zePrint("\n");

  command_queue->executeCommandList();
  _pi_event::deleteL0EventList(ze_event_wait_list);

  // TODO: translate errors
  return pi_cast<pi_result>(ze_result);
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

  L0(piEventCreate)(command_queue->Context, event);
  (*event)->Queue = command_queue;
  (*event)->CommandType = PI_COMMAND_TYPE_MEM_BUFFER_WRITE_RECT;

  ze_event_handle_t ze_event = (*event)->L0Event;

  ze_event_handle_t *ze_event_wait_list =
    _pi_event::createL0EventList(num_events_in_wait_list, event_wait_list);

  ze_result = ZE_CALL(zeCommandListAppendWaitOnEvents(
    command_queue->getCommandList(),
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

  // Z dim only for images
  if (host_offset[2] != 0)
    pi_throw("piEnqueueMemBufferWriteRect: 3D buffers not implemented");

  uint32_t srcPitch = host_row_pitch;
  if (srcPitch == 0)
    srcPitch = pi_cast<uint32_t>(region[0]);

  uint32_t dstOriginX = pi_cast<uint32_t>(buffer_offset[0]);
  uint32_t dstOriginY = pi_cast<uint32_t>(buffer_offset[1]);

  // Z dim only for images
  if (buffer_offset[2] != 0)
    pi_throw("piEnqueueMemBufferWriteRect: 3D buffers not implemented");

  uint32_t dstPitch = buffer_row_pitch;
  if (dstPitch == 0)
    dstPitch = pi_cast<uint32_t>(region[0]);

  uint32_t width = pi_cast<uint32_t>(region[0]);
  uint32_t height = pi_cast<uint32_t>(region[1]);

  const ze_copy_region_t srcRegion = {srcOriginX, srcOriginY, width, height};
  const ze_copy_region_t dstRegion = {dstOriginX, dstOriginY, width, height};

  ze_result = ZE_CALL(zeCommandListAppendMemoryCopyRegion(
    command_queue->getCommandList(),
    pi_cast<void*>(buffer),
    &dstRegion,
    dstPitch,
    ptr,
    &srcRegion,
    srcPitch,
    ze_event
  ));

  if (blocking_write) {
    ze_result = ZE_CALL(zeCommandListAppendWaitOnEvents(
      command_queue->getCommandList(),
      1,
      &ze_event));
  }

  zePrint("calling zeCommandListAppendMemoryCopyRegion() with\n"
                  "  ze_event %lx\n", pi_cast<std::uintptr_t>(ze_event));

  command_queue->executeCommandList();
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

  ze_result_t               ze_result;

  L0(piEventCreate)(command_queue->Context, event);
  (*event)->Queue = command_queue;
  (*event)->CommandType = PI_COMMAND_TYPE_MEM_BUFFER_COPY;

  ze_event_handle_t ze_event = (*event)->L0Event;

  ze_event_handle_t *ze_event_wait_list =
    _pi_event::createL0EventList(num_events_in_wait_list, event_wait_list);

  ze_result = ZE_CALL(zeCommandListAppendWaitOnEvents(
    command_queue->getCommandList(),
    num_events_in_wait_list,
    ze_event_wait_list
  ));
  pi_assert(ze_result == 0);

  ze_result = ZE_CALL(zeCommandListAppendMemoryCopy(
    command_queue->getCommandList(),
    pi_cast<void*>(pi_cast<char*>(dst_buffer) + dst_offset),
    pi_cast<void*>(pi_cast<char*>(src_buffer) + src_offset),
    size,
    ze_event
  ));

  zePrint("calling zeCommandListAppendMemoryCopy() with\n"
                  "  xe_event %lx\n"
                  "  num_events_in_wait_list %d:",
          pi_cast<std::uintptr_t>(ze_event), num_events_in_wait_list);
  for (pi_uint32 i = 0; i < num_events_in_wait_list; i++) {
    zePrint(" %lx", pi_cast<std::uintptr_t>(ze_event_wait_list[i]));
  }
  zePrint("\n");

  command_queue->executeCommandList();
  _pi_event::deleteL0EventList(ze_event_wait_list);

  // TODO: translate errors
  return pi_cast<pi_result>(ze_result);
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

  pi_throw("piEnqueueMemBufferFill: not implemented");
}

pi_result L0(piEnqueueMemBufferMap)(
  pi_queue          command_queue,
  pi_mem            buffer,
  pi_bool           blocking_map,
  cl_map_flags      map_flags,  // TODO: untie from OpenCL
  size_t            offset,
  size_t            size,
  pi_uint32         num_events_in_wait_list,
  const pi_event *  event_wait_list,
  pi_event *        event,
  void **           ret_map) {

  pi_throw("piEnqueueMemBufferMap: not implemented");
}

pi_result L0(piEnqueueMemUnmap)(
  pi_queue         command_queue,
  pi_mem           memobj,
  void *           mapped_ptr,
  pi_uint32        num_events_in_wait_list,
  const pi_event * event_wait_list,
  pi_event *       event) {

  pi_throw("piEnqueueMemUnmap: not implemented");
}

pi_result L0(piMemImageGetInfo) (
  pi_mem          image,
  pi_image_info   param_name,
  size_t          param_value_size,
  void *          param_value ,
  size_t *        param_value_size_ret) {

  pi_throw("piMemImageGetInfo: not implemented");
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

  pi_throw("piEnqueueMemImageRead: not implemented");
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

  pi_throw("piEnqueueMemImageWrite: not implemented");
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

  pi_throw("piEnqueueMemImageCopy: not implemented");
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

pi_result L0(piextGetDeviceFunctionPointer)(
  pi_device        device,
  pi_program       program,
  const char *     function_name,
  pi_uint64 *      function_pointer_ret) {

  pi_throw("piextGetDeviceFunctionPointer: not implemented");
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

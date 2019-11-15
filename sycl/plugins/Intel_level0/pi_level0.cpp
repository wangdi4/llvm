#include "pi_level0.hpp"
#include <map>
#include <memory>

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
    fprintf(stderr, "Error (%d) in %s\n", pi_cast<uint32_t>(ze_result), call_str);
    if (!nothrow)
      pi_throw("L0 Error");
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

// Convinience macro makes source code search easier
#define L0(pi_api) pi_api##OclPtr

// Forward declararitons
decltype(piEventCreate) L0(piEventCreate);

// No generic lambdas in C++11, so use this convinence macro.
// NOTE: to be used in API returning "param_value".
  #define SET_PARAM_VALUE(value)        \
{                                       \
  typedef decltype(value) T;            \
  if (param_value)                      \
    *(T*)param_value = value;           \
  if (param_value_size_ret)             \
    *param_value_size_ret = sizeof(T);  \
}
#define SET_PARAM_VALUE_STR(value)                  \
{                                                   \
  if (param_value)                                  \
    memcpy(param_value, value, param_value_size);   \
  if (param_value_size_ret)                         \
    *param_value_size_ret = strlen(value) + 1;      \
}

pi_result L0(piPlatformsGet)(pi_uint32       num_entries,
                             pi_platform *   platforms,
                             pi_uint32 *     num_platforms) {

  static const char *getEnv = std::getenv("ZE_DEBUG");
  if (getEnv)
    ZE_DEBUG = true;

  zePrint("==========================\n");
  zePrint("SYCL over Level-Zero 0.3.0\n"); // TODO: get the version
  zePrint("==========================\n");

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
    SET_PARAM_VALUE_STR("Level-Zero 0.2.0");
  }
  else {
    // TODO: implement other parameters
    pi_throw("Unsuppported param_name in piPlatformGetInfo");
  }

  return PI_SUCCESS;
}

// TODO: Figure out how to pass these objects and eliminated these globals.
ze_driver_handle_t ze_driver_global = {0};

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

  if (param_name == PI_DEVICE_INFO_TYPE) {
    // TODO: how to query device type in L0 ? (assume GPU for now)
    SET_PARAM_VALUE(PI_DEVICE_TYPE_GPU);
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
    // TODO: what would be the correct ID?
    SET_PARAM_VALUE(pi_uint32{0});
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
    // TODO: can L0 give the name? zeDeviceGetProperties?
    SET_PARAM_VALUE_STR("");
  }
  else if (param_name == PI_DEVICE_INFO_COMPILER_AVAILABLE) {
    SET_PARAM_VALUE(pi_bool{1});
  }
  else if (param_name == PI_DEVICE_INFO_LINKER_AVAILABLE) {
    SET_PARAM_VALUE(pi_bool{1});
  }
  else if (param_name == PI_DEVICE_INFO_MAX_COMPUTE_UNITS) {
    // TODO: query from L0
    SET_PARAM_VALUE(pi_uint32{1});
  }
  else {
    // TODO: implement other parameters
    fprintf(stderr, "param_name=%d\n", param_name);
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

  // TODO: implement
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

// TODO: change the interface to return error code instead
pi_context L0(piContextCreate)(
  const cl_context_properties * properties,
  pi_uint32         num_devices,
  const pi_device * devices,
  void (*           pfn_notify)(
    const char * errinfo,
    const void * private_info,
    size_t       cb,
    void *       user_data),
  void *            user_data,
  pi_result *       errcode_ret) {

  // L0 does not have notion of contexts.
  // Return the device handle (only single device is allowed) as a context handle.
  //
  if (num_devices != 1) {
    pi_throw("l0_piCreateContext: context should have exactly one device");
  }
  if (errcode_ret) {
    *errcode_ret = PI_SUCCESS;
  }

  auto L0PiContext = new _pi_context();
  L0PiContext->Device = *devices;
  L0PiContext->RefCount = 1;

  return L0PiContext;
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
  pi_context              context,
  pi_device               device,
  const cl_queue_properties *    properties,
  pi_queue *              queue,
  pi_result *             result) {

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

  pi_throw("piGetQueueInfo: not implemented");
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

pi_mem L0(piMemBufferCreate)( // TODO: change interface to return error code
  pi_context      context,
  pi_mem_flags    flags,
  size_t          size,
  void *          host_ptr,
  pi_result *     errcode_ret) {

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

  if ((flags & PI_MEM_FLAGS_HOST_PTR_USE)  != 0 ||
      (flags & PI_MEM_FLAGS_HOST_PTR_COPY) != 0) {

    // TODO: how to PI_MEM_PROP_HOST_PTR_USE without the copy ?
    memcpy(ptr, host_ptr, size);

    if (errcode_ret) {
      *errcode_ret = PI_SUCCESS;
    }
    return pi_cast<pi_mem>(ptr);
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
    static_cast<void*>(ptr)));

  return PI_SUCCESS;
}

pi_result L0(piMemImageCreate)(pi_mem) {
  pi_throw("piMemImageCreate: not implemented");
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

pi_program L0(piclProgramCreateWithBinary)( // TODO: change to return pi_result
  pi_context                     context,
  pi_uint32                      num_devices,
  const pi_device *              device_list,
  const size_t *                 lengths,
  const unsigned char **         binaries,
  pi_int32 *                     binary_status,
  pi_result *                    errcode_ret) {

  pi_throw("piclProgramCreateWithBinary: not implemented");
}

pi_program L0(piclProgramCreateWithSource)( // TODO:  change to return pi_result
  pi_context        context,
  pi_uint32         count,
  const char **     strings,
  const size_t *    lengths,
  pi_result *       errcode) {

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

pi_program L0(piProgramLink)( // TODO: change interface to return error code
  pi_context          context,
  pi_uint32           num_devices,
  const pi_device *   device_list,
  const char *        options,
  pi_uint32           num_input_programs,
  const pi_program *  input_programs,
  void (*  pfn_notify)(pi_program program,
                       void * user_data),
  void *              user_data,
  pi_result *         errcode_ret) {

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

pi_kernel L0(piKernelCreate)( // TODO: change interface to return error code
  pi_program      program,
  const char *    kernel_name,
  pi_result *     errcode_ret) {

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

  if (errcode_ret)
    *errcode_ret = PI_SUCCESS;

  return pi_cast<pi_kernel>(L0PiKernel);
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
  if (param_name == PI_KERNEL_INFO_CONTEXT) {
    SET_PARAM_VALUE(pi_context{kernel->Program->Context});
  }
  else if (param_name == PI_KERNEL_INFO_PROGRAM) {
    SET_PARAM_VALUE(pi_program{kernel->Program});
  }
  else {
    pi_throw("piKernelGetInfo: unknown param_name");
  }

  return PI_SUCCESS;
}

pi_result L0(piKernelGetGroupInfo)(
  pi_kernel                  kernel,
  pi_device                  device,
  cl_kernel_work_group_info  param_name, // TODO: untie from OpenCL
  size_t                     param_value_size,
  void *                     param_value,
  size_t *                   param_value_size_ret)
{
  if (param_name == CL_KERNEL_WORK_GROUP_SIZE) {
    // The CL version of this interface returns the maximum total group
    // size, and not by suggested by components.
    // TODO: Tracked in level_zero/issues/262
    //
    uint32_t X, Y, Z;
    ZE_CALL(zeKernelSuggestGroupSize(
      kernel->L0Kernel,
      10000, 10000, 10000, &X, &Y, &Z));

    //fprintf(stderr, "(%d %d %d)\n", X, Y, Z);
    SET_PARAM_VALUE(size_t{X * Y * Z});
  }
  else {
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
  return PI_SUCCESS;
}

pi_result L0(piKernelRelease)(pi_kernel    kernel) {
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

  *event = L0(piEventCreate)(kernel->Program->Context, nullptr);
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
    (unsigned long int)ze_event, num_events_in_wait_list);
  for (pi_uint32 i = 0; i < num_events_in_wait_list; i++) {
    zePrint(" %lx", (unsigned long int)ze_event_wait_list[i]);
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
pi_event L0(piEventCreate)( // TODO: change to return pi_result
  pi_context    context,
  pi_result *   errcode_ret)
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
    *errcode_ret = pi_cast<pi_result>(ze_res);
    return 0;
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
    *errcode_ret = pi_cast<pi_result>(ze_res);
    return 0;
  }

  auto PiEvent = new _pi_event;
  PiEvent->RefCount = 1;
  PiEvent->L0Event = ze_event;

  return PiEvent;
}

pi_result L0(piEventGetInfo)(
  pi_event         event,
  cl_event_info    param_name, // TODO: untie from OpenCL
  size_t           param_value_size,
  void *           param_value,
  size_t *         param_value_size_ret) {

  pi_throw("piEventGetInfo: not implemented");
}

pi_result L0(piEventGetProfilingInfo)(
  pi_event            event,
  cl_profiling_info   param_name, // TODO: untie from OpenCL
  size_t              param_value_size,
  void *              param_value,
  size_t *            param_value_size_ret) {

  pi_throw("piEventGetProfilingInfo: not implemented");
}

pi_result L0(piEventsWait)(
  pi_uint32           num_events,
  const pi_event *    event_list)
{
  ze_result_t ze_result;

  for (uint32_t i = 0; i < num_events; i++) {
    ze_event_handle_t ze_event = event_list[i]->L0Event;
    zePrint("ze_event = %lx\n", (unsigned long int)ze_event);
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

  pi_throw("piEventSetCallback: not implemented");
}

pi_result L0(piEventSetStatus)(
  pi_event   event,
  pi_int32   execution_status) {

  pi_throw("piEventSetStatus: not implemented");
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
pi_sampler L0(piSamplerCreate)(
  pi_context                     context,
  const cl_sampler_properties *  sampler_properties, // TODO: untie from OpenCL
  pi_result *                    errcode_ret) {

  pi_throw("piSamplerCreate: not implemented");
}

pi_result L0(piSamplerGetInfo)(
  pi_sampler         sampler,
  cl_sampler_info    param_name, // TODO: untie from OpenCL
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

  *event = L0(piEventCreate)(queue->Context, nullptr);
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

  zePrint("calling zeCommandListAppendMemoryCopy() with\n"
          "  xe_event %lx\n"
          "  num_events_in_wait_list %d:",
      (unsigned long int)ze_event, num_events_in_wait_list);
  for (pi_uint32 i = 0; i < num_events_in_wait_list; i++) {
    zePrint(" %lx", (unsigned long int)ze_event_wait_list[i]);
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

  pi_throw("piEnqueueMemBufferReadRect: not implemented");
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

  pi_throw("piEnqueueMemBufferWrite: not implemented");
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

  pi_throw("piEnqueueMemBufferWriteRect: not implemented");
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

  pi_throw("piEnqueueMemBufferCopy: not implemented");
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

void * L0(piEnqueueMemBufferMap)( // TODO: change to return pi_result
  pi_queue          command_queue,
  pi_mem            buffer,
  pi_bool           blocking_map,
  cl_map_flags      map_flags,  // TODO: untie from OpenCL
  size_t            offset,
  size_t            size,
  pi_uint32         num_events_in_wait_list,
  const pi_event *  event_wait_list,
  pi_event *        event,
  pi_result *       errcode_ret) {

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

// TODO: implement support for images

pi_result L0(piMemImageGetInfo)()       {   pi_throw("piMemImageGetInfo: not implemented"); }
pi_result L0(piEnqueueMemImageRead)()   {   pi_throw("piEnqueueMemImageRead: not implemented"); }
pi_result L0(piEnqueueMemImageWrite)()  {   pi_throw("piEnqueueMemImageWrite: not implemented"); }
pi_result L0(piEnqueueMemImageCopy)()   {   pi_throw("piEnqueueMemImageCopy: not implemented"); }
pi_result L0(piEnqueueMemImageFill)()   {   pi_throw("piEnqueueMemImageFill: not implemented"); }
pi_result L0(piMemBufferPartition)()    {   pi_throw("piMemBufferPartition: not implemented"); }
pi_result L0(piEnqueueNativeKernel)()   {   pi_throw("piEnqueueNativeKernel: not implemented"); }

pi_result L0(piextGetDeviceFunctionPointer)() {
  pi_throw("piextGetDeviceFunctionPointer: not implemented");
}

#define _PI_API(api) \
  typedef decltype(::api) __type##api; \
  const __type##api * api##OtherPtr = (__type##api *) &L0(api);
#include <CL/sycl/detail/pi.def>

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

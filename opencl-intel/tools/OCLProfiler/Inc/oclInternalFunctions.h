#pragma once

// TODO: consider changing the header to src/cl_api
#include <CL/cl.h>

// these functions are used to bypass the CL intercept mechanism
// i.e. to avoid unnecessary tracing of internal information
//      retrieved & maintained by the server and API Debugger model

CL_API_ENTRY cl_int CL_API_CALL _clGetPlatformInfoINTERNAL(
    cl_platform_id platform, 
    cl_platform_info param_name, 
    size_t param_value_size, 
    void* param_value, 
    size_t* param_value_size_ret );

CL_API_ENTRY cl_int CL_API_CALL _clGetDeviceInfoINTERNAL(
    cl_device_id device,
    cl_device_info param_name, 
    size_t param_value_size, 
    void* param_value,
    size_t* param_value_size_ret );

CL_API_ENTRY cl_int CL_API_CALL _clGetContextInfoINTERNAL(
    cl_context context, 
    cl_context_info param_name, 
    size_t param_value_size, 
    void* param_value, 
    size_t* param_value_size_ret );

CL_API_ENTRY cl_int CL_API_CALL _clGetCommandQueueInfoINTERNAL(
    cl_command_queue command_queue,
    cl_command_queue_info param_name,
    size_t param_value_size,
    void* param_value,
    size_t* param_value_size_ret );

CL_API_ENTRY cl_int CL_API_CALL _clGetEventInfoINTERNAL(
    cl_event event,
    cl_event_info param_name,
    size_t param_value_size,
    void* param_value,
    size_t* param_value_size_ret );

CL_API_ENTRY cl_int CL_API_CALL _clGetImageInfoINTERNAL(
    cl_mem image,
    cl_image_info param_name, 
    size_t param_value_size,
    void* param_value,
    size_t* param_value_size_ret );

CL_API_ENTRY cl_int CL_API_CALL _clGetSamplerInfoINTERNAL(
    cl_sampler sampler,
    cl_sampler_info param_name,
    size_t param_value_size,
    void* param_value,
    size_t* param_value_size_ret );

CL_API_ENTRY cl_int CL_API_CALL _clGetProgramInfoINTERNAL(
    cl_program program,
    cl_program_info param_name,
    size_t param_value_size,
    void* param_value,
    size_t* param_value_size_ret );

CL_API_ENTRY cl_int CL_API_CALL _clGetProgramBuildInfoINTERNAL(
    cl_program program,
    cl_device_id device,
    cl_program_build_info param_name,
    size_t param_value_size,
    void* param_value,
    size_t* param_value_size_ret );

CL_API_ENTRY cl_int CL_API_CALL _clGetKernelInfoINTERNAL(
    cl_kernel kernel,
    cl_kernel_info param_name,
    size_t param_value_size,
    void* param_value,
    size_t* param_value_size_ret );

CL_API_ENTRY cl_int CL_API_CALL _clGetKernelArgInfoINTERNAL(
    cl_kernel kernel,
    cl_uint arg_indx,
    cl_kernel_arg_info param_name,
    size_t param_value_size,
    void* param_value,
    size_t* param_value_size_ret );

CL_API_ENTRY cl_int CL_API_CALL _clGetMemObjectInfoINTERNAL(
    cl_mem memobj,
    cl_mem_info param_name, 
    size_t param_value_size,
    void* param_value,
    size_t* param_value_size_ret );

CL_API_ENTRY cl_int CL_API_CALL _clGetEventProfilingInfoINTERNAL(
    cl_event event,
    cl_profiling_info param_name,
    size_t param_value_size,
    void* param_value,
    size_t* param_value_size_ret );

CL_API_ENTRY cl_int CL_API_CALL _clGetPlatformIDsINTERNAL(
    cl_uint num_entries, 
    cl_platform_id* platforms, 
    cl_uint* num_platforms );

CL_API_ENTRY cl_int CL_API_CALL _clGetDeviceIDsINTERNAL(
    cl_platform_id platform,
    cl_device_type device_type, 
    cl_uint num_entries, 
    cl_device_id* devices, 
    cl_uint* num_devices );

CL_API_ENTRY cl_int CL_API_CALL _clReleaseEventINTERNAL(
    cl_event event, bool sendNotify = true);

CL_API_ENTRY cl_int CL_API_CALL _clRetainEventINTERNAL(
    cl_event event, bool sendNotify = true );

CL_API_ENTRY cl_int CL_API_CALL _clSetEventCallbackINTERNAL(
    cl_event event,
    cl_int command_exec_callback_type,
    void (CL_CALLBACK *pfn_notify)( cl_event, cl_int, void * ),
    void *user_data );

CL_API_ENTRY cl_command_queue CL_API_CALL _clCreateCommandQueueINTERNAL(
    cl_context context,
    cl_device_id device,
    cl_command_queue_properties properties,
    cl_int* errcode_ret );

CL_API_ENTRY void* CL_API_CALL _clEnqueueMapImageINTERNAL(
    cl_command_queue command_queue,
    cl_mem image, 
    cl_bool blocking_map, 
    cl_map_flags map_flags, 
    const size_t* origin,
    const size_t* region,
    size_t* image_row_pitch,
    size_t* image_slice_pitch,
    cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list,
    cl_event* event,
    cl_int* errcode_ret );

CL_API_ENTRY cl_int CL_API_CALL _clReleaseCommandQueueINTERNAL(
    cl_command_queue command_queue, bool sendNotify = true);

CL_API_ENTRY cl_int CL_API_CALL _clRetainCommandQueueINTERNAL(
    cl_command_queue command_queue, bool sendNotify = true);

CL_API_ENTRY cl_int CL_API_CALL _clEnqueueUnmapMemObjectINTERNAL(
    cl_command_queue command_queue,
    cl_mem memobj,
    void* mapped_ptr,
    cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list,
    cl_event* event );

CL_API_ENTRY cl_int CL_API_CALL _clWaitForEventsINTERNAL(
    cl_uint num_events,
    const cl_event* event_list );

CL_API_ENTRY cl_int CL_API_CALL _clRetainMemObjectINTERNAL(
    cl_mem memobj, bool sendNotify = true );

CL_API_ENTRY cl_int CL_API_CALL _clReleaseMemObjectINTERNAL(
    cl_mem memobj, bool sendNotify = true );


CL_API_ENTRY cl_int CL_API_CALL _clEnqueueReadImageINTERNAL(
    cl_command_queue command_queue,
    cl_mem image,
    cl_bool blocking_read, 
    const size_t* origin,
    const size_t* region,
    size_t row_pitch,
    size_t slice_pitch, 
    void* ptr,
    cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list,
    cl_event* event );

CL_API_ENTRY cl_int CL_API_CALL _clEnqueueReadBufferINTERNAL(
    cl_command_queue command_queue,
    cl_mem buffer,
    cl_bool blocking_read,
    size_t offset,
    size_t cb, 
    void* ptr,
    cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list,
    cl_event* event );

CL_API_ENTRY cl_int CL_API_CALL _clSetUserEventStatusINTERNAL(
    cl_event event,
    cl_int execution_status);

CL_API_ENTRY cl_int CL_API_CALL _clEnqueueMarkerWithWaitListINTERNAL(
    cl_command_queue command_queue,
    cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list,
    cl_event* event );

CL_API_ENTRY cl_int CL_API_CALL _clRetainKernelINTERNAL(
    cl_kernel kernel, bool sendNotify );

CL_API_ENTRY cl_int CL_API_CALL _clReleaseKernelINTERNAL(
    cl_kernel kernel, bool sendNotify );

CL_API_ENTRY cl_int CL_API_CALL _clEnqueueAcquireGLObjectsINTERNAL(
    cl_command_queue command_queue,
    cl_uint num_objects,
    const cl_mem* mem_objects,
    cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list,
    cl_event* event);

CL_API_ENTRY cl_int CL_API_CALL _clEnqueueReleaseGLObjectsINTERNAL(
    cl_command_queue command_queue,
    cl_uint num_objects,
    const cl_mem* mem_objects,
    cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list,
    cl_event* event);

#if defined(_WIN32)
CL_API_ENTRY cl_int CL_API_CALL _clEnqueueAcquireD3D10ObjectsKHRINTERNAL(
    cl_command_queue command_queue,
    cl_uint num_objects,
    const cl_mem* mem_objects,
    cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list,
    cl_event* event);

CL_API_ENTRY cl_int CL_API_CALL _clEnqueueReleaseD3D10ObjectsKHRINTERNAL(
    cl_command_queue command_queue,
    cl_uint num_objects,
    const cl_mem* mem_objects,
    cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list,
    cl_event* event);

CL_API_ENTRY cl_int CL_API_CALL _clEnqueueAcquireD3D11ObjectsKHRINTERNAL(
    cl_command_queue command_queue,
    cl_uint num_objects,
    const cl_mem* mem_objects,
    cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list,
    cl_event* event);

CL_API_ENTRY cl_int CL_API_CALL _clEnqueueAcquireDX9MediaSurfacesKHRINTERNAL(
    cl_command_queue command_queue,
    cl_uint num_objects,
    const cl_mem* mem_objects,
    cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list,
    cl_event* event);

CL_API_ENTRY cl_int CL_API_CALL _clEnqueueReleaseDX9MediaSurfacesKHRINTERNAL(
    cl_command_queue command_queue,
    cl_uint num_objects,
    const cl_mem* mem_objects,
    cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list,
    cl_event* event);

CL_API_ENTRY cl_int CL_API_CALL _clEnqueueAcquireDX9ObjectsINTELINTERNAL(
    cl_command_queue command_queue,
    cl_uint num_objects,
    const cl_mem* mem_objects,
    cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list,
    cl_event* event);

CL_API_ENTRY cl_int CL_API_CALL _clEnqueueReleaseDX9ObjectsINTELINTERNAL(
    cl_command_queue command_queue,
    cl_uint num_objects,
    const cl_mem* mem_objects,
    cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list,
    cl_event* event);

#endif

CL_API_ENTRY cl_event CL_API_CALL _clCreateUserEventINTERNAL(
    cl_context context,
    cl_int *errcode_ret);

CL_API_ENTRY cl_int CL_API_CALL _clFinishINTERNAL(
    cl_command_queue command_queue);

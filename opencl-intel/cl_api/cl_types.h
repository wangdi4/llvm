// Copyright (c) 2006-2007 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#pragma once
/**************************************************************************************************
 *  cl_types.h                                         
 *  Created on: 10-Dec-2008 11:42:24 AM                      
 *  Implementation of the Class OpenCLFramework       
 *  Original author: ulevy                     
 *************************************************************************************************/

#include "cl_device_api.h"

#include <tmmintrin.h>
#include <CL\cl.h>

/**************************************************************************************************
* cl_err_code
* initial data type which represents the return values inside the framework
**************************************************************************************************/
typedef cl_int	cl_err_code;

typedef void (*logging_fn)(const char *, const void *, size_t, void *);

/**************************************************************************************************
* define widen string into multibyte
**************************************************************************************************/
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)

#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef INOUT
#define INOUT
#endif

/**************************************************************************************************
* CL_SUCCEEDED
* Checks whether a return code is success
**************************************************************************************************/
#define CL_SUCCEEDED(code)         (CL_SUCCESS == (code))

/**************************************************************************************************
* CL_FAILED
* Checks whether a return code is failure
**************************************************************************************************/
#define CL_FAILED(code)				(CL_SUCCESS > (code))

/**************************************************************************************************
* CL_ERR_OUT
* filter internal error codes
**************************************************************************************************/
#define CL_ERR_OUT(code)			((code) <= CL_ERR_START) ? CL_ERR_FAILURE : (code)

#define CL_INVALID_HANDLE			0

/**************************************************************************************************
* internal error codes
**************************************************************************************************/
#define		CL_ERR_START					-800	// marker
#define     CL_ERR_FAILURE                  CL_ERR_START
//////////////////////////////////////////////////////////////////////////
#define		CL_ERR_LOGGER_FAILED			-801
#define		CL_ERR_NOT_IMPLEMENTED			-802
#define		CL_ERR_NOT_SUPPORTED			-803
#define		CL_ERR_INITILIZATION_FAILED		-804
#define		CL_ERR_PLATFORM_FAILED			-805
#define		CL_ERR_CONTEXT_FAILED			-806
#define		CL_ERR_EXECUTION_FAILED			-807
#define		CL_ERR_FILE_NOT_EXISTS			-808
#define		CL_ERR_KEY_NOT_FOUND			-809
#define		CL_ERR_KEY_ALLREADY_EXISTS		-810
#define		CL_ERR_LIST_EMPTY				-811
#define		CL_ERR_DEVICE_INIT_FAIL			-850
#define		CL_ERR_FE_COMPILER_INIT_FAIL	-851
//////////////////////////////////////////////////////////////////////////
#define		CL_ERR_END						-899	// marker


//// ------------------------------------
//// vendor dispatch table structure
//

// Platform APIs
typedef cl_int CL_API_CALL(*pfn_clGetPlatformIDs)(
	cl_uint          num_entries,
	cl_platform_id * platforms,
	cl_uint *        num_platforms) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clGetPlatformInfo)(
	cl_platform_id   platform, 
	cl_platform_info param_name,
	size_t           param_value_size, 
	void *           param_value,
	size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

// Device APIs
typedef cl_int CL_API_CALL(*pfn_clGetDeviceIDs)(
	cl_platform_id   platform,
	cl_device_type   device_type, 
	cl_uint          num_entries, 
	cl_device_id *   devices, 
	cl_uint *        num_devices) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clGetDeviceInfo)(
	cl_device_id    device,
	cl_device_info  param_name, 
	size_t          param_value_size, 
	void *          param_value,
	size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

// Context APIs  
typedef cl_context CL_API_CALL(*pfn_clCreateContext)(
	const cl_context_properties * properties,
	cl_uint                 num_devices,
	const cl_device_id *    devices,
	void (*pfn_notify)(const char *, const void *, size_t, void *),
	void *                  user_data,
	cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef cl_context CL_API_CALL(*pfn_clCreateContextFromType)(
	const cl_context_properties * properties,
	cl_device_type          device_type,
	void (*pfn_notify)(const char *, const void *, size_t, void *),
	void *                  user_data,
	cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clRetainContext)(
	cl_context context) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clReleaseContext)(
	cl_context context) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clGetContextInfo)(
	cl_context         context, 
	cl_context_info    param_name, 
	size_t             param_value_size, 
	void *             param_value, 
	size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

// Command Queue APIs
typedef cl_command_queue CL_API_CALL(*pfn_clCreateCommandQueue)(
	cl_context                     context, 
	cl_device_id                   device, 
	cl_command_queue_properties    properties,
	cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clRetainCommandQueue)(
	cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clReleaseCommandQueue)(
	cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clGetCommandQueueInfo)(
	cl_command_queue      command_queue,
	cl_command_queue_info param_name,
	size_t                param_value_size,
	void *                param_value,
	size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clSetCommandQueueProperty)(
	cl_command_queue              command_queue,
	cl_command_queue_properties   properties, 
	cl_bool                        enable,
	cl_command_queue_properties * old_properties) CL_API_SUFFIX__VERSION_1_0;

// Memory Object APIs
typedef cl_mem CL_API_CALL(*pfn_clCreateBuffer)(
	cl_context   context,
	cl_mem_flags flags,
	size_t       size,
	void *       host_ptr,
	cl_int *     errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef cl_mem CL_API_CALL(*pfn_clCreateImage2D)(
	cl_context              context,
	cl_mem_flags            flags,
	const cl_image_format * image_format,
	size_t                  image_width,
	size_t                  image_height,
	size_t                  image_row_pitch, 
	void *                  host_ptr,
	cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef cl_mem CL_API_CALL(*pfn_clCreateImage3D)(
	cl_context              context,
	cl_mem_flags            flags,
	const cl_image_format * image_format,
	size_t                  image_width, 
	size_t                  image_height,
	size_t                  image_depth, 
	size_t                  image_row_pitch, 
	size_t                  image_slice_pitch, 
	void *                  host_ptr,
	cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clRetainMemObject)(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clReleaseMemObject)(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clGetSupportedImageFormats)(
	cl_context           context,
	cl_mem_flags         flags,
	cl_mem_object_type   image_type,
	cl_uint              num_entries,
	cl_image_format *    image_formats,
	cl_uint *            num_image_formats) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clGetMemObjectInfo)(
	cl_mem           memobj,
	cl_mem_info      param_name, 
	size_t           param_value_size,
	void *           param_value,
	size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clGetImageInfo)(
	cl_mem           image,
	cl_image_info    param_name, 
	size_t           param_value_size,
	void *           param_value,
	size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

// Sampler APIs
typedef cl_sampler CL_API_CALL(*pfn_clCreateSampler)(
	cl_context          context,
	cl_bool             normalized_coords, 
	cl_addressing_mode  addressing_mode, 
	cl_filter_mode      filter_mode,
	cl_int *            errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clRetainSampler)(cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clReleaseSampler)(cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clGetSamplerInfo)(
	cl_sampler         sampler,
	cl_sampler_info    param_name,
	size_t             param_value_size,
	void *             param_value,
	size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

// Program Object APIs
typedef cl_program CL_API_CALL(*pfn_clCreateProgramWithSource)(
	cl_context        context,
	cl_uint           count,
	const char **     strings,
	const size_t *    lengths,
	cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef cl_program CL_API_CALL(*pfn_clCreateProgramWithBinary)(
	cl_context                     context,
	cl_uint                        num_devices,
	const cl_device_id *           device_list,
	const size_t *                 lengths,
	const unsigned char **         binaries,
	cl_int *                       binary_status,
	cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clRetainProgram)(cl_program program) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clReleaseProgram)(cl_program program) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clBuildProgram)(
	cl_program           program,
	cl_uint              num_devices,
	const cl_device_id * device_list,
	const char *         options, 
	void (*pfn_notify)(cl_program program, void * user_data),
	void *               user_data) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clUnloadCompiler)(void) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clGetProgramInfo)(
	cl_program         program,
	cl_program_info    param_name,
	size_t             param_value_size,
	void *             param_value,
	size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clGetProgramBuildInfo)(
	cl_program            program,
	cl_device_id          device,
	cl_program_build_info param_name,
	size_t                param_value_size,
	void *                param_value,
	size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

// Kernel Object APIs
typedef cl_kernel CL_API_CALL(*pfn_clCreateKernel)(
	cl_program      program,
	const char *    kernel_name,
	cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clCreateKernelsInProgram)(
	cl_program     program,
	cl_uint        num_kernels,
	cl_kernel *    kernels,
	cl_uint *      num_kernels_ret) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clRetainKernel)(cl_kernel    kernel) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clReleaseKernel)(cl_kernel   kernel) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clSetKernelArg)(
	cl_kernel    kernel,
	cl_uint      arg_index,
	size_t       arg_size,
	const void * arg_value) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clGetKernelInfo)(
	cl_kernel       kernel,
	cl_kernel_info  param_name,
	size_t          param_value_size,
	void *          param_value,
	size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clGetKernelWorkGroupInfo)(
	cl_kernel                  kernel,
	cl_device_id               device,
	cl_kernel_work_group_info  param_name,
	size_t                     param_value_size,
	void *                     param_value,
	size_t *                   param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

// Event Object APIs
typedef cl_int CL_API_CALL(*pfn_clWaitForEvents)(
	cl_uint             num_events,
	const cl_event *    event_list) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clGetEventInfo)(
	cl_event         event,
	cl_event_info    param_name,
	size_t           param_value_size,
	void *           param_value,
	size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clRetainEvent)(cl_event event) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clReleaseEvent)(cl_event event) CL_API_SUFFIX__VERSION_1_0;

// Profiling APIs
typedef cl_int CL_API_CALL(*pfn_clGetEventProfilingInfo)(
	cl_event            event,
	cl_profiling_info   param_name,
	size_t              param_value_size,
	void *              param_value,
	size_t *            param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

// Flush and Finish APIs
typedef cl_int CL_API_CALL(*pfn_clFlush)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clFinish)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

// Enqueued Commands APIs
typedef cl_int CL_API_CALL(*pfn_clEnqueueReadBuffer)(
	cl_command_queue    command_queue,
	cl_mem              buffer,
	cl_bool             blocking_read,
	size_t              offset,
	size_t              cb, 
	void *              ptr,
	cl_uint             num_events_in_wait_list,
	const cl_event *    event_wait_list,
	cl_event *          event) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clEnqueueWriteBuffer)(
	cl_command_queue   command_queue, 
	cl_mem             buffer, 
	cl_bool            blocking_write, 
	size_t             offset, 
	size_t             cb, 
	const void *       ptr, 
	cl_uint            num_events_in_wait_list, 
	const cl_event *   event_wait_list, 
	cl_event *         event) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clEnqueueCopyBuffer)(
	cl_command_queue    command_queue, 
	cl_mem              src_buffer,
	cl_mem              dst_buffer, 
	size_t              src_offset,
	size_t              dst_offset,
	size_t              cb, 
	cl_uint             num_events_in_wait_list,
	const cl_event *    event_wait_list,
	cl_event *          event) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clEnqueueReadImage)(
	cl_command_queue     command_queue,
	cl_mem               image,
	cl_bool              blocking_read, 
	const size_t *       origin,
	const size_t *       region,
	size_t               row_pitch,
	size_t               slice_pitch, 
	void *               ptr,
	cl_uint              num_events_in_wait_list,
	const cl_event *     event_wait_list,
	cl_event *           event) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clEnqueueWriteImage)(
	cl_command_queue    command_queue,
	cl_mem              image,
	cl_bool             blocking_write, 
	const size_t *      origin,
	const size_t *      region,
	size_t              input_row_pitch,
	size_t              input_slice_pitch, 
	const void *        ptr,
	cl_uint             num_events_in_wait_list,
	const cl_event *    event_wait_list,
	cl_event *          event) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clEnqueueCopyImage)(
	cl_command_queue     command_queue,
	cl_mem               src_image,
	cl_mem               dst_image, 
	const size_t *       src_origin,
	const size_t *       dst_origin,
	const size_t *       region, 
	cl_uint              num_events_in_wait_list,
	const cl_event *     event_wait_list,
	cl_event *           event) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clEnqueueCopyImageToBuffer)(
	cl_command_queue command_queue,
	cl_mem           src_image,
	cl_mem           dst_buffer, 
	const size_t *   src_origin,
	const size_t *   region, 
	size_t           dst_offset,
	cl_uint          num_events_in_wait_list,
	const cl_event * event_wait_list,
	cl_event *       event) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clEnqueueCopyBufferToImage)(
	cl_command_queue command_queue,
	cl_mem           src_buffer,
	cl_mem           dst_image, 
	size_t           src_offset,
	const size_t *   dst_origin,
	const size_t *   region, 
	cl_uint          num_events_in_wait_list,
	const cl_event * event_wait_list,
	cl_event *       event) CL_API_SUFFIX__VERSION_1_0;

typedef void * CL_API_CALL(*pfn_clEnqueueMapBuffer)(
	cl_command_queue command_queue,
	cl_mem           buffer,
	cl_bool          blocking_map, 
	cl_map_flags     map_flags,
	size_t           offset,
	size_t           cb,
	cl_uint          num_events_in_wait_list,
	const cl_event * event_wait_list,
	cl_event *       event,
	cl_int *         errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef void * CL_API_CALL(*pfn_clEnqueueMapImage)(
	cl_command_queue  command_queue,
	cl_mem            image, 
	cl_bool           blocking_map, 
	cl_map_flags      map_flags, 
	const size_t *    origin,
	const size_t *    region,
	size_t *          image_row_pitch,
	size_t *          image_slice_pitch,
	cl_uint           num_events_in_wait_list,
	const cl_event *  event_wait_list,
	cl_event *        event,
	cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clEnqueueUnmapMemObject)(
	cl_command_queue command_queue,
	cl_mem           memobj,
	void *           mapped_ptr,
	cl_uint          num_events_in_wait_list,
	const cl_event *  event_wait_list,
	cl_event *        event) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clEnqueueNDRangeKernel)(
	cl_command_queue command_queue,
	cl_kernel        kernel,
	cl_uint          work_dim,
	const size_t *   global_work_offset,
	const size_t *   global_work_size,
	const size_t *   local_work_size,
	cl_uint          num_events_in_wait_list,
	const cl_event * event_wait_list,
	cl_event *       event) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clEnqueueTask)(
	cl_command_queue  command_queue,
	cl_kernel         kernel,
	cl_uint           num_events_in_wait_list,
	const cl_event *  event_wait_list,
	cl_event *        event) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clEnqueueNativeKernel)(
	cl_command_queue  command_queue,
	void (*user_func)(void *), 
	void *            args,
	size_t            cb_args, 
	cl_uint           num_mem_objects,
	const cl_mem *    mem_list,
	const void **     args_mem_loc,
	cl_uint           num_events_in_wait_list,
	const cl_event *  event_wait_list,
	cl_event *        event) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clEnqueueMarker)(
	cl_command_queue    command_queue,
	cl_event *          event) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clEnqueueWaitForEvents)(
	cl_command_queue command_queue,
	cl_uint          num_events,
	const cl_event * event_list) CL_API_SUFFIX__VERSION_1_0;

typedef cl_int CL_API_CALL(*pfn_clEnqueueBarrier)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

struct ocl_entry_points
{
	pfn_clGetPlatformInfo              clGetPlatformInfo;
	pfn_clGetDeviceIDs                 clGetDeviceIDs;
	pfn_clGetDeviceInfo                clGetDeviceInfo;
	pfn_clCreateContext                clCreateContext;
	pfn_clCreateContextFromType        clCreateContextFromType;
	pfn_clRetainContext                clRetainContext;
	pfn_clReleaseContext               clReleaseContext;
	pfn_clGetContextInfo               clGetContextInfo;
	pfn_clCreateCommandQueue           clCreateCommandQueue;
	pfn_clRetainCommandQueue           clRetainCommandQueue;
	pfn_clReleaseCommandQueue          clReleaseCommandQueue;
	pfn_clGetCommandQueueInfo          clGetCommandQueueInfo;
	pfn_clSetCommandQueueProperty      clSetCommandQueueProperty;
	pfn_clCreateBuffer                 clCreateBuffer;
	pfn_clCreateImage2D                clCreateImage2D;
	pfn_clCreateImage3D                clCreateImage3D;
	pfn_clRetainMemObject              clRetainMemObject;
	pfn_clReleaseMemObject             clReleaseMemObject;
	pfn_clGetSupportedImageFormats     clGetSupportedImageFormats;
	pfn_clGetMemObjectInfo             clGetMemObjectInfo;
	pfn_clGetImageInfo                 clGetImageInfo;
	pfn_clCreateSampler                clCreateSampler;
	pfn_clRetainSampler                clRetainSampler;
	pfn_clReleaseSampler               clReleaseSampler;
	pfn_clGetSamplerInfo               clGetSamplerInfo;
	pfn_clCreateProgramWithSource      clCreateProgramWithSource;
	pfn_clCreateProgramWithBinary      clCreateProgramWithBinary;
	pfn_clRetainProgram                clRetainProgram;
	pfn_clReleaseProgram               clReleaseProgram;
	pfn_clBuildProgram                 clBuildProgram;
	pfn_clUnloadCompiler               clUnloadCompiler;
	pfn_clGetProgramInfo               clGetProgramInfo;
	pfn_clGetProgramBuildInfo          clGetProgramBuildInfo;
	pfn_clCreateKernel                 clCreateKernel;
	pfn_clCreateKernelsInProgram       clCreateKernelsInProgram;
	pfn_clRetainKernel                 clRetainKernel;
	pfn_clReleaseKernel                clReleaseKernel;
	pfn_clSetKernelArg                 clSetKernelArg;
	pfn_clGetKernelInfo                clGetKernelInfo;
	pfn_clGetKernelWorkGroupInfo       clGetKernelWorkGroupInfo;
	pfn_clWaitForEvents                clWaitForEvents;
	pfn_clGetEventInfo                 clGetEventInfo;
	pfn_clRetainEvent                  clRetainEvent;
	pfn_clReleaseEvent                 clReleaseEvent;
	pfn_clGetEventProfilingInfo        clGetEventProfilingInfo;
	pfn_clFlush                        clFlush;
	pfn_clFinish                       clFinish;
	pfn_clEnqueueReadBuffer            clEnqueueReadBuffer;
	pfn_clEnqueueWriteBuffer           clEnqueueWriteBuffer;
	pfn_clEnqueueCopyBuffer            clEnqueueCopyBuffer;
	pfn_clEnqueueReadImage             clEnqueueReadImage;
	pfn_clEnqueueWriteImage            clEnqueueWriteImage;
	pfn_clEnqueueCopyImage             clEnqueueCopyImage;
	pfn_clEnqueueCopyImageToBuffer     clEnqueueCopyImageToBuffer;
	pfn_clEnqueueCopyBufferToImage     clEnqueueCopyBufferToImage;
	pfn_clEnqueueMapBuffer             clEnqueueMapBuffer;
	pfn_clEnqueueMapImage              clEnqueueMapImage;
	pfn_clEnqueueUnmapMemObject        clEnqueueUnmapMemObject;
	pfn_clEnqueueNDRangeKernel         clEnqueueNDRangeKernel;
	pfn_clEnqueueTask                  clEnqueueTask;
	pfn_clEnqueueNativeKernel          clEnqueueNativeKernel;
	pfn_clEnqueueMarker                clEnqueueMarker;
	pfn_clEnqueueWaitForEvents         clEnqueueWaitForEvents;
	pfn_clEnqueueBarrier               clEnqueueBarrier;
};



struct _cl_object
{
	void * dispatch;
	void * object;
};

struct _cl_platform_id : public _cl_object
{
};

struct _cl_device_id : public _cl_object
{
};

struct _cl_context : public _cl_object
{
};

struct _cl_command_queue : public _cl_object
{
};

struct _cl_mem : public _cl_object
{
};

struct _cl_program : public _cl_object
{
};

struct _cl_kernel : public _cl_object
{
};

struct _cl_event : public _cl_object
{
};

struct _cl_sampler : public _cl_object
{
};


// ------------------------------------
// Define OCL types
// Single precision floating point
// Used by built-in functions
typedef __m64 float2;
typedef __m128 float4;
struct float8
{
	__m128	a; __m128	b;
};
struct float16
{
	__m128	a; __m128	b; __m128	c;	__m128	d;
};

// ------------------------------------
// Define OCL integer types
// char{2|4|8|16}, uchar, uchar{2|4|8|16}, short, short{2|4|8|16},
// ushort, ushort{2|4|8|16}, int, int{2|4|8|16}, uint,
// uint{2|4|8|16}, long, long{2|4|8|16} ulong, or ulong{2|4|8|16}

// TODO: The short vector parameters ( < 64bit) are passed, one by one in stack
// So each element should be defined as 'int'
// Change LLVM backend (JIT) to perform more efficient call to short vectors

//char - 8bit
typedef char			_1i8;
struct _2i8 { int a; int b;};
typedef __m128i			_2i8p;
struct _4i8 { int a; int b; int c; int d;};
typedef __m128i			_4i8p;
typedef __m64			_8i8;
typedef __m128i			_8i8p;
typedef __m128i			_16i8;

//uchar - 8bit
typedef unsigned char	_1u8;
struct _2u8 { unsigned int a; unsigned int b;};
typedef __m128i			_2u8p;
struct _4u8 { int a; int b; int c; int d;};
typedef __m128i			_4u8p;
typedef __m64			_8u8;
typedef __m128i			_8u8p;
typedef __m128i			_16u8;

//short - 16 bit
typedef short			_1i16;
struct _2i16 { int a; int b;};
typedef __m128i			_2i16p;
typedef __m64			_4i16;
typedef __m128i			_4i16p;
typedef __m128i			_8i16;
struct _16i16 { __m128i a; __m128i b;};

//ushort - 16 bit
typedef unsigned short	_1u16;
struct _2u16 { unsigned int a; unsigned int b;};
typedef __m128i			_2u16p;
typedef __m64			_4u16;
typedef __m128i			_4u16p;
typedef __m128i			_8u16;
struct _16u16 { __m128i a; __m128i b;};

//int - 32 bit
typedef int				_1i32;
typedef __m64			_2i32;
typedef __m128i			_4i32;
struct _8i32 { __m128i a; __m128i b;};
struct _16i32 { __m128i a; __m128i b;__m128i c; __m128i d;};

//unit - 32 bit
typedef unsigned int	_1u32;
typedef __m64			_2u32;
typedef __m128i			_4u32;
struct _8u32 { __m128i a; __m128i b;};
struct _16u32 { __m128i a; __m128i b;__m128i c; __m128i d;};

//long - 64 bit
typedef __int64		_1i64;
typedef __m128i		_2i64;
struct _4i64 { __m128i a; __m128i b;};
struct _8i64 { __m128i a; __m128i b;__m128i c; __m128i d;};
struct _16i64 { __m128i a; __m128i b;__m128i c; __m128i d;
				__m128i e; __m128i f;__m128i g; __m128i h;};

//ulong 64 bit
typedef unsigned __int64	_1u64;
typedef __m128i				_2u64;
struct _4u64 { __m128i a; __m128i b;};
struct _8u64 { __m128i a; __m128i b;__m128i c; __m128i d;};
struct _16u64 { __m128i a; __m128i b;__m128i c; __m128i d;
				__m128i e; __m128i f;__m128i g; __m128i h;};


///////////////////////////////////////
// Memory runtime declaration
typedef struct _cl_mem_obj_descriptor
{
	cl_uint			dim_count;				// A number of dimensions in the memory object.
	unsigned int	dim[MAX_WORK_DIM];		// Multi-dimentional size of the object.
	size_t			pitch[MAX_WORK_DIM-1];	// Multi-dimentional pitch of the object, valid only for images (2D/3D).
	cl_image_format	format;					// Format of the memory object,valid only for images (2D/3D).
	void*			pData;					// A pointer to the object wherein the object data is stored.
											// Could be a valid memory pointer or a handle to other object.
	unsigned		uiElementSize;			// Size of image pixel element.
} cl_mem_obj_descriptor;

typedef _cl_mem_obj_descriptor* image2d_t;
typedef _cl_mem_obj_descriptor* image3d_t;

// Channel order
enum {
  CLK_R,
  CLK_A,
  CLK_RG,
  CLK_RA,
  CLK_RGB,
  CLK_RGBA,
  CLK_BGRA,
  CLK_ARGB,
  CLK_INTENSITY,
  CLK_LUMINANCE
};

// Channel Type
enum {
  // valid formats for float return types
  CLK_SNORM_INT8,
  CLK_SNORM_INT16,
  CLK_UNORM_INT8,
  CLK_UNORM_INT16,
  CLK_UNORM_SHORT_565,
  CLK_UNORM_SHORT_555,
  CLK_UNORM_INT_101010,

  CLK_SIGNED_INT8,
  CLK_SIGNED_INT16,
  CLK_SIGNED_INT32,
  CLK_UNSIGNED_INT8,
  CLK_UNSIGNED_INT16,
  CLK_UNSIGNED_INT32,

  CLK_HALF_FLOAT,            // four channel RGBA half
  CLK_FLOAT,                 // four channel RGBA float
};


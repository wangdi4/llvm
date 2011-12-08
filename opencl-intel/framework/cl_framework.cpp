// OpenCL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "cl_framework.h"
#include "cl_objects_map.h"
#include "framework_proxy.h"
#include "cl_cpu_detect.h"
#include "cl_linux_utils.h"
#include "ocl_itt.h"

#ifndef _WIN32
#include "cl_framework_alias_linux.h"
#endif





#if defined(USE_GPA)

#define CALL_INSTRUMENTED_API(module, return_type, function_call) \
ocl_gpa_data *pGPAData = module->GetGPAData(); \
if ((NULL != pGPAData) && (pGPAData->bUseGPA) && (pGPAData->bEnableAPITracing)) \
{ \
    __itt_string_handle* pAPINameHandle = __itt_string_handle_createA(__FUNCTION__); \
    __itt_task_begin(pGPAData->pAPIDomain, __itt_null, __itt_null, pAPINameHandle); \
} \
return_type ret_val = module->function_call; \
if ((NULL != pGPAData) && (pGPAData->bUseGPA) && (pGPAData->bEnableAPITracing)) \
{ \
	__itt_task_end(pGPAData->pAPIDomain); \
} \
return ret_val;

#else

#define CALL_INSTRUMENTED_API(module, return_type, function_call) \
return module->function_call;

#endif

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

ExtensionFunctionAddressResolveMap g_extFuncResolveMap;
void* RegisterExtensionFunctionAddress(const char* pFuncName, void* pFuncPtr)
{
	g_extFuncResolveMap.insert(std::pair<std::string, void*>(pFuncName,pFuncPtr));
	return pFuncPtr;
}

void * CL_API_CALL clGetExtensionFunctionAddress(const char *funcname)
{
	if ( NULL == funcname )
	{
		return NULL;
	}
	ExtensionFunctionAddressResolveMap::const_iterator ptr = g_extFuncResolveMap.find(funcname);
	if ( g_extFuncResolveMap.end() == ptr)
	{
		return NULL;
	}

	return ptr->second;
}
SET_ALIAS(clGetExtensionFunctionAddress);

///////////////////////////////////////////////////////////////////////////////////////////////////
// Platform APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_int CL_API_CALL clGetPlatformIDs(cl_uint num_entries, cl_platform_id * platforms, cl_uint * num_platforms) 
{
    CALL_INSTRUMENTED_API(PLATFORM_MODULE,  cl_int, GetPlatformIDs(num_entries, platforms, num_platforms));
}
SET_ALIAS(clGetPlatformIDs);
REGISTER_EXTENSION_FUNCTION(clIcdGetPlatformIDsKHR, clGetPlatformIDs);

cl_int CL_API_CALL clGetPlatformInfo(cl_platform_id platform,
						 cl_platform_info param_name, 
						 size_t param_value_size, 
						 void* param_value, 
						 size_t* param_value_size_ret)
{
    CALL_INSTRUMENTED_API(PLATFORM_MODULE,  cl_int, GetPlatformInfo(platform, param_name, param_value_size, param_value, param_value_size_ret));
}
SET_ALIAS(clGetPlatformInfo);

///////////////////////////////////////////////////////////////////////////////////////////////////
// Device APIs
///////////////////////////////////////////////////////////////////////////////////////////////////

cl_int CL_API_CALL clGetDeviceIDs(cl_platform_id platform,
					  cl_device_type device_type, 
					  cl_uint num_entries, 
					  cl_device_id* devices, 
			          cl_uint* num_devices)
{
    CALL_INSTRUMENTED_API(PLATFORM_MODULE, cl_int, GetDeviceIDs(platform, device_type, num_entries, devices, num_devices));
}
SET_ALIAS(clGetDeviceIDs);

cl_int CL_API_CALL clGetDeviceInfo(cl_device_id device,
					   cl_device_info param_name, 
					   size_t param_value_size, 
					   void* param_value,
					   size_t* param_value_size_ret)
{
    CALL_INSTRUMENTED_API(PLATFORM_MODULE, cl_int, GetDeviceInfo(device, param_name, param_value_size, param_value, param_value_size_ret));
}
SET_ALIAS(clGetDeviceInfo);

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context APIs
///////////////////////////////////////////////////////////////////////////////////////////////////

cl_context CL_API_CALL clCreateContext(const cl_context_properties * properties,
						   cl_uint num_devices,
						   const cl_device_id * devices,
						   logging_fn pfn_notify,
						   void * user_data,
						   cl_int * errcode_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_context, CreateContext(properties, num_devices, devices, pfn_notify, user_data, errcode_ret));
}
SET_ALIAS(clCreateContext);

cl_context CL_API_CALL clCreateContextFromType(const cl_context_properties * properties,
								   cl_device_type          device_type,
								   logging_fn              pfn_notify,
								   void *                  user_data,
								   cl_int *                errcode_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_context, CreateContextFromType(properties, device_type, pfn_notify, user_data, errcode_ret));
}
SET_ALIAS(clCreateContextFromType);

cl_int CL_API_CALL clRetainContext(cl_context context)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, RetainContext(context));
}
SET_ALIAS(clRetainContext);

cl_int CL_API_CALL clReleaseContext(cl_context context)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, ReleaseContext(context));
}
SET_ALIAS(clReleaseContext);

cl_int CL_API_CALL clGetContextInfo(cl_context      context,
						cl_context_info param_name,
						size_t          param_value_size,
						void *          param_value,
						size_t *        param_value_size_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetContextInfo(context, param_name, param_value_size, param_value, param_value_size_ret));
}
SET_ALIAS(clGetContextInfo);
///////////////////////////////////////////////////////////////////////////////////////////////////
// Command Queue APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_command_queue CL_API_CALL clCreateCommandQueue(cl_context                  context, 
									  cl_device_id                device, 
									  cl_command_queue_properties properties, 
									  cl_int *                    errcode_ret)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_command_queue, CreateCommandQueue(context, device, properties, errcode_ret));
}
SET_ALIAS(clCreateCommandQueue);
cl_int CL_API_CALL clRetainCommandQueue(cl_command_queue command_queue)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, RetainCommandQueue(command_queue));
}
SET_ALIAS(clRetainCommandQueue);
cl_int CL_API_CALL clReleaseCommandQueue(cl_command_queue command_queue)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, ReleaseCommandQueue(command_queue));
}
SET_ALIAS(clReleaseCommandQueue);
cl_int CL_API_CALL clGetCommandQueueInfo(cl_command_queue      command_queue, 
							 cl_command_queue_info param_name, 
							 size_t                param_value_size, 
							 void *                param_value, 
							 size_t *              param_value_size_ret)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, GetCommandQueueInfo(command_queue, param_name, param_value_size, param_value, param_value_size_ret));
}
SET_ALIAS(clGetCommandQueueInfo);
cl_int CL_API_CALL clSetCommandQueueProperty(cl_command_queue              command_queue,
								 cl_command_queue_properties   properties,
								 cl_bool                       enable,
								 cl_command_queue_properties * old_properties)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, SetCommandQueueProperty(command_queue, properties, enable, old_properties));
}
#ifdef CL_USE_DEPRECATED_OPENCL_1_0_APIS
#warning CL_USE_DEPRECATED_OPENCL_1_0_APIS is defined. These APIs are unsupported and untested in OpenCL 1.1!
	SET_ALIAS(clSetCommandQueueProperty);
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////
// Memory Object APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_mem CL_API_CALL clCreateBuffer(cl_context   context, 
					  cl_mem_flags flags, 
					  size_t       size, 
					  void *       host_ptr, 
					  cl_int *     errcode_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_mem, CreateBuffer(context, flags, size, host_ptr, errcode_ret));
}
SET_ALIAS(clCreateBuffer);

cl_mem CL_API_CALL clCreateSubBuffer(cl_mem buffer,
				  cl_mem_flags				flags,
				  cl_buffer_create_type     buffer_create_type,
				  const void *              buffer_create_info,
				  cl_int *                  errcode_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_mem, CreateSubBuffer(buffer, flags, buffer_create_type, buffer_create_info, errcode_ret));
}
SET_ALIAS(clCreateSubBuffer);

cl_int CL_API_CALL
clSetMemObjectDestructorCallback(cl_mem			memObj,
								 mem_dtor_fn	pfn_notify,
								void *			pUserData )
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, SetMemObjectDestructorCallback(memObj, pfn_notify, pUserData));
}
SET_ALIAS(clSetMemObjectDestructorCallback);

cl_mem CL_API_CALL clCreateImage(
               cl_context context,
               cl_mem_flags flags,
               const cl_image_format *image_format,
               const cl_image_desc *image_desc,
               void *host_ptr,
               cl_int *errcode_ret)
{
    CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_mem, CreateImage(context, flags, image_format, image_desc, host_ptr, errcode_ret));
}
SET_ALIAS(clCreateImage);

cl_mem CL_API_CALL clCreateImage2D(cl_context              context,
					   cl_mem_flags            flags,
					   const cl_image_format * image_format,
					   size_t                  image_width,
					   size_t                  image_height,
					   size_t                  image_row_pitch,
					   void *                  host_ptr,
					   cl_int *                errcode_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_mem, CreateImage2D(context, flags, image_format, image_width, image_height, image_row_pitch, host_ptr, errcode_ret));
}
SET_ALIAS(clCreateImage2D);
                        
cl_mem CL_API_CALL clCreateImage3D(cl_context              context,
					   cl_mem_flags            flags,
					   const cl_image_format * image_format,
					   size_t                  image_width,
					   size_t                  image_height,
					   size_t                  image_depth,
					   size_t                  image_row_pitch,
					   size_t                  image_slice_pitch,
					   void *                  host_ptr,
					   cl_int *                errcode_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_mem, CreateImage3D(context, flags, image_format, image_width, image_height, image_depth, image_row_pitch, image_slice_pitch, host_ptr, errcode_ret));
}
SET_ALIAS(clCreateImage3D);
                       
cl_int CL_API_CALL clRetainMemObject(cl_mem memobj)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, RetainMemObject(memobj));
}
SET_ALIAS(clRetainMemObject);

cl_int CL_API_CALL clReleaseMemObject(cl_mem memobj)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, ReleaseMemObject(memobj));
}
SET_ALIAS(clReleaseMemObject);

cl_int CL_API_CALL clGetSupportedImageFormats(cl_context           context,
								  cl_mem_flags         flags,
								  cl_mem_object_type   image_type,
								  cl_uint              num_entries,
								  cl_image_format *    image_formats,
								  cl_uint *            num_image_formats)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetSupportedImageFormats(context, flags, image_type, num_entries, image_formats, num_image_formats));
}
SET_ALIAS(clGetSupportedImageFormats);
                                    
cl_int CL_API_CALL clGetMemObjectInfo(cl_mem           memobj,
						  cl_mem_info      param_name, 
						  size_t           param_value_size,
						  void *           param_value,
						  size_t *         param_value_size_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetMemObjectInfo(memobj, param_name, param_value_size, param_value, param_value_size_ret));
}
SET_ALIAS(clGetMemObjectInfo);

cl_int CL_API_CALL clGetImageInfo(cl_mem           image,
					  cl_image_info    param_name, 
					  size_t           param_value_size,
					  void *           param_value,
					  size_t *         param_value_size_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetImageInfo(image, param_name, param_value_size, param_value, param_value_size_ret));
}
SET_ALIAS(clGetImageInfo);
///////////////////////////////////////////////////////////////////////////////////////////////////
// Sampler APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_sampler CL_API_CALL clCreateSampler(cl_context			context,
						   cl_bool				normalized_coords,
						   cl_addressing_mode	addressing_mode,
						   cl_filter_mode		filter_mode,
						   cl_int *				errcode_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_sampler, CreateSampler(context, normalized_coords, addressing_mode, filter_mode, errcode_ret));
}
SET_ALIAS(clCreateSampler);
cl_int CL_API_CALL clRetainSampler(cl_sampler sampler)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, RetainSampler(sampler));
}
SET_ALIAS(clRetainSampler);

cl_int CL_API_CALL clReleaseSampler(cl_sampler sampler)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, ReleaseSampler(sampler));
}
SET_ALIAS(clReleaseSampler);

cl_int CL_API_CALL clGetSamplerInfo(cl_sampler		sampler,
						cl_sampler_info	param_name,
						size_t			param_value_size,
						void *			param_value,
						size_t *		param_value_size_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetSamplerInfo(sampler, param_name, param_value_size, param_value, param_value_size_ret));
}
SET_ALIAS(clGetSamplerInfo);
                            
///////////////////////////////////////////////////////////////////////////////////////////////////
// Program Object APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_program CL_API_CALL clCreateProgramWithSource(cl_context     context,
									 cl_uint        count,
									 const char **  strings,
									 const size_t * lengths,
									 cl_int *       errcode_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_program, CreateProgramWithSource(context, count, strings, lengths, errcode_ret));
}
SET_ALIAS(clCreateProgramWithSource);

cl_program CL_API_CALL clCreateProgramWithBinary(cl_context           context,
									 cl_uint              num_devices,
									 const cl_device_id *	device_list,
									 const size_t *			lengths,
									 const unsigned char **	binaries,
									 cl_int *				binary_status,
									 cl_int *				errcode_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_program, CreateProgramWithBinary(context, num_devices, device_list, lengths, binaries, binary_status, errcode_ret));
}
SET_ALIAS(clCreateProgramWithBinary);

cl_int CL_API_CALL clRetainProgram(cl_program program)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, RetainProgram(program));
}
SET_ALIAS(clRetainProgram);

cl_int CL_API_CALL clReleaseProgram(cl_program program)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, ReleaseProgram(program));
}
SET_ALIAS(clReleaseProgram);

cl_int CL_API_CALL clBuildProgram(cl_program           program,
					  cl_uint              num_devices,
					  const cl_device_id * device_list,
					  const char *         options, 
					  void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data),
					  void *               user_data)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, BuildProgram(program, num_devices, device_list, options, pfn_notify, user_data));
}
SET_ALIAS(clBuildProgram);

cl_int CL_API_CALL clUnloadCompiler(void)
{
	CALL_INSTRUMENTED_API(PLATFORM_MODULE, cl_int, UnloadCompiler());
}
SET_ALIAS(clUnloadCompiler);

cl_int CL_API_CALL clGetProgramInfo(cl_program      program,
						cl_program_info param_name,
						size_t          param_value_size,
						void *          param_value,
						size_t *        param_value_size_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetProgramInfo(program, param_name, param_value_size, param_value, param_value_size_ret));
}
SET_ALIAS(clGetProgramInfo);

cl_int CL_API_CALL clGetProgramBuildInfo(cl_program            program,
							 cl_device_id          device,
							 cl_program_build_info param_name,
							 size_t                param_value_size,
							 void *                param_value,
							 size_t *              param_value_size_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetProgramBuildInfo(program, device, param_name, param_value_size, param_value, param_value_size_ret));
}
SET_ALIAS(clGetProgramBuildInfo);
                           
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel Object APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_kernel CL_API_CALL clCreateKernel(cl_program   program,
						 const char * kernel_name,
						 cl_int *     errcode_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_kernel, CreateKernel(program, kernel_name, errcode_ret));
}
SET_ALIAS(clCreateKernel);

cl_int CL_API_CALL clCreateKernelsInProgram(cl_program  program,
								cl_uint     num_kernels,
								cl_kernel * kernels,
								cl_uint *   num_kernels_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, CreateKernelsInProgram(program, num_kernels, kernels, num_kernels_ret));
}
SET_ALIAS(clCreateKernelsInProgram);

cl_int CL_API_CALL clRetainKernel(cl_kernel kernel)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, RetainKernel(kernel));
}
SET_ALIAS(clRetainKernel);

cl_int CL_API_CALL clReleaseKernel(cl_kernel kernel)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, ReleaseKernel(kernel));
}
SET_ALIAS(clReleaseKernel);

cl_int CL_API_CALL clSetKernelArg(cl_kernel    kernel,
					  cl_uint      arg_indx,
					  size_t       arg_size,
					  const void * arg_value)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, SetKernelArg(kernel, arg_indx, arg_size, arg_value));
}
SET_ALIAS(clSetKernelArg);

cl_int CL_API_CALL clGetKernelInfo(cl_kernel      kernel,
					   cl_kernel_info param_name,
					   size_t         param_value_size,
					   void *         param_value,
					   size_t *       param_value_size_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetKernelInfo(kernel, param_name, param_value_size, param_value, param_value_size_ret));
}
SET_ALIAS(clGetKernelInfo);

cl_int CL_API_CALL clGetKernelWorkGroupInfo(cl_kernel                 kernel,
								cl_device_id              device,
								cl_kernel_work_group_info param_name,
								size_t                    param_value_size,
								void *                    param_value,
								size_t *                  param_value_size_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetKernelWorkGroupInfo(kernel, device, param_name, param_value_size, param_value, param_value_size_ret));
}
SET_ALIAS(clGetKernelWorkGroupInfo);
///////////////////////////////////////////////////////////////////////////////////////////////////
// Event Object APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_int CL_API_CALL clWaitForEvents(cl_uint num_events, const cl_event * event_list)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, WaitForEvents(num_events, event_list));
}
SET_ALIAS(clWaitForEvents);
cl_int CL_API_CALL clGetEventInfo(cl_event		event,
					  cl_event_info	param_name,
					  size_t		param_value_size,
					  void *		param_value,
					  size_t *		param_value_size_ret)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, GetEventInfo(event, param_name, param_value_size, param_value, param_value_size_ret));
}
SET_ALIAS(clGetEventInfo);

cl_int CL_API_CALL clRetainEvent(cl_event event)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, RetainEvent(event));
}
SET_ALIAS(clRetainEvent);

cl_int CL_API_CALL clReleaseEvent(cl_event event)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, ReleaseEvent(event));
}
SET_ALIAS(clReleaseEvent);
///////////////////////////////////////////////////////////////////////////////////////////////////
// Profiling APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_int CL_API_CALL clGetEventProfilingInfo(cl_event				event,
							   cl_profiling_info	param_name,
							   size_t				param_value_size,
							   void *				param_value,
							   size_t *				param_value_size_ret)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, GetEventProfilingInfo(event, param_name, param_value_size, param_value, param_value_size_ret));
}
SET_ALIAS(clGetEventProfilingInfo);
///////////////////////////////////////////////////////////////////////////////////////////////////
// Flush and Finish APIs
///////////////////////////////////////////////////////////////////////////////////////////////////                              
cl_int CL_API_CALL clFlush(cl_command_queue command_queue)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, Flush(command_queue));
}
SET_ALIAS(clFlush);
cl_int CL_API_CALL clFinish(cl_command_queue command_queue)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, Finish(command_queue));
}
SET_ALIAS(clFinish);

///////////////////////////////////////////////////////////////////////////////////////////////////
// Enqueued Commands APIs
///////////////////////////////////////////////////////////////////////////////////////////////////                              
cl_int CL_API_CALL clEnqueueReadBuffer(cl_command_queue	command_queue,
						   cl_mem			buffer,
						   cl_bool			blocking_read,
						   size_t			offset,
						   size_t			cb,
						   void *			ptr,
						   cl_uint			num_events_in_wait_list,
						   const cl_event * event_wait_list,
						   cl_event *		event)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueReadBuffer(command_queue, buffer, blocking_read, offset, cb, ptr, num_events_in_wait_list, event_wait_list, event));
}
SET_ALIAS(clEnqueueReadBuffer);

cl_int CL_API_CALL clEnqueueReadBufferRect(
					cl_command_queue    command_queue,
                        cl_mem              buffer,
                        cl_bool             blocking_read,
                        const size_t        buffer_origin[MAX_WORK_DIM],
                        const size_t        host_origin[MAX_WORK_DIM], 
                        const size_t        region[MAX_WORK_DIM],
                        size_t              buffer_row_pitch,
                        size_t              buffer_slice_pitch,
                        size_t              host_row_pitch,
                        size_t              host_slice_pitch,
                        void *              ptr,
                        cl_uint             num_events_in_wait_list,
                        const cl_event *    event_wait_list,
                        cl_event *          event )
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueReadBufferRect(command_queue, buffer, blocking_read, buffer_origin, host_origin, region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event));
}
SET_ALIAS(clEnqueueReadBufferRect);

cl_int CL_API_CALL clEnqueueWriteBuffer(cl_command_queue	command_queue,
							cl_mem				buffer,
							cl_bool				blocking_write,
							size_t				offset,
							size_t				cb,
							const void *		ptr,
							cl_uint				num_events_in_wait_list,
							const cl_event *	event_wait_list,
							cl_event *			event)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueWriteBuffer(command_queue, buffer, blocking_write, offset, cb, ptr, num_events_in_wait_list, event_wait_list, event));
}
SET_ALIAS(clEnqueueWriteBuffer);

cl_int CL_API_CALL  clEnqueueWriteBufferRect(
                         cl_command_queue    command_queue,
                         cl_mem              buffer,
                         cl_bool             blocking_read,
                         const size_t        buffer_origin[MAX_WORK_DIM],
                         const size_t        host_origin[MAX_WORK_DIM], 
                         const size_t        region[MAX_WORK_DIM],
                         size_t              buffer_row_pitch,
                         size_t              buffer_slice_pitch,
                         size_t              host_row_pitch,
                         size_t              host_slice_pitch,
                         const void *        ptr,
                         cl_uint             num_events_in_wait_list,
                         const cl_event *    event_wait_list,
                         cl_event *          event)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueWriteBufferRect(command_queue, buffer, blocking_read, buffer_origin, host_origin, region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event));
}
SET_ALIAS(clEnqueueWriteBufferRect);

cl_int CL_API_CALL clEnqueueCopyBuffer(cl_command_queue	command_queue,
						   cl_mem			src_buffer,
						   cl_mem			dst_buffer,
						   size_t			src_offset,
						   size_t			dst_offset,
						   size_t			cb,
						   cl_uint			num_events_in_wait_list,
						   const cl_event * event_wait_list,
						   cl_event *		event)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueCopyBuffer(command_queue, src_buffer, dst_buffer, src_offset, dst_offset, cb, num_events_in_wait_list, event_wait_list, event));
}
SET_ALIAS(clEnqueueCopyBuffer);

cl_int CL_API_CALL clEnqueueCopyBufferRect(cl_command_queue    command_queue, 
							cl_mem              src_buffer,
							cl_mem              dst_buffer, 
							const size_t        src_origin[MAX_WORK_DIM],
							const size_t        dst_origin[MAX_WORK_DIM],
							const size_t        region[MAX_WORK_DIM], 
							size_t              src_row_pitch,
							size_t              src_slice_pitch,
							size_t              dst_row_pitch,
							size_t              dst_slice_pitch,
							cl_uint             num_events_in_wait_list,
							const cl_event *    event_wait_list,
							cl_event *          event)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueCopyBufferRect(command_queue, src_buffer, dst_buffer, src_origin, dst_origin, region, src_row_pitch, src_slice_pitch, dst_row_pitch, dst_slice_pitch, num_events_in_wait_list, event_wait_list, event));
}
SET_ALIAS(clEnqueueCopyBufferRect);
                            
cl_int CL_API_CALL clEnqueueReadImage(cl_command_queue command_queue,
						  cl_mem			image,
						  cl_bool			blocking_read, 
						  const size_t	    origin[MAX_WORK_DIM],
						  const size_t	    region[MAX_WORK_DIM],
						  size_t			row_pitch,
						  size_t			slice_pitch, 
						  void *			ptr,
						  cl_uint			num_events_in_wait_list,
						  const cl_event *	event_wait_list,
						  cl_event *		event)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueReadImage(command_queue, image, blocking_read, origin, region, row_pitch, slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event));
}
SET_ALIAS(clEnqueueReadImage);

cl_int CL_API_CALL clEnqueueWriteImage(cl_command_queue command_queue,
						   cl_mem			image,
						   cl_bool			blocking_write, 
						   const size_t	    origin[MAX_WORK_DIM],
						   const size_t	    region[MAX_WORK_DIM],
						   size_t			input_row_pitch,
						   size_t			input_slice_pitch, 
						   const void *		ptr,
						   cl_uint			num_events_in_wait_list,
						   const cl_event *	event_wait_list,
						   cl_event *		event)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueWriteImage(command_queue, image, blocking_write, origin, region, input_row_pitch, input_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event));
}
SET_ALIAS(clEnqueueWriteImage);

cl_int CL_API_CALL clEnqueueCopyImage(cl_command_queue	command_queue,
						  cl_mem			src_image,
						  cl_mem			dst_image, 
						  const size_t  	src_origin[MAX_WORK_DIM],
						  const size_t  	dst_origin[MAX_WORK_DIM],
						  const size_t  	region[MAX_WORK_DIM], 
						  cl_uint			num_events_in_wait_list,
						  const cl_event *	event_wait_list,
						  cl_event *		event)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueCopyImage(command_queue, src_image, dst_image, src_origin, dst_origin, region, num_events_in_wait_list, event_wait_list, event));
}
SET_ALIAS(clEnqueueCopyImage);

cl_int CL_API_CALL clEnqueueCopyImageToBuffer(cl_command_queue	command_queue,
								  cl_mem			src_image,
								  cl_mem			dst_buffer,
								  const size_t  	src_origin[MAX_WORK_DIM],
								  const size_t  	region[MAX_WORK_DIM],
								  size_t			dst_offset,
								  cl_uint			num_events_in_wait_list,
								  const cl_event *	event_wait_list,
								  cl_event *		event)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueCopyImageToBuffer(command_queue, src_image, dst_buffer, src_origin, region, dst_offset, num_events_in_wait_list, event_wait_list, event));
}
SET_ALIAS(clEnqueueCopyImageToBuffer);

cl_int CL_API_CALL clEnqueueCopyBufferToImage(cl_command_queue	command_queue,
								  cl_mem			src_buffer,
								  cl_mem			dst_image,
								  size_t			src_offset,
								  const size_t  	dst_origin[MAX_WORK_DIM],
								  const size_t  	region[MAX_WORK_DIM], 
								  cl_uint			num_events_in_wait_list,
								  const cl_event *	event_wait_list,
								  cl_event *		event)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueCopyBufferToImage(command_queue, src_buffer, dst_image, src_offset, dst_origin, region, num_events_in_wait_list, event_wait_list, event));
}
SET_ALIAS(clEnqueueCopyBufferToImage);

void * CL_API_CALL clEnqueueMapBuffer(cl_command_queue	command_queue,
						  cl_mem			buffer,
						  cl_bool			blocking_map,
						  cl_map_flags		map_flags,
						  size_t			offset,
						  size_t			cb,
						  cl_uint			num_events_in_wait_list,
						  const cl_event *	event_wait_list,
						  cl_event *		event,
						  cl_int *			errcode_ret)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, void *, EnqueueMapBuffer(command_queue, buffer, blocking_map, map_flags, offset, cb, num_events_in_wait_list, event_wait_list, event, errcode_ret));
}
SET_ALIAS(clEnqueueMapBuffer);

void * CL_API_CALL clEnqueueMapImage(cl_command_queue	command_queue,
						 cl_mem				image,
						 cl_bool			blocking_map,
						 cl_map_flags		map_flags,
						 const size_t 		origin[MAX_WORK_DIM],
						 const size_t 		region[MAX_WORK_DIM],
						 size_t *			image_row_pitch,
						 size_t *			image_slice_pitch,
						 cl_uint			num_events_in_wait_list,
						 const cl_event *	event_wait_list,
						 cl_event *			event,
						 cl_int *			errcode_ret)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, void *, EnqueueMapImage(command_queue, image, blocking_map, map_flags, origin, region, image_row_pitch, image_slice_pitch, num_events_in_wait_list, event_wait_list, event, errcode_ret));
}
SET_ALIAS(clEnqueueMapImage);

cl_int CL_API_CALL clEnqueueUnmapMemObject(cl_command_queue	command_queue,
							   cl_mem			memobj,
							   void *			mapped_ptr,
							   cl_uint			num_events_in_wait_list,
							   const cl_event * event_wait_list,
							   cl_event *		event)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueUnmapMemObject(command_queue, memobj, mapped_ptr, num_events_in_wait_list, event_wait_list, event));
}
SET_ALIAS(clEnqueueUnmapMemObject);

cl_int CL_API_CALL clEnqueueNDRangeKernel(cl_command_queue	command_queue,
							  cl_kernel			kernel,
							  cl_uint			work_dim,
							  const size_t *	global_work_offset,
							  const size_t *	global_work_size,
							  const size_t *	local_work_size,
							  cl_uint			num_events_in_wait_list,
							  const cl_event *	event_wait_list,
							  cl_event *		event)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueNDRangeKernel(command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event));
}
SET_ALIAS(clEnqueueNDRangeKernel);

cl_int CL_API_CALL clEnqueueTask(cl_command_queue	command_queue,
					 cl_kernel			kernel,
					 cl_uint			num_events_in_wait_list,
					 const cl_event *	event_wait_list,
					 cl_event *			event)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueTask(command_queue, kernel, num_events_in_wait_list, event_wait_list, event));
}
SET_ALIAS(clEnqueueTask);

cl_int CL_API_CALL clEnqueueNativeKernel(cl_command_queue	command_queue,
							 void (CL_CALLBACK *user_func)(void *), 
							 void *				args,
							 size_t				cb_args, 
							 cl_uint			num_mem_objects,
							 const cl_mem *		mem_list,
							 const void **		args_mem_loc,
							 cl_uint			num_events_in_wait_list,
							 const cl_event *	event_wait_list,
							 cl_event *			event)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueNativeKernel(command_queue, user_func, args, cb_args, num_mem_objects, mem_list, args_mem_loc, num_events_in_wait_list, event_wait_list, event));
}
SET_ALIAS(clEnqueueNativeKernel);

cl_int CL_API_CALL clEnqueueMarker(cl_command_queue command_queue, cl_event * event)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueMarker(command_queue, event));
}
SET_ALIAS(clEnqueueMarker);

cl_int CL_API_CALL clEnqueueWaitForEvents(cl_command_queue	command_queue,
							  cl_uint			num_events,
							  const cl_event *	event_list)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueWaitForEvents(command_queue, num_events, event_list));
}
SET_ALIAS(clEnqueueWaitForEvents);

cl_int CL_API_CALL clEnqueueBarrier(cl_command_queue command_queue)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueBarrier(command_queue));
}
SET_ALIAS(clEnqueueBarrier);

///////////////////////////////////////////////////////////////////////////////////////////////////
// Sharing Memory Objects with OpenGL / OpenGL ES Buffer, Texture and Renderbuffer Objects
///////////////////////////////////////////////////////////////////////////////////////////////////                              
SET_ALIAS(clCreateFromGLBuffer);
REGISTER_EXTENSION_FUNCTION(clCreateFromGLBuffer, clCreateFromGLBuffer);
cl_mem CL_API_CALL clCreateFromGLBuffer(cl_context   context,
							cl_mem_flags flags,
							GLuint       bufobj,
							int *        errcode_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_mem, CreateFromGLBuffer(context, flags, bufobj, errcode_ret));
}

SET_ALIAS(clCreateFromGLTexture2D);
REGISTER_EXTENSION_FUNCTION(clCreateFromGLTexture2D, clCreateFromGLTexture2D);
cl_mem CL_API_CALL clCreateFromGLTexture2D(cl_context   context,
							   cl_mem_flags flags,
							   GLenum       target,
							   GLint        miplevel,
							   GLuint       texture,
							   cl_int *     errcode_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_mem, CreateFromGLTexture2D(context, flags, target, miplevel, texture, errcode_ret));
}

SET_ALIAS(clCreateFromGLTexture3D);
REGISTER_EXTENSION_FUNCTION(clCreateFromGLTexture3D, clCreateFromGLTexture3D);
cl_mem CL_API_CALL clCreateFromGLTexture3D(cl_context   context,
							   cl_mem_flags flags,
							   GLenum       target,
							   GLint        miplevel,
							   GLuint       texture,
							   cl_int *     errcode_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_mem, CreateFromGLTexture3D(context, flags, target, miplevel, texture, errcode_ret));
}

SET_ALIAS(clCreateFromGLRenderbuffer);
REGISTER_EXTENSION_FUNCTION(clCreateFromGLRenderbuffer, clCreateFromGLRenderbuffer);
cl_mem CL_API_CALL clCreateFromGLRenderbuffer(cl_context   context,
								  cl_mem_flags flags,
								  GLuint       renderbuffer,
								  cl_int *     errcode_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_mem, CreateFromGLRenderbuffer(context, flags, renderbuffer, errcode_ret));
}

SET_ALIAS(clGetGLObjectInfo);
REGISTER_EXTENSION_FUNCTION(clGetGLObjectInfo, clGetGLObjectInfo);
cl_int CL_API_CALL clGetGLObjectInfo(cl_mem              memobj,
						 cl_gl_object_type * gl_object_type,
						 GLuint *            gl_object_name)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetGLObjectInfo(memobj, gl_object_type, gl_object_name));
}

SET_ALIAS(clGetGLTextureInfo);
REGISTER_EXTENSION_FUNCTION(clGetGLTextureInfo, clGetGLTextureInfo);
cl_int CL_API_CALL clGetGLTextureInfo(cl_mem             memobj,
						  cl_gl_texture_info param_name,
						  size_t             param_value_size,
						  void *             param_value,
						  size_t *           param_value_size_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetGLTextureInfo(memobj, param_name, param_value_size, param_value, param_value_size_ret));
}

SET_ALIAS(clEnqueueAcquireGLObjects);
REGISTER_EXTENSION_FUNCTION(clEnqueueAcquireGLObjects, clEnqueueAcquireGLObjects);
cl_int CL_API_CALL clEnqueueAcquireGLObjects(cl_command_queue command_queue,
								 cl_uint          num_objects,
								 const cl_mem *   mem_objects,
								 cl_uint          num_events_in_wait_list,
								 const cl_event * event_wait_list,
								 cl_event *       event)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueSyncGLObjects(command_queue, CL_COMMAND_ACQUIRE_GL_OBJECTS, num_objects, mem_objects, num_events_in_wait_list, event_wait_list, event));
}

SET_ALIAS(clEnqueueReleaseGLObjects);
REGISTER_EXTENSION_FUNCTION(clEnqueueReleaseGLObjects, clEnqueueReleaseGLObjects);
cl_int CL_API_CALL clEnqueueReleaseGLObjects(cl_command_queue command_queue,
								 cl_uint          num_objects,
								 const cl_mem *   mem_objects,
								 cl_uint          num_events_in_wait_list,
								 const cl_event * event_wait_list,
								 cl_event *       event)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, EnqueueSyncGLObjects(command_queue, CL_COMMAND_RELEASE_GL_OBJECTS, num_objects, mem_objects, num_events_in_wait_list, event_wait_list, event));
}

SET_ALIAS(clGetGLContextInfoKHR);
REGISTER_EXTENSION_FUNCTION(clGetGLContextInfoKHR, clGetGLContextInfoKHR);
cl_int CL_API_CALL clGetGLContextInfoKHR(const cl_context_properties * properties,
					  cl_gl_context_info            param_name,
					  size_t                        param_value_size,
					  void *                        param_value,
					  size_t *                      param_value_size_ret)
{
	CALL_INSTRUMENTED_API(PLATFORM_MODULE, cl_int, GetGLContextInfo(properties, param_name, param_value_size, param_value, param_value_size_ret));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// New OpenCL 1.1 functions
///////////////////////////////////////////////////////////////////////////////////////////////////                              

cl_event CL_API_CALL
clCreateUserEvent(cl_context    context,
				  cl_int *      errcode_ret)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_event, CreateUserEvent(context, errcode_ret));
}
SET_ALIAS(clCreateUserEvent);

cl_int CL_API_CALL
clSetEventCallback( cl_event    evt,
				   cl_int      command_exec_callback_type,
				   void (CL_CALLBACK *pfn_notify)(cl_event, cl_int, void *),
				   void *      user_data)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, SetEventCallback(evt, command_exec_callback_type, pfn_notify, user_data));
}
SET_ALIAS(clSetEventCallback);

cl_int CL_API_CALL
clSetUserEventStatus(cl_event   evt,
					 cl_int     execution_status)
{
	CALL_INSTRUMENTED_API(EXECUTION_MODULE, cl_int, SetUserEventStatus(evt, execution_status));
}
SET_ALIAS(clSetUserEventStatus);

// Check if the current CPU is supported. returns 0 if it does and 1 othrewise
// Criteria: supports SSSE3 and SSE4.1 and SSE4.2

int IsCPUSupported(void)
{
	if( CPUDetect::GetInstance()->IsFeatureSupported(CFS_SSE41) )
	{
		return 0;
	}
	return 1;
}

// check if the cpu feature is supported
// returns 0 is it does and 1 otherwise
int IsFeatureSupported(int iCPUFeature)
{
	if (CPUDetect::GetInstance()->IsFeatureSupported((Intel::OpenCL::Utils::ECPUFeatureSupport)iCPUFeature))
	{
		return 0;
	}
	return 1;
}

cl_int CL_API_CALL clRetainDeviceEXT(cl_device_id devId)
{
	CALL_INSTRUMENTED_API(PLATFORM_MODULE, cl_int, clRetainDevice(devId));
}
SET_ALIAS(clRetainDeviceEXT);
REGISTER_EXTENSION_FUNCTION(clRetainDeviceEXT, clRetainDeviceEXT);

cl_int CL_API_CALL clReleaseDeviceEXT(cl_device_id device)
{
	CALL_INSTRUMENTED_API(PLATFORM_MODULE, cl_int, clReleaseDevice(device));
}
SET_ALIAS(clReleaseDeviceEXT);
REGISTER_EXTENSION_FUNCTION(clReleaseDeviceEXT, clReleaseDeviceEXT);

cl_int CL_API_CALL clCreateSubDevicesEXT(cl_device_id device,
										 const cl_device_partition_property_ext* properties,
										 cl_uint num_entries,
										 cl_device_id* out_devices,
										 cl_uint* num_devices)
{
	CALL_INSTRUMENTED_API(PLATFORM_MODULE, cl_int, clCreateSubDevices(device, properties, num_entries, out_devices, num_devices));
}
SET_ALIAS(clCreateSubDevicesEXT);
REGISTER_EXTENSION_FUNCTION(clCreateSubDevicesEXT, clCreateSubDevicesEXT);

#if defined (DX9_SHARING)
cl_mem CL_API_CALL clCreateFromD3D9VertexBufferINTEL(cl_context context,
                                                     cl_mem_flags flags,
                                                     IDirect3DVertexBuffer9 *resource,
                                                     cl_int *errcode_ret)
{
    return CONTEXT_MODULE->CreateFromD3D9VertexBuffer(context, flags, resource, errcode_ret);
}
REGISTER_EXTENSION_FUNCTION(clCreateFromD3D9VertexBufferINTEL, clCreateFromD3D9VertexBufferINTEL);

cl_mem CL_API_CALL clCreateFromD3D9IndexBufferINTEL(cl_context context,
                                                    cl_mem_flags flags,
                                                    IDirect3DIndexBuffer9 *resource,
                                                    cl_int *errcode_ret)
{
    return CONTEXT_MODULE->CreateFromD3D9IndexBuffer(context, flags, resource, errcode_ret);
}
REGISTER_EXTENSION_FUNCTION(clCreateFromD3D9IndexBufferINTEL, clCreateFromD3D9IndexBufferINTEL);

cl_mem CL_API_CALL clCreateFromD3D9TextureINTEL(cl_context context,
                                                cl_mem_flags flags,
                                                IDirect3DTexture9 *resource,
                                                UINT miplevel,
                                                cl_int *errcode_ret)
{
    return CONTEXT_MODULE->CreateFromD3D9Texture(context, flags, resource, miplevel, errcode_ret);
}
REGISTER_EXTENSION_FUNCTION(clCreateFromD3D9TextureINTEL, clCreateFromD3D9TextureINTEL);

cl_mem CL_API_CALL clCreateFromD3D9CubeTextureINTEL(cl_context context,
                                                    cl_mem_flags flags,
                                                    IDirect3DCubeTexture9 *resource,
                                                    D3DCUBEMAP_FACES facetype,
                                                    UINT miplevel,
                                                    cl_int *errcode_ret)
{
    return CONTEXT_MODULE->CreateFromD3D9CubeTexture(context, flags, resource, facetype, miplevel, errcode_ret);
}
REGISTER_EXTENSION_FUNCTION(clCreateFromD3D9CubeTextureINTEL, clCreateFromD3D9CubeTextureINTEL);

cl_mem CL_API_CALL clCreateFromD3D9VolumeTextureINTEL(cl_context context,
                                                      cl_mem_flags flags,
                                                      IDirect3DVolumeTexture9 *resource,
                                                      UINT miplevel,
                                                      cl_int *errcode_ret)
{
    return CONTEXT_MODULE->CreateFromD3D9VolumeTexture(context, flags, resource, miplevel, errcode_ret);
}
REGISTER_EXTENSION_FUNCTION(clCreateFromD3D9VolumeTextureINTEL, clCreateFromD3D9VolumeTextureINTEL);
#endif

#if defined (DX9_MEDIA_SHARING)

cl_mem CL_API_CALL clCreateFromDX9MediaSurfaceINTEL(cl_context context,
                                                cl_mem_flags flags,
                                                IDirect3DSurface9 *resource,
                                                HANDLE sharehandle,
                                                UINT plane,
                                                cl_int *errcode_ret)
{
    return CONTEXT_MODULE->CreateFromD3D9Surface(context, flags, resource, sharehandle, plane, errcode_ret);
}
REGISTER_EXTENSION_FUNCTION(clCreateFromDX9MediaSurfaceINTEL, clCreateFromDX9MediaSurfaceINTEL);

cl_int CL_API_CALL clEnqueueAcquireDX9ObjectsINTEL(cl_command_queue command_queue,
                                                 cl_uint num_objects,
                                                 const cl_mem *mem_objects,
                                                 cl_uint num_events_in_wait_list,
                                                 const cl_event *event_wait_list,
                                                 cl_event *event)
{
    return EXECUTION_MODULE->EnqueueSyncD3D9Objects(command_queue,
        CL_COMMAND_ACQUIRE_DX9_OBJECTS_INTEL, num_objects, mem_objects, num_events_in_wait_list,
        event_wait_list, event);
}
REGISTER_EXTENSION_FUNCTION(clEnqueueAcquireDX9ObjectsINTEL, clEnqueueAcquireDX9ObjectsINTEL);

cl_int CL_API_CALL clEnqueueReleaseDX9ObjectsINTEL(cl_command_queue command_queue,
                                                 cl_uint num_objects,
                                                 cl_mem *mem_objects,
                                                 cl_uint num_events_in_wait_list,
                                                 const cl_event *event_wait_list,
                                                 cl_event *event)
{
    return EXECUTION_MODULE->EnqueueSyncD3D9Objects(command_queue,
        CL_COMMAND_RELEASE_DX9_OBJECTS_INTEL, num_objects, mem_objects, num_events_in_wait_list,
        event_wait_list, event);
}
REGISTER_EXTENSION_FUNCTION(clEnqueueReleaseDX9ObjectsINTEL, clEnqueueReleaseDX9ObjectsINTEL);

cl_int CL_API_CALL clGetDeviceIDsFromDX9INTEL(cl_platform_id platform,
                                            cl_dx9_device_source_intel d3d_device_source,
                                            void *d3d_object,
                                            cl_dx9_device_set_intel d3d_device_set,
                                            cl_uint num_entries, 
                                            cl_device_id *devices, 
                                            cl_uint *num_devices)
{
    return PLATFORM_MODULE->GetDeviceIDsFromD3D9(platform, d3d_device_source, d3d_object, d3d_device_set, num_entries, devices, num_devices);
}
REGISTER_EXTENSION_FUNCTION(clGetDeviceIDsFromDX9INTEL, clGetDeviceIDsFromDX9INTEL);

#else
// dummy definitions
#ifdef DX9_SHARING
void CL_API_CALL clCreateFromD3D9VertexBufferINTEL() { }
void CL_API_CALL clCreateFromD3D9IndexBufferINTEL() { }
void CL_API_CALL clCreateFromD3D9TextureINTEL() { }
void CL_API_CALL clCreateFromD3D9CubeTextureINTEL() { }
void CL_API_CALL clCreateFromD3D9VolumeTextureINTEL() { }
#endif
void CL_API_CALL clCreateFromDX9MediaSurfaceINTEL() { }
void CL_API_CALL clEnqueueAcquireDX9ObjectsINTEL() { }
void CL_API_CALL clEnqueueReleaseDX9ObjectsINTEL() { }
void CL_API_CALL clGetDeviceIDsFromDX9INTEL() { }
#endif

///////////////////////////////////////////////////////////////////////////////////////////
// OpenCL 1.2 functions
///////////////////////////////////////////////////////////////////////////////////////////
cl_int CL_API_CALL clGetKernelArgInfo(cl_kernel		kernel,
								cl_uint				arg_indx,
								cl_kernel_arg_info	param_name,
								size_t				param_value_size,
								void *				param_value,
								size_t *			param_value_size_ret)
{
	CALL_INSTRUMENTED_API(CONTEXT_MODULE, cl_int, GetKernelArgInfo(kernel, arg_indx, param_name, param_value_size, param_value, param_value_size_ret));
}
SET_ALIAS(clGetKernelArgInfo);

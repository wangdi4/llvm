// OpenCL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "cl_types.h"
#include "cl_objects_map.h"
#include "framework_proxy.h"

using namespace Intel::OpenCL::Framework;

#define PLATFORM_MODULE		FrameworkProxy::Instance()->GetPlatformModule()

#define CONTEXT_MODULE		FrameworkProxy::Instance()->GetContextModule()

#define EXECUTION_MODULE	FrameworkProxy::Instance()->GetExecutionModule()

///////////////////////////////////////////////////////////////////////////////////////////////////
// Platform APIs
///////////////////////////////////////////////////////////////////////////////////////////////////

cl_int clGetPlatformInfo(cl_platform_info param_name, 
						 size_t param_value_size, 
						 void* param_value, 
						 size_t* param_value_size_ret)
{
	// get instance of the framework factory class
	//FrameworkProxy* pFramework = FrameworkProxy::Instance();
	// get the platform module
	//PlatformModule *pPlatformModule = pFramework->GetPlatformModule();
	return PLATFORM_MODULE->GetPlatformInfo(param_name, param_value_size, param_value, param_value_size_ret);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Device APIs
///////////////////////////////////////////////////////////////////////////////////////////////////

cl_int clGetDeviceIDs(cl_device_type device_type, 
					  cl_uint num_entries, 
					  cl_device_id* devices, 
			          cl_uint* num_devices)
{
	return PLATFORM_MODULE->GetDeviceIDs(device_type, num_entries, devices, num_devices);
}

cl_int clGetDeviceInfo(cl_device_id device,
					   cl_device_info param_name, 
					   size_t param_value_size, 
					   void* param_value,
					   size_t* param_value_size_ret)
{
	return PLATFORM_MODULE->GetDeviceInfo(device, param_name, param_value_size, param_value, param_value_size_ret);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context APIs
///////////////////////////////////////////////////////////////////////////////////////////////////

cl_context clCreateContext(cl_context_properties properties,
					   cl_uint num_devices,
					   const cl_device_id * devices,
					   logging_fn pfn_notify,
					   void * user_data,
					   cl_int * errcode_ret)
{
	return CONTEXT_MODULE->CreateContext(properties, num_devices, devices, pfn_notify, user_data, errcode_ret);
}

cl_context clCreateContextFromType(cl_context_properties properties,
								   cl_device_type        device_type,
								   logging_fn            pfn_notify,
								   void *                user_data,
								   cl_int *              errcode_ret)
{
	return CONTEXT_MODULE->CreateContextFromType(properties, device_type, pfn_notify, user_data, errcode_ret);
}

cl_int clRetainContext(cl_context context)
{
	return CONTEXT_MODULE->RetainContext(context);
}

cl_int clReleaseContext(cl_context context)
{
	return CONTEXT_MODULE->ReleaseContext(context);
}

cl_int clGetContextInfo(cl_context      context,
						cl_context_info param_name,
						size_t          param_value_size,
						void *          param_value,
						size_t *        param_value_size_ret)
{
	return CONTEXT_MODULE->GetContextInfo(context, param_name, param_value_size, param_value, param_value_size_ret);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Command Queue APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_command_queue clCreateCommandQueue(cl_context                  context, 
									  cl_device_id                device, 
									  cl_command_queue_properties properties, 
									  cl_int *                    errcode_ret)
{
	return EXECUTION_MODULE->CreateCommandQueue(context, device, properties, errcode_ret);
}
cl_int clRetainCommandQueue(cl_command_queue command_queue)
{
	return EXECUTION_MODULE->RetainCommandQueue(command_queue);
}
cl_int clReleaseCommandQueue(cl_command_queue command_queue)
{
	return EXECUTION_MODULE->ReleaseCommandQueue(command_queue);
}
cl_int clGetCommandQueueInfo(cl_command_queue      command_queue, 
							 cl_command_queue_info param_name, 
							 size_t                param_value_size, 
							 void *                param_value, 
							 size_t *              param_value_size_ret)
{
	return EXECUTION_MODULE->GetCommandQueueInfo(command_queue, param_name, param_value_size, param_value, param_value_size_ret);
}
cl_int clSetCommandQueueProperty(cl_command_queue              command_queue, 
								 cl_command_queue_properties   properties, 
								 cl_int                        enable, 
								 cl_command_queue_properties * old_properties)
{
	return EXECUTION_MODULE->SetCommandQueueProperty(command_queue, properties, enable, old_properties);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Memory Object APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_mem clCreateBuffer(cl_context   context, 
					  cl_mem_flags flags, 
					  size_t       size, 
					  void *       host_ptr, 
					  cl_int *     errcode_ret)
{
	return CONTEXT_MODULE->CreateBuffer(context, flags, size, host_ptr, errcode_ret);
}

cl_mem clCreateImage2D(cl_context              context,
					   cl_mem_flags            flags,
					   const cl_image_format * image_format,
					   size_t                  image_width,
					   size_t                  image_height,
					   size_t                  image_row_pitch,
					   void *                  host_ptr,
					   cl_int *                errcode_ret)
{
	return CONTEXT_MODULE->CreateImage2D(context, flags, image_format, image_width, image_height, image_row_pitch, host_ptr, errcode_ret);
}
                        
cl_mem clCreateImage3D(cl_context              context,
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
	return CONTEXT_MODULE->CreateImage3D(context, flags, image_format, image_width, image_height, image_depth, image_row_pitch, image_slice_pitch, host_ptr, errcode_ret);
}
                        
cl_int clRetainMemObject(cl_mem memobj)
{
	return CONTEXT_MODULE->RetainMemObject(memobj);
}

cl_int clReleaseMemObject(cl_mem memobj)
{
	return CONTEXT_MODULE->ReleaseMemObject(memobj);
}

cl_int clGetSupportedImageFormats(cl_context           context,
								  cl_mem_flags         flags,
								  cl_mem_object_type   image_type,
								  cl_uint              num_entries,
								  cl_image_format *    image_formats,
								  cl_uint *            num_image_formats)
{
	return CONTEXT_MODULE->GetSupportedImageFormats(context, flags, image_type, num_entries, image_formats, num_image_formats);
}
                                    
cl_int clGetMemObjectInfo(cl_mem           memobj,
						  cl_mem_info      param_name, 
						  size_t           param_value_size,
						  void *           param_value,
						  size_t *         param_value_size_ret)
{
	return CONTEXT_MODULE->GetMemObjectInfo(memobj, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_int clGetImageInfo(cl_mem           image,
					  cl_image_info    param_name, 
					  size_t           param_value_size,
					  void *           param_value,
					  size_t *         param_value_size_ret)
{
	return CONTEXT_MODULE->GetImageInfo(image, param_name, param_value_size, param_value, param_value_size_ret);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Sampler APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_sampler clCreateSampler(cl_context			context,
						   cl_bool				normalized_coords,
						   cl_addressing_mode	addressing_mode,
						   cl_filter_mode		filter_mode,
						   cl_int *				errcode_ret)
{
		return CONTEXT_MODULE->CreateSampler(context, normalized_coords, addressing_mode, filter_mode, errcode_ret);
}
cl_int clRetainSampler(cl_sampler sampler)
{
	return CONTEXT_MODULE->RetainSampler(sampler);
}

cl_int clReleaseSampler(cl_sampler sampler)
{
	return CONTEXT_MODULE->ReleaseSampler(sampler);
}

cl_int clGetSamplerInfo(cl_sampler		sampler,
						cl_sampler_info	param_name,
						size_t			param_value_size,
						void *			param_value,
						size_t *		param_value_size_ret)
{
	return CONTEXT_MODULE->GetSamplerInfo(sampler, param_name, param_value_size, param_value, param_value_size_ret);
}
                            
///////////////////////////////////////////////////////////////////////////////////////////////////
// Program Object APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_program clCreateProgramWithSource(cl_context     context,
									 cl_uint        count,
									 const char **  strings,
									 const size_t * lengths,
									 cl_int *       errcode_ret)
{
	return CONTEXT_MODULE->CreateProgramWithSource(context, count, strings, lengths, errcode_ret);
}

cl_program clCreateProgramWithBinary(cl_context           context,
									 cl_uint              num_devices,
									 const cl_device_id * device_list,
									 const size_t *       lengths,
									 const void **        binaries,
									 cl_int *             binary_status,
									 cl_int *             errcode_ret)
{
	return CONTEXT_MODULE->CreateProgramWithBinary(context, num_devices, device_list, lengths, binaries, binary_status, errcode_ret);
}

cl_int clRetainProgram(cl_program program)
{
	return CONTEXT_MODULE->RetainProgram(program);
}

cl_int clReleaseProgram(cl_program program)
{
	return CONTEXT_MODULE->ReleaseProgram(program);
}

cl_int clBuildProgram(cl_program           program,
					  cl_uint              num_devices,
					  const cl_device_id * device_list,
					  const char *         options, 
					  void (*pfn_notify)(cl_program program, void * user_data),
					  void *               user_data)
{
	return CONTEXT_MODULE->BuildProgram(program, num_devices, device_list, options, pfn_notify, user_data);
}

cl_int clUnloadCompiler(void)
{
	return CONTEXT_MODULE->UnloadCompiler();
}

cl_int clGetProgramInfo(cl_program      program,
						cl_program_info param_name,
						size_t          param_value_size,
						void *          param_value,
						size_t *        param_value_size_ret)
{
	return CONTEXT_MODULE->GetProgramInfo(program, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_int clGetProgramBuildInfo(cl_program            program,
							 cl_device_id          device,
							 cl_program_build_info param_name,
							 size_t                param_value_size,
							 void *                param_value,
							 size_t *              param_value_size_ret)
{
	return CONTEXT_MODULE->GetProgramBuildInfo(program, device, param_name, param_value_size, param_value, param_value_size_ret);
}
                           
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel Object APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_kernel clCreateKernel(cl_program   program,
						 const char * kernel_name,
						 cl_int *     errcode_ret)
{
	return CONTEXT_MODULE->CreateKernel(program, kernel_name, errcode_ret);
}

cl_int clCreateKernelsInProgram(cl_program  program,
								cl_uint     num_kernels,
								cl_kernel * kernels,
								cl_uint *   num_kernels_ret)
{
	return CONTEXT_MODULE->CreateKernelsInProgram(program, num_kernels, kernels, num_kernels_ret);
}

cl_int clRetainKernel(cl_kernel kernel)
{
	return CONTEXT_MODULE->RetainKernel(kernel);
}

cl_int clReleaseKernel(cl_kernel kernel)
{
	return CONTEXT_MODULE->ReleaseKernel(kernel);
}

cl_int clSetKernelArg(cl_kernel    kernel,
					  cl_uint      arg_indx,
					  size_t       arg_size,
					  const void * arg_value)
{
	return CONTEXT_MODULE->SetKernelArg(kernel, arg_indx, arg_size, arg_value);
}

cl_int clGetKernelInfo(cl_kernel      kernel,
					   cl_kernel_info param_name,
					   size_t         param_value_size,
					   void *         param_value,
					   size_t *       param_value_size_ret)
{
	return CONTEXT_MODULE->GetKernelInfo(kernel, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_int clGetKernelWorkGroupInfo(cl_kernel                 kernel,
								cl_device_id              device,
								cl_kernel_work_group_info param_name,
								size_t                    param_value_size,
								void *                    param_value,
								size_t *                  param_value_size_ret)
{
	return CONTEXT_MODULE->GetKernelWorkGroupInfo(kernel, device, param_name, param_value_size, param_value, param_value_size_ret);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Event Object APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_int clWaitForEvents(cl_uint num_events, const cl_event * event_list)
{
	return EXECUTION_MODULE->WaitForEvents(num_events, event_list);
}
cl_int clGetEventInfo(cl_event		event,
					  cl_event_info	param_name,
					  size_t		param_value_size,
					  void *		param_value,
					  size_t *		param_value_size_ret)
{
	return EXECUTION_MODULE->GetEventInfo(event, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_int clRetainEvent(cl_event event)
{
	return EXECUTION_MODULE->RetainEvent(event);
}

cl_int clReleaseEvent(cl_event event)
{
	return EXECUTION_MODULE->ReleaseEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Profiling APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_int clGetEventProfilingInfo(cl_event				event,
							   cl_profiling_info	param_name,
							   size_t				param_value_size,
							   void *				param_value,
							   size_t *				param_value_size_ret)
{
	//EXECUTION_MODULE->GetEventProfilingInfo(event, param_name, param_value_size, param_value, param_value_size_ret);
	return CL_ERR_NOT_IMPLEMENTED;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Flush and Finish APIs
///////////////////////////////////////////////////////////////////////////////////////////////////                              
cl_int clFlush(cl_command_queue command_queue)
{
	//EXECUTION_MODULE->Flush(command_queue);
	return CL_ERR_NOT_IMPLEMENTED;
}
cl_int clFinish(cl_command_queue command_queue)
{
	//EXECUTION_MODULE->Finish(command_queue);
	return CL_ERR_NOT_IMPLEMENTED;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Enqueued Commands APIs
///////////////////////////////////////////////////////////////////////////////////////////////////                              
cl_int clEnqueueReadBuffer(cl_command_queue	command_queue,
						   cl_mem			buffer,
						   cl_bool			blocking_read,
						   size_t			offset,
						   size_t			cb,
						   void *			ptr,
						   cl_uint			num_events_in_wait_list,
						   const cl_event * event_wait_list,
						   cl_event *		event)
{
	return EXECUTION_MODULE->EnqueueReadBuffer(command_queue, buffer, blocking_read, offset, cb, ptr, num_events_in_wait_list, event_wait_list, event);
}
cl_int clEnqueueWriteBuffer(cl_command_queue	command_queue,
							cl_mem				buffer,
							cl_bool				blocking_write,
							size_t				offset,
							size_t				cb,
							const void *		ptr,
							cl_uint				num_events_in_wait_list,
							const cl_event *	event_wait_list,
							cl_event *			event)
{
	return EXECUTION_MODULE->EnqueueWriteBuffer(command_queue, buffer, blocking_write, offset, cb, ptr, num_events_in_wait_list, event_wait_list, event);
}
cl_int clEnqueueCopyBuffer(cl_command_queue	command_queue,
						   cl_mem			src_buffer,
						   cl_mem			dst_buffer,
						   size_t			src_offset,
						   size_t			dst_offset,
						   size_t			cb,
						   cl_uint			num_events_in_wait_list,
						   const cl_event * event_wait_list,
						   cl_event *		event)
{
	return EXECUTION_MODULE->EnqueueCopyBuffer(command_queue, src_buffer, dst_buffer, src_offset, dst_offset, cb, num_events_in_wait_list, event_wait_list, event);
}
                            
cl_int clEnqueueReadImage(cl_command_queue command_queue,
						  cl_mem			image,
						  cl_bool			blocking_read, 
						  const size_t *	origin[3],
						  const size_t *	region[3],
						  size_t			row_pitch,
						  size_t			slice_pitch, 
						  void *			ptr,
						  cl_uint			num_events_in_wait_list,
						  const cl_event *	event_wait_list,
						  cl_event *		event)
{
	return EXECUTION_MODULE->EnqueueReadImage(command_queue, image, blocking_read, origin, region, row_pitch, slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
}

cl_int clEnqueueWriteImage(cl_command_queue command_queue,
						   cl_mem			image,
						   cl_bool			blocking_write, 
						   const size_t *	origin[3],
						   const size_t *	region[3],
						   size_t			input_row_pitch,
						   size_t			input_slice_pitch, 
						   const void *		ptr,
						   cl_uint			num_events_in_wait_list,
						   const cl_event *	event_wait_list,
						   cl_event *		event)
{
	return EXECUTION_MODULE->EnqueueWriteImage(command_queue, image, blocking_write, origin, region, input_row_pitch, input_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
}

cl_int clEnqueueCopyImage(cl_command_queue	command_queue,
						  cl_mem			src_image,
						  cl_mem			dst_image, 
						  const size_t *	src_origin[3],
						  const size_t *	dst_origin[3],
						  const size_t *	region[3], 
						  cl_uint			num_events_in_wait_list,
						  const cl_event *	event_wait_list,
						  cl_event *		event)
{
	return EXECUTION_MODULE->EnqueueCopyImage(command_queue, src_image, dst_image, src_origin, dst_origin, region, num_events_in_wait_list, event_wait_list, event);
}

cl_int clEnqueueCopyImageToBuffer(cl_command_queue	command_queue,
								  cl_mem			src_image,
								  cl_mem			dst_buffer,
								  const size_t *	src_origin[3],
								  const size_t *	region[3],
								  size_t			dst_offset,
								  cl_uint			num_events_in_wait_list,
								  const cl_event *	event_wait_list,
								  cl_event *		event)
{
	return EXECUTION_MODULE->EnqueueCopyImageToBuffer(command_queue, src_image, dst_buffer, src_origin, region, dst_offset, num_events_in_wait_list, event_wait_list, event);
}

cl_int clEnqueueCopyBufferToImage(cl_command_queue	command_queue,
								  cl_mem			src_buffer,
								  cl_mem			dst_image,
								  size_t			src_offset,
								  const size_t *	dst_origin[3],
								  const size_t *	region[3], 
								  cl_uint			num_events_in_wait_list,
								  const cl_event *	event_wait_list,
								  cl_event *		event)
{
	return EXECUTION_MODULE->EnqueueCopyBufferToImage(command_queue, src_buffer, dst_image, src_offset, dst_origin, region, num_events_in_wait_list, event_wait_list, event);
}
void * clEnqueueMapBuffer(cl_command_queue	command_queue,
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
	return EXECUTION_MODULE->EnqueueMapBuffer(command_queue, buffer, blocking_map, map_flags, offset, cb, num_events_in_wait_list, event_wait_list, event, errcode_ret);
}

void * clEnqueueMapImage(cl_command_queue	command_queue,
						 cl_mem				image,
						 cl_bool			blocking_map,
						 cl_map_flags		map_flags,
						 const size_t *		origin[3],
						 const size_t *		region[3],
						 size_t *			image_row_pitch,
						 size_t *			image_slice_pitch,
						 cl_uint			num_events_in_wait_list,
						 const cl_event *	event_wait_list,
						 cl_event *			event,
						 cl_int *			errcode_ret)
{
	return EXECUTION_MODULE->EnqueueMapImage(command_queue, image, blocking_map, map_flags, origin, region, image_row_pitch, image_slice_pitch, num_events_in_wait_list, event_wait_list, event, errcode_ret);
}

cl_int clEnqueueUnmapMemObject(cl_command_queue	command_queue,
							   cl_mem			memobj,
							   void *			mapped_ptr,
							   cl_uint			num_events_in_wait_list,
							   const cl_event * event_wait_list,
							   cl_event *		event)
{
	return EXECUTION_MODULE->EnqueueUnmapMemObject(command_queue, memobj, mapped_ptr, num_events_in_wait_list, event_wait_list, event);
}

cl_int clEnqueueNDRangeKernel(cl_command_queue	command_queue,
							  cl_kernel			kernel,
							  cl_uint			work_dim,
							  const size_t *	global_work_offset,
							  const size_t *	global_work_size,
							  const size_t *	local_work_size,
							  cl_uint			num_events_in_wait_list,
							  const cl_event *	event_wait_list,
							  cl_event *		event)
{
	return EXECUTION_MODULE->EnqueueNDRangeKernel(command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event);
}

cl_int clEnqueueTask(cl_command_queue	command_queue,
					 cl_kernel			kernel,
					 cl_uint			num_events_in_wait_list,
					 const cl_event *	event_wait_list,
					 cl_event *			event)
{
	return EXECUTION_MODULE->EnqueueTask(command_queue, kernel, num_events_in_wait_list, event_wait_list, event);
}

cl_int clEnqueueNativeKernel(cl_command_queue	command_queue,
							 void (*user_func)(void *), 
							 void *				args,
							 size_t				cb_args, 
							 cl_uint			num_mem_objects,
							 const cl_mem *		mem_list,
							 const void **		args_mem_loc,
							 cl_uint			num_events_in_wait_list,
							 const cl_event *	event_wait_list,
							 cl_event *			event)
{
	return EXECUTION_MODULE->EnqueueNativeFnAsKernel(command_queue, user_func, args, cb_args, num_mem_objects, mem_list, args_mem_loc, num_events_in_wait_list, event_wait_list, event);
}

cl_int clEnqueueMarker(cl_command_queue command_queue, cl_event * event)
{
	return EXECUTION_MODULE->EnqueueMarker(command_queue, event);
}

cl_int clEnqueueWaitForEvents(cl_command_queue	command_queue,
							  cl_uint			num_events,
							  const cl_event *	event_list)
{
	return EXECUTION_MODULE->EnqueueWaitForEvents(command_queue, num_events, event_list);
}

cl_int clEnqueueBarrier(cl_command_queue command_queue)
{
	return EXECUTION_MODULE->EnqueueBarrier(command_queue);
}
// OpenCL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "cl_framework.h"
#include "cl_objects_map.h"
#include "framework_proxy.h"

using namespace Intel::OpenCL::Framework;

#define PLATFORM_MODULE		FrameworkProxy::Instance()->GetPlatformModule()

#define CONTEXT_MODULE		FrameworkProxy::Instance()->GetContextModule()

#define EXECUTION_MODULE	FrameworkProxy::Instance()->GetExecutionModule()

void * CL_API_CALL clGetExtensionFunctionAddress(const char *funcname)
{
	if (!strcmp(funcname, "clIcdGetPlatformIDsKHR"))
	{
		return clGetPlatformIDs;
	}
	return NULL;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Platform APIs
///////////////////////////////////////////////////////////////////////////////////////////////////

cl_int CL_API_CALL clGetPlatformIDs(cl_uint num_entries, cl_platform_id * platforms, cl_uint * num_platforms)
{
	return PLATFORM_MODULE->GetPlatformIDs(num_entries, platforms, num_platforms);
}

cl_int CL_API_CALL clGetPlatformInfo(cl_platform_id platform,
						 cl_platform_info param_name, 
						 size_t param_value_size, 
						 void* param_value, 
						 size_t* param_value_size_ret)
{
	return PLATFORM_MODULE->GetPlatformInfo(platform, param_name, param_value_size, param_value, param_value_size_ret);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Device APIs
///////////////////////////////////////////////////////////////////////////////////////////////////

cl_int CL_API_CALL clGetDeviceIDs(cl_platform_id platform,
					  cl_device_type device_type, 
					  cl_uint num_entries, 
					  cl_device_id* devices, 
			          cl_uint* num_devices)
{
	return PLATFORM_MODULE->GetDeviceIDs(platform, device_type, num_entries, devices, num_devices);
}

cl_int CL_API_CALL clGetDeviceInfo(cl_device_id device,
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

cl_context CL_API_CALL clCreateContext(const cl_context_properties * properties,
						   cl_uint num_devices,
						   const cl_device_id * devices,
						   logging_fn pfn_notify,
						   void * user_data,
						   cl_int * errcode_ret)
{
	return CONTEXT_MODULE->CreateContext(properties, num_devices, devices, pfn_notify, user_data, errcode_ret);
}

cl_context CL_API_CALL clCreateContextFromType(const cl_context_properties * properties,
								   cl_device_type          device_type,
								   logging_fn              pfn_notify,
								   void *                  user_data,
								   cl_int *                errcode_ret)
{
	return CONTEXT_MODULE->CreateContextFromType(properties, device_type, pfn_notify, user_data, errcode_ret);
}

cl_int CL_API_CALL clRetainContext(cl_context context)
{
	return CONTEXT_MODULE->RetainContext(context);
}

cl_int CL_API_CALL clReleaseContext(cl_context context)
{
	return CONTEXT_MODULE->ReleaseContext(context);
}

cl_int CL_API_CALL clGetContextInfo(cl_context      context,
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
cl_command_queue CL_API_CALL clCreateCommandQueue(cl_context                  context, 
									  cl_device_id                device, 
									  cl_command_queue_properties properties, 
									  cl_int *                    errcode_ret)
{
	return EXECUTION_MODULE->CreateCommandQueue(context, device, properties, errcode_ret);
}
cl_int CL_API_CALL clRetainCommandQueue(cl_command_queue command_queue)
{
	return EXECUTION_MODULE->RetainCommandQueue(command_queue);
}
cl_int CL_API_CALL clReleaseCommandQueue(cl_command_queue command_queue)
{
	return EXECUTION_MODULE->ReleaseCommandQueue(command_queue);
}
cl_int CL_API_CALL clGetCommandQueueInfo(cl_command_queue      command_queue, 
							 cl_command_queue_info param_name, 
							 size_t                param_value_size, 
							 void *                param_value, 
							 size_t *              param_value_size_ret)
{
	return EXECUTION_MODULE->GetCommandQueueInfo(command_queue, param_name, param_value_size, param_value, param_value_size_ret);
}
cl_int CL_API_CALL clSetCommandQueueProperty(cl_command_queue              command_queue,
								 cl_command_queue_properties   properties,
								 cl_bool                       enable,
								 cl_command_queue_properties * old_properties)
{
	return EXECUTION_MODULE->SetCommandQueueProperty(command_queue, properties, enable, old_properties);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Memory Object APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_mem CL_API_CALL clCreateBuffer(cl_context   context, 
					  cl_mem_flags flags, 
					  size_t       size, 
					  void *       host_ptr, 
					  cl_int *     errcode_ret)
{
	return CONTEXT_MODULE->CreateBuffer(context, flags, size, host_ptr, errcode_ret);
}

cl_mem CL_API_CALL clCreateImage2D(cl_context              context,
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
	return CONTEXT_MODULE->CreateImage3D(context, flags, image_format, image_width, image_height, image_depth, image_row_pitch, image_slice_pitch, host_ptr, errcode_ret);
}
                        
cl_int CL_API_CALL clRetainMemObject(cl_mem memobj)
{
	return CONTEXT_MODULE->RetainMemObject(memobj);
}

cl_int CL_API_CALL clReleaseMemObject(cl_mem memobj)
{
	return CONTEXT_MODULE->ReleaseMemObject(memobj);
}

cl_int CL_API_CALL clGetSupportedImageFormats(cl_context           context,
								  cl_mem_flags         flags,
								  cl_mem_object_type   image_type,
								  cl_uint              num_entries,
								  cl_image_format *    image_formats,
								  cl_uint *            num_image_formats)
{
	return CONTEXT_MODULE->GetSupportedImageFormats(context, flags, image_type, num_entries, image_formats, num_image_formats);
}
                                    
cl_int CL_API_CALL clGetMemObjectInfo(cl_mem           memobj,
						  cl_mem_info      param_name, 
						  size_t           param_value_size,
						  void *           param_value,
						  size_t *         param_value_size_ret)
{
	return CONTEXT_MODULE->GetMemObjectInfo(memobj, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_int CL_API_CALL clGetImageInfo(cl_mem           image,
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
cl_sampler CL_API_CALL clCreateSampler(cl_context			context,
						   cl_bool				normalized_coords,
						   cl_addressing_mode	addressing_mode,
						   cl_filter_mode		filter_mode,
						   cl_int *				errcode_ret)
{
		return CONTEXT_MODULE->CreateSampler(context, normalized_coords, addressing_mode, filter_mode, errcode_ret);
}
cl_int CL_API_CALL clRetainSampler(cl_sampler sampler)
{
	return CONTEXT_MODULE->RetainSampler(sampler);
}

cl_int CL_API_CALL clReleaseSampler(cl_sampler sampler)
{
	return CONTEXT_MODULE->ReleaseSampler(sampler);
}

cl_int CL_API_CALL clGetSamplerInfo(cl_sampler		sampler,
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
cl_program CL_API_CALL clCreateProgramWithSource(cl_context     context,
									 cl_uint        count,
									 const char **  strings,
									 const size_t * lengths,
									 cl_int *       errcode_ret)
{
	return CONTEXT_MODULE->CreateProgramWithSource(context, count, strings, lengths, errcode_ret);
}

cl_program CL_API_CALL clCreateProgramWithBinary(cl_context           context,
									 cl_uint              num_devices,
									 const cl_device_id *	device_list,
									 const size_t *			lengths,
									 const unsigned char **	binaries,
									 cl_int *				binary_status,
									 cl_int *				errcode_ret)
{
	return CONTEXT_MODULE->CreateProgramWithBinary(context, num_devices, device_list, lengths, binaries, binary_status, errcode_ret);
}

cl_int CL_API_CALL clRetainProgram(cl_program program)
{
	return CONTEXT_MODULE->RetainProgram(program);
}

cl_int CL_API_CALL clReleaseProgram(cl_program program)
{
	return CONTEXT_MODULE->ReleaseProgram(program);
}

cl_int CL_API_CALL clBuildProgram(cl_program           program,
					  cl_uint              num_devices,
					  const cl_device_id * device_list,
					  const char *         options, 
					  void (*pfn_notify)(cl_program program, void * user_data),
					  void *               user_data)
{
	return CONTEXT_MODULE->BuildProgram(program, num_devices, device_list, options, pfn_notify, user_data);
}

cl_int CL_API_CALL clUnloadCompiler(void)
{
	return PLATFORM_MODULE->UnloadCompiler();
}

cl_int CL_API_CALL clGetProgramInfo(cl_program      program,
						cl_program_info param_name,
						size_t          param_value_size,
						void *          param_value,
						size_t *        param_value_size_ret)
{
	return CONTEXT_MODULE->GetProgramInfo(program, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_int CL_API_CALL clGetProgramBuildInfo(cl_program            program,
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
cl_kernel CL_API_CALL clCreateKernel(cl_program   program,
						 const char * kernel_name,
						 cl_int *     errcode_ret)
{
	return CONTEXT_MODULE->CreateKernel(program, kernel_name, errcode_ret);
}

cl_int CL_API_CALL clCreateKernelsInProgram(cl_program  program,
								cl_uint     num_kernels,
								cl_kernel * kernels,
								cl_uint *   num_kernels_ret)
{
	return CONTEXT_MODULE->CreateKernelsInProgram(program, num_kernels, kernels, num_kernels_ret);
}

cl_int CL_API_CALL clRetainKernel(cl_kernel kernel)
{
	return CONTEXT_MODULE->RetainKernel(kernel);
}

cl_int CL_API_CALL clReleaseKernel(cl_kernel kernel)
{
	return CONTEXT_MODULE->ReleaseKernel(kernel);
}

cl_int CL_API_CALL clSetKernelArg(cl_kernel    kernel,
					  cl_uint      arg_indx,
					  size_t       arg_size,
					  const void * arg_value)
{
	return CONTEXT_MODULE->SetKernelArg(kernel, arg_indx, arg_size, arg_value);
}

cl_int CL_API_CALL clGetKernelInfo(cl_kernel      kernel,
					   cl_kernel_info param_name,
					   size_t         param_value_size,
					   void *         param_value,
					   size_t *       param_value_size_ret)
{
	return CONTEXT_MODULE->GetKernelInfo(kernel, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_int CL_API_CALL clGetKernelWorkGroupInfo(cl_kernel                 kernel,
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
cl_int CL_API_CALL clWaitForEvents(cl_uint num_events, const cl_event * event_list)
{
	return EXECUTION_MODULE->WaitForEvents(num_events, event_list);
}
cl_int CL_API_CALL clGetEventInfo(cl_event		event,
					  cl_event_info	param_name,
					  size_t		param_value_size,
					  void *		param_value,
					  size_t *		param_value_size_ret)
{
	return EXECUTION_MODULE->GetEventInfo(event, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_int CL_API_CALL clRetainEvent(cl_event event)
{
	return EXECUTION_MODULE->RetainEvent(event);
}

cl_int CL_API_CALL clReleaseEvent(cl_event event)
{
	return EXECUTION_MODULE->ReleaseEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Profiling APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_int CL_API_CALL clGetEventProfilingInfo(cl_event				event,
							   cl_profiling_info	param_name,
							   size_t				param_value_size,
							   void *				param_value,
							   size_t *				param_value_size_ret)
{
	return EXECUTION_MODULE->GetEventProfilingInfo(event, param_name, param_value_size, param_value, param_value_size_ret);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Flush and Finish APIs
///////////////////////////////////////////////////////////////////////////////////////////////////                              
cl_int CL_API_CALL clFlush(cl_command_queue command_queue)
{
	return EXECUTION_MODULE->Flush(command_queue);	
}
cl_int CL_API_CALL clFinish(cl_command_queue command_queue)
{
	return EXECUTION_MODULE->Finish(command_queue);	
}

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
	return EXECUTION_MODULE->EnqueueReadBuffer(command_queue, buffer, blocking_read, offset, cb, ptr, num_events_in_wait_list, event_wait_list, event);
}
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
	return EXECUTION_MODULE->EnqueueWriteBuffer(command_queue, buffer, blocking_write, offset, cb, ptr, num_events_in_wait_list, event_wait_list, event);
}
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
	return EXECUTION_MODULE->EnqueueCopyBuffer(command_queue, src_buffer, dst_buffer, src_offset, dst_offset, cb, num_events_in_wait_list, event_wait_list, event);
}
                            
cl_int CL_API_CALL clEnqueueReadImage(cl_command_queue command_queue,
						  cl_mem			image,
						  cl_bool			blocking_read, 
						  const size_t	    origin[3],
						  const size_t	    region[3],
						  size_t			row_pitch,
						  size_t			slice_pitch, 
						  void *			ptr,
						  cl_uint			num_events_in_wait_list,
						  const cl_event *	event_wait_list,
						  cl_event *		event)
{
	return EXECUTION_MODULE->EnqueueReadImage(command_queue, image, blocking_read, origin, region, row_pitch, slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
}

cl_int CL_API_CALL clEnqueueWriteImage(cl_command_queue command_queue,
						   cl_mem			image,
						   cl_bool			blocking_write, 
						   const size_t	    origin[3],
						   const size_t	    region[3],
						   size_t			input_row_pitch,
						   size_t			input_slice_pitch, 
						   const void *		ptr,
						   cl_uint			num_events_in_wait_list,
						   const cl_event *	event_wait_list,
						   cl_event *		event)
{
	return EXECUTION_MODULE->EnqueueWriteImage(command_queue, image, blocking_write, origin, region, input_row_pitch, input_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
}

cl_int CL_API_CALL clEnqueueCopyImage(cl_command_queue	command_queue,
						  cl_mem			src_image,
						  cl_mem			dst_image, 
						  const size_t  	src_origin[3],
						  const size_t  	dst_origin[3],
						  const size_t  	region[3], 
						  cl_uint			num_events_in_wait_list,
						  const cl_event *	event_wait_list,
						  cl_event *		event)
{
	return EXECUTION_MODULE->EnqueueCopyImage(command_queue, src_image, dst_image, src_origin, dst_origin, region, num_events_in_wait_list, event_wait_list, event);
}

cl_int CL_API_CALL clEnqueueCopyImageToBuffer(cl_command_queue	command_queue,
								  cl_mem			src_image,
								  cl_mem			dst_buffer,
								  const size_t  	src_origin[3],
								  const size_t  	region[3],
								  size_t			dst_offset,
								  cl_uint			num_events_in_wait_list,
								  const cl_event *	event_wait_list,
								  cl_event *		event)
{
	return EXECUTION_MODULE->EnqueueCopyImageToBuffer(command_queue, src_image, dst_buffer, src_origin, region, dst_offset, num_events_in_wait_list, event_wait_list, event);
}

cl_int CL_API_CALL clEnqueueCopyBufferToImage(cl_command_queue	command_queue,
								  cl_mem			src_buffer,
								  cl_mem			dst_image,
								  size_t			src_offset,
								  const size_t  	dst_origin[3],
								  const size_t  	region[3], 
								  cl_uint			num_events_in_wait_list,
								  const cl_event *	event_wait_list,
								  cl_event *		event)
{
	return EXECUTION_MODULE->EnqueueCopyBufferToImage(command_queue, src_buffer, dst_image, src_offset, dst_origin, region, num_events_in_wait_list, event_wait_list, event);
}
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
	return EXECUTION_MODULE->EnqueueMapBuffer(command_queue, buffer, blocking_map, map_flags, offset, cb, num_events_in_wait_list, event_wait_list, event, errcode_ret);
}

void * CL_API_CALL clEnqueueMapImage(cl_command_queue	command_queue,
						 cl_mem				image,
						 cl_bool			blocking_map,
						 cl_map_flags		map_flags,
						 const size_t 		origin[3],
						 const size_t 		region[3],
						 size_t *			image_row_pitch,
						 size_t *			image_slice_pitch,
						 cl_uint			num_events_in_wait_list,
						 const cl_event *	event_wait_list,
						 cl_event *			event,
						 cl_int *			errcode_ret)
{
	return EXECUTION_MODULE->EnqueueMapImage(command_queue, image, blocking_map, map_flags, origin, region, image_row_pitch, image_slice_pitch, num_events_in_wait_list, event_wait_list, event, errcode_ret);
}

cl_int CL_API_CALL clEnqueueUnmapMemObject(cl_command_queue	command_queue,
							   cl_mem			memobj,
							   void *			mapped_ptr,
							   cl_uint			num_events_in_wait_list,
							   const cl_event * event_wait_list,
							   cl_event *		event)
{
	return EXECUTION_MODULE->EnqueueUnmapMemObject(command_queue, memobj, mapped_ptr, num_events_in_wait_list, event_wait_list, event);
}

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
	return EXECUTION_MODULE->EnqueueNDRangeKernel(command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event);
}

cl_int CL_API_CALL clEnqueueTask(cl_command_queue	command_queue,
					 cl_kernel			kernel,
					 cl_uint			num_events_in_wait_list,
					 const cl_event *	event_wait_list,
					 cl_event *			event)
{
	return EXECUTION_MODULE->EnqueueTask(command_queue, kernel, num_events_in_wait_list, event_wait_list, event);
}

cl_int CL_API_CALL clEnqueueNativeKernel(cl_command_queue	command_queue,
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
	return EXECUTION_MODULE->EnqueueNativeKernel(command_queue, user_func, args, cb_args, num_mem_objects, mem_list, args_mem_loc, num_events_in_wait_list, event_wait_list, event);
}

cl_int CL_API_CALL clEnqueueMarker(cl_command_queue command_queue, cl_event * event)
{
	return EXECUTION_MODULE->EnqueueMarker(command_queue, event);
}

cl_int CL_API_CALL clEnqueueWaitForEvents(cl_command_queue	command_queue,
							  cl_uint			num_events,
							  const cl_event *	event_list)
{
	return EXECUTION_MODULE->EnqueueWaitForEvents(command_queue, num_events, event_list);
}

cl_int CL_API_CALL clEnqueueBarrier(cl_command_queue command_queue)
{
	return EXECUTION_MODULE->EnqueueBarrier(command_queue);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Sharing Memory Objects with OpenGL / OpenGL ES Buffer, Texture and Renderbuffer Objects
///////////////////////////////////////////////////////////////////////////////////////////////////                              
cl_mem CL_API_CALL clCreateFromGLBuffer(cl_context   context,
							cl_mem_flags flags,
							GLuint       bufobj,
							int *        errcode_ret)
{
	return CONTEXT_MODULE->CreateFromGLBuffer(context, flags, bufobj, errcode_ret);
}
cl_mem CL_API_CALL clCreateFromGLTexture2D(cl_context   context,
							   cl_mem_flags flags,
							   GLenum       target,
							   GLint        miplevel,
							   GLuint       texture,
							   cl_int *     errcode_ret)
{
	return CONTEXT_MODULE->CreateFromGLTexture2D(context, flags, target, miplevel, texture, errcode_ret);
}
cl_mem CL_API_CALL clCreateFromGLTexture3D(cl_context   context,
							   cl_mem_flags flags,
							   GLenum       target,
							   GLint        miplevel,
							   GLuint       texture,
							   cl_int *     errcode_ret)
{
	return CONTEXT_MODULE->CreateFromGLTexture3D(context, flags, target, miplevel, texture, errcode_ret);
}
cl_mem CL_API_CALL clCreateFromGLRenderbuffer(cl_context   context,
								  cl_mem_flags flags,
								  GLuint       renderbuffer,
								  cl_int *     errcode_ret)
{
	return CONTEXT_MODULE->CreateFromGLRenderbuffer(context, flags, renderbuffer, errcode_ret);
}
cl_int CL_API_CALL clGetGLObjectInfo(cl_mem              memobj,
						 cl_gl_object_type * gl_object_type,
						 GLuint *            gl_object_name)
{
	return CONTEXT_MODULE->GetGLObjectInfo(memobj, gl_object_type, gl_object_name);
}
cl_int CL_API_CALL clGetGLTextureInfo(cl_mem             memobj,
						  cl_gl_texture_info param_name,
						  size_t             param_value_size,
						  void *             param_value,
						  size_t *           param_value_size_ret)
{
	return CONTEXT_MODULE->GetGLTextureInfo(memobj, param_name, param_value_size, param_value, param_value_size_ret);
}
cl_int CL_API_CALL clEnqueueAcquireGLObjects(cl_command_queue command_queue,
								 cl_uint          num_objects,
								 const cl_mem *   mem_objects,
								 cl_uint          num_events_in_wait_list,
								 const cl_event * event_wait_list,
								 cl_event *       event)
{
	return EXECUTION_MODULE->EnqueueAcquireGLObjects(command_queue, num_objects, mem_objects, num_events_in_wait_list, event_wait_list, event);
}
cl_int CL_API_CALL clEnqueueReleaseGLObjects(cl_command_queue command_queue,
								 cl_uint          num_objects,
								 const cl_mem *   mem_objects,
								 cl_uint          num_events_in_wait_list,
								 const cl_event * event_wait_list,
								 cl_event *       event)
{
	return EXECUTION_MODULE->EnqueueReleaseGLObjects(command_queue, num_objects, mem_objects, num_events_in_wait_list, event_wait_list, event);
}


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
	if (NULL != *errcode_ret)
	{
		*errcode_ret = CL_ERR_NOT_IMPLEMENTED;
	}
	return 0;
}
cl_int clRetainCommandQueue(cl_command_queue command_queue)
{
	return CL_ERR_NOT_IMPLEMENTED;
}
cl_int clReleaseCommandQueue(cl_command_queue command_queue)
{
	return CL_ERR_NOT_IMPLEMENTED;
}
cl_int clGetCommandQueueInfo(cl_command_queue      command_queue, 
							 cl_command_queue_info param_name, 
							 size_t                param_value_size, 
							 void *                param_value, 
							 size_t *              param_value_size_ret)
{
	return CL_ERR_NOT_IMPLEMENTED;
}
cl_int clSetCommandQueueProperty(cl_command_queue              command_queue, 
								 cl_command_queue_properties   properties, 
								 cl_int                        enable, 
								 cl_command_queue_properties * old_properties)
{
	return CL_ERR_NOT_IMPLEMENTED;
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
	if (NULL != *errcode_ret)
	{
		*errcode_ret = CL_ERR_NOT_IMPLEMENTED;
	}
	return 0;
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
	if (NULL != *errcode_ret)
	{
		*errcode_ret = CL_ERR_NOT_SUPPORTED;
	}
	return 0;
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
	if (NULL != *errcode_ret)
	{
		*errcode_ret = CL_ERR_NOT_SUPPORTED;
	}
	return 0;
}
                        
cl_int clRetainMemObject(cl_mem memobj)
{
	return CL_ERR_NOT_IMPLEMENTED;
}

cl_int clReleaseMemObject(cl_mem memobj)
{
	return CL_ERR_NOT_IMPLEMENTED;
}

cl_int clGetSupportedImageFormats(cl_context           context,
								  cl_mem_flags         flags,
								  cl_mem_object_type   image_type,
								  cl_uint              num_entries,
								  cl_image_format *    image_formats,
								  cl_uint *            num_image_formats)
{
	return CL_ERR_NOT_SUPPORTED;
}
                                    
cl_int clGetMemObjectInfo(cl_mem           memobj,
						  cl_mem_info      param_name, 
						  size_t           param_value_size,
						  void *           param_value,
						  size_t *         param_value_size_ret)
{
	return CL_ERR_NOT_IMPLEMENTED;
}

cl_int clGetImageInfo(cl_mem           image,
					  cl_image_info    param_name, 
					  size_t           param_value_size,
					  void *           param_value,
					  size_t *         param_value_size_ret)
{
	return CL_ERR_NOT_SUPPORTED;
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
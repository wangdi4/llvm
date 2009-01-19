// OpenCL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "cl_types.h"
#include "cl_objects_map.h"
#include "framework_proxy.h"

using namespace Intel::OpenCL::Framework;

wchar_t* ClErrTxt(cl_err_code error_code)
{
	switch(error_code)
	{
	case (CL_SUCCESS): return L"CL_SUCCESS";
	case (CL_DEVICE_NOT_FOUND): return L"CL_DEVICE_NOT_FOUND";
	case (CL_DEVICE_NOT_AVAILABLE): return L"CL_DEVICE_NOT_AVAILABLE";
	case (CL_DEVICE_COMPILER_NOT_AVAILABLE): return L"CL_DEVICE_COMPILER_NOT_AVAILABLE";
	case (CL_MEM_OBJECT_ALLOCATION_FAILURE): return L"CL_MEM_OBJECT_ALLOCATION_FAILURE";
	case (CL_OUT_OF_RESOURCES): return L"CL_OUT_OF_RESOURCES";
	case (CL_OUT_OF_HOST_MEMORY): return L"CL_OUT_OF_HOST_MEMORY";
	case (CL_PROFILING_INFO_NOT_AVAILABLE): return L"CL_PROFILING_INFO_NOT_AVAILABLE";
	case (CL_MEM_COPY_OVERLAP): return L"CL_MEM_COPY_OVERLAP";
	case (CL_IMAGE_FORMAT_MISMATCH): return L"CL_IMAGE_FORMAT_MISMATCH";
	case (CL_IMAGE_FORMAT_NOT_SUPPORTED): return L"CL_IMAGE_FORMAT_NOT_SUPPORTED";
	case (CL_INVALID_VALUE): return L"CL_INVALID_VALUE";
	case (CL_INVALID_DEVICE_TYPE): return L"CL_INVALID_DEVICE_TYPE";
	case (CL_INVALID_DEVICE): return L"CL_INVALID_DEVICE";
	case (CL_INVALID_CONTEXT): return L"CL_INVALID_CONTEXT";
	case (CL_INVALID_QUEUE_PROPERTIES): return L"CL_INVALID_QUEUE_PROPERTIES";
	case (CL_INVALID_COMMAND_QUEUE): return L"CL_INVALID_COMMAND_QUEUE";
	case (CL_INVALID_HOST_PTR): return L"CL_INVALID_HOST_PTR";
	case (CL_INVALID_MEM_OBJECT): return L"CL_INVALID_MEM_OBJECT";
	case (CL_INVALID_IMAGE_FORMAT_DESCRIPTOR): return L"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	case (CL_INVALID_IMAGE_SIZE): return L"CL_INVALID_IMAGE_SIZE";
	case (CL_INVALID_SAMPLER): return L"CL_INVALID_SAMPLER";
	case (CL_INVALID_BINARY): return L"CL_INVALID_BINARY";
	case (CL_INVALID_BUILD_OPTIONS): return L"CL_INVALID_BUILD_OPTIONS";
	case (CL_INVALID_PROGRAM): return L"CL_INVALID_PROGRAM";
	case (CL_INVALID_PROGRAM_EXECUTABLE): return L"CL_INVALID_PROGRAM_EXECUTABLE";
	case (CL_INVALID_KERNEL_NAME): return L"CL_INVALID_KERNEL_NAME";
	case (CL_INVALID_KERNEL): return L"CL_INVALID_KERNEL";
	case (CL_INVALID_ARG_INDEX): return L"CL_INVALID_ARG_INDEX";
	case (CL_INVALID_ARG_VALUE): return L"CL_INVALID_ARG_VALUE";
	case (CL_INVALID_ARG_SIZE): return L"CL_INVALID_ARG_SIZE";
	case (CL_INVALID_KERNEL_ARGS): return L"CL_INVALID_KERNEL_ARGS";
	case (CL_INVALID_WORK_DIMENSION): return L"CL_INVALID_WORK_DIMENSION";
	case (CL_INVALID_WORK_GROUP_SIZE): return L"CL_INVALID_WORK_GROUP_SIZE";
	case (CL_INVALID_WORK_ITEM_SIZE): return L"CL_INVALID_WORK_ITEM_SIZE";
	case (CL_INVALID_GLOBAL_OFFSET): return L"CL_INVALID_GLOBAL_OFFSET";
	case (CL_INVALID_EVENT_WAIT_LIST): return L"CL_INVALID_EVENT_WAIT_LIST";
	case (CL_INVALID_EVENT): return L"CL_INVALID_EVENT";
	case (CL_INVALID_OPERATION): return L"CL_INVALID_GL_OBJECT";
	case (CL_INVALID_GL_OBJECT): return L"CL_INVALID_GL_OBJECT";
	case (CL_INVALID_BUFFER_SIZE): return L"CL_INVALID_BUFFER_SIZE";
	case (CL_ERR_LOGGER_FAILED): return L"CL_ERR_LOGGER_FAILED";
	case (CL_ERR_NOT_IMPLEMENTED): return L"CL_ERR_NOT_IMPLEMENTED";
	case (CL_ERR_NOT_SUPPORTED): return L"CL_ERR_NOT_SUPPORTED";
	case (CL_ERR_INITILIZATION_FAILED): return L"CL_ERR_INITILIZATION_FAILED";
	case (CL_ERR_PLATFORM_FAILED): return L"CL_ERR_PLATFORM_FAILED";
	case (CL_ERR_CONTEXT_FAILED): return L"CL_ERR_CONTEXT_FAILED";
	case (CL_ERR_EXECUTION_FAILED): return L"CL_ERR_EXECUTION_FAILED";
	case (CL_ERR_FILE_NOT_EXISTS): return L"CL_ERR_FILE_NOT_EXISTS";
	case (CL_ERR_KEY_NOT_FOUND): return L"CL_ERR_KEY_NOT_FOUND";
	case (CL_ERR_KEY_ALLREADY_EXISTS): return L"CL_ERR_KEY_ALLREADY_EXISTS";
	case (CL_ERR_LIST_EMPTY): return L"CL_ERR_LIST_EMPTY";
	case (CL_ERR_DEVICE_INIT_FAIL): return L"CL_ERR_DEVICE_INIT_FAIL";
	default: return L"Unknown Error Code";
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Platform APIs
///////////////////////////////////////////////////////////////////////////////////////////////////

cl_int clGetPlatformInfo(cl_platform_info param_name, 
						 size_t param_value_size, 
						 void* param_value, 
						 size_t* param_value_size_ret)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = Intel::OpenCL::Framework::FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		CL_ERR_INITILIZATION_FAILED;
	}
	// get the platform module
	PlatformModule *pPlatformModule = pFramework->GetPlatformModule();
	if (NULL == pPlatformModule)
	{
		CL_ERR_PLATFORM_FAILED;
	}
	cl_err_code clRet = pPlatformModule->GetPlatformInfo(param_name, param_value_size, param_value, param_value_size_ret);
	return (CL_ERR_OUT(clRet));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Device APIs
///////////////////////////////////////////////////////////////////////////////////////////////////

cl_int clGetDeviceIDs(cl_device_type device_type, 
					  cl_uint num_entries, 
					  cl_device_id* devices, 
			          cl_uint* num_devices)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = Intel::OpenCL::Framework::FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		CL_ERR_INITILIZATION_FAILED;
	}
	// get the platform module
	PlatformModule *pPlatformModule = pFramework->GetPlatformModule();
	if (NULL == pPlatformModule)
	{
		CL_ERR_PLATFORM_FAILED;
	}
	cl_err_code clRet = pPlatformModule->GetDeviceIDs(device_type, num_entries, devices, num_devices);
	return (CL_ERR_OUT(clRet));
}

cl_int clGetDeviceInfo(cl_device_id device,
					   cl_device_info param_name, 
					   size_t param_value_size, 
					   void* param_value,
					   size_t* param_value_size_ret)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		CL_ERR_INITILIZATION_FAILED;
	}
	// get the platform module
	PlatformModule *pPlatformModule = pFramework->GetPlatformModule();
	if (NULL == pPlatformModule)
	{
		CL_ERR_PLATFORM_FAILED;
	}
	cl_err_code clRet = pPlatformModule->GetDeviceInfo(device, param_name, param_value_size, param_value, param_value_size_ret);
	return (CL_ERR_OUT(clRet));
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
	cl_err_code clRet = CL_SUCCESS;
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		*errcode_ret = CL_ERR_INITILIZATION_FAILED;
		return 0;
	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		*errcode_ret = CL_ERR_CONTEXT_FAILED;
		return 0;
	}
	cl_context iContextId = pContextModule->CreateContext(properties, num_devices, devices, pfn_notify, user_data, &clRet);
	*errcode_ret = CL_ERR_OUT(clRet);
	return iContextId;
}

cl_context clCreateContextFromType(cl_context_properties properties,
								   cl_device_type        device_type,
								   logging_fn            pfn_notify,
								   void *                user_data,
								   cl_int *              errcode_ret)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		if (NULL != errcode_ret)
		{
			*errcode_ret = CL_ERR_INITILIZATION_FAILED;
		}
		return CL_INVALID_HANDLE;
	}
	// get the platform module
	PlatformModule *pPlatformModule = pFramework->GetPlatformModule();
	if (NULL == pPlatformModule)
	{
		if (NULL != errcode_ret)
		{
			*errcode_ret = CL_ERR_PLATFORM_FAILED;
		}
		return CL_INVALID_HANDLE;
	}
	cl_uint uiNumDevices = 0;
	// get number of devices for device type
	cl_err_code clRet = pPlatformModule->GetDeviceIDs(device_type, 0, NULL, &uiNumDevices);
	if (CL_FAILED(clRet))
	{
		if (NULL != errcode_ret)
		{
			*errcode_ret = CL_ERR_OUT(clRet);
		}
		return CL_INVALID_HANDLE;
	}
	// allocate array for devices
	cl_device_id * pDevices = new cl_device_id[uiNumDevices];
	if (NULL == pDevices)
	{
		if (NULL != errcode_ret)
		{
			*errcode_ret = CL_ERR_INITILIZATION_FAILED;
		}
		return CL_INVALID_HANDLE;
	}
	// get devices
	clRet = pPlatformModule->GetDeviceIDs(device_type, uiNumDevices, pDevices, NULL);
	if (CL_FAILED(clRet))
	{
		delete[] pDevices;
		if (NULL != errcode_ret)
		{
			*errcode_ret = CL_ERR_OUT(clRet);
		}
		return CL_INVALID_HANDLE;
	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		if (NULL != errcode_ret)
		{
			*errcode_ret = CL_ERR_CONTEXT_FAILED;
		}
		return CL_INVALID_HANDLE;
	}
	cl_context iContextId = pContextModule->CreateContext(properties, uiNumDevices, pDevices, pfn_notify, user_data, &clRet);
	if (NULL != errcode_ret)
	{
		*errcode_ret = CL_ERR_OUT(clRet);
	}
	return iContextId;
}

cl_int clRetainContext(cl_context context)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		return CL_ERR_INITILIZATION_FAILED;
	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		return CL_ERR_CONTEXT_FAILED;
	}
	cl_err_code clRet = pContextModule->RetainContext(context);
	return CL_ERR_OUT(clRet);
}

cl_int clReleaseContext(cl_context context)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		return CL_ERR_INITILIZATION_FAILED;
	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		return CL_ERR_CONTEXT_FAILED;
		return 0;
	}
	cl_err_code clRet = pContextModule->ReleaseContext(context);
	return CL_ERR_OUT(clRet);
}

cl_int clGetContextInfo(cl_context      context,
						cl_context_info param_name,
						size_t          param_value_size,
						void *          param_value,
						size_t *        param_value_size_ret)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		return CL_ERR_INITILIZATION_FAILED;
	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		return CL_ERR_CONTEXT_FAILED;
	}
	cl_err_code clRet = pContextModule->GetContextInfo(context, param_name, param_value_size, param_value, param_value_size_ret);
	return CL_ERR_OUT(clRet);
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
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		if (NULL != errcode_ret)
		{
			*errcode_ret = CL_ERR_INITILIZATION_FAILED;
			return CL_INVALID_HANDLE;
		}
	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		if (NULL != errcode_ret)
		{
			*errcode_ret = CL_ERR_CONTEXT_FAILED;
			return CL_INVALID_HANDLE;
		}
	}
	cl_err_code clRet = CL_SUCCESS;
	cl_program iProgram = pContextModule->CreateProgramWithSource(context, count, strings, lengths, &clRet);
	if (NULL != errcode_ret)
	{
		*errcode_ret = CL_ERR_OUT(clRet);
	}
	return iProgram;
}

cl_program clCreateProgramWithBinary(cl_context           context,
									 cl_uint              num_devices,
									 const cl_device_id * device_list,
									 const size_t *       lengths,
									 const void **        binaries,
									 cl_int *             binary_status,
									 cl_int *             errcode_ret)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		if (NULL != errcode_ret)
		{
			*errcode_ret = CL_ERR_INITILIZATION_FAILED;
			return CL_INVALID_HANDLE;
		}
	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		if (NULL != errcode_ret)
		{
			*errcode_ret = CL_ERR_CONTEXT_FAILED;
			return CL_INVALID_HANDLE;
		}
	}
	cl_err_code clRet = CL_SUCCESS;
	cl_program iProgram = pContextModule->CreateProgramWithBinary(context, num_devices, device_list, lengths, binaries, binary_status, &clRet);
	if (NULL != errcode_ret)
	{
		*errcode_ret = CL_ERR_OUT(clRet);
	}
	return iProgram;
}

cl_int clRetainProgram(cl_program program)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		return CL_ERR_INITILIZATION_FAILED;
	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		return CL_ERR_CONTEXT_FAILED;
	}
	cl_err_code clRet = pContextModule->RetainProgram(program);
	return CL_ERR_OUT(clRet);
}

cl_int clReleaseProgram(cl_program program)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		return CL_ERR_INITILIZATION_FAILED;
	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		return CL_ERR_CONTEXT_FAILED;
		return 0;
	}
	cl_err_code clRet = pContextModule->ReleaseProgram(program);
	return CL_ERR_OUT(clRet);
}

cl_int clBuildProgram(cl_program           program,
					  cl_uint              num_devices,
					  const cl_device_id * device_list,
					  const char *         options, 
					  void (*pfn_notify)(cl_program program, void * user_data),
					  void *               user_data)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		return CL_ERR_INITILIZATION_FAILED;
	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		return CL_ERR_CONTEXT_FAILED;
		return 0;
	}
	cl_err_code clRet = pContextModule->BuildProgram(program, num_devices, device_list, options, pfn_notify, user_data);
	return CL_ERR_OUT(clRet);
}

cl_int clUnloadCompiler(void)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		return CL_ERR_INITILIZATION_FAILED;
	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		return CL_ERR_CONTEXT_FAILED;
		return 0;
	}
	cl_err_code clRet = pContextModule->UnloadCompiler();
	return CL_ERR_OUT(clRet);
}

cl_int clGetProgramInfo(cl_program      program,
						cl_program_info param_name,
						size_t          param_value_size,
						void *          param_value,
						size_t *        param_value_size_ret)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		return CL_ERR_INITILIZATION_FAILED;
	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		return CL_ERR_CONTEXT_FAILED;
		return 0;
	}
	cl_err_code clRet = pContextModule->GetProgramInfo(program, param_name, param_value_size, param_value, param_value_size_ret);
	return CL_ERR_OUT(clRet);
}

cl_int clGetProgramBuildInfo(cl_program            program,
							 cl_device_id          device,
							 cl_program_build_info param_name,
							 size_t                param_value_size,
							 void *                param_value,
							 size_t *              param_value_size_ret)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		return CL_ERR_INITILIZATION_FAILED;
	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		return CL_ERR_CONTEXT_FAILED;
	}
	cl_err_code clRet = pContextModule->GetProgramBuildInfo(program, device, param_name, param_value_size, param_value, param_value_size_ret);
	return CL_ERR_OUT(clRet);
}
                           
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel Object APIs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_kernel clCreateKernel(cl_program   program,
						 const char * kernel_name,
						 cl_int *     errcode_ret)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		if (NULL != *errcode_ret)
		{
			*errcode_ret = CL_ERR_INITILIZATION_FAILED;
		}
		return CL_INVALID_HANDLE;

	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		if (NULL != *errcode_ret)
		{
			*errcode_ret = CL_ERR_CONTEXT_FAILED;
		}
		return CL_INVALID_HANDLE;
	}
	cl_err_code clRet = CL_SUCCESS;
	cl_kernel clKernel = pContextModule->CreateKernel(program, kernel_name, &clRet);
	if (NULL != *errcode_ret)
	{
		*errcode_ret = CL_ERR_OUT(clRet);
	}
	return clKernel;
}

cl_int clCreateKernelsInProgram(cl_program  program,
								cl_uint     num_kernels,
								cl_kernel * kernels,
								cl_uint *   num_kernels_ret)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		return CL_ERR_INITILIZATION_FAILED;

	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		return CL_ERR_CONTEXT_FAILED;
	}
	cl_err_code clRet = pContextModule->CreateKernelsInProgram(program, num_kernels, kernels, num_kernels_ret);
	return CL_ERR_OUT(clRet);
}

cl_int clRetainKernel(cl_kernel kernel)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		return CL_ERR_INITILIZATION_FAILED;

	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		return CL_ERR_CONTEXT_FAILED;
	}
	cl_err_code clRet = pContextModule->RetainKernel(kernel);
	return CL_ERR_OUT(clRet);
}

cl_int clReleaseKernel(cl_kernel kernel)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		return CL_ERR_INITILIZATION_FAILED;

	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		return CL_ERR_CONTEXT_FAILED;
	}
	cl_err_code clRet = pContextModule->ReleaseKernel(kernel);
	return CL_ERR_OUT(clRet);
}

cl_int clSetKernelArg(cl_kernel    kernel,
					  cl_uint      arg_indx,
					  size_t       arg_size,
					  const void * arg_value)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		return CL_ERR_INITILIZATION_FAILED;

	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		return CL_ERR_CONTEXT_FAILED;
	}
	cl_err_code clRet = pContextModule->SetKernelArg(kernel, arg_indx, arg_size, arg_value);
	return CL_ERR_OUT(clRet);
}

cl_int clGetKernelInfo(cl_kernel      kernel,
					   cl_kernel_info param_name,
					   size_t         param_value_size,
					   void *         param_value,
					   size_t *       param_value_size_ret)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		return CL_ERR_INITILIZATION_FAILED;

	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		return CL_ERR_CONTEXT_FAILED;
	}
	cl_err_code clRet = pContextModule->GetKernelInfo(kernel, param_name, param_value_size, param_value, param_value_size_ret);
	return CL_ERR_OUT(clRet);
}

cl_int clGetKernelWorkGroupInfo(cl_kernel                 kernel,
								cl_device_id              device,
								cl_kernel_work_group_info param_name,
								size_t                    param_value_size,
								void *                    param_value,
								size_t *                  param_value_size_ret)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		return CL_ERR_INITILIZATION_FAILED;

	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		return CL_ERR_CONTEXT_FAILED;
	}
	cl_err_code clRet = pContextModule->GetKernelWorkGroupInfo(kernel, device, param_name, param_value_size, param_value, param_value_size_ret);
	return CL_ERR_OUT(clRet);
}
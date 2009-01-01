// OpenCL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "cl_types.h"
#include "framework_proxy.h"

using namespace Intel::OpenCL::Framework;

wchar_t* ClErrTxt(cl_err_code error_code)
{
	switch(error_code)
	{
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

cl_int TEST(cl_int test_number)
{
	OCLObjectsMap *pObjMap = new OCLObjectsMap();
	OCLObject *pObj1 = new OCLObject();
	OCLObject *pObj2 = new OCLObject();
	OCLObject *pObj3 = new OCLObject();

	cl_err_code clErrRet = CL_SUCCESS;

	cl_int id = pObjMap->AddObject(pObj1);
	id = pObjMap->AddObject(pObj2);
	id = pObjMap->AddObject(pObj3);
	clErrRet = pObjMap->RemoveObject(pObj1->GetId());
	clErrRet = pObjMap->RemoveObject(pObj2->GetId());
	clErrRet = pObjMap->RemoveObject(pObj3->GetId());
	clErrRet = pObjMap->RemoveObject(pObj1->GetId());
	return CL_SUCCESS;
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
		CL_ERR_RET(errcode_ret, CL_ERR_OUT(CL_ERR_INITILIZATION_FAILED));
		return 0;
	}
	// get the platform module
	PlatformModule *pPlatformModule = pFramework->GetPlatformModule();
	if (NULL == pPlatformModule)
	{
		CL_ERR_RET(errcode_ret, CL_ERR_OUT(CL_ERR_PLATFORM_FAILED));
		return 0;
	}
	cl_uint uiNumDevices = 0;
	// get number of devices for device type
	cl_err_code clRet = pPlatformModule->GetDeviceIDs(device_type, 0, NULL, &uiNumDevices);
	if (CL_FAILED(clRet))
	{
		CL_ERR_RET(errcode_ret, CL_ERR_OUT(clRet));
		return 0;
	}
	// allocate array for devices
	cl_device_id * pDevices = new cl_device_id[uiNumDevices];
	if (NULL == pDevices)
	{
		CL_ERR_RET(errcode_ret, CL_ERR_OUT(CL_ERR_INITILIZATION_FAILED));
		return 0;
	}
	// get devices
	clRet = pPlatformModule->GetDeviceIDs(device_type, uiNumDevices, pDevices, NULL);
	if (CL_FAILED(clRet))
	{
		delete[] pDevices;
		CL_ERR_RET(errcode_ret, CL_ERR_OUT(clRet));
		return 0;
	}
	// get the context module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		CL_ERR_RET(errcode_ret, CL_ERR_OUT(CL_ERR_CONTEXT_FAILED));
		return 0;
	}
	cl_context iContextId = pContextModule->CreateContext(properties, uiNumDevices, pDevices, pfn_notify, user_data, &clRet);
	CL_ERR_RET(errcode_ret, CL_ERR_OUT(clRet));
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
cl_command_queue clCreateCommandQueue(cl_context context, cl_device_id device, cl_command_queue_properties properties, cl_int * errcode_ret)
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
cl_int clGetCommandQueueInfo(cl_command_queue command_queue, cl_command_queue_info param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret)
{
	return CL_ERR_NOT_IMPLEMENTED;
}
cl_int clSetCommandQueueProperty(cl_command_queue command_queue, cl_command_queue_properties properties, cl_int enable, cl_command_queue_properties * old_properties)
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